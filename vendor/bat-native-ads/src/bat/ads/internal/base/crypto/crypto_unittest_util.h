/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_BASE_CRYPTO_CRYPTO_UNITTEST_UTIL_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_BASE_CRYPTO_CRYPTO_UNITTEST_UTIL_H_

#include <cstdint>
#include <vector>

namespace ads {
namespace security {

std::vector<uint8_t> Decrypt(const std::vector<uint8_t>& ciphertext,
                             const std::vector<uint8_t>& nonce,
                             const std::vector<uint8_t>& ephemeral_public_key,
                             const std::vector<uint8_t>& secret_key);

}  // namespace security
}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_BASE_CRYPTO_CRYPTO_UNITTEST_UTIL_H_
