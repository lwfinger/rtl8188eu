// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


/* ************************************************************
 * include files
 * ************************************************************ */

#include "mp_precomp.h"
#include "phydm_precomp.h"

#define READ_AND_CONFIG_MP(ic, txt) (odm_read_and_config_mp_##ic##txt(p_dm_odm))
#define READ_AND_CONFIG_TC(ic, txt) (odm_read_and_config_tc_##ic##txt(p_dm_odm))

#if (PHYDM_TESTCHIP_SUPPORT == 1)
#define READ_AND_CONFIG(ic, txt) do {\
		if (p_dm_odm->is_mp_chip)\
			READ_AND_CONFIG_MP(ic, txt);\
		else\
			READ_AND_CONFIG_TC(ic, txt);\
	} while (0)
#else
#define READ_AND_CONFIG     READ_AND_CONFIG_MP
#endif

#define READ_FIRMWARE_MP(ic, txt)		(odm_read_firmware_mp_##ic##txt(p_dm_odm, p_firmware, p_size))
#define READ_FIRMWARE_TC(ic, txt)		(odm_read_firmware_tc_##ic##txt(p_dm_odm, p_firmware, p_size))

#if (PHYDM_TESTCHIP_SUPPORT == 1)
#define READ_FIRMWARE(ic, txt) do {\
		if (p_dm_odm->is_mp_chip)\
			READ_FIRMWARE_MP(ic, txt);\
		else\
			READ_FIRMWARE_TC(ic, txt);\
	} while (0)
#else
#define READ_FIRMWARE     READ_FIRMWARE_MP
#endif

#define GET_VERSION_MP(ic, txt)		(odm_get_version_mp_##ic##txt())
#define GET_VERSION_TC(ic, txt)		(odm_get_version_tc_##ic##txt())

#if (PHYDM_TESTCHIP_SUPPORT == 1)
	#define GET_VERSION(ic, txt) (p_dm_odm->is_mp_chip ? GET_VERSION_MP(ic, txt) : GET_VERSION_TC(ic, txt))
#else
	#define GET_VERSION(ic, txt) GET_VERSION_MP(ic, txt)
#endif

static u8
odm_query_rx_pwr_percentage(
	s8		ant_power
)
{
	if ((ant_power <= -100) || (ant_power >= 20))
		return	0;
	else if (ant_power >= 0)
		return	100;
	else
		return 100 + ant_power;
}

/*
 * 2012/01/12 MH MOve some signal strength smooth method to MP HAL layer.
 * IF other SW team do not support the feature, remove this section.??
 *   */
static s32
odm_signal_scale_mapping_92c_series_patch_rt_cid_819x_lenovo(
	struct PHY_DM_STRUCT *p_dm_odm,
	s32 curr_sig
)
{
	s32 ret_sig = 0;
	return ret_sig;
}

static s32
odm_signal_scale_mapping_92c_series_patch_rt_cid_819x_netcore(
	struct PHY_DM_STRUCT *p_dm_odm,
	s32 curr_sig
)
{
	s32 ret_sig = 0;
	return ret_sig;
}

static s32
odm_signal_scale_mapping_92c_series(
	struct PHY_DM_STRUCT *p_dm_odm,
	s32 curr_sig
)
{
	s32 ret_sig = 0;

	if ((p_dm_odm->support_interface  == ODM_ITRF_USB) || (p_dm_odm->support_interface  == ODM_ITRF_SDIO)) {
		if (curr_sig >= 51 && curr_sig <= 100)
			ret_sig = 100;
		else if (curr_sig >= 41 && curr_sig <= 50)
			ret_sig = 80 + ((curr_sig - 40) * 2);
		else if (curr_sig >= 31 && curr_sig <= 40)
			ret_sig = 66 + (curr_sig - 30);
		else if (curr_sig >= 21 && curr_sig <= 30)
			ret_sig = 54 + (curr_sig - 20);
		else if (curr_sig >= 10 && curr_sig <= 20)
			ret_sig = 42 + (((curr_sig - 10) * 2) / 3);
		else if (curr_sig >= 5 && curr_sig <= 9)
			ret_sig = 22 + (((curr_sig - 5) * 3) / 2);
		else if (curr_sig >= 1 && curr_sig <= 4)
			ret_sig = 6 + (((curr_sig - 1) * 3) / 2);
		else
			ret_sig = curr_sig;
	}

	return ret_sig;
}
s32
odm_signal_scale_mapping(
	struct PHY_DM_STRUCT *p_dm_odm,
	s32 curr_sig
)
{
	{
#ifdef CONFIG_SIGNAL_SCALE_MAPPING
		return odm_signal_scale_mapping_92c_series(p_dm_odm, curr_sig);
#else
		return curr_sig;
#endif
	}

}

static u8 odm_sq_process_patch_rt_cid_819x_lenovo(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u8		is_cck_rate,
	u8		PWDB_ALL,
	u8		path,
	u8		RSSI
)
{
	u8	SQ = 0;
	return SQ;
}

static u8 odm_sq_process_patch_rt_cid_819x_acer(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u8		is_cck_rate,
	u8		PWDB_ALL,
	u8		path,
	u8		RSSI
)
{
	u8	SQ = 0;

	return SQ;
}

static u8
odm_evm_db_to_percentage(
	s8 value
)
{
	/*  */
	/* -33dB~0dB to 0%~99% */
	/*  */
	s8 ret_val;

	ret_val = value;
	ret_val /= 2;

	/*dbg_print("value=%d\n", value);*/
	/*ODM_RT_DISP(FRX, RX_PHY_SQ, ("EVMdbToPercentage92C value=%d / %x\n", ret_val, ret_val));*/
#ifdef ODM_EVM_ENHANCE_ANTDIV
	if (ret_val >= 0)
		ret_val = 0;

	if (ret_val <= -40)
		ret_val = -40;

	ret_val = 0 - ret_val;
	ret_val *= 3;
#else
	if (ret_val >= 0)
		ret_val = 0;

	if (ret_val <= -33)
		ret_val = -33;

	ret_val = 0 - ret_val;
	ret_val *= 3;

	if (ret_val == 99)
		ret_val = 100;
#endif

	return (u8)ret_val;
}

static u8
odm_evm_dbm_jaguar_series(
	s8 value
)
{
	s8 ret_val = value;

	/* -33dB~0dB to 33dB ~ 0dB */
	if (ret_val == -128)
		ret_val = 127;
	else if (ret_val < 0)
		ret_val = 0 - ret_val;

	ret_val  = ret_val >> 1;
	return (u8)ret_val;
}

static s16
odm_cfo(
	s8 value
)
{
	s16  ret_val;

	if (value < 0) {
		ret_val = 0 - value;
		ret_val = (ret_val << 1) + (ret_val >> 1) ;  /* *2.5~=312.5/2^7 */
		ret_val = ret_val | BIT(12);  /* set bit12 as 1 for negative cfo */
	} else {
		ret_val = value;
		ret_val = (ret_val << 1) + (ret_val >> 1) ; /* *2.5~=312.5/2^7 */
	}
	return ret_val;
}

static u8
phydm_rate_to_num_ss(
	struct PHY_DM_STRUCT		*p_dm_odm,
	u8			data_rate
)
{
	u8	num_ss = 1;

	if (data_rate  <= ODM_RATE54M)
		num_ss = 1;
	else if (data_rate  <= ODM_RATEMCS31)
		num_ss = ((data_rate  - ODM_RATEMCS0) >> 3) + 1;
	else if (data_rate  <= ODM_RATEVHTSS1MCS9)
		num_ss = 1;
	else if (data_rate  <= ODM_RATEVHTSS2MCS9)
		num_ss = 2;
	else if (data_rate  <= ODM_RATEVHTSS3MCS9)
		num_ss = 3;
	else if (data_rate  <= ODM_RATEVHTSS4MCS9)
		num_ss = 4;

	return num_ss;
}

#if (ODM_IC_11N_SERIES_SUPPORT == 1)

#if (RTL8703B_SUPPORT == 1)
s8
odm_CCKRSSI_8703B(
	u16	LNA_idx,
	u8	VGA_idx
)
{
	s8	rx_pwr_all = 0x00;

	switch (LNA_idx) {
	case 0xf:
		rx_pwr_all = -48 - (2 * VGA_idx);
		break;
	case 0xb:
		rx_pwr_all = -42 - (2 * VGA_idx); /*TBD*/
		break;
	case 0xa:
		rx_pwr_all = -36 - (2 * VGA_idx);
		break;
	case 8:
		rx_pwr_all = -32 - (2 * VGA_idx);
		break;
	case 7:
		rx_pwr_all = -19 - (2 * VGA_idx);
		break;
	case 4:
		rx_pwr_all = -6 - (2 * VGA_idx);
		break;
	case 0:
		rx_pwr_all = -2 - (2 * VGA_idx);
		break;
	default:
		/*rx_pwr_all = -53+(2*(31-VGA_idx));*/
		/*dbg_print("wrong LNA index\n");*/
		break;

	}
	return	rx_pwr_all;
}
#endif

#if (RTL8195A_SUPPORT == 1)
s8
odm_CCKRSSI_8195A(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u16		LNA_idx,
	u8		VGA_idx
)
{
	s8	rx_pwr_all = 0;
	s8	lna_gain = 0;
	s8	lna_gain_table_0[8] = {0, -8, -15, -22, -29, -36, -45, -54};
	s8	lna_gain_table_1[8] = {0, -8, -15, -22, -29, -36, -45, -54};/*use 8195A to calibrate this table. 2016.06.24, Dino*/

	if (p_dm_odm->cck_agc_report_type == 0)
		lna_gain = lna_gain_table_0[LNA_idx];
	else
		lna_gain = lna_gain_table_1[LNA_idx];

	rx_pwr_all = lna_gain - (2 * VGA_idx);

	return	rx_pwr_all;
}
#endif

#if (RTL8192E_SUPPORT == 1)
s8
odm_CCKRSSI_8192E(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u16		LNA_idx,
	u8		VGA_idx
)
{
	s8	rx_pwr_all = 0;
	s8	lna_gain = 0;
	s8	lna_gain_table_0[8] = {15, 9, -10, -21, -23, -27, -43, -44};
	s8	lna_gain_table_1[8] = {24, 18, 13, -4, -11, -18, -31, -36};/*use 8192EU to calibrate this table. 2015.12.15, Dino*/

	if (p_dm_odm->cck_agc_report_type == 0)
		lna_gain = lna_gain_table_0[LNA_idx];
	else
		lna_gain = lna_gain_table_1[LNA_idx];

	rx_pwr_all = lna_gain - (2 * VGA_idx);

	return	rx_pwr_all;
}
#endif

#if (RTL8188E_SUPPORT == 1)
static s8
odm_CCKRSSI_8188E(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u16		LNA_idx,
	u8		VGA_idx
)
{
	s8	rx_pwr_all = 0;
	s8	lna_gain = 0;
	s8	lna_gain_table_0[8] = {17, -1, -13, -29, -32, -35, -38, -41};/*only use lna0/1/2/3/7*/
	s8	lna_gain_table_1[8] = {29, 20, 12, 3, -6, -15, -24, -33}; /*only use lna3 /7*/

	if (p_dm_odm->cut_version >= ODM_CUT_I) /*SMIC*/
		lna_gain = lna_gain_table_0[LNA_idx];
	else	 /*TSMC*/
		lna_gain = lna_gain_table_1[LNA_idx];

	rx_pwr_all = lna_gain - (2 * VGA_idx);

	return	rx_pwr_all;
}
#endif

