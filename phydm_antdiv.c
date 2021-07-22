// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


/* ************************************************************
 * include files
 * ************************************************************ */

#include "mp_precomp.h"
#include "phydm_precomp.h"

/* ******************************************************
 * when antenna test utility is on or some testing need to disable antenna diversity
 * call this function to disable all ODM related mechanisms which will switch antenna.
 * ****************************************************** */
void
odm_stop_antenna_switch_dm(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	/* disable ODM antenna diversity */
	p_dm_odm->support_ability &= ~ODM_BB_ANT_DIV;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("STOP Antenna Diversity\n"));
}

void
phydm_enable_antenna_diversity(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	p_dm_odm->support_ability |= ODM_BB_ANT_DIV;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("AntDiv is enabled & Re-Init AntDiv\n"));
	odm_antenna_diversity_init(p_dm_odm);
}

void
odm_set_ant_config(
	void	*p_dm_void,
	u8		ant_setting	/* 0=A, 1=B, 2=C, .... */
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	if (p_dm_odm->support_ic_type == ODM_RTL8723B) {
		if (ant_setting == 0)		/* ant A*/
			odm_set_bb_reg(p_dm_odm, 0x948, MASKDWORD, 0x00000000);
		else if (ant_setting == 1)
			odm_set_bb_reg(p_dm_odm, 0x948, MASKDWORD, 0x00000280);
	} else if (p_dm_odm->support_ic_type == ODM_RTL8723D) {
		if (ant_setting == 0)		/* ant A*/
			odm_set_bb_reg(p_dm_odm, 0x948, MASKLWORD, 0x0000);
		else if (ant_setting == 1)
			odm_set_bb_reg(p_dm_odm, 0x948, MASKLWORD, 0x0280);
	}
}

/* ****************************************************** */


void
odm_sw_ant_div_rest_after_link(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _sw_antenna_switch_		*p_dm_swat_table = &p_dm_odm->dm_swat_table;
	struct _FAST_ANTENNA_TRAINNING_		*p_dm_fat_table = &p_dm_odm->dm_fat_table;
	u32             i;

	if (p_dm_odm->ant_div_type == S0S1_SW_ANTDIV) {

		p_dm_swat_table->try_flag = SWAW_STEP_INIT;
		p_dm_swat_table->rssi_trying = 0;
		p_dm_swat_table->double_chk_flag = 0;

		p_dm_fat_table->rx_idle_ant = MAIN_ANT;

#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
		for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++)
			phydm_antdiv_reset_statistic(p_dm_odm, i);
#endif


	}
}


#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
void
phydm_antdiv_reset_statistic(
	void	*p_dm_void,
	u32	macid
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _FAST_ANTENNA_TRAINNING_		*p_dm_fat_table = &p_dm_odm->dm_fat_table;

	p_dm_fat_table->main_ant_sum[macid] = 0;
	p_dm_fat_table->aux_ant_sum[macid] = 0;
	p_dm_fat_table->main_ant_cnt[macid] = 0;
	p_dm_fat_table->aux_ant_cnt[macid] = 0;
	p_dm_fat_table->main_ant_sum_cck[macid] = 0;
	p_dm_fat_table->aux_ant_sum_cck[macid] = 0;
	p_dm_fat_table->main_ant_cnt_cck[macid] = 0;
	p_dm_fat_table->aux_ant_cnt_cck[macid] = 0;
}

void
odm_ant_div_on_off(
	void		*p_dm_void,
	u8		swch
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _FAST_ANTENNA_TRAINNING_	*p_dm_fat_table = &p_dm_odm->dm_fat_table;

	if (p_dm_fat_table->ant_div_on_off != swch) {
		if (p_dm_odm->ant_div_type == S0S1_SW_ANTDIV)
			return;

		if (p_dm_odm->support_ic_type & ODM_N_ANTDIV_SUPPORT) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("(( Turn %s )) N-Series HW-AntDiv block\n", (swch == ANTDIV_ON) ? "ON" : "OFF"));
			odm_set_bb_reg(p_dm_odm, 0xc50, BIT(7), swch);
			odm_set_bb_reg(p_dm_odm, 0xa00, BIT(15), swch);

		} else if (p_dm_odm->support_ic_type & ODM_AC_ANTDIV_SUPPORT) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("(( Turn %s )) AC-Series HW-AntDiv block\n", (swch == ANTDIV_ON) ? "ON" : "OFF"));
			if (p_dm_odm->support_ic_type & (ODM_RTL8812 | ODM_RTL8822B)) {
				odm_set_bb_reg(p_dm_odm, 0xc50, BIT(7), swch); /* OFDM AntDiv function block enable */
				odm_set_bb_reg(p_dm_odm, 0xa00, BIT(15), swch); /* CCK AntDiv function block enable */
			} else {
				odm_set_bb_reg(p_dm_odm, 0x8D4, BIT(24), swch); /* OFDM AntDiv function block enable */

				if ((p_dm_odm->cut_version >= ODM_CUT_C) && (p_dm_odm->support_ic_type == ODM_RTL8821) && (p_dm_odm->ant_div_type != S0S1_SW_ANTDIV)) {
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("(( Turn %s )) CCK HW-AntDiv block\n", (swch == ANTDIV_ON) ? "ON" : "OFF"));
					odm_set_bb_reg(p_dm_odm, 0x800, BIT(25), swch);
					odm_set_bb_reg(p_dm_odm, 0xA00, BIT(15), swch); /* CCK AntDiv function block enable */
				} else if (p_dm_odm->support_ic_type == ODM_RTL8821C) {
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("(( Turn %s )) CCK HW-AntDiv block\n", (swch == ANTDIV_ON) ? "ON" : "OFF"));
					odm_set_bb_reg(p_dm_odm, 0x800, BIT(25), swch);
					odm_set_bb_reg(p_dm_odm, 0xA00, BIT(15), swch); /* CCK AntDiv function block enable */
				}
			}
		}
	}
	p_dm_fat_table->ant_div_on_off = swch;

}

void
phydm_fast_training_enable(
	void		*p_dm_void,
	u8			swch
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8			enable;

	if (swch == FAT_ON)
		enable = 1;
	else
		enable = 0;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Fast ant Training_en = ((%d))\n", enable));

	if (p_dm_odm->support_ic_type == ODM_RTL8188E) {
		odm_set_bb_reg(p_dm_odm, 0xe08, BIT(16), enable);	/*enable fast training*/
		/**/
	} else if (p_dm_odm->support_ic_type == ODM_RTL8192E) {
		odm_set_bb_reg(p_dm_odm, 0xB34, BIT(28), enable);	/*enable fast training (path-A)*/
		/*odm_set_bb_reg(p_dm_odm, 0xB34, BIT(29), enable);*/	/*enable fast training (path-B)*/
	} else if (p_dm_odm->support_ic_type & (ODM_RTL8821 | ODM_RTL8822B)) {
		odm_set_bb_reg(p_dm_odm, 0x900, BIT(19), enable);	/*enable fast training */
		/**/
	}
}

void
phydm_keep_rx_ack_ant_by_tx_ant_time(
	void		*p_dm_void,
	u32		time
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	/* Timming issue: keep Rx ant after tx for ACK ( time x 3.2 mu sec)*/
	if (p_dm_odm->support_ic_type & ODM_N_ANTDIV_SUPPORT) {

		odm_set_bb_reg(p_dm_odm, 0xE20, BIT(23) | BIT(22) | BIT(21) | BIT(20), time);
		/**/
	} else if (p_dm_odm->support_ic_type & ODM_AC_ANTDIV_SUPPORT) {

		odm_set_bb_reg(p_dm_odm, 0x818, BIT(23) | BIT(22) | BIT(21) | BIT(20), time);
		/**/
	}
}

void
odm_tx_by_tx_desc_or_reg(
	void		*p_dm_void,
	u8			swch
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _FAST_ANTENNA_TRAINNING_	*p_dm_fat_table = &p_dm_odm->dm_fat_table;
	u8 enable;

	if (p_dm_fat_table->b_fix_tx_ant == NO_FIX_TX_ANT)
		enable = (swch == TX_BY_DESC) ? 1 : 0;
	else
		enable = 0;/*Force TX by Reg*/

	if (p_dm_odm->ant_div_type != CGCS_RX_HW_ANTDIV) {
		if (p_dm_odm->support_ic_type & ODM_N_ANTDIV_SUPPORT)
			odm_set_bb_reg(p_dm_odm, 0x80c, BIT(21), enable);
		else if (p_dm_odm->support_ic_type & ODM_AC_ANTDIV_SUPPORT)
			odm_set_bb_reg(p_dm_odm, 0x900, BIT(18), enable);

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[AntDiv] TX_Ant_BY (( %s ))\n", (enable == TX_BY_DESC) ? "DESC" : "REG"));
	}
}

void
odm_update_rx_idle_ant(
	void		*p_dm_void,
	u8		ant
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _FAST_ANTENNA_TRAINNING_			*p_dm_fat_table = &p_dm_odm->dm_fat_table;
	u32			default_ant, optional_ant, value32, default_tx_ant;

	if (p_dm_fat_table->rx_idle_ant != ant) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Update Rx-Idle-ant ] rx_idle_ant =%s\n", (ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT"));

		if (!(p_dm_odm->support_ic_type & ODM_RTL8723B))
			p_dm_fat_table->rx_idle_ant = ant;

		if (ant == MAIN_ANT) {
			default_ant   =  ANT1_2G;
			optional_ant =  ANT2_2G;
		} else {
			default_ant  =   ANT2_2G;
			optional_ant =  ANT1_2G;
		}

		if (p_dm_fat_table->b_fix_tx_ant != NO_FIX_TX_ANT)
			default_tx_ant = (p_dm_fat_table->b_fix_tx_ant == FIX_TX_AT_MAIN) ? 0 : 1;
		else
			default_tx_ant = default_ant;

		if (p_dm_odm->support_ic_type & ODM_N_ANTDIV_SUPPORT) {
			if (p_dm_odm->support_ic_type == ODM_RTL8192E) {
				odm_set_bb_reg(p_dm_odm, 0xB38, BIT(5) | BIT4 | BIT3, default_ant); /* Default RX */
				odm_set_bb_reg(p_dm_odm, 0xB38, BIT(8) | BIT7 | BIT6, optional_ant); /* Optional RX */
				odm_set_bb_reg(p_dm_odm, 0x860, BIT(14) | BIT13 | BIT12, default_ant); /* Default TX */
			}
#if (RTL8723B_SUPPORT == 1)
			else if (p_dm_odm->support_ic_type == ODM_RTL8723B) {

				value32 = odm_get_bb_reg(p_dm_odm, 0x948, 0xFFF);

				if (value32 != 0x280)
					odm_update_rx_idle_ant_8723b(p_dm_odm, ant, default_ant, optional_ant);
				else
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Update Rx-Idle-ant ] 8723B: Fail to set RX antenna due to 0x948 = 0x280\n"));
			}
#endif
			else { /*8188E & 8188F*/

				if (p_dm_odm->support_ic_type == ODM_RTL8723D) {
#if (RTL8723D_SUPPORT == 1)
					phydm_set_tx_ant_pwr_8723d(p_dm_odm, ant);
#endif
				}
#if (RTL8188F_SUPPORT == 1)
				else if (p_dm_odm->support_ic_type == ODM_RTL8188F) {
					phydm_update_rx_idle_antenna_8188F(p_dm_odm, default_ant);
					/**/
				}
#endif

				odm_set_bb_reg(p_dm_odm, 0x864, BIT(5) | BIT4 | BIT3, default_ant);		/*Default RX*/
				odm_set_bb_reg(p_dm_odm, 0x864, BIT(8) | BIT7 | BIT6, optional_ant);	/*Optional RX*/
				odm_set_bb_reg(p_dm_odm, 0x860, BIT(14) | BIT13 | BIT12, default_tx_ant);	/*Default TX*/
			}
		} else if (p_dm_odm->support_ic_type & ODM_AC_ANTDIV_SUPPORT) {
			u16	value16 = odm_read_2byte(p_dm_odm, ODM_REG_TRMUX_11AC + 2);
			/*  */
			/* 2014/01/14 MH/Luke.Lee Add direct write for register 0xc0a to prevnt */
			/* incorrect 0xc08 bit0-15 .We still not know why it is changed. */
			/*  */
			value16 &= ~(BIT(11) | BIT(10) | BIT(9) | BIT(8) | BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(3));
			value16 |= ((u16)default_ant << 3);
			value16 |= ((u16)optional_ant << 6);
			value16 |= ((u16)default_ant << 9);
			odm_write_2byte(p_dm_odm, ODM_REG_TRMUX_11AC + 2, value16);
		}

		if (p_dm_odm->support_ic_type == ODM_RTL8188E) {
			odm_set_mac_reg(p_dm_odm, 0x6D8, BIT(7) | BIT6, default_tx_ant);		/*PathA Resp Tx*/
			/**/
		} else {
			odm_set_mac_reg(p_dm_odm, 0x6D8, BIT(10) | BIT9 | BIT8, default_tx_ant);	/*PathA Resp Tx*/
			/**/
		}

	} else { /* p_dm_fat_table->rx_idle_ant == ant */
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Stay in Ori-ant ]  rx_idle_ant =%s\n", (ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT"));
		p_dm_fat_table->rx_idle_ant = ant;
	}
}

void
odm_update_tx_ant(
	void		*p_dm_void,
	u8		ant,
	u32		mac_id
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _FAST_ANTENNA_TRAINNING_	*p_dm_fat_table = &p_dm_odm->dm_fat_table;
	u8	tx_ant;

	if (p_dm_fat_table->b_fix_tx_ant != NO_FIX_TX_ANT)
		ant = (p_dm_fat_table->b_fix_tx_ant == FIX_TX_AT_MAIN) ? MAIN_ANT : AUX_ANT;

	if (p_dm_odm->ant_div_type == CG_TRX_SMART_ANTDIV)
		tx_ant = ant;
	else {
		if (ant == MAIN_ANT)
			tx_ant = ANT1_2G;
		else
			tx_ant = ANT2_2G;
	}

	p_dm_fat_table->antsel_a[mac_id] = tx_ant & BIT(0);
	p_dm_fat_table->antsel_b[mac_id] = (tx_ant & BIT(1)) >> 1;
	p_dm_fat_table->antsel_c[mac_id] = (tx_ant & BIT(2)) >> 2;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Set TX-DESC value]: mac_id:(( %d )),  tx_ant = (( %s ))\n", mac_id, (ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT"));
	/* ODM_RT_TRACE(p_dm_odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("antsel_tr_mux=(( 3'b%d%d%d ))\n",p_dm_fat_table->antsel_c[mac_id] , p_dm_fat_table->antsel_b[mac_id] , p_dm_fat_table->antsel_a[mac_id] )); */

}

#ifdef BEAMFORMING_SUPPORT

void
odm_rx_hw_ant_div_init_88e(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32	value32;
	struct _FAST_ANTENNA_TRAINNING_	*p_dm_fat_table = &p_dm_odm->dm_fat_table;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8188E AntDiv_Init =>  ant_div_type=[CGCS_RX_HW_ANTDIV]\n"));

	/* MAC setting */
	value32 = odm_get_mac_reg(p_dm_odm, ODM_REG_ANTSEL_PIN_11N, MASKDWORD);
	odm_set_mac_reg(p_dm_odm, ODM_REG_ANTSEL_PIN_11N, MASKDWORD, value32 | (BIT(23) | BIT25)); /* Reg4C[25]=1, Reg4C[23]=1 for pin output */
	/* Pin Settings */
	odm_set_bb_reg(p_dm_odm, ODM_REG_PIN_CTRL_11N, BIT(9) | BIT8, 0);/* reg870[8]=1'b0, reg870[9]=1'b0		 */ /* antsel antselb by HW */
	odm_set_bb_reg(p_dm_odm, ODM_REG_RX_ANT_CTRL_11N, BIT(10), 0);	/* reg864[10]=1'b0	 */ /* antsel2 by HW */
	odm_set_bb_reg(p_dm_odm, ODM_REG_LNA_SWITCH_11N, BIT(22), 1);	/* regb2c[22]=1'b0	 */ /* disable CS/CG switch */
	odm_set_bb_reg(p_dm_odm, ODM_REG_LNA_SWITCH_11N, BIT(31), 1);	/* regb2c[31]=1'b1	 */ /* output at CG only */
	/* OFDM Settings */
	odm_set_bb_reg(p_dm_odm, ODM_REG_ANTDIV_PARA1_11N, MASKDWORD, 0x000000a0);
	/* CCK Settings */
	odm_set_bb_reg(p_dm_odm, ODM_REG_BB_PWR_SAV4_11N, BIT(7), 1); /* Fix CCK PHY status report issue */
	odm_set_bb_reg(p_dm_odm, ODM_REG_CCK_ANTDIV_PARA2_11N, BIT(4), 1); /* CCK complete HW AntDiv within 64 samples */

	odm_set_bb_reg(p_dm_odm, ODM_REG_ANT_MAPPING1_11N, 0xFFFF, 0x0001);	/* antenna mapping table */

	p_dm_fat_table->enable_ctrl_frame_antdiv = 1;
}

void
odm_trx_hw_ant_div_init_88e(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32	value32;
	struct _FAST_ANTENNA_TRAINNING_	*p_dm_fat_table = &p_dm_odm->dm_fat_table;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8188E AntDiv_Init =>  ant_div_type=[CG_TRX_HW_ANTDIV (SPDT)]\n"));

	/* MAC setting */
	value32 = odm_get_mac_reg(p_dm_odm, ODM_REG_ANTSEL_PIN_11N, MASKDWORD);
	odm_set_mac_reg(p_dm_odm, ODM_REG_ANTSEL_PIN_11N, MASKDWORD, value32 | (BIT(23) | BIT25)); /* Reg4C[25]=1, Reg4C[23]=1 for pin output */
	/* Pin Settings */
	odm_set_bb_reg(p_dm_odm, ODM_REG_PIN_CTRL_11N, BIT(9) | BIT8, 0);/* reg870[8]=1'b0, reg870[9]=1'b0		 */ /* antsel antselb by HW */
	odm_set_bb_reg(p_dm_odm, ODM_REG_RX_ANT_CTRL_11N, BIT(10), 0);	/* reg864[10]=1'b0	 */ /* antsel2 by HW */
	odm_set_bb_reg(p_dm_odm, ODM_REG_LNA_SWITCH_11N, BIT(22), 0);	/* regb2c[22]=1'b0	 */ /* disable CS/CG switch */
	odm_set_bb_reg(p_dm_odm, ODM_REG_LNA_SWITCH_11N, BIT(31), 1);	/* regb2c[31]=1'b1	 */ /* output at CG only */
	/* OFDM Settings */
	odm_set_bb_reg(p_dm_odm, ODM_REG_ANTDIV_PARA1_11N, MASKDWORD, 0x000000a0);
	/* CCK Settings */
	odm_set_bb_reg(p_dm_odm, ODM_REG_BB_PWR_SAV4_11N, BIT(7), 1); /* Fix CCK PHY status report issue */
	odm_set_bb_reg(p_dm_odm, ODM_REG_CCK_ANTDIV_PARA2_11N, BIT(4), 1); /* CCK complete HW AntDiv within 64 samples */

	/* antenna mapping table */
	if (!p_dm_odm->is_mp_chip) { /* testchip */
		odm_set_bb_reg(p_dm_odm, ODM_REG_RX_DEFUALT_A_11N, BIT(10) | BIT9 | BIT8, 1);	/* Reg858[10:8]=3'b001 */
		odm_set_bb_reg(p_dm_odm, ODM_REG_RX_DEFUALT_A_11N, BIT(13) | BIT12 | BIT11, 2);	/* Reg858[13:11]=3'b010 */
	} else /* MPchip */
		odm_set_bb_reg(p_dm_odm, ODM_REG_ANT_MAPPING1_11N, MASKDWORD, 0x0201);	/*Reg914=3'b010, Reg915=3'b001*/

	p_dm_fat_table->enable_ctrl_frame_antdiv = 1;
}


