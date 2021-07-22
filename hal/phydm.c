// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


/* ************************************************************
 * include files
 * ************************************************************ */

#include "mp_precomp.h"
#include "phydm_precomp.h"

static const u16 db_invert_table[12][8] = {
	{	1,		1,		1,		2,		2,		2,		2,		3},
	{	3,		3,		4,		4,		4,		5,		6,		6},
	{	7,		8,		9,		10,		11,		13,		14,		16},
	{	18,		20,		22,		25,		28,		32,		35,		40},
	{	45,		50,		56,		63,		71,		79,		89,		100},
	{	112,		126,		141,		158,		178,		200,		224,		251},
	{	282,		316,		355,		398,		447,		501,		562,		631},
	{	708,		794,		891,		1000,	1122,	1259,	1413,	1585},
	{	1778,	1995,	2239,	2512,	2818,	3162,	3548,	3981},
	{	4467,	5012,	5623,	6310,	7079,	7943,	8913,	10000},
	{	11220,	12589,	14125,	15849,	17783,	19953,	22387,	25119},
	{	28184,	31623,	35481,	39811,	44668,	50119,	56234,	65535}
};


/* ************************************************************
 * Local Function predefine.
 * ************************************************************ */

/* START------------COMMON INFO RELATED--------------- */

void
odm_global_adapter_check(
	void
);

/* move to odm_PowerTacking.h by YuChen */



void
odm_update_power_training_state(
	struct PHY_DM_STRUCT	*p_dm_odm
);

/* ************************************************************
 * 3 Export Interface
 * ************************************************************ */

/*Y = 10*log(X)*/
s32
odm_pwdb_conversion(
	s32 X,
	u32 total_bit,
	u32 decimal_bit
)
{
	s32 Y, integer = 0, decimal = 0;
	u32 i;

	if (X == 0)
		X = 1; /* log2(x), x can't be 0 */

	for (i = (total_bit - 1); i > 0; i--) {
		if (X & BIT(i)) {
			integer = i;
			if (i > 0)
				decimal = (X & BIT(i - 1)) ? 2 : 0; /* decimal is 0.5dB*3=1.5dB~=2dB */
			break;
		}
	}

	Y = 3 * (integer - decimal_bit) + decimal; /* 10*log(x)=3*log2(x), */

	return Y;
}

s32
odm_sign_conversion(
	s32 value,
	u32 total_bit
)
{
	if (value & BIT(total_bit - 1))
		value -= BIT(total_bit);
	return value;
}

void
phydm_seq_sorting(
	void	*p_dm_void,
	u32	*p_value,
	u32	*rank_idx,
	u32	*p_idx_out,
	u8	seq_length
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8		i = 0, j = 0;
	u32		tmp_a, tmp_b;
	u32		tmp_idx_a, tmp_idx_b;

	for (i = 0; i < seq_length; i++) {
		rank_idx[i] = i;
		/**/
	}

	for (i = 0; i < (seq_length - 1); i++) {

		for (j = 0; j < (seq_length - 1 - i); j++) {

			tmp_a = p_value[j];
			tmp_b = p_value[j + 1];

			tmp_idx_a = rank_idx[j];
			tmp_idx_b = rank_idx[j + 1];

			if (tmp_a < tmp_b) {
				p_value[j] = tmp_b;
				p_value[j + 1] = tmp_a;

				rank_idx[j] = tmp_idx_b;
				rank_idx[j + 1] = tmp_idx_a;
			}
		}
	}

	for (i = 0; i < seq_length; i++) {
		p_idx_out[rank_idx[i]] = i + 1;
		/**/
	}



}

void
odm_init_mp_driver_status(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
	struct _ADAPTER	*adapter =  p_dm_odm->adapter;

	/* Update information every period */
	p_dm_odm->mp_mode = (bool)adapter->registrypriv.mp_mode;

}

static void
odm_update_mp_driver_status(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
	struct _ADAPTER	*adapter =  p_dm_odm->adapter;

	/* Update information erery period */
	p_dm_odm->mp_mode = (bool)adapter->registrypriv.mp_mode;
}

static void
phydm_init_trx_antenna_setting(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
	if (p_dm_odm->support_ic_type & (ODM_RTL8814A)) {
		u8	rx_ant = 0, tx_ant = 0;

		rx_ant = (u8)odm_get_bb_reg(p_dm_odm, ODM_REG(BB_RX_PATH, p_dm_odm), ODM_BIT(BB_RX_PATH, p_dm_odm));
		tx_ant = (u8)odm_get_bb_reg(p_dm_odm, ODM_REG(BB_TX_PATH, p_dm_odm), ODM_BIT(BB_TX_PATH, p_dm_odm));
		p_dm_odm->tx_ant_status = (tx_ant & 0xf);
		p_dm_odm->rx_ant_status = (rx_ant & 0xf);
	} else if (p_dm_odm->support_ic_type & (ODM_RTL8723D | ODM_RTL8821C)) {
		p_dm_odm->tx_ant_status = 0x1;
		p_dm_odm->rx_ant_status = 0x1;

	}
}

static void
phydm_traffic_load_decision(
	void	*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _sw_antenna_switch_		*p_dm_swat_table = &p_dm_odm->dm_swat_table;

	/*---TP & Trafic-load calculation---*/

	if (p_dm_odm->last_tx_ok_cnt > (*(p_dm_odm->p_num_tx_bytes_unicast)))
		p_dm_odm->last_tx_ok_cnt = (*(p_dm_odm->p_num_tx_bytes_unicast));

	if (p_dm_odm->last_rx_ok_cnt > (*(p_dm_odm->p_num_rx_bytes_unicast)))
		p_dm_odm->last_rx_ok_cnt = (*(p_dm_odm->p_num_rx_bytes_unicast));

	p_dm_odm->cur_tx_ok_cnt =  *(p_dm_odm->p_num_tx_bytes_unicast) - p_dm_odm->last_tx_ok_cnt;
	p_dm_odm->cur_rx_ok_cnt =  *(p_dm_odm->p_num_rx_bytes_unicast) - p_dm_odm->last_rx_ok_cnt;
	p_dm_odm->last_tx_ok_cnt =  *(p_dm_odm->p_num_tx_bytes_unicast);
	p_dm_odm->last_rx_ok_cnt =  *(p_dm_odm->p_num_rx_bytes_unicast);

	p_dm_odm->tx_tp = ((p_dm_odm->tx_tp) >> 1) + (u32)(((p_dm_odm->cur_tx_ok_cnt) >> 18) >> 1); /* <<3(8bit), >>20(10^6,M), >>1(2sec)*/
	p_dm_odm->rx_tp = ((p_dm_odm->rx_tp) >> 1) + (u32)(((p_dm_odm->cur_rx_ok_cnt) >> 18) >> 1); /* <<3(8bit), >>20(10^6,M), >>1(2sec)*/
	p_dm_odm->total_tp = p_dm_odm->tx_tp + p_dm_odm->rx_tp;

	if (p_dm_odm->total_tp == 0)
		p_dm_odm->consecutive_idlel_time += PHYDM_WATCH_DOG_PERIOD;
	else
		p_dm_odm->consecutive_idlel_time = 0;

	p_dm_odm->pre_traffic_load = p_dm_odm->traffic_load;

	if (p_dm_odm->cur_tx_ok_cnt > 1875000 || p_dm_odm->cur_rx_ok_cnt > 1875000) {		/* ( 1.875M * 8bit ) / 2sec= 7.5M bits /sec )*/

		p_dm_odm->traffic_load = TRAFFIC_HIGH;
		/**/
	} else if (p_dm_odm->cur_tx_ok_cnt > 500000 || p_dm_odm->cur_rx_ok_cnt > 500000) { /*( 0.5M * 8bit ) / 2sec =  2M bits /sec )*/

		p_dm_odm->traffic_load = TRAFFIC_MID;
		/**/
	} else if (p_dm_odm->cur_tx_ok_cnt > 100000 || p_dm_odm->cur_rx_ok_cnt > 100000)  { /*( 0.1M * 8bit ) / 2sec =  0.4M bits /sec )*/

		p_dm_odm->traffic_load = TRAFFIC_LOW;
		/**/
	} else {

		p_dm_odm->traffic_load = TRAFFIC_ULTRA_LOW;
		/**/
	}
}

static void
phydm_config_ofdm_tx_path(
	struct PHY_DM_STRUCT		*p_dm_odm,
	u32			path
)
{
	u8	ofdm_tx_path = 0x33;
}

void
phydm_config_ofdm_rx_path(
	struct PHY_DM_STRUCT		*p_dm_odm,
	u32			path
)
{
	u8	ofdm_rx_path = 0;


	if (p_dm_odm->support_ic_type & (ODM_RTL8192E)) {
	}
}

static void
phydm_config_cck_rx_antenna_init(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
}

static void
phydm_config_cck_rx_path(
	struct PHY_DM_STRUCT		*p_dm_odm,
	u8			path,
	u8			path_div_en
)
{
	u8	path_div_select = 0;
	u8	cck_1_path = 0, cck_2_path = 0;
}

void
phydm_config_trx_path(
	void		*p_dm_void,
	u32		*const dm_value,
	u32		*_used,
	char			*output,
	u32		*_out_len
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32			pre_support_ability;
	u32 used = *_used;
	u32 out_len = *_out_len;

	/* CCK */
	if (dm_value[0] == 0) {

		if (dm_value[1] == 1) { /*TX*/
			if (dm_value[2] == 1)
				odm_set_bb_reg(p_dm_odm, 0xa04, 0xf0000000, 0x8);
			else if (dm_value[2] == 2)
				odm_set_bb_reg(p_dm_odm, 0xa04, 0xf0000000, 0x4);
			else if (dm_value[2] == 3)
				odm_set_bb_reg(p_dm_odm, 0xa04, 0xf0000000, 0xc);
		} else if (dm_value[1] == 2) { /*RX*/

			phydm_config_cck_rx_antenna_init(p_dm_odm);

			if (dm_value[2] == 1)
				phydm_config_cck_rx_path(p_dm_odm, PHYDM_A, CCA_PATHDIV_DISABLE);
			else  if (dm_value[2] == 2)
				phydm_config_cck_rx_path(p_dm_odm, PHYDM_B, CCA_PATHDIV_DISABLE);
			else  if (dm_value[2] == 3) {
				if (dm_value[3] == 1) /*enable path diversity*/
					phydm_config_cck_rx_path(p_dm_odm, PHYDM_AB, CCA_PATHDIV_ENABLE);
				else
					phydm_config_cck_rx_path(p_dm_odm, PHYDM_B, CCA_PATHDIV_DISABLE);
			}
		}
	}
	/* OFDM */
	else if (dm_value[0] == 1) {

		if (dm_value[1] == 1) { /*TX*/
			phydm_config_ofdm_tx_path(p_dm_odm, dm_value[2]);
			/**/
		} else if (dm_value[1] == 2) { /*RX*/
			phydm_config_ofdm_rx_path(p_dm_odm, dm_value[2]);
			/**/
		}
	}

	PHYDM_SNPRINTF((output + used, out_len - used, "PHYDM Set path [%s] [%s] = [%s%s%s%s]\n",
			(dm_value[0] == 1) ? "OFDM" : "CCK",
			(dm_value[1] == 1) ? "TX" : "RX",
			(dm_value[2] & 0x1) ? "A" : "",
			(dm_value[2] & 0x2) ? "B" : "",
			(dm_value[2] & 0x4) ? "C" : "",
			(dm_value[2] & 0x8) ? "D" : ""
		       ));

}

