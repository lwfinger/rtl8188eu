// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


/* ************************************************************
 * include files
 * ************************************************************ */
#include "mp_precomp.h"
#include "phydm_precomp.h"


u8
odm_get_auto_channel_select_result(
	void			*p_dm_void,
	u8			band
)
{
	struct PHY_DM_STRUCT				*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ACS_					*p_acs = &p_dm_odm->dm_acs;

	if (band == ODM_BAND_2_4G) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("[struct _ACS_] odm_get_auto_channel_select_result(): clean_channel_2g(%d)\n", p_acs->clean_channel_2g));
		return (u8)p_acs->clean_channel_2g;
	} else {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("[struct _ACS_] odm_get_auto_channel_select_result(): clean_channel_5g(%d)\n", p_acs->clean_channel_5g));
		return (u8)p_acs->clean_channel_5g;
	}
}

static void
odm_auto_channel_select_setting(
	void			*p_dm_void,
	bool			is_enable
)
{
	struct PHY_DM_STRUCT					*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u16						period = 0x2710;/* 40ms in default */
	u16						nhm_type = 0x7;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("odm_auto_channel_select_setting()=========>\n"));

	if (is_enable) {
		/* 20 ms */
		period = 0x1388;
		nhm_type = 0x1;
	}

	if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES) {
		/* PHY parameters initialize for ac series */
		odm_write_2byte(p_dm_odm, ODM_REG_CCX_PERIOD_11AC + 2, period);	/* 0x990[31:16]=0x2710	Time duration for NHM unit: 4us, 0x2710=40ms */
		/* odm_set_bb_reg(p_dm_odm, ODM_REG_NHM_TH9_TH10_11AC, BIT(8)|BIT9|BIT10, nhm_type);	 */ /* 0x994[9:8]=3			enable CCX */
	} else if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES) {
		/* PHY parameters initialize for n series */
		odm_write_2byte(p_dm_odm, ODM_REG_CCX_PERIOD_11N + 2, period);	/* 0x894[31:16]=0x2710	Time duration for NHM unit: 4us, 0x2710=40ms */
		/* odm_set_bb_reg(p_dm_odm, ODM_REG_NHM_TH9_TH10_11N, BIT(10)|BIT9|BIT8, nhm_type);	 */ /* 0x890[9:8]=3			enable CCX */
	}
}

void
odm_auto_channel_select_init(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT					*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ACS_						*p_acs = &p_dm_odm->dm_acs;
	u8						i;

	if (!(p_dm_odm->support_ability & ODM_BB_NHM_CNT))
		return;

	if (p_acs->is_force_acs_result)
		return;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("odm_auto_channel_select_init()=========>\n"));

	p_acs->clean_channel_2g = 1;
	p_acs->clean_channel_5g = 36;

	for (i = 0; i < ODM_MAX_CHANNEL_2G; ++i) {
		p_acs->channel_info_2g[0][i] = 0;
		p_acs->channel_info_2g[1][i] = 0;
	}

	if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES) {
		for (i = 0; i < ODM_MAX_CHANNEL_5G; ++i) {
			p_acs->channel_info_5g[0][i] = 0;
			p_acs->channel_info_5g[1][i] = 0;
		}
	}
}

void
odm_auto_channel_select_reset(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT					*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ACS_						*p_acs = &p_dm_odm->dm_acs;

	if (!(p_dm_odm->support_ability & ODM_BB_NHM_CNT))
		return;

	if (p_acs->is_force_acs_result)
		return;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("odm_auto_channel_select_reset()=========>\n"));

	odm_auto_channel_select_setting(p_dm_odm, true); /* for 20ms measurement */
	phydm_nhm_counter_statistics_reset(p_dm_odm);
}

void
odm_auto_channel_select(
	void			*p_dm_void,
	u8			channel
)
{
	struct PHY_DM_STRUCT					*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ACS_						*p_acs = &p_dm_odm->dm_acs;
	u8						channel_idx = 0, search_idx = 0;
	u16						max_score = 0;

	if (!(p_dm_odm->support_ability & ODM_BB_NHM_CNT)) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_auto_channel_select(): Return: support_ability ODM_BB_NHM_CNT is disabled\n"));
		return;
	}

	if (p_acs->is_force_acs_result) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_DIG, ODM_DBG_LOUD, ("odm_auto_channel_select(): Force 2G clean channel = %d, 5G clean channel = %d\n",
			p_acs->clean_channel_2g, p_acs->clean_channel_5g));
		return;
	}

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("odm_auto_channel_select(): channel = %d=========>\n", channel));

	phydm_get_nhm_counter_statistics(p_dm_odm);
	odm_auto_channel_select_setting(p_dm_odm, false);

	if (channel >= 1 && channel <= 14) {
		channel_idx = channel - 1;
		p_acs->channel_info_2g[1][channel_idx]++;

		if (p_acs->channel_info_2g[1][channel_idx] >= 2)
			p_acs->channel_info_2g[0][channel_idx] = (p_acs->channel_info_2g[0][channel_idx] >> 1) +
				(p_acs->channel_info_2g[0][channel_idx] >> 2) + (p_dm_odm->nhm_cnt_0 >> 2);
		else
			p_acs->channel_info_2g[0][channel_idx] = p_dm_odm->nhm_cnt_0;

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("odm_auto_channel_select(): nhm_cnt_0 = %d\n", p_dm_odm->nhm_cnt_0));
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("odm_auto_channel_select(): Channel_Info[0][%d] = %d, Channel_Info[1][%d] = %d\n", channel_idx, p_acs->channel_info_2g[0][channel_idx], channel_idx, p_acs->channel_info_2g[1][channel_idx]));

		for (search_idx = 0; search_idx < ODM_MAX_CHANNEL_2G; search_idx++) {
			if (p_acs->channel_info_2g[1][search_idx] != 0) {
				if (p_acs->channel_info_2g[0][search_idx] >= max_score) {
					max_score = p_acs->channel_info_2g[0][search_idx];
					p_acs->clean_channel_2g = search_idx + 1;
				}
			}
		}
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ACS, ODM_DBG_LOUD, ("(1)odm_auto_channel_select(): 2G: clean_channel_2g = %d, max_score = %d\n",
				p_acs->clean_channel_2g, max_score));

	} else if (channel >= 36) {
		/* Need to do */
		p_acs->clean_channel_5g = channel;
	}
}