static void
odm_rx_phy_status92c_series_parsing(
	struct PHY_DM_STRUCT					*p_dm_odm,
	struct _odm_phy_status_info_			*p_phy_info,
	u8						*p_phy_status,
	struct _odm_per_pkt_info_			*p_pktinfo
)
{
	struct _sw_antenna_switch_				*p_dm_swat_table = &p_dm_odm->dm_swat_table;
	u8				i, max_spatial_stream;
	s8				rx_pwr[4], rx_pwr_all = 0;
	u8				EVM, PWDB_ALL = 0, PWDB_ALL_BT;
	u8				RSSI, total_rssi = 0;
	bool				is_cck_rate = false;
	u8				rf_rx_num = 0;
	u8				cck_highpwr = 0;
	u8				LNA_idx = 0;
	u8				VGA_idx = 0;
	u8				cck_agc_rpt;
	u8				num_ss;
	struct _phy_status_rpt_8192cd *p_phy_sta_rpt = (struct _phy_status_rpt_8192cd *)p_phy_status;

	is_cck_rate = (p_pktinfo->data_rate <= ODM_RATE11M) ? true : false;

	if (p_pktinfo->is_to_self)
		p_dm_odm->curr_station_id = p_pktinfo->station_id;

	p_phy_info->rx_mimo_signal_quality[ODM_RF_PATH_A] = -1;
	p_phy_info->rx_mimo_signal_quality[ODM_RF_PATH_B] = -1;

	if (is_cck_rate) {
		p_dm_odm->phy_dbg_info.num_qry_phy_status_cck++;
		cck_agc_rpt =  p_phy_sta_rpt->cck_agc_rpt_ofdm_cfosho_a ;

		if (p_dm_odm->support_ic_type & (ODM_RTL8703B)) {

#if (RTL8703B_SUPPORT == 1)
			if (p_dm_odm->cck_agc_report_type == 1) {  /*4 bit LNA*/

				u8 cck_agc_rpt_b = (p_phy_sta_rpt->cck_rpt_b_ofdm_cfosho_b & BIT(7)) ? 1 : 0;

				LNA_idx = (cck_agc_rpt_b << 3) | ((cck_agc_rpt & 0xE0) >> 5);
				VGA_idx = (cck_agc_rpt & 0x1F);

				rx_pwr_all = odm_CCKRSSI_8703B(LNA_idx, VGA_idx);
			}
#endif
		} else { /*3 bit LNA*/

			LNA_idx = ((cck_agc_rpt & 0xE0) >> 5);
			VGA_idx = (cck_agc_rpt & 0x1F);

			if (p_dm_odm->support_ic_type & (ODM_RTL8188E)) {

#if (RTL8188E_SUPPORT == 1)
				rx_pwr_all = odm_CCKRSSI_8188E(p_dm_odm, LNA_idx, VGA_idx);
				/**/
#endif
			}
#if (RTL8192E_SUPPORT == 1)
			else if (p_dm_odm->support_ic_type & (ODM_RTL8192E)) {

				rx_pwr_all = odm_CCKRSSI_8192E(p_dm_odm, LNA_idx, VGA_idx);
				/**/
			}
#endif
#if (RTL8723B_SUPPORT == 1)
			else if (p_dm_odm->support_ic_type & (ODM_RTL8723B)) {

				rx_pwr_all = odm_CCKRSSI_8723B(LNA_idx, VGA_idx);
				/**/
			}
#endif
#if (RTL8188F_SUPPORT == 1)
			else if (p_dm_odm->support_ic_type & (ODM_RTL8188F)) {

				rx_pwr_all = odm_CCKRSSI_8188E(p_dm_odm, LNA_idx, VGA_idx);
				/**/
			}
#endif
#if (RTL8195A_SUPPORT == 1)
			else if (p_dm_odm->support_ic_type & (ODM_RTL8195A)) {

				rx_pwr_all = odm_CCKRSSI_8195A(LNA_idx, VGA_idx);
				/**/
			}
#endif
		}

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("ext_lna_gain (( %d )), LNA_idx: (( 0x%x )), VGA_idx: (( 0x%x )), rx_pwr_all: (( %d ))\n",
			p_dm_odm->ext_lna_gain, LNA_idx, VGA_idx, rx_pwr_all));

		if (p_dm_odm->board_type & ODM_BOARD_EXT_LNA)
			rx_pwr_all -= p_dm_odm->ext_lna_gain;

		PWDB_ALL = odm_query_rx_pwr_percentage(rx_pwr_all);

		if (p_pktinfo->is_to_self) {
			p_dm_odm->cck_lna_idx = LNA_idx;
			p_dm_odm->cck_vga_idx = VGA_idx;
		}
		p_phy_info->rx_pwdb_all = PWDB_ALL;

		p_phy_info->bt_rx_rssi_percentage = PWDB_ALL;
		p_phy_info->recv_signal_power = rx_pwr_all;
		/*  */
		/* (3) Get Signal Quality (EVM) */
		/*  */
		/* if(p_pktinfo->is_packet_match_bssid) */
		{
			u8	SQ, SQ_rpt;

				if (p_phy_info->rx_pwdb_all > 40 && !p_dm_odm->is_in_hct_test)
					SQ = 100;
				else {
					SQ_rpt = p_phy_sta_rpt->cck_sig_qual_ofdm_pwdb_all;

					if (SQ_rpt > 64)
						SQ = 0;
					else if (SQ_rpt < 20)
						SQ = 100;
					else
						SQ = ((64 - SQ_rpt) * 100) / 44;

				}

			/* dbg_print("cck SQ = %d\n", SQ); */
			p_phy_info->signal_quality = SQ;
			p_phy_info->rx_mimo_signal_quality[ODM_RF_PATH_A] = SQ;
			p_phy_info->rx_mimo_signal_quality[ODM_RF_PATH_B] = -1;
		}

		for (i = ODM_RF_PATH_A; i < ODM_RF_PATH_MAX; i++) {
			if (i == 0)
				p_phy_info->rx_mimo_signal_strength[0] = PWDB_ALL;
			else
				p_phy_info->rx_mimo_signal_strength[1] = 0;
		}
	} else { /* 2 is OFDM rate */
		p_dm_odm->phy_dbg_info.num_qry_phy_status_ofdm++;

		/*  */
		/* (1)Get RSSI for HT rate */
		/*  */

		for (i = ODM_RF_PATH_A; i < ODM_RF_PATH_MAX; i++) {
			/* 2008/01/30 MH we will judge RF RX path now. */
			if (p_dm_odm->rf_path_rx_enable & BIT(i))
				rf_rx_num++;
			/* else */
			/* continue; */

			rx_pwr[i] = ((p_phy_sta_rpt->path_agc[i].gain & 0x3F) * 2) - 110;

			if (p_pktinfo->is_to_self) {
				p_dm_odm->ofdm_agc_idx[i] = (p_phy_sta_rpt->path_agc[i].gain & 0x3F);
				/**/
			}

			p_phy_info->rx_pwr[i] = rx_pwr[i];

			/* Translate DBM to percentage. */
			RSSI = odm_query_rx_pwr_percentage(rx_pwr[i]);
			total_rssi += RSSI;
			/* RT_DISP(FRX, RX_PHY_SS, ("RF-%d RXPWR=%x RSSI=%d\n", i, rx_pwr[i], RSSI)); */

			p_phy_info->rx_mimo_signal_strength[i] = (u8) RSSI;

			/* Get Rx snr value in DB */
			p_phy_info->rx_snr[i] = p_dm_odm->phy_dbg_info.rx_snr_db[i] = (s32)(p_phy_sta_rpt->path_rxsnr[i] / 2);

		}

		/*  */
		/* (2)PWDB, Average PWDB cacluated by hardware (for rate adaptive) */
		/*  */
		rx_pwr_all = (((p_phy_sta_rpt->cck_sig_qual_ofdm_pwdb_all) >> 1) & 0x7f) - 110;

		PWDB_ALL_BT = PWDB_ALL = odm_query_rx_pwr_percentage(rx_pwr_all);

		p_phy_info->rx_pwdb_all = PWDB_ALL;
		/* ODM_RT_TRACE(p_dm_odm,ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("ODM OFDM RSSI=%d\n",p_phy_info->rx_pwdb_all)); */
		p_phy_info->bt_rx_rssi_percentage = PWDB_ALL_BT;
		p_phy_info->rx_power = rx_pwr_all;
		p_phy_info->recv_signal_power = rx_pwr_all;

		if ((p_dm_odm->support_platform == ODM_WIN) && (p_dm_odm->patch_id == 19)) {
			/* do nothing */
		} else if ((p_dm_odm->support_platform == ODM_WIN) && (p_dm_odm->patch_id == 25)) {
			/* do nothing */
		} else { /* p_mgnt_info->customer_id != RT_CID_819X_LENOVO */
			/*  */
			/* (3)EVM of HT rate */
			/*  */
			if (p_pktinfo->data_rate >= ODM_RATEMCS8 && p_pktinfo->data_rate <= ODM_RATEMCS15)
				max_spatial_stream = 2; /* both spatial stream make sense */
			else
				max_spatial_stream = 1; /* only spatial stream 1 makes sense */

			for (i = 0; i < max_spatial_stream; i++) {
				/* Do not use shift operation like "rx_evmX >>= 1" because the compilor of free build environment */
				/* fill most significant bit to "zero" when doing shifting operation which may change a negative */
				/* value to positive one, then the dbm value (which is supposed to be negative)  is not correct anymore. */
				EVM = odm_evm_db_to_percentage((p_phy_sta_rpt->stream_rxevm[i]));	/* dbm */

				/* GET_RX_STATUS_DESC_RX_MCS(p_desc), p_drv_info->rxevm[i], "%", EVM)); */

				/* if(p_pktinfo->is_packet_match_bssid) */
				{
					if (i == ODM_RF_PATH_A) /* Fill value in RFD, Get the first spatial stream only */
						p_phy_info->signal_quality = (u8)(EVM & 0xff);
					p_phy_info->rx_mimo_signal_quality[i] = (u8)(EVM & 0xff);
				}
			}
		}

		num_ss = phydm_rate_to_num_ss(p_dm_odm, p_pktinfo->data_rate);
		odm_parsing_cfo(p_dm_odm, p_pktinfo, p_phy_sta_rpt->path_cfotail, num_ss);

	}
	/* UI BSS List signal strength(in percentage), make it good looking, from 0~100. */
	/* It is assigned to the BSS List in GetValueFromBeaconOrProbeRsp(). */
	if (is_cck_rate) {
		p_phy_info->signal_strength = (u8)(odm_signal_scale_mapping(p_dm_odm, PWDB_ALL));/*PWDB_ALL;*/
	} else {
		if (rf_rx_num != 0)
			p_phy_info->signal_strength = (u8)(odm_signal_scale_mapping(p_dm_odm, total_rssi /= rf_rx_num));
	}

	/* For 92C/92D HW (Hybrid) Antenna Diversity */
#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
	/* For 88E HW Antenna Diversity */
	p_dm_odm->dm_fat_table.antsel_rx_keep_0 = p_phy_sta_rpt->ant_sel;
	p_dm_odm->dm_fat_table.antsel_rx_keep_1 = p_phy_sta_rpt->ant_sel_b;
	p_dm_odm->dm_fat_table.antsel_rx_keep_2 = p_phy_sta_rpt->antsel_rx_keep_2;
#endif
}
#endif

#if	ODM_IC_11AC_SERIES_SUPPORT

void
odm_rx_phy_bw_jaguar_series_parsing(
	struct _odm_phy_status_info_			*p_phy_info,
	struct _odm_per_pkt_info_			*p_pktinfo,
	struct _phy_status_rpt_8812		*p_phy_sta_rpt
)
{

	if (p_pktinfo->data_rate <= ODM_RATE54M) {
		switch (p_phy_sta_rpt->r_RFMOD) {
		case 1:
			if (p_phy_sta_rpt->sub_chnl == 0)
				p_phy_info->band_width = 1;
			else
				p_phy_info->band_width = 0;
			break;

		case 2:
			if (p_phy_sta_rpt->sub_chnl == 0)
				p_phy_info->band_width = 2;
			else if (p_phy_sta_rpt->sub_chnl == 9 || p_phy_sta_rpt->sub_chnl == 10)
				p_phy_info->band_width = 1;
			else
				p_phy_info->band_width = 0;
			break;

		default:
		case 0:
			p_phy_info->band_width = 0;
			break;
		}
	}

}

