/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "base/command_line.h"
#include "base/feature_list.h"
#include "base/memory/raw_ptr.h"
#include "base/path_service.h"
#include "base/strings/strcat.h"
#include "base/test/bind.h"
#include "base/test/scoped_feature_list.h"
#include "brave/browser/brave_wallet/brave_wallet_service_factory.h"
#include "brave/browser/brave_wallet/brave_wallet_tab_helper.h"
#include "brave/browser/brave_wallet/json_rpc_service_factory.h"
#include "brave/browser/brave_wallet/keyring_service_factory.h"
#include "brave/browser/brave_wallet/tx_service_factory.h"
#include "brave/components/brave_wallet/browser/brave_wallet_service.h"
#include "brave/components/brave_wallet/browser/brave_wallet_utils.h"
#include "brave/components/brave_wallet/browser/json_rpc_service.h"
#include "brave/components/brave_wallet/browser/keyring_service.h"
#include "brave/components/brave_wallet/browser/tx_service.h"
#include "brave/components/brave_wallet/common/brave_wallet_constants.h"
#include "brave/components/brave_wallet/common/features.h"
#include "brave/components/brave_wallet/common/solana_utils.h"
#include "brave/components/brave_wallet/renderer/resource_helper.h"
#include "brave/components/brave_wallet/resources/grit/brave_wallet_script_generated.h"
#include "brave/components/constants/brave_paths.h"
#include "brave/components/permissions/contexts/brave_wallet_permission_context.h"
#include "chrome/browser/content_settings/host_content_settings_map_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chrome/test/base/ui_test_utils.h"
#include "components/content_settings/core/browser/host_content_settings_map.h"
#include "components/grit/brave_components_strings.h"
#include "content/public/browser/web_contents.h"
#include "content/public/test/browser_test.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "net/dns/mock_host_resolver.h"
#include "net/test/embedded_test_server/embedded_test_server.h"
#include "ui/base/l10n/l10n_util.h"
#include "url/gurl.h"

namespace brave_wallet {

namespace {

static base::NoDestructor<std::string> g_provider_internal_script("");

constexpr char kFirstAccount[] = "8J7fu34oNJSKXcauNQMXRdKAHY7zQ7rEaQng8xtQNpSu";

// First byte = 0 is the length of signatures.
// Rest bytes are from the serialized message below.
// SolanaInstruction instruction(
//     kSolanaSystemProgramId,
//     {SolanaAccountMeta(kFirstAccount, true, true),
//      SolanaAccountMeta(kFirstAccount, false, true)},
//     {2, 0, 0, 0, 128, 150, 152, 0, 0, 0, 0, 0});
// SolanaMessage("9sHcv6xwn9YkB8nxTUGKDwPwNnmqVp5oAXxU8Fdkm4J6", 0,
//               kFirstAccount, {instruction});
constexpr char kUnsignedTxArrayStr[] =
    "0,1,0,1,2,108,100,57,137,161,117,30,158,157,136,81,70,62,51,111,138,48,"
    "102,91,148,103,82,143,30,248,0,4,91,18,170,94,82,0,0,0,0,0,0,0,0,0,0,0,0,"
    "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,131,191,83,201,108,193,222,255,"
    "176,67,136,209,219,42,6,169,240,137,142,185,169,6,17,87,123,6,42,55,162,"
    "64,120,91,1,1,2,0,0,12,2,0,0,0,128,150,152,0,0,0,0,0";
constexpr char kEncodedUnsignedTxArrayStr[] =
    "QwE1cGTGKt9G9zyqwBzqPp711HGUJH15frCMMWp9ooU4DjCQpVSrFxeGxfqnMmw91nWkdFY42H"
    "Wnqqyw2fXYk4kspucEtan8vrRAUPRAr2ansqm52VZe8ocdZudeoXJqHYNPjuPYBawsYFKms5Wu"
    "NBVNFy4UTURd";

// Result of the above transaction signed by kFirstAccount.
constexpr char kSignedTxArrayStr[] =
    "1,39,140,195,60,14,241,115,164,59,230,251,59,231,246,11,104,246,137,211,"
    "101,131,22,147,172,87,182,89,213,67,79,6,233,245,66,112,55,246,89,97,111,"
    "7,99,99,42,32,15,205,69,189,151,25,172,15,166,171,238,153,135,118,192,135,"
    "93,168,10,1,0,1,2,108,100,57,137,161,117,30,158,157,136,81,70,62,51,111,"
    "138,48,102,91,148,103,82,143,30,248,0,4,91,18,170,94,82,0,0,0,0,0,0,0,0,0,"
    "0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,131,191,83,201,108,193,222,"
    "255,176,67,136,209,219,42,6,169,240,137,142,185,169,6,17,87,123,6,42,55,"
    "162,64,120,91,1,1,2,0,0,12,2,0,0,0,128,150,152,0,0,0,0,0";

// Base58 encoded signature of the above transaction.
constexpr char kEncodedSignature[] =
    "ns1aBL6AowxpiPzQL3ZeBK1RpCSLq1VfhqNw9KFSsytayARYdYrqrmbmhaizUTTkT4SXEnjnbV"
    "mPBrie3o9yuyB";

std::unique_ptr<net::test_server::HttpResponse> HandleRequest(
    const net::test_server::HttpRequest& request) {
  std::unique_ptr<net::test_server::BasicHttpResponse> http_response(
      new net::test_server::BasicHttpResponse());
  http_response->set_code(net::HTTP_OK);
  http_response->set_content_type("text/html");
  std::string request_path = request.GetURL().path();
  http_response->set_content(R"({
    "jsonrpc": "2.0",
    "id": 1,
    "result": "ns1aBL6AowxpiPzQL3ZeBK1RpCSLq1VfhqNw9KFSsytayARYdYrqrmbmhaizUTTkT4SXEnjnbVmPBrie3o9yuyB"
  })");
  return std::move(http_response);
}

// signMessage
constexpr char kMessage[] = "bravey baby!";
constexpr char kEncodedMessage[] = "98,114,97,118,121,32,98,97,98,121,33";
constexpr char kExpectedSignature[] =
    "98,100,65,130,165,105,247,254,176,58,137,184,149,50,202,4,239,34,179,15,"
    "99,184,125,255,9,227,4,118,70,108,153,191,78,251,150,104,239,24,191,139,"
    "242,54,150,144,96,249,42,106,199,171,222,72,108,190,206,193,130,47,125,"
    "239,173,127,238,11";
constexpr char kExpectedEncodedSignature[] =
    "5KVxa2RGmhE2Ldfctr42MQCrAQrT2NTFcoCD65KUNRYki6CPctUzsnR2xm4sPLzeQvSdCS6Gib"
    "4ScfYJUJQrNE4C";

class TestTxServiceObserver : public mojom::TxServiceObserver {
 public:
  TestTxServiceObserver() {}