#if (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
void odm_smart_hw_ant_div_init_88e(void *p_dm_void)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32	value32, i;
	struct _FAST_ANTENNA_TRAINNING_	*p_dm_fat_table = &p_dm_odm->dm_fat_table;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***8188E AntDiv_Init =>  ant_div_type=[CG_TRX_SMART_ANTDIV]\n"));

	p_dm_fat_table->train_idx = 0;
	p_dm_fat_table->fat_state = FAT_PREPARE_STATE;

	p_dm_odm->fat_comb_a = 5;
	p_dm_odm->antdiv_intvl = 0x64; /* 100ms */

	for (i = 0; i < 6; i++)
		p_dm_fat_table->bssid[i] = 0;
	for (i = 0; i < (p_dm_odm->fat_comb_a) ; i++) {
		p_dm_fat_table->ant_sum_rssi[i] = 0;
		p_dm_fat_table->ant_rssi_cnt[i] = 0;
		p_dm_fat_table->ant_ave_rssi[i] = 0;
	}

	/* MAC setting */
	value32 = odm_get_mac_reg(p_dm_odm, 0x4c, MASKDWORD);
	odm_set_mac_reg(p_dm_odm, 0x4c, MASKDWORD, value32 | (BIT(23) | BIT25)); /* Reg4C[25]=1, Reg4C[23]=1 for pin output */
	value32 = odm_get_mac_reg(p_dm_odm,  0x7B4, MASKDWORD);
	odm_set_mac_reg(p_dm_odm, 0x7b4, MASKDWORD, value32 | (BIT(16) | BIT17)); /* Reg7B4[16]=1 enable antenna training, Reg7B4[17]=1 enable A2 match */
	/* value32 = platform_efio_read_4byte(adapter, 0x7B4); */
	/* platform_efio_write_4byte(adapter, 0x7b4, value32|BIT(18));	 */ /* append MACID in reponse packet */

	/* Match MAC ADDR */
	odm_set_mac_reg(p_dm_odm, 0x7b4, 0xFFFF, 0);
	odm_set_mac_reg(p_dm_odm, 0x7b0, MASKDWORD, 0);

	odm_set_bb_reg(p_dm_odm, 0x870, BIT(9) | BIT8, 0);/* reg870[8]=1'b0, reg870[9]=1'b0		 */ /* antsel antselb by HW */
	odm_set_bb_reg(p_dm_odm, 0x864, BIT(10), 0);	/* reg864[10]=1'b0	 */ /* antsel2 by HW */
	odm_set_bb_reg(p_dm_odm, 0xb2c, BIT(22), 0);	/* regb2c[22]=1'b0	 */ /* disable CS/CG switch */
	odm_set_bb_reg(p_dm_odm, 0xb2c, BIT(31), 0);	/* regb2c[31]=1'b1	 */ /* output at CS only */
	odm_set_bb_reg(p_dm_odm, 0xca4, MASKDWORD, 0x000000a0);

	/* antenna mapping table */
	if (p_dm_odm->fat_comb_a == 2) {
		if (!p_dm_odm->is_mp_chip) { /* testchip */
			odm_set_bb_reg(p_dm_odm, 0x858, BIT(10) | BIT9 | BIT8, 1);	/* Reg858[10:8]=3'b001 */
			odm_set_bb_reg(p_dm_odm, 0x858, BIT(13) | BIT12 | BIT11, 2);	/* Reg858[13:11]=3'b010 */
		} else { /* MPchip */
			odm_set_bb_reg(p_dm_odm, 0x914, MASKBYTE0, 1);
			odm_set_bb_reg(p_dm_odm, 0x914, MASKBYTE1, 2);
		}
	} else {
		if (!p_dm_odm->is_mp_chip) { /* testchip */
			odm_set_bb_reg(p_dm_odm, 0x858, BIT(10) | BIT9 | BIT8, 0);	/* Reg858[10:8]=3'b000 */
			odm_set_bb_reg(p_dm_odm, 0x858, BIT(13) | BIT12 | BIT11, 1);	/* Reg858[13:11]=3'b001 */
			odm_set_bb_reg(p_dm_odm, 0x878, BIT(16), 0);
			odm_set_bb_reg(p_dm_odm, 0x858, BIT(15) | BIT14, 2);	/* (Reg878[0],Reg858[14:15])=3'b010 */
			odm_set_bb_reg(p_dm_odm, 0x878, BIT(19) | BIT18 | BIT17, 3); /* Reg878[3:1]=3b'011 */
			odm_set_bb_reg(p_dm_odm, 0x878, BIT(22) | BIT21 | BIT20, 4); /* Reg878[6:4]=3b'100 */
			odm_set_bb_reg(p_dm_odm, 0x878, BIT(25) | BIT24 | BIT23, 5); /* Reg878[9:7]=3b'101 */
			odm_set_bb_reg(p_dm_odm, 0x878, BIT(28) | BIT27 | BIT26, 6); /* Reg878[12:10]=3b'110 */
			odm_set_bb_reg(p_dm_odm, 0x878, BIT(31) | BIT30 | BIT29, 7); /* Reg878[15:13]=3b'111 */
		} else { /* MPchip */
			odm_set_bb_reg(p_dm_odm, 0x914, MASKBYTE0, 4);     /* 0: 3b'000 */
			odm_set_bb_reg(p_dm_odm, 0x914, MASKBYTE1, 2);     /* 1: 3b'001 */
			odm_set_bb_reg(p_dm_odm, 0x914, MASKBYTE2, 0);     /* 2: 3b'010 */
			odm_set_bb_reg(p_dm_odm, 0x914, MASKBYTE3, 1);     /* 3: 3b'011 */
			odm_set_bb_reg(p_dm_odm, 0x918, MASKBYTE0, 3);     /* 4: 3b'100 */
			odm_set_bb_reg(p_dm_odm, 0x918, MASKBYTE1, 5);     /* 5: 3b'101 */
			odm_set_bb_reg(p_dm_odm, 0x918, MASKBYTE2, 6);     /* 6: 3b'110 */
			odm_set_bb_reg(p_dm_odm, 0x918, MASKBYTE3, 255); /* 7: 3b'111 */
		}
	}

	/* Default ant setting when no fast training */
	odm_set_bb_reg(p_dm_odm, 0x864, BIT(5) | BIT4 | BIT3, 0);	/* Default RX */
	odm_set_bb_reg(p_dm_odm, 0x864, BIT(8) | BIT7 | BIT6, 1);	/* Optional RX */
	odm_set_bb_reg(p_dm_odm, 0x860, BIT(14) | BIT13 | BIT12, 0); /* Default TX */

	/* Enter Traing state */
	odm_set_bb_reg(p_dm_odm, 0x864, BIT(2) | BIT1 | BIT0, (p_dm_odm->fat_comb_a - 1));	/* reg864[2:0]=3'd6	 */ /* ant combination=reg864[2:0]+1 */

	/* SW Control */
	/* phy_set_bb_reg(adapter, 0x864, BIT10, 1); */
	/* phy_set_bb_reg(adapter, 0x870, BIT9, 1); */
	/* phy_set_bb_reg(adapter, 0x870, BIT8, 1); */
	/* phy_set_bb_reg(adapter, 0x864, BIT11, 1); */
	/* phy_set_bb_reg(adapter, 0x860, BIT9, 0); */
	/* phy_set_bb_reg(adapter, 0x860, BIT8, 0); */
}
#endif

#ifdef ODM_EVM_ENHANCE_ANTDIV

void
odm_evm_fast_ant_reset(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _FAST_ANTENNA_TRAINNING_	*p_dm_fat_table = &p_dm_odm->dm_fat_table;

	p_dm_fat_table->EVM_method_enable = 0;
	odm_ant_div_on_off(p_dm_odm, ANTDIV_ON);
	p_dm_fat_table->fat_state = NORMAL_STATE_MIAN;
	p_dm_odm->antdiv_period = 0;
	odm_set_mac_reg(p_dm_odm, 0x608, BIT(8), 0);
}


void
odm_evm_enhance_ant_div(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32	main_rssi, aux_rssi ;
	u32	main_crc_utility = 0, aux_crc_utility = 0, utility_ratio = 1;
	u32	main_evm, aux_evm, diff_rssi = 0, diff_EVM = 0;
	u8	score_EVM = 0, score_CRC = 0;
	u8	rssi_larger_ant = 0;
	struct _FAST_ANTENNA_TRAINNING_	*p_dm_fat_table = &p_dm_odm->dm_fat_table;
	u32	value32, i;
	bool main_above1 = false, aux_above1 = false;
	bool force_antenna = false;
	struct sta_info	*p_entry;
	p_dm_fat_table->target_ant_enhance = 0xFF;


	if ((p_dm_odm->support_ic_type & ODM_EVM_ENHANCE_ANTDIV_SUPPORT_IC)) {
		if (p_dm_odm->is_one_entry_only) {
			/* ODM_RT_TRACE(p_dm_odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[One Client only]\n")); */
			i = p_dm_odm->one_entry_macid;

			main_rssi = (p_dm_fat_table->main_ant_cnt[i] != 0) ? (p_dm_fat_table->main_ant_sum[i] / p_dm_fat_table->main_ant_cnt[i]) : 0;
			aux_rssi = (p_dm_fat_table->aux_ant_cnt[i] != 0) ? (p_dm_fat_table->aux_ant_sum[i] / p_dm_fat_table->aux_ant_cnt[i]) : 0;

			if ((main_rssi == 0 && aux_rssi != 0 && aux_rssi >= FORCE_RSSI_DIFF) || (main_rssi != 0 && aux_rssi == 0 && main_rssi >= FORCE_RSSI_DIFF))
				diff_rssi = FORCE_RSSI_DIFF;
			else if (main_rssi != 0 && aux_rssi != 0)
				diff_rssi = (main_rssi >= aux_rssi) ? (main_rssi - aux_rssi) : (aux_rssi - main_rssi);

			if (main_rssi >= aux_rssi)
				rssi_larger_ant = MAIN_ANT;
			else
				rssi_larger_ant = AUX_ANT;

			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" Main_Cnt = (( %d ))  , main_rssi= ((  %d ))\n", p_dm_fat_table->main_ant_cnt[i], main_rssi));
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" Aux_Cnt   = (( %d ))  , aux_rssi = ((  %d ))\n", p_dm_fat_table->aux_ant_cnt[i], aux_rssi));

			if (((main_rssi >= evm_rssi_th_high || aux_rssi >= evm_rssi_th_high) || (p_dm_fat_table->EVM_method_enable == 1))
			    /* && (diff_rssi <= FORCE_RSSI_DIFF + 1) */
			   ) {
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[> TH_H || EVM_method_enable==1]  && "));

				if (((main_rssi >= evm_rssi_th_low) || (aux_rssi >= evm_rssi_th_low))) {
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[> TH_L ]\n"));

					/* 2 [ Normal state Main] */
					if (p_dm_fat_table->fat_state == NORMAL_STATE_MIAN) {

						p_dm_fat_table->EVM_method_enable = 1;
						odm_ant_div_on_off(p_dm_odm, ANTDIV_OFF);
						p_dm_odm->antdiv_period = p_dm_odm->evm_antdiv_period;

						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ start training: MIAN]\n"));
						p_dm_fat_table->main_ant_evm_sum[i] = 0;
						p_dm_fat_table->aux_ant_evm_sum[i] = 0;
						p_dm_fat_table->main_ant_evm_cnt[i] = 0;
						p_dm_fat_table->aux_ant_evm_cnt[i] = 0;

						p_dm_fat_table->fat_state = NORMAL_STATE_AUX;
						odm_set_mac_reg(p_dm_odm, 0x608, BIT(8), 1); /* Accept CRC32 Error packets. */
						odm_update_rx_idle_ant(p_dm_odm, MAIN_ANT);

						p_dm_fat_table->crc32_ok_cnt = 0;
						p_dm_fat_table->crc32_fail_cnt = 0;
						odm_set_timer(p_dm_odm, &p_dm_odm->evm_fast_ant_training_timer, p_dm_odm->antdiv_intvl); /* m */
					}
					/* 2 [ Normal state Aux ] */
					else if (p_dm_fat_table->fat_state == NORMAL_STATE_AUX) {
						p_dm_fat_table->main_crc32_ok_cnt = p_dm_fat_table->crc32_ok_cnt;
						p_dm_fat_table->main_crc32_fail_cnt = p_dm_fat_table->crc32_fail_cnt;

						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ start training: AUX]\n"));
						p_dm_fat_table->fat_state = TRAINING_STATE;
						odm_update_rx_idle_ant(p_dm_odm, AUX_ANT);

						p_dm_fat_table->crc32_ok_cnt = 0;
						p_dm_fat_table->crc32_fail_cnt = 0;
						odm_set_timer(p_dm_odm, &p_dm_odm->evm_fast_ant_training_timer, p_dm_odm->antdiv_intvl); /* ms */
					} else if (p_dm_fat_table->fat_state == TRAINING_STATE) {
						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Training state ]\n"));
						p_dm_fat_table->fat_state = NORMAL_STATE_MIAN;

						/* 3 [CRC32 statistic] */
						p_dm_fat_table->aux_crc32_ok_cnt = p_dm_fat_table->crc32_ok_cnt;
						p_dm_fat_table->aux_crc32_fail_cnt = p_dm_fat_table->crc32_fail_cnt;

						if ((p_dm_fat_table->main_crc32_ok_cnt > ((p_dm_fat_table->aux_crc32_ok_cnt) << 1)) || ((diff_rssi >= 20) && (rssi_larger_ant == MAIN_ANT))) {
							p_dm_fat_table->target_ant_crc32 = MAIN_ANT;
							force_antenna = true;
							ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("CRC32 Force Main\n"));
						} else if ((p_dm_fat_table->aux_crc32_ok_cnt > ((p_dm_fat_table->main_crc32_ok_cnt) << 1)) || ((diff_rssi >= 20) && (rssi_larger_ant == AUX_ANT))) {
							p_dm_fat_table->target_ant_crc32 = AUX_ANT;
							force_antenna = true;
							ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("CRC32 Force Aux\n"));
						} else {
							if (p_dm_fat_table->main_crc32_fail_cnt <= 5)
								p_dm_fat_table->main_crc32_fail_cnt = 5;

							if (p_dm_fat_table->aux_crc32_fail_cnt <= 5)
								p_dm_fat_table->aux_crc32_fail_cnt = 5;

							if (p_dm_fat_table->main_crc32_ok_cnt > p_dm_fat_table->main_crc32_fail_cnt)
								main_above1 = true;

							if (p_dm_fat_table->aux_crc32_ok_cnt > p_dm_fat_table->aux_crc32_fail_cnt)
								aux_above1 = true;

							if (main_above1 == true && aux_above1 == false) {
								force_antenna = true;
								p_dm_fat_table->target_ant_crc32 = MAIN_ANT;
							} else if (main_above1 == false && aux_above1 == true) {
								force_antenna = true;
								p_dm_fat_table->target_ant_crc32 = AUX_ANT;
							} else if (main_above1 == true && aux_above1 == true) {
								main_crc_utility = ((p_dm_fat_table->main_crc32_ok_cnt) << 7) / p_dm_fat_table->main_crc32_fail_cnt;
								aux_crc_utility = ((p_dm_fat_table->aux_crc32_ok_cnt) << 7) / p_dm_fat_table->aux_crc32_fail_cnt;
								p_dm_fat_table->target_ant_crc32 = (main_crc_utility == aux_crc_utility) ? (p_dm_fat_table->pre_target_ant_enhance) : ((main_crc_utility >= aux_crc_utility) ? MAIN_ANT : AUX_ANT);

								if (main_crc_utility != 0 && aux_crc_utility != 0) {
									if (main_crc_utility >= aux_crc_utility)
										utility_ratio = (main_crc_utility << 1) / aux_crc_utility;
									else
										utility_ratio = (aux_crc_utility << 1) / main_crc_utility;
								}
							} else if (main_above1 == false && aux_above1 == false) {
								if (p_dm_fat_table->main_crc32_ok_cnt == 0)
									p_dm_fat_table->main_crc32_ok_cnt = 1;
								if (p_dm_fat_table->aux_crc32_ok_cnt == 0)
									p_dm_fat_table->aux_crc32_ok_cnt = 1;

								main_crc_utility = ((p_dm_fat_table->main_crc32_fail_cnt) << 7) / p_dm_fat_table->main_crc32_ok_cnt;
								aux_crc_utility = ((p_dm_fat_table->aux_crc32_fail_cnt) << 7) / p_dm_fat_table->aux_crc32_ok_cnt;
								p_dm_fat_table->target_ant_crc32 = (main_crc_utility == aux_crc_utility) ? (p_dm_fat_table->pre_target_ant_enhance) : ((main_crc_utility <= aux_crc_utility) ? MAIN_ANT : AUX_ANT);

								if (main_crc_utility != 0 && aux_crc_utility != 0) {
									if (main_crc_utility >= aux_crc_utility)
										utility_ratio = (main_crc_utility << 1) / (aux_crc_utility);
									else
										utility_ratio = (aux_crc_utility << 1) / (main_crc_utility);
								}
							}
						}
						odm_set_mac_reg(p_dm_odm, 0x608, BIT(8), 0);/* NOT Accept CRC32 Error packets. */

						/* 3 [EVM statistic] */
						main_evm = (p_dm_fat_table->main_ant_evm_cnt[i] != 0) ? (p_dm_fat_table->main_ant_evm_sum[i] / p_dm_fat_table->main_ant_evm_cnt[i]) : 0;
						aux_evm = (p_dm_fat_table->aux_ant_evm_cnt[i] != 0) ? (p_dm_fat_table->aux_ant_evm_sum[i] / p_dm_fat_table->aux_ant_evm_cnt[i]) : 0;
						p_dm_fat_table->target_ant_evm = (main_evm == aux_evm) ? (p_dm_fat_table->pre_target_ant_enhance) : ((main_evm >= aux_evm) ? MAIN_ANT : AUX_ANT);

						if ((main_evm == 0 || aux_evm == 0))
							diff_EVM = 0;
						else if (main_evm >= aux_evm)
							diff_EVM = main_evm - aux_evm;
						else
							diff_EVM = aux_evm - main_evm;

						/* 2 [ Decision state ] */
						if (p_dm_fat_table->target_ant_evm == p_dm_fat_table->target_ant_crc32) {
							ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Decision type 1, CRC_utility = ((%d)), EVM_diff = ((%d))\n", utility_ratio, diff_EVM));

							if ((utility_ratio < 2 && force_antenna == false) && diff_EVM <= 30)
								p_dm_fat_table->target_ant_enhance = p_dm_fat_table->pre_target_ant_enhance;
							else
								p_dm_fat_table->target_ant_enhance = p_dm_fat_table->target_ant_evm;
						} else if ((diff_EVM <= 50 && (utility_ratio > 4 && force_antenna == false)) || (force_antenna == true)) {
							ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Decision type 2, CRC_utility = ((%d)), EVM_diff = ((%d))\n", utility_ratio, diff_EVM));
							p_dm_fat_table->target_ant_enhance = p_dm_fat_table->target_ant_crc32;
						} else if (diff_EVM >= 100) {
							ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Decision type 3, CRC_utility = ((%d)), EVM_diff = ((%d))\n", utility_ratio, diff_EVM));
							p_dm_fat_table->target_ant_enhance = p_dm_fat_table->target_ant_evm;
						} else if (utility_ratio >= 6 && force_antenna == false) {
							ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Decision type 4, CRC_utility = ((%d)), EVM_diff = ((%d))\n", utility_ratio, diff_EVM));
							p_dm_fat_table->target_ant_enhance = p_dm_fat_table->target_ant_crc32;
						} else {

							ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Decision type 5, CRC_utility = ((%d)), EVM_diff = ((%d))\n", utility_ratio, diff_EVM));

							if (force_antenna == true)
								score_CRC = 3;
							else if (utility_ratio >= 3) /*>0.5*/
								score_CRC = 2;
							else if (utility_ratio >= 2) /*>1*/
								score_CRC = 1;
							else
								score_CRC = 0;

							if (diff_EVM >= 100)
								score_EVM = 2;
							else if (diff_EVM  >= 50)
								score_EVM = 1;
							else
								score_EVM = 0;

							if (score_CRC > score_EVM)
								p_dm_fat_table->target_ant_enhance = p_dm_fat_table->target_ant_crc32;
							else if (score_CRC < score_EVM)
								p_dm_fat_table->target_ant_enhance = p_dm_fat_table->target_ant_evm;
							else
								p_dm_fat_table->target_ant_enhance = p_dm_fat_table->pre_target_ant_enhance;
						}
						p_dm_fat_table->pre_target_ant_enhance = p_dm_fat_table->target_ant_enhance;

						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] : MainEVM_Cnt = (( %d ))  , main_evm= ((  %d ))\n", i, p_dm_fat_table->main_ant_evm_cnt[i], main_evm));
						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] : AuxEVM_Cnt   = (( %d ))  , aux_evm = ((  %d ))\n", i, p_dm_fat_table->aux_ant_evm_cnt[i], aux_evm));
						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** target_ant_evm = (( %s ))\n", (p_dm_fat_table->target_ant_evm  == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT"));
						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("M_CRC_Ok = (( %d ))  , M_CRC_Fail = ((  %d )), main_crc_utility = (( %d ))\n", p_dm_fat_table->main_crc32_ok_cnt, p_dm_fat_table->main_crc32_fail_cnt, main_crc_utility));
						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("A_CRC_Ok  = (( %d ))  , A_CRC_Fail = ((  %d )), aux_crc_utility   = ((  %d ))\n", p_dm_fat_table->aux_crc32_ok_cnt, p_dm_fat_table->aux_crc32_fail_cnt, aux_crc_utility));
						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** target_ant_crc32 = (( %s ))\n", (p_dm_fat_table->target_ant_crc32 == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT"));
						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("****** target_ant_enhance = (( %s ))******\n", (p_dm_fat_table->target_ant_enhance == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT"));


					}
				} else { /* RSSI< = evm_rssi_th_low */
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ <TH_L: escape from > TH_L ]\n"));
					odm_evm_fast_ant_reset(p_dm_odm);
				}
			} else {
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[escape from> TH_H || EVM_method_enable==1]\n"));
				odm_evm_fast_ant_reset(p_dm_odm);
			}
		} else {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[multi-Client]\n"));
			odm_evm_fast_ant_reset(p_dm_odm);
		}
	}
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void odm_evm_fast_ant_training_callback(void *p_dm_void)
#else
void odm_evm_fast_ant_training_callback(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	struct PHY_DM_STRUCT *p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
#else
	struct PHY_DM_STRUCT *p_dm_odm = from_timer(p_dm_odm, t, evm_fast_ant_training_timer);
#endif

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("******odm_evm_fast_ant_training_callback******\n"));
	odm_hw_ant_div(p_dm_odm);
}
#endif

