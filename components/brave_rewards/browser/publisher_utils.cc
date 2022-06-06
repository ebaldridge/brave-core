/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/components/brave_rewards/browser/publisher_utils.h"

#include <algorithm>
#include <array>

#include "brave/components/ipfs/buildflags/buildflags.h"
#include "net/base/registry_controlled_domains/registry_controlled_domain.h"

#if BUILDFLAG(ENABLE_IPFS)
#include "brave/components/ipfs/ipfs_constants.h"
#include "brave/components/ipfs/ipfs_utils.h"
#endif

namespace brave_rewards {

namespace {

const std::array<std::string, 6> kMediaPlatformDomains = {
    "twitter.com", "github.com", "reddit.com",
    "twitch.tv",   "vimeo.com",  "youtube.com"};

bool IsMediaPlatformURL(const GURL& url) {
  return std::any_of(
      kMediaPlatformDomains.begin(), kMediaPlatformDomains.end(),
      [&url](auto& domain) {
        return net::registry_controlled_domains::SameDomainOrHost(
            url, GURL("https://" + domain),
            net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
      });
}

}  // namespace

std::string GetPublisherIdFromURL(const GURL& url) {
  if (IsMediaPlatformURL(url)) {
    return "";
  }

#if BUILDFLAG(ENABLE_IPFS)
  if (url.SchemeIs(ipfs::kIPNSScheme)) {
    return ipfs::GetRegistryDomainFromIPNS(url);
  }
#endif

  if (!url.SchemeIsHTTPOrHTTPS()) {
    return "";
  }

  return net::registry_controlled_domains::GetDomainAndRegistry(
      url, net::registry_controlled_domains::INCLUDE_PRIVATE_REGISTRIES);
}

}  // namespace brave_rewards