  void OnNewUnapprovedTx(mojom::TransactionInfoPtr tx) override {
    run_loop_new_unapproved_->Quit();
  }

  void OnUnapprovedTxUpdated(mojom::TransactionInfoPtr tx) override {}

  void OnTransactionStatusChanged(mojom::TransactionInfoPtr tx) override {
    if (tx->tx_status == mojom::TransactionStatus::Rejected) {
      run_loop_rejected_->Quit();
    }
  }

  void WaitForNewUnapprovedTx() {
    run_loop_new_unapproved_ = std::make_unique<base::RunLoop>();
    run_loop_new_unapproved_->Run();
  }

  void WaitForRjectedStatus() {
    run_loop_rejected_ = std::make_unique<base::RunLoop>();
    run_loop_rejected_->Run();
  }

  mojo::PendingRemote<brave_wallet::mojom::TxServiceObserver> GetReceiver() {
    return observer_receiver_.BindNewPipeAndPassRemote();
  }

 private:
  mojo::Receiver<brave_wallet::mojom::TxServiceObserver> observer_receiver_{
      this};
  std::unique_ptr<base::RunLoop> run_loop_new_unapproved_;
  std::unique_ptr<base::RunLoop> run_loop_rejected_;
};

}  // namespace

class SolanaProviderTest : public InProcessBrowserTest {
 public:
  SolanaProviderTest()
      : https_server_for_files_(net::EmbeddedTestServer::TYPE_HTTPS),
        https_server_for_rpc_(net::EmbeddedTestServer::TYPE_HTTPS) {
    feature_list_.InitWithFeatures(
        {brave_wallet::features::kBraveWalletSolanaFeature,
         brave_wallet::features::kBraveWalletSolanaProviderFeature},
        {});
  }

  ~SolanaProviderTest() override = default;

