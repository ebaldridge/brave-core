/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

import * as mojom from 'gen/brave/components/brave_rewards/common/brave_rewards_panel.mojom.m.js'

let instance: RewardsPanelProxy|null = null

export class RewardsPanelProxy {
  handler: mojom.PanelHandlerRemote
  callbackRouter: mojom.PanelCallbackRouter

  constructor (
      handler: mojom.PanelHandlerRemote,
      callbackRouter: mojom.PanelCallbackRouter) {
    this.handler = handler
    this.callbackRouter = callbackRouter
  }

  static getInstance (): RewardsPanelProxy {
    if (!instance) {
      const handler = new mojom.PanelHandlerRemote()
      const callbackRouter = new mojom.PanelCallbackRouter()
      mojom.PanelHandlerFactory.getRemote().createPanelHandler(
        handler.$.bindNewPipeAndPassReceiver(),
        callbackRouter.$.bindNewPipeAndPassRemote())
      instance = new RewardsPanelProxy(handler, callbackRouter)
    }
    return instance
  }
}