static void
phydm_init_cck_setting(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
	u32 value_824, value_82c;

	p_dm_odm->is_cck_high_power = (bool) odm_get_bb_reg(p_dm_odm, ODM_REG(CCK_RPT_FORMAT, p_dm_odm), ODM_BIT(CCK_RPT_FORMAT, p_dm_odm));

	p_dm_odm->cck_new_agc = false;
}

static void
phydm_init_soft_ml_setting(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
}

static void
phydm_init_hw_info_by_rfe(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
}

static void
odm_common_info_self_init(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
	phydm_init_cck_setting(p_dm_odm);
	p_dm_odm->rf_path_rx_enable = (u8) odm_get_bb_reg(p_dm_odm, ODM_REG(BB_RX_PATH, p_dm_odm), ODM_BIT(BB_RX_PATH, p_dm_odm));
	odm_init_mp_driver_status(p_dm_odm);
	phydm_init_trx_antenna_setting(p_dm_odm);
	phydm_init_soft_ml_setting(p_dm_odm);

	p_dm_odm->phydm_period = PHYDM_WATCH_DOG_PERIOD;
	p_dm_odm->phydm_sys_up_time = 0;

	if (p_dm_odm->support_ic_type & ODM_IC_1SS)
		p_dm_odm->num_rf_path = 1;
	else if (p_dm_odm->support_ic_type & ODM_IC_2SS)
		p_dm_odm->num_rf_path = 2;
	else if (p_dm_odm->support_ic_type & ODM_IC_3SS)
		p_dm_odm->num_rf_path = 3;
	else if (p_dm_odm->support_ic_type & ODM_IC_4SS)
		p_dm_odm->num_rf_path = 4;

	p_dm_odm->tx_rate = 0xFF;

	p_dm_odm->number_linked_client = 0;
	p_dm_odm->pre_number_linked_client = 0;
	p_dm_odm->number_active_client = 0;
	p_dm_odm->pre_number_active_client = 0;

	p_dm_odm->last_tx_ok_cnt = 0;
	p_dm_odm->last_rx_ok_cnt = 0;
	p_dm_odm->tx_tp = 0;
	p_dm_odm->rx_tp = 0;
	p_dm_odm->total_tp = 0;
	p_dm_odm->traffic_load = TRAFFIC_LOW;

	p_dm_odm->nbi_set_result = 0;
	p_dm_odm->is_init_hw_info_by_rfe = false;

}

static void
odm_common_info_self_update(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
	u8	entry_cnt = 0, num_active_client = 0;
	u32	i, one_entry_macid = 0, ma_rx_tp = 0;
	struct sta_info	*p_entry;

	if (*(p_dm_odm->p_band_width) == ODM_BW40M) {
		if (*(p_dm_odm->p_sec_ch_offset) == 1)
			p_dm_odm->control_channel = *(p_dm_odm->p_channel) - 2;
		else if (*(p_dm_odm->p_sec_ch_offset) == 2)
			p_dm_odm->control_channel = *(p_dm_odm->p_channel) + 2;
	} else
		p_dm_odm->control_channel = *(p_dm_odm->p_channel);

	for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
		p_entry = p_dm_odm->p_odm_sta_info[i];
		if (IS_STA_VALID(p_entry)) {
			entry_cnt++;
			if (entry_cnt == 1)
				one_entry_macid = i;
		}
	}

	if (entry_cnt == 1) {
		p_dm_odm->is_one_entry_only = true;
		p_dm_odm->one_entry_macid = one_entry_macid;
	} else
		p_dm_odm->is_one_entry_only = false;

	p_dm_odm->pre_number_linked_client = p_dm_odm->number_linked_client;
	p_dm_odm->pre_number_active_client = p_dm_odm->number_active_client;

	p_dm_odm->number_linked_client = entry_cnt;
	p_dm_odm->number_active_client = num_active_client;

	/* Update MP driver status*/
	odm_update_mp_driver_status(p_dm_odm);

	/*Traffic load information update*/
	phydm_traffic_load_decision(p_dm_odm);

	p_dm_odm->phydm_sys_up_time += p_dm_odm->phydm_period;
}

static void
odm_common_info_self_reset(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
	p_dm_odm->phy_dbg_info.num_qry_beacon_pkt = 0;
}

void *
phydm_get_structure(
	struct PHY_DM_STRUCT		*p_dm_odm,
	u8			structure_type
)

{
	void	*p_struct = NULL;

	switch (structure_type) {
	case	PHYDMfalseALMCNT:
		p_struct = &(p_dm_odm->false_alm_cnt);
		break;
	case	PHYDM_CFOTRACK:
		p_struct = &(p_dm_odm->dm_cfo_track);
		break;
	case	PHYDM_ADAPTIVITY:
		p_struct = &(p_dm_odm->adaptivity);
		break;
	default:
		break;
	}

	return	p_struct;
}

static void
odm_hw_setting(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
}
#if SUPPORTABLITY_PHYDMLIZE
static void
phydm_supportability_init(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32			support_ability = 0;

	if (p_dm_odm->support_ic_type != ODM_RTL8821C)
		return;

	switch (p_dm_odm->support_ic_type) {

	/*---------------N Series--------------------*/
	case	ODM_RTL8188E:
		support_ability |=
			ODM_BB_DIG			|
			ODM_BB_RA_MASK		|
			ODM_BB_DYNAMIC_TXPWR	|
			ODM_BB_FA_CNT			|
			ODM_BB_RSSI_MONITOR	|
			ODM_BB_CCK_PD			|
			ODM_RF_TX_PWR_TRACK	|
			ODM_RF_RX_GAIN_TRACK	|
			ODM_RF_CALIBRATION		|
			ODM_BB_CFO_TRACKING	|
			ODM_BB_NHM_CNT		|
			ODM_BB_PRIMARY_CCA;
		break;

	case	ODM_RTL8192E:
		support_ability |=
			ODM_BB_DIG			|
			ODM_RF_TX_PWR_TRACK	|
			ODM_BB_RA_MASK		|
			ODM_BB_FA_CNT			|
			ODM_BB_RSSI_MONITOR	|
			ODM_BB_CFO_TRACKING	|
			/*				ODM_BB_PWR_TRAIN		|*/
			ODM_BB_NHM_CNT		|
			ODM_BB_PRIMARY_CCA;
		break;

	case	ODM_RTL8723B:
		support_ability |=
			ODM_BB_DIG			|
			ODM_BB_RA_MASK		|
			ODM_BB_FA_CNT			|
			ODM_BB_RSSI_MONITOR	|
			ODM_BB_CCK_PD			|
			ODM_RF_TX_PWR_TRACK	|
			ODM_RF_RX_GAIN_TRACK	|
			ODM_RF_CALIBRATION		|
			ODM_BB_CFO_TRACKING	|
			/*				ODM_BB_PWR_TRAIN		|*/
			ODM_BB_NHM_CNT;
		break;

	case	ODM_RTL8703B:
		support_ability |=
			ODM_BB_DIG			|
			ODM_BB_RA_MASK		|
			ODM_BB_FA_CNT			|
			ODM_BB_RSSI_MONITOR	|
			ODM_BB_CCK_PD			|
			ODM_BB_CFO_TRACKING	|
			/* ODM_BB_PWR_TRAIN	| */
			ODM_BB_NHM_CNT		|
			ODM_RF_TX_PWR_TRACK	|
			/* ODM_RF_RX_GAIN_TRACK	| */
			ODM_RF_CALIBRATION;
		break;

	case	ODM_RTL8723D:
		support_ability |=
			ODM_BB_DIG				|
			ODM_BB_RA_MASK		|
			ODM_BB_FA_CNT			|
			ODM_BB_RSSI_MONITOR	|
			ODM_BB_CCK_PD			|
			ODM_BB_CFO_TRACKING	|
			/* ODM_BB_PWR_TRAIN	| */
			ODM_BB_NHM_CNT		|
			ODM_RF_TX_PWR_TRACK;
			/* ODM_RF_RX_GAIN_TRACK	| */
			/* ODM_RF_CALIBRATION	| */
		break;

	case	ODM_RTL8188F:
		support_ability |=
			ODM_BB_DIG			|
			ODM_BB_RA_MASK		|
			ODM_BB_FA_CNT			|
			ODM_BB_RSSI_MONITOR	|
			ODM_BB_CCK_PD			|
			ODM_BB_CFO_TRACKING	|
			ODM_BB_NHM_CNT		|
			ODM_RF_TX_PWR_TRACK	|
			ODM_RF_CALIBRATION;
		break;
	/*---------------AC Series-------------------*/

	case	ODM_RTL8812:
	case	ODM_RTL8821:
		support_ability |=
			ODM_BB_DIG			|
			ODM_BB_FA_CNT			|
			ODM_BB_RSSI_MONITOR	|
			ODM_BB_RA_MASK		|
			ODM_RF_TX_PWR_TRACK	|
			ODM_BB_CFO_TRACKING	|
			/*				ODM_BB_PWR_TRAIN		|*/
			ODM_BB_DYNAMIC_TXPWR	|
			ODM_BB_NHM_CNT;
		break;

	case ODM_RTL8814A:
		support_ability |=
			ODM_BB_DIG				|
			ODM_BB_FA_CNT			|
			ODM_BB_RSSI_MONITOR	|
			ODM_BB_RA_MASK		|
			ODM_RF_TX_PWR_TRACK	|
			ODM_BB_CCK_PD			|
			ODM_BB_CFO_TRACKING	|
			ODM_BB_DYNAMIC_TXPWR	|
			ODM_BB_NHM_CNT;
		break;

	case ODM_RTL8822B:
		support_ability |=
			ODM_BB_DIG				|
			ODM_BB_FA_CNT			|
			ODM_BB_CCK_PD			|
			ODM_BB_CFO_TRACKING	|
			ODM_BB_RATE_ADAPTIVE	|
			ODM_BB_RSSI_MONITOR	|
			ODM_BB_RA_MASK		|
			ODM_RF_TX_PWR_TRACK;
		break;

	case ODM_RTL8821C:
		support_ability |=
			ODM_BB_DIG			|
			ODM_BB_RA_MASK		|
			ODM_BB_CCK_PD			|
			ODM_BB_FA_CNT			|
			ODM_BB_RSSI_MONITOR	|
			ODM_BB_RATE_ADAPTIVE	|
			ODM_RF_TX_PWR_TRACK	|
			ODM_MAC_EDCA_TURBO	|
			ODM_BB_CFO_TRACKING;
		break;
	default:
		dbg_print("[Warning] Supportability Init error !!!\n");
		break;

	}

	if (*(p_dm_odm->p_enable_antdiv))
		support_ability |= ODM_BB_ANT_DIV;

	if (*(p_dm_odm->p_enable_adaptivity)) {

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_INIT, ODM_DBG_LOUD, ("ODM adaptivity is set to Enabled!!!\n"));

		support_ability |= ODM_BB_ADAPTIVITY;
	} else {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_INIT, ODM_DBG_LOUD, ("ODM adaptivity is set to disnabled!!!\n"));
		/**/
	}
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_INIT, ODM_DBG_LOUD, ("PHYDM support_ability = ((0x%x))\n", support_ability));
	odm_cmn_info_init(p_dm_odm, ODM_CMNINFO_ABILITY, support_ability);
}
#endif