void
odm_rx_phy_status_jaguar_series_parsing(
	struct PHY_DM_STRUCT					*p_dm_odm,
	struct _odm_phy_status_info_			*p_phy_info,
	u8						*p_phy_status,
	struct _odm_per_pkt_info_			*p_pktinfo
)
{
	u8					i, max_spatial_stream;
	s8					rx_pwr[4], rx_pwr_all = 0;
	u8					EVM, evm_dbm, PWDB_ALL = 0, PWDB_ALL_BT;
	u8					RSSI, avg_rssi = 0, best_rssi = 0, second_rssi = 0;
	u8					is_cck_rate = 0;
	u8					rf_rx_num = 0;
	u8					cck_highpwr = 0;
	u8					LNA_idx, VGA_idx;
	struct _phy_status_rpt_8812 *p_phy_sta_rpt = (struct _phy_status_rpt_8812 *)p_phy_status;
	struct _FAST_ANTENNA_TRAINNING_					*p_dm_fat_table = &p_dm_odm->dm_fat_table;
	u8					num_ss;

	odm_rx_phy_bw_jaguar_series_parsing(p_phy_info, p_pktinfo, p_phy_sta_rpt);

	if (p_pktinfo->data_rate <= ODM_RATE11M)
		is_cck_rate = true;
	else
		is_cck_rate = false;

	if (p_pktinfo->is_to_self)
		p_dm_odm->curr_station_id = p_pktinfo->station_id;
	else
		p_dm_odm->curr_station_id = 0xff;

	p_phy_info->rx_mimo_signal_quality[ODM_RF_PATH_A] = -1;
	p_phy_info->rx_mimo_signal_quality[ODM_RF_PATH_B] = -1;
	p_phy_info->rx_mimo_signal_quality[ODM_RF_PATH_C] = -1;
	p_phy_info->rx_mimo_signal_quality[ODM_RF_PATH_D] = -1;

	if (is_cck_rate) {
		u8 cck_agc_rpt;
		p_dm_odm->phy_dbg_info.num_qry_phy_status_cck++;

		/*(1)Hardware does not provide RSSI for CCK*/
		/*(2)PWDB, Average PWDB calculated by hardware (for rate adaptive)*/

		/*if(p_hal_data->e_rf_power_state == e_rf_on)*/
		cck_highpwr = p_dm_odm->is_cck_high_power;
		/*else*/
		/*cck_highpwr = false;*/

		cck_agc_rpt =  p_phy_sta_rpt->cfosho[0] ;
		LNA_idx = ((cck_agc_rpt & 0xE0) >> 5);
		VGA_idx = (cck_agc_rpt & 0x1F);

		if (p_dm_odm->support_ic_type == ODM_RTL8812) {
			switch (LNA_idx) {
			case 7:
				if (VGA_idx <= 27)
					rx_pwr_all = -100 + 2 * (27 - VGA_idx); /*VGA_idx = 27~2*/
				else
					rx_pwr_all = -100;
				break;
			case 6:
				rx_pwr_all = -48 + 2 * (2 - VGA_idx); /*VGA_idx = 2~0*/
				break;
			case 5:
				rx_pwr_all = -42 + 2 * (7 - VGA_idx); /*VGA_idx = 7~5*/
				break;
			case 4:
				rx_pwr_all = -36 + 2 * (7 - VGA_idx); /*VGA_idx = 7~4*/
				break;
			case 3:
				/*rx_pwr_all = -28 + 2*(7-VGA_idx); VGA_idx = 7~0*/
				rx_pwr_all = -24 + 2 * (7 - VGA_idx); /*VGA_idx = 7~0*/
				break;
			case 2:
				if (cck_highpwr)
					rx_pwr_all = -12 + 2 * (5 - VGA_idx); /*VGA_idx = 5~0*/
				else
					rx_pwr_all = -6 + 2 * (5 - VGA_idx);
				break;
			case 1:
				rx_pwr_all = 8 - 2 * VGA_idx;
				break;
			case 0:
				rx_pwr_all = 14 - 2 * VGA_idx;
				break;
			default:
				/*dbg_print("CCK Exception default\n");*/
				break;
			}
			rx_pwr_all += 6;
			PWDB_ALL = odm_query_rx_pwr_percentage(rx_pwr_all);

			if (cck_highpwr == false) {
				if (PWDB_ALL >= 80)
					PWDB_ALL = ((PWDB_ALL - 80) << 1) + ((PWDB_ALL - 80) >> 1) + 80;
				else if ((PWDB_ALL <= 78) && (PWDB_ALL >= 20))
					PWDB_ALL += 3;
				if (PWDB_ALL > 100)
					PWDB_ALL = 100;
			}
		} else if (p_dm_odm->support_ic_type & (ODM_RTL8821 | ODM_RTL8881A)) {
			s8 pout = -6;

			switch (LNA_idx) {
			case 5:
				rx_pwr_all = pout - 32 - (2 * VGA_idx);
				break;
			case 4:
				rx_pwr_all = pout - 24 - (2 * VGA_idx);
				break;
			case 2:
				rx_pwr_all = pout - 11 - (2 * VGA_idx);
				break;
			case 1:
				rx_pwr_all = pout + 5 - (2 * VGA_idx);
				break;
			case 0:
				rx_pwr_all = pout + 21 - (2 * VGA_idx);
				break;
			}
			PWDB_ALL = odm_query_rx_pwr_percentage(rx_pwr_all);
		} else if (p_dm_odm->support_ic_type == ODM_RTL8814A || p_dm_odm->support_ic_type == ODM_RTL8822B) {
			s8 pout = -6;

			switch (LNA_idx) {
			/*CCK only use LNA: 2, 3, 5, 7*/
			case 7:
				rx_pwr_all = pout - 32 - (2 * VGA_idx);
				break;
			case 5:
				rx_pwr_all = pout - 22 - (2 * VGA_idx);
				break;
			case 3:
				rx_pwr_all = pout - 2 - (2 * VGA_idx);
				break;
			case 2:
				rx_pwr_all = pout + 5 - (2 * VGA_idx);
				break;
			/*case 6:*/
			/*rx_pwr_all = pout -26 - (2*VGA_idx);*/
			/*break;*/
			/*case 4:*/
			/*rx_pwr_all = pout - 8 - (2*VGA_idx);*/
			/*break;*/
			/*case 1:*/
			/*rx_pwr_all = pout + 21 - (2*VGA_idx);*/
			/*break;*/
			/*case 0:*/
			/*rx_pwr_all = pout + 10 - (2*VGA_idx);*/
			/*	break; */
			default:
				/* dbg_print("CCK Exception default\n"); */
				break;
			}
			PWDB_ALL = odm_query_rx_pwr_percentage(rx_pwr_all);
		}

		p_dm_odm->cck_lna_idx = LNA_idx;
		p_dm_odm->cck_vga_idx = VGA_idx;
		p_phy_info->rx_pwdb_all = PWDB_ALL;
		/* if(p_pktinfo->station_id == 0) */
		/* { */
		/*	dbg_print("CCK: LNA_idx = %d, VGA_idx = %d, p_phy_info->rx_pwdb_all = %d\n", */
		/*		LNA_idx, VGA_idx, p_phy_info->rx_pwdb_all); */
		/* } */
		p_phy_info->bt_rx_rssi_percentage = PWDB_ALL;
		p_phy_info->recv_signal_power = rx_pwr_all;
		/*(3) Get Signal Quality (EVM)*/
		/*if (p_pktinfo->is_packet_match_bssid)*/
		{
			u8	SQ, SQ_rpt;

			if ((p_dm_odm->support_platform == ODM_WIN) &&
			    (p_dm_odm->patch_id == RT_CID_819X_LENOVO))
				SQ = odm_sq_process_patch_rt_cid_819x_lenovo(p_dm_odm, is_cck_rate, PWDB_ALL, 0, 0);
			else if (p_phy_info->rx_pwdb_all > 40 && !p_dm_odm->is_in_hct_test)
				SQ = 100;
			else {
				SQ_rpt = p_phy_sta_rpt->pwdb_all;

				if (SQ_rpt > 64)
					SQ = 0;
				else if (SQ_rpt < 20)
					SQ = 100;
				else
					SQ = ((64 - SQ_rpt) * 100) / 44;
			}

			/* dbg_print("cck SQ = %d\n", SQ); */
			p_phy_info->signal_quality = SQ;
			p_phy_info->rx_mimo_signal_quality[ODM_RF_PATH_A] = SQ;
		}

		for (i = ODM_RF_PATH_A; i < ODM_RF_PATH_MAX_JAGUAR; i++) {
			if (i == 0)
				p_phy_info->rx_mimo_signal_strength[0] = PWDB_ALL;
			else
				p_phy_info->rx_mimo_signal_strength[i] = 0;
		}
	} else {
		/*is OFDM rate*/
		p_dm_fat_table->hw_antsw_occur = p_phy_sta_rpt->hw_antsw_occur;

		p_dm_odm->phy_dbg_info.num_qry_phy_status_ofdm++;

		/*(1)Get RSSI for OFDM rate*/

		for (i = ODM_RF_PATH_A; i < ODM_RF_PATH_MAX_JAGUAR; i++) {
			/*2008/01/30 MH we will judge RF RX path now.*/
			/* dbg_print("p_dm_odm->rf_path_rx_enable = %x\n", p_dm_odm->rf_path_rx_enable); */
			if (p_dm_odm->rf_path_rx_enable & BIT(i))
				rf_rx_num++;
			/* else */
			/* continue; */
			/*2012.05.25 LukeLee: Testchip AGC report is wrong, it should be restored back to old formula in MP chip*/
			/* if((p_dm_odm->support_ic_type & (ODM_RTL8812|ODM_RTL8821)) && (!p_dm_odm->is_mp_chip)) */
			if (i < ODM_RF_PATH_C)
				rx_pwr[i] = (p_phy_sta_rpt->gain_trsw[i] & 0x7F) - 110;
			else
				rx_pwr[i] = (p_phy_sta_rpt->gain_trsw_cd[i - 2] & 0x7F) - 110;
			/* else */
			/*rx_pwr[i] = ((p_phy_sta_rpt->gain_trsw[i]& 0x3F)*2) - 110;  OLD FORMULA*/

			p_phy_info->rx_pwr[i] = rx_pwr[i];

			/* Translate DBM to percentage. */
			RSSI = odm_query_rx_pwr_percentage(rx_pwr[i]);

			/*total_rssi += RSSI;*/
			/*Get the best two RSSI*/
			if (RSSI > best_rssi && RSSI > second_rssi) {
				second_rssi = best_rssi;
				best_rssi = RSSI;
			} else if (RSSI > second_rssi && RSSI <= best_rssi)
				second_rssi = RSSI;

			/*RT_DISP(FRX, RX_PHY_SS, ("RF-%d RXPWR=%x RSSI=%d\n", i, rx_pwr[i], RSSI));*/

			p_phy_info->rx_mimo_signal_strength[i] = (u8) RSSI;

			/*Get Rx snr value in DB*/
			if (i < ODM_RF_PATH_C)
				p_phy_info->rx_snr[i] = p_dm_odm->phy_dbg_info.rx_snr_db[i] = p_phy_sta_rpt->rxsnr[i] / 2;
			else if (p_dm_odm->support_ic_type & (ODM_RTL8814A | ODM_RTL8822B))
				p_phy_info->rx_snr[i] = p_dm_odm->phy_dbg_info.rx_snr_db[i] = p_phy_sta_rpt->csi_current[i - 2] / 2;

			/*(2) CFO_short  & CFO_tail*/
			if (i < ODM_RF_PATH_C) {
				p_phy_info->cfo_short[i] = odm_cfo((p_phy_sta_rpt->cfosho[i]));
				p_phy_info->cfo_tail[i] = odm_cfo((p_phy_sta_rpt->cfotail[i]));
			}
		}

		/*(3)PWDB, Average PWDB calculated by hardware (for rate adaptive)*/

		/*2012.05.25 LukeLee: Testchip AGC report is wrong, it should be restored back to old formula in MP chip*/
		if ((p_dm_odm->support_ic_type & (ODM_RTL8812 | ODM_RTL8821 | ODM_RTL8881A)) && (!p_dm_odm->is_mp_chip))
			rx_pwr_all = (p_phy_sta_rpt->pwdb_all & 0x7f) - 110;
		else
			rx_pwr_all = (((p_phy_sta_rpt->pwdb_all) >> 1) & 0x7f) - 110;	 /*OLD FORMULA*/

		PWDB_ALL_BT = PWDB_ALL = odm_query_rx_pwr_percentage(rx_pwr_all);

		p_phy_info->rx_pwdb_all = PWDB_ALL;
		/*ODM_RT_TRACE(p_dm_odm,ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("ODM OFDM RSSI=%d\n",p_phy_info->rx_pwdb_all));*/
		p_phy_info->bt_rx_rssi_percentage = PWDB_ALL_BT;
		p_phy_info->rx_power = rx_pwr_all;
		p_phy_info->recv_signal_power = rx_pwr_all;

		if ((p_dm_odm->support_platform == ODM_WIN) && (p_dm_odm->patch_id == 19)) {
			/*do nothing*/
		} else {
			/*p_mgnt_info->customer_id != RT_CID_819X_LENOVO*/

			/*(4)EVM of OFDM rate*/

			if ((p_pktinfo->data_rate >= ODM_RATEMCS8) &&
			    (p_pktinfo->data_rate <= ODM_RATEMCS15))
				max_spatial_stream = 2;
			else if ((p_pktinfo->data_rate >= ODM_RATEVHTSS2MCS0) &&
				 (p_pktinfo->data_rate <= ODM_RATEVHTSS2MCS9))
				max_spatial_stream = 2;
			else if ((p_pktinfo->data_rate >= ODM_RATEMCS16) &&
				 (p_pktinfo->data_rate <= ODM_RATEMCS23))
				max_spatial_stream = 3;
			else if ((p_pktinfo->data_rate >= ODM_RATEVHTSS3MCS0) &&
				 (p_pktinfo->data_rate <= ODM_RATEVHTSS3MCS9))
				max_spatial_stream = 3;
			else
				max_spatial_stream = 1;

			/*if (p_pktinfo->is_packet_match_bssid) */
			{
				/*dbg_print("p_pktinfo->data_rate = %d\n", p_pktinfo->data_rate);*/

				for (i = 0; i < max_spatial_stream; i++) {
					/*Do not use shift operation like "rx_evmX >>= 1" because the compilor of free build environment*/
					/*fill most significant bit to "zero" when doing shifting operation which may change a negative*/
					/*value to positive one, then the dbm value (which is supposed to be negative)  is not correct anymore.*/

					if (p_pktinfo->data_rate >= ODM_RATE6M && p_pktinfo->data_rate <= ODM_RATE54M) {
						if (i == ODM_RF_PATH_A) {
							EVM = odm_evm_db_to_percentage((p_phy_sta_rpt->sigevm));	/*dbm*/
							EVM += 20;
							if (EVM > 100)
								EVM = 100;
						}
					} else {
						if (i < ODM_RF_PATH_C) {
							if (p_phy_sta_rpt->rxevm[i] == -128)
								p_phy_sta_rpt->rxevm[i] = -25;
							EVM = odm_evm_db_to_percentage((p_phy_sta_rpt->rxevm[i]));	/*dbm*/
						} else {
							if (p_phy_sta_rpt->rxevm_cd[i - 2] == -128)
								p_phy_sta_rpt->rxevm_cd[i - 2] = -25;
							EVM = odm_evm_db_to_percentage((p_phy_sta_rpt->rxevm_cd[i - 2]));	/*dbm*/
						}
					}

					if (i < ODM_RF_PATH_C)
						evm_dbm = odm_evm_dbm_jaguar_series(p_phy_sta_rpt->rxevm[i]);
					else
						evm_dbm = odm_evm_dbm_jaguar_series(p_phy_sta_rpt->rxevm_cd[i - 2]);
					/*RT_DISP(FRX, RX_PHY_SQ, ("RXRATE=%x RXEVM=%x EVM=%s%d\n",*/
					/*p_pktinfo->data_rate, p_phy_sta_rpt->rxevm[i], "%", EVM));*/

					{
						if (i == ODM_RF_PATH_A) {
							/*Fill value in RFD, Get the first spatial stream only*/
							p_phy_info->signal_quality = EVM;
						}
						p_phy_info->rx_mimo_signal_quality[i] = EVM;
						p_phy_info->rx_mimo_evm_dbm[i] = evm_dbm;
					}
				}
			}
		}

		num_ss = phydm_rate_to_num_ss(p_dm_odm, p_pktinfo->data_rate);
		odm_parsing_cfo(p_dm_odm, p_pktinfo, p_phy_sta_rpt->cfotail, num_ss);

	}
	/* dbg_print("is_cck_rate= %d, p_phy_info->signal_strength=%d % PWDB_AL=%d rf_rx_num=%d\n", is_cck_rate, p_phy_info->signal_strength, PWDB_ALL, rf_rx_num); */

	/*UI BSS List signal strength(in percentage), make it good looking, from 0~100.*/
	/*It is assigned to the BSS List in GetValueFromBeaconOrProbeRsp().*/
	if (is_cck_rate) {
		p_phy_info->signal_strength = (u8)(odm_signal_scale_mapping(p_dm_odm, PWDB_ALL));/*PWDB_ALL;*/
	} else {
		if (rf_rx_num != 0) {
			/* 2015/01 Sean, use the best two RSSI only, suggested by Ynlin and ChenYu.*/
			if (rf_rx_num == 1)
				avg_rssi = best_rssi;
			else
				avg_rssi = (best_rssi + second_rssi) / 2;
			p_phy_info->signal_strength = (u8)(odm_signal_scale_mapping(p_dm_odm, avg_rssi));
		}
	}
	p_dm_odm->rx_pwdb_ave = p_dm_odm->rx_pwdb_ave + p_phy_info->rx_pwdb_all;

	p_dm_odm->dm_fat_table.antsel_rx_keep_0 = p_phy_sta_rpt->antidx_anta;
	p_dm_odm->dm_fat_table.antsel_rx_keep_1 = p_phy_sta_rpt->antidx_antb;
	p_dm_odm->dm_fat_table.antsel_rx_keep_2 = p_phy_sta_rpt->antidx_antc;
	p_dm_odm->dm_fat_table.antsel_rx_keep_3 = p_phy_sta_rpt->antidx_antd;
	/*ODM_RT_TRACE(p_dm_odm, ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("StaID[%d]:  antidx_anta = ((%d)), MatchBSSID =  ((%d))\n", p_pktinfo->station_id, p_phy_sta_rpt->antidx_anta, p_pktinfo->is_packet_match_bssid));*/

	/*		dbg_print("p_phy_sta_rpt->antidx_anta = %d, p_phy_sta_rpt->antidx_antb = %d\n",*/
	/*			p_phy_sta_rpt->antidx_anta, p_phy_sta_rpt->antidx_antb);*/
	/*		dbg_print("----------------------------\n");*/
	/*		dbg_print("p_pktinfo->station_id=%d, p_pktinfo->data_rate=0x%x\n",p_pktinfo->station_id, p_pktinfo->data_rate);*/
	/*		dbg_print("p_phy_sta_rpt->r_RFMOD = %d\n", p_phy_sta_rpt->r_RFMOD);*/
	/*		dbg_print("p_phy_sta_rpt->gain_trsw[0]=0x%x, p_phy_sta_rpt->gain_trsw[1]=0x%x\n",*/
	/*				p_phy_sta_rpt->gain_trsw[0],p_phy_sta_rpt->gain_trsw[1]);*/
	/*		dbg_print("p_phy_sta_rpt->gain_trsw[2]=0x%x, p_phy_sta_rpt->gain_trsw[3]=0x%x\n",*/
	/*				p_phy_sta_rpt->gain_trsw_cd[0],p_phy_sta_rpt->gain_trsw_cd[1]);*/
	/*		dbg_print("p_phy_sta_rpt->pwdb_all = 0x%x, p_phy_info->rx_pwdb_all = %d\n", p_phy_sta_rpt->pwdb_all, p_phy_info->rx_pwdb_all);*/
	/*		dbg_print("p_phy_sta_rpt->cfotail[i] = 0x%x, p_phy_sta_rpt->CFO_tail[i] = 0x%x\n", p_phy_sta_rpt->cfotail[0], p_phy_sta_rpt->cfotail[1]);*/
	/*		dbg_print("p_phy_sta_rpt->rxevm[0] = %d, p_phy_sta_rpt->rxevm[1] = %d\n", p_phy_sta_rpt->rxevm[0], p_phy_sta_rpt->rxevm[1]);*/
	/*		dbg_print("p_phy_sta_rpt->rxevm[2] = %d, p_phy_sta_rpt->rxevm[3] = %d\n", p_phy_sta_rpt->rxevm_cd[0], p_phy_sta_rpt->rxevm_cd[1]);*/
	/*		dbg_print("p_phy_info->rx_mimo_signal_strength[0]=%d, p_phy_info->rx_mimo_signal_strength[1]=%d, rx_pwdb_all=%d\n",*/
	/*				p_phy_info->rx_mimo_signal_strength[0], p_phy_info->rx_mimo_signal_strength[1], p_phy_info->rx_pwdb_all);*/
	/*		dbg_print("p_phy_info->rx_mimo_signal_strength[2]=%d, p_phy_info->rx_mimo_signal_strength[3]=%d\n",*/
	/*				p_phy_info->rx_mimo_signal_strength[2], p_phy_info->rx_mimo_signal_strength[3]);*/
	/*		dbg_print("ppPhyInfo->rx_mimo_signal_quality[0]=%d, p_phy_info->rx_mimo_signal_quality[1]=%d\n",*/
	/*				p_phy_info->rx_mimo_signal_quality[0], p_phy_info->rx_mimo_signal_quality[1]);*/
	/*		dbg_print("ppPhyInfo->rx_mimo_signal_quality[2]=%d, p_phy_info->rx_mimo_signal_quality[3]=%d\n",*/
	/*				p_phy_info->rx_mimo_signal_quality[2], p_phy_info->rx_mimo_signal_quality[3]);*/

}