void
odm_hw_ant_div(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32	i, min_max_rssi = 0xFF,  ant_div_max_rssi = 0, max_rssi = 0, local_max_rssi;
	u32	main_rssi, aux_rssi, mian_cnt, aux_cnt;
	struct _FAST_ANTENNA_TRAINNING_	*p_dm_fat_table = &p_dm_odm->dm_fat_table;
	u8	rx_idle_ant = p_dm_fat_table->rx_idle_ant, target_ant = 7;
	struct _dynamic_initial_gain_threshold_	*p_dm_dig_table = &p_dm_odm->dm_dig_table;
	struct sta_info	*p_entry;

	if (!p_dm_odm->is_linked) { /* is_linked==False */
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[No Link!!!]\n"));

		if (p_dm_fat_table->is_become_linked == true) {
			odm_ant_div_on_off(p_dm_odm, ANTDIV_OFF);
			odm_update_rx_idle_ant(p_dm_odm, MAIN_ANT);
			odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_REG);
			p_dm_odm->antdiv_period = 0;

			p_dm_fat_table->is_become_linked = p_dm_odm->is_linked;
		}
		return;
	} else {
		if (p_dm_fat_table->is_become_linked == false) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Linked !!!]\n"));
			odm_ant_div_on_off(p_dm_odm, ANTDIV_ON);
			p_dm_fat_table->is_become_linked = p_dm_odm->is_linked;

			if (p_dm_odm->support_ic_type == ODM_RTL8723B && p_dm_odm->ant_div_type == CG_TRX_HW_ANTDIV) {
				odm_set_bb_reg(p_dm_odm, 0x930, 0xF0, 8); /* DPDT_P = ANTSEL[0]   */ /* for 8723B AntDiv function patch.  BB  Dino  130412 */
				odm_set_bb_reg(p_dm_odm, 0x930, 0xF, 8); /* DPDT_N = ANTSEL[0] */
			}

			/* 2 BDC Init */

#ifdef ODM_EVM_ENHANCE_ANTDIV
			odm_evm_fast_ant_reset(p_dm_odm);
#endif
		}
	}

	if (*(p_dm_fat_table->p_force_tx_ant_by_desc) == false) {
		if (p_dm_odm->is_one_entry_only == true)
			odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_REG);
		else
			odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_DESC);
	}

#ifdef ODM_EVM_ENHANCE_ANTDIV
	if (p_dm_odm->antdiv_evm_en == 1) {
		odm_evm_enhance_ant_div(p_dm_odm);
		if (p_dm_fat_table->fat_state != NORMAL_STATE_MIAN)
			return;
	} else
		odm_evm_fast_ant_reset(p_dm_odm);
#endif

	/* 2 BDC mode Arbitration */

	for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
		p_entry = p_dm_odm->p_odm_sta_info[i];
		if (IS_STA_VALID(p_entry)) {
			/* 2 Caculate RSSI per Antenna */
			if ((p_dm_fat_table->main_ant_cnt[i] != 0) || (p_dm_fat_table->aux_ant_cnt[i] != 0)) {
				mian_cnt = p_dm_fat_table->main_ant_cnt[i];
				aux_cnt = p_dm_fat_table->aux_ant_cnt[i];
				main_rssi = (mian_cnt != 0) ? (p_dm_fat_table->main_ant_sum[i] / mian_cnt) : 0;
				aux_rssi = (aux_cnt != 0) ? (p_dm_fat_table->aux_ant_sum[i] / aux_cnt) : 0;
				target_ant = (mian_cnt == aux_cnt) ? p_dm_fat_table->rx_idle_ant : ((mian_cnt >= aux_cnt) ? MAIN_ANT : AUX_ANT); /*Use counter number for OFDM*/

			} else {	/*CCK only case*/
				mian_cnt = p_dm_fat_table->main_ant_cnt_cck[i];
				aux_cnt = p_dm_fat_table->aux_ant_cnt_cck[i];
				main_rssi = (mian_cnt != 0) ? (p_dm_fat_table->main_ant_sum_cck[i] / mian_cnt) : 0;
				aux_rssi = (aux_cnt != 0) ? (p_dm_fat_table->aux_ant_sum_cck[i] / aux_cnt) : 0;
				target_ant = (main_rssi == aux_rssi) ? p_dm_fat_table->rx_idle_ant : ((main_rssi >= aux_rssi) ? MAIN_ANT : AUX_ANT); /*Use RSSI for CCK only case*/
			}

			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] : Main_Cnt = (( %d ))  ,  CCK_Main_Cnt = (( %d )) ,  main_rssi= ((  %d ))\n", i, p_dm_fat_table->main_ant_cnt[i], p_dm_fat_table->main_ant_cnt_cck[i], main_rssi));
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** Client[ %d ] : Aux_Cnt   = (( %d ))  , CCK_Aux_Cnt   = (( %d )) ,  aux_rssi = ((  %d ))\n", i, p_dm_fat_table->aux_ant_cnt[i], p_dm_fat_table->aux_ant_cnt_cck[i], aux_rssi));
			/* ODM_RT_TRACE(p_dm_odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** MAC ID:[ %d ] , target_ant = (( %s ))\n", i ,( target_ant ==MAIN_ANT)?"MAIN_ANT":"AUX_ANT")); */

			local_max_rssi = (main_rssi > aux_rssi) ? main_rssi : aux_rssi;
			/* 2 Select max_rssi for DIG */
			if ((local_max_rssi > ant_div_max_rssi) && (local_max_rssi < 40))
				ant_div_max_rssi = local_max_rssi;
			if (local_max_rssi > max_rssi)
				max_rssi = local_max_rssi;

			/* 2 Select RX Idle Antenna */
			if ((local_max_rssi != 0) && (local_max_rssi < min_max_rssi)) {
				rx_idle_ant = target_ant;
				min_max_rssi = local_max_rssi;
			}

#ifdef ODM_EVM_ENHANCE_ANTDIV
			if (p_dm_odm->antdiv_evm_en == 1) {
				if (p_dm_fat_table->target_ant_enhance != 0xFF) {
					target_ant = p_dm_fat_table->target_ant_enhance;
					rx_idle_ant = p_dm_fat_table->target_ant_enhance;
				}
			}
#endif

			/* 2 Select TX Antenna */
			if (p_dm_odm->ant_div_type != CGCS_RX_HW_ANTDIV) {
					odm_update_tx_ant(p_dm_odm, target_ant, i);
			}

			/* ------------------------------------------------------------ */

		}
		phydm_antdiv_reset_statistic(p_dm_odm, i);
	}



	/* 2 Set RX Idle Antenna & TX Antenna(Because of HW Bug ) */

	odm_update_rx_idle_ant(p_dm_odm, rx_idle_ant);

	/* 2 BDC Main Algorithm */
	if (ant_div_max_rssi == 0)
		p_dm_dig_table->ant_div_rssi_max = p_dm_odm->rssi_min;
	else
		p_dm_dig_table->ant_div_rssi_max = ant_div_max_rssi;

	p_dm_dig_table->RSSI_max = max_rssi;
}



#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY

void
odm_s0s1_sw_ant_div_reset(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _sw_antenna_switch_		*p_dm_swat_table	= &p_dm_odm->dm_swat_table;
	struct _FAST_ANTENNA_TRAINNING_		*p_dm_fat_table		= &p_dm_odm->dm_fat_table;

	p_dm_fat_table->is_become_linked  = false;
	p_dm_swat_table->try_flag = SWAW_STEP_INIT;
	p_dm_swat_table->double_chk_flag = 0;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_s0s1_sw_ant_div_reset(): p_dm_fat_table->is_become_linked = %d\n", p_dm_fat_table->is_become_linked));
}

