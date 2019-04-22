/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#ifndef __RTL8188E_RF_H__
#define __RTL8188E_RF_H__



int	PHY_RF6052_Config8188E(PADAPTER		Adapter);
void		rtl8188e_RF_ChangeTxPath(PADAPTER	Adapter,
		u16		DataRate);
void		rtl8188e_PHY_RF6052SetBandwidth(
	PADAPTER				Adapter,
	CHANNEL_WIDTH		Bandwidth);

#endif/* __RTL8188E_RF_H__ */