#endif

void
phydm_reset_rssi_for_dm(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u8		station_id
)
{
	struct sta_info			*p_entry;

	p_entry = p_dm_odm->p_odm_sta_info[station_id];

	if (!IS_STA_VALID(p_entry)) {
		/**/
		return;
	}
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("Reset RSSI for macid = (( %d ))\n", station_id));

	p_entry->rssi_stat.undecorated_smoothed_cck = -1;
	p_entry->rssi_stat.undecorated_smoothed_ofdm = -1;
	p_entry->rssi_stat.undecorated_smoothed_pwdb = -1;
	p_entry->rssi_stat.ofdm_pkt = 0;
	p_entry->rssi_stat.cck_pkt = 0;
	p_entry->rssi_stat.cck_sum_power = 0;
	p_entry->rssi_stat.is_send_rssi = RA_RSSI_STATE_INIT;
	p_entry->rssi_stat.packet_map = 0;
	p_entry->rssi_stat.valid_bit = 0;
}

void
odm_init_rssi_for_dm(
	struct PHY_DM_STRUCT	*p_dm_odm
)
{

}

static void
odm_process_rssi_for_dm(
	struct PHY_DM_STRUCT					*p_dm_odm,
	struct _odm_phy_status_info_			*p_phy_info,
	struct _odm_per_pkt_info_			*p_pktinfo
)
{

	s32			undecorated_smoothed_pwdb, undecorated_smoothed_cck, undecorated_smoothed_ofdm, rssi_ave, cck_pkt;
	u8			i, is_cck_rate = 0;
	u8			RSSI_max, RSSI_min;
	u32			weighting = 0;
	u8			send_rssi_2_fw = 0;
	struct sta_info			*p_entry;

	if (p_pktinfo->station_id >= ODM_ASSOCIATE_ENTRY_NUM)
		return;

#ifdef CONFIG_S0S1_SW_ANTENNA_DIVERSITY
	odm_s0s1_sw_ant_div_by_ctrl_frame_process_rssi(p_dm_odm, p_phy_info, p_pktinfo);
#endif

	/*  */
	/* 2012/05/30 MH/Luke.Lee Add some description */
	/* In windows driver: AP/IBSS mode STA */
	/*  */
	/* if (p_dm_odm->support_platform == ODM_WIN) */
	/* { */
	/*	p_entry = p_dm_odm->p_odm_sta_info[p_dm_odm->pAidMap[p_pktinfo->station_id-1]]; */
	/* } */
	/* else */
	p_entry = p_dm_odm->p_odm_sta_info[p_pktinfo->station_id];

	if (!IS_STA_VALID(p_entry)) {
		return;
		/**/
	}

	if ((!p_pktinfo->is_packet_match_bssid))/*data frame only*/
		return;

	if (p_pktinfo->is_packet_beacon)
		p_dm_odm->phy_dbg_info.num_qry_beacon_pkt++;

	is_cck_rate = (p_pktinfo->data_rate <= ODM_RATE11M) ? true : false;
	p_dm_odm->rx_rate = p_pktinfo->data_rate;

	/* --------------Statistic for antenna/path diversity------------------ */
	if (p_dm_odm->support_ability & ODM_BB_ANT_DIV) {
#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
		odm_process_rssi_for_ant_div(p_dm_odm, p_phy_info, p_pktinfo);
#endif
	}
#if (defined(CONFIG_PATH_DIVERSITY))
	else if (p_dm_odm->support_ability & ODM_BB_PATH_DIV)
		phydm_process_rssi_for_path_div(p_dm_odm, p_phy_info, p_pktinfo);
#endif
	/* -----------------Smart Antenna Debug Message------------------ */

	undecorated_smoothed_cck =  p_entry->rssi_stat.undecorated_smoothed_cck;
	undecorated_smoothed_ofdm = p_entry->rssi_stat.undecorated_smoothed_ofdm;
	undecorated_smoothed_pwdb = p_entry->rssi_stat.undecorated_smoothed_pwdb;

	if (p_pktinfo->is_packet_to_self || p_pktinfo->is_packet_beacon) {

		if (!is_cck_rate) { /* ofdm rate */
			{
				if (p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_B] == 0) {
					rssi_ave = p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_A];
					p_dm_odm->RSSI_A = p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_A];
					p_dm_odm->RSSI_B = 0;
				} else {
					/*dbg_print("p_rfd->status.rx_mimo_signal_strength[0] = %d, p_rfd->status.rx_mimo_signal_strength[1] = %d\n",*/
					/*p_rfd->status.rx_mimo_signal_strength[0], p_rfd->status.rx_mimo_signal_strength[1]);*/
					p_dm_odm->RSSI_A =  p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_A];
					p_dm_odm->RSSI_B = p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_B];

					if (p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_A] > p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_B]) {
						RSSI_max = p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_A];
						RSSI_min = p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_B];
					} else {
						RSSI_max = p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_B];
						RSSI_min = p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_A];
					}
					if ((RSSI_max - RSSI_min) < 3)
						rssi_ave = RSSI_max;
					else if ((RSSI_max - RSSI_min) < 6)
						rssi_ave = RSSI_max - 1;
					else if ((RSSI_max - RSSI_min) < 10)
						rssi_ave = RSSI_max - 2;
					else
						rssi_ave = RSSI_max - 3;
				}
			}

			/* 1 Process OFDM RSSI */
			if (undecorated_smoothed_ofdm <= 0) {	/* initialize */
				undecorated_smoothed_ofdm = p_phy_info->rx_pwdb_all;
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("OFDM_INIT: (( %d ))\n", undecorated_smoothed_ofdm));
			} else {
				if (p_phy_info->rx_pwdb_all > (u32)undecorated_smoothed_ofdm) {
					undecorated_smoothed_ofdm =
						(((undecorated_smoothed_ofdm)*(RX_SMOOTH_FACTOR - 1)) +
						(rssi_ave)) / (RX_SMOOTH_FACTOR);
					undecorated_smoothed_ofdm = undecorated_smoothed_ofdm + 1;
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("OFDM_1: (( %d ))\n", undecorated_smoothed_ofdm));
				} else {
					undecorated_smoothed_ofdm =
						(((undecorated_smoothed_ofdm)*(RX_SMOOTH_FACTOR - 1)) +
						(rssi_ave)) / (RX_SMOOTH_FACTOR);
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("OFDM_2: (( %d ))\n", undecorated_smoothed_ofdm));
				}
			}
			if (p_entry->rssi_stat.ofdm_pkt != 64) {
				i = 63;
				p_entry->rssi_stat.ofdm_pkt -= (u8)(((p_entry->rssi_stat.packet_map >> i) & BIT(0)) - 1);
			}
			p_entry->rssi_stat.packet_map = (p_entry->rssi_stat.packet_map << 1) | BIT(0);

		} else {
			rssi_ave = p_phy_info->rx_pwdb_all;
			p_dm_odm->RSSI_A = (u8) p_phy_info->rx_pwdb_all;
			p_dm_odm->RSSI_B = 0xFF;
			p_dm_odm->RSSI_C = 0xFF;
			p_dm_odm->RSSI_D = 0xFF;

			if (p_entry->rssi_stat.cck_pkt <= 63)
				p_entry->rssi_stat.cck_pkt++;

			/* 1 Process CCK RSSI */
			if (undecorated_smoothed_cck <= 0) {	/* initialize */
				undecorated_smoothed_cck = p_phy_info->rx_pwdb_all;
				p_entry->rssi_stat.cck_sum_power = (u16)p_phy_info->rx_pwdb_all ; /*reset*/
				p_entry->rssi_stat.cck_pkt = 1; /*reset*/
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("CCK_INIT: (( %d ))\n", undecorated_smoothed_cck));
			} else if (p_entry->rssi_stat.cck_pkt <= CCK_RSSI_INIT_COUNT) {

				p_entry->rssi_stat.cck_sum_power = p_entry->rssi_stat.cck_sum_power + (u16)p_phy_info->rx_pwdb_all;
				undecorated_smoothed_cck = p_entry->rssi_stat.cck_sum_power / p_entry->rssi_stat.cck_pkt;

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("CCK_0: (( %d )), SumPow = (( %d )), cck_pkt = (( %d ))\n",
					undecorated_smoothed_cck, p_entry->rssi_stat.cck_sum_power, p_entry->rssi_stat.cck_pkt));
			} else {
				if (p_phy_info->rx_pwdb_all > (u32)undecorated_smoothed_cck) {
					undecorated_smoothed_cck =
						(((undecorated_smoothed_cck)*(RX_SMOOTH_FACTOR - 1)) +
						(p_phy_info->rx_pwdb_all)) / (RX_SMOOTH_FACTOR);
					undecorated_smoothed_cck = undecorated_smoothed_cck + 1;
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("CCK_1: (( %d ))\n", undecorated_smoothed_cck));
				} else {
					undecorated_smoothed_cck =
						(((undecorated_smoothed_cck)*(RX_SMOOTH_FACTOR - 1)) +
						(p_phy_info->rx_pwdb_all)) / (RX_SMOOTH_FACTOR);
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("CCK_2: (( %d ))\n", undecorated_smoothed_cck));
				}
			}
			i = 63;
			p_entry->rssi_stat.ofdm_pkt -= (u8)((p_entry->rssi_stat.packet_map >> i) & BIT(0));
			p_entry->rssi_stat.packet_map = p_entry->rssi_stat.packet_map << 1;
		}

		/* if(p_entry) */
		{
			/* 2011.07.28 LukeLee: modified to prevent unstable CCK RSSI */
			if (p_entry->rssi_stat.ofdm_pkt == 64) { /* speed up when all packets are OFDM*/
				undecorated_smoothed_pwdb = undecorated_smoothed_ofdm;
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("PWDB_0[%d] = (( %d ))\n", p_pktinfo->station_id, undecorated_smoothed_cck));
			} else {
				if (p_entry->rssi_stat.valid_bit < 64)
					p_entry->rssi_stat.valid_bit++;

				if (p_entry->rssi_stat.valid_bit == 64) {
					weighting = ((p_entry->rssi_stat.ofdm_pkt) > 4) ? 64 : (p_entry->rssi_stat.ofdm_pkt << 4);
					undecorated_smoothed_pwdb = (weighting * undecorated_smoothed_ofdm + (64 - weighting) * undecorated_smoothed_cck) >> 6;
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("PWDB_1[%d] = (( %d )), W = (( %d ))\n", p_pktinfo->station_id, undecorated_smoothed_cck, weighting));
				} else {
					if (p_entry->rssi_stat.valid_bit != 0)
						undecorated_smoothed_pwdb = (p_entry->rssi_stat.ofdm_pkt * undecorated_smoothed_ofdm + (p_entry->rssi_stat.valid_bit - p_entry->rssi_stat.ofdm_pkt) * undecorated_smoothed_cck) / p_entry->rssi_stat.valid_bit;
					else
						undecorated_smoothed_pwdb = 0;

					ODM_RT_TRACE(p_dm_odm, ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("PWDB_2[%d] = (( %d )), ofdm_pkt = (( %d )), Valid_Bit = (( %d ))\n", p_pktinfo->station_id, undecorated_smoothed_cck, p_entry->rssi_stat.ofdm_pkt, p_entry->rssi_stat.valid_bit));
				}
			}

			if ((p_entry->rssi_stat.ofdm_pkt >= 1 || p_entry->rssi_stat.cck_pkt >= 5) && (p_entry->rssi_stat.is_send_rssi == RA_RSSI_STATE_INIT)) {

				send_rssi_2_fw = 1;
				p_entry->rssi_stat.is_send_rssi = RA_RSSI_STATE_SEND;
			}

			p_entry->rssi_stat.undecorated_smoothed_cck = undecorated_smoothed_cck;
			p_entry->rssi_stat.undecorated_smoothed_ofdm = undecorated_smoothed_ofdm;
			p_entry->rssi_stat.undecorated_smoothed_pwdb = undecorated_smoothed_pwdb;

			if (send_rssi_2_fw) { /* Trigger init rate by RSSI */

				if (p_entry->rssi_stat.ofdm_pkt != 0)
					p_entry->rssi_stat.undecorated_smoothed_pwdb = undecorated_smoothed_ofdm;

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_RSSI_MONITOR, ODM_DBG_LOUD, ("[Send to FW] PWDB = (( %d )), ofdm_pkt = (( %d )), cck_pkt = (( %d ))\n",
					undecorated_smoothed_pwdb, p_entry->rssi_stat.ofdm_pkt, p_entry->rssi_stat.cck_pkt));

				phydm_ra_rssi_rpt_wk(p_dm_odm);
			}
		}

	}
}