void
odm_s0s1_sw_ant_div(
	void			*p_dm_void,
	u8			step
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _sw_antenna_switch_			*p_dm_swat_table = &p_dm_odm->dm_swat_table;
	struct _FAST_ANTENNA_TRAINNING_			*p_dm_fat_table = &p_dm_odm->dm_fat_table;
	u32			i, min_max_rssi = 0xFF, local_max_rssi, local_min_rssi;
	u32			main_rssi, aux_rssi;
	u8			high_traffic_train_time_u = 0x32, high_traffic_train_time_l = 0, train_time_temp;
	u8			low_traffic_train_time_u = 200, low_traffic_train_time_l = 0;
	u8			rx_idle_ant = p_dm_swat_table->pre_antenna, target_ant, next_ant = 0;
	struct sta_info		*p_entry = NULL;
	u32			value32;
	u32			main_ant_sum;
	u32			aux_ant_sum;
	u32			main_ant_cnt;
	u32			aux_ant_cnt;


	if (!p_dm_odm->is_linked) { /* is_linked==False */
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[No Link!!!]\n"));
		if (p_dm_fat_table->is_become_linked == true) {
			odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_REG);
			if (p_dm_odm->support_ic_type == ODM_RTL8723B) {

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Set REG 948[9:6]=0x0\n"));
				odm_set_bb_reg(p_dm_odm, 0x948, (BIT(9) | BIT(8) | BIT(7) | BIT(6)), 0x0);
			}
			p_dm_fat_table->is_become_linked = p_dm_odm->is_linked;
		}
		return;
	} else {
		if (p_dm_fat_table->is_become_linked == false) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Linked !!!]\n"));

			if (p_dm_odm->support_ic_type == ODM_RTL8723B) {
				value32 = odm_get_bb_reg(p_dm_odm, 0x864, BIT(5) | BIT(4) | BIT(3));

#if (RTL8723B_SUPPORT == 1)
				if (value32 == 0x0)
					odm_update_rx_idle_ant_8723b(p_dm_odm, MAIN_ANT, ANT1_2G, ANT2_2G);
				else if (value32 == 0x1)
					odm_update_rx_idle_ant_8723b(p_dm_odm, AUX_ANT, ANT2_2G, ANT1_2G);
#endif

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("8723B: First link! Force antenna to  %s\n", (value32 == 0x0 ? "MAIN" : "AUX")));
			}
			p_dm_fat_table->is_become_linked = p_dm_odm->is_linked;
		}
	}

	if (*(p_dm_fat_table->p_force_tx_ant_by_desc) == false) {
		if (p_dm_odm->is_one_entry_only == true)
			odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_REG);
		else
			odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_DESC);
	}

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[%d] { try_flag=(( %d )), step=(( %d )), double_chk_flag = (( %d )) }\n",
		__LINE__, p_dm_swat_table->try_flag, step, p_dm_swat_table->double_chk_flag));

	/* Handling step mismatch condition. */
	/* Peak step is not finished at last time. Recover the variable and check again. */
	if (step != p_dm_swat_table->try_flag) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[step != try_flag]    Need to Reset After Link\n"));
		odm_sw_ant_div_rest_after_link(p_dm_odm);
	}

	if (p_dm_swat_table->try_flag == SWAW_STEP_INIT) {

		p_dm_swat_table->try_flag = SWAW_STEP_PEEK;
		p_dm_swat_table->train_time_flag = 0;
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[set try_flag = 0]  Prepare for peek!\n\n"));
		return;

	} else {

		/* 1 Normal state (Begin Trying) */
		if (p_dm_swat_table->try_flag == SWAW_STEP_PEEK) {

			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("TxOkCnt=(( %llu )), RxOkCnt=(( %llu )), traffic_load = (%d))\n", p_dm_odm->cur_tx_ok_cnt, p_dm_odm->cur_rx_ok_cnt, p_dm_odm->traffic_load));

			if (p_dm_odm->traffic_load == TRAFFIC_HIGH) {
				train_time_temp = p_dm_swat_table->train_time ;

				if (p_dm_swat_table->train_time_flag == 3) {
					high_traffic_train_time_l = 0xa;

					if (train_time_temp <= 16)
						train_time_temp = high_traffic_train_time_l;
					else
						train_time_temp -= 16;

				} else if (p_dm_swat_table->train_time_flag == 2) {
					train_time_temp -= 8;
					high_traffic_train_time_l = 0xf;
				} else if (p_dm_swat_table->train_time_flag == 1) {
					train_time_temp -= 4;
					high_traffic_train_time_l = 0x1e;
				} else if (p_dm_swat_table->train_time_flag == 0) {
					train_time_temp += 8;
					high_traffic_train_time_l = 0x28;
				}


				/* ODM_RT_TRACE(p_dm_odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** train_time_temp = ((%d))\n",train_time_temp)); */

				/* -- */
				if (train_time_temp > high_traffic_train_time_u)
					train_time_temp = high_traffic_train_time_u;

				else if (train_time_temp < high_traffic_train_time_l)
					train_time_temp = high_traffic_train_time_l;

				p_dm_swat_table->train_time = train_time_temp; /*10ms~200ms*/

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("train_time_flag=((%d)), train_time=((%d))\n", p_dm_swat_table->train_time_flag, p_dm_swat_table->train_time));

			} else if ((p_dm_odm->traffic_load == TRAFFIC_MID) || (p_dm_odm->traffic_load == TRAFFIC_LOW)) {

				train_time_temp = p_dm_swat_table->train_time ;

				if (p_dm_swat_table->train_time_flag == 3) {
					low_traffic_train_time_l = 10;
					if (train_time_temp < 50)
						train_time_temp = low_traffic_train_time_l;
					else
						train_time_temp -= 50;
				} else if (p_dm_swat_table->train_time_flag == 2) {
					train_time_temp -= 30;
					low_traffic_train_time_l = 36;
				} else if (p_dm_swat_table->train_time_flag == 1) {
					train_time_temp -= 10;
					low_traffic_train_time_l = 40;
				} else {

					train_time_temp += 10;
					low_traffic_train_time_l = 50;
				}

				/* -- */
				if (train_time_temp >= low_traffic_train_time_u)
					train_time_temp = low_traffic_train_time_u;

				else if (train_time_temp <= low_traffic_train_time_l)
					train_time_temp = low_traffic_train_time_l;

				p_dm_swat_table->train_time = train_time_temp; /*10ms~200ms*/

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("train_time_flag=((%d)) , train_time=((%d))\n", p_dm_swat_table->train_time_flag, p_dm_swat_table->train_time));

			} else {
				p_dm_swat_table->train_time = 0xc8; /*200ms*/

			}

			/* ----------------- */

			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Current min_max_rssi is ((%d))\n", p_dm_fat_table->min_max_rssi));

			/* ---reset index--- */
			if (p_dm_swat_table->reset_idx >= RSSI_CHECK_RESET_PERIOD) {

				p_dm_fat_table->min_max_rssi = 0;
				p_dm_swat_table->reset_idx = 0;
			}
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("reset_idx = (( %d ))\n", p_dm_swat_table->reset_idx));

			p_dm_swat_table->reset_idx++;

			/* ---double check flag--- */
			if ((p_dm_fat_table->min_max_rssi > RSSI_CHECK_THRESHOLD) && (p_dm_swat_table->double_chk_flag == 0)) {
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" min_max_rssi is ((%d)), and > %d\n",
					p_dm_fat_table->min_max_rssi, RSSI_CHECK_THRESHOLD));

				p_dm_swat_table->double_chk_flag = 1;
				p_dm_swat_table->try_flag = SWAW_STEP_DETERMINE;
				p_dm_swat_table->rssi_trying = 0;

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Test the current ant for (( %d )) ms again\n", p_dm_swat_table->train_time));
				odm_update_rx_idle_ant(p_dm_odm, p_dm_fat_table->rx_idle_ant);
				odm_set_timer(p_dm_odm, &(p_dm_swat_table->phydm_sw_antenna_switch_timer), p_dm_swat_table->train_time); /*ms*/
				return;
			}

			next_ant = (p_dm_fat_table->rx_idle_ant == MAIN_ANT) ? AUX_ANT : MAIN_ANT;

			p_dm_swat_table->try_flag = SWAW_STEP_DETERMINE;

			if (p_dm_swat_table->reset_idx <= 1)
				p_dm_swat_table->rssi_trying = 2;
			else
				p_dm_swat_table->rssi_trying = 1;

			odm_s0s1_sw_ant_div_by_ctrl_frame(p_dm_odm, SWAW_STEP_PEEK);
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[set try_flag=1]  Normal state:  Begin Trying!!\n"));

		} else if ((p_dm_swat_table->try_flag == SWAW_STEP_DETERMINE) && (p_dm_swat_table->double_chk_flag == 0)) {

			next_ant = (p_dm_fat_table->rx_idle_ant  == MAIN_ANT) ? AUX_ANT : MAIN_ANT;
			p_dm_swat_table->rssi_trying--;
		}

		/* 1 Decision state */
		if ((p_dm_swat_table->try_flag == SWAW_STEP_DETERMINE) && (p_dm_swat_table->rssi_trying == 0)) {

			bool is_by_ctrl_frame = false;
			u64	pkt_cnt_total = 0;

			for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
				p_entry = p_dm_odm->p_odm_sta_info[i];
				if (IS_STA_VALID(p_entry)) {
					/* 2 Caculate RSSI per Antenna */

					main_ant_sum = (u32)p_dm_fat_table->main_ant_sum[i] + (u32)p_dm_fat_table->main_ant_sum_cck[i];
					aux_ant_sum = (u32)p_dm_fat_table->aux_ant_sum[i] + (u32)p_dm_fat_table->aux_ant_sum_cck[i];
					main_ant_cnt = (u32)p_dm_fat_table->main_ant_cnt[i] + (u32)p_dm_fat_table->main_ant_cnt_cck[i];
					aux_ant_cnt = (u32)p_dm_fat_table->aux_ant_cnt[i] + (u32)p_dm_fat_table->aux_ant_cnt_cck[i];

					main_rssi = (main_ant_cnt != 0) ? (main_ant_sum / main_ant_cnt) : 0;
					aux_rssi = (aux_ant_cnt != 0) ? (aux_ant_sum / aux_ant_cnt) : 0;

					if (p_dm_fat_table->main_ant_cnt[i] <= 1 && p_dm_fat_table->main_ant_cnt_cck[i] >= 1)
						main_rssi = 0;

					if (p_dm_fat_table->aux_ant_cnt[i] <= 1 && p_dm_fat_table->aux_ant_cnt_cck[i] >= 1)
						aux_rssi = 0;

					target_ant = (main_rssi == aux_rssi) ? p_dm_swat_table->pre_antenna : ((main_rssi >= aux_rssi) ? MAIN_ANT : AUX_ANT);
					local_max_rssi = (main_rssi >= aux_rssi) ? main_rssi : aux_rssi;
					local_min_rssi = (main_rssi >= aux_rssi) ? aux_rssi : main_rssi;

					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***  CCK_counter_main = (( %d ))  , CCK_counter_aux= ((  %d ))\n", p_dm_fat_table->main_ant_cnt_cck[i], p_dm_fat_table->aux_ant_cnt_cck[i]));
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***  OFDM_counter_main = (( %d ))  , OFDM_counter_aux= ((  %d ))\n", p_dm_fat_table->main_ant_cnt[i], p_dm_fat_table->aux_ant_cnt[i]));
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***  Main_Cnt = (( %d ))  , main_rssi= ((  %d ))\n", main_ant_cnt, main_rssi));
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("***  Aux_Cnt   = (( %d ))  , aux_rssi = ((  %d ))\n", aux_ant_cnt, aux_rssi));
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** MAC ID:[ %d ] , target_ant = (( %s ))\n", i, (target_ant == MAIN_ANT) ? "MAIN_ANT" : "AUX_ANT"));

					/* 2 Select RX Idle Antenna */

					if (local_max_rssi != 0 && local_max_rssi < min_max_rssi) {
						rx_idle_ant = target_ant;
						min_max_rssi = local_max_rssi;
						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("*** local_max_rssi-local_min_rssi = ((%d))\n", (local_max_rssi - local_min_rssi)));

						if ((local_max_rssi - local_min_rssi) > 8) {
							if (local_min_rssi != 0)
								p_dm_swat_table->train_time_flag = 3;
							else {
								if (min_max_rssi > RSSI_CHECK_THRESHOLD)
									p_dm_swat_table->train_time_flag = 0;
								else
									p_dm_swat_table->train_time_flag = 3;
							}
						} else if ((local_max_rssi - local_min_rssi) > 5)
							p_dm_swat_table->train_time_flag = 2;
						else if ((local_max_rssi - local_min_rssi) > 2)
							p_dm_swat_table->train_time_flag = 1;
						else
							p_dm_swat_table->train_time_flag = 0;

					}

					/* 2 Select TX Antenna */
					if (target_ant == MAIN_ANT)
						p_dm_fat_table->antsel_a[i] = ANT1_2G;
					else
						p_dm_fat_table->antsel_a[i] = ANT2_2G;

				}
				phydm_antdiv_reset_statistic(p_dm_odm, i);
				pkt_cnt_total += (main_ant_cnt + aux_ant_cnt);
			}

			if (p_dm_swat_table->is_sw_ant_div_by_ctrl_frame) {
				odm_s0s1_sw_ant_div_by_ctrl_frame(p_dm_odm, SWAW_STEP_DETERMINE);
				is_by_ctrl_frame = true;
			}

			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Control frame packet counter = %d, data frame packet counter = %llu\n",
				p_dm_swat_table->pkt_cnt_sw_ant_div_by_ctrl_frame, pkt_cnt_total));

			if (min_max_rssi == 0xff || ((pkt_cnt_total < (p_dm_swat_table->pkt_cnt_sw_ant_div_by_ctrl_frame >> 1)) && p_dm_odm->phy_dbg_info.num_qry_beacon_pkt < 2)) {
				min_max_rssi = 0;
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Check RSSI of control frame because min_max_rssi == 0xff\n"));
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("is_by_ctrl_frame = %d\n", is_by_ctrl_frame));

				if (is_by_ctrl_frame) {
					main_rssi = (p_dm_fat_table->main_ant_ctrl_frame_cnt != 0) ? (p_dm_fat_table->main_ant_ctrl_frame_sum / p_dm_fat_table->main_ant_ctrl_frame_cnt) : 0;
					aux_rssi = (p_dm_fat_table->aux_ant_ctrl_frame_cnt != 0) ? (p_dm_fat_table->aux_ant_ctrl_frame_sum / p_dm_fat_table->aux_ant_ctrl_frame_cnt) : 0;

					if (p_dm_fat_table->main_ant_ctrl_frame_cnt <= 1 && p_dm_fat_table->cck_ctrl_frame_cnt_main >= 1)
						main_rssi = 0;

					if (p_dm_fat_table->aux_ant_ctrl_frame_cnt <= 1 && p_dm_fat_table->cck_ctrl_frame_cnt_aux >= 1)
						aux_rssi = 0;

					if (main_rssi != 0 || aux_rssi != 0) {
						rx_idle_ant = (main_rssi == aux_rssi) ? p_dm_swat_table->pre_antenna : ((main_rssi >= aux_rssi) ? MAIN_ANT : AUX_ANT);
						local_max_rssi = (main_rssi >= aux_rssi) ? main_rssi : aux_rssi;
						local_min_rssi = (main_rssi >= aux_rssi) ? aux_rssi : main_rssi;

						if ((local_max_rssi - local_min_rssi) > 8)
							p_dm_swat_table->train_time_flag = 3;
						else if ((local_max_rssi - local_min_rssi) > 5)
							p_dm_swat_table->train_time_flag = 2;
						else if ((local_max_rssi - local_min_rssi) > 2)
							p_dm_swat_table->train_time_flag = 1;
						else
							p_dm_swat_table->train_time_flag = 0;

						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Control frame: main_rssi = %d, aux_rssi = %d\n", main_rssi, aux_rssi));
						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("rx_idle_ant decided by control frame = %s\n", (rx_idle_ant == MAIN_ANT ? "MAIN" : "AUX")));
					}
				}
			}

			p_dm_fat_table->min_max_rssi = min_max_rssi;
			p_dm_swat_table->try_flag = SWAW_STEP_PEEK;

			if (p_dm_swat_table->double_chk_flag == 1) {
				p_dm_swat_table->double_chk_flag = 0;

				if (p_dm_fat_table->min_max_rssi > RSSI_CHECK_THRESHOLD) {

					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" [Double check] min_max_rssi ((%d)) > %d again!!\n",
						p_dm_fat_table->min_max_rssi, RSSI_CHECK_THRESHOLD));

					odm_update_rx_idle_ant(p_dm_odm, rx_idle_ant);

					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[reset try_flag = 0] Training accomplished !!!]\n\n\n"));
					return;
				} else {
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" [Double check] min_max_rssi ((%d)) <= %d !!\n",
						p_dm_fat_table->min_max_rssi, RSSI_CHECK_THRESHOLD));

					next_ant = (p_dm_fat_table->rx_idle_ant  == MAIN_ANT) ? AUX_ANT : MAIN_ANT;
					p_dm_swat_table->try_flag = SWAW_STEP_PEEK;
					p_dm_swat_table->reset_idx = RSSI_CHECK_RESET_PERIOD;
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[set try_flag=0]  Normal state:  Need to tryg again!!\n\n\n"));
					return;
				}
			} else {
				if (p_dm_fat_table->min_max_rssi < RSSI_CHECK_THRESHOLD)
					p_dm_swat_table->reset_idx = RSSI_CHECK_RESET_PERIOD;

				p_dm_swat_table->pre_antenna = rx_idle_ant;
				odm_update_rx_idle_ant(p_dm_odm, rx_idle_ant);
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[reset try_flag = 0] Training accomplished !!!] \n\n\n"));
				return;
			}

		}

	}

	/* 1 4.Change TRX antenna */

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("rssi_trying = (( %d )),    ant: (( %s )) >>> (( %s ))\n",
		p_dm_swat_table->rssi_trying, (p_dm_fat_table->rx_idle_ant  == MAIN_ANT ? "MAIN" : "AUX"), (next_ant == MAIN_ANT ? "MAIN" : "AUX")));

	odm_update_rx_idle_ant(p_dm_odm, next_ant);

	/* 1 5.Reset Statistics */

	p_dm_fat_table->rx_idle_ant  = next_ant;

	/* 1 6.Set next timer   (Trying state) */

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" Test ((%s)) ant for (( %d )) ms\n", (next_ant == MAIN_ANT ? "MAIN" : "AUX"), p_dm_swat_table->train_time));
	odm_set_timer(p_dm_odm, &(p_dm_swat_table->phydm_sw_antenna_switch_timer), p_dm_swat_table->train_time); /*ms*/
}

void odm_sw_antdiv_workitem_callback(void *p_context)
{
	struct _ADAPTER * p_adapter = (struct _ADAPTER *)p_context;
	HAL_DATA_TYPE *p_hal_data = GET_HAL_DATA(p_adapter);

	/*dbg_print("SW_antdiv_Workitem_Callback");*/
	odm_s0s1_sw_ant_div(&p_hal_data->odmpriv, SWAW_STEP_DETERMINE);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void odm_sw_antdiv_callback(void *function_context)
#else
void odm_sw_antdiv_callback(istruct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)function_context;
#else
	struct PHY_DM_STRUCT	*p_dm_odm = timer_list(p_dm_odm, t, dm_swat_table.phydm_sw_antenna_switch_timer);
#endif
	struct _ADAPTER	*padapter = p_dm_odm->adapter;
	if (padapter->net_closed == true)
		return;

	rtw_run_in_thread_cmd(padapter, odm_sw_antdiv_workitem_callback, padapter);
}


#endif

void
odm_s0s1_sw_ant_div_by_ctrl_frame(
	void			*p_dm_void,
	u8			step
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _sw_antenna_switch_	*p_dm_swat_table = &p_dm_odm->dm_swat_table;
	struct _FAST_ANTENNA_TRAINNING_		*p_dm_fat_table = &p_dm_odm->dm_fat_table;

	switch (step) {
	case SWAW_STEP_PEEK:
		p_dm_swat_table->pkt_cnt_sw_ant_div_by_ctrl_frame = 0;
		p_dm_swat_table->is_sw_ant_div_by_ctrl_frame = true;
		p_dm_fat_table->main_ant_ctrl_frame_cnt = 0;
		p_dm_fat_table->aux_ant_ctrl_frame_cnt = 0;
		p_dm_fat_table->main_ant_ctrl_frame_sum = 0;
		p_dm_fat_table->aux_ant_ctrl_frame_sum = 0;
		p_dm_fat_table->cck_ctrl_frame_cnt_main = 0;
		p_dm_fat_table->cck_ctrl_frame_cnt_aux = 0;
		p_dm_fat_table->ofdm_ctrl_frame_cnt_main = 0;
		p_dm_fat_table->ofdm_ctrl_frame_cnt_aux = 0;
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_S0S1_SwAntDivForAPMode(): Start peek and reset counter\n"));
		break;
	case SWAW_STEP_DETERMINE:
		p_dm_swat_table->is_sw_ant_div_by_ctrl_frame = false;
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_S0S1_SwAntDivForAPMode(): Stop peek\n"));
		break;
	default:
		p_dm_swat_table->is_sw_ant_div_by_ctrl_frame = false;
		break;
	}
}

void
odm_antsel_statistics_of_ctrl_frame(
	void			*p_dm_void,
	u8			antsel_tr_mux,
	u32			rx_pwdb_all

)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _FAST_ANTENNA_TRAINNING_	*p_dm_fat_table = &p_dm_odm->dm_fat_table;

	if (antsel_tr_mux == ANT1_2G) {
		p_dm_fat_table->main_ant_ctrl_frame_sum += rx_pwdb_all;
		p_dm_fat_table->main_ant_ctrl_frame_cnt++;
	} else {
		p_dm_fat_table->aux_ant_ctrl_frame_sum += rx_pwdb_all;
		p_dm_fat_table->aux_ant_ctrl_frame_cnt++;
	}
}

void
odm_s0s1_sw_ant_div_by_ctrl_frame_process_rssi(
	void			*p_dm_void,
	void			*p_phy_info_void,
	void			*p_pkt_info_void
	/*	struct _odm_phy_status_info_*		p_phy_info, */
	/*	struct _odm_per_pkt_info_*		p_pktinfo */
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _odm_phy_status_info_	*p_phy_info = (struct _odm_phy_status_info_ *)p_phy_info_void;
	struct _odm_per_pkt_info_	*p_pktinfo = (struct _odm_per_pkt_info_ *)p_pkt_info_void;
	struct _sw_antenna_switch_	*p_dm_swat_table = &p_dm_odm->dm_swat_table;
	struct _FAST_ANTENNA_TRAINNING_		*p_dm_fat_table = &p_dm_odm->dm_fat_table;
	bool		is_cck_rate;

	if (!(p_dm_odm->support_ability & ODM_BB_ANT_DIV))
		return;

	if (p_dm_odm->ant_div_type != S0S1_SW_ANTDIV)
		return;

	/* In try state */
	if (!p_dm_swat_table->is_sw_ant_div_by_ctrl_frame)
		return;

	/* No HW error and match receiver address */
	if (!p_pktinfo->is_to_self)
		return;

	p_dm_swat_table->pkt_cnt_sw_ant_div_by_ctrl_frame++;
	is_cck_rate = ((p_pktinfo->data_rate >= DESC_RATE1M) && (p_pktinfo->data_rate <= DESC_RATE11M)) ? true : false;

	if (is_cck_rate) {
		p_dm_fat_table->antsel_rx_keep_0 = (p_dm_fat_table->rx_idle_ant == MAIN_ANT) ? ANT1_2G : ANT2_2G;

		if (p_dm_fat_table->antsel_rx_keep_0 == ANT1_2G)
			p_dm_fat_table->cck_ctrl_frame_cnt_main++;
		else
			p_dm_fat_table->cck_ctrl_frame_cnt_aux++;

		odm_antsel_statistics_of_ctrl_frame(p_dm_odm, p_dm_fat_table->antsel_rx_keep_0, p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_A]);
	} else {
		if (p_dm_fat_table->antsel_rx_keep_0 == ANT1_2G)
			p_dm_fat_table->ofdm_ctrl_frame_cnt_main++;
		else
			p_dm_fat_table->ofdm_ctrl_frame_cnt_aux++;

		odm_antsel_statistics_of_ctrl_frame(p_dm_odm, p_dm_fat_table->antsel_rx_keep_0, p_phy_info->rx_pwdb_all);
	}
}

#endif /* #if (RTL8723B_SUPPORT == 1) || (RTL8821A_SUPPORT == 1) */




