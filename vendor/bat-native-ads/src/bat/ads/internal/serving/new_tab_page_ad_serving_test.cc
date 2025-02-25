/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/serving/new_tab_page_ad_serving.h"

#include <algorithm>

#include "bat/ads/internal/ad_events/ad_event_unittest_util.h"
#include "bat/ads/internal/base/net/http/http_status_code.h"
#include "bat/ads/internal/base/unittest/unittest_base.h"
#include "bat/ads/internal/base/unittest/unittest_mock_util.h"
#include "bat/ads/internal/base/unittest/unittest_time_util.h"
#include "bat/ads/internal/creatives/new_tab_page_ads/creative_new_tab_page_ad_unittest_util.h"
#include "bat/ads/internal/creatives/new_tab_page_ads/creative_new_tab_page_ads_database_table.h"
#include "bat/ads/internal/creatives/new_tab_page_ads/new_tab_page_ad_builder.h"
#include "bat/ads/internal/geographic/subdivision/subdivision_targeting.h"
#include "bat/ads/internal/resources/behavioral/anti_targeting/anti_targeting_resource.h"
#include "bat/ads/internal/serving/permission_rules/permission_rules_unittest_util.h"
#include "bat/ads/internal/serving/permission_rules/user_activity_permission_rule_unittest_util.h"
#include "bat/ads/internal/serving/serving_features.h"
#include "bat/ads/new_tab_page_ad_info.h"

// npm run test -- brave_unit_tests --filter=BatAds*

namespace ads {

class BatAdsNewTabPageServingIntegrationTest : public UnitTestBase {
 protected:
  BatAdsNewTabPageServingIntegrationTest()
      : subdivision_targeting_(
            std::make_unique<geographic::SubdivisionTargeting>()),
        anti_targeting_resource_(std::make_unique<resource::AntiTargeting>()),
        serving_(std::make_unique<new_tab_page_ads::Serving>(
            subdivision_targeting_.get(),
            anti_targeting_resource_.get())),
        database_table_(
            std::make_unique<database::table::CreativeNewTabPageAds>()) {}

  ~BatAdsNewTabPageServingIntegrationTest() override = default;

  void SetUp() override {
    UnitTestBase::SetUpForTesting(/* is_integration_test */ true);
  }