#if (ODM_IC_11N_SERIES_SUPPORT == 1)
/*
 * Endianness before calling this API
 *   */
static void
odm_phy_status_query_92c_series(
	struct PHY_DM_STRUCT					*p_dm_odm,
	struct _odm_phy_status_info_				*p_phy_info,
	u8						*p_phy_status,
	struct _odm_per_pkt_info_			*p_pktinfo
)
{
	odm_rx_phy_status92c_series_parsing(p_dm_odm, p_phy_info, p_phy_status, p_pktinfo);
	odm_process_rssi_for_dm(p_dm_odm, p_phy_info, p_pktinfo);
}
#endif

/*
 * Endianness before calling this API
 *   */
#if	ODM_IC_11AC_SERIES_SUPPORT

void
odm_phy_status_query_jaguar_series(
	struct PHY_DM_STRUCT					*p_dm_odm,
	struct _odm_phy_status_info_			*p_phy_info,
	u8						*p_phy_status,
	struct _odm_per_pkt_info_			*p_pktinfo
)
{
	odm_rx_phy_status_jaguar_series_parsing(p_dm_odm, p_phy_info,	p_phy_status, p_pktinfo);
	odm_process_rssi_for_dm(p_dm_odm, p_phy_info, p_pktinfo);

}
#endif