  void SetUpOnMainThread() override {
    host_resolver()->AddRule("*", "127.0.0.1");

    https_server_for_files_.SetSSLConfig(
        net::EmbeddedTestServer::CERT_TEST_NAMES);
    brave::RegisterPathProvider();
    base::FilePath test_data_dir;
    base::PathService::Get(brave::DIR_TEST_DATA, &test_data_dir);
    test_data_dir = test_data_dir.AppendASCII("brave-wallet");
    https_server_for_files_.ServeFilesFromDirectory(test_data_dir);
    ASSERT_TRUE(https_server_for_files()->Start());

    brave_wallet_service_ =
        brave_wallet::BraveWalletServiceFactory::GetServiceForContext(
            browser()->profile());
    keyring_service_ =
        KeyringServiceFactory::GetServiceForContext(browser()->profile());
    json_rpc_service_ =
        JsonRpcServiceFactory::GetServiceForContext(browser()->profile());
    tx_service_ = TxServiceFactory::GetServiceForContext(browser()->profile());
    tx_service_->AddObserver(observer()->GetReceiver());

    StartRPCServer(base::BindRepeating(&HandleRequest));

    // setup _brave_solana
    if (g_provider_internal_script->empty()) {
      *g_provider_internal_script = brave_wallet::LoadDataResource(
          IDR_BRAVE_WALLET_SCRIPT_SOLANA_PROVIDER_INTERNAL_SCRIPT_BUNDLE_JS);
    }
    ASSERT_TRUE(ExecJs(web_contents(), *g_provider_internal_script));
  }

  void StartRPCServer(
      const net::EmbeddedTestServer::HandleRequestCallback& callback) {
    https_server_for_rpc()->SetSSLConfig(net::EmbeddedTestServer::CERT_OK);
    https_server_for_rpc()->RegisterRequestHandler(callback);
    ASSERT_TRUE(https_server_for_rpc()->Start());
    json_rpc_service_->SetCustomNetworkForTesting(
        mojom::kLocalhostChainId, mojom::CoinType::SOL,
        https_server_for_rpc()->base_url());
  }

  content::WebContents* web_contents() {
    return browser()->tab_strip_model()->GetActiveWebContents();
  }

  net::EmbeddedTestServer* https_server_for_files() {
    return &https_server_for_files_;
  }
  net::EmbeddedTestServer* https_server_for_rpc() {
    return &https_server_for_rpc_;
  }
  TestTxServiceObserver* observer() { return &observer_; }

  HostContentSettingsMap* host_content_settings_map() {
    return HostContentSettingsMapFactory::GetForProfile(browser()->profile());
  }

  void RestoreWallet() {
    const char mnemonic[] =
        "scare piece awesome elite long drift control cabbage glass dash coral "
        "angry";
    base::RunLoop run_loop;
    keyring_service_->RestoreWallet(
        mnemonic, "brave123", false,
        base::BindLambdaForTesting([&run_loop](bool success) {
          ASSERT_TRUE(success);
          run_loop.Quit();
        }));
    run_loop.Run();
  }

  void LockWallet() {
    keyring_service_->Lock();
    // Needed so KeyringServiceObserver::Locked handler can be hit
    // which the provider object listens to for the accountsChanged event.
    base::RunLoop().RunUntilIdle();
  }

  void AddAccount() {
    base::RunLoop run_loop;
    keyring_service_->AddAccount(
        "Account 1", mojom::CoinType::SOL,
        base::BindLambdaForTesting([&run_loop](bool success) {
          ASSERT_TRUE(success);
          run_loop.Quit();
        }));
    run_loop.Run();
  }

  void SetSelectedAccount(const std::string& address) {
    base::RunLoop run_loop;
    keyring_service_->SetSelectedAccount(
        address, mojom::CoinType::SOL,
        base::BindLambdaForTesting([&run_loop](bool success) {
          EXPECT_TRUE(success);
          run_loop.Quit();
        }));
    run_loop.Run();
  }

  void UserGrantPermission(bool granted) {
    if (granted)
      permissions::BraveWalletPermissionContext::AcceptOrCancel(
          std::vector<std::string>{kFirstAccount}, web_contents());
    else
      permissions::BraveWalletPermissionContext::Cancel(web_contents());
    ASSERT_EQ(EvalJs(web_contents(), "getConnectedAccount()",
                     content::EXECUTE_SCRIPT_USE_MANUAL_REPLY)
                  .ExtractString(),
              granted ? kFirstAccount : "");
  }

  std::vector<mojom::TransactionInfoPtr> GetAllTransactionInfo() {
    std::vector<mojom::TransactionInfoPtr> transaction_infos;
    base::RunLoop run_loop;
    tx_service_->GetAllTransactionInfo(
        mojom::CoinType::SOL, kFirstAccount,
        base::BindLambdaForTesting(
            [&](std::vector<mojom::TransactionInfoPtr> v) {
              transaction_infos = std::move(v);
              run_loop.Quit();
            }));
    run_loop.Run();
    return transaction_infos;
  }

