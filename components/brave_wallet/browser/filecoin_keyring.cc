/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_wallet/browser/filecoin_keyring.h"

#include <utility>

#include "base/base64.h"
#include "base/json/json_reader.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/stringprintf.h"
#include "brave/components/brave_wallet/browser/brave_wallet_utils.h"
#include "brave/components/brave_wallet/browser/fil_transaction.h"
#include "brave/components/brave_wallet/common/brave_wallet.mojom.h"
#include "brave/components/brave_wallet/common/fil_address.h"
#include "brave/components/filecoin/rs/src/lib.rs.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

namespace brave_wallet {

namespace {
bool GetBLSPublicKey(const std::vector<uint8_t>& private_key,
                     std::vector<uint8_t>* public_key_out) {
  if (private_key.size() != 32 || !public_key_out)
    return false;

  auto result = filecoin::bls_private_key_to_public_key(
      rust::Slice<const uint8_t>{private_key.data(), private_key.size()});
  std::vector<uint8_t> public_key(result.begin(), result.end());
  if (std::all_of(public_key.begin(), public_key.end(),
                  [](int i) { return i == 0; }))
    return false;
  *public_key_out = public_key;
  return true;
}
}  // namespace

FilecoinKeyring::FilecoinKeyring() = default;
FilecoinKeyring::~FilecoinKeyring() = default;

// static
bool FilecoinKeyring::DecodeImportPayload(
    const std::string& payload_hex,
    std::vector<uint8_t>* private_key_out,
    mojom::FilecoinAddressProtocol* protocol_out) {
  if (!private_key_out || !protocol_out || payload_hex.empty())
    return false;
  std::string key_payload;
  if (!base::HexStringToString(payload_hex, &key_payload)) {
    return false;
  }
  base::JSONReader::ValueWithError value_with_error =
      base::JSONReader::ReadAndReturnValueWithError(
          key_payload, base::JSON_PARSE_CHROMIUM_EXTENSIONS |
                           base::JSONParserOptions::JSON_PARSE_RFC);
  absl::optional<base::Value>& records_v = value_with_error.value;
  if (!records_v) {
    VLOG(1) << "Invalid payload, could not parse JSON, JSON is: "
            << key_payload;
    return false;
  }

  const base::DictionaryValue* dict = nullptr;
  if (!records_v->GetAsDictionary(&dict)) {
    return false;
  }
  const std::string* type = dict->FindStringKey("Type");
  if (!type || (*type != "secp256k1" && *type != "bls")) {
    return false;
  }

  *protocol_out = *type == "secp256k1"
                      ? mojom::FilecoinAddressProtocol::SECP256K1
                      : mojom::FilecoinAddressProtocol::BLS;

  const std::string* private_key_encoded = dict->FindStringKey("PrivateKey");
  if (!private_key_encoded || private_key_encoded->empty()) {
    return false;
  }
  std::string private_key_decoded;
  if (!base::Base64Decode(*private_key_encoded, &private_key_decoded)) {
    return false;
  }

  *private_key_out = std::vector<uint8_t>(private_key_decoded.begin(),
                                          private_key_decoded.end());
  return true;
}

// static
absl::optional<mojom::FilecoinAddressProtocol>
FilecoinKeyring::GetProtocolFromAddress(const std::string& address) {
  if (address.size() < 2) {
    return absl::nullopt;
  }
  const char protocol_symbol = address[1];
  switch (protocol_symbol) {
    case '1': {
      return mojom::FilecoinAddressProtocol::SECP256K1;
    }
    case '3': {
      return mojom::FilecoinAddressProtocol::BLS;
    }
    default: {
      NOTREACHED() << "Unknown filecoin protocol";
      return absl::nullopt;
    }
  }
  return absl::nullopt;
}

std::string FilecoinKeyring::GetEncodedPrivateKey(const std::string& address) {
  HDKeyBase* key = GetHDKeyFromAddress(address);
  if (!key) {
    return "";
  }
  return GetExportEncodedJSON(
      base::Base64Encode(static_cast<HDKey*>(key)->private_key()), address);
}
// static
std::string FilecoinKeyring::GetExportEncodedJSON(
    const std::string& base64_encoded_private_key,
    const std::string& address) {
  absl::optional<mojom::FilecoinAddressProtocol> protocol =
      GetProtocolFromAddress(address);
  if (!protocol) {
    return "";
  }
  std::string json = base::StringPrintf(
      "{\"Type\":\"%s\",\"PrivateKey\":\"%s\"}",
      protocol.value() == mojom::FilecoinAddressProtocol::BLS ? "bls"
                                                              : "secp256k1",
      base64_encoded_private_key.c_str());
  return base::ToLowerASCII(base::HexEncode(json.data(), json.size()));
}

std::string FilecoinKeyring::ImportFilecoinAccount(
    const std::vector<uint8_t>& private_key,
    const std::string& network,
    mojom::FilecoinAddressProtocol protocol) {
  if (private_key.empty()) {
    return std::string();
  }

  std::unique_ptr<HDKey> hd_key = HDKey::GenerateFromPrivateKey(private_key);
  if (!hd_key)
    return std::string();
  FilAddress address;
  if (protocol == mojom::FilecoinAddressProtocol::BLS) {
    std::vector<uint8_t> public_key;
    if (!GetBLSPublicKey(private_key, &public_key)) {
      return std::string();
    }
    address = FilAddress::FromPayload(public_key, protocol, network);
  } else if (protocol == mojom::FilecoinAddressProtocol::SECP256K1) {
    auto uncompressed_public_key = hd_key->GetUncompressedPublicKey();
    address = FilAddress::FromUncompressedPublicKey(
        uncompressed_public_key, mojom::FilecoinAddressProtocol::SECP256K1,
        network);
  }

  if (address.IsEmpty() ||
      !AddImportedAddress(address.EncodeAsString(), std::move(hd_key))) {
    return std::string();
  }
  return address.EncodeAsString();
}

// This method is used when filecoin account is imported because
// we need to know which protocol to use, so private_key is just not enough.
void FilecoinKeyring::RestoreFilecoinAccount(
    const std::vector<uint8_t>& input_key,
    const std::string& address) {
  std::unique_ptr<HDKey> hd_key = HDKey::GenerateFromPrivateKey(input_key);
  if (!hd_key)
    return;
  if (!AddImportedAddress(address, std::move(hd_key))) {
    return;
  }
}

std::string FilecoinKeyring::GetAddressInternal(HDKeyBase* hd_key_base) const {
  if (!hd_key_base)
    return std::string();
  HDKey* hd_key = static_cast<HDKey*>(hd_key_base);
  // TODO(cypt4): Get network from settings.
  return FilAddress::FromUncompressedPublicKey(
             hd_key->GetUncompressedPublicKey(),
             mojom::FilecoinAddressProtocol::SECP256K1,
             // See description of IsFilecoinTestnetEnabled
             IsFilecoinTestnetEnabled() ? mojom::kFilecoinTestnet
                                        : mojom::kFilecoinMainnet)
      .EncodeAsString();
}

absl::optional<std::string> FilecoinKeyring::SignTransaction(
    const FilTransaction* tx) {
  if (!tx)
    return absl::nullopt;
  auto address = tx->from().EncodeAsString();
  HDKey* hd_key = static_cast<HDKey*>(GetHDKeyFromAddress(address));
  if (!hd_key)
    return absl::nullopt;
  return tx->GetSignedTransaction(hd_key->private_key());
}

}  // namespace brave_wallet
