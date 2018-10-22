/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/

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

#if RTL8188E_SUPPORT == 1
	#define RTL8188E_T_SUPPORT 1
	#ifdef CONFIG_SFW_SUPPORTED
		#define RTL8188E_S_SUPPORT 1
	#else
		#define RTL8188E_S_SUPPORT 0
	#endif
#endif

#if (RTL8188E_SUPPORT == 1)
	#include "hal8188erateadaptive.h" /* for  RA,Power training */
	#include "halhwimg8188e_mac.h"
	#include "halhwimg8188e_rf.h"
	#include "halhwimg8188e_bb.h"
	#include "halhwimg8188e_t_fw.h"
	#include "halhwimg8188e_s_fw.h"
	#include "phydm_regconfig8188e.h"
	#include "phydm_rtl8188e.h"
	#include "hal8188ereg.h"
	#include "version_rtl8188e.h"
	#include "rtl8188e_hal.h"
	#include "halphyrf_8188e_ce.h"
#endif /* 88E END */

#if (RTL8192E_SUPPORT == 1)
	#include "rtl8192e/halphyrf_8192e_ce.h" /*FOR_8192E_IQK*/

	#include "rtl8192e/phydm_rtl8192e.h" /* FOR_8192E_IQK */
	#include "rtl8192e/version_rtl8192e.h"
	#include "rtl8192e/halhwimg8192e_bb.h"
	#include "rtl8192e/halhwimg8192e_mac.h"
	#include "rtl8192e/halhwimg8192e_rf.h"
	#include "rtl8192e/phydm_regconfig8192e.h"
	#include "rtl8192e/halhwimg8192e_fw.h"
	#include "rtl8192e/hal8192ereg.h"
	#include "rtl8192e_hal.h"
#endif /* 92E END */

#if (RTL8812A_SUPPORT == 1)

	#include "rtl8812a/halphyrf_8812a_ce.h"

	/* #include "rtl8812a/HalPhyRf_8812A.h"  */ /* FOR_8812_IQK */
	#include "rtl8812a/halhwimg8812a_bb.h"
	#include "rtl8812a/halhwimg8812a_mac.h"
	#include "rtl8812a/halhwimg8812a_rf.h"
	#include "rtl8812a/phydm_regconfig8812a.h"
	#include "rtl8812a/halhwimg8812a_fw.h"
	#include "rtl8812a/phydm_rtl8812a.h"

	#include "rtl8812a_hal.h"
	#include "rtl8812a/version_rtl8812a.h"

#endif /* 8812 END */

#if (RTL8814A_SUPPORT == 1)

	#include "rtl8814a/halhwimg8814a_mac.h"
	#include "rtl8814a/halhwimg8814a_rf.h"
	#include "rtl8814a/halhwimg8814a_bb.h"
	#include "rtl8814a/version_rtl8814a.h"
	#include "rtl8814a/phydm_rtl8814a.h"
	#include "rtl8814a/halhwimg8814a_fw.h"
	#include "rtl8814a/halphyrf_8814a_ce.h"
	#include "rtl8814a/phydm_regconfig8814a.h"
	#include "rtl8814a_hal.h"
	#include "rtl8814a/phydm_iqk_8814a.h"
#endif /* 8814 END */

#if (RTL8881A_SUPPORT == 1)/* FOR_8881_IQK */
	#include "rtl8821a/phydm_iqk_8821a_ce.h"
#endif

#if (RTL8723B_SUPPORT == 1)
	#include "rtl8723b/halhwimg8723b_mac.h"
	#include "rtl8723b/halhwimg8723b_rf.h"
	#include "rtl8723b/halhwimg8723b_bb.h"
	#include "rtl8723b/halhwimg8723b_fw.h"
	#include "rtl8723b/phydm_regconfig8723b.h"
	#include "rtl8723b/phydm_rtl8723b.h"
	#include "rtl8723b/hal8723breg.h"
	#include "rtl8723b/version_rtl8723b.h"
	#include "rtl8723b/halphyrf_8723b_ce.h"
	#include "rtl8723b/halhwimg8723b_mp.h"
	#include "rtl8723b_hal.h"
#endif

#if (RTL8821A_SUPPORT == 1)
	#include "rtl8821a/halhwimg8821a_mac.h"
	#include "rtl8821a/halhwimg8821a_rf.h"
	#include "rtl8821a/halhwimg8821a_bb.h"
	#include "rtl8821a/halhwimg8821a_fw.h"
	#include "rtl8821a/phydm_regconfig8821a.h"
	#include "rtl8821a/phydm_rtl8821a.h"
	#include "rtl8821a/version_rtl8821a.h"
	#include "rtl8821a/halphyrf_8821a_ce.h"
	#include "rtl8821a/phydm_iqk_8821a_ce.h"/*for IQK*/
	#include "rtl8812a/halphyrf_8812a_ce.h"/*for IQK,LCK,Power-tracking*/
	#include "rtl8812a_hal.h"