/*
 * 2011/09/21 MH Add to describe different team necessary resource allocate??
 *   */
void
odm_dm_init(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
#if SUPPORTABLITY_PHYDMLIZE
	phydm_supportability_init(p_dm_odm);
#endif
	odm_common_info_self_init(p_dm_odm);
	odm_dig_init(p_dm_odm);
	phydm_nhm_counter_statistics_init(p_dm_odm);
	phydm_adaptivity_init(p_dm_odm);
	phydm_ra_info_init(p_dm_odm);
	odm_rate_adaptive_mask_init(p_dm_odm);
	odm_cfo_tracking_init(p_dm_odm);
#if PHYDM_SUPPORT_EDCA
	odm_edca_turbo_init(p_dm_odm);
#endif
	odm_rssi_monitor_init(p_dm_odm);
	phydm_rf_init(p_dm_odm);
	odm_txpowertracking_init(p_dm_odm);

	odm_antenna_diversity_init(p_dm_odm);
#if (CONFIG_DYNAMIC_RX_PATH == 1)
	phydm_dynamic_rx_path_init(p_dm_odm);
#endif
	odm_auto_channel_select_init(p_dm_odm);
	odm_path_diversity_init(p_dm_odm);
	odm_dynamic_tx_power_init(p_dm_odm);
	phydm_init_ra_info(p_dm_odm);
#if (PHYDM_LA_MODE_SUPPORT == 1)
	adc_smp_init(p_dm_odm);
#endif


#ifdef BEAMFORMING_VERSION_1
	if (p_hal_data->beamforming_version == BEAMFORMING_VERSION_1)
#endif
	{
		phydm_beamforming_init(p_dm_odm);
	}

	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES) {
#if (defined(CONFIG_BB_POWER_SAVING))
		odm_dynamic_bb_power_saving_init(p_dm_odm);
#endif

		if (p_dm_odm->support_ic_type == ODM_RTL8188E) {
			odm_primary_cca_init(p_dm_odm);
			odm_ra_info_init_all(p_dm_odm);
		}
	}
}

void
odm_dm_reset(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
	struct _dynamic_initial_gain_threshold_ *p_dm_dig_table = &p_dm_odm->dm_dig_table;

	odm_ant_div_reset(p_dm_odm);
	phydm_set_edcca_threshold_api(p_dm_odm, p_dm_dig_table->cur_ig_value);
}


