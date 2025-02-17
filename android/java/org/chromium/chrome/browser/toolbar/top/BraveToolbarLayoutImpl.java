/* Copyright (c) 2019 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.toolbar.top;

import static org.chromium.ui.base.ViewUtils.dpToPx;

import android.animation.Animator;
import android.animation.ObjectAnimator;
import android.app.Dialog;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.graphics.Canvas;
import android.graphics.PorterDuff;
import android.graphics.drawable.Drawable;
import android.text.Spannable;
import android.text.SpannableString;
import android.text.SpannableStringBuilder;
import android.text.Spanned;
import android.text.TextPaint;
import android.text.method.LinkMovementMethod;
import android.text.style.ClickableSpan;
import android.text.style.ForegroundColorSpan;
import android.text.style.ImageSpan;
import android.util.AttributeSet;
import android.util.Pair;
import android.view.Gravity;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.PopupWindow;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AlertDialog;
import androidx.appcompat.widget.AppCompatImageView;
import androidx.core.content.ContextCompat;
import androidx.core.content.res.ResourcesCompat;

import org.chromium.base.ApiCompatibilityUtils;
import org.chromium.base.BraveReflectionUtil;
import org.chromium.base.ContextUtils;
import org.chromium.base.MathUtils;
import org.chromium.base.ThreadUtils;
import org.chromium.base.supplier.BooleanSupplier;
import org.chromium.base.supplier.ObservableSupplier;
import org.chromium.base.task.AsyncTask;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.BraveAdsNativeHelper;
import org.chromium.chrome.browser.BraveFeatureList;
import org.chromium.chrome.browser.BraveRelaunchUtils;
import org.chromium.chrome.browser.BraveRewardsHelper;
import org.chromium.chrome.browser.BraveRewardsNativeWorker;
import org.chromium.chrome.browser.BraveRewardsObserver;
import org.chromium.chrome.browser.BraveRewardsPanelPopup;
import org.chromium.chrome.browser.app.BraveActivity;
import org.chromium.chrome.browser.brave_stats.BraveStatsUtil;
import org.chromium.chrome.browser.crypto_wallet.controller.DAppsWalletController;
import org.chromium.chrome.browser.crypto_wallet.util.Utils;
import org.chromium.chrome.browser.custom_layout.popup_window_tooltip.PopupWindowTooltip;
import org.chromium.chrome.browser.custom_layout.popup_window_tooltip.PopupWindowTooltipUtils;
import org.chromium.chrome.browser.customtabs.CustomTabActivity;
import org.chromium.chrome.browser.customtabs.features.toolbar.CustomTabToolbar;
import org.chromium.chrome.browser.dialogs.BraveAdsSignupDialog;
import org.chromium.chrome.browser.flags.ChromeFeatureList;
import org.chromium.chrome.browser.lifecycle.ConfigurationChangedObserver;
import org.chromium.chrome.browser.local_database.BraveStatsTable;
import org.chromium.chrome.browser.local_database.DatabaseHelper;
import org.chromium.chrome.browser.local_database.SavedBandwidthTable;
import org.chromium.chrome.browser.notifications.retention.RetentionNotificationUtil;
import org.chromium.chrome.browser.ntp.NewTabPage;
import org.chromium.chrome.browser.omnibox.LocationBarCoordinator;
import org.chromium.chrome.browser.onboarding.BraveTalkOptInPopup;
import org.chromium.chrome.browser.onboarding.BraveTalkOptInPopupListener;
import org.chromium.chrome.browser.onboarding.OnboardingPrefManager;
import org.chromium.chrome.browser.onboarding.SearchActivity;
import org.chromium.chrome.browser.onboarding.v2.HighlightItem;
import org.chromium.chrome.browser.onboarding.v2.HighlightView;
import org.chromium.chrome.browser.preferences.BravePref;
import org.chromium.chrome.browser.preferences.BravePrefServiceBridge;
import org.chromium.chrome.browser.preferences.website.BraveShieldsContentSettings;
import org.chromium.chrome.browser.preferences.website.BraveShieldsContentSettingsObserver;
import org.chromium.chrome.browser.profiles.Profile;
import org.chromium.chrome.browser.rewards.BraveRewardsPanel;
import org.chromium.chrome.browser.settings.AppearancePreferences;
import org.chromium.chrome.browser.settings.BraveSearchEngineUtils;
import org.chromium.chrome.browser.shields.BraveShieldsHandler;
import org.chromium.chrome.browser.shields.BraveShieldsMenuObserver;
import org.chromium.chrome.browser.shields.BraveShieldsUtils;
import org.chromium.chrome.browser.tab.Tab;
import org.chromium.chrome.browser.tab.TabImpl;
import org.chromium.chrome.browser.tab.TabSelectionType;
import org.chromium.chrome.browser.tabmodel.TabModelSelector;
import org.chromium.chrome.browser.tabmodel.TabModelSelectorTabModelObserver;
import org.chromium.chrome.browser.tabmodel.TabModelSelectorTabObserver;
import org.chromium.chrome.browser.theme.ThemeUtils;
import org.chromium.chrome.browser.toolbar.HomeButton;
import org.chromium.chrome.browser.toolbar.ToolbarColors;
import org.chromium.chrome.browser.toolbar.ToolbarDataProvider;
import org.chromium.chrome.browser.toolbar.ToolbarTabController;
import org.chromium.chrome.browser.toolbar.bottom.BottomToolbarVariationManager;
import org.chromium.chrome.browser.toolbar.menu_button.BraveMenuButtonCoordinator;
import org.chromium.chrome.browser.toolbar.menu_button.MenuButtonCoordinator;
import org.chromium.chrome.browser.toolbar.top.NavigationPopup.HistoryDelegate;
import org.chromium.chrome.browser.toolbar.top.ToolbarLayout;
import org.chromium.chrome.browser.toolbar.top.ToolbarTablet.OfflineDownloader;
import org.chromium.chrome.browser.util.PackageUtils;
import org.chromium.components.browser_ui.styles.ChromeColors;
import org.chromium.components.embedder_support.util.UrlConstants;
import org.chromium.components.embedder_support.util.UrlUtilities;
import org.chromium.components.url_formatter.UrlFormatter;
import org.chromium.components.user_prefs.UserPrefs;
import org.chromium.content_public.browser.NavigationHandle;
import org.chromium.ui.UiUtils;
import org.chromium.ui.interpolators.BakedBezierInterpolator;
import org.chromium.ui.widget.Toast;
import org.chromium.url.GURL;

import java.net.URL;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.Collections;
import java.util.Date;
import java.util.EnumSet;
import java.util.HashSet;
import java.util.List;
import java.util.Locale;
import java.util.Set;

public abstract class BraveToolbarLayoutImpl extends ToolbarLayout
        implements BraveToolbarLayout, OnClickListener, View.OnLongClickListener,
                   BraveRewardsObserver, BraveRewardsNativeWorker.PublisherObserver {
    private static final String JAPAN_COUNTRY_CODE = "JP";
    private static final List<String> mBraveSearchEngineDefaultRegions =
            Arrays.asList("CA", "DE", "FR", "GB", "US");
    private static final long MB_10 = 10000000;
    private static final long MINUTES_10 = 10 * 60 * 1000;
    private static final int URL_FOCUS_TOOLBAR_BUTTONS_TRANSLATION_X_DP = 10;

    private DatabaseHelper mDatabaseHelper = DatabaseHelper.getInstance();

    private ImageButton mBraveWalletButton;
    private ImageButton mBraveShieldsButton;
    private ImageButton mBraveRewardsButton;
    private HomeButton mHomeButton;
    private FrameLayout mWalletLayout;
    private FrameLayout mShieldsLayout;
    private FrameLayout mRewardsLayout;
    private BraveShieldsHandler mBraveShieldsHandler;
    private TabModelSelectorTabObserver mTabModelSelectorTabObserver;
    private TabModelSelectorTabModelObserver mTabModelSelectorTabModelObserver;
    private BraveRewardsNativeWorker mBraveRewardsNativeWorker;
    private BraveRewardsPanel mRewardsPopup;
    private DAppsWalletController mDAppsWalletController;
    private BraveTalkOptInPopup mBraveTalkOptInPopup;
    private BraveShieldsContentSettings mBraveShieldsContentSettings;
    private BraveShieldsContentSettingsObserver mBraveShieldsContentSettingsObserver;
    private TextView mBraveRewardsNotificationsCount;
    private ImageView mBraveRewardsOnboardingIcon;
    private View mBraveWalletBadge;
    private boolean mShieldsLayoutIsColorBackground;
    private int mCurrentToolbarColor;

    private boolean mIsPublisherVerified;
    private boolean mIsNotificationPosted;
    private boolean mIsInitialNotificationPosted; // initial red circle notification

    private PopupWindowTooltip mShieldsPopupWindowTooltip;

    private boolean mIsBottomToolbarVisible;

    private final Set<Integer> mTabsWithWalletIcon =
            Collections.synchronizedSet(new HashSet<Integer>());

    private enum BIGTECH_COMPANY { Google, Facebook, Amazon }

    public BraveToolbarLayoutImpl(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    void destroy() {
        if (mBraveShieldsContentSettings != null) {
            mBraveShieldsContentSettings.removeObserver(mBraveShieldsContentSettingsObserver);
        }
        super.destroy();

        if (mBraveRewardsNativeWorker != null) {
            mBraveRewardsNativeWorker.RemoveObserver(this);
            mBraveRewardsNativeWorker.RemovePublisherObserver(this);
        }
    }

    @Override
    protected void onFinishInflate() {
        super.onFinishInflate();

        if (BraveReflectionUtil.EqualTypes(this.getClass(), ToolbarTablet.class)) {
            ImageButton forwardButton = findViewById(R.id.forward_button);
            if (forwardButton != null) {
                final Drawable forwardButtonDrawable = UiUtils.getTintedDrawable(getContext(),
                        R.drawable.btn_right_tablet, R.color.default_icon_color_tint_list);
                forwardButton.setImageDrawable(forwardButtonDrawable);
            }
        }

        mWalletLayout = (FrameLayout) findViewById(R.id.brave_wallet_button_layout);
        mShieldsLayout = (FrameLayout) findViewById(R.id.brave_shields_button_layout);
        mRewardsLayout = (FrameLayout) findViewById(R.id.brave_rewards_button_layout);
        mBraveRewardsNotificationsCount = (TextView) findViewById(R.id.br_notifications_count);
        mBraveRewardsOnboardingIcon = findViewById(R.id.br_rewards_onboarding_icon);
        mBraveWalletButton = (ImageButton) findViewById(R.id.brave_wallet_button);
        mBraveShieldsButton = (ImageButton) findViewById(R.id.brave_shields_button);
        mBraveRewardsButton = (ImageButton) findViewById(R.id.brave_rewards_button);
        mHomeButton = (HomeButton) findViewById(R.id.home_button);
        mBraveWalletBadge = findViewById(R.id.wallet_notfication_badge);

        if (mHomeButton != null) {
            mHomeButton.setOnLongClickListener(this);
        }

        if (mBraveShieldsButton != null) {
            mBraveShieldsButton.setClickable(true);
            mBraveShieldsButton.setOnClickListener(this);
            mBraveShieldsButton.setOnLongClickListener(this);
        }

        if (mBraveRewardsButton != null) {
            mBraveRewardsButton.setClickable(true);
            mBraveRewardsButton.setOnClickListener(this);
            mBraveRewardsButton.setOnLongClickListener(this);
        }

        if (mBraveWalletButton != null) {
            mBraveWalletButton.setClickable(true);
            mBraveWalletButton.setOnClickListener(this);
            mBraveWalletButton.setOnLongClickListener(this);
        }

        mBraveShieldsHandler = new BraveShieldsHandler(getContext());
        if (!mBraveShieldsHandler.isDisconnectEntityLoaded
                && !BraveShieldsUtils.hasShieldsTooltipShown(
                        BraveShieldsUtils.PREF_SHIELDS_TOOLTIP)) {
            mBraveShieldsHandler.loadDisconnectEntityList();
        }
        mBraveShieldsHandler.addObserver(new BraveShieldsMenuObserver() {
            @Override
            public void onMenuTopShieldsChanged(boolean isOn, boolean isTopShield) {
                Tab currentTab = getToolbarDataProvider().getTab();
                if (currentTab == null) {
                    return;
                }
                if (isTopShield) {
                    updateBraveShieldsButtonState(currentTab);
                }
                if (currentTab.isLoading()) {
                    currentTab.stopLoading();
                }
                currentTab.reloadIgnoringCache();
                if (null != mBraveShieldsHandler) {
                    // Clean the Bravery Panel
                    mBraveShieldsHandler.updateValues(0, 0, 0, 0);
                }
            }
        });
        mBraveShieldsContentSettingsObserver = new BraveShieldsContentSettingsObserver() {
            @Override
            public void blockEvent(int tabId, String block_type, String subresource) {
                mBraveShieldsHandler.addStat(tabId, block_type, subresource);
                Tab currentTab = getToolbarDataProvider().getTab();
                if (currentTab == null || currentTab.getId() != tabId) {
                    return;
                }
                mBraveShieldsHandler.updateValues(tabId);
                if (!isIncognito() && OnboardingPrefManager.getInstance().isBraveStatsEnabled()
                        && (block_type.equals(BraveShieldsContentSettings.RESOURCE_IDENTIFIER_ADS)
                                || block_type.equals(BraveShieldsContentSettings
                                                             .RESOURCE_IDENTIFIER_TRACKERS))) {
                    addStatsToDb(block_type, subresource, currentTab.getUrl().getSpec());
                }
            }

            @Override
            public void savedBandwidth(long savings) {
                if (!isIncognito() && OnboardingPrefManager.getInstance().isBraveStatsEnabled()) {
                    addSavedBandwidthToDb(savings);
                }
            }
        };
        // Initially show shields off image. Shields button state will be updated when tab is
        // shown and loading state is changed.
        updateBraveShieldsButtonState(null);
        if (BraveReflectionUtil.EqualTypes(this.getClass(), ToolbarPhone.class)) {
            if (getMenuButtonCoordinator() != null && isMenuButtonOnBottom()) {
                getMenuButtonCoordinator().setVisibility(false);
            }
        }

        if (BraveReflectionUtil.EqualTypes(this.getClass(), CustomTabToolbar.class)) {
            LinearLayout customActionButtons = findViewById(R.id.action_buttons);
            assert customActionButtons != null : "Something has changed in the upstream!";
            if (customActionButtons != null && mBraveShieldsButton != null) {
                ViewGroup.MarginLayoutParams braveShieldsButtonLayout =
                        (ViewGroup.MarginLayoutParams) mBraveShieldsButton.getLayoutParams();
                ViewGroup.MarginLayoutParams actionButtonsLayout =
                        (ViewGroup.MarginLayoutParams) customActionButtons.getLayoutParams();
                actionButtonsLayout.setMarginEnd(actionButtonsLayout.getMarginEnd()
                        + braveShieldsButtonLayout.getMarginEnd());
                customActionButtons.setLayoutParams(actionButtonsLayout);
            }
        }
    }

    @Override
    protected void onNativeLibraryReady() {
        super.onNativeLibraryReady();
        mBraveShieldsContentSettings = BraveShieldsContentSettings.getInstance();
        mBraveShieldsContentSettings.addObserver(mBraveShieldsContentSettingsObserver);

        SharedPreferences sharedPreferences = ContextUtils.getAppSharedPreferences();
        if (ChromeFeatureList.isEnabled(BraveFeatureList.BRAVE_REWARDS)
                && !BravePrefServiceBridge.getInstance().getSafetynetCheckFailed()
                && sharedPreferences.getBoolean(
                        AppearancePreferences.PREF_SHOW_BRAVE_REWARDS_ICON, true)
                && mRewardsLayout != null) {
            mRewardsLayout.setVisibility(View.VISIBLE);
        }
        if (mShieldsLayout != null) {
            updateShieldsLayoutBackground(
                    !(mRewardsLayout != null && mRewardsLayout.getVisibility() == View.VISIBLE));
            mShieldsLayout.setVisibility(View.VISIBLE);
        }
        mBraveRewardsNativeWorker = BraveRewardsNativeWorker.getInstance();
        if (mBraveRewardsNativeWorker != null) {
            mBraveRewardsNativeWorker.AddObserver(this);
            mBraveRewardsNativeWorker.AddPublisherObserver(this);
            mBraveRewardsNativeWorker.TriggerOnNotifyFrontTabUrlChanged();
            mBraveRewardsNativeWorker.GetAllNotifications();
        }
    }

    @Override
    public void setTabModelSelector(TabModelSelector selector) {
        // We might miss events before calling setTabModelSelector, so we need
        // to proactively update the shields button state here, otherwise shields
        // might sometimes show as disabled while it is actually enabled.
        updateBraveShieldsButtonState(getToolbarDataProvider().getTab());
        mTabModelSelectorTabObserver = new TabModelSelectorTabObserver(selector) {
            @Override
            protected void onTabRegistered(Tab tab) {
                super.onTabRegistered(tab);
                if (tab.isIncognito()) {
                    showWalletIcon(false);
                }
            }

            @Override
            public void onShown(Tab tab, @TabSelectionType int type) {
                // Update shields button state when visible tab is changed.
                updateBraveShieldsButtonState(tab);
            }

            @Override
            public void onPageLoadStarted(Tab tab, GURL url) {
                showWalletIcon(false);
                if (getToolbarDataProvider().getTab() == tab) {
                    updateBraveShieldsButtonState(tab);
                }
                mBraveShieldsHandler.clearBraveShieldsCount(tab.getId());
                dismissShieldsTooltip();
            }

            @Override
            public void onPageLoadFinished(final Tab tab, GURL url) {
                if (getToolbarDataProvider().getTab() == tab) {
                    mBraveShieldsHandler.updateHost(url.getSpec());
                    updateBraveShieldsButtonState(tab);

                    if (mBraveShieldsButton != null && mBraveShieldsButton.isShown()
                            && mBraveShieldsHandler != null && !mBraveShieldsHandler.isShowing()) {
                        checkForTooltip(tab);
                    }
                }
            }

            @Override
            public void onDidFinishNavigation(Tab tab, NavigationHandle navigation) {
                if (getToolbarDataProvider().getTab() == tab && mBraveRewardsNativeWorker != null
                        && !tab.isIncognito()) {
                    mBraveRewardsNativeWorker.OnNotifyFrontTabUrlChanged(
                            tab.getId(), tab.getUrl().getSpec());
                }
                if (PackageUtils.isFirstInstall(getContext()) && tab.getUrl().getSpec() != null
                        && (tab.getUrl().getSpec().equals(BraveActivity.BRAVE_REWARDS_SETTINGS_URL))
                        && !BraveAdsNativeHelper.nativeIsBraveAdsEnabled(
                                Profile.getLastUsedRegularProfile())
                        && BraveRewardsHelper.shouldShowBraveRewardsOnboardingModal()
                        && ChromeFeatureList.isEnabled(BraveFeatureList.BRAVE_REWARDS)) {
                    showBraveRewardsOnboardingModal();
                    BraveRewardsHelper.updateBraveRewardsAppOpenCount();
                }
            }

            @Override
            public void onDestroyed(Tab tab) {
                // Remove references for the ads from the Database. Tab is destroyed, they are not
                // needed anymore.
                new Thread() {
                    @Override
                    public void run() {
                        mDatabaseHelper.deleteDisplayAdsFromTab(tab.getId());
                    }
                }.start();
                mBraveShieldsHandler.removeStat(tab.getId());
                mTabsWithWalletIcon.remove(tab.getId());
            }
        };

        mTabModelSelectorTabModelObserver = new TabModelSelectorTabModelObserver(selector) {
            @Override
            public void didSelectTab(Tab tab, @TabSelectionType int type, int lastId) {
                if (mBraveRewardsNativeWorker != null && !tab.isIncognito()) {
                    mBraveRewardsNativeWorker.OnNotifyFrontTabUrlChanged(
                            tab.getId(), tab.getUrl().getSpec());
                    if (getToolbarDataProvider().getTab() == tab) {
                        showWalletIcon(mTabsWithWalletIcon.contains(tab.getId()));
                    } else {
                        mWalletLayout.setVisibility(mTabsWithWalletIcon.contains(tab.getId())
                                        ? View.VISIBLE
                                        : View.GONE);
                    }
                }
            }
        };
    }

    private void checkForTooltip(Tab tab) {
        if (!BraveShieldsUtils.isTooltipShown) {
            if (!BraveShieldsUtils.hasShieldsTooltipShown(BraveShieldsUtils.PREF_SHIELDS_TOOLTIP)
                    && mBraveShieldsHandler.getTrackersBlockedCount(tab.getId())
                                    + mBraveShieldsHandler.getAdsBlockedCount(tab.getId())
                            > 0) {
                showTooltip(BraveShieldsUtils.PREF_SHIELDS_TOOLTIP, tab.getId());
            }
        }
    }

    private void showTooltip(String tooltipPref, int tabId) {
        if (BraveActivity.getBraveActivity() != null) {
            HighlightView highlightView = new HighlightView(getContext(), null);
            highlightView.setColor(ContextCompat.getColor(
                    getContext(), R.color.onboarding_search_highlight_color));
            ViewGroup viewGroup =
                    BraveActivity.getBraveActivity().getWindow().getDecorView().findViewById(
                            android.R.id.content);
            float padding = (float) dpToPx(getContext(), 20);
            mShieldsPopupWindowTooltip =
                    new PopupWindowTooltip.Builder(getContext())
                            .anchorView(mBraveShieldsButton)
                            .arrowColor(ContextCompat.getColor(
                                    getContext(), R.color.onboarding_arrow_color))
                            .gravity(Gravity.BOTTOM)
                            .dismissOnOutsideTouch(true)
                            .dismissOnInsideTouch(false)
                            .backgroundDimDisabled(true)
                            .padding(padding)
                            .parentPaddingHorizontal(dpToPx(getContext(), 10))
                            .modal(true)
                            .onDismissListener(tooltip -> {
                                if (viewGroup != null && highlightView != null) {
                                    highlightView.stopAnimation();
                                    viewGroup.removeView(highlightView);
                                }
                            })
                            .contentView(R.layout.brave_shields_tooltip_layout)
                            .build();

            ArrayList<String> blockerNamesList = mBraveShieldsHandler.getBlockerNamesList(tabId);

            int adsTrackersCount = mBraveShieldsHandler.getTrackersBlockedCount(tabId)
                    + mBraveShieldsHandler.getAdsBlockedCount(tabId);

            String displayTrackerName = "";
            if (blockerNamesList.contains(BIGTECH_COMPANY.Google.name())) {
                displayTrackerName = BIGTECH_COMPANY.Google.name();
            } else if (blockerNamesList.contains(BIGTECH_COMPANY.Facebook.name())) {
                displayTrackerName = BIGTECH_COMPANY.Facebook.name();
            } else if (blockerNamesList.contains(BIGTECH_COMPANY.Amazon.name())) {
                displayTrackerName = BIGTECH_COMPANY.Amazon.name();
            }

            String trackerText = "";
            if (!displayTrackerName.isEmpty()) {
                trackerText = String.format(getContext().getResources().getString(
                                                    R.string.shield_bigtech_tracker_blocked),
                        displayTrackerName, String.valueOf(adsTrackersCount - 1));
            } else {
                trackerText = String.format(
                        getContext().getResources().getString(R.string.shield_tracker_blocked),
                        String.valueOf(adsTrackersCount));
            }

            TextView tvBlocked = mShieldsPopupWindowTooltip.findViewById(R.id.tv_blocked);
            tvBlocked.setText(trackerText);

            if (mBraveShieldsButton != null && mBraveShieldsButton.isShown()) {
                viewGroup.addView(highlightView);
                HighlightItem item = new HighlightItem(mBraveShieldsButton);

                ImageButton braveShieldButton =
                        new ImageButton(getContext(), null, R.style.ToolbarButton);
                braveShieldButton.setImageResource(R.drawable.btn_brave);
                FrameLayout.LayoutParams braveShieldParams =
                        new FrameLayout.LayoutParams(FrameLayout.LayoutParams.WRAP_CONTENT,
                                FrameLayout.LayoutParams.WRAP_CONTENT);

                int[] location = new int[2];
                highlightView.getLocationOnScreen(location);
                braveShieldParams.leftMargin = item.getScreenLeft() + dpToPx(getContext(), 10);
                braveShieldParams.topMargin = item.getScreenTop()
                        + ((item.getScreenBottom() - item.getScreenTop()) / 4) - location[1];
                braveShieldButton.setLayoutParams(braveShieldParams);
                highlightView.addView(braveShieldButton);

                highlightView.setShouldShowHighlight(true);
                highlightView.setHighlightTransparent(true);
                highlightView.setHighlightItem(item);
                highlightView.initializeAnimators();
                highlightView.startAnimation();

                mShieldsPopupWindowTooltip.show();
                BraveShieldsUtils.setShieldsTooltipShown(tooltipPref, true);
                BraveShieldsUtils.isTooltipShown = true;
            }
        }
    }

    public void dismissShieldsTooltip() {
        if (mShieldsPopupWindowTooltip != null && mShieldsPopupWindowTooltip.isShowing()) {
            mShieldsPopupWindowTooltip.dismiss();
            mShieldsPopupWindowTooltip = null;
        }
    }

    public void reopenShieldsPanel() {
        if (mBraveShieldsHandler != null && mBraveShieldsHandler.isShowing()) {
            mBraveShieldsHandler.hideBraveShieldsMenu();
            showShieldsMenu(mBraveShieldsButton);
        }
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        dismissShieldsTooltip();
        reopenShieldsPanel();
        // TODO: show wallet panel
    }

    private void showBraveRewardsOnboardingModal() {
        Context context = getContext();
        final Dialog dialog = new Dialog(context);
        dialog.requestWindowFeature(Window.FEATURE_NO_TITLE);
        dialog.setCancelable(false);
        dialog.setContentView(R.layout.brave_rewards_onboarding_modal);
        dialog.getWindow().setBackgroundDrawableResource(android.R.color.transparent);

        View braveRewardsOnboardingModalView =
                dialog.findViewById(R.id.brave_rewards_onboarding_modal_layout);
        // braveRewardsOnboardingModalView.setBackgroundColor(
        //         context.getResources().getColor(android.R.color.white));
        braveRewardsOnboardingModalView.setVisibility(View.VISIBLE);

        String tosText =
                String.format(context.getResources().getString(R.string.brave_rewards_tos_text),
                        context.getResources().getString(R.string.terms_of_service),
                        context.getResources().getString(R.string.privacy_policy));
        int termsOfServiceIndex =
                tosText.indexOf(context.getResources().getString(R.string.terms_of_service));
        Spanned tosTextSpanned = BraveRewardsHelper.spannedFromHtmlString(tosText);
        SpannableString tosTextSS = new SpannableString(tosTextSpanned.toString());

        ClickableSpan tosClickableSpan = new ClickableSpan() {
            @Override
            public void onClick(@NonNull View textView) {
                CustomTabActivity.showInfoPage(context, BraveActivity.BRAVE_TERMS_PAGE);
            }
            @Override
            public void updateDrawState(@NonNull TextPaint ds) {
                super.updateDrawState(ds);
                ds.setUnderlineText(false);
            }
        };

        tosTextSS.setSpan(tosClickableSpan, termsOfServiceIndex,
                termsOfServiceIndex
                        + context.getResources().getString(R.string.terms_of_service).length(),
                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        tosTextSS.setSpan(new ForegroundColorSpan(context.getResources().getColor(
                                  R.color.brave_rewards_modal_theme_color)),
                termsOfServiceIndex,
                termsOfServiceIndex
                        + context.getResources().getString(R.string.terms_of_service).length(),
                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);

        ClickableSpan privacyProtectionClickableSpan = new ClickableSpan() {
            @Override
            public void onClick(@NonNull View textView) {
                CustomTabActivity.showInfoPage(context, BraveActivity.BRAVE_PRIVACY_POLICY);
            }
            @Override
            public void updateDrawState(@NonNull TextPaint ds) {
                super.updateDrawState(ds);
                ds.setUnderlineText(false);
            }
        };

        int privacyPolicyIndex =
                tosText.indexOf(context.getResources().getString(R.string.privacy_policy));
        tosTextSS.setSpan(privacyProtectionClickableSpan, privacyPolicyIndex,
                privacyPolicyIndex
                        + context.getResources().getString(R.string.privacy_policy).length(),
                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        tosTextSS.setSpan(new ForegroundColorSpan(context.getResources().getColor(
                                  R.color.brave_rewards_modal_theme_color)),
                privacyPolicyIndex,
                privacyPolicyIndex
                        + context.getResources().getString(R.string.privacy_policy).length(),
                Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);

        TextView tosAndPpText = braveRewardsOnboardingModalView.findViewById(
                R.id.brave_rewards_onboarding_modal_tos_pp_text);
        tosAndPpText.setMovementMethod(LinkMovementMethod.getInstance());
        tosAndPpText.setText(tosTextSS);

        TextView takeQuickTourButton =
                braveRewardsOnboardingModalView.findViewById(R.id.take_quick_tour_button);
        takeQuickTourButton.setOnClickListener((new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                BraveRewardsHelper.setShowBraveRewardsOnboardingOnce(true);
                openRewardsPanel();
                dialog.dismiss();
            }
        }));
        TextView btnBraveRewards =
                braveRewardsOnboardingModalView.findViewById(R.id.start_using_brave_rewards_text);
        btnBraveRewards.setOnClickListener((new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                BraveAdsNativeHelper.nativeSetAdsEnabled(Profile.getLastUsedRegularProfile());
                BraveRewardsNativeWorker.getInstance().SetAutoContributeEnabled(true);
                BraveRewardsHelper.setShowBraveRewardsOnboardingModal(false);
                if (BraveActivity.getBraveActivity() != null) {
                    BraveRewardsHelper.setShowBraveRewardsOnboardingOnce(true);
                    BraveActivity.getBraveActivity().openRewardsPanel();
                }
                dialog.dismiss();
            }
        }));

        dialog.show();
    }

    private void addSavedBandwidthToDb(long savings) {
        new AsyncTask<Void>() {
            @Override
            protected Void doInBackground() {
                try {
                    SavedBandwidthTable savedBandwidthTable = new SavedBandwidthTable(
                            savings, BraveStatsUtil.getCalculatedDate("yyyy-MM-dd", 0));
                    long rowId = mDatabaseHelper.insertSavedBandwidth(savedBandwidthTable);
                } catch (Exception e) {
                    // Do nothing if url is invalid.
                    // Just return w/o showing shields popup.
                    return null;
                }
                return null;
            }
            @Override
            protected void onPostExecute(Void result) {
                assert ThreadUtils.runningOnUiThread();
                if (isCancelled()) return;
            }
        }.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    private void addStatsToDb(String statType, String statSite, String url) {
        new AsyncTask<Void>() {
            @Override
            protected Void doInBackground() {
                try {
                    URL urlObject = new URL(url);
                    URL siteObject = new URL(statSite);
                    BraveStatsTable braveStatsTable = new BraveStatsTable(url, urlObject.getHost(),
                            statType, statSite, siteObject.getHost(),
                            BraveStatsUtil.getCalculatedDate("yyyy-MM-dd", 0));
                    long rowId = mDatabaseHelper.insertStats(braveStatsTable);
                } catch (Exception e) {
                    // Do nothing if url is invalid.
                    // Just return w/o showing shields popup.
                    return null;
                }
                return null;
            }
            @Override
            protected void onPostExecute(Void result) {
                assert ThreadUtils.runningOnUiThread();
                if (isCancelled()) return;
            }
        }.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR);
    }

    public void showWalletIcon(boolean show) {
        assert mWalletLayout != null;
        if (mWalletLayout == null) {
            return;
        }
        Tab currentTab = getToolbarDataProvider().getTab();
        if (currentTab == null) {
            return;
        }
        if (show) {
            mWalletLayout.setVisibility(View.VISIBLE);
            mTabsWithWalletIcon.add(currentTab.getId());
        } else {
            mWalletLayout.setVisibility(View.GONE);
            mTabsWithWalletIcon.remove(currentTab.getId());
        }
    }

    public void hideRewardsOnboardingIcon() {
        if (mBraveRewardsOnboardingIcon != null) {
            mBraveRewardsOnboardingIcon.setVisibility(View.GONE);
        }
        if (mBraveRewardsNotificationsCount != null) {
            mBraveRewardsNotificationsCount.setVisibility(View.GONE);
        }
        SharedPreferences sharedPref = ContextUtils.getAppSharedPreferences();
        SharedPreferences.Editor editor = sharedPref.edit();
        editor.putBoolean(BraveRewardsPanelPopup.PREF_WAS_TOOLBAR_BAT_LOGO_BUTTON_PRESSED, true);
        editor.apply();
    }

    @Override
    public void onClickImpl(View v) {
        if (mBraveShieldsHandler == null) {
            assert false;
            return;
        }
        if (mBraveShieldsButton == v && mBraveShieldsButton != null) {
            showShieldsMenu(mBraveShieldsButton);
        } else if (mBraveRewardsButton == v && mBraveRewardsButton != null) {
            // Context context = getContext();
            // if (checkForRewardsOnboarding()) {
            //   OnboardingPrefManager.getInstance().showOnboarding(context);
            //   hideRewardsOnboardingIcon();
            // } else {
            //   if (null != mRewardsPopup) {
            //     return;
            //   }
            //   mRewardsPopup = new BraveRewardsPanelPopup(v);
            //   mRewardsPopup.showLikePopDownMenu();
            // }
            if (null != mRewardsPopup) {
                return;
            }
            hideRewardsOnboardingIcon();
            OnboardingPrefManager.getInstance().setOnboardingShown(true);
            mRewardsPopup = new BraveRewardsPanel(v);
            mRewardsPopup.showLikePopDownMenu();
            if (mBraveRewardsNotificationsCount.isShown()) {
                SharedPreferences sharedPref = ContextUtils.getAppSharedPreferences();
                SharedPreferences.Editor editor = sharedPref.edit();
                editor.putBoolean(
                        BraveRewardsPanelPopup.PREF_WAS_TOOLBAR_BAT_LOGO_BUTTON_PRESSED, true);
                editor.apply();
                mBraveRewardsNotificationsCount.setVisibility(View.INVISIBLE);
                mIsInitialNotificationPosted = false;
            }
        } else if (mHomeButton == v) {
            // Helps Brave News know how to behave on home button action
            BraveActivity.getBraveActivity().setComesFromNewTab(true);
        } else if (mBraveWalletButton == v && mBraveWalletButton != null) {
            maybeShowWalletPanel(v);
        }
    }

    private void maybeShowWalletPanel(View v) {
        BraveActivity activity = BraveActivity.getBraveActivity();
        assert activity != null;
        if (activity == null) {
            return;
        }
        activity.showWalletPanel(true);
    }

    private void showWalletPanelInternal(View v) {
        mDAppsWalletController =
                new DAppsWalletController(getContext(), v, dialog -> mDAppsWalletController = null);
        mDAppsWalletController.showWalletPanel();
    }

    public void showWalletPanel() {
        dismissWalletPanelOrDialog();
        showWalletPanelInternal(this);
    }

    @Override
    public void onClick(View v) {
        onClickImpl(v);
    }

    private boolean checkForRewardsOnboarding() {
        return PackageUtils.isFirstInstall(getContext())
                && !BraveAdsNativeHelper.nativeIsBraveAdsEnabled(
                        Profile.getLastUsedRegularProfile())
                && ChromeFeatureList.isEnabled(BraveFeatureList.BRAVE_REWARDS)
                && !OnboardingPrefManager.getInstance().isOnboardingShown();
    }

    private void showShieldsMenu(View mBraveShieldsButton) {
        Tab currentTab = getToolbarDataProvider().getTab();
        if (currentTab == null) {
            return;
        }
        try {
            URL url = new URL(currentTab.getUrl().getSpec());
            // Don't show shields popup if protocol is not valid for shields.
            if (!isValidProtocolForShields(url.getProtocol())) {
                return;
            }
            mBraveShieldsHandler.show(mBraveShieldsButton, currentTab);
        } catch (Exception e) {
            // Do nothing if url is invalid.
            // Just return w/o showing shields popup.
            return;
        }
    }

    @Override
    public boolean onLongClickImpl(View v) {
        // Use null as the default description since Toast.showAnchoredToast
        // will return false if it is null.
        String description = null;
        Context context = getContext();
        Resources resources = context.getResources();

        if (v == mBraveShieldsButton) {
            description = resources.getString(R.string.accessibility_toolbar_btn_brave_shields);
        } else if (v == mBraveRewardsButton) {
            description = resources.getString(R.string.accessibility_toolbar_btn_brave_rewards);
        } else if (v == mHomeButton) {
            description = resources.getString(R.string.accessibility_toolbar_btn_home);
        } else if (v == mBraveWalletButton) {
            description = resources.getString(R.string.accessibility_toolbar_btn_brave_wallet);
        }

        return Toast.showAnchoredToast(context, v, description);
    }

    @Override
    public boolean onLongClick(View v) {
        return onLongClickImpl(v);
    }

    @Override
    public void onUrlFocusChange(boolean hasFocus) {
        Context context = getContext();
        String countryCode = Locale.getDefault().getCountry();
        if (hasFocus && PackageUtils.isFirstInstall(context)
                && BraveActivity.getBraveActivity() != null
                && BraveActivity.getBraveActivity().getActivityTab() != null
                && UrlUtilities.isNTPUrl(
                        BraveActivity.getBraveActivity().getActivityTab().getUrl().getSpec())
                && !OnboardingPrefManager.getInstance().hasSearchEngineOnboardingShown()
                && !mBraveSearchEngineDefaultRegions.contains(countryCode)) {
            Intent searchActivityIntent = new Intent(context, SearchActivity.class);
            context.startActivity(searchActivityIntent);
        }
        super.onUrlFocusChange(hasFocus);
    }

    @Override
    public void populateUrlAnimatorSetImpl(boolean showExpandedState,
            int urlFocusToolbarButtonsDuration, int urlClearFocusTabStackDelayMs,
            List<Animator> animators) {
        if (mBraveShieldsButton != null) {
            Animator animator;
            if (showExpandedState) {
                float density = getContext().getResources().getDisplayMetrics().density;
                boolean isRtl = getLayoutDirection() == LAYOUT_DIRECTION_RTL;
                float toolbarButtonTranslationX =
                        MathUtils.flipSignIf(URL_FOCUS_TOOLBAR_BUTTONS_TRANSLATION_X_DP, isRtl)
                        * density;
                animator = ObjectAnimator.ofFloat(
                        mBraveShieldsButton, TRANSLATION_X, toolbarButtonTranslationX);
                animator.setDuration(urlFocusToolbarButtonsDuration);
                animator.setInterpolator(BakedBezierInterpolator.FADE_OUT_CURVE);
                animators.add(animator);

                animator = ObjectAnimator.ofFloat(mBraveShieldsButton, ALPHA, 0);
                animator.setDuration(urlFocusToolbarButtonsDuration);
                animator.setInterpolator(BakedBezierInterpolator.FADE_OUT_CURVE);
                animators.add(animator);
            } else {
                animator = ObjectAnimator.ofFloat(mBraveShieldsButton, TRANSLATION_X, 0);
                animator.setDuration(urlFocusToolbarButtonsDuration);
                animator.setStartDelay(urlClearFocusTabStackDelayMs);
                animator.setInterpolator(BakedBezierInterpolator.TRANSFORM_CURVE);
                animators.add(animator);

                animator = ObjectAnimator.ofFloat(mBraveShieldsButton, ALPHA, 1);
                animator.setDuration(urlFocusToolbarButtonsDuration);
                animator.setStartDelay(urlClearFocusTabStackDelayMs);
                animator.setInterpolator(BakedBezierInterpolator.TRANSFORM_CURVE);
                animators.add(animator);
            }
        }
    }

    @Override
    public void updateModernLocationBarColorImpl(int color) {
        if (mShieldsLayout != null && mShieldsLayoutIsColorBackground) {
            mShieldsLayout.setBackgroundColor(
                    ChromeColors.getDefaultThemeColor(getContext(), isIncognito()));
        }
        mCurrentToolbarColor = color;
        if (mShieldsLayout != null) {
            mShieldsLayout.getBackground().setColorFilter(color, PorterDuff.Mode.SRC_IN);
        }
        if (mRewardsLayout != null) {
            mRewardsLayout.getBackground().setColorFilter(color, PorterDuff.Mode.SRC_IN);
        }
        if (mWalletLayout != null) {
            mWalletLayout.getBackground().setColorFilter(color, PorterDuff.Mode.SRC_IN);
        }
    }

    @Override
    public int getBoundsAfterAccountingForRightButtonsImpl(ViewGroup toolbarButtonsContainer) {
        if (toolbarButtonsContainer == null || mShieldsLayout == null) {
            assert false;
            return 0;
        }
        ViewGroup.MarginLayoutParams params =
                (ViewGroup.MarginLayoutParams) toolbarButtonsContainer.getLayoutParams();

        int rewardsLen = (mRewardsLayout == null || mRewardsLayout.getVisibility() == View.GONE)
                ? 0
                : mRewardsLayout.getWidth();
        int walletLen = (mWalletLayout == null || mWalletLayout.getVisibility() == View.GONE)
                ? 0
                : mWalletLayout.getWidth();

        return toolbarButtonsContainer.getMeasuredWidth() - mShieldsLayout.getWidth() - rewardsLen
                - walletLen + params.getMarginEnd();
    }

    /**
     * If |tab| is null, set disabled image to shields button and |urlString| is
     * ignored.
     * If |urlString| is null, url is fetched from |tab|.
     */
    private void updateBraveShieldsButtonState(Tab tab) {
        if (mBraveShieldsButton == null) {
            assert false;
            return;
        }

        if (tab == null) {
            mBraveShieldsButton.setImageResource(R.drawable.btn_brave_off);
            return;
        }
        mBraveShieldsButton.setImageResource(
                isShieldsOnForTab(tab) ? R.drawable.btn_brave : R.drawable.btn_brave_off);

        SharedPreferences sharedPreferences = ContextUtils.getAppSharedPreferences();

        if (mRewardsLayout == null) return;
        if (isIncognito()) {
            mRewardsLayout.setVisibility(View.GONE);
            updateShieldsLayoutBackground(true);
        } else if (isNativeLibraryReady()
                && ChromeFeatureList.isEnabled(BraveFeatureList.BRAVE_REWARDS)
                && !BravePrefServiceBridge.getInstance().getSafetynetCheckFailed()
                && sharedPreferences.getBoolean(
                        AppearancePreferences.PREF_SHOW_BRAVE_REWARDS_ICON, true)) {
            mRewardsLayout.setVisibility(View.VISIBLE);
            updateShieldsLayoutBackground(false);
        }
    }

    private boolean isShieldsOnForTab(Tab tab) {
        if (tab == null) {
            assert false;
            return false;
        }
        return BraveShieldsContentSettings.getShields(
                Profile.fromWebContents(((TabImpl) tab).getWebContents()), tab.getUrl().getSpec(),
                BraveShieldsContentSettings.RESOURCE_IDENTIFIER_BRAVE_SHIELDS);
    }

    private boolean isValidProtocolForShields(String protocol) {
        if (protocol.equals("http") || protocol.equals("https")) {
            return true;
        }

        return false;
    }

    public void dismissRewardsPanel() {
        if (mRewardsPopup != null) {
            mRewardsPopup.dismiss();
            mRewardsPopup = null;
        }
    }

    public void dismissWalletPanelOrDialog() {
        if (mDAppsWalletController != null) {
            mDAppsWalletController.dismiss();
            mDAppsWalletController = null;
        }
    }

    public void onRewardsPanelDismiss() {
        mRewardsPopup = null;
    }

    public void openRewardsPanel() {
        onClick(mBraveRewardsButton);
    }

    public void openBraveTalkOptInPopup(BraveTalkOptInPopupListener popupListener) {
        if (mRewardsPopup != null) {
            dismissRewardsPanel();
        }
        dismissWalletPanelOrDialog();

        if (mBraveTalkOptInPopup != null) {
            mBraveTalkOptInPopup.dismissPopup();
        }

        mBraveTalkOptInPopup = new BraveTalkOptInPopup(mBraveRewardsButton, popupListener);
        mBraveTalkOptInPopup.showLikePopDownMenu();
    }

    public void onBraveTalkOptInPopupDismiss() {
        mBraveTalkOptInPopup = null;
    }

    public void closeBraveTalkOptInPopup() {
        if (mBraveTalkOptInPopup != null) {
            mBraveTalkOptInPopup.dismissPopup();
            mBraveTalkOptInPopup = null;
        }
    }

    public boolean isShieldsTooltipShown() {
        if (mShieldsPopupWindowTooltip != null) {
            return mShieldsPopupWindowTooltip.isShowing();
        }
        return false;
    }

    @Override
    public void OnNotificationAdded(String id, int type, long timestamp, String[] args) {
        if (mBraveRewardsNativeWorker == null) {
            return;
        }

        if (type == BraveRewardsNativeWorker.REWARDS_NOTIFICATION_BACKUP_WALLET) {
            mBraveRewardsNativeWorker.DeleteNotification(id);
        } else if (type == BraveRewardsNativeWorker.REWARDS_NOTIFICATION_GRANT) {
            // Set flag
            SharedPreferences sharedPreferences = ContextUtils.getAppSharedPreferences();
            SharedPreferences.Editor sharedPreferencesEditor = sharedPreferences.edit();
            sharedPreferencesEditor.putBoolean(
                    BraveRewardsPanelPopup.PREF_GRANTS_NOTIFICATION_RECEIVED, true);
            sharedPreferencesEditor.apply();
        }
        mBraveRewardsNativeWorker.GetAllNotifications();
    }

    private boolean mayShowBraveAdsOnboardingDialog() {
        Context context = getContext();

        if (BraveAdsSignupDialog.shouldShowNewUserDialog(context)) {
            BraveAdsSignupDialog.showNewUserDialog(getContext());
            return true;
        } else if (BraveAdsSignupDialog.shouldShowNewUserDialogIfRewardsIsSwitchedOff(context)) {
            BraveAdsSignupDialog.showNewUserDialog(getContext());
            return true;
        } else if (BraveAdsSignupDialog.shouldShowExistingUserDialog(context)) {
            BraveAdsSignupDialog.showExistingUserDialog(getContext());
            return true;
        }

        return false;
    }

    @Override
    public void OnNotificationsCount(int count) {
        if (mBraveRewardsNotificationsCount != null) {
            if (count != 0) {
                String value = Integer.toString(count);
                if (count > 99) {
                    mBraveRewardsNotificationsCount.setBackground(
                            ResourcesCompat.getDrawable(getContext().getResources(),
                                    R.drawable.brave_rewards_rectangle, /* theme= */ null));
                    value = "99+";
                } else {
                    mBraveRewardsNotificationsCount.setBackground(
                            ResourcesCompat.getDrawable(getContext().getResources(),
                                    R.drawable.brave_rewards_circle, /* theme= */ null));
                }
                mBraveRewardsNotificationsCount.setText(value);
                mBraveRewardsNotificationsCount.setVisibility(View.VISIBLE);
                mIsNotificationPosted = true;
            } else {
                mBraveRewardsNotificationsCount.setText("");
                mBraveRewardsNotificationsCount.setBackgroundResource(0);
                mBraveRewardsNotificationsCount.setVisibility(View.INVISIBLE);
                mIsNotificationPosted = false;
                updateVerifiedPublisherMark();
            }
        }

        updateNotificationBadgeForNewInstall();
        if (!PackageUtils.isFirstInstall(getContext())
                && !OnboardingPrefManager.getInstance().isAdsAvailable()) {
            mayShowBraveAdsOnboardingDialog();
        }

        if (checkForRewardsOnboarding()) {
            if (mBraveRewardsOnboardingIcon != null) {
                mBraveRewardsOnboardingIcon.setVisibility(View.VISIBLE);
            }
            if (mBraveRewardsNotificationsCount != null) {
                mBraveRewardsNotificationsCount.setVisibility(View.GONE);
            }
        }
    }

    private void updateNotificationBadgeForNewInstall() {
        SharedPreferences sharedPref = ContextUtils.getAppSharedPreferences();
        boolean shownBefore = sharedPref.getBoolean(
                BraveRewardsPanelPopup.PREF_WAS_TOOLBAR_BAT_LOGO_BUTTON_PRESSED, false);
        boolean shouldShow = mBraveRewardsNotificationsCount != null && !shownBefore;
        mIsInitialNotificationPosted = shouldShow; // initial notification

        if (!shouldShow) return;

        mBraveRewardsNotificationsCount.setText("");
        mBraveRewardsNotificationsCount.setBackground(ResourcesCompat.getDrawable(
                getContext().getResources(), R.drawable.brave_rewards_circle, /* theme= */ null));
        mBraveRewardsNotificationsCount.setVisibility(View.VISIBLE);
    }

    @Override
    public void onThemeColorChanged(int color, boolean shouldAnimate) {
        final int textBoxColor = ThemeUtils.getTextBoxColorForToolbarBackgroundInNonNativePage(
                getContext(), color, isIncognito());
        updateModernLocationBarColorImpl(textBoxColor);
    }

    /**
     * BraveRewardsNativeWorker.PublisherObserver:
     *   Update a 'verified publisher' checkmark on url bar BAT icon only if
     *   no notifications are posted.
     */
    @Override
    public void onFrontTabPublisherChanged(boolean verified) {
        mIsPublisherVerified = verified;
        updateVerifiedPublisherMark();
    }

    private void updateVerifiedPublisherMark() {
        if (mBraveRewardsNotificationsCount == null) {
            // Most likely we are on a custom page
            return;
        }
        if (mIsInitialNotificationPosted) {
            return;
        } else if (!mIsNotificationPosted) {
            if (mIsPublisherVerified) {
                mBraveRewardsNotificationsCount.setVisibility(View.VISIBLE);
                mBraveRewardsNotificationsCount.setBackground(ResourcesCompat.getDrawable(
                        getContext().getResources(), R.drawable.bat_verified, /* theme= */ null));
            } else {
                mBraveRewardsNotificationsCount.setBackgroundResource(0);
                mBraveRewardsNotificationsCount.setVisibility(View.INVISIBLE);
            }
        }
    }

    public void onBottomToolbarVisibilityChanged(boolean isVisible) {
        mIsBottomToolbarVisible = isVisible;
        if (BraveReflectionUtil.EqualTypes(this.getClass(), ToolbarPhone.class)
                && getMenuButtonCoordinator() != null) {
            getMenuButtonCoordinator().setVisibility(!isVisible);
            ToggleTabStackButton toggleTabStackButton = findViewById(R.id.tab_switcher_button);
            if (toggleTabStackButton != null) {
                toggleTabStackButton.setVisibility(isTabSwitcherOnBottom() ? GONE : VISIBLE);
            }
        }
    }

    private void updateShieldsLayoutBackground(boolean rounded) {
        if (!BraveReflectionUtil.EqualTypes(this.getClass(), ToolbarTablet.class)
                || (mShieldsLayout == null)) {
            return;
        }

        if (rounded) {
            mShieldsLayout.setBackgroundDrawable(
                    ApiCompatibilityUtils.getDrawable(getContext().getResources(),
                            R.drawable.modern_toolbar_background_grey_end_segment));
            mShieldsLayoutIsColorBackground = false;
        } else {
            mShieldsLayout.setBackgroundColor(
                    ChromeColors.getDefaultThemeColor(getContext(), isIncognito()));
            mShieldsLayoutIsColorBackground = true;
        }
        mWalletLayout.setBackgroundColor(ChromeColors.getDefaultThemeColor(getContext(), false));
        updateModernLocationBarColorImpl(mCurrentToolbarColor);
    }

    private boolean isTabSwitcherOnBottom() {
        return mIsBottomToolbarVisible && BottomToolbarVariationManager.isTabSwitcherOnBottom();
    }

    private boolean isMenuButtonOnBottom() {
        return mIsBottomToolbarVisible && BottomToolbarVariationManager.isMenuButtonOnBottom();
    }

    @Override
    protected void initialize(ToolbarDataProvider toolbarDataProvider,
            ToolbarTabController tabController, MenuButtonCoordinator menuButtonCoordinator,
            ObservableSupplier<Boolean> isProgressBarVisibleSupplier,
            HistoryDelegate historyDelegate, BooleanSupplier partnerHomepageEnabledSupplier,
            OfflineDownloader offlineDownloader) {
        super.initialize(toolbarDataProvider, tabController, menuButtonCoordinator,
                isProgressBarVisibleSupplier, historyDelegate, partnerHomepageEnabledSupplier,
                offlineDownloader);
        BraveMenuButtonCoordinator.setMenuFromBottom(isMenuButtonOnBottom());
    }

    public void updateWalletBadgeVisibility(boolean visible) {
        assert mBraveWalletBadge!=null;
        mBraveWalletBadge.setVisibility(visible ? View.VISIBLE : View.GONE);
    }

    public void updateMenuButtonState() {
        BraveMenuButtonCoordinator.setMenuFromBottom(mIsBottomToolbarVisible);
    }

    @Override
    protected void onDraw(Canvas canvas) {
        if (BraveReflectionUtil.EqualTypes(this.getClass(), CustomTabToolbar.class)
                || BraveReflectionUtil.EqualTypes(this.getClass(), ToolbarPhone.class)) {
            updateMenuButtonState();
            Tab tab = getToolbarDataProvider() != null ? getToolbarDataProvider().getTab() : null;
            if (tab != null && ((TabImpl) tab).getWebContents() != null) {
                updateBraveShieldsButtonState(tab);
            }
        }
        super.onDraw(canvas);
    }

    @Override
    public boolean isLocationBarValid(LocationBarCoordinator locationBar) {
        return locationBar != null && locationBar.getPhoneCoordinator() != null
                && locationBar.getPhoneCoordinator().getViewForDrawing() != null;
    }
}
