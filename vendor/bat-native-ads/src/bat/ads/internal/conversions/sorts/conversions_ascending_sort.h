/* Copyright (c) 2020 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_CONVERSIONS_SORTS_CONVERSIONS_ASCENDING_SORT_H_
#define BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_CONVERSIONS_SORTS_CONVERSIONS_ASCENDING_SORT_H_

#include "bat/ads/internal/conversions/sorts/conversions_sort_interface.h"

namespace ads {

class ConversionsAscendingSort final : public ConversionsSortInterface {
 public:
  ConversionsAscendingSort();
  ~ConversionsAscendingSort() override;
  ConversionsAscendingSort(const ConversionsAscendingSort&) = delete;
  ConversionsAscendingSort& operator=(const ConversionsAscendingSort&) = delete;

  ConversionList Apply(const ConversionList& list) const override;
};

}  // namespace ads

#endif  // BRAVE_VENDOR_BAT_NATIVE_ADS_SRC_BAT_ADS_INTERNAL_CONVERSIONS_SORTS_CONVERSIONS_ASCENDING_SORT_H_