void
phydm_support_ability_debug(
	void		*p_dm_void,
	u32		*const dm_value,
	u32			*_used,
	char			*output,
	u32			*_out_len
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32			pre_support_ability;
	u32 used = *_used;
	u32 out_len = *_out_len;

	pre_support_ability = p_dm_odm->support_ability ;
	PHYDM_SNPRINTF((output + used, out_len - used, "\n%s\n", "================================"));
	if (dm_value[0] == 100) {
		PHYDM_SNPRINTF((output + used, out_len - used, "[Supportability] PhyDM Selection\n"));
		PHYDM_SNPRINTF((output + used, out_len - used, "%s\n", "================================"));
		PHYDM_SNPRINTF((output + used, out_len - used, "00. (( %s ))DIG\n", ((p_dm_odm->support_ability & ODM_BB_DIG) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "01. (( %s ))RA_MASK\n", ((p_dm_odm->support_ability & ODM_BB_RA_MASK) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "02. (( %s ))DYNAMIC_TXPWR\n", ((p_dm_odm->support_ability & ODM_BB_DYNAMIC_TXPWR) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "03. (( %s ))FA_CNT\n", ((p_dm_odm->support_ability & ODM_BB_FA_CNT) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "04. (( %s ))RSSI_MONITOR\n", ((p_dm_odm->support_ability & ODM_BB_RSSI_MONITOR) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "05. (( %s ))CCK_PD\n", ((p_dm_odm->support_ability & ODM_BB_CCK_PD) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "06. (( %s ))ANT_DIV\n", ((p_dm_odm->support_ability & ODM_BB_ANT_DIV) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "08. (( %s ))PWR_TRAIN\n", ((p_dm_odm->support_ability & ODM_BB_PWR_TRAIN) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "09. (( %s ))RATE_ADAPTIVE\n", ((p_dm_odm->support_ability & ODM_BB_RATE_ADAPTIVE) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "10. (( %s ))PATH_DIV\n", ((p_dm_odm->support_ability & ODM_BB_PATH_DIV) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "13. (( %s ))ADAPTIVITY\n", ((p_dm_odm->support_ability & ODM_BB_ADAPTIVITY) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "14. (( %s ))struct _CFO_TRACKING_\n", ((p_dm_odm->support_ability & ODM_BB_CFO_TRACKING) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "15. (( %s ))NHM_CNT\n", ((p_dm_odm->support_ability & ODM_BB_NHM_CNT) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "16. (( %s ))PRIMARY_CCA\n", ((p_dm_odm->support_ability & ODM_BB_PRIMARY_CCA) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "17. (( %s ))TXBF\n", ((p_dm_odm->support_ability & ODM_BB_TXBF) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "18. (( %s ))DYNAMIC_ARFR\n", ((p_dm_odm->support_ability & ODM_BB_DYNAMIC_ARFR) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "20. (( %s ))EDCA_TURBO\n", ((p_dm_odm->support_ability & ODM_MAC_EDCA_TURBO) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "21. (( %s ))DYNAMIC_RX_PATH\n", ((p_dm_odm->support_ability & ODM_BB_DYNAMIC_RX_PATH) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "24. (( %s ))TX_PWR_TRACK\n", ((p_dm_odm->support_ability & ODM_RF_TX_PWR_TRACK) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "25. (( %s ))RX_GAIN_TRACK\n", ((p_dm_odm->support_ability & ODM_RF_RX_GAIN_TRACK) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "26. (( %s ))RF_CALIBRATION\n", ((p_dm_odm->support_ability & ODM_RF_CALIBRATION) ? ("V") : ("."))));
		PHYDM_SNPRINTF((output + used, out_len - used, "%s\n", "================================"));
	}
	/*
	else if(dm_value[0] == 101)
	{
		p_dm_odm->support_ability = 0 ;
		dbg_print("Disable all support_ability components\n");
		PHYDM_SNPRINTF((output+used, out_len-used,"%s\n", "Disable all support_ability components"));
	}
	*/
	else {

		if (dm_value[1] == 1) { /* enable */
			p_dm_odm->support_ability |= BIT(dm_value[0]) ;
			if (BIT(dm_value[0]) & ODM_BB_PATH_DIV)
				odm_path_diversity_init(p_dm_odm);
		} else if (dm_value[1] == 2) /* disable */
			p_dm_odm->support_ability &= ~(BIT(dm_value[0])) ;
		else {
			/* dbg_print("\n[Warning!!!]  1:enable,  2:disable \n\n"); */
			PHYDM_SNPRINTF((output + used, out_len - used, "%s\n", "[Warning!!!]  1:enable,  2:disable"));
		}
	}
	PHYDM_SNPRINTF((output + used, out_len - used, "pre-support_ability  =  0x%x\n",  pre_support_ability));
	PHYDM_SNPRINTF((output + used, out_len - used, "Curr-support_ability =  0x%x\n", p_dm_odm->support_ability));
	PHYDM_SNPRINTF((output + used, out_len - used, "%s\n", "================================"));
}

void
phydm_watchdog_mp(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
#if (CONFIG_DYNAMIC_RX_PATH == 1)
	phydm_dynamic_rx_path_caller(p_dm_odm);
#endif
}
/*
 * 2011/09/20 MH This is the entry pointer for all team to execute HW out source DM.
 * You can not add any dummy function here, be care, you can only use DM structure
 * to perform any new ODM_DM.
 *   */
void
odm_dm_watchdog(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
	odm_common_info_self_update(p_dm_odm);
	phydm_basic_dbg_message(p_dm_odm);
	phydm_receiver_blocking(p_dm_odm);
	odm_hw_setting(p_dm_odm);

	odm_false_alarm_counter_statistics(p_dm_odm);
	phydm_noisy_detection(p_dm_odm);

	odm_rssi_monitor_check(p_dm_odm);

	if (*(p_dm_odm->p_is_power_saving) == true) {
		odm_dig_by_rssi_lps(p_dm_odm);
		phydm_adaptivity(p_dm_odm);
		odm_antenna_diversity(p_dm_odm); /*enable AntDiv in PS mode, request from SD4 Jeff*/
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_COMMON, ODM_DBG_LOUD, ("DMWatchdog in power saving mode\n"));
		return;
	}

	phydm_check_adaptivity(p_dm_odm);
	odm_update_power_training_state(p_dm_odm);
	odm_DIG(p_dm_odm);
	phydm_adaptivity(p_dm_odm);
	odm_cck_packet_detection_thresh(p_dm_odm);

	phydm_ra_info_watchdog(p_dm_odm);
#if PHYDM_SUPPORT_EDCA
	odm_edca_turbo_check(p_dm_odm);
#endif
	odm_path_diversity(p_dm_odm);
	odm_cfo_tracking(p_dm_odm);
	odm_dynamic_tx_power(p_dm_odm);
	odm_antenna_diversity(p_dm_odm);
#if (CONFIG_DYNAMIC_RX_PATH == 1)
	phydm_dynamic_rx_path(p_dm_odm);
#endif

	phydm_beamforming_watchdog(p_dm_odm);

	phydm_rf_watchdog(p_dm_odm);

	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES) {

		if (p_dm_odm->support_ic_type == ODM_RTL8188E)
			odm_dynamic_primary_cca(p_dm_odm);
	}
	odm_dtc(p_dm_odm);

	odm_common_info_self_reset(p_dm_odm);
}

/*
 * Init /.. Fixed HW value. Only init time.
 *   */
void
odm_cmn_info_init(
	struct PHY_DM_STRUCT		*p_dm_odm,
	enum odm_cmninfo_e	cmn_info,
	u32			value
)
{
	/*  */
	/* This section is used for init value */
	/*  */
	switch	(cmn_info) {
	/*  */
	/* Fixed ODM value. */
	/*  */
	case	ODM_CMNINFO_ABILITY:
		p_dm_odm->support_ability = (u32)value;
		break;

	case	ODM_CMNINFO_RF_TYPE:
		p_dm_odm->rf_type = (u8)value;
		break;

	case	ODM_CMNINFO_PLATFORM:
		p_dm_odm->support_platform = (u8)value;
		break;

	case	ODM_CMNINFO_INTERFACE:
		p_dm_odm->support_interface = (u8)value;
		break;

	case	ODM_CMNINFO_MP_TEST_CHIP:
		p_dm_odm->is_mp_chip = (u8)value;
		break;

	case	ODM_CMNINFO_IC_TYPE:
		p_dm_odm->support_ic_type = value;
		break;

	case	ODM_CMNINFO_CUT_VER:
		p_dm_odm->cut_version = (u8)value;
		break;

	case	ODM_CMNINFO_FAB_VER:
		p_dm_odm->fab_version = (u8)value;
		break;

	case	ODM_CMNINFO_RFE_TYPE:
		p_dm_odm->rfe_type = (u8)value;
		phydm_init_hw_info_by_rfe(p_dm_odm);
		break;

	case    ODM_CMNINFO_RF_ANTENNA_TYPE:
		p_dm_odm->ant_div_type = (u8)value;
		break;

	case	ODM_CMNINFO_WITH_EXT_ANTENNA_SWITCH:
		p_dm_odm->with_extenal_ant_switch = (u8)value;
		break;

	case    ODM_CMNINFO_BE_FIX_TX_ANT:
		p_dm_odm->dm_fat_table.b_fix_tx_ant = (u8)value;
		break;

	case	ODM_CMNINFO_BOARD_TYPE:
		if (!p_dm_odm->is_init_hw_info_by_rfe)
			p_dm_odm->board_type = (u8)value;
		break;

	case	ODM_CMNINFO_PACKAGE_TYPE:
		if (!p_dm_odm->is_init_hw_info_by_rfe)
			p_dm_odm->package_type = (u8)value;
		break;

	case	ODM_CMNINFO_EXT_LNA:
		if (!p_dm_odm->is_init_hw_info_by_rfe)
			p_dm_odm->ext_lna = (u8)value;
		break;

	case	ODM_CMNINFO_5G_EXT_LNA:
		if (!p_dm_odm->is_init_hw_info_by_rfe)
			p_dm_odm->ext_lna_5g = (u8)value;
		break;

	case	ODM_CMNINFO_EXT_PA:
		if (!p_dm_odm->is_init_hw_info_by_rfe)
			p_dm_odm->ext_pa = (u8)value;
		break;

	case	ODM_CMNINFO_5G_EXT_PA:
		if (!p_dm_odm->is_init_hw_info_by_rfe)
			p_dm_odm->ext_pa_5g = (u8)value;
		break;

	case	ODM_CMNINFO_GPA:
		if (!p_dm_odm->is_init_hw_info_by_rfe)
			p_dm_odm->type_gpa = (u16)value;
		break;

	case	ODM_CMNINFO_APA:
		if (!p_dm_odm->is_init_hw_info_by_rfe)
			p_dm_odm->type_apa = (u16)value;
		break;

	case	ODM_CMNINFO_GLNA:
		if (!p_dm_odm->is_init_hw_info_by_rfe)
			p_dm_odm->type_glna = (u16)value;
		break;

	case	ODM_CMNINFO_ALNA:
		if (!p_dm_odm->is_init_hw_info_by_rfe)
			p_dm_odm->type_alna = (u16)value;
		break;

	case	ODM_CMNINFO_EXT_TRSW:
		if (!p_dm_odm->is_init_hw_info_by_rfe)
			p_dm_odm->ext_trsw = (u8)value;
		break;
	case	ODM_CMNINFO_EXT_LNA_GAIN:
		p_dm_odm->ext_lna_gain = (u8)value;
		break;
	case	ODM_CMNINFO_PATCH_ID:
		p_dm_odm->patch_id = (u8)value;
		break;
	case	ODM_CMNINFO_BINHCT_TEST:
		p_dm_odm->is_in_hct_test = (bool)value;
		break;
	case	ODM_CMNINFO_BWIFI_TEST:
		p_dm_odm->wifi_test = (u8)value;
		break;
	case	ODM_CMNINFO_SMART_CONCURRENT:
		p_dm_odm->is_dual_mac_smart_concurrent = (bool)value;
		break;
	case	ODM_CMNINFO_DOMAIN_CODE_2G:
		p_dm_odm->odm_regulation_2_4g = (u8)value;
		break;
	case	ODM_CMNINFO_DOMAIN_CODE_5G:
		p_dm_odm->odm_regulation_5g = (u8)value;
		break;
	case	ODM_CMNINFO_CONFIG_BB_RF:
		p_dm_odm->config_bbrf = (bool)value;
		break;
	case	ODM_CMNINFO_IQKFWOFFLOAD:
		p_dm_odm->iqk_fw_offload = (u8)value;
		break;
	case	ODM_CMNINFO_IQKPAOFF:
		p_dm_odm->rf_calibrate_info.is_iqk_pa_off = (bool)value;
		break;
	case	ODM_CMNINFO_REGRFKFREEENABLE:
		p_dm_odm->rf_calibrate_info.reg_rf_kfree_enable = (u8)value;
		break;
	case	ODM_CMNINFO_RFKFREEENABLE:
		p_dm_odm->rf_calibrate_info.rf_kfree_enable = (u8)value;
		break;
	case	ODM_CMNINFO_NORMAL_RX_PATH_CHANGE:
		p_dm_odm->normal_rx_path = (u8)value;
		break;
	case	ODM_CMNINFO_EFUSE0X3D8:
		p_dm_odm->efuse0x3d8 = (u8)value;
		break;
	case	ODM_CMNINFO_EFUSE0X3D7:
		p_dm_odm->efuse0x3d7 = (u8)value;
		break;
#ifdef CONFIG_PHYDM_DFS_MASTER
	case	ODM_CMNINFO_DFS_REGION_DOMAIN:
		p_dm_odm->dfs_region_domain = (u8)value;
		break;
#endif
	/* To remove the compiler warning, must add an empty default statement to handle the other values. */
	default:
		/* do nothing */
		break;

	}

}


void
odm_cmn_info_hook(
	struct PHY_DM_STRUCT		*p_dm_odm,
	enum odm_cmninfo_e	cmn_info,
	void			*p_value
)
{
	/*  */
	/* Hook call by reference pointer. */
	/*  */
	switch	(cmn_info) {
	/*  */
	/* Dynamic call by reference pointer. */
	/*  */
	case	ODM_CMNINFO_MAC_PHY_MODE:
		p_dm_odm->p_mac_phy_mode = (u8 *)p_value;
		break;

	case	ODM_CMNINFO_TX_UNI:
		p_dm_odm->p_num_tx_bytes_unicast = (u64 *)p_value;
		break;

	case	ODM_CMNINFO_RX_UNI:
		p_dm_odm->p_num_rx_bytes_unicast = (u64 *)p_value;
		break;

	case	ODM_CMNINFO_WM_MODE:
		p_dm_odm->p_wireless_mode = (u8 *)p_value;
		break;

	case	ODM_CMNINFO_BAND:
		p_dm_odm->p_band_type = (u8 *)p_value;
		break;

	case	ODM_CMNINFO_SEC_CHNL_OFFSET:
		p_dm_odm->p_sec_ch_offset = (u8 *)p_value;
		break;

	case	ODM_CMNINFO_SEC_MODE:
		p_dm_odm->p_security = (u8 *)p_value;
		break;

	case	ODM_CMNINFO_BW:
		p_dm_odm->p_band_width = (u8 *)p_value;
		break;

	case	ODM_CMNINFO_CHNL:
		p_dm_odm->p_channel = (u8 *)p_value;
		break;

	case	ODM_CMNINFO_DMSP_GET_VALUE:
		p_dm_odm->p_is_get_value_from_other_mac = (bool *)p_value;
		break;

	case	ODM_CMNINFO_BUDDY_ADAPTOR:
		p_dm_odm->p_buddy_adapter = (struct _ADAPTER **)p_value;
		break;

	case	ODM_CMNINFO_DMSP_IS_MASTER:
		p_dm_odm->p_is_master_of_dmsp = (bool *)p_value;
		break;

	case	ODM_CMNINFO_SCAN:
		p_dm_odm->p_is_scan_in_process = (bool *)p_value;
		break;

	case	ODM_CMNINFO_POWER_SAVING:
		p_dm_odm->p_is_power_saving = (bool *)p_value;
		break;

	case	ODM_CMNINFO_ONE_PATH_CCA:
		p_dm_odm->p_one_path_cca = (u8 *)p_value;
		break;

	case	ODM_CMNINFO_DRV_STOP:
		p_dm_odm->p_is_driver_stopped = (bool *)p_value;
		break;

	case	ODM_CMNINFO_PNP_IN:
		p_dm_odm->p_is_driver_is_going_to_pnp_set_power_sleep = (bool *)p_value;
		break;

	case	ODM_CMNINFO_INIT_ON:
		p_dm_odm->pinit_adpt_in_progress = (bool *)p_value;
		break;

	case	ODM_CMNINFO_ANT_TEST:
		p_dm_odm->p_antenna_test = (u8 *)p_value;
		break;

	case	ODM_CMNINFO_NET_CLOSED:
		p_dm_odm->p_is_net_closed = (bool *)p_value;
		break;

	case	ODM_CMNINFO_FORCED_RATE:
		p_dm_odm->p_forced_data_rate = (u16 *)p_value;
		break;
	case	ODM_CMNINFO_ANT_DIV:
		p_dm_odm->p_enable_antdiv = (u8 *)p_value;
		break;
	case	ODM_CMNINFO_ADAPTIVITY:
		p_dm_odm->p_enable_adaptivity = (u8 *)p_value;
		break;
	case  ODM_CMNINFO_FORCED_IGI_LB:
		p_dm_odm->pu1_forced_igi_lb = (u8 *)p_value;
		break;

	case	ODM_CMNINFO_P2P_LINK:
		p_dm_odm->dm_dig_table.is_p2p_in_process = (u8 *)p_value;
		break;

	case	ODM_CMNINFO_IS1ANTENNA:
		p_dm_odm->p_is_1_antenna = (bool *)p_value;
		break;

	case	ODM_CMNINFO_RFDEFAULTPATH:
		p_dm_odm->p_rf_default_path = (u8 *)p_value;
		break;

	case	ODM_CMNINFO_FCS_MODE:
		p_dm_odm->p_is_fcs_mode_enable = (bool *)p_value;
		break;
	/*add by YuChen for beamforming PhyDM*/
	case	ODM_CMNINFO_HUBUSBMODE:
		p_dm_odm->hub_usb_mode = (u8 *)p_value;
		break;
	case	ODM_CMNINFO_FWDWRSVDPAGEINPROGRESS:
		p_dm_odm->p_is_fw_dw_rsvd_page_in_progress = (bool *)p_value;
		break;
	case	ODM_CMNINFO_TX_TP:
		p_dm_odm->p_current_tx_tp = (u32 *)p_value;
		break;
	case	ODM_CMNINFO_RX_TP:
		p_dm_odm->p_current_rx_tp = (u32 *)p_value;
		break;
	case	ODM_CMNINFO_SOUNDING_SEQ:
		p_dm_odm->p_sounding_seq = (u8 *)p_value;
		break;
#ifdef CONFIG_PHYDM_DFS_MASTER
	case	ODM_CMNINFO_DFS_MASTER_ENABLE:
		p_dm_odm->dfs_master_enabled = (u8 *)p_value;
		break;
#endif
	case	ODM_CMNINFO_FORCE_TX_ANT_BY_TXDESC:
		p_dm_odm->dm_fat_table.p_force_tx_ant_by_desc = (u8 *)p_value;
		break;
	case	ODM_CMNINFO_SET_S0S1_DEFAULT_ANTENNA:
		p_dm_odm->dm_fat_table.p_default_s0_s1 = (u8 *)p_value;
		break;

	default:
		/*do nothing*/
		break;

	}

}


void
odm_cmn_info_ptr_array_hook(
	struct PHY_DM_STRUCT		*p_dm_odm,
	enum odm_cmninfo_e	cmn_info,
	u16			index,
	void			*p_value
)
{
	/*Hook call by reference pointer.*/
	switch	(cmn_info) {
	/*Dynamic call by reference pointer.	*/
	case	ODM_CMNINFO_STA_STATUS:
		p_dm_odm->p_odm_sta_info[index] = (struct sta_info *)p_value;

		if (IS_STA_VALID(p_dm_odm->p_odm_sta_info[index]))
			p_dm_odm->platform2phydm_macid_table[((struct sta_info *)p_value)->mac_id] = index;

		break;
	/* To remove the compiler warning, must add an empty default statement to handle the other values. */
	default:
		/* do nothing */
		break;
	}

}


/*
 * Update band/CHannel/.. The values are dynamic but non-per-packet.
 *   */
void
odm_cmn_info_update(
	struct PHY_DM_STRUCT		*p_dm_odm,
	u32			cmn_info,
	u64			value
)
{
	/*  */
	/* This init variable may be changed in run time. */
	/*  */
	switch	(cmn_info) {
	case ODM_CMNINFO_LINK_IN_PROGRESS:
		p_dm_odm->is_link_in_process = (bool)value;
		break;

	case	ODM_CMNINFO_ABILITY:
		p_dm_odm->support_ability = (u32)value;
		break;

	case	ODM_CMNINFO_RF_TYPE:
		p_dm_odm->rf_type = (u8)value;
		break;

	case	ODM_CMNINFO_WIFI_DIRECT:
		p_dm_odm->is_wifi_direct = (bool)value;
		break;

	case	ODM_CMNINFO_WIFI_DISPLAY:
		p_dm_odm->is_wifi_display = (bool)value;
		break;

	case	ODM_CMNINFO_LINK:
		p_dm_odm->is_linked = (bool)value;
		break;

	case	ODM_CMNINFO_STATION_STATE:
		p_dm_odm->bsta_state = (bool)value;
		break;

	case	ODM_CMNINFO_RSSI_MIN:
		p_dm_odm->rssi_min = (u8)value;
		break;

	case	ODM_CMNINFO_DBG_COMP:
		p_dm_odm->debug_components = (u32)value;
		break;

	case	ODM_CMNINFO_DBG_LEVEL:
		p_dm_odm->debug_level = (u32)value;
		break;
	case	ODM_CMNINFO_RA_THRESHOLD_HIGH:
		p_dm_odm->rate_adaptive.high_rssi_thresh = (u8)value;
		break;

	case	ODM_CMNINFO_RA_THRESHOLD_LOW:
		p_dm_odm->rate_adaptive.low_rssi_thresh = (u8)value;
		break;
#if defined(BT_SUPPORT) && (BT_SUPPORT == 1)
	/* The following is for BT HS mode and BT coexist mechanism. */
	case ODM_CMNINFO_BT_ENABLED:
		p_dm_odm->is_bt_enabled = (bool)value;
		break;

	case ODM_CMNINFO_BT_HS_CONNECT_PROCESS:
		p_dm_odm->is_bt_connect_process = (bool)value;
		break;

	case ODM_CMNINFO_BT_HS_RSSI:
		p_dm_odm->bt_hs_rssi = (u8)value;
		break;

	case	ODM_CMNINFO_BT_OPERATION:
		p_dm_odm->is_bt_hs_operation = (bool)value;
		break;

	case	ODM_CMNINFO_BT_LIMITED_DIG:
		p_dm_odm->is_bt_limited_dig = (bool)value;
		break;

	case ODM_CMNINFO_BT_DIG:
		p_dm_odm->bt_hs_dig_val = (u8)value;
		break;

	case	ODM_CMNINFO_BT_BUSY:
		p_dm_odm->is_bt_busy = (bool)value;
		break;

	case	ODM_CMNINFO_BT_DISABLE_EDCA:
		p_dm_odm->is_bt_disable_edca_turbo = (bool)value;
		break;
#endif
	case	ODM_CMNINFO_AP_TOTAL_NUM:
		p_dm_odm->ap_total_num = (u8)value;
		break;

	case	ODM_CMNINFO_POWER_TRAINING:
		p_dm_odm->is_disable_power_training = (bool)value;
		break;

#ifdef CONFIG_PHYDM_DFS_MASTER
	case	ODM_CMNINFO_DFS_REGION_DOMAIN:
		p_dm_odm->dfs_region_domain = (u8)value;
		break;
#endif
	default:
		/* do nothing */
		break;
	}
}

u32
phydm_cmn_info_query(
	struct PHY_DM_STRUCT					*p_dm_odm,
	enum phydm_info_query_e			info_type
)
{
	struct false_ALARM_STATISTICS	*false_alm_cnt = (struct false_ALARM_STATISTICS *)phydm_get_structure(p_dm_odm, PHYDMfalseALMCNT);

	switch (info_type) {
	case PHYDM_INFO_FA_OFDM:
		return false_alm_cnt->cnt_ofdm_fail;

	case PHYDM_INFO_FA_CCK:
		return false_alm_cnt->cnt_cck_fail;

	case PHYDM_INFO_FA_TOTAL:
		return false_alm_cnt->cnt_all;

	case PHYDM_INFO_CCA_OFDM:
		return false_alm_cnt->cnt_ofdm_cca;

	case PHYDM_INFO_CCA_CCK:
		return false_alm_cnt->cnt_cck_cca;

	case PHYDM_INFO_CCA_ALL:
		return false_alm_cnt->cnt_cca_all;

	case PHYDM_INFO_CRC32_OK_VHT:
		return false_alm_cnt->cnt_vht_crc32_ok;

	case PHYDM_INFO_CRC32_OK_HT:
		return false_alm_cnt->cnt_ht_crc32_ok;

	case PHYDM_INFO_CRC32_OK_LEGACY:
		return false_alm_cnt->cnt_ofdm_crc32_ok;

	case PHYDM_INFO_CRC32_OK_CCK:
		return false_alm_cnt->cnt_cck_crc32_ok;

	case PHYDM_INFO_CRC32_ERROR_VHT:
		return false_alm_cnt->cnt_vht_crc32_error;

	case PHYDM_INFO_CRC32_ERROR_HT:
		return false_alm_cnt->cnt_ht_crc32_error;

	case PHYDM_INFO_CRC32_ERROR_LEGACY:
		return false_alm_cnt->cnt_ofdm_crc32_error;

	case PHYDM_INFO_CRC32_ERROR_CCK:
		return false_alm_cnt->cnt_cck_crc32_error;

	case PHYDM_INFO_EDCCA_FLAG:
		return false_alm_cnt->edcca_flag;

	case PHYDM_INFO_OFDM_ENABLE:
		return false_alm_cnt->ofdm_block_enable;

	case PHYDM_INFO_CCK_ENABLE:
		return false_alm_cnt->cck_block_enable;

	case PHYDM_INFO_DBG_PORT_0:
		return false_alm_cnt->dbg_port0;

	default:
		return 0xffffffff;

	}
}

void
odm_init_all_timers(
	struct PHY_DM_STRUCT	*p_dm_odm
)
{
#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
	odm_ant_div_timers(p_dm_odm, INIT_ANTDIV_TIMMER);
#endif

#if (CONFIG_DYNAMIC_RX_PATH == 1)
	phydm_dynamic_rx_path_timers(p_dm_odm, INIT_DRP_TIMMER);
#endif

#if (BEAMFORMING_SUPPORT == 1)
	odm_initialize_timer(p_dm_odm, &p_dm_odm->beamforming_info.beamforming_timer,
		(void *)beamforming_sw_timer_callback, NULL, "beamforming_timer");
#endif
}

void
odm_cancel_all_timers(
	struct PHY_DM_STRUCT	*p_dm_odm
)
{
#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
	odm_ant_div_timers(p_dm_odm, CANCEL_ANTDIV_TIMMER);
#endif

#if (CONFIG_DYNAMIC_RX_PATH == 1)
	phydm_dynamic_rx_path_timers(p_dm_odm, CANCEL_DRP_TIMMER);
#endif

#if (BEAMFORMING_SUPPORT == 1)
	odm_cancel_timer(p_dm_odm, &p_dm_odm->beamforming_info.beamforming_timer);
#endif
}


void
odm_release_all_timers(
	struct PHY_DM_STRUCT	*p_dm_odm
)
{
#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
	odm_ant_div_timers(p_dm_odm, RELEASE_ANTDIV_TIMMER);
#endif

#if (CONFIG_DYNAMIC_RX_PATH == 1)
	phydm_dynamic_rx_path_timers(p_dm_odm, RELEASE_DRP_TIMMER);
#endif

#if (BEAMFORMING_SUPPORT == 1)
	odm_release_timer(p_dm_odm, &p_dm_odm->beamforming_info.beamforming_timer);
#endif
}


/* 3============================================================
 * 3 Tx Power Tracking
 * 3============================================================ */

/* need to ODM CE Platform
 * move to here for ANT detection mechanism using */

u32
get_psd_data(
	struct PHY_DM_STRUCT	*p_dm_odm,
	unsigned int	point,
	u8 initial_gain_psd)
{
	/* unsigned int	val, rfval; */
	/* int	psd_report; */
	u32	psd_report;

	/* HAL_DATA_TYPE		*p_hal_data = GET_HAL_DATA(adapter); */
	/* Debug Message */
	/* val = phy_query_bb_reg(adapter,0x908, MASKDWORD); */
	/* dbg_print("reg908 = 0x%x\n",val); */
	/* val = phy_query_bb_reg(adapter,0xDF4, MASKDWORD); */
	/* rfval = phy_query_rf_reg(adapter, ODM_RF_PATH_A, 0x00, RFREGOFFSETMASK); */
	/* dbg_print("RegDF4 = 0x%x, RFReg00 = 0x%x\n",val, rfval); */
	/* dbg_print("PHYTXON = %x, OFDMCCA_PP = %x, CCKCCA_PP = %x, RFReg00 = %x\n", */
	/* (val&BIT25)>>25, (val&BIT14)>>14, (val&BIT15)>>15, rfval); */

	/* Set DCO frequency index, offset=(40MHz/SamplePts)*point */
	odm_set_bb_reg(p_dm_odm, 0x808, 0x3FF, point);

	/* Start PSD calculation, Reg808[22]=0->1 */
	odm_set_bb_reg(p_dm_odm, 0x808, BIT(22), 1);
	/* Need to wait for HW PSD report */
	odm_stall_execution(1000);
	odm_set_bb_reg(p_dm_odm, 0x808, BIT(22), 0);
	/* Read PSD report, Reg8B4[15:0] */
	psd_report = odm_get_bb_reg(p_dm_odm, 0x8B4, MASKDWORD) & 0x0000FFFF;

	psd_report = (u32)(odm_convert_to_db(psd_report)) + (u32)(initial_gain_psd - 0x1c);

	return psd_report;
}

u32
odm_convert_to_db(
	u32	value)
{
	u8 i;
	u8 j;
	u32 dB;

	value = value & 0xFFFF;

	for (i = 0; i < 12; i++) {
		if (value <= db_invert_table[i][7])
			break;
	}

	if (i >= 12) {
		return 96;	/* maximum 96 dB */
	}

	for (j = 0; j < 8; j++) {
		if (value <= db_invert_table[i][j])
			break;
	}

	dB = (i << 3) + j + 1;

	return dB;
}

u32
odm_convert_to_linear(
	u32	value)
{
	u8 i;
	u8 j;
	u32 linear;

	/* 1dB~96dB */

	value = value & 0xFF;

	i = (u8)((value - 1) >> 3);
	j = (u8)(value - 1) - (i << 3);

	linear = db_invert_table[i][j];

	return linear;
}

/*
 * ODM multi-port consideration, added by Roger, 2013.10.01.
 *   */
void
odm_asoc_entry_init(
	struct PHY_DM_STRUCT	*p_dm_odm
)
{
}

/* Justin: According to the current RRSI to adjust Response Frame TX power, 2012/11/05 */
void odm_dtc(struct PHY_DM_STRUCT *p_dm_odm)
{
#ifdef CONFIG_DM_RESP_TXAGC
#define DTC_BASE            35	/* RSSI higher than this value, start to decade TX power */
#define DTC_DWN_BASE       (DTC_BASE-5)	/* RSSI lower than this value, start to increase TX power */

	/* RSSI vs TX power step mapping: decade TX power */
	static const u8 dtc_table_down[] = {
		DTC_BASE,
		(DTC_BASE + 5),
		(DTC_BASE + 10),
		(DTC_BASE + 15),
		(DTC_BASE + 20),
		(DTC_BASE + 25)
	};

	/* RSSI vs TX power step mapping: increase TX power */
	static const u8 dtc_table_up[] = {
		DTC_DWN_BASE,
		(DTC_DWN_BASE - 5),
		(DTC_DWN_BASE - 10),
		(DTC_DWN_BASE - 15),
		(DTC_DWN_BASE - 15),
		(DTC_DWN_BASE - 20),
		(DTC_DWN_BASE - 20),
		(DTC_DWN_BASE - 25),
		(DTC_DWN_BASE - 25),
		(DTC_DWN_BASE - 30),
		(DTC_DWN_BASE - 35)
	};

	u8 i;
	u8 dtc_steps = 0;
	u8 sign;
	u8 resp_txagc = 0;

	if (DTC_BASE < p_dm_odm->rssi_min) {
		/* need to decade the CTS TX power */
		sign = 1;
		for (i = 0; i < ARRAY_SIZE(dtc_table_down); i++) {
			if ((dtc_table_down[i] >= p_dm_odm->rssi_min) || (dtc_steps >= 6))
				break;
			else
				dtc_steps++;
		}
	} else {
		sign = 0;
		dtc_steps = 0;
	}

	resp_txagc = dtc_steps | (sign << 4);
	resp_txagc = resp_txagc | (resp_txagc << 5);
	odm_write_1byte(p_dm_odm, 0x06d9, resp_txagc);

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_PWR_TRAIN, ODM_DBG_LOUD, ("%s rssi_min:%u, set RESP_TXAGC to %s %u\n",
		__func__, p_dm_odm->rssi_min, sign ? "minus" : "plus", dtc_steps));
#endif /* CONFIG_RESP_TXAGC_ADJUST */
}

void
odm_update_power_training_state(
	struct PHY_DM_STRUCT	*p_dm_odm
)
{
	struct false_ALARM_STATISTICS	*false_alm_cnt = (struct false_ALARM_STATISTICS *)phydm_get_structure(p_dm_odm, PHYDMfalseALMCNT);
	struct _dynamic_initial_gain_threshold_						*p_dm_dig_table = &p_dm_odm->dm_dig_table;
	u32						score = 0;

	if (!(p_dm_odm->support_ability & ODM_BB_PWR_TRAIN))
		return;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("odm_update_power_training_state()============>\n"));
	p_dm_odm->is_change_state = false;

	/* Debug command */
	if (p_dm_odm->force_power_training_state) {
		if (p_dm_odm->force_power_training_state == 1 && !p_dm_odm->is_disable_power_training) {
			p_dm_odm->is_change_state = true;
			p_dm_odm->is_disable_power_training = true;
		} else if (p_dm_odm->force_power_training_state == 2 && p_dm_odm->is_disable_power_training) {
			p_dm_odm->is_change_state = true;
			p_dm_odm->is_disable_power_training = false;
		}

		p_dm_odm->PT_score = 0;
		p_dm_odm->phy_dbg_info.num_qry_phy_status_ofdm = 0;
		p_dm_odm->phy_dbg_info.num_qry_phy_status_cck = 0;
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("odm_update_power_training_state(): force_power_training_state = %d\n",
				p_dm_odm->force_power_training_state));
		return;
	}

	if (!p_dm_odm->is_linked)
		return;

	/* First connect */
	if ((p_dm_odm->is_linked) && (p_dm_dig_table->is_media_connect_0 == false)) {
		p_dm_odm->PT_score = 0;
		p_dm_odm->is_change_state = true;
		p_dm_odm->phy_dbg_info.num_qry_phy_status_ofdm = 0;
		p_dm_odm->phy_dbg_info.num_qry_phy_status_cck = 0;
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("odm_update_power_training_state(): First Connect\n"));
		return;
	}

	/* Compute score */
	if (p_dm_odm->nhm_cnt_0 >= 215)
		score = 2;
	else if (p_dm_odm->nhm_cnt_0 >= 190)
		score = 1;							/* unknow state */
	else {
		u32	rx_pkt_cnt;

		rx_pkt_cnt = (u32)(p_dm_odm->phy_dbg_info.num_qry_phy_status_ofdm) + (u32)(p_dm_odm->phy_dbg_info.num_qry_phy_status_cck);

		if ((false_alm_cnt->cnt_cca_all > 31 && rx_pkt_cnt > 31) && (false_alm_cnt->cnt_cca_all >= rx_pkt_cnt)) {
			if ((rx_pkt_cnt + (rx_pkt_cnt >> 1)) <= false_alm_cnt->cnt_cca_all)
				score = 0;
			else if ((rx_pkt_cnt + (rx_pkt_cnt >> 2)) <= false_alm_cnt->cnt_cca_all)
				score = 1;
			else
				score = 2;
		}
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("odm_update_power_training_state(): rx_pkt_cnt = %d, cnt_cca_all = %d\n",
				rx_pkt_cnt, false_alm_cnt->cnt_cca_all));
	}
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("odm_update_power_training_state(): num_qry_phy_status_ofdm = %d, num_qry_phy_status_cck = %d\n",
		(u32)(p_dm_odm->phy_dbg_info.num_qry_phy_status_ofdm), (u32)(p_dm_odm->phy_dbg_info.num_qry_phy_status_cck)));
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("odm_update_power_training_state(): nhm_cnt_0 = %d, score = %d\n",
			p_dm_odm->nhm_cnt_0, score));

	/* smoothing */
	p_dm_odm->PT_score = (score << 4) + (p_dm_odm->PT_score >> 1) + (p_dm_odm->PT_score >> 2);
	score = (p_dm_odm->PT_score + 32) >> 6;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("odm_update_power_training_state(): PT_score = %d, score after smoothing = %d\n",
			p_dm_odm->PT_score, score));

	/* mode decision */
	if (score == 2) {
		if (p_dm_odm->is_disable_power_training) {
			p_dm_odm->is_change_state = true;
			p_dm_odm->is_disable_power_training = false;
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("odm_update_power_training_state(): Change state\n"));
		}
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("odm_update_power_training_state(): Enable Power Training\n"));
	} else if (score == 0) {
		if (!p_dm_odm->is_disable_power_training) {
			p_dm_odm->is_change_state = true;
			p_dm_odm->is_disable_power_training = true;
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("odm_update_power_training_state(): Change state\n"));
		}
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("odm_update_power_training_state(): Disable Power Training\n"));
	}

	p_dm_odm->phy_dbg_info.num_qry_phy_status_ofdm = 0;
	p_dm_odm->phy_dbg_info.num_qry_phy_status_cck = 0;
}



/*===========================================================*/
/* The following is for compile only*/
/*===========================================================*/
/*#define TARGET_CHNL_NUM_2G_5G	59*/
/*===========================================================*/

void
phydm_noisy_detection(
	struct PHY_DM_STRUCT	*p_dm_odm
)
{
	u32  total_fa_cnt, total_cca_cnt;
	u32  score = 0, i, score_smooth;

	total_cca_cnt = p_dm_odm->false_alm_cnt.cnt_cca_all;
	total_fa_cnt  = p_dm_odm->false_alm_cnt.cnt_all;

	for (i = 0; i <= 16; i++) {
		if (total_fa_cnt * 16 >= total_cca_cnt * (16 - i)) {
			score = 16 - i;
			break;
		}
	}

	/* noisy_decision_smooth = noisy_decision_smooth>>1 + (score<<3)>>1; */
	p_dm_odm->noisy_decision_smooth = (p_dm_odm->noisy_decision_smooth >> 1) + (score << 2);

	/* Round the noisy_decision_smooth: +"3" comes from (2^3)/2-1 */
	score_smooth = (total_cca_cnt >= 300) ? ((p_dm_odm->noisy_decision_smooth + 3) >> 3) : 0;

	p_dm_odm->noisy_decision = (score_smooth >= 3) ? 1 : 0;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_NOISY_DETECT, ODM_DBG_LOUD,
		("[NoisyDetection] total_cca_cnt=%d, total_fa_cnt=%d, noisy_decision_smooth=%d, score=%d, score_smooth=%d, p_dm_odm->noisy_decision=%d\n",
		total_cca_cnt, total_fa_cnt, p_dm_odm->noisy_decision_smooth, score, score_smooth, p_dm_odm->noisy_decision));

}

void
phydm_set_ext_switch(
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
	u32			ext_ant_switch =  dm_value[0];

	if (p_dm_odm->support_ic_type & (ODM_RTL8821 | ODM_RTL8881A)) {

		/*Output Pin Settings*/
		odm_set_mac_reg(p_dm_odm, 0x4C, BIT(23), 0); /*select DPDT_P and DPDT_N as output pin*/
		odm_set_mac_reg(p_dm_odm, 0x4C, BIT(24), 1); /*by WLAN control*/

		odm_set_bb_reg(p_dm_odm, 0xCB4, 0xF, 7); /*DPDT_P = 1b'0*/
		odm_set_bb_reg(p_dm_odm, 0xCB4, 0xF0, 7); /*DPDT_N = 1b'0*/

		if (ext_ant_switch == MAIN_ANT) {
			odm_set_bb_reg(p_dm_odm, 0xCB4, (BIT(29) | BIT(28)), 1);
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("***8821A set ant switch = 2b'01 (Main)\n"));
		} else if (ext_ant_switch == AUX_ANT) {
			odm_set_bb_reg(p_dm_odm, 0xCB4, BIT(29) | BIT(28), 2);
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("***8821A set ant switch = 2b'10 (Aux)\n"));
		}
	}
}

static void
phydm_csi_mask_enable(
	void		*p_dm_void,
	u32		enable
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32		reg_value = 0;

	reg_value = (enable == CSI_MASK_ENABLE) ? 1 : 0;

	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES) {

		odm_set_bb_reg(p_dm_odm, 0xD2C, BIT(28), reg_value);
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("Enable CSI Mask:  Reg 0xD2C[28] = ((0x%x))\n", reg_value));

	} else if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES) {

		odm_set_bb_reg(p_dm_odm, 0x874, BIT(0), reg_value);
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("Enable CSI Mask:  Reg 0x874[0] = ((0x%x))\n", reg_value));
	}

}

