/* Copyright (c) 2022 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#ifndef BRAVE_BROWSER_UI_WEBUI_NEW_TAB_PAGE_NTP_BACKGROUND_PREF_H_
#define BRAVE_BROWSER_UI_WEBUI_NEW_TAB_PAGE_NTP_BACKGROUND_PREF_H_

#include <string>

#include "base/values.h"
#include "third_party/abseil-cpp/absl/types/variant.h"
#include "url/gurl.h"

namespace user_prefs {
class PrefRegistrySyncable;
}  // namespace user_prefs

class PrefService;

// NTPBackgroundPref gives easy access to values for NTP background from prefs.
class NTPBackgroundPref final {
 public:
  enum class Type {
    kBrave,  // Images that we supply.
    kCustomImage,
    kSolidColor,
    kGradientColor
  };

  explicit NTPBackgroundPref(PrefService* service);
  NTPBackgroundPref(const NTPBackgroundPref& pref) = delete;
  NTPBackgroundPref& operator=(const NTPBackgroundPref& pref) = delete;
  ~NTPBackgroundPref();

  static void RegisterPref(user_prefs::PrefRegistrySyncable* registry);

  // Try to migrate the old pref for custom background into this new pref.
  void MigrateOldPref();

  // Types
  Type GetType() const;
  void SetType(Type type);
  bool is_brave_type() const { return GetType() == Type::kBrave; }
  bool is_custom_image_type() const { return GetType() == Type::kCustomImage; }
  bool is_solid_color_type() const { return GetType() == Type::kSolidColor; }
  bool is_gradient_color_type() const {
    return GetType() == Type::kGradientColor;
  }

  // Returns true when we should pick one item of selected type every time NTP
  // opens.
  bool ShouldUseRandomItem() const;
  void SetShouldUserRandomItem(bool random);

  // Return an item to use as NTP background.
  absl::variant<GURL, std::string> GetSelectedItem() const;
  absl::variant<GURL, std::string> GetRandomItem() const;

 private:
  const base::Value::Dict* GetPrefValue() const;

  PrefService* service_ = nullptr;
};

#endif  // BRAVE_BROWSER_UI_WEBUI_NEW_TAB_PAGE_NTP_BACKGROUND_PREF_H_
