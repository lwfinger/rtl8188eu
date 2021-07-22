// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


/* ************************************************************
 * include files
 * ************************************************************ */
#include "mp_precomp.h"
#include "phydm_precomp.h"

#if (CONFIG_DYNAMIC_RX_PATH == 1)

void
phydm_process_phy_status_for_dynamic_rx_path(
	void			*p_dm_void,
	void			*p_phy_info_void,
	void			*p_pkt_info_void
)
{
	struct PHY_DM_STRUCT				*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _odm_phy_status_info_		*p_phy_info = (struct _odm_phy_status_info_ *)p_phy_info_void;
	struct _odm_per_pkt_info_		*p_pktinfo = (struct _odm_per_pkt_info_ *)p_pkt_info_void;
	struct _DYNAMIC_RX_PATH_					*p_dm_drp_table	= &(p_dm_odm->dm_drp_table);
	/*u8					is_cck_rate=0;*/



}

void
phydm_drp_get_statistic(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT					*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _DYNAMIC_RX_PATH_						*p_dm_drp_table = &(p_dm_odm->dm_drp_table);
	struct false_ALARM_STATISTICS		*false_alm_cnt = (struct false_ALARM_STATISTICS *)phydm_get_structure(p_dm_odm, PHYDMfalseALMCNT);

	odm_false_alarm_counter_statistics(p_dm_odm);

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("[CCA Cnt] {CCK, OFDM, Total} = {%d, %d, %d}\n",
		false_alm_cnt->cnt_cck_cca, false_alm_cnt->cnt_ofdm_cca, false_alm_cnt->cnt_cca_all));

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("[FA Cnt] {CCK, OFDM, Total} = {%d, %d, %d}\n",
		false_alm_cnt->cnt_cck_fail, false_alm_cnt->cnt_ofdm_fail, false_alm_cnt->cnt_all));
}