static void
phydm_clean_all_csi_mask(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES) {

		odm_set_bb_reg(p_dm_odm, 0xD40, MASKDWORD, 0);
		odm_set_bb_reg(p_dm_odm, 0xD44, MASKDWORD, 0);
		odm_set_bb_reg(p_dm_odm, 0xD48, MASKDWORD, 0);
		odm_set_bb_reg(p_dm_odm, 0xD4c, MASKDWORD, 0);

	} else if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES) {

		odm_set_bb_reg(p_dm_odm, 0x880, MASKDWORD, 0);
		odm_set_bb_reg(p_dm_odm, 0x884, MASKDWORD, 0);
		odm_set_bb_reg(p_dm_odm, 0x888, MASKDWORD, 0);
		odm_set_bb_reg(p_dm_odm, 0x88c, MASKDWORD, 0);
		odm_set_bb_reg(p_dm_odm, 0x890, MASKDWORD, 0);
		odm_set_bb_reg(p_dm_odm, 0x894, MASKDWORD, 0);
		odm_set_bb_reg(p_dm_odm, 0x898, MASKDWORD, 0);
		odm_set_bb_reg(p_dm_odm, 0x89c, MASKDWORD, 0);
	}
}

static void
phydm_set_csi_mask_reg(
	void		*p_dm_void,
	u32		tone_idx_tmp,
	u8		tone_direction
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8		byte_offset, bit_offset;
	u32		target_reg;
	u8		reg_tmp_value;
	u32		tone_num = 64;
	u32		tone_num_shift = 0;
	u32		csi_mask_reg_p = 0, csi_mask_reg_n = 0;

	/* calculate real tone idx*/
	if ((tone_idx_tmp % 10) >= 5)
		tone_idx_tmp += 10;

	tone_idx_tmp = (tone_idx_tmp / 10);

	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES) {

		tone_num = 64;
		csi_mask_reg_p = 0xD40;
		csi_mask_reg_n = 0xD48;

	} else if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES) {

		tone_num = 128;
		csi_mask_reg_p = 0x880;
		csi_mask_reg_n = 0x890;
	}

	if (tone_direction == FREQ_POSITIVE) {

		if (tone_idx_tmp >= (tone_num - 1))
			tone_idx_tmp = (tone_num - 1);

		byte_offset = (u8)(tone_idx_tmp >> 3);
		bit_offset = (u8)(tone_idx_tmp & 0x7);
		target_reg = csi_mask_reg_p + byte_offset;

	} else {
		tone_num_shift = tone_num;

		if (tone_idx_tmp >= tone_num)
			tone_idx_tmp = tone_num;

		tone_idx_tmp = tone_num - tone_idx_tmp;

		byte_offset = (u8)(tone_idx_tmp >> 3);
		bit_offset = (u8)(tone_idx_tmp & 0x7);
		target_reg = csi_mask_reg_n + byte_offset;
	}

	reg_tmp_value = odm_read_1byte(p_dm_odm, target_reg);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("Pre Mask tone idx[%d]:  Reg0x%x = ((0x%x))\n", (tone_idx_tmp + tone_num_shift), target_reg, reg_tmp_value));
	reg_tmp_value |= BIT(bit_offset);
	odm_write_1byte(p_dm_odm, target_reg, reg_tmp_value);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("New Mask tone idx[%d]:  Reg0x%x = ((0x%x))\n", (tone_idx_tmp + tone_num_shift), target_reg, reg_tmp_value));
}

