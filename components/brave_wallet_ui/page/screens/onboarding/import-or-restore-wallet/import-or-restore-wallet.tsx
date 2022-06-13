// Copyright (c) 2022 The Brave Authors. All rights reserved.
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.

import * as React from 'react'
import { useDispatch, useSelector } from 'react-redux'

// utils
import { getLocale } from '../../../../../common/locale'

// types
import { PageState, WalletRoutes } from '../../../../constants/types'

// actions
import { WalletPageActions } from '../../../actions'

// images
import ImportFromMetaMaskSvg from '../../../../assets/svg-icons/onboarding/import-from-metamask.svg'
import BraveWalletSvg from '../../../../assets/svg-icons/onboarding/brave-wallet.svg'

// components
import { WalletPageLayout } from '../../../../components/desktop'
import { StepsNavigation } from '../../../../components/desktop/steps-navigation/steps-navigation'

// styles
import { WalletLink } from '../../../../components/shared/style'

import {
  StyledWrapper,
  Title,
  Description,
  MainWrapper
} from '../onboarding.style'

import {
 CardButton,
 LinkRow
} from './import-or-restore-wallet.style'

export const OnboardingImportOrRestoreWallet = () => {
  // redux
  const dispatch = useDispatch()
  const importWalletError = useSelector(({ page }: { page: PageState }) => page.importWalletError)

  // effects
  React.useEffect(() => {
    // reset any pending import errors
    if (importWalletError?.hasError) {
      dispatch(WalletPageActions.setImportWalletError({
        hasError: false
      }))
    }
  }, [importWalletError?.hasError])

  // render
  return (
    <WalletPageLayout>
      <MainWrapper>
        <StyledWrapper>

          <StepsNavigation
            goBackUrl={WalletRoutes.Onboarding}
            currentStep={''}
            steps={[]}
          />

          <div>
            <Title>
              {getLocale('braveWalletImportOrRestoreWalletTitle')}
            </Title>
            <Description>
              {getLocale('braveWalletImportOrRestoreDescription')}
            </Description>
          </div>

          <CardButton
            to={WalletRoutes.OnboardingRestoreWallet}
          >
            <p>
              {getLocale('braveWalletRestoreMyBraveWallet')}
            </p>

            <img src={BraveWalletSvg} height="50px" />
          </CardButton>

          <CardButton
            to={WalletRoutes.OnboardingImportMetaMask}
          >
            <p>
              {getLocale('braveWalletImportFromMetaMask')}
            </p>

            <img src={ImportFromMetaMaskSvg} height="50px" />
          </CardButton>

          <LinkRow>
            <WalletLink
              to={WalletRoutes.OnboardingCreatePassword}
            >
              {getLocale('braveWalletCreateWalletInsteadLink')}
            </WalletLink>
          </LinkRow>

        </StyledWrapper>
      </MainWrapper>
    </WalletPageLayout>
  )
}

export default OnboardingImportOrRestoreWallet
