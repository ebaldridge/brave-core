/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_COMPONENTS_BRAVE_WALLET_COMMON_FEATURES_H_
#define BRAVE_COMPONENTS_BRAVE_WALLET_COMMON_FEATURES_H_

#include "base/metrics/field_trial_params.h"

namespace base {
struct Feature;
}  // namespace base

namespace brave_wallet {
namespace features {

extern const base::Feature kNativeBraveWalletFeature;
extern const base::Feature kBraveWalletFilecoinFeature;
extern const base::Feature kBraveWalletSolanaFeature;
extern const base::Feature kBraveWalletSolanaProviderFeature;
extern const base::Feature kBraveWalletDappsSupportFeature;

extern const base::FeatureParam<bool> kFilecoinTestnetEnabled;

}  // namespace features
}  // namespace brave_wallet

#endif  // BRAVE_COMPONENTS_BRAVE_WALLET_COMMON_FEATURES_H_
