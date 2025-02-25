/* This Source Code Form is subject to the terms of the Mozilla Public
 * License. v. 2.0. If a copy of the MPL was not distributed with this file.
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import styled from 'styled-components'

export const TourPromoWrapper = styled('div')<{}>`
  margin-top: 30px;
`

export const PageWalletWrapper = styled.div`
  width: 373px;

  @media screen and (max-width: 800px) {
    width: auto;
    margin-bottom: 10px;
  }
`

export const StyledListContent = styled('div')<{}>`
  padding: 0 25px;
`

export const StyledSitesNum = styled('div')<{}>`
  height: 50px;
  padding: 20px 25px;
  margin-top: -21px;
`

export const StyledDisabledContent = styled('div')<{}>`
  padding: 0px 5px;
`

export const StyledHeading = styled('span')<{}>`
  font-size: 22px;
  font-weight: normal;
  letter-spacing: 0;
  line-height: 28px;
`

export const StyledSitesLink = styled('a')<{}>`
  float: right;
  color: #4C54D2;
  font-size: 13px;
  letter-spacing: 0;
`

export const StyledText = styled('p')<{}>`
  color: #838391;
  font-size: 14px;
  font-family: ${p => p.theme.fontFamily.body};
  font-weight: 300;
  letter-spacing: 0;
  line-height: 28px;
`

export const StyledTotalContent = styled('div')<{}>`
  position: relative;
  padding-right: 25px;
  text-align: right;

  @media (max-width: 366px) {
    top: 11px;
  }
`

export const StyledWalletOverlay = styled('div')<{}>`
  display: flex;
  position: fixed;
  top: 0;
  left: 0;
  width: 100%;
  height: 100%;
  background: ${p => p.theme.color.modalOverlayBackground};
  align-items: center;
  z-index: 999;
  justify-content: center;
`

export const StyledWalletWrapper = styled('div')<{}>`
  height: 90vh;
  overflow-y: scroll;
  width: 90%;
  position: absolute;
  top: 50px;

  @media (max-height: 515px) {
    max-height: 415px;
    overflow-y: scroll;
  }
`

export const StyledWalletClose = styled('div')<{}>`
  top: 15px;
  right: 15px;
  position: fixed;
  color: ${p => p.theme.color.subtleExclude};
  width: 25px;
`

export const StyledArrivingSoon = styled.div`
  margin-bottom: 8px;
  color: var(--brave-palette-neutral900);
  font-size: 14px;
  line-height: 24px;

  a {
    color: var(--brave-palette-blurple500);
    font-weight: 600;
    text-decoration: none;
  }

  > div {
    background: #E8F4FF;
    border-radius: 4px;
    padding: 6px 18px;
    display: flex;
    gap: 6px;
    font-weight: 600;
  }

  .rewards-payment-amount {
    .plus {
      margin-right: 2px;
    }
  }

  .rewards-payment-pending {
    background: #E8F4FF;

    .icon {
      color: var(--brave-palette-blue500);
      height: 16px;
      width: auto;
      vertical-align: middle;
      margin-bottom: 1px;
    }
  }

  .rewards-payment-completed {
    background: #E7FDEA;
    align-items: center;

    .icon {
      height: 20px;
      width: auto;
      vertical-align: middle;
    }
  }

  .rewards-payment-check-status {
    display: block;
  }
`

export const StyledNeedsBrowserUpdateView = styled.div`
  display: flex;
  align-items: center;
  justify-content: start;
  background: #FDF1F2;
  padding: 5px;
  border-radius: 4px 4px 0px 0px;
`

export const StyledNeedsBrowserUpdateIcon = styled.div`
  width: 30px;
  height: 30px;
  margin: 8px;
  color: ${p => p.theme.palette.red500};
`

export const StyledNeedsBrowserUpdateContent = styled.div`
  display: block;
`

export const StyledNeedsBrowserUpdateContentHeader = styled.div`
  margin: 5px;
  font-size: 15px;
  font-weight: 600;
  color: var(--brave-palette-neutral800);
`

export const StyledNeedsBrowserUpdateContentBody = styled.div`
  margin: 5px;
  font-size: 14px;
  color: var(--brave-palette-neutral800);
`