static void
phydm_set_nbi_reg(
	void		*p_dm_void,
	u32		tone_idx_tmp,
	u32		bw
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32	nbi_table_128[NBI_TABLE_SIZE_128] = {25, 55, 85, 115, 135, 155, 185, 205, 225, 245,		/*1~10*/		/*tone_idx X 10*/
		     265, 285, 305, 335, 355, 375, 395, 415, 435, 455,	/*11~20*/
					     485, 505, 525, 555, 585, 615, 635
						};				/*21~27*/

	u32	nbi_table_256[NBI_TABLE_SIZE_256] = { 25,   55,   85, 115, 135, 155, 175, 195, 225, 245,	/*1~10*/
		265, 285, 305, 325, 345, 365, 385, 405, 425, 445,	/*11~20*/
		465, 485, 505, 525, 545, 565, 585, 605, 625, 645,	/*21~30*/
		665, 695, 715, 735, 755, 775, 795, 815, 835, 855,	/*31~40*/
		875, 895, 915, 935, 955, 975, 995, 1015, 1035, 1055,	/*41~50*/
		      1085, 1105, 1125, 1145, 1175, 1195, 1225, 1255, 1275
						};	/*51~59*/

	u32	reg_idx = 0;
	u32	i;
	u8	nbi_table_idx = FFT_128_TYPE;

	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES)

		nbi_table_idx = FFT_128_TYPE;
	else if (p_dm_odm->support_ic_type & ODM_IC_11AC_1_SERIES)

		nbi_table_idx = FFT_256_TYPE;
	else if (p_dm_odm->support_ic_type & ODM_IC_11AC_2_SERIES) {

		if (bw == 80)
			nbi_table_idx = FFT_256_TYPE;
		else /*20M, 40M*/
			nbi_table_idx = FFT_128_TYPE;
	}

	if (nbi_table_idx == FFT_128_TYPE) {

		for (i = 0; i < NBI_TABLE_SIZE_128; i++) {
			if (tone_idx_tmp < nbi_table_128[i]) {
				reg_idx = i + 1;
				break;
			}
		}

	} else if (nbi_table_idx == FFT_256_TYPE) {

		for (i = 0; i < NBI_TABLE_SIZE_256; i++) {
			if (tone_idx_tmp < nbi_table_256[i]) {
				reg_idx = i + 1;
				break;
			}
		}
	}

	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES) {
		odm_set_bb_reg(p_dm_odm, 0xc40, 0x1f000000, reg_idx);
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("Set tone idx:  Reg0xC40[28:24] = ((0x%x))\n", reg_idx));
		/**/
	} else {
		odm_set_bb_reg(p_dm_odm, 0x87c, 0xfc000, reg_idx);
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("Set tone idx: Reg0x87C[19:14] = ((0x%x))\n", reg_idx));
		/**/
	}
}


