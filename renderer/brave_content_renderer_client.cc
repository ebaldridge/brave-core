/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/renderer/brave_content_renderer_client.h"

#include "base/feature_list.h"
#include "brave/components/brave_ads/common/features.h"
#include "brave/components/brave_ads/renderer/brave_ads_render_frame_observer.h"
#include "brave/components/brave_search/common/brave_search_utils.h"
#include "brave/components/brave_search/renderer/brave_search_render_frame_observer.h"
#include "brave/components/brave_shields/common/features.h"
#include "brave/components/brave_vpn/features.h"
#include "brave/components/brave_vpn/renderer/vpn_render_frame_observer.h"
#include "brave/components/brave_wallet/common/features.h"
#include "brave/components/cosmetic_filters/renderer/cosmetic_filters_js_render_frame_observer.h"
#include "brave/components/skus/common/features.h"
#include "brave/components/skus/renderer/skus_render_frame_observer.h"
#include "brave/renderer/brave_render_thread_observer.h"
#include "brave/renderer/brave_wallet/brave_wallet_render_frame_observer.h"
#include "chrome/common/chrome_isolated_world_ids.h"
#include "content/public/renderer/render_thread.h"
#include "third_party/blink/public/common/features.h"
#include "third_party/blink/public/platform/web_runtime_features.h"
#include "third_party/blink/public/web/modules/service_worker/web_service_worker_context_proxy.h"
#include "url/gurl.h"

BraveContentRendererClient::BraveContentRendererClient()
    : ChromeContentRendererClient() {}

void BraveContentRendererClient::
    SetRuntimeFeaturesDefaultsBeforeBlinkInitialization() {
  ChromeContentRendererClient::
      SetRuntimeFeaturesDefaultsBeforeBlinkInitialization();

  blink::WebRuntimeFeatures::EnableWebNfc(false);

  // These features don't have dedicated WebRuntimeFeatures wrappers.
  blink::WebRuntimeFeatures::EnableFeatureFromString("DigitalGoods", false);
  if (!base::FeatureList::IsEnabled(blink::features::kFileSystemAccessAPI)) {
    blink::WebRuntimeFeatures::EnableFeatureFromString("FileSystemAccess",
                                                       false);
    blink::WebRuntimeFeatures::EnableFeatureFromString(
        "FileSystemAccessAPIExperimental", false);
  }
  blink::WebRuntimeFeatures::EnableFeatureFromString("Serial", false);
  blink::WebRuntimeFeatures::EnableFeatureFromString(
      "SpeculationRulesPrefetchProxy", false);
}

BraveContentRendererClient::~BraveContentRendererClient() = default;

void BraveContentRendererClient::RenderThreadStarted() {
  ChromeContentRendererClient::RenderThreadStarted();

  brave_observer_ = std::make_unique<BraveRenderThreadObserver>();
  content::RenderThread::Get()->AddObserver(brave_observer_.get());
  brave_search_service_worker_holder_.SetBrowserInterfaceBrokerProxy(
      browser_interface_broker_.get());
}

void BraveContentRendererClient::RenderFrameCreated(
    content::RenderFrame* render_frame) {
  ChromeContentRendererClient::RenderFrameCreated(render_frame);

  if (base::FeatureList::IsEnabled(
          brave_shields::features::kBraveAdblockCosmeticFiltering)) {
    auto dynamic_params_closure = base::BindRepeating([]() {
      auto dynamic_params = BraveRenderThreadObserver::GetDynamicParams();
      return dynamic_params.de_amp_enabled;
    });

    new cosmetic_filters::CosmeticFiltersJsRenderFrameObserver(
        render_frame, ISOLATED_WORLD_ID_BRAVE_INTERNAL, dynamic_params_closure);
  }

  if (base::FeatureList::IsEnabled(
          brave_wallet::features::kNativeBraveWalletFeature)) {
    new brave_wallet::BraveWalletRenderFrameObserver(
        render_frame,
        base::BindRepeating(&BraveRenderThreadObserver::GetDynamicParams));
  }

  if (brave_search::IsDefaultAPIEnabled()) {
    new brave_search::BraveSearchRenderFrameObserver(
        render_frame, content::ISOLATED_WORLD_ID_GLOBAL);
  }

  if (brave_ads::features::IsRequestAdsEnabledApiEnabled()) {
    new brave_ads::BraveAdsRenderFrameObserver(
        render_frame, content::ISOLATED_WORLD_ID_GLOBAL);
  }

  if (base::FeatureList::IsEnabled(skus::features::kSkusFeature)) {
    new skus::SkusRenderFrameObserver(render_frame,
                                      content::ISOLATED_WORLD_ID_GLOBAL);
  }

  // TODO(bsclifton): also ensure user is Android
  if (base::FeatureList::IsEnabled(brave_vpn::features::kBraveVPN)) {
    new brave_vpn::VpnRenderFrameObserver(render_frame,
                                          content::ISOLATED_WORLD_ID_GLOBAL);
  }
}

void BraveContentRendererClient::RunScriptsAtDocumentStart(
    content::RenderFrame* render_frame) {
  auto* observer =
      cosmetic_filters::CosmeticFiltersJsRenderFrameObserver::Get(render_frame);
  // Run this before any extensions
  if (observer)
    observer->RunScriptsAtDocumentStart();

  ChromeContentRendererClient::RunScriptsAtDocumentStart(render_frame);
}

void BraveContentRendererClient::WillEvaluateServiceWorkerOnWorkerThread(
    blink::WebServiceWorkerContextProxy* context_proxy,
    v8::Local<v8::Context> v8_context,
    int64_t service_worker_version_id,
    const GURL& service_worker_scope,
    const GURL& script_url) {
  brave_search_service_worker_holder_.WillEvaluateServiceWorkerOnWorkerThread(
      context_proxy, v8_context, service_worker_version_id,
      service_worker_scope, script_url);
  ChromeContentRendererClient::WillEvaluateServiceWorkerOnWorkerThread(
      context_proxy, v8_context, service_worker_version_id,
      service_worker_scope, script_url);
}

void BraveContentRendererClient::WillDestroyServiceWorkerContextOnWorkerThread(
    v8::Local<v8::Context> v8_context,
    int64_t service_worker_version_id,
    const GURL& service_worker_scope,
    const GURL& script_url) {
  brave_search_service_worker_holder_
      .WillDestroyServiceWorkerContextOnWorkerThread(
          v8_context, service_worker_version_id, service_worker_scope,
          script_url);
  ChromeContentRendererClient::WillDestroyServiceWorkerContextOnWorkerThread(
      v8_context, service_worker_version_id, service_worker_scope, script_url);
}
