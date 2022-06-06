/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/browser/brave_rewards/rewards_context_utils.h"

#include "brave/browser/profiles/profile_util.h"
#include "content/public/browser/browser_context.h"

namespace brave_rewards {

bool IsAllowedForContext(content::BrowserContext* context) {
  if (context && !brave::IsRegularProfile(context)) {
    return false;
  }

  return true;
}

}  // namespace brave_rewards