static void
phydm_nbi_enable(
	void		*p_dm_void,
	u32		enable
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32		reg_value = 0;

	reg_value = (enable == NBI_ENABLE) ? 1 : 0;

	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES) {

		odm_set_bb_reg(p_dm_odm, 0xc40, BIT(9), reg_value);
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("Enable NBI Reg0xC40[9] = ((0x%x))\n", reg_value));

	} else if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES) {

		odm_set_bb_reg(p_dm_odm, 0x87c, BIT(13), reg_value);
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("Enable NBI Reg0x87C[13] = ((0x%x))\n", reg_value));
	}
}

static u8
phydm_calculate_fc(
	void		*p_dm_void,
	u32		channel,
	u32		bw,
	u32		second_ch,
	u32		*fc_in
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32		fc = *fc_in;
	u32		start_ch_per_40m[NUM_START_CH_40M] = {36, 44, 52, 60, 100, 108, 116, 124, 132, 140, 149, 157, 165, 173};
	u32		start_ch_per_80m[NUM_START_CH_80M] = {36, 52, 100, 116, 132, 149, 165};
	u32		*p_start_ch = &(start_ch_per_40m[0]);
	u32		num_start_channel = NUM_START_CH_40M;
	u32		channel_offset = 0;
	u32		i;

	/*2.4G*/
	if (channel <= 14 && channel > 0) {

		if (bw == 80)
			return	SET_ERROR;

		fc = 2412 + (channel - 1) * 5;

		if (bw == 40 && (second_ch == PHYDM_ABOVE)) {

			if (channel >= 10) {
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("CH = ((%d)), Scnd_CH = ((%d)) Error setting\n", channel, second_ch));
				return	SET_ERROR;
			}
			fc += 10;
		} else if (bw == 40 && (second_ch == PHYDM_BELOW)) {

			if (channel <= 2) {
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("CH = ((%d)), Scnd_CH = ((%d)) Error setting\n", channel, second_ch));
				return	SET_ERROR;
			}
			fc -= 10;
		}
	}
	/*5G*/
	else if (channel >= 36 && channel <= 177) {

		if (bw != 20) {

			if (bw == 40) {
				num_start_channel = NUM_START_CH_40M;
				p_start_ch = &(start_ch_per_40m[0]);
				channel_offset = CH_OFFSET_40M;
			} else if (bw == 80) {
				num_start_channel = NUM_START_CH_80M;
				p_start_ch = &(start_ch_per_80m[0]);
				channel_offset = CH_OFFSET_80M;
			}

			for (i = 0; i < num_start_channel; i++) {

				if (channel < p_start_ch[i + 1]) {
					channel = p_start_ch[i] + channel_offset;
					break;
				}
			}
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("Mod_CH = ((%d))\n", channel));
		}

		fc = 5180 + (channel - 36) * 5;

	} else {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("CH = ((%d)) Error setting\n", channel));
		return	SET_ERROR;
	}

	*fc_in = fc;

	return SET_SUCCESS;
}