void
phydm_dynamic_rx_path(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT				*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _DYNAMIC_RX_PATH_					*p_dm_drp_table	= &(p_dm_odm->dm_drp_table);
	u8		training_set_timmer_en;
	u8		curr_drp_state;
	u32		rx_ok_cal;
	u32		RSSI = 0;
	struct false_ALARM_STATISTICS		*false_alm_cnt = (struct false_ALARM_STATISTICS *)phydm_get_structure(p_dm_odm, PHYDMfalseALMCNT);

	if (!(p_dm_odm->support_ability & ODM_BB_DYNAMIC_RX_PATH)) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("[Return Init]   Not Support Dynamic RX PAth\n"));
		return;
	}

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("Current drp_state = ((%d))\n", p_dm_drp_table->drp_state));

	curr_drp_state = p_dm_drp_table->drp_state;

	if (p_dm_drp_table->drp_state == DRP_INIT_STATE) {

		phydm_drp_get_statistic(p_dm_odm);

		if (false_alm_cnt->cnt_crc32_ok_all > 20) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("[Stop DRP Training] cnt_crc32_ok_all = ((%d))\n", false_alm_cnt->cnt_crc32_ok_all));
			p_dm_drp_table->drp_state  = DRP_INIT_STATE;
			training_set_timmer_en = false;
		} else {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("[Start DRP Training] cnt_crc32_ok_all = ((%d))\n", false_alm_cnt->cnt_crc32_ok_all));
			p_dm_drp_table->drp_state  = DRP_TRAINING_STATE_0;
			p_dm_drp_table->curr_rx_path = PHYDM_AB;
			training_set_timmer_en = true;
		}

	} else if (p_dm_drp_table->drp_state == DRP_TRAINING_STATE_0) {

		phydm_drp_get_statistic(p_dm_odm);

		p_dm_drp_table->curr_cca_all_cnt_0 = false_alm_cnt->cnt_cca_all;
		p_dm_drp_table->curr_fa_all_cnt_0 = false_alm_cnt->cnt_all;

		p_dm_drp_table->drp_state  = DRP_TRAINING_STATE_1;
		p_dm_drp_table->curr_rx_path = PHYDM_B;
		training_set_timmer_en = true;

	} else if (p_dm_drp_table->drp_state == DRP_TRAINING_STATE_1) {
		phydm_drp_get_statistic(p_dm_odm);
		p_dm_drp_table->curr_cca_all_cnt_1 = false_alm_cnt->cnt_cca_all;
		p_dm_drp_table->curr_fa_all_cnt_1 = false_alm_cnt->cnt_all;
		p_dm_drp_table->drp_state  = DRP_DECISION_STATE;
	} else if (p_dm_drp_table->drp_state == DRP_TRAINING_STATE_2) {

		phydm_drp_get_statistic(p_dm_odm);

		p_dm_drp_table->curr_cca_all_cnt_2 = false_alm_cnt->cnt_cca_all;
		p_dm_drp_table->curr_fa_all_cnt_2 = false_alm_cnt->cnt_all;
		p_dm_drp_table->drp_state  = DRP_DECISION_STATE;
	}

	if (p_dm_drp_table->drp_state == DRP_DECISION_STATE) {

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("Current drp_state = ((%d))\n", p_dm_drp_table->drp_state));

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("[0] {CCA, FA} = {%d, %d}\n", p_dm_drp_table->curr_cca_all_cnt_0, p_dm_drp_table->curr_fa_all_cnt_0));
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("[1] {CCA, FA} = {%d, %d}\n", p_dm_drp_table->curr_cca_all_cnt_1, p_dm_drp_table->curr_fa_all_cnt_1));
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("[2] {CCA, FA} = {%d, %d}\n", p_dm_drp_table->curr_cca_all_cnt_2, p_dm_drp_table->curr_fa_all_cnt_2));

		if (p_dm_drp_table->curr_fa_all_cnt_1 < p_dm_drp_table->curr_fa_all_cnt_0) {

			if ((p_dm_drp_table->curr_fa_all_cnt_0 - p_dm_drp_table->curr_fa_all_cnt_1) > p_dm_drp_table->fa_diff_threshold)
				p_dm_drp_table->curr_rx_path = PHYDM_B;
			else
				p_dm_drp_table->curr_rx_path = PHYDM_AB;
		} else
			p_dm_drp_table->curr_rx_path = PHYDM_AB;

		phydm_config_ofdm_rx_path(p_dm_odm, p_dm_drp_table->curr_rx_path);
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("[Training Result]  curr_rx_path = ((%s%s)),\n",
			((p_dm_drp_table->curr_rx_path & PHYDM_A)  ? "A"  : " "), ((p_dm_drp_table->curr_rx_path & PHYDM_B)  ? "B"  : " ")));

		p_dm_drp_table->drp_state = DRP_INIT_STATE;
		training_set_timmer_en = false;
	}

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("DRP_state: ((%d)) -> ((%d))\n", curr_drp_state, p_dm_drp_table->drp_state));

	if (training_set_timmer_en) {

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("[Training en]  curr_rx_path = ((%s%s)), training_time = ((%d ms))\n",
			((p_dm_drp_table->curr_rx_path & PHYDM_A)  ? "A"  : " "), ((p_dm_drp_table->curr_rx_path & PHYDM_B)  ? "B"  : " "), p_dm_drp_table->training_time));

		phydm_config_ofdm_rx_path(p_dm_odm, p_dm_drp_table->curr_rx_path);
		odm_set_timer(p_dm_odm, &(p_dm_drp_table->phydm_dynamic_rx_path_timer), p_dm_drp_table->training_time); /*ms*/
	} else
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("DRP period end\n\n", curr_drp_state, p_dm_drp_table->drp_state));

}

void
phydm_dynamic_rx_path_callback(
	void *function_context
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)function_context;
	struct _ADAPTER	*padapter = p_dm_odm->adapter;

	if (padapter->net_closed == true)
		return;
}

