/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_AD_EVENTS_INLINE_CONTENT_ADS_INLINE_CONTENT_AD_EVENT_VIEWED_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_AD_EVENTS_INLINE_CONTENT_ADS_INLINE_CONTENT_AD_EVENT_VIEWED_H_

#include "bat/ads/inline_content_ad_info.h"
#include "bat/ads/internal/ad_events/ad_event_interface.h"

namespace ads {
namespace inline_content_ads {

class AdEventViewed final : public AdEventInterface<InlineContentAdInfo> {
 public:
  AdEventViewed();
  ~AdEventViewed() override;
  AdEventViewed(const AdEventViewed&) = delete;
  AdEventViewed& operator=(const AdEventViewed&) = delete;

  void FireEvent(const InlineContentAdInfo& ad) override;
};

}  // namespace inline_content_ads
}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_AD_EVENTS_INLINE_CONTENT_ADS_INLINE_CONTENT_AD_EVENT_VIEWED_H_
