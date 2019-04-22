/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


#ifndef	__ODM_PRECOMP_H__
#define __ODM_PRECOMP_H__

#include "phydm_types.h"

#define		TEST_FALG___		1

/* 2 Config Flags and Structs - defined by each ODM type */

#define __PACK
#define __WLAN_ATTRIB_PACK__

/* 2 OutSrc Header Files */

#include "phydm.h"
#include "phydm_hwconfig.h"
#include "phydm_debug.h"
#include "phydm_regdefine11ac.h"
#include "phydm_regdefine11n.h"
#include "phydm_interface.h"
#include "phydm_reg.h"

#include "phydm_adc_sampling.h"


void
phy_set_tx_power_limit(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u8	*regulation,
	u8	*band,
	u8	*bandwidth,
	u8	*rate_section,
	u8	*rf_path,
	u8	*channel,
	u8	*power_limit
);

#define RTL8188E_T_SUPPORT 1
#ifdef CONFIG_SFW_SUPPORTED
	#define RTL8188E_S_SUPPORT 1
#else
	#define RTL8188E_S_SUPPORT 0
#endif

#include "hal8188erateadaptive.h" /* for  RA,Power training */
#include "halhwimg8188e_mac.h"
#include "halhwimg8188e_rf.h"
#include "halhwimg8188e_bb.h"
#include "phydm_regconfig8188e.h"
#include "phydm_rtl8188e.h"
#include "hal8188ereg.h"
#include "version_rtl8188e.h"
#include "rtl8188e_hal.h"
#include "halphyrf_8188e_ce.h"

#endif /* __ODM_PRECOMP_H__ */