void
odm_set_next_mac_addr_target(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _FAST_ANTENNA_TRAINNING_			*p_dm_fat_table = &p_dm_odm->dm_fat_table;
	struct sta_info	*p_entry;
	u32			value32, i;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_set_next_mac_addr_target() ==>\n"));

	if (p_dm_odm->is_linked) {
		for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {

			if ((p_dm_fat_table->train_idx + 1) == ODM_ASSOCIATE_ENTRY_NUM)
				p_dm_fat_table->train_idx = 0;
			else
				p_dm_fat_table->train_idx++;

			p_entry = p_dm_odm->p_odm_sta_info[p_dm_fat_table->train_idx];

			if (IS_STA_VALID(p_entry)) {

				/*Match MAC ADDR*/
				value32 = (p_entry->hwaddr[5] << 8) | p_entry->hwaddr[4];

				odm_set_mac_reg(p_dm_odm, 0x7b4, 0xFFFF, value32);/*0x7b4~0x7b5*/

				value32 = (p_entry->hwaddr[3] << 24) | (p_entry->hwaddr[2] << 16) | (p_entry->hwaddr[1] << 8) | p_entry->hwaddr[0];
				odm_set_mac_reg(p_dm_odm, 0x7b0, MASKDWORD, value32);/*0x7b0~0x7b3*/

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("p_dm_fat_table->train_idx=%d\n", p_dm_fat_table->train_idx));

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Training MAC addr = %x:%x:%x:%x:%x:%x\n",
					p_entry->hwaddr[5], p_entry->hwaddr[4], p_entry->hwaddr[3], p_entry->hwaddr[2], p_entry->hwaddr[1], p_entry->hwaddr[0]));
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Training MAC addr = %x:%x:%x:%x:%x:%x\n",
					p_entry->MacAddr[5], p_entry->MacAddr[4], p_entry->MacAddr[3], p_entry->MacAddr[2], p_entry->MacAddr[1], p_entry->MacAddr[0]));
				break;
			}
		}
	}
}

#if (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))

void
odm_fast_ant_training(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _FAST_ANTENNA_TRAINNING_	*p_dm_fat_table = &p_dm_odm->dm_fat_table;

	u32	max_rssi_path_a = 0, pckcnt_path_a = 0;
	u8	i, target_ant_path_a = 0;
	bool	is_pkt_filter_macth_path_a = false;
#if (RTL8192E_SUPPORT == 1)
	u32	max_rssi_path_b = 0, pckcnt_path_b = 0;
	u8	target_ant_path_b = 0;
	bool	is_pkt_filter_macth_path_b = false;
#endif


	if (!p_dm_odm->is_linked) { /* is_linked==False */
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[No Link!!!]\n"));

		if (p_dm_fat_table->is_become_linked == true) {
			odm_ant_div_on_off(p_dm_odm, ANTDIV_OFF);
			phydm_fast_training_enable(p_dm_odm, FAT_OFF);
			odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_REG);
			p_dm_fat_table->is_become_linked = p_dm_odm->is_linked;
		}
		return;
	} else {
		if (p_dm_fat_table->is_become_linked == false) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Linked!!!]\n"));
			p_dm_fat_table->is_become_linked = p_dm_odm->is_linked;
		}
	}

	if (*(p_dm_fat_table->p_force_tx_ant_by_desc) == false) {
		if (p_dm_odm->is_one_entry_only == true)
			odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_REG);
		else
			odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_DESC);
	}


	if (p_dm_odm->support_ic_type == ODM_RTL8188E)
		odm_set_bb_reg(p_dm_odm, 0x864, BIT(2) | BIT(1) | BIT(0), ((p_dm_odm->fat_comb_a) - 1));
#if (RTL8192E_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8192E) {
		odm_set_bb_reg(p_dm_odm, 0xB38, BIT(2) | BIT1 | BIT0, ((p_dm_odm->fat_comb_a) - 1));	   /* path-A  */ /* ant combination=regB38[2:0]+1 */
		odm_set_bb_reg(p_dm_odm, 0xB38, BIT(18) | BIT17 | BIT16, ((p_dm_odm->fat_comb_b) - 1));  /* path-B  */ /* ant combination=regB38[18:16]+1 */
	}
#endif

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("==>odm_fast_ant_training()\n"));

	/* 1 TRAINING STATE */
	if (p_dm_fat_table->fat_state == FAT_TRAINING_STATE) {
		/* 2 Caculate RSSI per Antenna */

		/* 3 [path-A]--------------------------- */
		for (i = 0; i < (p_dm_odm->fat_comb_a); i++) { /* i : antenna index */
			if (p_dm_fat_table->ant_rssi_cnt[i] == 0)
				p_dm_fat_table->ant_ave_rssi[i] = 0;
			else {
				p_dm_fat_table->ant_ave_rssi[i] = p_dm_fat_table->ant_sum_rssi[i] / p_dm_fat_table->ant_rssi_cnt[i];
				is_pkt_filter_macth_path_a = true;
			}

			if (p_dm_fat_table->ant_ave_rssi[i] > max_rssi_path_a) {
				max_rssi_path_a = p_dm_fat_table->ant_ave_rssi[i];
				pckcnt_path_a = p_dm_fat_table->ant_rssi_cnt[i];
				target_ant_path_a =  i ;
			} else if (p_dm_fat_table->ant_ave_rssi[i] == max_rssi_path_a) {
				if ((p_dm_fat_table->ant_rssi_cnt[i])   >   pckcnt_path_a) {
					max_rssi_path_a = p_dm_fat_table->ant_ave_rssi[i];
					pckcnt_path_a = p_dm_fat_table->ant_rssi_cnt[i];
					target_ant_path_a = i ;
				}
			}

			ODM_RT_TRACE("*** ant-index : [ %d ],      counter = (( %d )),     Avg RSSI = (( %d ))\n", i, p_dm_fat_table->ant_rssi_cnt[i],  p_dm_fat_table->ant_ave_rssi[i]);
		}

		/* 1 DECISION STATE */

		/* 2 Select TRX Antenna */

		phydm_fast_training_enable(p_dm_odm, FAT_OFF);

		/* 3 [path-A]--------------------------- */
		if (is_pkt_filter_macth_path_a  == false) {
			/* ODM_RT_TRACE(p_dm_odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("{path-A}: None Packet is matched\n")); */
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("{path-A}: None Packet is matched\n"));
			odm_ant_div_on_off(p_dm_odm, ANTDIV_OFF);
		} else {
			ODM_RT_TRACE("target_ant_path_a = (( %d )) , max_rssi_path_a = (( %d ))\n", target_ant_path_a, max_rssi_path_a);

			/* 3 [ update RX-optional ant ]        Default RX is Omni, Optional RX is the best decision by FAT */
			if (p_dm_odm->support_ic_type == ODM_RTL8188E)
				odm_set_bb_reg(p_dm_odm, 0x864, BIT(8) | BIT(7) | BIT(6), target_ant_path_a);
			else if (p_dm_odm->support_ic_type == ODM_RTL8192E) {
				odm_set_bb_reg(p_dm_odm, 0xB38, BIT(8) | BIT7 | BIT6, target_ant_path_a); /* Optional RX [pth-A] */
			}
			/* 3 [ update TX ant ] */
			odm_update_tx_ant(p_dm_odm, target_ant_path_a, (p_dm_fat_table->train_idx));

			if (target_ant_path_a == 0)
				odm_ant_div_on_off(p_dm_odm, ANTDIV_OFF);
		}

		/* 2 Reset counter */
		for (i = 0; i < (p_dm_odm->fat_comb_a); i++) {
			p_dm_fat_table->ant_sum_rssi[i] = 0;
			p_dm_fat_table->ant_rssi_cnt[i] = 0;
		}

		p_dm_fat_table->fat_state = FAT_PREPARE_STATE;
		return;
	}

	/* 1 NORMAL STATE */
	if (p_dm_fat_table->fat_state == FAT_PREPARE_STATE) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Start Prepare state ]\n"));

		odm_set_next_mac_addr_target(p_dm_odm);

		/* 2 Prepare Training */
		p_dm_fat_table->fat_state = FAT_TRAINING_STATE;
		phydm_fast_training_enable(p_dm_odm, FAT_ON);
		odm_ant_div_on_off(p_dm_odm, ANTDIV_ON);		/* enable HW AntDiv */
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Start Training state]\n"));

		odm_set_timer(p_dm_odm, &p_dm_odm->fast_ant_training_timer, p_dm_odm->antdiv_intvl); /* ms */
	}

}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void odm_fast_ant_training_callback(void *p_dm_void)
#else
void odm_fast_ant_training_callback(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	struct PHY_DM_STRUCT *p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
#else
	struct PHY_DM_STRUCT *p_dm_odm = from_timer(p_dm_odm, t, fast_ant_training_timer);
#endif
	struct _ADAPTER	*padapter = p_dm_odm->adapter;

	if (padapter->net_closed == true)
		return;
	/* if(*p_dm_odm->p_is_net_closed == true) */
	/* return; */

#if USE_WORKITEM
	odm_schedule_work_item(&p_dm_odm->fast_ant_training_workitem);
#else
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("******odm_fast_ant_training_callback******\n"));
	odm_fast_ant_training(p_dm_odm);
#endif
}

void
odm_fast_ant_training_work_item_callback(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("******odm_fast_ant_training_work_item_callback******\n"));
	odm_fast_ant_training(p_dm_odm);
}

#endif

#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1

u32
phydm_construct_hl_beam_codeword(
	void		*p_dm_void,
	u32		*beam_pattern_idx,
	u32		ant_num
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _SMART_ANTENNA_TRAINNING_		*pdm_sat_table = &(p_dm_odm->dm_sat_table);
	u32		codeword = 0;
	u32		data_tmp;
	u32		i;
	u32		break_counter = 0;

	if (ant_num < 8) {
		for (i = 0; i < (pdm_sat_table->ant_num_total); i++) {
			/*ODM_RT_TRACE(p_dm_odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("beam_pattern_num[%x] = %x\n",i,beam_pattern_num[i] ));*/
			if ((i < (pdm_sat_table->first_train_ant - 1)) /*|| (break_counter >= (pdm_sat_table->ant_num))*/) {
				data_tmp = 0;
				/**/
			} else {

				break_counter++;

				if (beam_pattern_idx[i] == 0) {

					if (*p_dm_odm->p_band_type == ODM_BAND_5G)
						data_tmp = pdm_sat_table->rfu_codeword_table_5g[0];
					else
						data_tmp = pdm_sat_table->rfu_codeword_table[0];

				} else if (beam_pattern_idx[i] == 1) {


					if (*p_dm_odm->p_band_type == ODM_BAND_5G)
						data_tmp = pdm_sat_table->rfu_codeword_table_5g[1];
					else
						data_tmp = pdm_sat_table->rfu_codeword_table[1];

				} else if (beam_pattern_idx[i] == 2) {

					if (*p_dm_odm->p_band_type == ODM_BAND_5G)
						data_tmp = pdm_sat_table->rfu_codeword_table_5g[2];
					else
						data_tmp = pdm_sat_table->rfu_codeword_table[2];

				} else if (beam_pattern_idx[i] == 3) {

					if (*p_dm_odm->p_band_type == ODM_BAND_5G)
						data_tmp = pdm_sat_table->rfu_codeword_table_5g[3];
					else
						data_tmp = pdm_sat_table->rfu_codeword_table[3];
				}
			}


			codeword |= (data_tmp << (i * 4));

		}
	}

	return codeword;
}

void
phydm_update_beam_pattern(
	void		*p_dm_void,
	u32		codeword,
	u32		codeword_length
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _SMART_ANTENNA_TRAINNING_			*pdm_sat_table = &(p_dm_odm->dm_sat_table);
	u8			i;
	bool			beam_ctrl_signal;
	u32			one = 0x1;
	u32			reg44_tmp_p, reg44_tmp_n, reg44_ori;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ SmartAnt ] Set Beam Pattern =0x%x\n", codeword));

	reg44_ori = odm_get_mac_reg(p_dm_odm, 0x44, MASKDWORD);
	/*ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("reg44_ori =0x%x\n", reg44_ori));*/

	for (i = 0; i <= (codeword_length - 1); i++) {
		beam_ctrl_signal = (bool)((codeword & BIT(i)) >> i);

		if (p_dm_odm->debug_components & ODM_COMP_ANT_DIV) {

			if (i == (codeword_length - 1)) {
				dbg_print("%d ]\n", beam_ctrl_signal);
				/**/
			} else if (i == 0) {
				dbg_print("Send codeword[1:24] ---> [ %d ", beam_ctrl_signal);
				/**/
			} else if ((i % 4) == 3) {
				dbg_print("%d  |  ", beam_ctrl_signal);
				/**/
			} else {
				dbg_print("%d ", beam_ctrl_signal);
				/**/
			}
		}

		if (p_dm_odm->support_ic_type == ODM_RTL8821) {
		}
	}
}

void
phydm_update_rx_idle_beam(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _FAST_ANTENNA_TRAINNING_			*p_dm_fat_table = &p_dm_odm->dm_fat_table;
	struct _SMART_ANTENNA_TRAINNING_			*pdm_sat_table = &(p_dm_odm->dm_sat_table);
	u32			i;

	pdm_sat_table->update_beam_codeword = phydm_construct_hl_beam_codeword(p_dm_odm, &(pdm_sat_table->rx_idle_beam[0]), pdm_sat_table->ant_num);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Set target beam_pattern codeword = (( 0x%x ))\n", pdm_sat_table->update_beam_codeword));

	for (i = 0; i < (pdm_sat_table->ant_num); i++) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ Update Rx-Idle-Beam ] RxIdleBeam[%d] =%d\n", i, pdm_sat_table->rx_idle_beam[i]));
		/**/
	}

#if DEV_BUS_TYPE == RT_PCI_INTERFACE
	phydm_update_beam_pattern(p_dm_odm, pdm_sat_table->update_beam_codeword, pdm_sat_table->data_codeword_bit_num);
#else
	odm_schedule_work_item(&pdm_sat_table->hl_smart_antenna_workitem);
	/*odm_stall_execution(1);*/
#endif

	pdm_sat_table->pre_codeword = pdm_sat_table->update_beam_codeword;
}

void
phydm_hl_smart_ant_debug(
	void		*p_dm_void,
	u32		*const dm_value,
	u32		*_used,
	char			*output,
	u32		*_out_len
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _SMART_ANTENNA_TRAINNING_			*pdm_sat_table = &(p_dm_odm->dm_sat_table);
	u32			used = *_used;
	u32			out_len = *_out_len;
	u32			one = 0x1;
	u32			codeword_length = pdm_sat_table->data_codeword_bit_num;
	u32			beam_ctrl_signal, i;

	if (dm_value[0] == 1) { /*fix beam pattern*/

		pdm_sat_table->fix_beam_pattern_en = dm_value[1];

		if (pdm_sat_table->fix_beam_pattern_en == 1) {

			pdm_sat_table->fix_beam_pattern_codeword = dm_value[2];

			if (pdm_sat_table->fix_beam_pattern_codeword  > (one << codeword_length)) {

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ SmartAnt ] Codeword overflow, Current codeword is ((0x%x)), and should be less than ((%d))bit\n",
					pdm_sat_table->fix_beam_pattern_codeword, codeword_length));
				(pdm_sat_table->fix_beam_pattern_codeword) &= 0xffffff;
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ SmartAnt ] Auto modify to (0x%x)\n", pdm_sat_table->fix_beam_pattern_codeword));
			}

			pdm_sat_table->update_beam_codeword = pdm_sat_table->fix_beam_pattern_codeword;

			/*---------------------------------------------------------*/
			PHYDM_SNPRINTF((output + used, out_len - used, "Fix Beam Pattern\n"));
			for (i = 0; i <= (codeword_length - 1); i++) {
				beam_ctrl_signal = (bool)((pdm_sat_table->update_beam_codeword & BIT(i)) >> i);

				if (i == (codeword_length - 1)) {
					PHYDM_SNPRINTF((output + used, out_len - used, "%d]\n", beam_ctrl_signal));
					/**/
				} else if (i == 0) {
					PHYDM_SNPRINTF((output + used, out_len - used, "Send Codeword[1:24] to RFU -> [%d", beam_ctrl_signal));
					/**/
				} else if ((i % 4) == 3) {
					PHYDM_SNPRINTF((output + used, out_len - used, "%d|", beam_ctrl_signal));
					/**/
				} else {
					PHYDM_SNPRINTF((output + used, out_len - used, "%d", beam_ctrl_signal));
					/**/
				}
			}
			/*---------------------------------------------------------*/


#if DEV_BUS_TYPE == RT_PCI_INTERFACE
			phydm_update_beam_pattern(p_dm_odm, pdm_sat_table->update_beam_codeword, pdm_sat_table->data_codeword_bit_num);
#else
			odm_schedule_work_item(&pdm_sat_table->hl_smart_antenna_workitem);
			/*odm_stall_execution(1);*/
#endif
		} else if (pdm_sat_table->fix_beam_pattern_en == 0)
			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Smart Antenna: Enable\n"));

	} else if (dm_value[0] == 2) { /*set latch time*/

		pdm_sat_table->latch_time = dm_value[1];
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ SmartAnt ]  latch_time =0x%x\n", pdm_sat_table->latch_time));
	} else if (dm_value[0] == 3) {

		pdm_sat_table->fix_training_num_en = dm_value[1];

		if (pdm_sat_table->fix_training_num_en == 1) {
			pdm_sat_table->per_beam_training_pkt_num = (u8)dm_value[2];
			pdm_sat_table->decision_holding_period = (u8)dm_value[3];

			PHYDM_SNPRINTF((output + used, out_len - used, "[SmartAnt][Dbg] Fix_train_en = (( %d )), train_pkt_num = (( %d )), holding_period = (( %d )),\n",
				pdm_sat_table->fix_training_num_en, pdm_sat_table->per_beam_training_pkt_num, pdm_sat_table->decision_holding_period));

		} else if (pdm_sat_table->fix_training_num_en == 0) {
			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ]  AUTO per_beam_training_pkt_num\n"));
			/**/
		}
	} else if (dm_value[0] == 4) {

		if (dm_value[1] == 1) {
			pdm_sat_table->ant_num = 1;
			pdm_sat_table->first_train_ant = MAIN_ANT;

		} else if (dm_value[1] == 2) {
			pdm_sat_table->ant_num = 1;
			pdm_sat_table->first_train_ant = AUX_ANT;

		} else if (dm_value[1] == 3) {
			pdm_sat_table->ant_num = 2;
			pdm_sat_table->first_train_ant = MAIN_ANT;
		}

		PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ]  Set ant Num = (( %d )), first_train_ant = (( %d ))\n",
			pdm_sat_table->ant_num, (pdm_sat_table->first_train_ant - 1)));
	} else if (dm_value[0] == 5) {

		if (dm_value[1] <= 3) {
			pdm_sat_table->rfu_codeword_table[dm_value[1]] = dm_value[2];
			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Set Beam_2G: (( %d )), RFU codeword table = (( 0x%x ))\n",
					dm_value[1], dm_value[2]));
		} else {
			for (i = 0; i < 4; i++) {
				PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Show Beam_2G: (( %d )), RFU codeword table = (( 0x%x ))\n",
					i, pdm_sat_table->rfu_codeword_table[i]));
			}
		}
	} else if (dm_value[0] == 6) {

		if (dm_value[1] <= 3) {
			pdm_sat_table->rfu_codeword_table_5g[dm_value[1]] = dm_value[2];
			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Set Beam_5G: (( %d )), RFU codeword table = (( 0x%x ))\n",
					dm_value[1], dm_value[2]));
		} else {
			for (i = 0; i < 4; i++) {
				PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Show Beam_5G: (( %d )), RFU codeword table = (( 0x%x ))\n",
					i, pdm_sat_table->rfu_codeword_table_5g[i]));
			}
		}
	} else if (dm_value[0] == 7) {

		if (dm_value[1] <= 4) {

			pdm_sat_table->beam_patten_num_each_ant = dm_value[1];
			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Set Beam number = (( %d ))\n",
				pdm_sat_table->beam_patten_num_each_ant));
		} else {

			PHYDM_SNPRINTF((output + used, out_len - used, "[ SmartAnt ] Show Beam number = (( %d ))\n",
				pdm_sat_table->beam_patten_num_each_ant));
		}
	}

}


