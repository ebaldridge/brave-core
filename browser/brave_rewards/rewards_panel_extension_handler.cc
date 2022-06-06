/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/brave_rewards/rewards_panel_extension_handler.h"

#include <memory>
#include <string>

#include "base/strings/strcat.h"
#include "brave/browser/brave_rewards/rewards_service_factory.h"
#include "brave/browser/extensions/brave_component_loader.h"
#include "brave/browser/ui/views/brave_actions/brave_actions_container.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_window.h"
#include "extensions/browser/extension_system.h"

namespace brave_rewards {

namespace {

constexpr char kRewardsPanelUrl[] = "/brave_rewards_panel.html";
constexpr char kAdsEnableRelativeUrl[] = "/request_ads_enabled_panel.html";

std::string GetExtensionPath(const mojom::RewardsPanelArgs& args) {
  switch (args.view) {
    case mojom::RewardsPanelView::kDefault:
      return kRewardsPanelUrl;
    case mojom::RewardsPanelView::kRewardsTour:
      return base::StrCat({kRewardsPanelUrl, "#tour"});
    case mojom::RewardsPanelView::kGrantCaptcha:
      return base::StrCat({kRewardsPanelUrl, "#grant_", args.data});
    case mojom::RewardsPanelView::kAdaptiveCaptcha:
      return base::StrCat({kRewardsPanelUrl, "#load_adaptive_captcha"});
    case mojom::RewardsPanelView::kBraveTalkOptIn:
      return kAdsEnableRelativeUrl;
  }
}

}  // namespace

RewardsPanelExtensionHandler::RewardsPanelExtensionHandler(Browser* browser)
    : browser_(browser) {}

RewardsPanelExtensionHandler::~RewardsPanelExtensionHandler() = default;

bool RewardsPanelExtensionHandler::IsRewardsExtensionPanelURL(const GURL& url) {
  return url.SchemeIs("chrome-extension") &&
         url.host() == brave_rewards_extension_id &&
         (url.path() == kRewardsPanelUrl ||
          url.path() == kAdsEnableRelativeUrl);
}

void RewardsPanelExtensionHandler::OnRewardsPanelRequested(
    const mojom::RewardsPanelArgs& args) {
  DCHECK(browser_);
  auto* profile = browser_->profile();

  // Start the rewards ledger process if it is not already started
  auto* rewards_service = RewardsServiceFactory::GetForProfile(profile);
  if (!rewards_service) {
    return;
  }

  rewards_service->StartProcess(base::DoNothing());

  // Load the rewards extension if it is not already loaded.
  auto* extension_service =
      extensions::ExtensionSystem::Get(profile)->extension_service();
  if (!extension_service) {
    return;
  }

  static_cast<extensions::BraveComponentLoader*>(
      extension_service->component_loader())
      ->AddRewardsExtension();

  std::string error;
  extensions::BraveActionAPI::ShowActionUI(
      browser_, brave_rewards_extension_id,
      std::make_unique<std::string>(GetExtensionPath(args)), &error);
}

}  // namespace brave_rewards
