/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#ifndef __USB_HAL_H__
#define __USB_HAL_H__

int usb_init_recv_priv(_adapter *padapter, u16 ini_in_buf_sz);
void usb_free_recv_priv(_adapter *padapter, u16 ini_in_buf_sz);
#ifdef CONFIG_FW_C2H_REG
void usb_c2h_hisr_hdl(_adapter *adapter, u8 *buf);
#endif

u8 rtw_set_hal_ops(_adapter *padapter);

void rtl8188eu_set_hal_ops(_adapter *padapter);

#ifdef CONFIG_INTEL_PROXIM
extern _adapter  *rtw_usb_get_sw_pointer(void);
#endif /* CONFIG_INTEL_PROXIM */

#endif /* __USB_HAL_H__ */