void
phydm_set_all_ant_same_beam_num(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _SMART_ANTENNA_TRAINNING_			*pdm_sat_table = &(p_dm_odm->dm_sat_table);

	if (p_dm_odm->ant_div_type == HL_SW_SMART_ANT_TYPE1) { /*2ant for 8821A*/

		pdm_sat_table->rx_idle_beam[0] = pdm_sat_table->fast_training_beam_num;
		pdm_sat_table->rx_idle_beam[1] = pdm_sat_table->fast_training_beam_num;
	}

	pdm_sat_table->update_beam_codeword = phydm_construct_hl_beam_codeword(p_dm_odm, &(pdm_sat_table->rx_idle_beam[0]), pdm_sat_table->ant_num);

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ SmartAnt ] Set all ant beam_pattern: codeword = (( 0x%x ))\n", pdm_sat_table->update_beam_codeword));

#if DEV_BUS_TYPE == RT_PCI_INTERFACE
	phydm_update_beam_pattern(p_dm_odm, pdm_sat_table->update_beam_codeword, pdm_sat_table->data_codeword_bit_num);
#else
	odm_schedule_work_item(&pdm_sat_table->hl_smart_antenna_workitem);
	/*odm_stall_execution(1);*/
#endif
}

void
odm_fast_ant_training_hl_smart_antenna_type1(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _SMART_ANTENNA_TRAINNING_		*pdm_sat_table = &(p_dm_odm->dm_sat_table);
	struct _FAST_ANTENNA_TRAINNING_		*p_dm_fat_table	 = &(p_dm_odm->dm_fat_table);
	struct _sw_antenna_switch_		*p_dm_swat_table = &p_dm_odm->dm_swat_table;
	u32		codeword = 0, i, j;
	u32		target_ant;
	u32		avg_rssi_tmp, avg_rssi_tmp_ma;
	u32		target_ant_beam_max_rssi[SUPPORT_RF_PATH_NUM] = {0};
	u32		max_beam_ant_rssi = 0;
	u32		target_ant_beam[SUPPORT_RF_PATH_NUM] = {0};
	u32		beam_tmp;
	u8		next_ant;
	u32		rssi_sorting_seq[SUPPORT_BEAM_PATTERN_NUM] = {0};
	u32		rank_idx_seq[SUPPORT_BEAM_PATTERN_NUM] = {0};
	u32		rank_idx_out[SUPPORT_BEAM_PATTERN_NUM] = {0};
	u8		per_beam_rssi_diff_tmp = 0, training_pkt_num_offset;
	u32		break_counter = 0;
	u32		used_ant;


	if (!p_dm_odm->is_linked) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[No Link!!!]\n"));

		if (p_dm_fat_table->is_become_linked == true) {

			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Link->no Link\n"));
			p_dm_fat_table->fat_state = FAT_BEFORE_LINK_STATE;
			odm_ant_div_on_off(p_dm_odm, ANTDIV_OFF);
			odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_REG);
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("change to (( %d )) FAT_state\n", p_dm_fat_table->fat_state));

			p_dm_fat_table->is_become_linked = p_dm_odm->is_linked;
		}
		return;

	} else {
		if (p_dm_fat_table->is_become_linked == false) {

			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Linked !!!]\n"));

			p_dm_fat_table->fat_state = FAT_PREPARE_STATE;
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("change to (( %d )) FAT_state\n", p_dm_fat_table->fat_state));

			/*pdm_sat_table->fast_training_beam_num = 0;*/
			/*phydm_set_all_ant_same_beam_num(p_dm_odm);*/

			p_dm_fat_table->is_become_linked = p_dm_odm->is_linked;
		}
	}

	if (*(p_dm_fat_table->p_force_tx_ant_by_desc) == false) {
		if (p_dm_odm->is_one_entry_only == true)
			odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_REG);
		else
			odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_DESC);
	}

	/*ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("HL Smart ant Training: state (( %d ))\n", p_dm_fat_table->fat_state));*/

	/* [DECISION STATE] */
	/*=======================================================================================*/
	if (p_dm_fat_table->fat_state == FAT_DECISION_STATE) {

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 3. In Decision state]\n"));
		phydm_fast_training_enable(p_dm_odm, FAT_OFF);

		break_counter = 0;
		/*compute target beam in each antenna*/
		for (i = (pdm_sat_table->first_train_ant - 1); i < pdm_sat_table->ant_num_total; i++) {
			for (j = 0; j < (pdm_sat_table->beam_patten_num_each_ant); j++) {

				if (pdm_sat_table->pkt_rssi_cnt[i][j] == 0) {
					avg_rssi_tmp = pdm_sat_table->pkt_rssi_pre[i][j];
					avg_rssi_tmp = (avg_rssi_tmp >= 2) ? (avg_rssi_tmp - 2) : avg_rssi_tmp;
					avg_rssi_tmp_ma = avg_rssi_tmp;
				} else {
					avg_rssi_tmp = (pdm_sat_table->pkt_rssi_sum[i][j]) / (pdm_sat_table->pkt_rssi_cnt[i][j]);
					avg_rssi_tmp_ma = (avg_rssi_tmp + pdm_sat_table->pkt_rssi_pre[i][j]) >> 1;
				}

				rssi_sorting_seq[j] = avg_rssi_tmp;
				pdm_sat_table->pkt_rssi_pre[i][j] = avg_rssi_tmp;

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ant[%d], Beam[%d]: pkt_cnt=(( %d )), avg_rssi_MA=(( %d )), avg_rssi=(( %d ))\n",
					i, j, pdm_sat_table->pkt_rssi_cnt[i][j], avg_rssi_tmp_ma, avg_rssi_tmp));

				if (avg_rssi_tmp > target_ant_beam_max_rssi[i]) {
					target_ant_beam[i] = j;
					target_ant_beam_max_rssi[i] = avg_rssi_tmp;
				}

				/*reset counter value*/
				pdm_sat_table->pkt_rssi_sum[i][j] = 0;
				pdm_sat_table->pkt_rssi_cnt[i][j] = 0;

			}
			pdm_sat_table->rx_idle_beam[i] = target_ant_beam[i];
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("---------> Target of ant[%d]: Beam_num-(( %d )) RSSI= ((%d))\n",
				i,  target_ant_beam[i], target_ant_beam_max_rssi[i]));

			/*sorting*/
			/*
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Pre]rssi_sorting_seq = [%d, %d, %d, %d]\n", rssi_sorting_seq[0], rssi_sorting_seq[1], rssi_sorting_seq[2], rssi_sorting_seq[3]));
			*/

			/*phydm_seq_sorting(p_dm_odm, &rssi_sorting_seq[0], &rank_idx_seq[0], &rank_idx_out[0], SUPPORT_BEAM_PATTERN_NUM);*/

			/*
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Post]rssi_sorting_seq = [%d, %d, %d, %d]\n", rssi_sorting_seq[0], rssi_sorting_seq[1], rssi_sorting_seq[2], rssi_sorting_seq[3]));
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Post]rank_idx_seq = [%d, %d, %d, %d]\n", rank_idx_seq[0], rank_idx_seq[1], rank_idx_seq[2], rank_idx_seq[3]));
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Post]rank_idx_out = [%d, %d, %d, %d]\n", rank_idx_out[0], rank_idx_out[1], rank_idx_out[2], rank_idx_out[3]));
			*/

			if (target_ant_beam_max_rssi[i] > max_beam_ant_rssi) {
				target_ant = i;
				max_beam_ant_rssi = target_ant_beam_max_rssi[i];
				/*ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Target of ant = (( %d )) max_beam_ant_rssi = (( %d ))\n",
					target_ant,  max_beam_ant_rssi));*/
			}
			break_counter++;
			if (break_counter >= (pdm_sat_table->ant_num))
				break;
		}

#ifdef CONFIG_FAT_PATCH
		break_counter = 0;
		for (i = (pdm_sat_table->first_train_ant - 1); i < pdm_sat_table->ant_num_total; i++) {
			for (j = 0; j < (pdm_sat_table->beam_patten_num_each_ant); j++) {

				per_beam_rssi_diff_tmp = (u8)(max_beam_ant_rssi - pdm_sat_table->pkt_rssi_pre[i][j]);
				pdm_sat_table->beam_train_rssi_diff[i][j] = per_beam_rssi_diff_tmp;

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ant[%d], Beam[%d]: RSSI_diff= ((%d))\n",
						i,  j, per_beam_rssi_diff_tmp));
			}
			break_counter++;
			if (break_counter >= (pdm_sat_table->ant_num))
				break;
		}
#endif

		if (target_ant == 0)
			target_ant = MAIN_ANT;
		else if (target_ant == 1)
			target_ant = AUX_ANT;

		if (pdm_sat_table->ant_num > 1) {
			/* [ update RX ant ]*/
			odm_update_rx_idle_ant(p_dm_odm, (u8)target_ant);

			/* [ update TX ant ]*/
			odm_update_tx_ant(p_dm_odm, (u8)target_ant, (p_dm_fat_table->train_idx));
		}

		/*set beam in each antenna*/
		phydm_update_rx_idle_beam(p_dm_odm);

		odm_ant_div_on_off(p_dm_odm, ANTDIV_ON);
		p_dm_fat_table->fat_state = FAT_PREPARE_STATE;
		return;

	}
	/* [TRAINING STATE] */
	else if (p_dm_fat_table->fat_state == FAT_TRAINING_STATE) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ 2. In Training state]\n"));

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("fat_beam_n = (( %d )), pre_fat_beam_n = (( %d ))\n",
			pdm_sat_table->fast_training_beam_num, pdm_sat_table->pre_fast_training_beam_num));

		if (pdm_sat_table->fast_training_beam_num > pdm_sat_table->pre_fast_training_beam_num)

			pdm_sat_table->force_update_beam_en = 0;

		else {

			pdm_sat_table->force_update_beam_en = 1;

			pdm_sat_table->pkt_counter = 0;
			beam_tmp = pdm_sat_table->fast_training_beam_num;
			if (pdm_sat_table->fast_training_beam_num >= (pdm_sat_table->beam_patten_num_each_ant - 1)) {

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Timeout Update]  Beam_num (( %d )) -> (( decision ))\n", pdm_sat_table->fast_training_beam_num));
				p_dm_fat_table->fat_state = FAT_DECISION_STATE;
				odm_fast_ant_training_hl_smart_antenna_type1(p_dm_odm);

			} else {
				pdm_sat_table->fast_training_beam_num++;

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Timeout Update]  Beam_num (( %d )) -> (( %d ))\n", beam_tmp, pdm_sat_table->fast_training_beam_num));
				phydm_set_all_ant_same_beam_num(p_dm_odm);
				p_dm_fat_table->fat_state = FAT_TRAINING_STATE;

			}
		}
		pdm_sat_table->pre_fast_training_beam_num = pdm_sat_table->fast_training_beam_num;
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[prepare state] Update Pre_Beam =(( %d ))\n", pdm_sat_table->pre_fast_training_beam_num));
	}
	/*  [Prepare state] */
	/*=======================================================================================*/
	else if (p_dm_fat_table->fat_state == FAT_PREPARE_STATE) {

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("\n\n[ 1. In Prepare state]\n"));

		if (p_dm_odm->pre_traffic_load == (p_dm_odm->traffic_load)) {
			if (pdm_sat_table->decision_holding_period != 0) {
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Holding_period = (( %d )), return!!!\n", pdm_sat_table->decision_holding_period));
				pdm_sat_table->decision_holding_period--;
				return;
			}
		}


		/* Set training packet number*/
		if (pdm_sat_table->fix_training_num_en == 0) {

			switch (p_dm_odm->traffic_load) {

			case TRAFFIC_HIGH:
				pdm_sat_table->per_beam_training_pkt_num = 8;
				pdm_sat_table->decision_holding_period = 2;
				break;
			case TRAFFIC_MID:
				pdm_sat_table->per_beam_training_pkt_num = 6;
				pdm_sat_table->decision_holding_period = 3;
				break;
			case TRAFFIC_LOW:
				pdm_sat_table->per_beam_training_pkt_num = 3; /*ping 60000*/
				pdm_sat_table->decision_holding_period = 4;
				break;
			case TRAFFIC_ULTRA_LOW:
				pdm_sat_table->per_beam_training_pkt_num = 1;
				pdm_sat_table->decision_holding_period = 6;
				break;
			default:
				break;
			}
		}
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Fix_training_en = (( %d )), training_pkt_num_base = (( %d )), holding_period = ((%d))\n",
			pdm_sat_table->fix_training_num_en, pdm_sat_table->per_beam_training_pkt_num, pdm_sat_table->decision_holding_period));


#ifdef CONFIG_FAT_PATCH
		break_counter = 0;
		for (i = (pdm_sat_table->first_train_ant - 1); i < pdm_sat_table->ant_num_total; i++) {
			for (j = 0; j < (pdm_sat_table->beam_patten_num_each_ant); j++) {

				per_beam_rssi_diff_tmp = pdm_sat_table->beam_train_rssi_diff[i][j];
				training_pkt_num_offset = per_beam_rssi_diff_tmp;

				if ((pdm_sat_table->per_beam_training_pkt_num) > training_pkt_num_offset)
					pdm_sat_table->beam_train_cnt[i][j] = pdm_sat_table->per_beam_training_pkt_num - training_pkt_num_offset;
				else
					pdm_sat_table->beam_train_cnt[i][j] = 1;


				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ant[%d]: Beam_num-(( %d ))  training_pkt_num = ((%d))\n",
					i,  j, pdm_sat_table->beam_train_cnt[i][j]));
			}
			break_counter++;
			if (break_counter >= (pdm_sat_table->ant_num))
				break;
		}


		phydm_fast_training_enable(p_dm_odm, FAT_OFF);
		pdm_sat_table->pre_beacon_counter = pdm_sat_table->beacon_counter;
		pdm_sat_table->update_beam_idx = 0;

		if (*p_dm_odm->p_band_type == ODM_BAND_5G) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Set 5G ant\n"));
			/*used_ant = (pdm_sat_table->first_train_ant == MAIN_ANT) ? AUX_ANT : MAIN_ANT;*/
			used_ant = pdm_sat_table->first_train_ant;
		} else {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Set 2.4G ant\n"));
			used_ant = pdm_sat_table->first_train_ant;
		}

		odm_update_rx_idle_ant(p_dm_odm, (u8)used_ant);

#else
		/* Set training MAC addr. of target */
		odm_set_next_mac_addr_target(p_dm_odm);
		phydm_fast_training_enable(p_dm_odm, FAT_ON);
#endif

		odm_ant_div_on_off(p_dm_odm, ANTDIV_OFF);
		pdm_sat_table->pkt_counter = 0;
		pdm_sat_table->fast_training_beam_num = 0;
		phydm_set_all_ant_same_beam_num(p_dm_odm);
		pdm_sat_table->pre_fast_training_beam_num = pdm_sat_table->fast_training_beam_num;
		p_dm_fat_table->fat_state = FAT_TRAINING_STATE;
	}

}

#endif /*#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1*/

void
odm_ant_div_init(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _FAST_ANTENNA_TRAINNING_			*p_dm_fat_table = &p_dm_odm->dm_fat_table;
	struct _sw_antenna_switch_			*p_dm_swat_table = &p_dm_odm->dm_swat_table;


	if (!(p_dm_odm->support_ability & ODM_BB_ANT_DIV)) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Return!!!]   Not Support Antenna Diversity Function\n"));
		return;
	}
	/* --- */

	/* 2 [--General---] */
	p_dm_odm->antdiv_period = 0;

	p_dm_fat_table->is_become_linked = false;
	p_dm_fat_table->ant_div_on_off = 0xff;

	/* 3       -   AP   - */

	/* 2 [---Set MAIN_ANT as default antenna if Auto-ant enable---] */
	odm_ant_div_on_off(p_dm_odm, ANTDIV_OFF);

	p_dm_odm->ant_type = ODM_AUTO_ANT;

	p_dm_fat_table->rx_idle_ant = 0xff; /*to make RX-idle-antenna will be updated absolutly*/
	odm_update_rx_idle_ant(p_dm_odm, MAIN_ANT);
	phydm_keep_rx_ack_ant_by_tx_ant_time(p_dm_odm, 0);  /* Timming issue: keep Rx ant after tx for ACK ( 5 x 3.2 mu = 16mu sec)*/

	/* 2 [---Set TX Antenna---] */
	if (p_dm_fat_table->p_force_tx_ant_by_desc == NULL) {
	p_dm_fat_table->force_tx_ant_by_desc = 0;
	p_dm_fat_table->p_force_tx_ant_by_desc = &(p_dm_fat_table->force_tx_ant_by_desc);
	}
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("p_force_tx_ant_by_desc = %d\n", *p_dm_fat_table->p_force_tx_ant_by_desc));

	if (*(p_dm_fat_table->p_force_tx_ant_by_desc) == true)
		odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_DESC);
	else
	odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_REG);


	/* 2 [--88E---] */
	if (p_dm_odm->support_ic_type == ODM_RTL8188E) {
#if (RTL8188E_SUPPORT == 1)
		/* p_dm_odm->ant_div_type = CGCS_RX_HW_ANTDIV; */
		/* p_dm_odm->ant_div_type = CG_TRX_HW_ANTDIV; */
		/* p_dm_odm->ant_div_type = CG_TRX_SMART_ANTDIV; */

		if ((p_dm_odm->ant_div_type != CGCS_RX_HW_ANTDIV)  && (p_dm_odm->ant_div_type != CG_TRX_HW_ANTDIV) && (p_dm_odm->ant_div_type != CG_TRX_SMART_ANTDIV)) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Return!!!]  88E Not Supprrt This AntDiv type\n"));
			p_dm_odm->support_ability &= ~(ODM_BB_ANT_DIV);
			return;
		}

		if (p_dm_odm->ant_div_type == CGCS_RX_HW_ANTDIV)
			odm_rx_hw_ant_div_init_88e(p_dm_odm);
		else if (p_dm_odm->ant_div_type == CG_TRX_HW_ANTDIV)
			odm_trx_hw_ant_div_init_88e(p_dm_odm);
#if (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
		else if (p_dm_odm->ant_div_type == CG_TRX_SMART_ANTDIV)
			odm_smart_hw_ant_div_init_88e(p_dm_odm);
#endif
#endif
	}

	/* 2 [--92E---] */