void
phydm_dynamic_rx_path_timers(
	void		*p_dm_void,
	u8		state
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _DYNAMIC_RX_PATH_			*p_dm_drp_table	= &(p_dm_odm->dm_drp_table);

	if (state == INIT_DRP_TIMMER) {

		odm_initialize_timer(p_dm_odm, &(p_dm_drp_table->phydm_dynamic_rx_path_timer),
			(void *)phydm_dynamic_rx_path_callback, NULL, "phydm_sw_antenna_switch_timer");
	} else if (state == CANCEL_DRP_TIMMER)

		odm_cancel_timer(p_dm_odm, &(p_dm_drp_table->phydm_dynamic_rx_path_timer));

	else if (state == RELEASE_DRP_TIMMER)

		odm_release_timer(p_dm_odm, &(p_dm_drp_table->phydm_dynamic_rx_path_timer));

}

void
phydm_dynamic_rx_path_init(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT				*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _DYNAMIC_RX_PATH_					*p_dm_drp_table	= &(p_dm_odm->dm_drp_table);
	bool			ret_value;

	if (!(p_dm_odm->support_ability & ODM_BB_DYNAMIC_RX_PATH)) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("[Return]   Not Support Dynamic RX PAth\n"));
		return;
	}
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_DYNAMIC_RX_PATH, ODM_DBG_LOUD, ("phydm_dynamic_rx_path_init\n"));

	p_dm_drp_table->drp_state = DRP_INIT_STATE;
	p_dm_drp_table->rssi_threshold = DRP_RSSI_TH;
	p_dm_drp_table->fa_count_thresold = 50;
	p_dm_drp_table->fa_diff_threshold = 50;
	p_dm_drp_table->training_time = 100; /*ms*/
	p_dm_drp_table->drp_skip_counter = 0;
	p_dm_drp_table->drp_period  = 0;
	p_dm_drp_table->drp_init_finished = true;

	ret_value = phydm_api_trx_mode(p_dm_odm, (enum odm_rf_path_e)(ODM_RF_A | ODM_RF_B), (enum odm_rf_path_e)(ODM_RF_A | ODM_RF_B), true);

}

void
phydm_drp_debug(
	void		*p_dm_void,
	u32		*const dm_value,
	u32		*_used,
	char			*output,
	u32		*_out_len
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32			used = *_used;
	u32			out_len = *_out_len;
	struct _DYNAMIC_RX_PATH_			*p_dm_drp_table = &(p_dm_odm->dm_drp_table);

	switch (dm_value[0])	{

	case DRP_TRAINING_TIME:
		p_dm_drp_table->training_time = (u16)dm_value[1];
		break;
	case DRP_TRAINING_PERIOD:
		p_dm_drp_table->drp_period = (u8)dm_value[1];
		break;
	case DRP_RSSI_THRESHOLD:
		p_dm_drp_table->rssi_threshold = (u8)dm_value[1];
		break;
	case DRP_FA_THRESHOLD:
		p_dm_drp_table->fa_count_thresold = dm_value[1];
		break;
	case DRP_FA_DIFF_THRESHOLD:
		p_dm_drp_table->fa_diff_threshold = dm_value[1];
		break;
	default:
		PHYDM_SNPRINTF((output + used, out_len - used, "[DRP] unknown command\n"));
		break;
}
}

void
phydm_dynamic_rx_path_caller(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _DYNAMIC_RX_PATH_			*p_dm_drp_table	= &(p_dm_odm->dm_drp_table);

	if (p_dm_drp_table->drp_skip_counter <  p_dm_drp_table->drp_period)
		p_dm_drp_table->drp_skip_counter++;
	else
		p_dm_drp_table->drp_skip_counter = 0;

	if (p_dm_drp_table->drp_skip_counter != 0)
		return;

	if (p_dm_drp_table->drp_init_finished != true)
		return;

	phydm_dynamic_rx_path(p_dm_odm);

}
#endif
