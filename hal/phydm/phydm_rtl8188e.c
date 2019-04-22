// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


#include "mp_precomp.h"

#include "phydm_precomp.h"

void odm_dig_lower_bound_88e(struct PHY_DM_STRUCT *p_dm_odm)
{
	struct _dynamic_initial_gain_threshold_ *p_dm_dig_table =
					 &p_dm_odm->dm_dig_table;

	if (p_dm_odm->ant_div_type == CG_TRX_HW_ANTDIV) {
		p_dm_dig_table->rx_gain_range_min =
			 (u8)p_dm_dig_table->ant_div_rssi_max;
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD,
			     ("odm_dig_lower_bound_88e(): p_dm_dig_table->ant_div_rssi_max=%d\n",
			      p_dm_dig_table->ant_div_rssi_max));
	}
}

/*=============================================================
*  AntDiv Before Link
===============================================================*/
void odm_sw_ant_div_reset_before_link(struct PHY_DM_STRUCT *p_dm_odm)
{
	struct _sw_antenna_switch_ *p_dm_swat_table = &p_dm_odm->dm_swat_table;

	p_dm_swat_table->swas_no_link_state = 0;
}

/* 3============================================================
 * 3 Dynamic Primary CCA
 * 3============================================================ */

void odm_primary_cca_init(struct PHY_DM_STRUCT *p_dm_odm)
{
	struct _dynamic_primary_cca *primary_cca = &(p_dm_odm->dm_pri_cca);
	primary_cca->dup_rts_flag = 0;
	primary_cca->intf_flag = 0;
	primary_cca->intf_type = 0;
	primary_cca->monitor_flag = 0;
	primary_cca->pri_cca_flag = 0;
}

bool odm_dynamic_primary_cca_dup_rts(struct PHY_DM_STRUCT *p_dm_odm)
{
	struct _dynamic_primary_cca *primary_cca = &(p_dm_odm->dm_pri_cca);

	return	primary_cca->dup_rts_flag;
}

void odm_dynamic_primary_cca(struct PHY_DM_STRUCT *p_dm_odm)
{
}