#if (RTL8192E_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8192E) {
		/* p_dm_odm->ant_div_type = CGCS_RX_HW_ANTDIV; */
		/* p_dm_odm->ant_div_type = CG_TRX_HW_ANTDIV; */
		/* p_dm_odm->ant_div_type = CG_TRX_SMART_ANTDIV; */

		if ((p_dm_odm->ant_div_type != CGCS_RX_HW_ANTDIV) && (p_dm_odm->ant_div_type != CG_TRX_HW_ANTDIV)   && (p_dm_odm->ant_div_type != CG_TRX_SMART_ANTDIV)) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Return!!!]  8192E Not Supprrt This AntDiv type\n"));
			p_dm_odm->support_ability &= ~(ODM_BB_ANT_DIV);
			return;
		}

		if (p_dm_odm->ant_div_type == CGCS_RX_HW_ANTDIV)
			odm_rx_hw_ant_div_init_92e(p_dm_odm);
		else if (p_dm_odm->ant_div_type == CG_TRX_HW_ANTDIV)
			odm_trx_hw_ant_div_init_92e(p_dm_odm);
#if (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
		else if (p_dm_odm->ant_div_type == CG_TRX_SMART_ANTDIV)
			odm_smart_hw_ant_div_init_92e(p_dm_odm);
#endif

	}
#endif

	/* 2 [--8723B---] */
#if (RTL8723B_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8723B) {
		p_dm_odm->ant_div_type = S0S1_SW_ANTDIV;
		/* p_dm_odm->ant_div_type = CG_TRX_HW_ANTDIV; */

		if (p_dm_odm->ant_div_type != S0S1_SW_ANTDIV && p_dm_odm->ant_div_type != CG_TRX_HW_ANTDIV) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Return!!!] 8723B  Not Supprrt This AntDiv type\n"));
			p_dm_odm->support_ability &= ~(ODM_BB_ANT_DIV);
			return;
		}

		if (p_dm_odm->ant_div_type == S0S1_SW_ANTDIV)
			odm_s0s1_sw_ant_div_init_8723b(p_dm_odm);
		else if (p_dm_odm->ant_div_type == CG_TRX_HW_ANTDIV)
			odm_trx_hw_ant_div_init_8723b(p_dm_odm);
	}
#endif
	/*2 [--8723D---]*/
#if (RTL8723D_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8723D) {
		if (p_dm_fat_table->p_default_s0_s1 == NULL) {
			p_dm_fat_table->default_s0_s1 = 1;
			p_dm_fat_table->p_default_s0_s1 = &(p_dm_fat_table->default_s0_s1);
		}
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("default_s0_s1 = %d\n", *p_dm_fat_table->p_default_s0_s1));

		if (*(p_dm_fat_table->p_default_s0_s1) == true)
			odm_update_rx_idle_ant(p_dm_odm, MAIN_ANT);
		else
			odm_update_rx_idle_ant(p_dm_odm, AUX_ANT);

		if (p_dm_odm->ant_div_type == S0S1_TRX_HW_ANTDIV)
			odm_trx_hw_ant_div_init_8723d(p_dm_odm);
		else {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Return!!!] 8723D  Not Supprrt This AntDiv type\n"));
			p_dm_odm->support_ability &= ~(ODM_BB_ANT_DIV);
			return;
		}

	}
#endif
	/* 2 [--8811A 8821A---] */
#if (RTL8821A_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8821) {
#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1
		p_dm_odm->ant_div_type = HL_SW_SMART_ANT_TYPE1;

		if (p_dm_odm->ant_div_type == HL_SW_SMART_ANT_TYPE1) {

			odm_trx_hw_ant_div_init_8821a(p_dm_odm);
			phydm_hl_smart_ant_type1_init_8821a(p_dm_odm);
		} else
#endif
		{
			/*p_dm_odm->ant_div_type = CG_TRX_HW_ANTDIV;*/
			p_dm_odm->ant_div_type = S0S1_SW_ANTDIV;

			if (p_dm_odm->ant_div_type != CG_TRX_HW_ANTDIV && p_dm_odm->ant_div_type != S0S1_SW_ANTDIV) {
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Return!!!] 8821A & 8811A  Not Supprrt This AntDiv type\n"));
				p_dm_odm->support_ability &= ~(ODM_BB_ANT_DIV);
				return;
			}
			if (p_dm_odm->ant_div_type == CG_TRX_HW_ANTDIV)
				odm_trx_hw_ant_div_init_8821a(p_dm_odm);
			else if (p_dm_odm->ant_div_type == S0S1_SW_ANTDIV)
				odm_s0s1_sw_ant_div_init_8821a(p_dm_odm);
		}
	}
#endif

	/* 2 [--8821C---] */
#if (RTL8821C_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8821C) {
		p_dm_odm->ant_div_type = CG_TRX_HW_ANTDIV;
		if (p_dm_odm->ant_div_type != CG_TRX_HW_ANTDIV) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Return!!!] 8821C  Not Supprrt This AntDiv type\n"));
			p_dm_odm->support_ability &= ~(ODM_BB_ANT_DIV);
			return;
		}
		odm_trx_hw_ant_div_init_8821c(p_dm_odm);
	}
#endif

	/* 2 [--8881A---] */
#if (RTL8881A_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8881A) {
		/* p_dm_odm->ant_div_type = CGCS_RX_HW_ANTDIV; */
		/* p_dm_odm->ant_div_type = CG_TRX_HW_ANTDIV; */

		if (p_dm_odm->ant_div_type == CG_TRX_HW_ANTDIV) {

			odm_trx_hw_ant_div_init_8881a(p_dm_odm);
			/**/
		} else {

			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Return!!!] 8881A  Not Supprrt This AntDiv type\n"));
			p_dm_odm->support_ability &= ~(ODM_BB_ANT_DIV);
			return;
		}

		odm_trx_hw_ant_div_init_8881a(p_dm_odm);
	}
#endif

	/* 2 [--8812---] */
#if (RTL8812A_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8812) {
		/* p_dm_odm->ant_div_type = CG_TRX_HW_ANTDIV; */

		if (p_dm_odm->ant_div_type != CG_TRX_HW_ANTDIV) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Return!!!] 8812A  Not Supprrt This AntDiv type\n"));
			p_dm_odm->support_ability &= ~(ODM_BB_ANT_DIV);
			return;
		}
		odm_trx_hw_ant_div_init_8812a(p_dm_odm);
	}
#endif

	/*[--8188F---]*/
#if (RTL8188F_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8188F) {

		p_dm_odm->ant_div_type = S0S1_SW_ANTDIV;
		odm_s0s1_sw_ant_div_init_8188f(p_dm_odm);
	}
#endif

}

void
odm_ant_div(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ADAPTER		*p_adapter	= p_dm_odm->adapter;
	struct _FAST_ANTENNA_TRAINNING_			*p_dm_fat_table = &p_dm_odm->dm_fat_table;
#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1
	struct _SMART_ANTENNA_TRAINNING_			*pdm_sat_table = &(p_dm_odm->dm_sat_table);
#endif

	if (*p_dm_odm->p_band_type == ODM_BAND_5G) {
		if (p_dm_fat_table->idx_ant_div_counter_5g <  p_dm_odm->antdiv_period) {
			p_dm_fat_table->idx_ant_div_counter_5g++;
			return;
		} else
			p_dm_fat_table->idx_ant_div_counter_5g = 0;
	} else	if (*p_dm_odm->p_band_type == ODM_BAND_2_4G) {
		if (p_dm_fat_table->idx_ant_div_counter_2g <  p_dm_odm->antdiv_period) {
			p_dm_fat_table->idx_ant_div_counter_2g++;
			return;
		} else
			p_dm_fat_table->idx_ant_div_counter_2g = 0;
	}

	/* ---------- */
	if (!(p_dm_odm->support_ability & ODM_BB_ANT_DIV)) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[Return!!!]   Not Support Antenna Diversity Function\n"));
		return;
	}

	/* ---------- */

	if (p_dm_odm->antdiv_select == 1)
		p_dm_odm->ant_type = ODM_FIX_MAIN_ANT;
	else if (p_dm_odm->antdiv_select == 2)
		p_dm_odm->ant_type = ODM_FIX_AUX_ANT;
	else  /* if (p_dm_odm->antdiv_select==0) */
		p_dm_odm->ant_type = ODM_AUTO_ANT;

	if (p_dm_odm->ant_type != ODM_AUTO_ANT) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Fix Antenna at (( %s ))\n", (p_dm_odm->ant_type == ODM_FIX_MAIN_ANT) ? "MAIN" : "AUX"));

		if (p_dm_odm->ant_type != p_dm_odm->pre_ant_type) {
			odm_ant_div_on_off(p_dm_odm, ANTDIV_OFF);
			odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_REG);

			if (p_dm_odm->ant_type == ODM_FIX_MAIN_ANT)
				odm_update_rx_idle_ant(p_dm_odm, MAIN_ANT);
			else if (p_dm_odm->ant_type == ODM_FIX_AUX_ANT)
				odm_update_rx_idle_ant(p_dm_odm, AUX_ANT);
		}
		p_dm_odm->pre_ant_type = p_dm_odm->ant_type;
		return;
	} else {
		if (p_dm_odm->ant_type != p_dm_odm->pre_ant_type) {
			odm_ant_div_on_off(p_dm_odm, ANTDIV_ON);
			odm_tx_by_tx_desc_or_reg(p_dm_odm, TX_BY_DESC);
		}
		p_dm_odm->pre_ant_type = p_dm_odm->ant_type;
	}


	/* 3 ----------------------------------------------------------------------------------------------------------- */
	/* 2 [--88E---] */
	if (p_dm_odm->support_ic_type == ODM_RTL8188E) {
#if (RTL8188E_SUPPORT == 1)
		if (p_dm_odm->ant_div_type == CG_TRX_HW_ANTDIV || p_dm_odm->ant_div_type == CGCS_RX_HW_ANTDIV)
			odm_hw_ant_div(p_dm_odm);

#if (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
		else if (p_dm_odm->ant_div_type == CG_TRX_SMART_ANTDIV)
			odm_fast_ant_training(p_dm_odm);
#endif

#endif

	}
	/* 2 [--92E---] */
#if (RTL8192E_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8192E) {
		if (p_dm_odm->ant_div_type == CGCS_RX_HW_ANTDIV || p_dm_odm->ant_div_type == CG_TRX_HW_ANTDIV)
			odm_hw_ant_div(p_dm_odm);

#if (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
		else if (p_dm_odm->ant_div_type == CG_TRX_SMART_ANTDIV)
			odm_fast_ant_training(p_dm_odm);
#endif

	}
#endif

#if (RTL8723B_SUPPORT == 1)
	/* 2 [--8723B---] */
	else if (p_dm_odm->support_ic_type == ODM_RTL8723B) {
		if (phydm_is_bt_enable_8723b(p_dm_odm)) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[BT is enable!!!]\n"));
			if (p_dm_fat_table->is_become_linked == true) {
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Set REG 948[9:6]=0x0\n"));
				if (p_dm_odm->support_ic_type == ODM_RTL8723B)
					odm_set_bb_reg(p_dm_odm, 0x948, BIT(9) | BIT(8) | BIT(7) | BIT(6), 0x0);

				p_dm_fat_table->is_become_linked = false;
			}
		} else {
			if (p_dm_odm->ant_div_type == S0S1_SW_ANTDIV) {

#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
				odm_s0s1_sw_ant_div(p_dm_odm, SWAW_STEP_PEEK);
#endif
			} else if (p_dm_odm->ant_div_type == CG_TRX_HW_ANTDIV)
				odm_hw_ant_div(p_dm_odm);
		}
	}
#endif
	/*8723D*/
#if (RTL8723D_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8723D) {

		odm_hw_ant_div(p_dm_odm);
		/**/
	}
#endif

	/* 2 [--8821A---] */
#if (RTL8821A_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8821) {
#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1
		if (p_dm_odm->ant_div_type == HL_SW_SMART_ANT_TYPE1) {

			if (pdm_sat_table->fix_beam_pattern_en != 0) {
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" [ SmartAnt ] Fix SmartAnt Pattern = 0x%x\n", pdm_sat_table->fix_beam_pattern_codeword));
				/*return;*/
			} else {
				/*ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[ SmartAnt ] ant_div_type = HL_SW_SMART_ANT_TYPE1\n"));*/
				odm_fast_ant_training_hl_smart_antenna_type1(p_dm_odm);
			}

		} else
#endif
		{

			if (!p_dm_odm->is_bt_enabled) { /*BT disabled*/
				if (p_dm_odm->ant_div_type == S0S1_SW_ANTDIV) {
					p_dm_odm->ant_div_type = CG_TRX_HW_ANTDIV;
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" [S0S1_SW_ANTDIV]  ->  [CG_TRX_HW_ANTDIV]\n"));
					/*odm_set_bb_reg(p_dm_odm, 0x8D4, BIT24, 1); */
					if (p_dm_fat_table->is_become_linked == true)
						odm_ant_div_on_off(p_dm_odm, ANTDIV_ON);
				}

			} else { /*BT enabled*/

				if (p_dm_odm->ant_div_type == CG_TRX_HW_ANTDIV) {
					p_dm_odm->ant_div_type = S0S1_SW_ANTDIV;
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" [CG_TRX_HW_ANTDIV]  ->  [S0S1_SW_ANTDIV]\n"));
					/*odm_set_bb_reg(p_dm_odm, 0x8D4, BIT24, 0);*/
					odm_ant_div_on_off(p_dm_odm, ANTDIV_OFF);
				}
			}

			if (p_dm_odm->ant_div_type == S0S1_SW_ANTDIV) {

#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
				odm_s0s1_sw_ant_div(p_dm_odm, SWAW_STEP_PEEK);
#endif
			} else if (p_dm_odm->ant_div_type == CG_TRX_HW_ANTDIV)
				odm_hw_ant_div(p_dm_odm);
		}
	}
#endif

	/* 2 [--8821C---] */
#if (RTL8821C_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8821C)
		odm_hw_ant_div(p_dm_odm);
#endif

	/* 2 [--8881A---] */
#if (RTL8881A_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8881A)
		odm_hw_ant_div(p_dm_odm);
#endif

	/* 2 [--8812A---] */
#if (RTL8812A_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8812)
		odm_hw_ant_div(p_dm_odm);
#endif

#if (RTL8188F_SUPPORT == 1)
	/* [--8188F---]*/
	else if (p_dm_odm->support_ic_type == ODM_RTL8188F)	{

#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
		odm_s0s1_sw_ant_div(p_dm_odm, SWAW_STEP_PEEK);
#endif
	}
#endif

	/* [--8822B---]*/
#if (RTL8821A_SUPPORT == 1)
	else if (p_dm_odm->support_ic_type == ODM_RTL8822B) {
#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1
		if (p_dm_odm->ant_div_type == HL_SW_SMART_ANT_TYPE1) {

			if (pdm_sat_table->fix_beam_pattern_en != 0)
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, (" [ SmartAnt ] Fix SmartAnt Pattern = 0x%x\n", pdm_sat_table->fix_beam_pattern_codeword));
			else
				odm_fast_ant_training_hl_smart_antenna_type1(p_dm_odm);
		}
#endif
	}
#endif


}


void
odm_antsel_statistics(
	void			*p_dm_void,
	u8			antsel_tr_mux,
	u32			mac_id,
	u32			utility,
	u8			method,
	u8			is_cck_rate

)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _FAST_ANTENNA_TRAINNING_	*p_dm_fat_table = &p_dm_odm->dm_fat_table;

	if (method == RSSI_METHOD) {

		if (is_cck_rate) {
			if (antsel_tr_mux == ANT1_2G) {
				if (p_dm_fat_table->main_ant_sum_cck[mac_id] > 65435) /*to prevent u16 overflow, max(RSSI)=100, 65435+100 = 65535 (u16)*/
					return;

				p_dm_fat_table->main_ant_sum_cck[mac_id] += (u16)utility;
				p_dm_fat_table->main_ant_cnt_cck[mac_id]++;
			} else {
				if (p_dm_fat_table->aux_ant_sum_cck[mac_id] > 65435)
					return;

				p_dm_fat_table->aux_ant_sum_cck[mac_id] += (u16)utility;
				p_dm_fat_table->aux_ant_cnt_cck[mac_id]++;
			}

		} else { /*ofdm rate*/

			if (antsel_tr_mux == ANT1_2G) {
				if (p_dm_fat_table->main_ant_sum[mac_id] > 65435)
					return;

				p_dm_fat_table->main_ant_sum[mac_id] += (u16)utility;
				p_dm_fat_table->main_ant_cnt[mac_id]++;
			} else {
				if (p_dm_fat_table->aux_ant_sum[mac_id] > 65435)
					return;

				p_dm_fat_table->aux_ant_sum[mac_id] += (u16)utility;
				p_dm_fat_table->aux_ant_cnt[mac_id]++;
			}
		}
	}
#ifdef ODM_EVM_ENHANCE_ANTDIV
	else if (method == EVM_METHOD) {
		if (antsel_tr_mux == ANT1_2G) {
			p_dm_fat_table->main_ant_evm_sum[mac_id] += (utility << 5);
			p_dm_fat_table->main_ant_evm_cnt[mac_id]++;
		} else {
			p_dm_fat_table->aux_ant_evm_sum[mac_id] += (utility << 5);
			p_dm_fat_table->aux_ant_evm_cnt[mac_id]++;
		}
	} else if (method == CRC32_METHOD) {
		if (utility == 0)
			p_dm_fat_table->crc32_fail_cnt++;
		else
			p_dm_fat_table->crc32_ok_cnt += utility;
	}
#endif
}