  void ApproveTransaction(const std::string& tx_meta_id) {
    base::RunLoop run_loop;
    tx_service_->ApproveTransaction(
        mojom::CoinType::SOL, tx_meta_id,
        base::BindLambdaForTesting([&](bool success,
                                       mojom::ProviderErrorUnionPtr error_union,
                                       const std::string& error_message) {
          EXPECT_TRUE(success);
          ASSERT_TRUE(error_union->is_solana_provider_error());
          EXPECT_EQ(error_union->get_solana_provider_error(),
                    mojom::SolanaProviderError::kSuccess);
          EXPECT_TRUE(error_message.empty());
          run_loop.Quit();
        }));
    run_loop.Run();
  }

  void RejectTransaction(const std::string& tx_meta_id) {
    base::RunLoop run_loop;
    tx_service_->RejectTransaction(
        mojom::CoinType::SOL, tx_meta_id,
        base::BindLambdaForTesting([&](bool success) {
          EXPECT_TRUE(success);
          observer()->WaitForRjectedStatus();
          run_loop.Quit();
        }));
    run_loop.Run();
  }

  void CallSolanaConnect(bool is_expect_bubble = true) {
    ASSERT_TRUE(ExecJs(web_contents(), "solanaConnect()"));
    base::RunLoop().RunUntilIdle();
    if (is_expect_bubble) {
      ASSERT_TRUE(
          brave_wallet::BraveWalletTabHelper::FromWebContents(web_contents())
              ->IsShowingBubble());
    }
  }

  void CallSolanaDisconnect() {
    ASSERT_TRUE(EvalJs(web_contents(), "solanaDisconnect()",
                       content::EXECUTE_SCRIPT_USE_MANUAL_REPLY)
                    .ExtractBool());
  }

  void CallSolanaSignMessage(const std::string& message,
                             const std::string& encoding) {
    ASSERT_TRUE(ExecJs(web_contents(),
                       base::StringPrintf(R"(solanaSignMessage('%s', '%s'))",
                                          message.c_str(), encoding.c_str())));
  }

  void CallSolanaRequest(const std::string& json) {
    ASSERT_TRUE(
        ExecJs(web_contents(),
               base::StringPrintf(R"(solanaRequest(%s))", json.c_str())));
  }

  std::string GetSignMessageResult() {
    return EvalJs(web_contents(), "getSignMessageResult()",
                  content::EXECUTE_SCRIPT_USE_MANUAL_REPLY)
        .ExtractString();
  }

  void CallSolanaSignAndSendTransaction(
      const std::string& unsigned_tx_array_string,
      const std::string& send_options_string = "{}") {
    const std::string script = base::StringPrintf(
        R"(solanaSignAndSendTransaction(new Uint8Array([%s]), %s))",
        unsigned_tx_array_string.c_str(), send_options_string.c_str());
    ASSERT_TRUE(ExecJs(web_contents(), script));
  }

  std::string GetSignAndSendTransactionResult() {
    return EvalJs(web_contents(), "getSignAndSendTransactionResult()",
                  content::EXECUTE_SCRIPT_USE_MANUAL_REPLY)
        .ExtractString();
  }

  void CallSolanaSignTransaction(const std::string& unsigned_tx_array_string) {
    const std::string script =
        base::StringPrintf(R"(solanaSignTransaction(new Uint8Array([%s])))",
                           unsigned_tx_array_string.c_str());
    ASSERT_TRUE(ExecJs(web_contents(), script));
  }

  void CallSolanaSignAllTransactions(const std::string& unsigned_tx_array_str,
                                     const std::string& signed_tx_array_str) {
    const std::string script = base::StringPrintf(
        R"(solanaSignAllTransactions(new Uint8Array([%s]), new Uint8Array([%s])))",
        unsigned_tx_array_str.c_str(), signed_tx_array_str.c_str());
    ASSERT_TRUE(ExecJs(web_contents(), script));
  }

  std::string GetSignTransactionResult() {
    return EvalJs(web_contents(), "getSignTransactionResult()",
                  content::EXECUTE_SCRIPT_USE_MANUAL_REPLY)
        .ExtractString();
  }

  std::string GetSignAllTransactionsResult() {
    return EvalJs(web_contents(), "getSignAllTransactionsResult()",
                  content::EXECUTE_SCRIPT_USE_MANUAL_REPLY)
        .ExtractString();
  }

