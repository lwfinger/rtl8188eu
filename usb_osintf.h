/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#ifndef __USB_OSINTF_H
#define __USB_OSINTF_H

#include <usb_vendor_req.h>

#define USBD_HALTED(Status) ((u32)(Status) >> 30 == 3)


u8 usbvendorrequest(struct dvobj_priv *pdvobjpriv, RT_USB_BREQUEST brequest, RT_USB_WVALUE wvalue, u8 windex, void *data, u8 datalen, u8 isdirectionin);
void nat25_db_expire(_adapter *priv);
int nat25_db_handle(_adapter *priv, struct sk_buff *skb, int method);
int nat25_handle_frame(_adapter *priv, struct sk_buff *skb);

#endif
