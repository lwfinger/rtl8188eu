/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#ifndef	__ODM_RTL8188E_H__
#define __ODM_RTL8188E_H__

#if (RTL8188E_SUPPORT == 1)



#define	MAIN_ANT_CG_TRX	1
#define	AUX_ANT_CG_TRX	0
#define	MAIN_ANT_CGCS_RX	0
#define	AUX_ANT_CGCS_RX	1

void
odm_dig_lower_bound_88e(
	struct PHY_DM_STRUCT		*p_dm_odm
);

#define sw_ant_div_reset_before_link		odm_sw_ant_div_reset_before_link

void odm_sw_ant_div_reset_before_link(struct PHY_DM_STRUCT	*p_dm_odm);

void
odm_set_tx_ant_by_tx_info_88e(
	struct PHY_DM_STRUCT		*p_dm_odm,
	u8			*p_desc,
	u8			mac_id
);

void
odm_primary_cca_init(
	struct PHY_DM_STRUCT		*p_dm_odm);

bool
odm_dynamic_primary_cca_dup_rts(
	struct PHY_DM_STRUCT		*p_dm_odm);

void
odm_dynamic_primary_cca(
	struct PHY_DM_STRUCT		*p_dm_odm);

#else	/* (RTL8188E_SUPPORT == 0)*/

#define odm_primary_cca_init(_pdm_odm)
#define odm_dynamic_primary_cca(_pdm_odm)

#endif	/* RTL8188E_SUPPORT */

#endif
