/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "bat/ads/internal/creatives/campaigns_database_util.h"

#include "bat/ads/internal/base/logging_util.h"
#include "bat/ads/internal/creatives/campaigns_database_table.h"

namespace ads {
namespace database {

void DeleteCampaigns() {
  table::Campaigns database_table;
  database_table.Delete([](const bool success) {
    if (!success) {
      BLOG(0, "Failed to delete campaigns");
      return;
    }

    BLOG(3, "Successfully deleted campaigns");
  });
}

}  // namespace database
}  // namespace ads