static u8
phydm_calculate_intf_distance(
	void		*p_dm_void,
	u32		bw,
	u32		fc,
	u32		f_interference,
	u32		*p_tone_idx_tmp_in
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32		bw_up, bw_low;
	u32		int_distance;
	u32		tone_idx_tmp;
	u8		set_result = SET_NO_NEED;

	bw_up = fc + bw / 2;
	bw_low = fc - bw / 2;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("[f_l, fc, fh] = [ %d, %d, %d ], f_int = ((%d))\n", bw_low, fc, bw_up, f_interference));

	if ((f_interference >= bw_low) && (f_interference <= bw_up)) {

		int_distance = (fc >= f_interference) ? (fc - f_interference) : (f_interference - fc);
		tone_idx_tmp = (int_distance << 5); /* =10*(int_distance /0.3125) */
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("int_distance = ((%d MHz)) Mhz, tone_idx_tmp = ((%d.%d))\n", int_distance, (tone_idx_tmp / 10), (tone_idx_tmp % 10)));
		*p_tone_idx_tmp_in = tone_idx_tmp;
		set_result = SET_SUCCESS;
	}

	return	set_result;

}


static u8
phydm_csi_mask_setting(
	void		*p_dm_void,
	u32		enable,
	u32		channel,
	u32		bw,
	u32		f_interference,
	u32		second_ch
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32		fc;
	u32		int_distance;
	u8		tone_direction;
	u32		tone_idx_tmp;
	u8		set_result = SET_SUCCESS;

	if (enable == CSI_MASK_DISABLE) {
		set_result = SET_SUCCESS;
		phydm_clean_all_csi_mask(p_dm_odm);

	} else {

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("[Set CSI MASK_] CH = ((%d)), BW = ((%d)), f_intf = ((%d)), Scnd_CH = ((%s))\n",
			channel, bw, f_interference, (((bw == 20) || (channel > 14)) ? "Don't care" : (second_ch == PHYDM_ABOVE) ? "H" : "L")));

		/*calculate fc*/
		if (phydm_calculate_fc(p_dm_odm, channel, bw, second_ch, &fc) == SET_ERROR)
			set_result = SET_ERROR;

		else {
			/*calculate interference distance*/
			if (phydm_calculate_intf_distance(p_dm_odm, bw, fc, f_interference, &tone_idx_tmp) == SET_SUCCESS) {

				tone_direction = (f_interference >= fc) ? FREQ_POSITIVE : FREQ_NEGATIVE;
				phydm_set_csi_mask_reg(p_dm_odm, tone_idx_tmp, tone_direction);
				set_result = SET_SUCCESS;
			} else
				set_result = SET_NO_NEED;
		}
	}

	if (set_result == SET_SUCCESS)
		phydm_csi_mask_enable(p_dm_odm, enable);
	else
		phydm_csi_mask_enable(p_dm_odm, CSI_MASK_DISABLE);

	return	set_result;
}

u8
phydm_nbi_setting(
	void		*p_dm_void,
	u32		enable,
	u32		channel,
	u32		bw,
	u32		f_interference,
	u32		second_ch
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32		fc;
	u32		int_distance;
	u32		tone_idx_tmp;
	u8		set_result = SET_SUCCESS;
	u32		bw_max = 40;

	if (enable == NBI_DISABLE)
		set_result = SET_SUCCESS;

	else {

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_API, ODM_DBG_LOUD, ("[Set NBI] CH = ((%d)), BW = ((%d)), f_intf = ((%d)), Scnd_CH = ((%s))\n",
			channel, bw, f_interference, (((second_ch == PHYDM_DONT_CARE) || (bw == 20) || (channel > 14)) ? "Don't care" : (second_ch == PHYDM_ABOVE) ? "H" : "L")));

		/*calculate fc*/
		if (phydm_calculate_fc(p_dm_odm, channel, bw, second_ch, &fc) == SET_ERROR)
			set_result = SET_ERROR;

		else {
			/*calculate interference distance*/
			if (phydm_calculate_intf_distance(p_dm_odm, bw, fc, f_interference, &tone_idx_tmp) == SET_SUCCESS) {

				phydm_set_nbi_reg(p_dm_odm, tone_idx_tmp, bw);
				set_result = SET_SUCCESS;
			} else
				set_result = SET_NO_NEED;
		}
	}

	if (set_result == SET_SUCCESS)
		phydm_nbi_enable(p_dm_odm, enable);
	else
		phydm_nbi_enable(p_dm_odm, NBI_DISABLE);

	return	set_result;
}

void
phydm_api_debug(
	void		*p_dm_void,
	u32		function_map,
	u32		*const dm_value,
	u32		*_used,
	char			*output,
	u32		*_out_len
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32			used = *_used;
	u32			out_len = *_out_len;
	u32			channel =  dm_value[1];
	u32			bw =  dm_value[2];
	u32			f_interference =  dm_value[3];
	u32			second_ch =  dm_value[4];
	u8			set_result = 0;

	/*PHYDM_API_NBI*/
	/*-------------------------------------------------------------------------------------------------------------------------------*/
	if (function_map == PHYDM_API_NBI) {

		if (dm_value[0] == 100) {

			PHYDM_SNPRINTF((output + used, out_len - used, "[HELP-NBI]  EN(on=1, off=2)   CH   BW(20/40/80)  f_intf(Mhz)    Scnd_CH(L=1, H=2)\n"));
			return;

		} else if (dm_value[0] == NBI_ENABLE) {

			PHYDM_SNPRINTF((output + used, out_len - used, "[Enable NBI] CH = ((%d)), BW = ((%d)), f_intf = ((%d)), Scnd_CH = ((%s))\n",
				channel, bw, f_interference, ((second_ch == PHYDM_DONT_CARE) || (bw == 20) || (channel > 14)) ? "Don't care" : ((second_ch == PHYDM_ABOVE) ? "H" : "L")));
			set_result = phydm_nbi_setting(p_dm_odm, NBI_ENABLE, channel, bw, f_interference, second_ch);

		} else if (dm_value[0] == NBI_DISABLE) {

			PHYDM_SNPRINTF((output + used, out_len - used, "[Disable NBI]\n"));
			set_result = phydm_nbi_setting(p_dm_odm, NBI_DISABLE, channel, bw, f_interference, second_ch);

		} else

			set_result = SET_ERROR;
		PHYDM_SNPRINTF((output + used, out_len - used, "[NBI set result: %s]\n", (set_result == SET_SUCCESS) ? "Success" : ((set_result == SET_NO_NEED) ? "No need" : "Error")));

	}

	/*PHYDM_CSI_MASK*/
	/*-------------------------------------------------------------------------------------------------------------------------------*/
	else if (function_map == PHYDM_API_CSI_MASK) {

		if (dm_value[0] == 100) {

			PHYDM_SNPRINTF((output + used, out_len - used, "[HELP-CSI MASK]  EN(on=1, off=2)   CH   BW(20/40/80)  f_intf(Mhz)    Scnd_CH(L=1, H=2)\n"));
			return;

		} else if (dm_value[0] == CSI_MASK_ENABLE) {

			PHYDM_SNPRINTF((output + used, out_len - used, "[Enable CSI MASK] CH = ((%d)), BW = ((%d)), f_intf = ((%d)), Scnd_CH = ((%s))\n",
				channel, bw, f_interference, (channel > 14) ? "Don't care" : (((second_ch == PHYDM_DONT_CARE) || (bw == 20) || (channel > 14)) ? "H" : "L")));
			set_result = phydm_csi_mask_setting(p_dm_odm,	CSI_MASK_ENABLE, channel, bw, f_interference, second_ch);

		} else if (dm_value[0] == CSI_MASK_DISABLE) {

			PHYDM_SNPRINTF((output + used, out_len - used, "[Disable CSI MASK]\n"));
			set_result = phydm_csi_mask_setting(p_dm_odm, CSI_MASK_DISABLE, channel, bw, f_interference, second_ch);

		} else

			set_result = SET_ERROR;
		PHYDM_SNPRINTF((output + used, out_len - used, "[CSI MASK set result: %s]\n", (set_result == SET_SUCCESS) ? "Success" : ((set_result == SET_NO_NEED) ? "No need" : "Error")));
	}
}

void
phydm_receiver_blocking(
	void *p_dm_void
)
{
#ifdef CONFIG_RECEIVER_BLOCKING
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32	channel = *p_dm_odm->p_channel;
	u8	bw = *p_dm_odm->p_band_width;
	u8	set_result = 0;

	if (!(p_dm_odm->support_ic_type & ODM_RECEIVER_BLOCKING_SUPPORT))
		return;
	
	if (p_dm_odm->consecutive_idlel_time > 10 && p_dm_odm->mp_mode == false && p_dm_odm->adaptivity_enable == true) {
		if ((bw == ODM_BW20M) && (channel == 1)) {
			set_result = phydm_nbi_setting(p_dm_odm, NBI_ENABLE, channel, 20, 2410, PHYDM_DONT_CARE);
			p_dm_odm->is_receiver_blocking_en = true;
		} else if ((bw == ODM_BW20M) && (channel == 13)) {
			set_result = phydm_nbi_setting(p_dm_odm, NBI_ENABLE, channel, 20, 2473, PHYDM_DONT_CARE);
			p_dm_odm->is_receiver_blocking_en = true;
		} else if (*(p_dm_odm->p_is_scan_in_process) == false) {
			if (p_dm_odm->is_receiver_blocking_en && channel != 1 && channel != 13) {
				phydm_nbi_enable(p_dm_odm, NBI_DISABLE);
				odm_set_bb_reg(p_dm_odm, 0xc40, 0x1f000000, 0x1f);
				p_dm_odm->is_receiver_blocking_en = false;
			}
		}
	} else {
		if (p_dm_odm->is_receiver_blocking_en) {
			phydm_nbi_enable(p_dm_odm, NBI_DISABLE);
			odm_set_bb_reg(p_dm_odm, 0xc40, 0x1f000000, 0x1f);
			p_dm_odm->is_receiver_blocking_en = false;
		}
	}
	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, 
		("[NBI set result: %s]\n", (set_result == SET_SUCCESS ? "Success" : (set_result == SET_NO_NEED ? "No need" : "Error"))));
#endif
}
