/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#ifndef __HAL_IC_CFG_H__
#define __HAL_IC_CFG_H__

#define RTL8188E_SUPPORT				0
#define RTL8812A_SUPPORT				0
#define RTL8821A_SUPPORT				0
#define RTL8723B_SUPPORT				0
#define RTL8723D_SUPPORT				0
#define RTL8192E_SUPPORT				0
#define RTL8814A_SUPPORT				0
#define RTL8195A_SUPPORT				0
#define RTL8197F_SUPPORT				0
#define RTL8703B_SUPPORT				0
#define RTL8188F_SUPPORT				1
#define RTL8822B_SUPPORT				0
#define RTL8821B_SUPPORT				0
#define RTL8821C_SUPPORT				0

/*#if (RTL8188E_SUPPORT==1)*/
#define RATE_ADAPTIVE_SUPPORT			0
#define POWER_TRAINING_ACTIVE			0

#ifdef CONFIG_MULTIDRV
#endif

#undef RTL8188E_SUPPORT
#undef RATE_ADAPTIVE_SUPPORT
#undef POWER_TRAINING_ACTIVE

#define RTL8188E_SUPPORT				1
#define RATE_ADAPTIVE_SUPPORT			1
#define POWER_TRAINING_ACTIVE			1
#define CONFIG_GET_RAID_BY_DRV

#endif /*__HAL_IC_CFG_H__*/