  void SetUpMocks() override {
    CopyFileFromTestPathToTempPath("confirmations_with_unblinded_tokens.json",
                                   kConfirmationsFilename);

    const URLEndpoints endpoints = {
        {"/v9/catalog", {{net::HTTP_OK, "/empty_catalog.json"}}},
        {// Get issuers request
         R"(/v1/issuers/)",
         {{net::HTTP_OK, R"(
        {
          "ping": 7200000,
          "issuers": [
            {
              "name": "confirmations",
              "publicKeys": [
                {
                  "publicKey": "JsvJluEN35bJBgJWTdW/8dAgPrrTM1I1pXga+o7cllo=",
                  "associatedValue": ""
                },
                {
                  "publicKey": "crDVI1R6xHQZ4D9cQu4muVM5MaaM1QcOT4It8Y/CYlw=",
                  "associatedValue": ""
                }
              ]
            },
            {
              "name": "payments",
              "publicKeys": [
                {
                  "publicKey": "JiwFR2EU/Adf1lgox+xqOVPuc6a/rxdy/LguFG5eaXg=",
                  "associatedValue": "0.1"
                },
                {
                  "publicKey": "bPE1QE65mkIgytffeu7STOfly+x10BXCGuk5pVlOHQU=",
                  "associatedValue": "0.2"
                }
              ]
            }
          ]
        }
        )"}}}};
    MockUrlRequest(ads_client_mock_, endpoints);
  }

  void Save(const CreativeNewTabPageAdList& creative_ads) {
    database_table_->Save(creative_ads,
                          [](const bool success) { ASSERT_TRUE(success); });
  }

  std::unique_ptr<geographic::SubdivisionTargeting> subdivision_targeting_;
  std::unique_ptr<resource::AntiTargeting> anti_targeting_resource_;
  std::unique_ptr<new_tab_page_ads::Serving> serving_;

  std::unique_ptr<database::table::CreativeNewTabPageAds> database_table_;
};

TEST_F(BatAdsNewTabPageServingIntegrationTest, ServeAd) {
  // Arrange
  ForcePermissionRules();

  CreativeNewTabPageAdList creative_ads;
  const CreativeNewTabPageAdInfo& creative_ad = BuildCreativeNewTabPageAd();
  creative_ads.push_back(creative_ad);
  Save(creative_ads);

  // Act
  serving_->MaybeServeAd(
      [&creative_ad](const bool success, const NewTabPageAdInfo& ad) {
        ASSERT_TRUE(success);

        NewTabPageAdInfo expected_ad = BuildNewTabPageAd(creative_ad);
        expected_ad.placement_id = ad.placement_id;

        EXPECT_EQ(expected_ad, ad);
      });

  // Assert
}

TEST_F(BatAdsNewTabPageServingIntegrationTest,
       ServeAdIfNotExceededPerDayExclusionRuleFrequencyCap) {
  // Arrange
  ForcePermissionRules();

  CreativeNewTabPageAdList creative_ads;
  CreativeNewTabPageAdInfo creative_ad = BuildCreativeNewTabPageAd();
  creative_ads.push_back(creative_ad);
  Save(creative_ads);

  const int allowed_ad_count_per_day =
      std::min(creative_ad.per_day, creative_ad.daily_cap);

  AdEventInfo ad_event = BuildAdEvent(creative_ad, AdType::kNewTabPageAd,
                                      ConfirmationType::kServed, Now());
  for (int i = 0; i < allowed_ad_count_per_day - 1; ++i) {
    FireAdEvent(ad_event);
  }

  AdvanceClockBy(base::Hours(1));

  // Act
  serving_->MaybeServeAd(
      [&creative_ad](const bool success, const NewTabPageAdInfo& ad) {
        ASSERT_TRUE(success);

        NewTabPageAdInfo expected_ad = BuildNewTabPageAd(creative_ad);
        expected_ad.placement_id = ad.placement_id;

        EXPECT_EQ(expected_ad, ad);
      });

  // Assert
}

TEST_F(BatAdsNewTabPageServingIntegrationTest,
       DoNotServeAdIfExceededPerDayExclusionRuleFrequencyCap) {
  // Arrange
  ForcePermissionRules();

  CreativeNewTabPageAdList creative_ads;
  CreativeNewTabPageAdInfo creative_ad = BuildCreativeNewTabPageAd();
  creative_ads.push_back(creative_ad);
  Save(creative_ads);

  const int allowed_ad_count_per_day =
      std::min(creative_ad.per_day, creative_ad.daily_cap);

  AdEventInfo ad_event = BuildAdEvent(creative_ad, AdType::kNewTabPageAd,
                                      ConfirmationType::kServed, Now());
  for (int i = 0; i < allowed_ad_count_per_day; ++i) {
    FireAdEvent(ad_event);
  }

  // Act
  serving_->MaybeServeAd([](const bool success, const NewTabPageAdInfo& ad) {
    EXPECT_FALSE(success);
  });

  // Assert
}

TEST_F(BatAdsNewTabPageServingIntegrationTest,
       DoNotServeAdIfNotAllowedDueToPermissionRules) {
  // Arrange
  CreativeNewTabPageAdList creative_ads;
  CreativeNewTabPageAdInfo creative_ad = BuildCreativeNewTabPageAd();
  creative_ads.push_back(creative_ad);
  Save(creative_ads);

  // Act
  serving_->MaybeServeAd([](const bool success, const NewTabPageAdInfo& ad) {
    EXPECT_FALSE(success);
  });

  // Assert
}

TEST_F(BatAdsNewTabPageServingIntegrationTest,
       ServeAdIfNotExceededAdsPerHourPermissionRuleFrequencyCap) {
  // Arrange
  ForcePermissionRules();

  CreativeNewTabPageAdList creative_ads;
  CreativeNewTabPageAdInfo creative_ad1 = BuildCreativeNewTabPageAd();
  CreativeNewTabPageAdInfo creative_ad2 = BuildCreativeNewTabPageAd();
  creative_ads.push_back(creative_ad1);
  creative_ads.push_back(creative_ad2);
  Save(creative_ads);

  AdEventInfo ad_event1 = BuildAdEvent(creative_ad1, AdType::kNewTabPageAd,
                                       ConfirmationType::kServed, Now());

  const int ads_per_hour = features::GetMaximumNewTabPageAdsPerHour();
  for (int i = 0; i < ads_per_hour - 1; ++i) {
    FireAdEvent(ad_event1);
  }

  AdvanceClockBy(features::GetNewTabPageAdsMinimumWaitTime());

  // Act
  serving_->MaybeServeAd(
      [&creative_ad2](const bool success, const NewTabPageAdInfo& ad) {
        ASSERT_TRUE(success);

        NewTabPageAdInfo expected_ad = BuildNewTabPageAd(creative_ad2);
        expected_ad.placement_id = ad.placement_id;

        EXPECT_EQ(expected_ad, ad);
      });

  // Assert
}

TEST_F(BatAdsNewTabPageServingIntegrationTest,
       DoNotServeAdIfExceededAdsPerHourPermissionRuleFrequencyCap) {
  // Arrange
  ForcePermissionRules();

  CreativeNewTabPageAdList creative_ads;
  CreativeNewTabPageAdInfo creative_ad1 = BuildCreativeNewTabPageAd();
  CreativeNewTabPageAdInfo creative_ad2 = BuildCreativeNewTabPageAd();
  creative_ads.push_back(creative_ad1);
  creative_ads.push_back(creative_ad2);
  Save(creative_ads);

  AdEventInfo ad_event1 = BuildAdEvent(creative_ad1, AdType::kNewTabPageAd,
                                       ConfirmationType::kServed, Now());

  const int ads_per_hour = features::GetMaximumNewTabPageAdsPerHour();
  for (int i = 0; i < ads_per_hour; ++i) {
    FireAdEvent(ad_event1);
  }

  // Act
  serving_->MaybeServeAd([](const bool success, const NewTabPageAdInfo& ad) {
    EXPECT_FALSE(success);
  });

  // Assert
}

TEST_F(BatAdsNewTabPageServingIntegrationTest,
       ServeAdIfNotExceededAdsPerDayPermissionRuleFrequencyCap) {
  // Arrange
  ForcePermissionRules();

  CreativeNewTabPageAdList creative_ads;
  CreativeNewTabPageAdInfo creative_ad1 = BuildCreativeNewTabPageAd();
  CreativeNewTabPageAdInfo creative_ad2 = BuildCreativeNewTabPageAd();
  creative_ads.push_back(creative_ad1);
  creative_ads.push_back(creative_ad2);
  Save(creative_ads);

  AdEventInfo ad_event1 = BuildAdEvent(creative_ad1, AdType::kNewTabPageAd,
                                       ConfirmationType::kServed, Now());

  const int ads_per_day = features::GetMaximumNewTabPageAdsPerDay();
  for (int i = 0; i < ads_per_day - 1; ++i) {
    FireAdEvent(ad_event1);
  }

  AdvanceClockBy(base::Hours(1));

  // Act
  serving_->MaybeServeAd(
      [&creative_ad2](const bool success, const NewTabPageAdInfo& ad) {
        ASSERT_TRUE(success);

        NewTabPageAdInfo expected_ad = BuildNewTabPageAd(creative_ad2);
        expected_ad.placement_id = ad.placement_id;

        EXPECT_EQ(expected_ad, ad);
      });

  // Assert
}

TEST_F(BatAdsNewTabPageServingIntegrationTest,
       DoNotServeAdIfExceededAdsPerDayPermissionRuleFrequencyCap) {
  // Arrange
  ForcePermissionRules();

  CreativeNewTabPageAdList creative_ads;
  CreativeNewTabPageAdInfo creative_ad1 = BuildCreativeNewTabPageAd();
  CreativeNewTabPageAdInfo creative_ad2 = BuildCreativeNewTabPageAd();
  creative_ads.push_back(creative_ad1);
  creative_ads.push_back(creative_ad2);
  Save(creative_ads);

  AdEventInfo ad_event1 = BuildAdEvent(creative_ad1, AdType::kNewTabPageAd,
                                       ConfirmationType::kServed, Now());

  const int ads_per_day = features::GetMaximumNewTabPageAdsPerDay();
  for (int i = 0; i < ads_per_day; ++i) {
    FireAdEvent(ad_event1);
  }

  AdvanceClockBy(base::Hours(1));

  // Act
  serving_->MaybeServeAd([](const bool success, const NewTabPageAdInfo& ad) {
    EXPECT_FALSE(success);
  });

  // Assert
}

}  // namespace ads
