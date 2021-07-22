/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#ifndef __INC_ODM_REGCONFIG_H_8188E
#define __INC_ODM_REGCONFIG_H_8188E

#if (RTL8188E_SUPPORT == 1)

void
odm_config_rf_reg_8188e(
	struct PHY_DM_STRUCT				*p_dm_odm,
	u32					addr,
	u32					data,
	enum odm_rf_radio_path_e     RF_PATH,
	u32				    reg_addr
);

void
odm_config_rf_radio_a_8188e(
	struct PHY_DM_STRUCT				*p_dm_odm,
	u32					addr,
	u32					data
);

void
odm_config_rf_radio_b_8188e(
	struct PHY_DM_STRUCT				*p_dm_odm,
	u32					addr,
	u32					data
);

void
odm_config_mac_8188e(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u32		addr,
	u8		data
);

void
odm_config_bb_agc_8188e(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u32		addr,
	u32		bitmask,
	u32		data
);

void
odm_config_bb_phy_reg_pg_8188e(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u32		band,
	u32		rf_path,
	u32		tx_num,
	u32		addr,
	u32		bitmask,
	u32		data
);

void
odm_config_bb_phy_8188e(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u32		addr,
	u32		bitmask,
	u32		data
);

void
odm_config_bb_txpwr_lmt_8188e(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u8		*regulation,
	u8		*band,
	u8		*bandwidth,
	u8		*rate_section,
	u8		*rf_path,
	u8	*channel,
	u8		*power_limit
);

#endif
#endif /* end of SUPPORT */