#endif

#if (RTL8822B_SUPPORT == 1)
	#include "rtl8822b/halhwimg8822b_mac.h"
	#include "rtl8822b/halhwimg8822b_rf.h"
	#include "rtl8822b/halhwimg8822b_bb.h"
	#include "rtl8822b/halhwimg8822b_fw.h"
	#include "rtl8822b/phydm_regconfig8822b.h"
	#include "rtl8822b/halphyrf_8822b.h"
	#include "rtl8822b/phydm_rtl8822b.h"
	#include "rtl8822b/phydm_hal_api8822b.h"
	#include "rtl8822b/version_rtl8822b.h"

	#include <hal_data.h>		/* struct HAL_DATA_TYPE */
	#include <rtl8822b_hal.h>	/* RX_SMOOTH_FACTOR, reg definition and etc.*/

#endif

#if (RTL8703B_SUPPORT == 1)
	#include "rtl8703b/phydm_regconfig8703b.h"
	#include "rtl8703b/halhwimg8703b_mac.h"
	#include "rtl8703b/halhwimg8703b_rf.h"
	#include "rtl8703b/halhwimg8703b_bb.h"
	#include "rtl8703b/halhwimg8703b_fw.h"
	#include "rtl8703b/halphyrf_8703b.h"
	#include "rtl8703b/version_rtl8703b.h"
	#include "rtl8703b_hal.h"
#endif

#if (RTL8188F_SUPPORT == 1)
	#include "rtl8188f/halhwimg8188f_mac.h"
	#include "rtl8188f/halhwimg8188f_rf.h"
	#include "rtl8188f/halhwimg8188f_bb.h"
	#include "rtl8188f/halhwimg8188f_fw.h"
	#include "rtl8188f/hal8188freg.h"
	#include "rtl8188f/phydm_rtl8188f.h"
	#include "rtl8188f/phydm_regconfig8188f.h"
	#include "rtl8188f/halphyrf_8188f.h" /* for IQK,LCK,Power-tracking */
	#include "rtl8188f/version_rtl8188f.h"
	#include "rtl8188f_hal.h"
#endif

#if (RTL8723D_SUPPORT == 1)
	#include "rtl8723d/halhwimg8723d_bb.h"
	#include "rtl8723d/halhwimg8723d_mac.h"
	#include "rtl8723d/halhwimg8723d_rf.h"
	#include "rtl8723d/phydm_regconfig8723d.h"
	#include "rtl8723d/halhwimg8723d_fw.h"
	#include "rtl8723d/hal8723dreg.h"
	#include "rtl8723d/phydm_rtl8723d.h"
	#include "rtl8723d/halphyrf_8723d.h"
	#include "rtl8723d/version_rtl8723d.h"
	#include "rtl8723d_hal.h"
#endif /* 8723D End */

#if (RTL8197F_SUPPORT == 1)
	#include "rtl8197f/halhwimg8197f_mac.h"
	#include "rtl8197f/halhwimg8197f_rf.h"
	#include "rtl8197f/halhwimg8197f_bb.h"
	#include "rtl8197f/phydm_hal_api8197f.h"
	#include "rtl8197f/version_rtl8197f.h"
	#include "rtl8197f/phydm_rtl8197f.h"
	#include "rtl8197f/phydm_regconfig8197f.h"
	#include "rtl8197f/halphyrf_8197f.h"
	#include "rtl8197f/phydm_iqk_8197f.h"
#endif

#if (RTL8821C_SUPPORT == 1)
	#include "rtl8821c/phydm_hal_api8821c.h"
	#include "rtl8821c/halhwimg8821c_testchip_mac.h"
	#include "rtl8821c/halhwimg8821c_testchip_rf.h"
	#include "rtl8821c/halhwimg8821c_testchip_bb.h"
	#include "rtl8821c/halhwimg8821c_mac.h"
	#include "rtl8821c/halhwimg8821c_rf.h"
	#include "rtl8821c/halhwimg8821c_bb.h"
	#include "rtl8821c/halhwimg8821c_fw.h"
	#include "rtl8821c/phydm_regconfig8821c.h"
	#include "rtl8821c/halphyrf_8821c.h"
	#include "rtl8821c/version_rtl8821c.h"
	#include "rtl8821c_hal.h"
#endif

#endif /* __ODM_PRECOMP_H__ */
