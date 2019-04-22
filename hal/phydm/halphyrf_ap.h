/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


#ifndef __HAL_PHY_RF_H__
#define __HAL_PHY_RF_H__

#include "phydm_powertracking_ap.h"

enum pwrtrack_method {
	BBSWING,
	TXAGC,
	MIX_MODE,
	TSSI_MODE
};

typedef void	(*func_set_pwr)(void *, enum pwrtrack_method, u8, u8);
typedef void(*func_iqk)(void *, u8, u8, u8);
typedef void	(*func_lck)(void *);
/* refine by YuChen for 8814A */
typedef void	(*func_swing)(void *, u8 **, u8 **, u8 **, u8 **);
typedef void	(*func_swing8814only)(void *, u8 **, u8 **, u8 **, u8 **);
typedef void	(*func_all_swing)(void *, u8 **, u8 **, u8 **, u8 **, u8 **, u8 **, u8 **, u8 **);


struct _TXPWRTRACK_CFG {
	u8		swing_table_size_cck;
	u8		swing_table_size_ofdm;
	u8		threshold_iqk;
	u8		threshold_dpk;
	u8		average_thermal_num;
	u8		rf_path_count;
	u32		thermal_reg_addr;
	func_set_pwr	odm_tx_pwr_track_set_pwr;
	func_iqk	do_iqk;
	func_lck		phy_lc_calibrate;
	func_swing	get_delta_swing_table;
	func_swing8814only	get_delta_swing_table8814only;
	func_all_swing	get_delta_all_swing_table;
};

void
configure_txpower_track(
	void		*p_dm_void,
	struct _TXPWRTRACK_CFG	*p_config
);

void
odm_txpowertracking_callback_thermal_meter(
	struct _ADAPTER	*adapter
);

#if ODM_IC_11AC_SERIES_SUPPORT
void
odm_txpowertracking_callback_thermal_meter_jaguar_series(
	struct _ADAPTER	*adapter
);

#define IS_CCK_RATE(_rate)				(ODM_MGN_1M == _rate || _rate == ODM_MGN_2M || _rate == ODM_MGN_5_5M || _rate == ODM_MGN_11M)


#define ODM_TARGET_CHNL_NUM_2G_5G	59

void
odm_reset_iqk_result(
	void		*p_dm_void
);
u8
odm_get_right_chnl_place_for_iqk(
	u8 chnl
);

void phydm_rf_init(void		*p_dm_void);
void phydm_rf_watchdog(void		*p_dm_void);

#endif	/*  #ifndef __HAL_PHY_RF_H__ */