void
odm_phy_status_query(
	struct PHY_DM_STRUCT					*p_dm_odm,
	struct _odm_phy_status_info_			*p_phy_info,
	u8						*p_phy_status,
	struct _odm_per_pkt_info_			*p_pktinfo
)
{
#if (ODM_PHY_STATUS_NEW_TYPE_SUPPORT == 1)
	if (p_dm_odm->support_ic_type & ODM_IC_PHY_STATUE_NEW_TYPE) {
		phydm_rx_phy_status_new_type(p_dm_odm, p_phy_status, p_pktinfo, p_phy_info);
		return;
	}
#endif

#if	ODM_IC_11AC_SERIES_SUPPORT
	if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES)
		odm_phy_status_query_jaguar_series(p_dm_odm, p_phy_info, p_phy_status, p_pktinfo);
#endif

#if	ODM_IC_11N_SERIES_SUPPORT
	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES)
		odm_phy_status_query_92c_series(p_dm_odm, p_phy_info, p_phy_status, p_pktinfo);
#endif
}

/* For future use. */
void
odm_mac_status_query(
	struct PHY_DM_STRUCT					*p_dm_odm,
	u8						*p_mac_status,
	u8						mac_id,
	bool						is_packet_match_bssid,
	bool						is_packet_to_self,
	bool						is_packet_beacon
)
{
	/* 2011/10/19 Driver team will handle in the future. */

}

/*
 * If you want to add a new IC, Please follow below template and generate a new one.
 *
 *   */

enum hal_status
odm_config_rf_with_header_file(
	struct PHY_DM_STRUCT		*p_dm_odm,
	enum odm_rf_config_type		config_type,
	enum odm_rf_radio_path_e	e_rf_path
)
{
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("===>odm_config_rf_with_header_file (%s)\n", (p_dm_odm->is_mp_chip) ? "MPChip" : "TestChip"));
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("p_dm_odm->support_platform: 0x%X, p_dm_odm->support_interface: 0x%X, p_dm_odm->board_type: 0x%X\n",
		p_dm_odm->support_platform, p_dm_odm->support_interface, p_dm_odm->board_type));

	/* 1 AP doesn't use PHYDM power tracking table in these ICs */

	/* 1 All platforms support */
	if (p_dm_odm->support_ic_type == ODM_RTL8188E) {
		if (config_type == CONFIG_RF_RADIO) {
			if (e_rf_path == ODM_RF_PATH_A)
				READ_AND_CONFIG_MP(8188e, _radioa);
		} else if (config_type == CONFIG_RF_TXPWR_LMT)
			READ_AND_CONFIG_MP(8188e, _txpwr_lmt);
	}
	return HAL_STATUS_SUCCESS;
}

enum hal_status
odm_config_rf_with_tx_pwr_track_header_file(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("===>odm_config_rf_with_tx_pwr_track_header_file (%s)\n", (p_dm_odm->is_mp_chip) ? "MPChip" : "TestChip"));
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("p_dm_odm->support_platform: 0x%X, p_dm_odm->support_interface: 0x%X, p_dm_odm->board_type: 0x%X\n",
		p_dm_odm->support_platform, p_dm_odm->support_interface, p_dm_odm->board_type));

	/* 1 AP doesn't use PHYDM power tracking table in these ICs */
	if (p_dm_odm->support_ic_type == ODM_RTL8188E) {
		if (odm_get_mac_reg(p_dm_odm, 0xF0, 0xF000) >= 8) {		/*if 0xF0[15:12] >= 8, SMIC*/
			if (p_dm_odm->support_interface == ODM_ITRF_PCIE)
				READ_AND_CONFIG_MP(8188e, _txpowertrack_pcie_icut);
			else if (p_dm_odm->support_interface == ODM_ITRF_USB)
				READ_AND_CONFIG_MP(8188e, _txpowertrack_usb_icut);
			else if (p_dm_odm->support_interface == ODM_ITRF_SDIO)
				READ_AND_CONFIG_MP(8188e, _txpowertrack_sdio_icut);
		} else {	/*else 0xF0[15:12] < 8, TSMC*/
			if (p_dm_odm->support_interface == ODM_ITRF_PCIE)
				READ_AND_CONFIG_MP(8188e, _txpowertrack_pcie);
			else if (p_dm_odm->support_interface == ODM_ITRF_USB)
				READ_AND_CONFIG_MP(8188e, _txpowertrack_usb);
			else if (p_dm_odm->support_interface == ODM_ITRF_SDIO)
				READ_AND_CONFIG_MP(8188e, _txpowertrack_sdio);
		}

	}

	/* 1 All platforms support */
	return HAL_STATUS_SUCCESS;
}

enum hal_status
odm_config_bb_with_header_file(
	struct PHY_DM_STRUCT		*p_dm_odm,
	enum odm_bb_config_type		config_type
)
{
	/* 1 All platforms support */
	if (p_dm_odm->support_ic_type == ODM_RTL8188E) {
		if (config_type == CONFIG_BB_PHY_REG)
			READ_AND_CONFIG_MP(8188e, _phy_reg);
		else if (config_type == CONFIG_BB_AGC_TAB)
			READ_AND_CONFIG_MP(8188e, _agc_tab);
		else if (config_type == CONFIG_BB_PHY_REG_PG)
			READ_AND_CONFIG_MP(8188e, _phy_reg_pg);
	}
	return HAL_STATUS_SUCCESS;
}

enum hal_status
odm_config_mac_with_header_file(
	struct PHY_DM_STRUCT	*p_dm_odm
)
{
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("===>odm_config_mac_with_header_file (%s)\n", (p_dm_odm->is_mp_chip) ? "MPChip" : "TestChip"));
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_INIT, ODM_DBG_LOUD,
		("p_dm_odm->support_platform: 0x%X, p_dm_odm->support_interface: 0x%X, p_dm_odm->board_type: 0x%X\n",
		p_dm_odm->support_platform, p_dm_odm->support_interface, p_dm_odm->board_type));

	/* 1 All platforms support */
	if (p_dm_odm->support_ic_type == ODM_RTL8188E)
		READ_AND_CONFIG_MP(8188e, _mac_reg);
	return HAL_STATUS_SUCCESS;
}

enum hal_status
odm_config_fw_with_header_file(
	struct PHY_DM_STRUCT			*p_dm_odm,
	enum odm_fw_config_type	config_type,
	u8				*p_firmware,
	u32				*p_size
)
{
	return HAL_STATUS_SUCCESS;
}

u32
odm_get_hw_img_version(
	struct PHY_DM_STRUCT	*p_dm_odm
)
{
	u32  version = 0;

	/*1 All platforms support*/
	if (p_dm_odm->support_ic_type == ODM_RTL8188E)
		version = GET_VERSION_MP(8188e, _mac_reg);
	return version;
}

#if (ODM_PHY_STATUS_NEW_TYPE_SUPPORT == 1)
/* For 8822B only!! need to move to FW finally */
/*==============================================*/

bool
phydm_query_is_mu_api(
	struct PHY_DM_STRUCT					*p_phydm,
	u8							ppdu_idx,
	u8							*p_data_rate,
	u8							*p_gid
)
{
	u8	data_rate = 0, gid = 0;
	bool is_mu = FALSE;
	
	data_rate = p_phydm->phy_dbg_info.num_of_ppdu[ppdu_idx];
	gid = p_phydm->phy_dbg_info.gid_num[ppdu_idx];

	if (data_rate & BIT(7)) {
		is_mu = TRUE;
		data_rate = data_rate & ~(BIT(7));
	} else
		is_mu = FALSE;

	*p_data_rate = data_rate;
	*p_gid = gid;

	return is_mu;
	
}

void
phydm_rx_statistic_cal(
	struct PHY_DM_STRUCT				*p_phydm,
	u8									*p_phy_status,
	struct _odm_per_pkt_info_				*p_pktinfo
)
{
	struct _phy_status_rpt_jaguar2_type1	*p_phy_sta_rpt = (struct _phy_status_rpt_jaguar2_type1 *)p_phy_status;
	u8									date_rate = p_pktinfo->data_rate & ~(BIT(7));
	
	if ((p_phy_sta_rpt->gid != 0) && (p_phy_sta_rpt->gid != 63)) {
		if (date_rate >= ODM_RATEVHTSS1MCS0) {
			p_phydm->phy_dbg_info.num_qry_mu_vht_pkt[date_rate - 0x2C]++;
			p_phydm->phy_dbg_info.num_of_ppdu[p_pktinfo->ppdu_cnt] = date_rate | BIT(7);
			p_phydm->phy_dbg_info.gid_num[p_pktinfo->ppdu_cnt] = p_phy_sta_rpt->gid;
		}

	} else {
		if (date_rate >= ODM_RATEVHTSS1MCS0) {
			p_phydm->phy_dbg_info.num_qry_vht_pkt[date_rate - 0x2C]++;
			p_phydm->phy_dbg_info.num_of_ppdu[p_pktinfo->ppdu_cnt] = date_rate;
			p_phydm->phy_dbg_info.gid_num[p_pktinfo->ppdu_cnt] = p_phy_sta_rpt->gid;
		}
	}

}

void
phydm_reset_phy_info(
	struct PHY_DM_STRUCT					*p_phydm,
	struct _odm_phy_status_info_			*p_phy_info
)
{
	p_phy_info->rx_pwdb_all = 0;
	p_phy_info->signal_quality = 0;
	p_phy_info->band_width = 0;
	p_phy_info->rx_count = 0;
	odm_memory_set(p_phydm, p_phy_info->rx_mimo_signal_quality, 0, 4);
	odm_memory_set(p_phydm, p_phy_info->rx_mimo_signal_strength, 0, 4);
	odm_memory_set(p_phydm, p_phy_info->rx_snr, 0, 4);

	p_phy_info->rx_power = -110;
	p_phy_info->recv_signal_power = -110;
	p_phy_info->bt_rx_rssi_percentage = 0;
	p_phy_info->signal_strength = 0;
	p_phy_info->bt_coex_pwr_adjust = 0;
	p_phy_info->channel = 0;
	p_phy_info->is_mu_packet = 0;
	p_phy_info->is_beamformed = 0;
	p_phy_info->rxsc = 0;
	odm_memory_set(p_phydm, p_phy_info->rx_pwr, -110, 4);
	odm_memory_set(p_phydm, p_phy_info->rx_mimo_evm_dbm, 0, 4);
	odm_memory_set(p_phydm, p_phy_info->cfo_short, 0, 8);
	odm_memory_set(p_phydm, p_phy_info->cfo_tail, 0, 8);
}

void
phydm_set_per_path_phy_info(
	u8							rx_path,
	s8							rx_pwr,
	s8							rx_evm,
	s8							cfo_tail,
	s8							rx_snr,
	struct _odm_phy_status_info_				*p_phy_info
)
{
	u8			evm_dbm = 0;
	u8			evm_percentage = 0;

	/* SNR is S(8,1), EVM is S(8,1), CFO is S(8,7) */

	if (rx_evm < 0) {
		/* Calculate EVM in dBm */
		evm_dbm = ((u8)(0 - rx_evm) >> 1);

		/* Calculate EVM in percentage */
		if (evm_dbm >= 33)
			evm_percentage = 100;
		else
			evm_percentage = (evm_dbm << 1) + (evm_dbm);
	}

	p_phy_info->rx_pwr[rx_path] = rx_pwr;
	p_phy_info->rx_mimo_evm_dbm[rx_path] = evm_dbm;

	/* CFO = CFO_tail * 312.5 / 2^7 ~= CFO tail * 39/512 (kHz)*/
	p_phy_info->cfo_tail[rx_path] = cfo_tail;
	p_phy_info->cfo_tail[rx_path] = ((p_phy_info->cfo_tail[rx_path] << 5) + (p_phy_info->cfo_tail[rx_path] << 2) +
		(p_phy_info->cfo_tail[rx_path] << 1) + (p_phy_info->cfo_tail[rx_path])) >> 9;

	p_phy_info->rx_mimo_signal_strength[rx_path] = odm_query_rx_pwr_percentage(rx_pwr);
	p_phy_info->rx_mimo_signal_quality[rx_path] = evm_percentage;
	p_phy_info->rx_snr[rx_path] = rx_snr >> 1;
}

