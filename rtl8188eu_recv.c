// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#define _RTL8188EU_RECV_C_

#include <drv_types.h>
#include <rtl8188e_hal.h>

int	rtl8188eu_init_recv_priv(_adapter *padapter)
{
	return usb_init_recv_priv(padapter, INTERRUPT_MSG_FORMAT_LEN);
}

void rtl8188eu_free_recv_priv(_adapter *padapter)
{
	usb_free_recv_priv(padapter, INTERRUPT_MSG_FORMAT_LEN);
}
