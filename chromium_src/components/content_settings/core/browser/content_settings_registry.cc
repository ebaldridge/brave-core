/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at https://mozilla.org/MPL/2.0/. */

#define BRAVE_INIT BraveInit();
#include "src/components/content_settings/core/browser/content_settings_registry.cc"
#undef BRAVE_INIT

#include "brave/components/brave_shields/common/brave_shield_constants.h"

namespace content_settings {

void ContentSettingsRegistry::BraveInit() {
  Register(ContentSettingsType::BRAVE_ADS, brave_shields::kAds,
           CONTENT_SETTING_BLOCK, WebsiteSettingsInfo::SYNCABLE,
           AllowlistedSchemes(),
           ValidSettings(CONTENT_SETTING_ALLOW, CONTENT_SETTING_BLOCK),
           WebsiteSettingsInfo::SINGLE_ORIGIN_ONLY_SCOPE,
           WebsiteSettingsRegistry::DESKTOP |
               WebsiteSettingsRegistry::PLATFORM_ANDROID,
           ContentSettingsInfo::INHERIT_IN_INCOGNITO,
           ContentSettingsInfo::PERSISTENT,
           ContentSettingsInfo::EXCEPTIONS_ON_SECURE_AND_INSECURE_ORIGINS);

  Register(ContentSettingsType::BRAVE_TRACKERS, brave_shields::kTrackers,
           CONTENT_SETTING_BLOCK, WebsiteSettingsInfo::SYNCABLE,
           AllowlistedSchemes(),
           ValidSettings(CONTENT_SETTING_ALLOW, CONTENT_SETTING_BLOCK),
           WebsiteSettingsInfo::SINGLE_ORIGIN_ONLY_SCOPE,
           WebsiteSettingsRegistry::DESKTOP |
               WebsiteSettingsRegistry::PLATFORM_ANDROID,
           ContentSettingsInfo::INHERIT_IN_INCOGNITO,
           ContentSettingsInfo::PERSISTENT,
           ContentSettingsInfo::EXCEPTIONS_ON_SECURE_AND_INSECURE_ORIGINS);

  Register(ContentSettingsType::BRAVE_HTTP_UPGRADABLE_RESOURCES,
           brave_shields::kHTTPUpgradableResources, CONTENT_SETTING_BLOCK,
           WebsiteSettingsInfo::SYNCABLE, AllowlistedSchemes(),
           ValidSettings(CONTENT_SETTING_ALLOW, CONTENT_SETTING_BLOCK),
           WebsiteSettingsInfo::SINGLE_ORIGIN_ONLY_SCOPE,
           WebsiteSettingsRegistry::DESKTOP |
               WebsiteSettingsRegistry::PLATFORM_ANDROID,
           ContentSettingsInfo::INHERIT_IN_INCOGNITO,
           ContentSettingsInfo::PERSISTENT,
           ContentSettingsInfo::EXCEPTIONS_ON_SECURE_AND_INSECURE_ORIGINS);

  Register(ContentSettingsType::BRAVE_REFERRERS, brave_shields::kReferrers,
           CONTENT_SETTING_BLOCK, WebsiteSettingsInfo::SYNCABLE,
           AllowlistedSchemes(),
           ValidSettings(CONTENT_SETTING_ALLOW, CONTENT_SETTING_BLOCK),
           WebsiteSettingsInfo::SINGLE_ORIGIN_ONLY_SCOPE,
           WebsiteSettingsRegistry::DESKTOP |
               WebsiteSettingsRegistry::PLATFORM_ANDROID,
           ContentSettingsInfo::INHERIT_IN_INCOGNITO,
           ContentSettingsInfo::PERSISTENT,
           ContentSettingsInfo::EXCEPTIONS_ON_SECURE_AND_INSECURE_ORIGINS);

  Register(ContentSettingsType::BRAVE_COOKIES, brave_shields::kCookies,
           CONTENT_SETTING_DEFAULT, WebsiteSettingsInfo::SYNCABLE,
           AllowlistedSchemes(kChromeUIScheme, kChromeDevToolsScheme),
           ValidSettings(CONTENT_SETTING_ALLOW, CONTENT_SETTING_BLOCK),
           WebsiteSettingsInfo::COOKIES_SCOPE,
           WebsiteSettingsRegistry::DESKTOP |
               WebsiteSettingsRegistry::PLATFORM_ANDROID,
           ContentSettingsInfo::INHERIT_IN_INCOGNITO,
           ContentSettingsInfo::PERSISTENT,
           ContentSettingsInfo::EXCEPTIONS_ON_SECURE_AND_INSECURE_ORIGINS);

  Register(ContentSettingsType::BRAVE_COSMETIC_FILTERING,
           brave_shields::kCosmeticFiltering, CONTENT_SETTING_DEFAULT,
           WebsiteSettingsInfo::SYNCABLE, AllowlistedSchemes(),
           ValidSettings(CONTENT_SETTING_ALLOW, CONTENT_SETTING_BLOCK),
           WebsiteSettingsInfo::COOKIES_SCOPE,
           WebsiteSettingsRegistry::DESKTOP |
               WebsiteSettingsRegistry::PLATFORM_ANDROID,
           ContentSettingsInfo::INHERIT_IN_INCOGNITO,
           ContentSettingsInfo::PERSISTENT,
           ContentSettingsInfo::EXCEPTIONS_ON_SECURE_AND_INSECURE_ORIGINS);

  Register(ContentSettingsType::BRAVE_FINGERPRINTING_V2,
           brave_shields::kFingerprintingV2, CONTENT_SETTING_DEFAULT,
           WebsiteSettingsInfo::SYNCABLE, AllowlistedSchemes(),
           ValidSettings(CONTENT_SETTING_ALLOW, CONTENT_SETTING_BLOCK),
           WebsiteSettingsInfo::COOKIES_SCOPE,
           WebsiteSettingsRegistry::DESKTOP |
               WebsiteSettingsRegistry::PLATFORM_ANDROID,
           ContentSettingsInfo::INHERIT_IN_INCOGNITO,
           ContentSettingsInfo::PERSISTENT,
           ContentSettingsInfo::EXCEPTIONS_ON_SECURE_AND_INSECURE_ORIGINS);

  Register(ContentSettingsType::BRAVE_SHIELDS, brave_shields::kBraveShields,
           CONTENT_SETTING_ALLOW, WebsiteSettingsInfo::SYNCABLE,
           AllowlistedSchemes(),
           ValidSettings(CONTENT_SETTING_ALLOW, CONTENT_SETTING_BLOCK),
           WebsiteSettingsInfo::SINGLE_ORIGIN_ONLY_SCOPE,
           WebsiteSettingsRegistry::DESKTOP |
               WebsiteSettingsRegistry::PLATFORM_ANDROID,
           ContentSettingsInfo::INHERIT_IN_INCOGNITO,
           ContentSettingsInfo::PERSISTENT,
           ContentSettingsInfo::EXCEPTIONS_ON_SECURE_AND_INSECURE_ORIGINS);

  Register(ContentSettingsType::BRAVE_SPEEDREADER, "braveSpeedreader",
           CONTENT_SETTING_ALLOW, WebsiteSettingsInfo::SYNCABLE,
           AllowlistedSchemes(),
           ValidSettings(CONTENT_SETTING_ALLOW, CONTENT_SETTING_BLOCK),
           WebsiteSettingsInfo::SINGLE_ORIGIN_ONLY_SCOPE,
           WebsiteSettingsRegistry::DESKTOP |
               WebsiteSettingsRegistry::PLATFORM_ANDROID,
           ContentSettingsInfo::INHERIT_IN_INCOGNITO,
           ContentSettingsInfo::PERSISTENT,
           ContentSettingsInfo::EXCEPTIONS_ON_SECURE_AND_INSECURE_ORIGINS);

  // Add CONTENT_SETTING_ASK for autoplay
  // Note ASK has been deprecated, only keeping it for
  // DiscardObsoleteAutoplayAsk test case
  content_settings_info_.erase(ContentSettingsType::AUTOPLAY);
  website_settings_registry_->UnRegister(ContentSettingsType::AUTOPLAY);
  Register(ContentSettingsType::AUTOPLAY, "autoplay", CONTENT_SETTING_ALLOW,
           WebsiteSettingsInfo::UNSYNCABLE, AllowlistedSchemes(),
           ValidSettings(CONTENT_SETTING_ALLOW, CONTENT_SETTING_BLOCK,
                         CONTENT_SETTING_ASK),
           WebsiteSettingsInfo::SINGLE_ORIGIN_ONLY_SCOPE,
           WebsiteSettingsRegistry::DESKTOP |
               WebsiteSettingsRegistry::PLATFORM_ANDROID,
           ContentSettingsInfo::INHERIT_IN_INCOGNITO,
           ContentSettingsInfo::PERSISTENT,
           ContentSettingsInfo::EXCEPTIONS_ON_SECURE_AND_INSECURE_ORIGINS);

  // Register ethereum default value as Ask.
  Register(ContentSettingsType::BRAVE_ETHEREUM, "brave_ethereum",
           CONTENT_SETTING_ASK, WebsiteSettingsInfo::UNSYNCABLE,
           AllowlistedSchemes(),
           ValidSettings(CONTENT_SETTING_ALLOW, CONTENT_SETTING_BLOCK,
                         CONTENT_SETTING_ASK),
           WebsiteSettingsInfo::SINGLE_ORIGIN_ONLY_SCOPE,
           WebsiteSettingsRegistry::DESKTOP |
               WebsiteSettingsRegistry::PLATFORM_ANDROID,
           ContentSettingsInfo::INHERIT_IF_LESS_PERMISSIVE,
           ContentSettingsInfo::PERSISTENT,
           ContentSettingsInfo::EXCEPTIONS_ON_SECURE_AND_INSECURE_ORIGINS);

  // Register solana default value as Ask.
  Register(ContentSettingsType::BRAVE_SOLANA, "brave_solana",
           CONTENT_SETTING_ASK, WebsiteSettingsInfo::UNSYNCABLE,
           AllowlistedSchemes(),
           ValidSettings(CONTENT_SETTING_ALLOW, CONTENT_SETTING_BLOCK,
                         CONTENT_SETTING_ASK),
           WebsiteSettingsInfo::SINGLE_ORIGIN_ONLY_SCOPE,
           WebsiteSettingsRegistry::DESKTOP |
               WebsiteSettingsRegistry::PLATFORM_ANDROID,
           ContentSettingsInfo::INHERIT_IF_LESS_PERMISSIVE,
           ContentSettingsInfo::PERSISTENT,
           ContentSettingsInfo::EXCEPTIONS_ON_SECURE_AND_INSECURE_ORIGINS);

  // Disable background sync by default (brave/brave-browser#4709)
  content_settings_info_.erase(ContentSettingsType::BACKGROUND_SYNC);
  website_settings_registry_->UnRegister(ContentSettingsType::BACKGROUND_SYNC);
  Register(ContentSettingsType::BACKGROUND_SYNC, "background-sync",
           CONTENT_SETTING_BLOCK, WebsiteSettingsInfo::UNSYNCABLE,
           AllowlistedSchemes(),
           ValidSettings(CONTENT_SETTING_ALLOW, CONTENT_SETTING_BLOCK),
           WebsiteSettingsInfo::SINGLE_ORIGIN_ONLY_SCOPE,
           WebsiteSettingsRegistry::DESKTOP |
               WebsiteSettingsRegistry::PLATFORM_ANDROID,
           ContentSettingsInfo::INHERIT_IN_INCOGNITO,
           ContentSettingsInfo::PERSISTENT,
           ContentSettingsInfo::EXCEPTIONS_ON_SECURE_ORIGINS_ONLY);

  // Disable motion sensors by default (brave/brave-browser#4789)
  content_settings_info_.erase(ContentSettingsType::SENSORS);
  website_settings_registry_->UnRegister(ContentSettingsType::SENSORS);
  Register(ContentSettingsType::SENSORS, "sensors", CONTENT_SETTING_BLOCK,
           WebsiteSettingsInfo::UNSYNCABLE, AllowlistedSchemes(),
           ValidSettings(CONTENT_SETTING_ALLOW, CONTENT_SETTING_BLOCK),
           WebsiteSettingsInfo::SINGLE_ORIGIN_ONLY_SCOPE,
           WebsiteSettingsRegistry::DESKTOP |
               WebsiteSettingsRegistry::PLATFORM_ANDROID,
           ContentSettingsInfo::INHERIT_IN_INCOGNITO,
           ContentSettingsInfo::PERSISTENT,
           ContentSettingsInfo::EXCEPTIONS_ON_SECURE_AND_INSECURE_ORIGINS);
}

}  // namespace content_settings