void
phydm_set_common_phy_info(
	s8							rx_power,
	u8							channel,
	bool							is_beamformed,
	bool							is_mu_packet,
	u8							bandwidth,
	u8							signal_quality,
	u8							rxsc,
	struct _odm_phy_status_info_				*p_phy_info
)
{
	p_phy_info->rx_power = rx_power;											/* RSSI in dB */
	p_phy_info->recv_signal_power = rx_power;										/* RSSI in dB */
	p_phy_info->channel = channel;												/* channel number */
	p_phy_info->is_beamformed = is_beamformed;									/* apply BF */
	p_phy_info->is_mu_packet = is_mu_packet;										/* MU packet */
	p_phy_info->rxsc = rxsc;
	p_phy_info->rx_pwdb_all = odm_query_rx_pwr_percentage(rx_power);				/* RSSI in percentage */
	p_phy_info->signal_quality = signal_quality;										/* signal quality */
	p_phy_info->band_width = bandwidth;											/* bandwidth */
}

void
phydm_get_rx_phy_status_type0(
	struct PHY_DM_STRUCT						*p_dm_odm,
	u8							*p_phy_status,
	struct _odm_per_pkt_info_				*p_pktinfo,
	struct _odm_phy_status_info_				*p_phy_info
)
{
	/* type 0 is used for cck packet */

	struct _phy_status_rpt_jaguar2_type0	*p_phy_sta_rpt = (struct _phy_status_rpt_jaguar2_type0 *)p_phy_status;
	u8							i, SQ = 0;
	s8							rx_power = p_phy_sta_rpt->pwdb - 110;

#if (RTL8723D_SUPPORT == 1)
	if (p_dm_odm->support_ic_type & ODM_RTL8723D)
		rx_power = p_phy_sta_rpt->pwdb - 97;
#endif
#if (RTL8197F_SUPPORT == 1)
	if (p_dm_odm->support_ic_type & ODM_RTL8197F)
		rx_power = p_phy_sta_rpt->pwdb - 97;
#endif

	/* Calculate Signal Quality*/
	if (p_pktinfo->is_packet_match_bssid) {
		if (p_phy_sta_rpt->signal_quality >= 64)
			SQ = 0;
		else if (p_phy_sta_rpt->signal_quality <= 20)
			SQ = 100;
		else {
			/* mapping to 2~99% */
			SQ = 64 - p_phy_sta_rpt->signal_quality;
			SQ = ((SQ << 3) + SQ) >> 2;
		}
	}

	/* Modify CCK PWDB if old AGC */
	if (p_dm_odm->cck_new_agc == false) {
		u8	lna_idx, vga_idx;

			lna_idx = ((p_phy_sta_rpt->lna_h << 3) | p_phy_sta_rpt->lna_l);
		vga_idx = p_phy_sta_rpt->vga;
	}

	/* Update CCK packet counter */
	p_dm_odm->phy_dbg_info.num_qry_phy_status_cck++;

	/*CCK no STBC and LDPC*/
	p_dm_odm->phy_dbg_info.is_ldpc_pkt = false;
	p_dm_odm->phy_dbg_info.is_stbc_pkt = false;

	/* Update Common information */
	phydm_set_common_phy_info(rx_power, p_phy_sta_rpt->channel, false,
		  false, ODM_BW20M, SQ, p_phy_sta_rpt->rxsc, p_phy_info);

	/* Update CCK pwdb */
	phydm_set_per_path_phy_info(ODM_RF_PATH_A, rx_power, 0, 0, 0, p_phy_info);					/* Update per-path information */

	p_dm_odm->dm_fat_table.antsel_rx_keep_0 = p_phy_sta_rpt->antidx_a;
	p_dm_odm->dm_fat_table.antsel_rx_keep_1 = p_phy_sta_rpt->antidx_b;
	p_dm_odm->dm_fat_table.antsel_rx_keep_2 = p_phy_sta_rpt->antidx_c;
	p_dm_odm->dm_fat_table.antsel_rx_keep_3 = p_phy_sta_rpt->antidx_d;
}

void
phydm_get_rx_phy_status_type1(
	struct PHY_DM_STRUCT						*p_dm_odm,
	u8							*p_phy_status,
	struct _odm_per_pkt_info_				*p_pktinfo,
	struct _odm_phy_status_info_				*p_phy_info
)
{
	/* type 1 is used for ofdm packet */

	struct _phy_status_rpt_jaguar2_type1	*p_phy_sta_rpt = (struct _phy_status_rpt_jaguar2_type1 *)p_phy_status;
	s8							rx_pwr_db = -120;
	u8							i, rxsc, bw = ODM_BW20M, rx_count = 0;
	bool							is_mu;
	u8							num_ss;

	/* Update OFDM packet counter */
	p_dm_odm->phy_dbg_info.num_qry_phy_status_ofdm++;

	/* Update per-path information */
	for (i = ODM_RF_PATH_A; i < ODM_RF_PATH_MAX_JAGUAR; i++) {
		if (p_dm_odm->rx_ant_status & BIT(i)) {
			s8	rx_path_pwr_db;

			/* RX path counter */
			rx_count++;

			/* Update per-path information (RSSI_dB RSSI_percentage EVM SNR CFO SQ) */
			/* EVM report is reported by stream, not path */
			rx_path_pwr_db = p_phy_sta_rpt->pwdb[i] - 110;					/* per-path pwdb in dB domain */
			phydm_set_per_path_phy_info(i, rx_path_pwr_db, p_phy_sta_rpt->rxevm[rx_count - 1],
				p_phy_sta_rpt->cfo_tail[i], p_phy_sta_rpt->rxsnr[i], p_phy_info);

			/* search maximum pwdb */
			if (rx_path_pwr_db > rx_pwr_db)
				rx_pwr_db = rx_path_pwr_db;
		}
	}

	/* mapping RX counter from 1~4 to 0~3 */
	if (rx_count > 0)
		p_phy_info->rx_count = rx_count - 1;

	/* Check if MU packet or not */
	if ((p_phy_sta_rpt->gid != 0) && (p_phy_sta_rpt->gid != 63)) {
		is_mu = true;
		p_dm_odm->phy_dbg_info.num_qry_mu_pkt++;
	} else
		is_mu = false;

	/* count BF packet */
	p_dm_odm->phy_dbg_info.num_qry_bf_pkt = p_dm_odm->phy_dbg_info.num_qry_bf_pkt + p_phy_sta_rpt->beamformed;

	/*STBC or LDPC pkt*/
	p_dm_odm->phy_dbg_info.is_ldpc_pkt = p_phy_sta_rpt->ldpc;
	p_dm_odm->phy_dbg_info.is_stbc_pkt = p_phy_sta_rpt->stbc;

	/* Check sub-channel */
	if ((p_pktinfo->data_rate > ODM_RATE11M) && (p_pktinfo->data_rate < ODM_RATEMCS0))
		rxsc = p_phy_sta_rpt->l_rxsc;
	else
		rxsc = p_phy_sta_rpt->ht_rxsc;

	/* Check RX bandwidth */
	if (p_dm_odm->support_ic_type & ODM_RTL8822B) {
		if ((rxsc >= 1) && (rxsc <= 8))
			bw = ODM_BW20M;
		else if ((rxsc >= 9) && (rxsc <= 12))
			bw = ODM_BW40M;
		else if (rxsc >= 13)
			bw = ODM_BW80M;
		else
			bw = p_phy_sta_rpt->rf_mode;
	} else if (p_dm_odm->support_ic_type & (ODM_RTL8197F | ODM_RTL8723D)) {
		if (p_phy_sta_rpt->rf_mode == 0)
			bw = ODM_BW20M;
		else if ((rxsc == 1) || (rxsc == 2))
			bw = ODM_BW20M;
		else
			bw = ODM_BW40M;
	}

	/* Update packet information */
	phydm_set_common_phy_info(rx_pwr_db, p_phy_sta_rpt->channel, (bool)p_phy_sta_rpt->beamformed,
		is_mu, bw, odm_evm_db_to_percentage(p_phy_sta_rpt->rxevm[0]), rxsc, p_phy_info);

	num_ss = phydm_rate_to_num_ss(p_dm_odm, p_pktinfo->data_rate);

	odm_parsing_cfo(p_dm_odm, p_pktinfo, p_phy_sta_rpt->cfo_tail, num_ss);
	p_dm_odm->dm_fat_table.antsel_rx_keep_0 = p_phy_sta_rpt->antidx_a;
	p_dm_odm->dm_fat_table.antsel_rx_keep_1 = p_phy_sta_rpt->antidx_b;
	p_dm_odm->dm_fat_table.antsel_rx_keep_2 = p_phy_sta_rpt->antidx_c;
	p_dm_odm->dm_fat_table.antsel_rx_keep_3 = p_phy_sta_rpt->antidx_d;

	if (p_pktinfo->is_packet_match_bssid) {
		/*
				dbg_print("channel = %d, band = %d, l_rxsc = %d, ht_rxsc = %d, rf_mode = %d\n", p_phy_sta_rpt->channel, p_phy_sta_rpt->band, p_phy_sta_rpt->l_rxsc, p_phy_sta_rpt->ht_rxsc, p_phy_sta_rpt->rf_mode);
				dbg_print("Antidx A = %d, B = %d, C = %d, D = %d\n", p_phy_sta_rpt->antidx_a, p_phy_sta_rpt->antidx_b, p_phy_sta_rpt->antidx_c, p_phy_sta_rpt->antidx_d);
				dbg_print("pwdb A: 0x%x, B: 0x%x, C: 0x%x, D: 0x%x\n", p_phy_sta_rpt->pwdb[0], p_phy_sta_rpt->pwdb[1], p_phy_sta_rpt->pwdb[2], p_phy_sta_rpt->pwdb[3]);
				dbg_print("EVM  A: %d, B: %d, C: %d, D: %d\n", p_phy_sta_rpt->rxevm[0], p_phy_sta_rpt->rxevm[1], p_phy_sta_rpt->rxevm[2], p_phy_sta_rpt->rxevm[3]);
				dbg_print("SNR  A: %d, B: %d, C: %d, D: %d\n", p_phy_sta_rpt->rxsnr[0], p_phy_sta_rpt->rxsnr[1], p_phy_sta_rpt->rxsnr[2], p_phy_sta_rpt->rxsnr[3]);
				dbg_print("CFO  A: %d, B: %d, C: %d, D: %d\n", p_phy_sta_rpt->cfo_tail[0], p_phy_sta_rpt->cfo_tail[1], p_phy_sta_rpt->cfo_tail[2], p_phy_sta_rpt->cfo_tail[3]);
				dbg_print("paid = %d, gid = %d, length = %d\n", (p_phy_sta_rpt->paid + (p_phy_sta_rpt->paid_msb<<8)), p_phy_sta_rpt->gid, p_phy_sta_rpt->lsig_length);
				dbg_print("ldpc: %d, stbc: %d, bf: %d, gnt_bt: %d, antsw: %d\n", p_phy_sta_rpt->ldpc, p_phy_sta_rpt->stbc, p_phy_sta_rpt->beamformed, p_phy_sta_rpt->gnt_bt, p_phy_sta_rpt->hw_antsw_occu);
				dbg_print("NBI: %d, pos: %d\n", p_phy_sta_rpt->nb_intf_flag, (p_phy_sta_rpt->intf_pos + (p_phy_sta_rpt->intf_pos_msb<<8)));
				dbg_print("rsvd_0 = %d, rsvd_1 = %d, rsvd_2 = %d, rsvd_3 = %d, rsvd_4 = %d, rsvd_5 = %d\n", p_phy_sta_rpt->rsvd_0, p_phy_sta_rpt->rsvd_1, p_phy_sta_rpt->rsvd_2, p_phy_sta_rpt->rsvd_3, p_phy_sta_rpt->rsvd_4, p_phy_sta_rpt->rsvd_5);
		*/
		phydm_rx_statistic_cal(p_dm_odm, p_phy_status, p_pktinfo);
	}
	/*
		dbg_print("phydm_get_rx_phy_status_type1   p_pktinfo->is_packet_match_bssid = %d\n", p_pktinfo->is_packet_match_bssid);
		dbg_print("p_pktinfo->data_rate = 0x%x\n", p_pktinfo->data_rate);
	*/
}