void
odm_process_rssi_for_ant_div(
	void			*p_dm_void,
	void			*p_phy_info_void,
	void			*p_pkt_info_void
	/*	struct _odm_phy_status_info_*				p_phy_info, */
	/*	struct _odm_per_pkt_info_*			p_pktinfo */
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _odm_phy_status_info_	*p_phy_info = (struct _odm_phy_status_info_ *)p_phy_info_void;
	struct _odm_per_pkt_info_	*p_pktinfo = (struct _odm_per_pkt_info_ *)p_pkt_info_void;
	u8			is_cck_rate = 0, cck_max_rate = ODM_RATE11M;
	struct _FAST_ANTENNA_TRAINNING_			*p_dm_fat_table = &p_dm_odm->dm_fat_table;
#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1
	struct _SMART_ANTENNA_TRAINNING_			*pdm_sat_table = &(p_dm_odm->dm_sat_table);
	u32			beam_tmp;
	u8			next_ant;
	u8			train_pkt_number;
#endif

	u8			rx_power_ant0, rx_power_ant1;
	u8			rx_evm_ant0, rx_evm_ant1;

	cck_max_rate = ODM_RATE11M;
	is_cck_rate = (p_pktinfo->data_rate <= cck_max_rate) ? true : false;

	if ((p_dm_odm->support_ic_type & ODM_IC_2SS) && (p_pktinfo->data_rate > cck_max_rate)) {
		rx_power_ant0 = p_phy_info->rx_mimo_signal_strength[0];
		rx_power_ant1 = p_phy_info->rx_mimo_signal_strength[1];

		rx_evm_ant0 = p_phy_info->rx_mimo_signal_quality[0];
		rx_evm_ant1 = p_phy_info->rx_mimo_signal_quality[1];
	} else
		rx_power_ant0 = p_phy_info->rx_pwdb_all;

#ifdef CONFIG_HL_SMART_ANTENNA_TYPE1
#ifdef CONFIG_FAT_PATCH
	if ((p_dm_odm->ant_div_type == HL_SW_SMART_ANT_TYPE1) && (p_dm_fat_table->fat_state == FAT_TRAINING_STATE)) {

		/*[Beacon]*/
		if (p_pktinfo->is_packet_beacon) {

			pdm_sat_table->beacon_counter++;
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("MatchBSSID_beacon_counter = ((%d))\n", pdm_sat_table->beacon_counter));

			if (pdm_sat_table->beacon_counter >= pdm_sat_table->pre_beacon_counter + 2) {

				if (pdm_sat_table->ant_num > 1) {
					next_ant = (p_dm_fat_table->rx_idle_ant == MAIN_ANT) ? AUX_ANT : MAIN_ANT;
					odm_update_rx_idle_ant(p_dm_odm, next_ant);
				}

				pdm_sat_table->update_beam_idx++;

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("pre_beacon_counter = ((%d)), pkt_counter = ((%d)), update_beam_idx = ((%d))\n",
					pdm_sat_table->pre_beacon_counter, pdm_sat_table->pkt_counter, pdm_sat_table->update_beam_idx));

				pdm_sat_table->pre_beacon_counter = pdm_sat_table->beacon_counter;
				pdm_sat_table->pkt_counter = 0;
			}
		}
		/*[data]*/
		else if (p_pktinfo->is_packet_to_self) {

			if (pdm_sat_table->pkt_skip_statistic_en == 0) {
				/*
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("StaID[%d]:  antsel_pathA = ((%d)), hw_antsw_occur = ((%d)), Beam_num = ((%d)), RSSI = ((%d))\n",
					p_pktinfo->station_id, p_dm_fat_table->antsel_rx_keep_0, p_dm_fat_table->hw_antsw_occur, pdm_sat_table->fast_training_beam_num, rx_power_ant0));
				*/
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ID[%d][pkt_cnt = %d]: {ANT, Beam} = {%d, %d}, RSSI = ((%d))\n",
					p_pktinfo->station_id, pdm_sat_table->pkt_counter, p_dm_fat_table->antsel_rx_keep_0, pdm_sat_table->fast_training_beam_num, rx_power_ant0));

				pdm_sat_table->pkt_rssi_sum[p_dm_fat_table->antsel_rx_keep_0][pdm_sat_table->fast_training_beam_num] += rx_power_ant0;
				pdm_sat_table->pkt_rssi_cnt[p_dm_fat_table->antsel_rx_keep_0][pdm_sat_table->fast_training_beam_num]++;
				pdm_sat_table->pkt_counter++;

				train_pkt_number = pdm_sat_table->beam_train_cnt[p_dm_fat_table->rx_idle_ant - 1][pdm_sat_table->fast_training_beam_num];

				/*Swich Antenna erery N pkts*/
				if (pdm_sat_table->pkt_counter == train_pkt_number) {

					if (pdm_sat_table->ant_num > 1) {

						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("packet enugh ((%d ))pkts ---> Switch antenna\n", train_pkt_number));
						next_ant = (p_dm_fat_table->rx_idle_ant == MAIN_ANT) ? AUX_ANT : MAIN_ANT;
						odm_update_rx_idle_ant(p_dm_odm, next_ant);
					}

					pdm_sat_table->update_beam_idx++;
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("pre_beacon_counter = ((%d)), update_beam_idx_counter = ((%d))\n",
						pdm_sat_table->pre_beacon_counter, pdm_sat_table->update_beam_idx));

					pdm_sat_table->pre_beacon_counter = pdm_sat_table->beacon_counter;
					pdm_sat_table->pkt_counter = 0;
				}
			}
		}

		/*Swich Beam after switch "pdm_sat_table->ant_num" antennas*/
		if (pdm_sat_table->update_beam_idx == pdm_sat_table->ant_num) {

			pdm_sat_table->update_beam_idx = 0;
			pdm_sat_table->pkt_counter = 0;
			beam_tmp = pdm_sat_table->fast_training_beam_num;

			if (pdm_sat_table->fast_training_beam_num >= (pdm_sat_table->beam_patten_num_each_ant - 1)) {

				p_dm_fat_table->fat_state = FAT_DECISION_STATE;

#if DEV_BUS_TYPE == RT_PCI_INTERFACE
				odm_fast_ant_training_hl_smart_antenna_type1(p_dm_odm);
#else
				odm_schedule_work_item(&pdm_sat_table->hl_smart_antenna_decision_workitem);
#endif


			} else {
				pdm_sat_table->fast_training_beam_num++;
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Update Beam_num (( %d )) -> (( %d ))\n", beam_tmp, pdm_sat_table->fast_training_beam_num));
				phydm_set_all_ant_same_beam_num(p_dm_odm);

				p_dm_fat_table->fat_state = FAT_TRAINING_STATE;
			}
		}

	}
#else

	if (p_dm_odm->ant_div_type == HL_SW_SMART_ANT_TYPE1) {
		if ((p_dm_odm->support_ic_type & ODM_HL_SMART_ANT_TYPE1_SUPPORT) &&
		    (p_pktinfo->is_packet_to_self)   &&
		    (p_dm_fat_table->fat_state == FAT_TRAINING_STATE)
		   ) {

			if (pdm_sat_table->pkt_skip_statistic_en == 0) {
				/*
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("StaID[%d]:  antsel_pathA = ((%d)), hw_antsw_occur = ((%d)), Beam_num = ((%d)), RSSI = ((%d))\n",
					p_pktinfo->station_id, p_dm_fat_table->antsel_rx_keep_0, p_dm_fat_table->hw_antsw_occur, pdm_sat_table->fast_training_beam_num, rx_power_ant0));
				*/
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("StaID[%d]:  antsel_pathA = ((%d)), is_packet_to_self = ((%d)), Beam_num = ((%d)), RSSI = ((%d))\n",
					p_pktinfo->station_id, p_dm_fat_table->antsel_rx_keep_0, p_pktinfo->is_packet_to_self, pdm_sat_table->fast_training_beam_num, rx_power_ant0));


				pdm_sat_table->pkt_rssi_sum[p_dm_fat_table->antsel_rx_keep_0][pdm_sat_table->fast_training_beam_num] += rx_power_ant0;
				pdm_sat_table->pkt_rssi_cnt[p_dm_fat_table->antsel_rx_keep_0][pdm_sat_table->fast_training_beam_num]++;
				pdm_sat_table->pkt_counter++;

				/*swich beam every N pkt*/
				if ((pdm_sat_table->pkt_counter) >= (pdm_sat_table->per_beam_training_pkt_num)) {

					pdm_sat_table->pkt_counter = 0;
					beam_tmp = pdm_sat_table->fast_training_beam_num;

					if (pdm_sat_table->fast_training_beam_num >= (pdm_sat_table->beam_patten_num_each_ant - 1)) {

						p_dm_fat_table->fat_state = FAT_DECISION_STATE;

#if DEV_BUS_TYPE == RT_PCI_INTERFACE
						odm_fast_ant_training_hl_smart_antenna_type1(p_dm_odm);
#else
						odm_schedule_work_item(&pdm_sat_table->hl_smart_antenna_decision_workitem);
#endif


					} else {
						pdm_sat_table->fast_training_beam_num++;
						phydm_set_all_ant_same_beam_num(p_dm_odm);

						p_dm_fat_table->fat_state = FAT_TRAINING_STATE;
						ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Update  Beam_num (( %d )) -> (( %d ))\n", beam_tmp, pdm_sat_table->fast_training_beam_num));
					}
				}
			}
		}
	}
#endif
	else
#endif
		if (p_dm_odm->ant_div_type == CG_TRX_SMART_ANTDIV) {
			if ((p_dm_odm->support_ic_type & ODM_SMART_ANT_SUPPORT) && (p_pktinfo->is_packet_to_self)   && (p_dm_fat_table->fat_state == FAT_TRAINING_STATE)) { /* (p_pktinfo->is_packet_match_bssid && (!p_pktinfo->is_packet_beacon)) */
				u8	antsel_tr_mux;
				antsel_tr_mux = (p_dm_fat_table->antsel_rx_keep_2 << 2) | (p_dm_fat_table->antsel_rx_keep_1 << 1) | p_dm_fat_table->antsel_rx_keep_0;
				p_dm_fat_table->ant_sum_rssi[antsel_tr_mux] += rx_power_ant0;
				p_dm_fat_table->ant_rssi_cnt[antsel_tr_mux]++;
			}
		} else { /* ant_div_type != CG_TRX_SMART_ANTDIV */
			if ((p_dm_odm->support_ic_type & ODM_ANTDIV_SUPPORT) && (p_pktinfo->is_packet_to_self || p_dm_fat_table->use_ctrl_frame_antdiv)) {

				if (p_dm_odm->ant_div_type == S0S1_SW_ANTDIV) {

					if (is_cck_rate)
						p_dm_fat_table->antsel_rx_keep_0 = (p_dm_fat_table->rx_idle_ant == MAIN_ANT) ? ANT1_2G : ANT2_2G;

						odm_antsel_statistics(p_dm_odm, p_dm_fat_table->antsel_rx_keep_0, p_pktinfo->station_id, rx_power_ant0, RSSI_METHOD, is_cck_rate);

					} else {

					odm_antsel_statistics(p_dm_odm, p_dm_fat_table->antsel_rx_keep_0, p_pktinfo->station_id, rx_power_ant0, RSSI_METHOD, is_cck_rate);

#ifdef ODM_EVM_ENHANCE_ANTDIV
					if (p_dm_odm->support_ic_type == ODM_RTL8192E) {
						if (!is_cck_rate)
							odm_antsel_statistics(p_dm_odm, p_dm_fat_table->antsel_rx_keep_0, p_pktinfo->station_id, rx_evm_ant0, EVM_METHOD, is_cck_rate);

					}
#endif
				}
			}
		}
	/* ODM_RT_TRACE(p_dm_odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("is_cck_rate=%d, PWDB_ALL=%d\n",is_cck_rate, p_phy_info->rx_pwdb_all)); */
	/* ODM_RT_TRACE(p_dm_odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("antsel_tr_mux=3'b%d%d%d\n",p_dm_fat_table->antsel_rx_keep_2, p_dm_fat_table->antsel_rx_keep_1, p_dm_fat_table->antsel_rx_keep_0)); */
}

void
odm_set_tx_ant_by_tx_info(
	void			*p_dm_void,
	u8			*p_desc,
	u8			mac_id

)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _FAST_ANTENNA_TRAINNING_	*p_dm_fat_table = &p_dm_odm->dm_fat_table;

	if (!(p_dm_odm->support_ability & ODM_BB_ANT_DIV))
		return;

	if (p_dm_odm->ant_div_type == CGCS_RX_HW_ANTDIV)
		return;


	if (p_dm_odm->support_ic_type == ODM_RTL8723B) {
#if (RTL8723B_SUPPORT == 1)
		SET_TX_DESC_ANTSEL_A_8723B(p_desc, p_dm_fat_table->antsel_a[mac_id]);
		/*ODM_RT_TRACE(p_dm_odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[8723B] SetTxAntByTxInfo_WIN: mac_id=%d, antsel_tr_mux=3'b%d%d%d\n",
			mac_id, p_dm_fat_table->antsel_c[mac_id], p_dm_fat_table->antsel_b[mac_id], p_dm_fat_table->antsel_a[mac_id]));*/
#endif
	} else if (p_dm_odm->support_ic_type == ODM_RTL8821) {
#if (RTL8821A_SUPPORT == 1)
		SET_TX_DESC_ANTSEL_A_8812(p_desc, p_dm_fat_table->antsel_a[mac_id]);
		/*ODM_RT_TRACE(p_dm_odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[8821A] SetTxAntByTxInfo_WIN: mac_id=%d, antsel_tr_mux=3'b%d%d%d\n",
			mac_id, p_dm_fat_table->antsel_c[mac_id], p_dm_fat_table->antsel_b[mac_id], p_dm_fat_table->antsel_a[mac_id]));*/
#endif
	} else if (p_dm_odm->support_ic_type == ODM_RTL8188E) {
#if (RTL8188E_SUPPORT == 1)
		SET_TX_DESC_ANTSEL_A_88E(p_desc, p_dm_fat_table->antsel_a[mac_id]);
		SET_TX_DESC_ANTSEL_B_88E(p_desc, p_dm_fat_table->antsel_b[mac_id]);
		SET_TX_DESC_ANTSEL_C_88E(p_desc, p_dm_fat_table->antsel_c[mac_id]);
		/*ODM_RT_TRACE(p_dm_odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[8188E] SetTxAntByTxInfo_WIN: mac_id=%d, antsel_tr_mux=3'b%d%d%d\n",
			mac_id, p_dm_fat_table->antsel_c[mac_id], p_dm_fat_table->antsel_b[mac_id], p_dm_fat_table->antsel_a[mac_id]));*/
#endif
	} else if (p_dm_odm->support_ic_type == ODM_RTL8821C) {
#if (RTL8821C_SUPPORT == 1)
		SET_TX_DESC_ANTSEL_A_8821C(p_desc, p_dm_fat_table->antsel_a[mac_id]);
		/*ODM_RT_TRACE(p_dm_odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[8821C] SetTxAntByTxInfo_WIN: mac_id=%d, antsel_tr_mux=3'b%d%d%d\n",
			mac_id, p_dm_fat_table->antsel_c[mac_id], p_dm_fat_table->antsel_b[mac_id], p_dm_fat_table->antsel_a[mac_id]));*/
#endif
	}
}

void
odm_ant_div_config(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _FAST_ANTENNA_TRAINNING_			*p_dm_fat_table = &p_dm_odm->dm_fat_table;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("CE Config Antenna Diversity\n"));

	if (p_dm_odm->support_ic_type == ODM_RTL8723B)
		p_dm_odm->ant_div_type = S0S1_SW_ANTDIV;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[AntDiv Config Info] AntDiv_SupportAbility = (( %x ))\n", ((p_dm_odm->support_ability & ODM_BB_ANT_DIV) ? 1 : 0)));
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("[AntDiv Config Info] be_fix_tx_ant = ((%d))\n", p_dm_odm->dm_fat_table.b_fix_tx_ant));

}


void
odm_ant_div_timers(
	void		*p_dm_void,
	u8		state
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	if (state == INIT_ANTDIV_TIMMER) {
#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
		odm_initialize_timer(p_dm_odm, &(p_dm_odm->dm_swat_table.phydm_sw_antenna_switch_timer),
			(void *)odm_sw_antdiv_callback, NULL, "phydm_sw_antenna_switch_timer");
#else
		timer_setup(&p_dm_odm->dm_swat_table.phydm_sw_antenna_switch_timer, odm_sw_antdiv_callback, 0);
#endif
#elif (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
		odm_initialize_timer(p_dm_odm, &p_dm_odm->fast_ant_training_timer,
			(void *)odm_fast_ant_training_callback, NULL, "fast_ant_training_timer");
#else
		timer_setup(&p_dm_odm->fast_ant_training_timer, odm_fast_ant_training_callback, 0);
#endif
#endif

#ifdef ODM_EVM_ENHANCE_ANTDIV
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
		odm_initialize_timer(p_dm_odm, &p_dm_odm->evm_fast_ant_training_timer,
			(void *)odm_evm_fast_ant_training_callback, NULL, "evm_fast_ant_training_timer");
#else
		timer_setup(&p_dm_odm->evm_fast_ant_training_timer, odm_evm_fast_ant_training_callback, 0);
#endif
#endif
	} else if (state == CANCEL_ANTDIV_TIMMER) {
#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
		odm_cancel_timer(p_dm_odm, &(p_dm_odm->dm_swat_table.phydm_sw_antenna_switch_timer));
#elif (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
		odm_cancel_timer(p_dm_odm, &p_dm_odm->fast_ant_training_timer);
#endif

#ifdef ODM_EVM_ENHANCE_ANTDIV
		odm_cancel_timer(p_dm_odm, &p_dm_odm->evm_fast_ant_training_timer);
#endif
	} else if (state == RELEASE_ANTDIV_TIMMER) {
#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
		odm_release_timer(p_dm_odm, &(p_dm_odm->dm_swat_table.phydm_sw_antenna_switch_timer));
#elif (defined(CONFIG_5G_CG_SMART_ANT_DIVERSITY)) || (defined(CONFIG_2G_CG_SMART_ANT_DIVERSITY))
		odm_release_timer(p_dm_odm, &p_dm_odm->fast_ant_training_timer);
#endif

#ifdef ODM_EVM_ENHANCE_ANTDIV
		odm_release_timer(p_dm_odm, &p_dm_odm->evm_fast_ant_training_timer);
#endif
	}

}

void
phydm_antdiv_debug(
	void		*p_dm_void,
	u32		*const dm_value,
	u32		*_used,
	char			*output,
	u32		*_out_len
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	/*struct _FAST_ANTENNA_TRAINNING_*			p_dm_fat_table = &p_dm_odm->dm_fat_table;*/
	u32 used = *_used;
	u32 out_len = *_out_len;

	if (dm_value[0] == 1) { /*fixed or auto antenna*/

		if (dm_value[1] == 0) {
			p_dm_odm->antdiv_select = 0;
			PHYDM_SNPRINTF((output + used, out_len - used, "AntDiv: Auto\n"));
		} else if (dm_value[1] == 1) {
			p_dm_odm->antdiv_select = 1;
			PHYDM_SNPRINTF((output + used, out_len - used, "AntDiv: Fix MAin\n"));
		} else if (dm_value[1] == 2) {
			p_dm_odm->antdiv_select = 2;
			PHYDM_SNPRINTF((output + used, out_len - used, "AntDiv: Fix Aux\n"));
		}
	} else if (dm_value[0] == 2) { /*dynamic period for AntDiv*/

		p_dm_odm->antdiv_period = (u8)dm_value[1];
		PHYDM_SNPRINTF((output + used, out_len - used, "AntDiv_period = ((%d))\n", p_dm_odm->antdiv_period));
	}
}

#endif /*#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))*/

void
odm_ant_div_reset(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	if (p_dm_odm->ant_div_type == S0S1_SW_ANTDIV) {
#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
		odm_s0s1_sw_ant_div_reset(p_dm_odm);
#endif
	}

}

void
odm_antenna_diversity_init(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
	odm_ant_div_config(p_dm_odm);
	odm_ant_div_init(p_dm_odm);
#endif
}

void
odm_antenna_diversity(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	if (p_dm_odm->mp_mode == true)
		return;

#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
	odm_ant_div(p_dm_odm);
#endif
}
