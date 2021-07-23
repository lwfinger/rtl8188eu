/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#ifndef __RTL8188E_DM_H__
#define __RTL8188E_DM_H__

void rtl8188e_init_dm_priv(PADAPTER Adapter);
void rtl8188e_deinit_dm_priv(PADAPTER Adapter);
void rtl8188e_InitHalDm(PADAPTER Adapter);
void rtl8188e_HalDmWatchDog(PADAPTER Adapter);

/* void rtl8192c_dm_CheckTXPowerTracking(IN PADAPTER Adapter); */

/* void rtl8192c_dm_RF_Saving(IN PADAPTER pAdapter, IN u8 bForceInNormal); */

#endif