void
phydm_get_rx_phy_status_type2(
	struct PHY_DM_STRUCT						*p_dm_odm,
	u8							*p_phy_status,
	struct _odm_per_pkt_info_				*p_pktinfo,
	struct _odm_phy_status_info_				*p_phy_info
)
{
	struct _phy_status_rpt_jaguar2_type2	*p_phy_sta_rpt = (struct _phy_status_rpt_jaguar2_type2 *)p_phy_status;
	s8							rx_pwr_db = -120;
	u8							i, rxsc, bw = ODM_BW20M, rx_count = 0;

	/* Update OFDM packet counter */
	p_dm_odm->phy_dbg_info.num_qry_phy_status_ofdm++;

	/* Update per-path information */
	for (i = ODM_RF_PATH_A; i < ODM_RF_PATH_MAX_JAGUAR; i++) {
		if (p_dm_odm->rx_ant_status & BIT(i)) {
			s8	rx_path_pwr_db;

			/* RX path counter */
			rx_count++;

			/* Update per-path information (RSSI_dB RSSI_percentage EVM SNR CFO SQ) */
#if (RTL8197F_SUPPORT == 1)
			if ((p_dm_odm->support_ic_type & ODM_RTL8197F) && (p_phy_sta_rpt->pwdb[i] == 0x7f)) { /*for 97f workaround*/

				if (i == ODM_RF_PATH_A) {
					rx_path_pwr_db = (p_phy_sta_rpt->gain_a) << 1;
					rx_path_pwr_db = rx_path_pwr_db - 110;
				} else if (i == ODM_RF_PATH_B) {
					rx_path_pwr_db = (p_phy_sta_rpt->gain_b) << 1;
					rx_path_pwr_db = rx_path_pwr_db - 110;
				} else
					rx_path_pwr_db = 0;
			} else
#endif
				rx_path_pwr_db = p_phy_sta_rpt->pwdb[i] - 110;					/* per-path pwdb in dB domain */

			phydm_set_per_path_phy_info(i, rx_path_pwr_db, 0, 0, 0, p_phy_info);

			/* search maximum pwdb */
			if (rx_path_pwr_db > rx_pwr_db)
				rx_pwr_db = rx_path_pwr_db;
		}
	}

	/* mapping RX counter from 1~4 to 0~3 */
	if (rx_count > 0)
		p_phy_info->rx_count = rx_count - 1;

	/* Check RX sub-channel */
	if ((p_pktinfo->data_rate > ODM_RATE11M) && (p_pktinfo->data_rate < ODM_RATEMCS0))
		rxsc = p_phy_sta_rpt->l_rxsc;
	else
		rxsc = p_phy_sta_rpt->ht_rxsc;

	/*STBC or LDPC pkt*/
	p_dm_odm->phy_dbg_info.is_ldpc_pkt = p_phy_sta_rpt->ldpc;
	p_dm_odm->phy_dbg_info.is_stbc_pkt = p_phy_sta_rpt->stbc;

	/* Check RX bandwidth */
	/* the BW information of sc=0 is useless, because there is no information of RF mode*/

	if (p_dm_odm->support_ic_type & ODM_RTL8822B) {
		if ((rxsc >= 1) && (rxsc <= 8))
			bw = ODM_BW20M;
		else if ((rxsc >= 9) && (rxsc <= 12))
			bw = ODM_BW40M;
		else if (rxsc >= 13)
			bw = ODM_BW80M;
		else
			bw = ODM_BW20M;
	} else if (p_dm_odm->support_ic_type & (ODM_RTL8197F | ODM_RTL8723D)) {
		if (rxsc == 3)
			bw = ODM_BW40M;
		else if ((rxsc == 1) || (rxsc == 2))
			bw = ODM_BW20M;
		else
			bw = ODM_BW20M;
	}

	/* Update packet information */
	phydm_set_common_phy_info(rx_pwr_db, p_phy_sta_rpt->channel, (bool)p_phy_sta_rpt->beamformed,
				  false, bw, 0, rxsc, p_phy_info);

}

void
phydm_get_rx_phy_status_type5(
	u8				*p_phy_status
)
{
}

void
phydm_process_rssi_for_dm_new_type(
	struct PHY_DM_STRUCT					*p_dm_odm,
	struct _odm_phy_status_info_			*p_phy_info,
	struct _odm_per_pkt_info_			*p_pktinfo
)
{
	s32				undecorated_smoothed_pwdb, accumulate_pwdb;
	u32				rssi_ave;
	u8				i;
	struct sta_info			*p_entry;
	u8				scaling_factor = 4;

	if (p_pktinfo->station_id >= ODM_ASSOCIATE_ENTRY_NUM)
		return;

	p_entry = p_dm_odm->p_odm_sta_info[p_pktinfo->station_id];

	if (!IS_STA_VALID(p_entry))
		return;

	if ((!p_pktinfo->is_packet_match_bssid))/*data frame only*/
		return;

	if (p_pktinfo->is_packet_beacon)
		p_dm_odm->phy_dbg_info.num_qry_beacon_pkt++;

#if (defined(CONFIG_PHYDM_ANTENNA_DIVERSITY))
	if (p_dm_odm->support_ability & ODM_BB_ANT_DIV)
		odm_process_rssi_for_ant_div(p_dm_odm, p_phy_info, p_pktinfo);
#endif

#if (CONFIG_DYNAMIC_RX_PATH == 1)
	phydm_process_phy_status_for_dynamic_rx_path(p_dm_odm, p_phy_info, p_pktinfo);
	dbg_print("====>\n");
#endif

	if (p_pktinfo->is_packet_to_self || p_pktinfo->is_packet_beacon) {
		u32 RSSI_linear = 0;

		p_dm_odm->rx_rate = p_pktinfo->data_rate;
		undecorated_smoothed_pwdb = p_entry->rssi_stat.undecorated_smoothed_pwdb;
		accumulate_pwdb = p_dm_odm->accumulate_pwdb[p_pktinfo->station_id];
		p_dm_odm->RSSI_A = p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_A];
		p_dm_odm->RSSI_B = p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_B];
		p_dm_odm->RSSI_C = p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_C];
		p_dm_odm->RSSI_D = p_phy_info->rx_mimo_signal_strength[ODM_RF_PATH_D];

		for (i = ODM_RF_PATH_A; i < ODM_RF_PATH_MAX_JAGUAR; i++) {
			if (p_phy_info->rx_mimo_signal_strength[i] != 0)
				RSSI_linear += odm_convert_to_linear(p_phy_info->rx_mimo_signal_strength[i]);
		}

		switch (p_phy_info->rx_count + 1) {
		case 2:
			RSSI_linear = (RSSI_linear >> 1);
			break;
		case 3:
			RSSI_linear = ((RSSI_linear) + (RSSI_linear << 1) + (RSSI_linear << 3)) >> 5;	/* RSSI_linear/3 ~ RSSI_linear*11/32 */
			break;
		case 4:
			RSSI_linear = (RSSI_linear >> 2);
			break;
		}
		rssi_ave = odm_convert_to_db(RSSI_linear);

		if (undecorated_smoothed_pwdb <= 0) {
			accumulate_pwdb = (p_phy_info->rx_pwdb_all << scaling_factor);
			undecorated_smoothed_pwdb = p_phy_info->rx_pwdb_all;
		} else {
			accumulate_pwdb = accumulate_pwdb - (accumulate_pwdb >> scaling_factor) + rssi_ave;
			undecorated_smoothed_pwdb = (accumulate_pwdb + (1 << (scaling_factor - 1))) >> scaling_factor;
		}

		if (p_entry->rssi_stat.undecorated_smoothed_pwdb == -1)
			phydm_ra_rssi_rpt_wk(p_dm_odm);
		p_entry->rssi_stat.undecorated_smoothed_pwdb = undecorated_smoothed_pwdb;
		p_dm_odm->accumulate_pwdb[p_pktinfo->station_id] = accumulate_pwdb;
	}
}

void
phydm_rx_phy_status_new_type(
	struct PHY_DM_STRUCT					*p_phydm,
	u8						*p_phy_status,
	struct _odm_per_pkt_info_			*p_pktinfo,
	struct _odm_phy_status_info_			*p_phy_info
)
{
	u8		phy_status_type = (*p_phy_status & 0xf);

	/* Memory reset */
	phydm_reset_phy_info(p_phydm, p_phy_info);

	/* Phy status parsing */
	switch (phy_status_type) {
	case 0:
		phydm_get_rx_phy_status_type0(p_phydm, p_phy_status, p_pktinfo, p_phy_info);
		break;
	case 1:
		phydm_get_rx_phy_status_type1(p_phydm, p_phy_status, p_pktinfo, p_phy_info);
		break;
	case 2:
		phydm_get_rx_phy_status_type2(p_phydm, p_phy_status, p_pktinfo, p_phy_info);
		break;
	default:
		return;
	}

	/* Update signal strength to UI, and p_phy_info->rx_pwdb_all is the maximum RSSI of all path */
	p_phy_info->signal_strength = (u8)(odm_signal_scale_mapping(p_phydm, p_phy_info->rx_pwdb_all));

	/* Calculate average RSSI and smoothed RSSI */
	phydm_process_rssi_for_dm_new_type(p_phydm, p_phy_info, p_pktinfo);

}
/*==============================================*/
#endif

u32
query_phydm_trx_capability(
	struct PHY_DM_STRUCT					*p_dm_odm
)
{
	u32 value32 = 0xFFFFFFFF;

#if (RTL8821C_SUPPORT == 1)
	if (p_dm_odm->support_ic_type == ODM_RTL8821C)
		value32 = query_phydm_trx_capability_8821c(p_dm_odm);
#endif

	return value32;
}

u32
query_phydm_stbc_capability(
	struct PHY_DM_STRUCT					*p_dm_odm
)
{
	u32 value32 = 0xFFFFFFFF;

#if (RTL8821C_SUPPORT == 1)
	if (p_dm_odm->support_ic_type == ODM_RTL8821C)
		value32 = query_phydm_stbc_capability_8821c(p_dm_odm);
#endif

	return value32;
}

u32
query_phydm_ldpc_capability(
	struct PHY_DM_STRUCT					*p_dm_odm
)
{
	u32 value32 = 0xFFFFFFFF;

#if (RTL8821C_SUPPORT == 1)
	if (p_dm_odm->support_ic_type == ODM_RTL8821C)
		value32 = query_phydm_ldpc_capability_8821c(p_dm_odm);
#endif

	return value32;
}

u32
query_phydm_txbf_parameters(
	struct PHY_DM_STRUCT					*p_dm_odm
)
{
	u32 value32 = 0xFFFFFFFF;

#if (RTL8821C_SUPPORT == 1)
	if (p_dm_odm->support_ic_type == ODM_RTL8821C)
		value32 = query_phydm_txbf_parameters_8821c(p_dm_odm);
#endif

	return value32;
}

u32
query_phydm_txbf_capability(
	struct PHY_DM_STRUCT					*p_dm_odm
)
{
	u32 value32 = 0xFFFFFFFF;

#if (RTL8821C_SUPPORT == 1)
	if (p_dm_odm->support_ic_type == ODM_RTL8821C)
		value32 = query_phydm_txbf_capability_8821c(p_dm_odm);
#endif

	return value32;
}