  std::string GetRequestResult() {
    return EvalJs(web_contents(), "getRequestResult()",
                  content::EXECUTE_SCRIPT_USE_MANUAL_REPLY)
        .ExtractString();
  }

  bool IsSolanaConnected() {
    return EvalJs(web_contents(), "isSolanaConnected()",
                  content::EXECUTE_SCRIPT_USE_MANUAL_REPLY)
        .ExtractBool();
  }

  void WaitForResultReady() {
    content::DOMMessageQueue message_queue;
    std::string message;
    EXPECT_TRUE(message_queue.WaitForMessage(&message));
    EXPECT_EQ("\"result ready\"", message);
  }

 protected:
  raw_ptr<BraveWalletService> brave_wallet_service_ = nullptr;
  raw_ptr<KeyringService> keyring_service_ = nullptr;

 private:
  TestTxServiceObserver observer_;
  base::test::ScopedFeatureList feature_list_;
  net::test_server::EmbeddedTestServer https_server_for_files_;
  net::test_server::EmbeddedTestServer https_server_for_rpc_;
  raw_ptr<TxService> tx_service_ = nullptr;
  raw_ptr<JsonRpcService> json_rpc_service_ = nullptr;
};

IN_PROC_BROWSER_TEST_F(SolanaProviderTest, ConnectedStatusAndPermission) {
  RestoreWallet();
  AddAccount();
  SetSelectedAccount(kFirstAccount);
  GURL url =
      https_server_for_files()->GetURL("a.test", "/solana_provider.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));

  ASSERT_FALSE(IsSolanaConnected());
  CallSolanaConnect();
  UserGrantPermission(true);
  EXPECT_TRUE(IsSolanaConnected());

  // Removing solana permission doesn't affect connected status.
  host_content_settings_map()->ClearSettingsForOneType(
      ContentSettingsType::BRAVE_SOLANA);
  EXPECT_TRUE(IsSolanaConnected());

  // Doing connect again and reject it doesn't affect connected status either.
  CallSolanaConnect();
  UserGrantPermission(false);
  EXPECT_TRUE(IsSolanaConnected());

  // Only disconnect will set connected status to false.
  CallSolanaDisconnect();
  EXPECT_FALSE(IsSolanaConnected());
}

IN_PROC_BROWSER_TEST_F(SolanaProviderTest, ConnectedStatusInMultiFrames) {
  RestoreWallet();
  AddAccount();
  SetSelectedAccount(kFirstAccount);
  GURL url =
      https_server_for_files()->GetURL("a.test", "/solana_provider.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));

  ASSERT_FALSE(IsSolanaConnected());
  CallSolanaConnect();
  UserGrantPermission(true);
  // First tab is now connected.
  EXPECT_TRUE(IsSolanaConnected());
  // Add same url at second tab
  ASSERT_TRUE(AddTabAtIndex(1, url, ui::PAGE_TRANSITION_TYPED));
  ASSERT_EQ(browser()->tab_strip_model()->active_index(), 1);
  // Connected status of second tab is separate from first tab.
  EXPECT_FALSE(IsSolanaConnected());
  // Doing successful connect and disconnect shouldn't affect first tab.
  // Since a.test already has the permission so connect would success without
  // asking.
  CallSolanaConnect(false);
  EXPECT_TRUE(IsSolanaConnected());
  CallSolanaDisconnect();
  EXPECT_FALSE(IsSolanaConnected());

  // Swtich back to first tab and it should still be connected,
  browser()->tab_strip_model()->ActivateTabAt(0);
  ASSERT_EQ(browser()->tab_strip_model()->active_index(), 0);
  EXPECT_TRUE(IsSolanaConnected());
}

IN_PROC_BROWSER_TEST_F(SolanaProviderTest, SignMessage) {
  RestoreWallet();
  AddAccount();
  SetSelectedAccount(kFirstAccount);
  GURL url =
      https_server_for_files()->GetURL("a.test", "/solana_provider.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));

  CallSolanaConnect();
  UserGrantPermission(true);
  ASSERT_TRUE(IsSolanaConnected());

  size_t request_index = 0;
  CallSolanaSignMessage(kMessage, "utf8");
  EXPECT_TRUE(
      brave_wallet::BraveWalletTabHelper::FromWebContents(web_contents())
          ->IsShowingBubble());
  // user rejected request
  brave_wallet_service_->NotifySignMessageRequestProcessed(false,
                                                           request_index++);
  WaitForResultReady();
  EXPECT_EQ(GetSignMessageResult(),
            l10n_util::GetStringUTF8(IDS_WALLET_USER_REJECTED_REQUEST));

  for (const std::string& encoding : {"utf8", "hex", "invalid", ""}) {
    CallSolanaSignMessage(kMessage, encoding);
    EXPECT_TRUE(
        brave_wallet::BraveWalletTabHelper::FromWebContents(web_contents())
            ->IsShowingBubble());
    // user approved request
    brave_wallet_service_->NotifySignMessageRequestProcessed(true,
                                                             request_index++);
    WaitForResultReady();
    EXPECT_EQ(GetSignMessageResult(), kExpectedSignature);
  }
}

IN_PROC_BROWSER_TEST_F(SolanaProviderTest, GetPublicKey) {
  RestoreWallet();
  AddAccount();
  SetSelectedAccount(kFirstAccount);
  GURL url =
      https_server_for_files()->GetURL("a.test", "/solana_provider.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));

  constexpr char get_public_key_script[] =
      "window.domAutomationController.send(window.solana."
      "publicKey ? window.solana.publicKey.toString() : '')";

  // Will get null in disconnected state
  EXPECT_EQ(EvalJs(web_contents(), get_public_key_script,
                   content::EXECUTE_SCRIPT_USE_MANUAL_REPLY)
                .ExtractString(),
            "");

  CallSolanaConnect();
  UserGrantPermission(true);
  ASSERT_TRUE(IsSolanaConnected());

  EXPECT_EQ(EvalJs(web_contents(), get_public_key_script,
                   content::EXECUTE_SCRIPT_USE_MANUAL_REPLY)
                .ExtractString(),
            kFirstAccount);

  LockWallet();
  // Publickey is still accessible when wallet is locked
  EXPECT_EQ(EvalJs(web_contents(), get_public_key_script,
                   content::EXECUTE_SCRIPT_USE_MANUAL_REPLY)
                .ExtractString(),
            kFirstAccount);
}

IN_PROC_BROWSER_TEST_F(SolanaProviderTest, SignAndSendTransaction) {
  RestoreWallet();
  AddAccount();
  SetSelectedAccount(kFirstAccount);
  GURL url =
      https_server_for_files()->GetURL("a.test", "/solana_provider.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));

  CallSolanaConnect();
  UserGrantPermission(true);
  ASSERT_TRUE(IsSolanaConnected());

  CallSolanaSignAndSendTransaction(kUnsignedTxArrayStr);
  observer()->WaitForNewUnapprovedTx();
  EXPECT_TRUE(
      brave_wallet::BraveWalletTabHelper::FromWebContents(web_contents())
          ->IsShowingBubble());

  auto infos = GetAllTransactionInfo();
  EXPECT_EQ(1UL, infos.size());
  EXPECT_EQ(kFirstAccount, infos[0]->from_address);
  EXPECT_EQ(mojom::TransactionStatus::Unapproved, infos[0]->tx_status);
  EXPECT_EQ(mojom::TransactionType::SolanaDappSignAndSendTransaction,
            infos[0]->tx_type);
  EXPECT_EQ(MakeOriginInfo(https_server_for_files()->GetOrigin("a.test")),
            infos[0]->origin_info);
  const std::string tx1_id = infos[0]->id;
  RejectTransaction(tx1_id);

  infos = GetAllTransactionInfo();
  EXPECT_EQ(1UL, infos.size());
  EXPECT_EQ(kFirstAccount, infos[0]->from_address);
  EXPECT_EQ(mojom::TransactionStatus::Rejected, infos[0]->tx_status);
  EXPECT_EQ(mojom::TransactionType::SolanaDappSignAndSendTransaction,
            infos[0]->tx_type);
  EXPECT_TRUE(infos[0]->tx_hash.empty());

  WaitForResultReady();
  EXPECT_EQ(GetSignAndSendTransactionResult(),
            l10n_util::GetStringUTF8(IDS_WALLET_USER_REJECTED_REQUEST));

  const std::string send_options =
      R"({"maxRetries":1,"preflightCommitment":"confirmed","skipPreflight":true})";
  CallSolanaSignAndSendTransaction(kUnsignedTxArrayStr, send_options);
  observer()->WaitForNewUnapprovedTx();
  EXPECT_TRUE(
      brave_wallet::BraveWalletTabHelper::FromWebContents(web_contents())
          ->IsShowingBubble());

  infos = GetAllTransactionInfo();
  EXPECT_EQ(2UL, infos.size());
  size_t tx2_index = 0;
  for (size_t i = 0; i < infos.size(); i++) {
    if (infos[i]->id != tx1_id) {
      tx2_index = i;
      break;
    }
  }
  EXPECT_EQ(kFirstAccount, infos[tx2_index]->from_address);
  EXPECT_EQ(mojom::TransactionStatus::Unapproved, infos[tx2_index]->tx_status);
  EXPECT_EQ(mojom::TransactionType::SolanaDappSignAndSendTransaction,
            infos[tx2_index]->tx_type);
  EXPECT_EQ(MakeOriginInfo(https_server_for_files()->GetOrigin("a.test")),
            infos[tx2_index]->origin_info);

  ApproveTransaction(infos[tx2_index]->id);

  infos = GetAllTransactionInfo();
  EXPECT_EQ(2UL, infos.size());
  EXPECT_EQ(kFirstAccount, infos[tx2_index]->from_address);
  EXPECT_EQ(mojom::TransactionStatus::Submitted, infos[tx2_index]->tx_status);
  EXPECT_EQ(mojom::TransactionType::SolanaDappSignAndSendTransaction,
            infos[tx2_index]->tx_type);
  EXPECT_EQ(infos[tx2_index]->tx_hash, kEncodedSignature);
  ASSERT_TRUE(infos[tx2_index]->tx_data_union->is_solana_tx_data());
  EXPECT_EQ(infos[tx2_index]->tx_data_union->get_solana_tx_data()->send_options,
            mojom::SolanaSendTransactionOptions::New(
                mojom::OptionalMaxRetries::New(1), "confirmed",
                mojom::OptionalSkipPreflight::New(true)));

  WaitForResultReady();
  EXPECT_EQ(GetSignAndSendTransactionResult(), kEncodedSignature);
}

IN_PROC_BROWSER_TEST_F(SolanaProviderTest, SignTransaction) {
  RestoreWallet();
  AddAccount();
  SetSelectedAccount(kFirstAccount);
  GURL url =
      https_server_for_files()->GetURL("a.test", "/solana_provider.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));

  CallSolanaConnect();
  UserGrantPermission(true);
  ASSERT_TRUE(IsSolanaConnected());

  size_t request_index = 0;
  CallSolanaSignTransaction(kUnsignedTxArrayStr);

  EXPECT_TRUE(
      brave_wallet::BraveWalletTabHelper::FromWebContents(web_contents())
          ->IsShowingBubble());
  // user rejected request
  brave_wallet_service_->NotifySignTransactionRequestProcessed(false,
                                                               request_index++);
  WaitForResultReady();
  EXPECT_EQ(GetSignTransactionResult(),
            l10n_util::GetStringUTF8(IDS_WALLET_USER_REJECTED_REQUEST));

  CallSolanaSignTransaction(kUnsignedTxArrayStr);
  EXPECT_TRUE(
      brave_wallet::BraveWalletTabHelper::FromWebContents(web_contents())
          ->IsShowingBubble());
  // user approved request
  brave_wallet_service_->NotifySignTransactionRequestProcessed(true,
                                                               request_index++);
  WaitForResultReady();
  EXPECT_EQ(GetSignTransactionResult(), kSignedTxArrayStr);
}

IN_PROC_BROWSER_TEST_F(SolanaProviderTest, SignAllTransactions) {
  RestoreWallet();
  AddAccount();
  SetSelectedAccount(kFirstAccount);
  GURL url =
      https_server_for_files()->GetURL("a.test", "/solana_provider.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));

  CallSolanaConnect();
  UserGrantPermission(true);
  ASSERT_TRUE(IsSolanaConnected());

  size_t request_index = 0;
  CallSolanaSignAllTransactions(kUnsignedTxArrayStr, kSignedTxArrayStr);

  EXPECT_TRUE(
      brave_wallet::BraveWalletTabHelper::FromWebContents(web_contents())
          ->IsShowingBubble());
  // user rejected request
  brave_wallet_service_->NotifySignAllTransactionsRequestProcessed(
      false, request_index++);
  WaitForResultReady();
  EXPECT_EQ(GetSignAllTransactionsResult(),
            l10n_util::GetStringUTF8(IDS_WALLET_USER_REJECTED_REQUEST));

  CallSolanaSignAllTransactions(kUnsignedTxArrayStr, kSignedTxArrayStr);
  EXPECT_TRUE(
      brave_wallet::BraveWalletTabHelper::FromWebContents(web_contents())
          ->IsShowingBubble());
  // user approved request
  brave_wallet_service_->NotifySignAllTransactionsRequestProcessed(
      true, request_index++);
  WaitForResultReady();
  EXPECT_EQ(GetSignAllTransactionsResult(), "success");
}

IN_PROC_BROWSER_TEST_F(SolanaProviderTest, Request) {
  RestoreWallet();
  AddAccount();
  SetSelectedAccount(kFirstAccount);
  GURL url =
      https_server_for_files()->GetURL("a.test", "/solana_provider.html");
  ASSERT_TRUE(ui_test_utils::NavigateToURL(browser(), url));

  // connect and accept
  CallSolanaRequest(R"({method: "connect"})");
  base::RunLoop().RunUntilIdle();
  EXPECT_TRUE(
      brave_wallet::BraveWalletTabHelper::FromWebContents(web_contents())
          ->IsShowingBubble());
  permissions::BraveWalletPermissionContext::AcceptOrCancel(
      std::vector<std::string>{kFirstAccount}, web_contents());
  WaitForResultReady();
  EXPECT_EQ(GetRequestResult(), kFirstAccount);
  ASSERT_TRUE(IsSolanaConnected());

  // disconnect
  CallSolanaRequest(R"({method: "disconnect"})");
  WaitForResultReady();
  EXPECT_EQ(GetRequestResult(), "success");
  ASSERT_FALSE(IsSolanaConnected());

  // eagerly connect
  CallSolanaRequest(R"({method: "connect", params: { onlyIfTrusted: true }})");
  EXPECT_FALSE(
      brave_wallet::BraveWalletTabHelper::FromWebContents(web_contents())
          ->IsShowingBubble());
  WaitForResultReady();
  EXPECT_EQ(GetRequestResult(), kFirstAccount);
  ASSERT_TRUE(IsSolanaConnected());

  // signMessage
  CallSolanaRequest(base::StringPrintf(R"(
    {method: "signMessage", params: { message: new Uint8Array([%s]) }})",
                                       kEncodedMessage));
  EXPECT_TRUE(
      brave_wallet::BraveWalletTabHelper::FromWebContents(web_contents())
          ->IsShowingBubble());
  brave_wallet_service_->NotifySignMessageRequestProcessed(true, 0);
  WaitForResultReady();
  EXPECT_EQ(GetRequestResult(), kExpectedEncodedSignature);

  // signTransaction
  CallSolanaRequest(base::StringPrintf(R"(
    {method: "signTransaction", params: { message: '%s' }})",
                                       kEncodedUnsignedTxArrayStr));
  EXPECT_TRUE(
      brave_wallet::BraveWalletTabHelper::FromWebContents(web_contents())
          ->IsShowingBubble());
  brave_wallet_service_->NotifySignTransactionRequestProcessed(true, 0);
  WaitForResultReady();
  EXPECT_EQ(GetRequestResult(), kEncodedSignature);

  // signAndSendTransaction
  const std::string send_options =
      R"({"maxRetries":1,"preflightCommitment":"confirmed","skipPreflight":true})";
  CallSolanaRequest(base::StringPrintf(R"(
    {method: "signAndSendTransaction", params: { message: '%s', options: %s }})",
                                       kEncodedUnsignedTxArrayStr,
                                       send_options.c_str()));
  observer()->WaitForNewUnapprovedTx();
  EXPECT_TRUE(
      brave_wallet::BraveWalletTabHelper::FromWebContents(web_contents())
          ->IsShowingBubble());
  auto infos = GetAllTransactionInfo();
  ASSERT_EQ(infos.size(), 1u);
  ASSERT_TRUE(infos[0]->tx_data_union->is_solana_tx_data());
  EXPECT_EQ(infos[0]->tx_data_union->get_solana_tx_data()->send_options,
            mojom::SolanaSendTransactionOptions::New(
                mojom::OptionalMaxRetries::New(1), "confirmed",
                mojom::OptionalSkipPreflight::New(true)));
  ApproveTransaction(infos[0]->id);
  WaitForResultReady();
  EXPECT_EQ(GetRequestResult(), kEncodedSignature);

  // signAllTransactions
  CallSolanaRequest(base::StringPrintf(R"(
    {method: "signAllTransactions", params: { message: ['%s', '%s'] }})",
                                       kEncodedUnsignedTxArrayStr,
                                       kEncodedUnsignedTxArrayStr));
  EXPECT_TRUE(
      brave_wallet::BraveWalletTabHelper::FromWebContents(web_contents())
          ->IsShowingBubble());
  brave_wallet_service_->NotifySignAllTransactionsRequestProcessed(true, 0);
  WaitForResultReady();
  EXPECT_EQ(GetRequestResult(),
            base::StrCat({kEncodedSignature, ",", kEncodedSignature}));
}

}  // namespace brave_wallet
