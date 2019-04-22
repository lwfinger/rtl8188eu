/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#ifndef _RTL8188E_SRESET_H_
#define _RTL8188E_SRESET_H_

#include <rtw_sreset.h>

#ifdef DBG_CONFIG_ERROR_DETECT
	extern void rtl8188e_sreset_xmit_status_check(_adapter *padapter);
	extern void rtl8188e_sreset_linked_status_check(_adapter *padapter);
#endif
#endif
