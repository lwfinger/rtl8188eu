// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


#include "mp_precomp.h"
#include "phydm_precomp.h"

/*---------------------------Define Local Constant---------------------------*/
/* 2010/04/25 MH Define the max tx power tracking tx agc power. */
#define		ODM_TXPWRTRACK_MAX_IDX_88E		6

/*---------------------------Define Local Constant---------------------------*/

/* 3============================================================
 * 3 Tx Power Tracking
 * 3============================================================ */

static void set_iqk_matrix_8188e(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u8		OFDM_index,
	u8		rf_path,
	s32		iqk_result_x,
	s32		iqk_result_y
)
{
	s32			ele_A = 0, ele_D, ele_C = 0, value32;

	ele_D = (ofdm_swing_table_new[OFDM_index] & 0xFFC00000) >> 22;

	/* new element A = element D x X */
	if ((iqk_result_x != 0) && (*(p_dm_odm->p_band_type) == ODM_BAND_2_4G)) {
		if ((iqk_result_x & 0x00000200) != 0)	/* consider minus */
			iqk_result_x = iqk_result_x | 0xFFFFFC00;
		ele_A = ((iqk_result_x * ele_D) >> 8) & 0x000003FF;

		/* new element C = element D x Y */
		if ((iqk_result_y & 0x00000200) != 0)
			iqk_result_y = iqk_result_y | 0xFFFFFC00;
		ele_C = ((iqk_result_y * ele_D) >> 8) & 0x000003FF;

		/* if (rf_path == ODM_RF_PATH_A) */
		switch (rf_path) {
		case ODM_RF_PATH_A:
			/* wirte new elements A, C, D to regC80 and regC94, element B is always 0 */
			value32 = (ele_D << 22) | ((ele_C & 0x3F) << 16) | ele_A;
			odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XA_TX_IQ_IMBALANCE, MASKDWORD, value32);

			value32 = (ele_C & 0x000003C0) >> 6;
			odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XC_TX_AFE, MASKH4BITS, value32);

			value32 = ((iqk_result_x * ele_D) >> 7) & 0x01;
			odm_set_bb_reg(p_dm_odm, REG_OFDM_0_ECCA_THRESHOLD, BIT(24), value32);
			break;
		case ODM_RF_PATH_B:
			/* wirte new elements A, C, D to regC88 and regC9C, element B is always 0 */
			value32 = (ele_D << 22) | ((ele_C & 0x3F) << 16) | ele_A;
			odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XB_TX_IQ_IMBALANCE, MASKDWORD, value32);

			value32 = (ele_C & 0x000003C0) >> 6;
			odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XD_TX_AFE, MASKH4BITS, value32);

			value32 = ((iqk_result_x * ele_D) >> 7) & 0x01;
			odm_set_bb_reg(p_dm_odm, REG_OFDM_0_ECCA_THRESHOLD, BIT(28), value32);

			break;
		default:
			break;
		}
	} else {
		switch (rf_path) {
		case ODM_RF_PATH_A:
			odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XA_TX_IQ_IMBALANCE, MASKDWORD, ofdm_swing_table_new[OFDM_index]);
			odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XC_TX_AFE, MASKH4BITS, 0x00);
			odm_set_bb_reg(p_dm_odm, REG_OFDM_0_ECCA_THRESHOLD, BIT(24), 0x00);
			break;

		case ODM_RF_PATH_B:
			odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XB_TX_IQ_IMBALANCE, MASKDWORD, ofdm_swing_table_new[OFDM_index]);
			odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XD_TX_AFE, MASKH4BITS, 0x00);
			odm_set_bb_reg(p_dm_odm, REG_OFDM_0_ECCA_THRESHOLD, BIT(28), 0x00);
			break;

		default:
			break;
		}
	}

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("TxPwrTracking path B: X = 0x%x, Y = 0x%x ele_A = 0x%x ele_C = 0x%x ele_D = 0x%x 0xeb4 = 0x%x 0xebc = 0x%x\n",
		(u32)iqk_result_x, (u32)iqk_result_y, (u32)ele_A, (u32)ele_C, (u32)ele_D, (u32)iqk_result_x, (u32)iqk_result_y));
}

void do_iqk_8188e(
	void		*p_dm_void,
	u8		delta_thermal_index,
	u8		thermal_value,
	u8		threshold
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ADAPTER		*adapter = p_dm_odm->adapter;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(adapter);

	odm_reset_iqk_result(p_dm_odm);

	p_dm_odm->rf_calibrate_info.thermal_value_iqk = thermal_value;
	phy_iq_calibrate_8188e(adapter, false);
}

/*-----------------------------------------------------------------------------
 * Function:	odm_TxPwrTrackSetPwr88E()
 *
 * Overview:	88E change all channel tx power accordign to flag.
 *				OFDM & CCK are all different.
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	04/23/2012	MHC		Create version 0.
 *
 *---------------------------------------------------------------------------*/
void
odm_tx_pwr_track_set_pwr88_e(
	void		*p_dm_void,
	enum pwrtrack_method	method,
	u8				rf_path,
	u8				channel_mapped_index
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ADAPTER	*adapter = p_dm_odm->adapter;
	PHAL_DATA_TYPE	p_hal_data = GET_HAL_DATA(adapter);
	u8		pwr_tracking_limit_ofdm = 30; /* +0dB */
	u8	    pwr_tracking_limit_cck = 28;  /* -2dB */
	u8		tx_rate = 0xFF;
	u8		final_ofdm_swing_index = 0;
	u8		final_cck_swing_index = 0;
	u8		i = 0;
	struct odm_rf_calibration_structure	*p_rf_calibrate_info = &(p_dm_odm->rf_calibrate_info);

	if (p_dm_odm->mp_mode == true) {
		PMPT_CONTEXT p_mpt_ctx = &(adapter->mppriv.mpt_ctx);

		tx_rate = mpt_to_mgnt_rate(p_mpt_ctx->mpt_rate_index);
	} else {
		u16	rate	 = *(p_dm_odm->p_forced_data_rate);

		if (!rate) { /*auto rate*/
			if (rate != 0xFF) {
				if (p_dm_odm->number_linked_client != 0)
					tx_rate = hw_rate_to_m_rate(p_dm_odm->tx_rate);
			}
		} else   /*force rate*/
			tx_rate = (u8)rate;
	}

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Power Tracking tx_rate=0x%X\n", tx_rate));
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("===>ODM_TxPwrTrackSetPwr8188E\n"));

	if (tx_rate != 0xFF) {
		/* 2 CCK */
		if (((tx_rate >= MGN_1M) && (tx_rate <= MGN_5_5M)) || (tx_rate == MGN_11M))
			pwr_tracking_limit_cck = 28;  /* -2dB */
		/* 2 OFDM */
		else if ((tx_rate >= MGN_6M) && (tx_rate <= MGN_48M))
			pwr_tracking_limit_ofdm = 36; /* +3dB */
		else if (tx_rate == MGN_54M)
			pwr_tracking_limit_ofdm = 34; /* +2dB */

		/* 2 HT */
		else if ((tx_rate >= MGN_MCS0) && (tx_rate <= MGN_MCS2)) /* QPSK/BPSK */
			pwr_tracking_limit_ofdm = 38; /* +4dB */
		else if ((tx_rate >= MGN_MCS3) && (tx_rate <= MGN_MCS4)) /* 16QAM */
			pwr_tracking_limit_ofdm = 36; /* +3dB */
		else if ((tx_rate >= MGN_MCS5) && (tx_rate <= MGN_MCS7)) /* 64QAM */
			pwr_tracking_limit_ofdm = 34; /* +2dB */

		else
			pwr_tracking_limit_ofdm =  p_rf_calibrate_info->default_ofdm_index;   /*Default OFDM index = 30*/
	}
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("tx_rate=0x%x, pwr_tracking_limit=%d\n", tx_rate, pwr_tracking_limit_ofdm));

	if (method == TXAGC) {
		u32	pwr = 0, tx_agc = 0;
		struct _ADAPTER *adapter = p_dm_odm->adapter;

		p_rf_calibrate_info->remnant_ofdm_swing_idx[rf_path] = p_rf_calibrate_info->absolute_ofdm_swing_idx[rf_path];   /*Remnant index equal to aboslute compensate value.*/

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("odm_TxPwrTrackSetPwr88E CH=%d\n", *(p_dm_odm->p_channel)));

		if (p_dm_odm->mp_mode == true) {
			pwr = phy_query_bb_reg(adapter, REG_TX_AGC_A_RATE18_06, 0xFF);
			pwr += p_dm_odm->rf_calibrate_info.power_index_offset[ODM_RF_PATH_A];
			phy_set_bb_reg(adapter, REG_TX_AGC_A_CCK_1_MCS32, MASKBYTE1, pwr);
			tx_agc = (pwr << 16) | (pwr << 8) | (pwr);
			phy_set_bb_reg(adapter, REG_TX_AGC_B_CCK_11_A_CCK_2_11, MASKH3BYTES, tx_agc);
			/* RT_DISP(FPHY, PHY_TXPWR, ("odm_tx_pwr_track_set_pwr88_e: CCK Tx-rf(A) Power = 0x%x\n", tx_agc)); */

			pwr = phy_query_bb_reg(adapter, REG_TX_AGC_A_RATE18_06, 0xFF);
			pwr += (p_rf_calibrate_info->bb_swing_idx_ofdm[ODM_RF_PATH_A] - p_rf_calibrate_info->bb_swing_idx_ofdm_base[RF_PATH_A]);
			tx_agc |= ((pwr << 24) | (pwr << 16) | (pwr << 8) | pwr);
			phy_set_bb_reg(adapter, REG_TX_AGC_A_RATE18_06, MASKDWORD, tx_agc);
			phy_set_bb_reg(adapter, REG_TX_AGC_A_RATE54_24, MASKDWORD, tx_agc);
			phy_set_bb_reg(adapter, REG_TX_AGC_A_MCS03_MCS00, MASKDWORD, tx_agc);
			phy_set_bb_reg(adapter, REG_TX_AGC_A_MCS07_MCS04, MASKDWORD, tx_agc);
			phy_set_bb_reg(adapter, REG_TX_AGC_A_MCS11_MCS08, MASKDWORD, tx_agc);
			phy_set_bb_reg(adapter, REG_TX_AGC_A_MCS15_MCS12, MASKDWORD, tx_agc);
			/* RT_DISP(FPHY, PHY_TXPWR, ("odm_tx_pwr_track_set_pwr88_e: OFDM Tx-rf(A) Power = 0x%x\n", tx_agc)); */
		} else {
			/* phy_set_tx_power_level8188e(p_dm_odm->adapter, *p_dm_odm->p_channel); */
			p_rf_calibrate_info->modify_tx_agc_flag_path_a = true;
			p_rf_calibrate_info->modify_tx_agc_flag_path_a_cck = true;

			if (rf_path == ODM_RF_PATH_A) {
				phy_set_tx_power_index_by_rate_section(adapter, ODM_RF_PATH_A, *p_dm_odm->p_channel, CCK);
				phy_set_tx_power_index_by_rate_section(adapter, ODM_RF_PATH_A, *p_dm_odm->p_channel, OFDM);
				phy_set_tx_power_index_by_rate_section(adapter, ODM_RF_PATH_A, *p_dm_odm->p_channel, HT_MCS0_MCS7);
			}
		}
	} else if (method == BBSWING) {
		final_ofdm_swing_index = p_rf_calibrate_info->default_ofdm_index + p_rf_calibrate_info->absolute_ofdm_swing_idx[rf_path];
		final_cck_swing_index = p_rf_calibrate_info->default_cck_index + p_rf_calibrate_info->absolute_ofdm_swing_idx[rf_path];

		if (final_ofdm_swing_index >= pwr_tracking_limit_ofdm)
			final_ofdm_swing_index = pwr_tracking_limit_ofdm;
		else if (final_ofdm_swing_index <= 0)
			final_ofdm_swing_index = 0;

		if (final_cck_swing_index >= CCK_TABLE_SIZE)
			final_cck_swing_index = CCK_TABLE_SIZE - 1;
		else if (p_rf_calibrate_info->bb_swing_idx_cck <= 0)
			final_cck_swing_index = 0;

		/* Adjust BB swing by OFDM IQ matrix */
		if (rf_path == ODM_RF_PATH_A) {
			set_iqk_matrix_8188e(p_dm_odm, final_ofdm_swing_index, ODM_RF_PATH_A,
				p_dm_odm->rf_calibrate_info.iqk_matrix_reg_setting[channel_mapped_index].value[0][0],
				p_dm_odm->rf_calibrate_info.iqk_matrix_reg_setting[channel_mapped_index].value[0][1]);
			/* Adjust BB swing by CCK filter coefficient */
			if (!p_dm_odm->rf_calibrate_info.is_cck_in_ch14) {
				odm_write_1byte(p_dm_odm, 0xa22, cck_swing_table_ch1_ch13_new[final_cck_swing_index][0]);
				odm_write_1byte(p_dm_odm, 0xa23, cck_swing_table_ch1_ch13_new[final_cck_swing_index][1]);
				odm_write_1byte(p_dm_odm, 0xa24, cck_swing_table_ch1_ch13_new[final_cck_swing_index][2]);
				odm_write_1byte(p_dm_odm, 0xa25, cck_swing_table_ch1_ch13_new[final_cck_swing_index][3]);
				odm_write_1byte(p_dm_odm, 0xa26, cck_swing_table_ch1_ch13_new[final_cck_swing_index][4]);
				odm_write_1byte(p_dm_odm, 0xa27, cck_swing_table_ch1_ch13_new[final_cck_swing_index][5]);
				odm_write_1byte(p_dm_odm, 0xa28, cck_swing_table_ch1_ch13_new[final_cck_swing_index][6]);
				odm_write_1byte(p_dm_odm, 0xa29, cck_swing_table_ch1_ch13_new[final_cck_swing_index][7]);
			} else {
				odm_write_1byte(p_dm_odm, 0xa22, cck_swing_table_ch14_new[final_cck_swing_index][0]);
				odm_write_1byte(p_dm_odm, 0xa23, cck_swing_table_ch14_new[final_cck_swing_index][1]);
				odm_write_1byte(p_dm_odm, 0xa24, cck_swing_table_ch14_new[final_cck_swing_index][2]);
				odm_write_1byte(p_dm_odm, 0xa25, cck_swing_table_ch14_new[final_cck_swing_index][3]);
				odm_write_1byte(p_dm_odm, 0xa26, cck_swing_table_ch14_new[final_cck_swing_index][4]);
				odm_write_1byte(p_dm_odm, 0xa27, cck_swing_table_ch14_new[final_cck_swing_index][5]);
				odm_write_1byte(p_dm_odm, 0xa28, cck_swing_table_ch14_new[final_cck_swing_index][6]);
				odm_write_1byte(p_dm_odm, 0xa29, cck_swing_table_ch14_new[final_cck_swing_index][7]);
			}
		}
	} else if (method == MIX_MODE) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("p_rf_calibrate_info->default_ofdm_index=%d,  p_dm_odm->DefaultCCKIndex=%d, p_rf_calibrate_info->absolute_ofdm_swing_idx[rf_path]=%d, rf_path = %d\n",
			p_rf_calibrate_info->default_ofdm_index, p_rf_calibrate_info->default_cck_index, p_rf_calibrate_info->absolute_ofdm_swing_idx[rf_path], rf_path));

		final_cck_swing_index  = p_rf_calibrate_info->default_cck_index + p_rf_calibrate_info->absolute_ofdm_swing_idx[rf_path];
		final_ofdm_swing_index = p_rf_calibrate_info->default_ofdm_index + p_rf_calibrate_info->absolute_ofdm_swing_idx[rf_path];

		if (final_ofdm_swing_index > pwr_tracking_limit_ofdm) {   /* BBSwing higher then Limit */
			p_rf_calibrate_info->remnant_ofdm_swing_idx[rf_path] = final_ofdm_swing_index - pwr_tracking_limit_ofdm;

			set_iqk_matrix_8188e(p_dm_odm, pwr_tracking_limit_ofdm, ODM_RF_PATH_A,
				p_dm_odm->rf_calibrate_info.iqk_matrix_reg_setting[channel_mapped_index].value[0][0],
				p_dm_odm->rf_calibrate_info.iqk_matrix_reg_setting[channel_mapped_index].value[0][1]);

			p_rf_calibrate_info->modify_tx_agc_flag_path_a = true;

			phy_set_tx_power_index_by_rate_section(adapter, ODM_RF_PATH_A, *p_dm_odm->p_channel, OFDM);
			phy_set_tx_power_index_by_rate_section(adapter, ODM_RF_PATH_A, *p_dm_odm->p_channel, HT_MCS0_MCS7);

			ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Over BBSwing Limit, pwr_tracking_limit = %d, Remnant tx_agc value = %d\n", pwr_tracking_limit_ofdm, p_rf_calibrate_info->remnant_ofdm_swing_idx[rf_path]));
		} else if (final_ofdm_swing_index <= 0) {
			p_rf_calibrate_info->remnant_ofdm_swing_idx[rf_path] = final_ofdm_swing_index;

			set_iqk_matrix_8188e(p_dm_odm, 0, ODM_RF_PATH_A,
				p_dm_odm->rf_calibrate_info.iqk_matrix_reg_setting[channel_mapped_index].value[0][0],
				p_dm_odm->rf_calibrate_info.iqk_matrix_reg_setting[channel_mapped_index].value[0][1]);

			p_rf_calibrate_info->modify_tx_agc_flag_path_a = true;

			phy_set_tx_power_index_by_rate_section(adapter, ODM_RF_PATH_A, *p_dm_odm->p_channel, OFDM);
			phy_set_tx_power_index_by_rate_section(adapter, ODM_RF_PATH_A, *p_dm_odm->p_channel, HT_MCS0_MCS7);

			ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Lower then BBSwing lower bound  0, Remnant tx_agc value = %d\n", p_rf_calibrate_info->remnant_ofdm_swing_idx[rf_path]));
		} else {
			set_iqk_matrix_8188e(p_dm_odm, final_ofdm_swing_index, ODM_RF_PATH_A,
				p_dm_odm->rf_calibrate_info.iqk_matrix_reg_setting[channel_mapped_index].value[0][0],
				p_dm_odm->rf_calibrate_info.iqk_matrix_reg_setting[channel_mapped_index].value[0][1]);

			ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A Compensate with BBSwing, final_ofdm_swing_index = %d\n", final_ofdm_swing_index));

			if (p_rf_calibrate_info->modify_tx_agc_flag_path_a) { /*If tx_agc has changed, reset tx_agc again*/
				p_rf_calibrate_info->remnant_ofdm_swing_idx[rf_path] = 0;

				phy_set_tx_power_index_by_rate_section(adapter, ODM_RF_PATH_A, *p_dm_odm->p_channel, OFDM);
				phy_set_tx_power_index_by_rate_section(adapter, ODM_RF_PATH_A, *p_dm_odm->p_channel, HT_MCS0_MCS7);

				p_rf_calibrate_info->modify_tx_agc_flag_path_a = false;

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A p_dm_odm->Modify_TxAGC_Flag = false\n"));
			}
		}

		if (final_cck_swing_index > pwr_tracking_limit_cck) {
			p_rf_calibrate_info->remnant_cck_swing_idx = final_cck_swing_index - pwr_tracking_limit_cck;

			ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A CCK Over Limit, pwr_tracking_limit_cck = %d, p_rf_calibrate_info->remnant_cck_swing_idx  = %d\n", pwr_tracking_limit_cck, p_rf_calibrate_info->remnant_cck_swing_idx));

			/* Adjust BB swing by CCK filter coefficient */

			if (!p_dm_odm->rf_calibrate_info.is_cck_in_ch14) {
				odm_write_1byte(p_dm_odm, 0xa22, cck_swing_table_ch1_ch13_new[pwr_tracking_limit_cck][0]);
				odm_write_1byte(p_dm_odm, 0xa23, cck_swing_table_ch1_ch13_new[pwr_tracking_limit_cck][1]);
				odm_write_1byte(p_dm_odm, 0xa24, cck_swing_table_ch1_ch13_new[pwr_tracking_limit_cck][2]);
				odm_write_1byte(p_dm_odm, 0xa25, cck_swing_table_ch1_ch13_new[pwr_tracking_limit_cck][3]);
				odm_write_1byte(p_dm_odm, 0xa26, cck_swing_table_ch1_ch13_new[pwr_tracking_limit_cck][4]);
				odm_write_1byte(p_dm_odm, 0xa27, cck_swing_table_ch1_ch13_new[pwr_tracking_limit_cck][5]);
				odm_write_1byte(p_dm_odm, 0xa28, cck_swing_table_ch1_ch13_new[pwr_tracking_limit_cck][6]);
				odm_write_1byte(p_dm_odm, 0xa29, cck_swing_table_ch1_ch13_new[pwr_tracking_limit_cck][7]);
			} else {
				odm_write_1byte(p_dm_odm, 0xa22, cck_swing_table_ch14_new[pwr_tracking_limit_cck][0]);
				odm_write_1byte(p_dm_odm, 0xa23, cck_swing_table_ch14_new[pwr_tracking_limit_cck][1]);
				odm_write_1byte(p_dm_odm, 0xa24, cck_swing_table_ch14_new[pwr_tracking_limit_cck][2]);
				odm_write_1byte(p_dm_odm, 0xa25, cck_swing_table_ch14_new[pwr_tracking_limit_cck][3]);
				odm_write_1byte(p_dm_odm, 0xa26, cck_swing_table_ch14_new[pwr_tracking_limit_cck][4]);
				odm_write_1byte(p_dm_odm, 0xa27, cck_swing_table_ch14_new[pwr_tracking_limit_cck][5]);
				odm_write_1byte(p_dm_odm, 0xa28, cck_swing_table_ch14_new[pwr_tracking_limit_cck][6]);
				odm_write_1byte(p_dm_odm, 0xa29, cck_swing_table_ch14_new[pwr_tracking_limit_cck][7]);
			}

			p_rf_calibrate_info->modify_tx_agc_flag_path_a_cck = true;

			phy_set_tx_power_index_by_rate_section(adapter, ODM_RF_PATH_A, *p_dm_odm->p_channel, CCK);

		} else if (final_cck_swing_index <= 0) { /* Lowest CCK index = 0 */
			p_rf_calibrate_info->remnant_cck_swing_idx = final_cck_swing_index;

			ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A CCK Under Limit, pwr_tracking_limit_cck = %d, p_rf_calibrate_info->remnant_cck_swing_idx  = %d\n", 0, p_rf_calibrate_info->remnant_cck_swing_idx));

			if (!p_dm_odm->rf_calibrate_info.is_cck_in_ch14) {
				odm_write_1byte(p_dm_odm, 0xa22, cck_swing_table_ch1_ch13_new[0][0]);
				odm_write_1byte(p_dm_odm, 0xa23, cck_swing_table_ch1_ch13_new[0][1]);
				odm_write_1byte(p_dm_odm, 0xa24, cck_swing_table_ch1_ch13_new[0][2]);
				odm_write_1byte(p_dm_odm, 0xa25, cck_swing_table_ch1_ch13_new[0][3]);
				odm_write_1byte(p_dm_odm, 0xa26, cck_swing_table_ch1_ch13_new[0][4]);
				odm_write_1byte(p_dm_odm, 0xa27, cck_swing_table_ch1_ch13_new[0][5]);
				odm_write_1byte(p_dm_odm, 0xa28, cck_swing_table_ch1_ch13_new[0][6]);
				odm_write_1byte(p_dm_odm, 0xa29, cck_swing_table_ch1_ch13_new[0][7]);
			} else {
				odm_write_1byte(p_dm_odm, 0xa22, cck_swing_table_ch14_new[0][0]);
				odm_write_1byte(p_dm_odm, 0xa23, cck_swing_table_ch14_new[0][1]);
				odm_write_1byte(p_dm_odm, 0xa24, cck_swing_table_ch14_new[0][2]);
				odm_write_1byte(p_dm_odm, 0xa25, cck_swing_table_ch14_new[0][3]);
				odm_write_1byte(p_dm_odm, 0xa26, cck_swing_table_ch14_new[0][4]);
				odm_write_1byte(p_dm_odm, 0xa27, cck_swing_table_ch14_new[0][5]);
				odm_write_1byte(p_dm_odm, 0xa28, cck_swing_table_ch14_new[0][6]);
				odm_write_1byte(p_dm_odm, 0xa29, cck_swing_table_ch14_new[0][7]);
			}

			p_rf_calibrate_info->modify_tx_agc_flag_path_a_cck = true;

			phy_set_tx_power_index_by_rate_section(adapter, ODM_RF_PATH_A, *p_dm_odm->p_channel, CCK);

		} else {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A CCK Compensate with BBSwing, final_cck_swing_index = %d\n", final_cck_swing_index));

			if (!p_dm_odm->rf_calibrate_info.is_cck_in_ch14) {
				odm_write_1byte(p_dm_odm, 0xa22, cck_swing_table_ch1_ch13_new[final_cck_swing_index][0]);
				odm_write_1byte(p_dm_odm, 0xa23, cck_swing_table_ch1_ch13_new[final_cck_swing_index][1]);
				odm_write_1byte(p_dm_odm, 0xa24, cck_swing_table_ch1_ch13_new[final_cck_swing_index][2]);
				odm_write_1byte(p_dm_odm, 0xa25, cck_swing_table_ch1_ch13_new[final_cck_swing_index][3]);
				odm_write_1byte(p_dm_odm, 0xa26, cck_swing_table_ch1_ch13_new[final_cck_swing_index][4]);
				odm_write_1byte(p_dm_odm, 0xa27, cck_swing_table_ch1_ch13_new[final_cck_swing_index][5]);
				odm_write_1byte(p_dm_odm, 0xa28, cck_swing_table_ch1_ch13_new[final_cck_swing_index][6]);
				odm_write_1byte(p_dm_odm, 0xa29, cck_swing_table_ch1_ch13_new[final_cck_swing_index][7]);
			} else {
				odm_write_1byte(p_dm_odm, 0xa22, cck_swing_table_ch14_new[final_cck_swing_index][0]);
				odm_write_1byte(p_dm_odm, 0xa23, cck_swing_table_ch14_new[final_cck_swing_index][1]);
				odm_write_1byte(p_dm_odm, 0xa24, cck_swing_table_ch14_new[final_cck_swing_index][2]);
				odm_write_1byte(p_dm_odm, 0xa25, cck_swing_table_ch14_new[final_cck_swing_index][3]);
				odm_write_1byte(p_dm_odm, 0xa26, cck_swing_table_ch14_new[final_cck_swing_index][4]);
				odm_write_1byte(p_dm_odm, 0xa27, cck_swing_table_ch14_new[final_cck_swing_index][5]);
				odm_write_1byte(p_dm_odm, 0xa28, cck_swing_table_ch14_new[final_cck_swing_index][6]);
				odm_write_1byte(p_dm_odm, 0xa29, cck_swing_table_ch14_new[final_cck_swing_index][7]);
			}

			if (p_rf_calibrate_info->modify_tx_agc_flag_path_a_cck) { /*If tx_agc has changed, reset tx_agc again*/
				p_rf_calibrate_info->remnant_cck_swing_idx = 0;
				phy_set_tx_power_index_by_rate_section(adapter, ODM_RF_PATH_A, *p_dm_odm->p_channel, CCK);
				p_rf_calibrate_info->modify_tx_agc_flag_path_a_cck = false;

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("******Path_A p_dm_odm->Modify_TxAGC_Flag_CCK = false\n"));
			}
		}
	} else
		return;
}	/* odm_TxPwrTrackSetPwr88E */

void
get_delta_swing_table_8188e(
	void		*p_dm_void,
	u8 **temperature_up_a,
	u8 **temperature_down_a,
	u8 **temperature_up_b,
	u8 **temperature_down_b
)
{
	struct PHY_DM_STRUCT			*p_dm_odm	= (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ADAPTER			*adapter		= p_dm_odm->adapter;
	PHAL_DATA_TYPE	p_hal_data	= GET_HAL_DATA(adapter);
	struct odm_rf_calibration_structure		*p_rf_calibrate_info = &(p_dm_odm->rf_calibrate_info);

	u8			tx_rate			= 0xFF;
	u8			channel			= *p_dm_odm->p_channel;

	if (p_dm_odm->mp_mode == true) {
		PMPT_CONTEXT p_mpt_ctx = &(adapter->mppriv.mpt_ctx);

		tx_rate = mpt_to_mgnt_rate(p_mpt_ctx->mpt_rate_index);
	} else {
		u16	rate	 = *(p_dm_odm->p_forced_data_rate);

		if (!rate) { /*auto rate*/
			if (rate != 0xFF) {
				if (p_dm_odm->number_linked_client != 0)
					tx_rate = hw_rate_to_m_rate(p_dm_odm->tx_rate);
			}
		} else   /*force rate*/
			tx_rate = (u8)rate;
	}

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Power Tracking tx_rate=0x%X\n", tx_rate));

	if (1 <= channel && channel <= 14) {
		if (IS_CCK_RATE(tx_rate)) {
			*temperature_up_a = p_rf_calibrate_info->delta_swing_table_idx_2g_cck_a_p;
			*temperature_down_a = p_rf_calibrate_info->delta_swing_table_idx_2g_cck_a_n;
		} else {
			*temperature_up_a = p_rf_calibrate_info->delta_swing_table_idx_2ga_p;
			*temperature_down_a = p_rf_calibrate_info->delta_swing_table_idx_2ga_n;
		}
	} else {
		*temperature_up_a = (u8 *)delta_swing_table_idx_2ga_p_8188e;
		*temperature_down_a = (u8 *)delta_swing_table_idx_2ga_n_8188e;
	}

}

void configure_txpower_track_8188e(
	struct _TXPWRTRACK_CFG	*p_config
)
{
	p_config->swing_table_size_cck = CCK_TABLE_SIZE;
	p_config->swing_table_size_ofdm = OFDM_TABLE_SIZE;
	p_config->threshold_iqk = IQK_THRESHOLD;
	p_config->average_thermal_num = AVG_THERMAL_NUM_88E;
	p_config->rf_path_count = MAX_PATH_NUM_8188E;
	p_config->thermal_reg_addr = RF_T_METER_88E;

	p_config->odm_tx_pwr_track_set_pwr = odm_tx_pwr_track_set_pwr88_e;
	p_config->do_iqk = do_iqk_8188e;
	p_config->phy_lc_calibrate = phy_lc_calibrate_8188e;
	p_config->get_delta_swing_table = get_delta_swing_table_8188e;
}

/* 1 7.	IQK */
#define MAX_TOLERANCE		5
#define IQK_DELAY_TIME		1		/* ms */

static u8			/* bit0 = 1 => Tx OK, bit1 = 1 => Rx OK */
phy_path_a_iqk_8188e(
	struct _ADAPTER	*p_adapter,
	bool		config_path_b
)
{
	u32 reg_eac, reg_e94, reg_e9c, reg_ea4;
	u8 result = 0x00;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path A IQK!\n"));

	/* 1 Tx IQK */
	/* path-A IQK setting */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path-A IQK setting!\n"));
	odm_set_bb_reg(p_dm_odm, REG_TX_IQK_TONE_A, MASKDWORD, 0x10008c1c);
	odm_set_bb_reg(p_dm_odm, REG_RX_IQK_TONE_A, MASKDWORD, 0x30008c1c);
	odm_set_bb_reg(p_dm_odm, REG_TX_IQK_PI_A, MASKDWORD, 0x8214032a);
	odm_set_bb_reg(p_dm_odm, REG_RX_IQK_PI_A, MASKDWORD, 0x28160000);

	/* LO calibration setting */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LO calibration setting!\n"));
	odm_set_bb_reg(p_dm_odm, REG_IQK_AGC_RSP, MASKDWORD, 0x00462911);

	/* One shot, path A LOK & IQK */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("One shot, path A LOK & IQK!\n"));
	odm_set_bb_reg(p_dm_odm, REG_IQK_AGC_PTS, MASKDWORD, 0xf9000000);
	odm_set_bb_reg(p_dm_odm, REG_IQK_AGC_PTS, MASKDWORD, 0xf8000000);

	/* delay x ms */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("delay %d ms for One shot, path A LOK & IQK.\n", IQK_DELAY_TIME_88E));
	/* platform_stall_execution(IQK_DELAY_TIME_88E*1000); */
	ODM_delay_ms(IQK_DELAY_TIME_88E);

	/* Check failed */
	reg_eac = odm_get_bb_reg(p_dm_odm, REG_RX_POWER_AFTER_IQK_A_2, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xeac = 0x%x\n", reg_eac));
	reg_e94 = odm_get_bb_reg(p_dm_odm, REG_TX_POWER_BEFORE_IQK_A, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe94 = 0x%x\n", reg_e94));
	reg_e9c = odm_get_bb_reg(p_dm_odm, REG_TX_POWER_AFTER_IQK_A, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe9c = 0x%x\n", reg_e9c));
	reg_ea4 = odm_get_bb_reg(p_dm_odm, REG_RX_POWER_BEFORE_IQK_A_2, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xea4 = 0x%x\n", reg_ea4));

	if (!(reg_eac & BIT(28)) &&
	    (((reg_e94 & 0x03FF0000) >> 16) != 0x142) &&
	    (((reg_e9c & 0x03FF0000) >> 16) != 0x42))
		result |= 0x01;
	else							/* if Tx not OK, ignore Rx */
		return result;
	return result;
}

static u8			/* bit0 = 1 => Tx OK, bit1 = 1 => Rx OK */
phy_path_a_rx_iqk(
	struct _ADAPTER	*p_adapter,
	bool		config_path_b
)
{
	u32 reg_eac, reg_e94, reg_e9c, reg_ea4, u4tmp;
	u8 result = 0x00;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path A Rx IQK!\n"));

	/* 1 Get TXIMR setting */
	/* modify RXIQK mode table */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path-A Rx IQK modify RXIQK mode table!\n"));
	odm_set_bb_reg(p_dm_odm, REG_FPGA0_IQK, MASKH3BYTES, 0x000000);
	odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_WE_LUT, RFREGOFFSETMASK, 0x800a0);
	odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_RCK_OS, RFREGOFFSETMASK, 0x30000);
	odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_TXPA_G1, RFREGOFFSETMASK, 0x0000f);
	odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_TXPA_G2, RFREGOFFSETMASK, 0xf117B);
	odm_set_bb_reg(p_dm_odm, REG_FPGA0_IQK, MASKH3BYTES, 0x808000);

	/* IQK setting */
	odm_set_bb_reg(p_dm_odm, REG_TX_IQK, MASKDWORD, 0x01007c00);
	odm_set_bb_reg(p_dm_odm, REG_RX_IQK, MASKDWORD, 0x81004800);

	/* path-A IQK setting */
	odm_set_bb_reg(p_dm_odm, REG_TX_IQK_TONE_A, MASKDWORD, 0x10008c1c);
	odm_set_bb_reg(p_dm_odm, REG_RX_IQK_TONE_A, MASKDWORD, 0x30008c1c);
	odm_set_bb_reg(p_dm_odm, REG_TX_IQK_PI_A, MASKDWORD, 0x82160804);
	odm_set_bb_reg(p_dm_odm, REG_RX_IQK_PI_A, MASKDWORD, 0x28160000);

	/* LO calibration setting */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LO calibration setting!\n"));
	odm_set_bb_reg(p_dm_odm, REG_IQK_AGC_RSP, MASKDWORD, 0x0046a911);

	/* One shot, path A LOK & IQK */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("One shot, path A LOK & IQK!\n"));
	odm_set_bb_reg(p_dm_odm, REG_IQK_AGC_PTS, MASKDWORD, 0xf9000000);
	odm_set_bb_reg(p_dm_odm, REG_IQK_AGC_PTS, MASKDWORD, 0xf8000000);

	/* delay x ms */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("delay %d ms for One shot, path A LOK & IQK.\n", IQK_DELAY_TIME_88E));
	/* platform_stall_execution(IQK_DELAY_TIME_88E*1000); */
	ODM_delay_ms(IQK_DELAY_TIME_88E);

	/* Check failed */
	reg_eac = odm_get_bb_reg(p_dm_odm, REG_RX_POWER_AFTER_IQK_A_2, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xeac = 0x%x\n", reg_eac));
	reg_e94 = odm_get_bb_reg(p_dm_odm, REG_TX_POWER_BEFORE_IQK_A, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe94 = 0x%x\n", reg_e94));
	reg_e9c = odm_get_bb_reg(p_dm_odm, REG_TX_POWER_AFTER_IQK_A, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe9c = 0x%x\n", reg_e9c));

	if (!(reg_eac & BIT(28)) &&
	    (((reg_e94 & 0x03FF0000) >> 16) != 0x142) &&
	    (((reg_e9c & 0x03FF0000) >> 16) != 0x42))
		result |= 0x01;
	else							/* if Tx not OK, ignore Rx */
		return result;

	u4tmp = 0x80007C00 | (reg_e94 & 0x3FF0000)  | ((reg_e9c & 0x3FF0000) >> 16);
	odm_set_bb_reg(p_dm_odm, REG_TX_IQK, MASKDWORD, u4tmp);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe40 = 0x%x u4tmp = 0x%x\n", odm_get_bb_reg(p_dm_odm, REG_TX_IQK, MASKDWORD), u4tmp));

	/* 1 RX IQK */
	/* modify RXIQK mode table */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path-A Rx IQK modify RXIQK mode table 2!\n"));
	odm_set_bb_reg(p_dm_odm, REG_FPGA0_IQK, MASKH3BYTES, 0x000000);
	odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_WE_LUT, RFREGOFFSETMASK, 0x800a0);
	odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_RCK_OS, RFREGOFFSETMASK, 0x30000);
	odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_TXPA_G1, RFREGOFFSETMASK, 0x0000f);
	odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_TXPA_G2, RFREGOFFSETMASK, 0xf7ffa);
	odm_set_bb_reg(p_dm_odm, REG_FPGA0_IQK, MASKH3BYTES, 0x808000);

	/* IQK setting */
	odm_set_bb_reg(p_dm_odm, REG_RX_IQK, MASKDWORD, 0x01004800);

	/* path-A IQK setting */
	odm_set_bb_reg(p_dm_odm, REG_TX_IQK_TONE_A, MASKDWORD, 0x30008c1c);
	odm_set_bb_reg(p_dm_odm, REG_RX_IQK_TONE_A, MASKDWORD, 0x10008c1c);
	odm_set_bb_reg(p_dm_odm, REG_TX_IQK_PI_A, MASKDWORD, 0x82160c05);
	odm_set_bb_reg(p_dm_odm, REG_RX_IQK_PI_A, MASKDWORD, 0x28160c05);

	/* LO calibration setting */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LO calibration setting!\n"));
	odm_set_bb_reg(p_dm_odm, REG_IQK_AGC_RSP, MASKDWORD, 0x0046a911);

	/* One shot, path A LOK & IQK */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("One shot, path A LOK & IQK!\n"));
	odm_set_bb_reg(p_dm_odm, REG_IQK_AGC_PTS, MASKDWORD, 0xf9000000);
	odm_set_bb_reg(p_dm_odm, REG_IQK_AGC_PTS, MASKDWORD, 0xf8000000);

	/* delay x ms */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("delay %d ms for One shot, path A LOK & IQK.\n", IQK_DELAY_TIME_88E));
	/* platform_stall_execution(IQK_DELAY_TIME_88E*1000); */
	ODM_delay_ms(IQK_DELAY_TIME_88E);

	/* Check failed */
	reg_eac = odm_get_bb_reg(p_dm_odm, REG_RX_POWER_AFTER_IQK_A_2, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xeac = 0x%x\n", reg_eac));
	reg_e94 = odm_get_bb_reg(p_dm_odm, REG_TX_POWER_BEFORE_IQK_A, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe94 = 0x%x\n", reg_e94));
	reg_e9c = odm_get_bb_reg(p_dm_odm, REG_TX_POWER_AFTER_IQK_A, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xe9c = 0x%x\n", reg_e9c));
	reg_ea4 = odm_get_bb_reg(p_dm_odm, REG_RX_POWER_BEFORE_IQK_A_2, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xea4 = 0x%x\n", reg_ea4));

	if (!(reg_eac & BIT(27)) &&		/* if Tx is OK, check whether Rx is OK */
	    (((reg_ea4 & 0x03FF0000) >> 16) != 0x132) &&
	    (((reg_eac & 0x03FF0000) >> 16) != 0x36))
		result |= 0x02;
	else
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path A Rx IQK fail!!\n"));

	return result;
}

static u8				/* bit0 = 1 => Tx OK, bit1 = 1 => Rx OK */
phy_path_b_iqk_8188e(
	struct _ADAPTER	*p_adapter
)
{
	u32 reg_eac, reg_eb4, reg_ebc, reg_ec4, reg_ecc;
	u8	result = 0x00;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path B IQK!\n"));

	/* One shot, path B LOK & IQK */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("One shot, path A LOK & IQK!\n"));
	odm_set_bb_reg(p_dm_odm, REG_IQK_AGC_CONT, MASKDWORD, 0x00000002);
	odm_set_bb_reg(p_dm_odm, REG_IQK_AGC_CONT, MASKDWORD, 0x00000000);

	/* delay x ms */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("delay %d ms for One shot, path B LOK & IQK.\n", IQK_DELAY_TIME_88E));
	/* platform_stall_execution(IQK_DELAY_TIME_88E*1000); */
	ODM_delay_ms(IQK_DELAY_TIME_88E);

	/* Check failed */
	reg_eac = odm_get_bb_reg(p_dm_odm, REG_RX_POWER_AFTER_IQK_A_2, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xeac = 0x%x\n", reg_eac));
	reg_eb4 = odm_get_bb_reg(p_dm_odm, REG_TX_POWER_BEFORE_IQK_B, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xeb4 = 0x%x\n", reg_eb4));
	reg_ebc = odm_get_bb_reg(p_dm_odm, REG_TX_POWER_AFTER_IQK_B, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xebc = 0x%x\n", reg_ebc));
	reg_ec4 = odm_get_bb_reg(p_dm_odm, REG_RX_POWER_BEFORE_IQK_B_2, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xec4 = 0x%x\n", reg_ec4));
	reg_ecc = odm_get_bb_reg(p_dm_odm, REG_RX_POWER_AFTER_IQK_B_2, MASKDWORD);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("0xecc = 0x%x\n", reg_ecc));

	if (!(reg_eac & BIT(31)) &&
	    (((reg_eb4 & 0x03FF0000) >> 16) != 0x142) &&
	    (((reg_ebc & 0x03FF0000) >> 16) != 0x42))
		result |= 0x01;
	else
		return result;

	if (!(reg_eac & BIT(30)) &&
	    (((reg_ec4 & 0x03FF0000) >> 16) != 0x132) &&
	    (((reg_ecc & 0x03FF0000) >> 16) != 0x36))
		result |= 0x02;
	else
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path B Rx IQK fail!!\n"));

	return result;

}

static void _phy_path_a_fill_iqk_matrix(
	struct _ADAPTER	*p_adapter,
	bool	is_iqk_ok,
	s32		result[][8],
	u8		final_candidate,
	bool		is_tx_only
)
{
	u32	oldval_0, X, TX0_A, reg;
	s32	Y, TX0_C;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path A IQ Calibration %s !\n", (is_iqk_ok) ? "Success" : "Failed"));

	if (final_candidate == 0xFF)
		return;

	else if (is_iqk_ok) {
		oldval_0 = (odm_get_bb_reg(p_dm_odm, REG_OFDM_0_XA_TX_IQ_IMBALANCE, MASKDWORD) >> 22) & 0x3FF;

		X = result[final_candidate][0];
		if ((X & 0x00000200) != 0)
			X = X | 0xFFFFFC00;
		TX0_A = (X * oldval_0) >> 8;
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("X = 0x%x, TX0_A = 0x%x, oldval_0 0x%x\n", X, TX0_A, oldval_0));
		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XA_TX_IQ_IMBALANCE, 0x3FF, TX0_A);

		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_ECCA_THRESHOLD, BIT(31), ((X * oldval_0 >> 7) & 0x1));

		Y = result[final_candidate][1];
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;

		TX0_C = (Y * oldval_0) >> 8;
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Y = 0x%x, TX = 0x%x\n", Y, TX0_C));
		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XC_TX_AFE, 0xF0000000, ((TX0_C & 0x3C0) >> 6));
		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XA_TX_IQ_IMBALANCE, 0x003F0000, (TX0_C & 0x3F));

		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_ECCA_THRESHOLD, BIT(29), ((Y * oldval_0 >> 7) & 0x1));

		if (is_tx_only) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_path_a_fill_iqk_matrix only Tx OK\n"));
			return;
		}

		reg = result[final_candidate][2];
		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XA_RX_IQ_IMBALANCE, 0x3FF, reg);

		reg = result[final_candidate][3] & 0x3F;
		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XA_RX_IQ_IMBALANCE, 0xFC00, reg);

		reg = (result[final_candidate][3] >> 6) & 0xF;
		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_RX_IQ_EXT_ANTA, 0xF0000000, reg);
	}
}

static void _phy_path_b_fill_iqk_matrix(
	struct _ADAPTER	*p_adapter,
	bool	is_iqk_ok,
	s32		result[][8],
	u8		final_candidate,
	bool		is_tx_only			/* do Tx only */
)
{
	u32	oldval_1, X, TX1_A, reg;
	s32	Y, TX1_C;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path B IQ Calibration %s !\n", (is_iqk_ok) ? "Success" : "Failed"));

	if (final_candidate == 0xFF)
		return;

	else if (is_iqk_ok) {
		oldval_1 = (odm_get_bb_reg(p_dm_odm, REG_OFDM_0_XB_TX_IQ_IMBALANCE, MASKDWORD) >> 22) & 0x3FF;

		X = result[final_candidate][4];
		if ((X & 0x00000200) != 0)
			X = X | 0xFFFFFC00;
		TX1_A = (X * oldval_1) >> 8;
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("X = 0x%x, TX1_A = 0x%x\n", X, TX1_A));
		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XB_TX_IQ_IMBALANCE, 0x3FF, TX1_A);

		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_ECCA_THRESHOLD, BIT(27), ((X * oldval_1 >> 7) & 0x1));

		Y = result[final_candidate][5];
		if ((Y & 0x00000200) != 0)
			Y = Y | 0xFFFFFC00;

		TX1_C = (Y * oldval_1) >> 8;
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Y = 0x%x, TX1_C = 0x%x\n", Y, TX1_C));
		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XD_TX_AFE, 0xF0000000, ((TX1_C & 0x3C0) >> 6));
		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XB_TX_IQ_IMBALANCE, 0x003F0000, (TX1_C & 0x3F));

		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_ECCA_THRESHOLD, BIT(25), ((Y * oldval_1 >> 7) & 0x1));

		if (is_tx_only)
			return;

		reg = result[final_candidate][6];
		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XB_RX_IQ_IMBALANCE, 0x3FF, reg);

		reg = result[final_candidate][7] & 0x3F;
		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_XB_RX_IQ_IMBALANCE, 0xFC00, reg);

		reg = (result[final_candidate][7] >> 6) & 0xF;
		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_AGC_RSSI_TABLE, 0x0000F000, reg);
	}
}

/*
 * 2011/07/26 MH Add an API for testing IQK fail case.
 *
 * MP Already declare in odm.c */
static bool odm_check_power_status(
	struct _ADAPTER		*adapter)
{
	return	true;
}

void
_phy_save_adda_registers(
	struct _ADAPTER	*p_adapter,
	u32		*adda_reg,
	u32		*adda_backup,
	u32		register_num
)
{
	u32	i;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;

	if (odm_check_power_status(p_adapter) == false)
		return;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Save ADDA parameters.\n"));
	for (i = 0 ; i < register_num ; i++)
		adda_backup[i] = odm_get_bb_reg(p_dm_odm, adda_reg[i], MASKDWORD);
}

static void _phy_save_mac_registers(
	struct _ADAPTER	*p_adapter,
	u32		*mac_reg,
	u32		*mac_backup
)
{
	u32	i;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Save MAC parameters.\n"));
	for (i = 0 ; i < (IQK_MAC_REG_NUM - 1); i++)
		mac_backup[i] = odm_read_1byte(p_dm_odm, mac_reg[i]);
	mac_backup[i] = odm_read_4byte(p_dm_odm, mac_reg[i]);

}

static void _phy_reload_adda_registers(
	struct _ADAPTER	*p_adapter,
	u32		*adda_reg,
	u32		*adda_backup,
	u32		regiester_num
)
{
	u32	i;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Reload ADDA power saving parameters !\n"));
	for (i = 0 ; i < regiester_num; i++)
		odm_set_bb_reg(p_dm_odm, adda_reg[i], MASKDWORD, adda_backup[i]);
}

static void _phy_reload_mac_registers(
	struct _ADAPTER	*p_adapter,
	u32		*mac_reg,
	u32		*mac_backup
)
{
	u32	i;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("Reload MAC parameters !\n"));
	for (i = 0 ; i < (IQK_MAC_REG_NUM - 1); i++)
		odm_write_1byte(p_dm_odm, mac_reg[i], (u8)mac_backup[i]);
	odm_write_4byte(p_dm_odm, mac_reg[i], mac_backup[i]);
}

void
_phy_path_adda_on(
	struct _ADAPTER	*p_adapter,
	u32		*adda_reg,
	bool		is_path_a_on,
	bool		is2T
)
{
	u32	path_on;
	u32	i;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("ADDA ON.\n"));

	path_on = is_path_a_on ? 0x04db25a4 : 0x0b1b25a4;
	if (false == is2T) {
		path_on = 0x0bdb25a0;
		odm_set_bb_reg(p_dm_odm, adda_reg[0], MASKDWORD, 0x0b1b25a0);
	} else
		odm_set_bb_reg(p_dm_odm, adda_reg[0], MASKDWORD, path_on);

	for (i = 1 ; i < IQK_ADDA_REG_NUM ; i++)
		odm_set_bb_reg(p_dm_odm, adda_reg[i], MASKDWORD, path_on);

}

void
_phy_mac_setting_calibration(
	struct _ADAPTER	*p_adapter,
	u32		*mac_reg,
	u32		*mac_backup
)
{
	u32	i = 0;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("MAC settings for Calibration.\n"));

	odm_write_1byte(p_dm_odm, mac_reg[i], 0x3F);

	for (i = 1 ; i < (IQK_MAC_REG_NUM - 1); i++)
		odm_write_1byte(p_dm_odm, mac_reg[i], (u8)(mac_backup[i] & (~BIT(3))));
	odm_write_1byte(p_dm_odm, mac_reg[i], (u8)(mac_backup[i] & (~BIT(5))));

}

void _phy_path_a_stand_by(struct _ADAPTER *p_adapter)
{
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path-A standby mode!\n"));

	odm_set_bb_reg(p_dm_odm, REG_FPGA0_IQK, MASKH3BYTES, 0x0);
	odm_set_bb_reg(p_dm_odm, 0x840, MASKDWORD, 0x00010000);
	odm_set_bb_reg(p_dm_odm, REG_FPGA0_IQK, MASKH3BYTES, 0x808000);
}

static void _phy_pi_mode_switch(
	struct _ADAPTER	*p_adapter,
	bool		pi_mode
)
{
	u32	mode;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("BB Switch to %s mode!\n", (pi_mode ? "PI" : "SI")));

	mode = pi_mode ? 0x01000100 : 0x01000000;
	odm_set_bb_reg(p_dm_odm, REG_FPGA0_XA_HSSI_PARAMETER1, MASKDWORD, mode);
	odm_set_bb_reg(p_dm_odm, REG_FPGA0_XB_HSSI_PARAMETER1, MASKDWORD, mode);
}

static bool phy_simularity_compare_8188e(
	struct _ADAPTER	*p_adapter,
	s32		result[][8],
	u8		 c1,
	u8		 c2
)
{
	u32		i, j, diff, simularity_bit_map, bound;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;
	u8		final_candidate[2] = {0xFF, 0xFF};	/* for path A and path B */
	bool		is_result = true;
	bool		is2T = IS_2T2R(p_hal_data->version_id);

	if (is2T)
		bound = 8;
	else
		bound = 4;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("===> IQK:phy_simularity_compare_8188e c1 %d c2 %d!!!\n", c1, c2));

	simularity_bit_map = 0;

	for (i = 0; i < bound; i++) {
		diff = (result[c1][i] > result[c2][i]) ? (result[c1][i] - result[c2][i]) : (result[c2][i] - result[c1][i]);
		if (diff > MAX_TOLERANCE) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK:phy_simularity_compare_8188e differnece overflow index %d compare1 0x%x compare2 0x%x!!!\n",  i, result[c1][i], result[c2][i]));

			if ((i == 2 || i == 6) && !simularity_bit_map) {
				if (result[c1][i] + result[c1][i + 1] == 0)
					final_candidate[(i / 4)] = c2;
				else if (result[c2][i] + result[c2][i + 1] == 0)
					final_candidate[(i / 4)] = c1;
				else
					simularity_bit_map = simularity_bit_map | (1 << i);
			} else
				simularity_bit_map = simularity_bit_map | (1 << i);
		}
	}

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK:phy_simularity_compare_8188e simularity_bit_map   %d !!!\n", simularity_bit_map));

	if (simularity_bit_map == 0) {
		for (i = 0; i < (bound / 4); i++) {
			if (final_candidate[i] != 0xFF) {
				for (j = i * 4; j < (i + 1) * 4 - 2; j++)
					result[3][j] = result[final_candidate[i]][j];
				is_result = false;
			}
		}
		return is_result;
	} else if (!(simularity_bit_map & 0x0F)) {		/* path A OK */
		for (i = 0; i < 4; i++)
			result[3][i] = result[c1][i];
		return false;
	} else if (!(simularity_bit_map & 0xF0) && is2T) {	/* path B OK */
		for (i = 4; i < 8; i++)
			result[3][i] = result[c1][i];
		return false;
	} else
		return false;

}

static void _phy_iq_calibrate_8188e(
	struct _ADAPTER	*p_adapter,
	s32		result[][8],
	u8		t,
	bool		is2T
)
{
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;
	u32			i;
	u8			path_aok = 0, path_bok = 0;
	u32			ADDA_REG[IQK_ADDA_REG_NUM] = {
		REG_FPGA0_XCD_SWITCH_CONTROL,	REG_BLUE_TOOTH,
		REG_RX_WAIT_CCA,		REG_TX_CCK_RFON,
		REG_TX_CCK_BBON,	REG_TX_OFDM_RFON,
		REG_TX_OFDM_BBON,	REG_TX_TO_RX,
		REG_TX_TO_TX,		REG_RX_CCK,
		REG_RX_OFDM,		REG_RX_WAIT_RIFS,
		REG_RX_TO_RX,		REG_STANDBY,
		REG_SLEEP,			REG_PMPD_ANAEN
	};
	u32			IQK_MAC_REG[IQK_MAC_REG_NUM] = {
		REG_TXPAUSE,		REG_BCN_CTRL,
		REG_BCN_CTRL_1,	REG_GPIO_MUXCFG
	};

	/* since 92C & 92D have the different define in IQK_BB_REG */
	u32	IQK_BB_REG_92C[IQK_BB_REG_NUM] = {
		REG_OFDM_0_TRX_PATH_ENABLE,		REG_OFDM_0_TR_MUX_PAR,
		REG_FPGA0_XCD_RF_INTERFACE_SW,	REG_CONFIG_ANT_A,	REG_CONFIG_ANT_B,
		REG_FPGA0_XAB_RF_INTERFACE_SW,	REG_FPGA0_XA_RF_INTERFACE_OE,
		REG_FPGA0_XB_RF_INTERFACE_OE,	REG_CCK_0_AFE_SETTING
	};

	u32	retry_count = 2;

	if (p_dm_odm->mp_mode == true)
		retry_count = 9;

	/* Note: IQ calibration must be performed after loading */
	/*		PHY_REG.txt , and radio_a, radio_b.txt */

	/* u32 bbvalue; */

	if (t == 0) {
		/*	 	 bbvalue = odm_get_bb_reg(p_dm_odm, REG_FPGA0_RFMOD, MASKDWORD);
		 * 			RT_DISP(FINIT, INIT_IQK, ("_phy_iq_calibrate_8188e()==>0x%08x\n",bbvalue)); */

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQ Calibration for %s for %d times\n", (is2T ? "2T2R" : "1T1R"), t));

		/* Save ADDA parameters, turn path A ADDA on */
		_phy_save_adda_registers(p_adapter, ADDA_REG, p_dm_odm->rf_calibrate_info.ADDA_backup, IQK_ADDA_REG_NUM);
		_phy_save_mac_registers(p_adapter, IQK_MAC_REG, p_dm_odm->rf_calibrate_info.IQK_MAC_backup);
		_phy_save_adda_registers(p_adapter, IQK_BB_REG_92C, p_dm_odm->rf_calibrate_info.IQK_BB_backup, IQK_BB_REG_NUM);
	}
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQ Calibration for %s for %d times\n", (is2T ? "2T2R" : "1T1R"), t));

	_phy_path_adda_on(p_adapter, ADDA_REG, true, is2T);

	if (t == 0)
		p_dm_odm->rf_calibrate_info.is_rf_pi_enable = (u8)odm_get_bb_reg(p_dm_odm, REG_FPGA0_XA_HSSI_PARAMETER1, BIT(8));

	if (!p_dm_odm->rf_calibrate_info.is_rf_pi_enable) {
		/* Switch BB to PI mode to do IQ Calibration. */
		_phy_pi_mode_switch(p_adapter, true);
	}

	/* MAC settings */
	_phy_mac_setting_calibration(p_adapter, IQK_MAC_REG, p_dm_odm->rf_calibrate_info.IQK_MAC_backup);

	/* BB setting */
	/* odm_set_bb_reg(p_dm_odm, REG_FPGA0_RFMOD, BIT24, 0x00); */
	odm_set_bb_reg(p_dm_odm, REG_CCK_0_AFE_SETTING, 0x0f000000, 0xf);
	odm_set_bb_reg(p_dm_odm, REG_OFDM_0_TRX_PATH_ENABLE, MASKDWORD, 0x03a05600);
	odm_set_bb_reg(p_dm_odm, REG_OFDM_0_TR_MUX_PAR, MASKDWORD, 0x000800e4);
	odm_set_bb_reg(p_dm_odm, REG_FPGA0_XCD_RF_INTERFACE_SW, MASKDWORD, 0x22204000);

	odm_set_bb_reg(p_dm_odm, REG_FPGA0_XAB_RF_INTERFACE_SW, BIT(10), 0x01);
	odm_set_bb_reg(p_dm_odm, REG_FPGA0_XAB_RF_INTERFACE_SW, BIT(26), 0x01);
	odm_set_bb_reg(p_dm_odm, REG_FPGA0_XA_RF_INTERFACE_OE, BIT(10), 0x00);
	odm_set_bb_reg(p_dm_odm, REG_FPGA0_XB_RF_INTERFACE_OE, BIT(10), 0x00);

	if (is2T) {
		odm_set_bb_reg(p_dm_odm, REG_FPGA0_XA_LSSI_PARAMETER, MASKDWORD, 0x00010000);
		odm_set_bb_reg(p_dm_odm, REG_FPGA0_XB_LSSI_PARAMETER, MASKDWORD, 0x00010000);
	}

	/* Page B init */
	/* AP or IQK */
	odm_set_bb_reg(p_dm_odm, REG_CONFIG_ANT_A, MASKDWORD, 0x0f600000);

	if (is2T)
		odm_set_bb_reg(p_dm_odm, REG_CONFIG_ANT_B, MASKDWORD, 0x0f600000);

	/* IQ calibration setting */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK setting!\n"));
	odm_set_bb_reg(p_dm_odm, REG_FPGA0_IQK, MASKH3BYTES, 0x808000);
	odm_set_bb_reg(p_dm_odm, REG_TX_IQK, MASKDWORD, 0x01007c00);
	odm_set_bb_reg(p_dm_odm, REG_RX_IQK, MASKDWORD, 0x81004800);

	for (i = 0 ; i < retry_count ; i++) {
		path_aok = phy_path_a_iqk_8188e(p_adapter, is2T);
		/*		if(path_aok == 0x03){ */
		if (path_aok == 0x01) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path A Tx IQK Success!!\n"));
			result[t][0] = (odm_get_bb_reg(p_dm_odm, REG_TX_POWER_BEFORE_IQK_A, MASKDWORD) & 0x3FF0000) >> 16;
			result[t][1] = (odm_get_bb_reg(p_dm_odm, REG_TX_POWER_AFTER_IQK_A, MASKDWORD) & 0x3FF0000) >> 16;
			break;
		}
	}

	for (i = 0 ; i < retry_count ; i++) {
		path_aok = phy_path_a_rx_iqk(p_adapter, is2T);
		if (path_aok == 0x03) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path A Rx IQK Success!!\n"));
			/*				result[t][0] = (odm_get_bb_reg(p_dm_odm, REG_TX_POWER_BEFORE_IQK_A, MASKDWORD)&0x3FF0000)>>16;
			 *				result[t][1] = (odm_get_bb_reg(p_dm_odm, REG_TX_POWER_AFTER_IQK_A, MASKDWORD)&0x3FF0000)>>16; */
			result[t][2] = (odm_get_bb_reg(p_dm_odm, REG_RX_POWER_BEFORE_IQK_A_2, MASKDWORD) & 0x3FF0000) >> 16;
			result[t][3] = (odm_get_bb_reg(p_dm_odm, REG_RX_POWER_AFTER_IQK_A_2, MASKDWORD) & 0x3FF0000) >> 16;
			break;
		} else
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path A Rx IQK Fail!!\n"));
	}

	if (0x00 == path_aok)
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path A IQK failed!!\n"));

	if (is2T) {
		_phy_path_a_stand_by(p_adapter);

		/* Turn path B ADDA on */
		_phy_path_adda_on(p_adapter, ADDA_REG, false, is2T);

		for (i = 0 ; i < retry_count ; i++) {
			path_bok = phy_path_b_iqk_8188e(p_adapter);
			if (path_bok == 0x03) {
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path B IQK Success!!\n"));
				result[t][4] = (odm_get_bb_reg(p_dm_odm, REG_TX_POWER_BEFORE_IQK_B, MASKDWORD) & 0x3FF0000) >> 16;
				result[t][5] = (odm_get_bb_reg(p_dm_odm, REG_TX_POWER_AFTER_IQK_B, MASKDWORD) & 0x3FF0000) >> 16;
				result[t][6] = (odm_get_bb_reg(p_dm_odm, REG_RX_POWER_BEFORE_IQK_B_2, MASKDWORD) & 0x3FF0000) >> 16;
				result[t][7] = (odm_get_bb_reg(p_dm_odm, REG_RX_POWER_AFTER_IQK_B_2, MASKDWORD) & 0x3FF0000) >> 16;
				break;
			} else if (i == (retry_count - 1) && path_bok == 0x01) {	/* Tx IQK OK */
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path B Only Tx IQK Success!!\n"));
				result[t][4] = (odm_get_bb_reg(p_dm_odm, REG_TX_POWER_BEFORE_IQK_B, MASKDWORD) & 0x3FF0000) >> 16;
				result[t][5] = (odm_get_bb_reg(p_dm_odm, REG_TX_POWER_AFTER_IQK_B, MASKDWORD) & 0x3FF0000) >> 16;
			}
		}

		if (0x00 == path_bok)
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("path B IQK failed!!\n"));
	}

	/* Back to BB mode, load original value */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK:Back to BB mode, load original value!\n"));
	odm_set_bb_reg(p_dm_odm, REG_FPGA0_IQK, MASKH3BYTES, 0);

	if (t != 0) {
		if (!p_dm_odm->rf_calibrate_info.is_rf_pi_enable) {
			/* Switch back BB to SI mode after finish IQ Calibration. */
			_phy_pi_mode_switch(p_adapter, false);
		}

		/* Reload ADDA power saving parameters */
		_phy_reload_adda_registers(p_adapter, ADDA_REG, p_dm_odm->rf_calibrate_info.ADDA_backup, IQK_ADDA_REG_NUM);

		/* Reload MAC parameters */
		_phy_reload_mac_registers(p_adapter, IQK_MAC_REG, p_dm_odm->rf_calibrate_info.IQK_MAC_backup);

		_phy_reload_adda_registers(p_adapter, IQK_BB_REG_92C, p_dm_odm->rf_calibrate_info.IQK_BB_backup, IQK_BB_REG_NUM);

		/* Restore RX initial gain */
		odm_set_bb_reg(p_dm_odm, REG_FPGA0_XA_LSSI_PARAMETER, MASKDWORD, 0x00032ed3);
		if (is2T)
			odm_set_bb_reg(p_dm_odm, REG_FPGA0_XB_LSSI_PARAMETER, MASKDWORD, 0x00032ed3);

		/* load 0xe30 IQC default value */
		odm_set_bb_reg(p_dm_odm, REG_TX_IQK_TONE_A, MASKDWORD, 0x01008c00);
		odm_set_bb_reg(p_dm_odm, REG_RX_IQK_TONE_A, MASKDWORD, 0x01008c00);

	}
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_iq_calibrate_8188e() <==\n"));

}

static void _phy_lc_calibrate_8188e(struct PHY_DM_STRUCT *p_dm_odm, bool is2T)
{
	u8	tmp_reg;
	u32	rf_amode = 0, rf_bmode = 0, lc_cal;
	struct _ADAPTER *p_adapter = p_dm_odm->adapter;
	/* Check continuous TX and Packet TX */
	tmp_reg = odm_read_1byte(p_dm_odm, 0xd03);

	if ((tmp_reg & 0x70) != 0)			/* Deal with contisuous TX case */
		odm_write_1byte(p_dm_odm, 0xd03, tmp_reg & 0x8F);	/* disable all continuous TX */
	else							/* Deal with Packet TX case */
		odm_write_1byte(p_dm_odm, REG_TXPAUSE, 0xFF);			/* block all queues */

	if ((tmp_reg & 0x70) != 0) {
		/* 1. Read original RF mode */
		/* path-A */
		rf_amode = phy_query_rf_reg(p_adapter, ODM_RF_PATH_A, RF_AC, MASK12BITS);

		/* path-B */
		if (is2T)
			rf_bmode = phy_query_rf_reg(p_adapter, ODM_RF_PATH_B, RF_AC, MASK12BITS);

		/* 2. Set RF mode = standby mode */
		/* path-A */
		odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_AC, MASK12BITS, (rf_amode & 0x8FFFF) | 0x10000);

		/* path-B */
		if (is2T)
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, RF_AC, MASK12BITS, (rf_bmode & 0x8FFFF) | 0x10000);
	}

	/* 3. Read RF reg18 */
	lc_cal = phy_query_rf_reg(p_adapter, ODM_RF_PATH_A, RF_CHNLBW, MASK12BITS);

	/* 4. Set LC calibration begin	bit15 */
	odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_CHNLBW, MASK12BITS, lc_cal | 0x08000);

	ODM_delay_ms(100);

	/* Restore original situation */
	if ((tmp_reg & 0x70) != 0) {	/* Deal with contisuous TX case */
		/* path-A */
		odm_write_1byte(p_dm_odm, 0xd03, tmp_reg);
		odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_AC, MASK12BITS, rf_amode);

		/* path-B */
		if (is2T)
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, RF_AC, MASK12BITS, rf_bmode);
	} else /* Deal with Packet TX case */
		odm_write_1byte(p_dm_odm, REG_TXPAUSE, 0x00);
}

/* Analog Pre-distortion calibration */
#define		APK_BB_REG_NUM	8
#define		APK_CURVE_REG_NUM 4
#define		PATH_NUM		2

static void _phy_ap_calibrate_8188e(
	struct _ADAPTER	*p_adapter,
	s8		delta,
	bool		is2T
)
{
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;
	u32			reg_d[PATH_NUM];
	u32			tmp_reg, index, offset,  apkbound;
	u8			path, i, pathbound = PATH_NUM;
	u32			BB_backup[APK_BB_REG_NUM];
	u32			BB_REG[APK_BB_REG_NUM] = {
		REG_FPGA1_TX_BLOCK,	REG_OFDM_0_TRX_PATH_ENABLE,
		REG_FPGA0_RFMOD,	REG_OFDM_0_TR_MUX_PAR,
		REG_FPGA0_XCD_RF_INTERFACE_SW,	REG_FPGA0_XAB_RF_INTERFACE_SW,
		REG_FPGA0_XA_RF_INTERFACE_OE,	REG_FPGA0_XB_RF_INTERFACE_OE
	};
	u32			BB_AP_MODE[APK_BB_REG_NUM] = {
		0x00000020, 0x00a05430, 0x02040000,
		0x000800e4, 0x00204000
	};
	u32			BB_normal_AP_MODE[APK_BB_REG_NUM] = {
		0x00000020, 0x00a05430, 0x02040000,
		0x000800e4, 0x22204000
	};

	u32			AFE_backup[IQK_ADDA_REG_NUM];
	u32			AFE_REG[IQK_ADDA_REG_NUM] = {
		REG_FPGA0_XCD_SWITCH_CONTROL,	REG_BLUE_TOOTH,
		REG_RX_WAIT_CCA,		REG_TX_CCK_RFON,
		REG_TX_CCK_BBON,	REG_TX_OFDM_RFON,
		REG_TX_OFDM_BBON,	REG_TX_TO_RX,
		REG_TX_TO_TX,		REG_RX_CCK,
		REG_RX_OFDM,		REG_RX_WAIT_RIFS,
		REG_RX_TO_RX,		REG_STANDBY,
		REG_SLEEP,			REG_PMPD_ANAEN
	};

	u32			MAC_backup[IQK_MAC_REG_NUM];
	u32			MAC_REG[IQK_MAC_REG_NUM] = {
		REG_TXPAUSE,		REG_BCN_CTRL,
		REG_BCN_CTRL_1,	REG_GPIO_MUXCFG
	};

	u32			APK_RF_init_value[PATH_NUM][APK_BB_REG_NUM] = {
		{0x0852c, 0x1852c, 0x5852c, 0x1852c, 0x5852c},
		{0x2852e, 0x0852e, 0x3852e, 0x0852e, 0x0852e}
	};

	u32			APK_normal_RF_init_value[PATH_NUM][APK_BB_REG_NUM] = {
		{0x0852c, 0x0a52c, 0x3a52c, 0x5a52c, 0x5a52c},	/* path settings equal to path b settings */
		{0x0852c, 0x0a52c, 0x5a52c, 0x5a52c, 0x5a52c}
	};

	u32			APK_RF_value_0[PATH_NUM][APK_BB_REG_NUM] = {
		{0x52019, 0x52014, 0x52013, 0x5200f, 0x5208d},
		{0x5201a, 0x52019, 0x52016, 0x52033, 0x52050}
	};

	u32			APK_normal_RF_value_0[PATH_NUM][APK_BB_REG_NUM] = {
		{0x52019, 0x52017, 0x52010, 0x5200d, 0x5206a},	/* path settings equal to path b settings */
		{0x52019, 0x52017, 0x52010, 0x5200d, 0x5206a}
	};

	u32			AFE_on_off[PATH_NUM] = {
		0x04db25a4, 0x0b1b25a4
	};	/* path A on path B off / path A off path B on */

	u32			APK_offset[PATH_NUM] = {
		REG_CONFIG_ANT_A, REG_CONFIG_ANT_B
	};

	u32			APK_normal_offset[PATH_NUM] = {
		REG_CONFIG_PMPD_ANT_A, REG_CONFIG_PMPD_ANT_B
	};

	u32			APK_value[PATH_NUM] = {
		0x92fc0000, 0x12fc0000
	};

	u32			APK_normal_value[PATH_NUM] = {
		0x92680000, 0x12680000
	};

	s8			APK_delta_mapping[APK_BB_REG_NUM][13] = {
		{-4, -3, -2, -2, -1, -1, 0, 1, 2, 3, 4, 5, 6},
		{-4, -3, -2, -2, -1, -1, 0, 1, 2, 3, 4, 5, 6},
		{-6, -4, -2, -2, -1, -1, 0, 1, 2, 3, 4, 5, 6},
		{-1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6},
		{-11, -9, -7, -5, -3, -1, 0, 0, 0, 0, 0, 0, 0}
	};

	u32			APK_normal_setting_value_1[13] = {
		0x01017018, 0xf7ed8f84, 0x1b1a1816, 0x2522201e, 0x322e2b28,
		0x433f3a36, 0x5b544e49, 0x7b726a62, 0xa69a8f84, 0xdfcfc0b3,
		0x12680000, 0x00880000, 0x00880000
	};

	u32			APK_normal_setting_value_2[16] = {
		0x01c7021d, 0x01670183, 0x01000123, 0x00bf00e2, 0x008d00a3,
		0x0068007b, 0x004d0059, 0x003a0042, 0x002b0031, 0x001f0025,
		0x0017001b, 0x00110014, 0x000c000f, 0x0009000b, 0x00070008,
		0x00050006
	};

	u32			APK_result[PATH_NUM][APK_BB_REG_NUM];	/* val_1_1a, val_1_2a, val_2a, val_3a, val_4a
 *	u32			AP_curve[PATH_NUM][APK_CURVE_REG_NUM]; */

	s32			BB_offset, delta_V, delta_offset;

#if MP_DRIVER == 1
	PMPT_CONTEXT	p_mpt_ctx = &(p_adapter->mppriv.mpt_ctx);

	if (p_dm_odm->mp_mode == true) {
		p_mpt_ctx->APK_bound[0] = 45;
		p_mpt_ctx->APK_bound[1] = 52;
	}
#endif

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("==>_phy_ap_calibrate_8188e() delta %d\n", delta));
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("AP Calibration for %s\n", (is2T ? "2T2R" : "1T1R")));
	if (!is2T)
		pathbound = 1;

	/* 2 FOR NORMAL CHIP SETTINGS */

	/* Temporarily do not allow normal driver to do the following settings because these offset
	 * and value will cause RF internal PA to be unpredictably disabled by HW, such that RF Tx signal
	 * will disappear after disable/enable card many times on 88CU. RF SD and DD have not find the
	 * root cause, so we remove these actions temporarily. Added by tynli and SD3 Allen. 2010.05.31.
	 * #if MP_DRIVER != 1 */
	if (p_dm_odm->mp_mode == false)
		return;
	/* #endif */
	/* settings adjust for normal chip */
	for (index = 0; index < PATH_NUM; index++) {
		APK_offset[index] = APK_normal_offset[index];
		APK_value[index] = APK_normal_value[index];
		AFE_on_off[index] = 0x6fdb25a4;
	}

	for (index = 0; index < APK_BB_REG_NUM; index++) {
		for (path = 0; path < pathbound; path++) {
			APK_RF_init_value[path][index] = APK_normal_RF_init_value[path][index];
			APK_RF_value_0[path][index] = APK_normal_RF_value_0[path][index];
		}
		BB_AP_MODE[index] = BB_normal_AP_MODE[index];
	}

	apkbound = 6;

	/* save BB default value */
	for (index = 0; index < APK_BB_REG_NUM ; index++) {
		if (index == 0)		/* skip */
			continue;
		BB_backup[index] = odm_get_bb_reg(p_dm_odm, BB_REG[index], MASKDWORD);
	}

	/* save MAC default value */
	_phy_save_mac_registers(p_adapter, MAC_REG, MAC_backup);

	/* save AFE default value */
	_phy_save_adda_registers(p_adapter, AFE_REG, AFE_backup, IQK_ADDA_REG_NUM);

	for (path = 0; path < pathbound; path++) {
		if (path == ODM_RF_PATH_A) {
			/* path A APK */
			/* load APK setting */
			/* path-A */
			offset = REG_PDP_ANT_A;
			for (index = 0; index < 11; index++) {
				odm_set_bb_reg(p_dm_odm, offset, MASKDWORD, APK_normal_setting_value_1[index]);
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_ap_calibrate_8188e() offset 0x%x value 0x%x\n", offset, odm_get_bb_reg(p_dm_odm, offset, MASKDWORD)));

				offset += 0x04;
			}

			odm_set_bb_reg(p_dm_odm, REG_CONFIG_PMPD_ANT_B, MASKDWORD, 0x12680000);

			offset = REG_CONFIG_ANT_A;
			for (; index < 13; index++) {
				odm_set_bb_reg(p_dm_odm, offset, MASKDWORD, APK_normal_setting_value_1[index]);
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_ap_calibrate_8188e() offset 0x%x value 0x%x\n", offset, odm_get_bb_reg(p_dm_odm, offset, MASKDWORD)));

				offset += 0x04;
			}

			/* page-B1 */
			odm_set_bb_reg(p_dm_odm, REG_FPGA0_IQK, MASKH3BYTES, 0x400000);

			/* path A */
			offset = REG_PDP_ANT_A;
			for (index = 0; index < 16; index++) {
				odm_set_bb_reg(p_dm_odm, offset, MASKDWORD, APK_normal_setting_value_2[index]);
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_ap_calibrate_8188e() offset 0x%x value 0x%x\n", offset, odm_get_bb_reg(p_dm_odm, offset, MASKDWORD)));

				offset += 0x04;
			}
			odm_set_bb_reg(p_dm_odm, REG_FPGA0_IQK, MASKH3BYTES, 0x000000);
		} else if (path == ODM_RF_PATH_B) {
			/* path B APK */
			/* load APK setting */
			/* path-B */
			offset = REG_PDP_ANT_B;
			for (index = 0; index < 10; index++) {
				odm_set_bb_reg(p_dm_odm, offset, MASKDWORD, APK_normal_setting_value_1[index]);
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_ap_calibrate_8188e() offset 0x%x value 0x%x\n", offset, odm_get_bb_reg(p_dm_odm, offset, MASKDWORD)));

				offset += 0x04;
			}
			odm_set_bb_reg(p_dm_odm, REG_CONFIG_PMPD_ANT_A, MASKDWORD, 0x12680000);
			phy_set_bb_reg(p_adapter, REG_CONFIG_PMPD_ANT_B, MASKDWORD, 0x12680000);

			offset = REG_CONFIG_ANT_A;
			index = 11;
			for (; index < 13; index++) { /* offset 0xb68, 0xb6c */
				odm_set_bb_reg(p_dm_odm, offset, MASKDWORD, APK_normal_setting_value_1[index]);
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_ap_calibrate_8188e() offset 0x%x value 0x%x\n", offset, odm_get_bb_reg(p_dm_odm, offset, MASKDWORD)));

				offset += 0x04;
			}

			/* page-B1 */
			odm_set_bb_reg(p_dm_odm, REG_FPGA0_IQK, MASKH3BYTES, 0x400000);

			/* path B */
			offset = 0xb60;
			for (index = 0; index < 16; index++) {
				odm_set_bb_reg(p_dm_odm, offset, MASKDWORD, APK_normal_setting_value_2[index]);
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_ap_calibrate_8188e() offset 0x%x value 0x%x\n", offset, odm_get_bb_reg(p_dm_odm, offset, MASKDWORD)));

				offset += 0x04;
			}
			odm_set_bb_reg(p_dm_odm, REG_FPGA0_IQK, MASKH3BYTES, 0);
		}

		/* save RF default value */
		reg_d[path] = phy_query_rf_reg(p_adapter, path, RF_TXBIAS_A, MASKDWORD);

		/* path A AFE all on, path B AFE All off or vise versa */
		for (index = 0; index < IQK_ADDA_REG_NUM ; index++)
			odm_set_bb_reg(p_dm_odm, AFE_REG[index], MASKDWORD, AFE_on_off[path]);
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_ap_calibrate_8188e() offset 0xe70 %x\n", odm_get_bb_reg(p_dm_odm, REG_RX_WAIT_CCA, MASKDWORD)));

		/* BB to AP mode */
		if (path == 0) {
			for (index = 0; index < APK_BB_REG_NUM ; index++) {

				if (index == 0)		/* skip */
					continue;
				else if (index < 5)
					odm_set_bb_reg(p_dm_odm, BB_REG[index], MASKDWORD, BB_AP_MODE[index]);
				else if (BB_REG[index] == 0x870)
					odm_set_bb_reg(p_dm_odm, BB_REG[index], MASKDWORD, BB_backup[index] | BIT(10) | BIT(26));
				else
					odm_set_bb_reg(p_dm_odm, BB_REG[index], BIT(10), 0x0);
			}

			odm_set_bb_reg(p_dm_odm, REG_TX_IQK_TONE_A, MASKDWORD, 0x01008c00);
			odm_set_bb_reg(p_dm_odm, REG_RX_IQK_TONE_A, MASKDWORD, 0x01008c00);
		} else {	/* path B */
			odm_set_bb_reg(p_dm_odm, REG_TX_IQK_TONE_B, MASKDWORD, 0x01008c00);
			odm_set_bb_reg(p_dm_odm, REG_RX_IQK_TONE_B, MASKDWORD, 0x01008c00);

		}

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_ap_calibrate_8188e() offset 0x800 %x\n", odm_get_bb_reg(p_dm_odm, 0x800, MASKDWORD)));

		/* MAC settings */
		_phy_mac_setting_calibration(p_adapter, MAC_REG, MAC_backup);

		if (path == ODM_RF_PATH_A)	/* path B to standby mode */
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, RF_AC, MASKDWORD, 0x10000);
		else {		/* path A to standby mode */
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_AC, MASKDWORD, 0x10000);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_MODE1, MASKDWORD, 0x1000f);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_MODE2, MASKDWORD, 0x20103);
		}

		delta_offset = ((delta + 14) / 2);
		if (delta_offset < 0)
			delta_offset = 0;
		else if (delta_offset > 12)
			delta_offset = 12;

		/* AP calibration */
		for (index = 0; index < APK_BB_REG_NUM; index++) {
			if (index != 1)	/* only DO PA11+PAD01001, AP RF setting */
				continue;

			tmp_reg = APK_RF_init_value[path][index];
			if (!p_dm_odm->rf_calibrate_info.is_apk_thermal_meter_ignore) {
				BB_offset = (tmp_reg & 0xF0000) >> 16;

				if (!(tmp_reg & BIT(15))) /* sign bit 0 */
					BB_offset = -BB_offset;

				delta_V = APK_delta_mapping[index][delta_offset];

				BB_offset += delta_V;

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_ap_calibrate_8188e() APK index %d tmp_reg 0x%x delta_V %d delta_offset %d\n", index, tmp_reg, delta_V, delta_offset));

				if (BB_offset < 0) {
					tmp_reg = tmp_reg & (~BIT(15));
					BB_offset = -BB_offset;
				} else
					tmp_reg = tmp_reg | BIT(15);
				tmp_reg = (tmp_reg & 0xFFF0FFFF) | (BB_offset << 16);
			}
			odm_set_rf_reg(p_dm_odm, (enum odm_rf_radio_path_e)path, RF_IPA_A, MASKDWORD, 0x8992e);
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_ap_calibrate_8188e() offset 0xc %x\n", phy_query_rf_reg(p_adapter, path, RF_IPA_A, MASKDWORD)));
			odm_set_rf_reg(p_dm_odm, (enum odm_rf_radio_path_e)path, RF_AC, MASKDWORD, APK_RF_value_0[path][index]);
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_ap_calibrate_8188e() offset 0x0 %x\n", phy_query_rf_reg(p_adapter, path, RF_AC, MASKDWORD)));
			odm_set_rf_reg(p_dm_odm, (enum odm_rf_radio_path_e)path, RF_TXBIAS_A, MASKDWORD, tmp_reg);
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_ap_calibrate_8188e() offset 0xd %x\n", phy_query_rf_reg(p_adapter, path, RF_TXBIAS_A, MASKDWORD)));

			/* PA11+PAD01111, one shot */
			i = 0;
			do {
				odm_set_bb_reg(p_dm_odm, REG_FPGA0_IQK, MASKH3BYTES, 0x800000);
				{
					odm_set_bb_reg(p_dm_odm, APK_offset[path], MASKDWORD, APK_value[0]);
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_ap_calibrate_8188e() offset 0x%x value 0x%x\n", APK_offset[path], odm_get_bb_reg(p_dm_odm, APK_offset[path], MASKDWORD)));
					ODM_delay_ms(3);
					odm_set_bb_reg(p_dm_odm, APK_offset[path], MASKDWORD, APK_value[1]);
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_ap_calibrate_8188e() offset 0x%x value 0x%x\n", APK_offset[path], odm_get_bb_reg(p_dm_odm, APK_offset[path], MASKDWORD)));

					ODM_delay_ms(20);
				}
				odm_set_bb_reg(p_dm_odm, REG_FPGA0_IQK, MASKH3BYTES, 0x000000);

				if (path == ODM_RF_PATH_A)
					tmp_reg = odm_get_bb_reg(p_dm_odm, REG_APK, 0x03E00000);
				else
					tmp_reg = odm_get_bb_reg(p_dm_odm, REG_APK, 0xF8000000);
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("_phy_ap_calibrate_8188e() offset 0xbd8[25:21] %x\n", tmp_reg));

				i++;
			} while (tmp_reg > apkbound && i < 4);

			APK_result[path][index] = tmp_reg;
		}
	}

	/* reload MAC default value */
	_phy_reload_mac_registers(p_adapter, MAC_REG, MAC_backup);

	/* reload BB default value */
	for (index = 0; index < APK_BB_REG_NUM ; index++) {

		if (index == 0)		/* skip */
			continue;
		odm_set_bb_reg(p_dm_odm, BB_REG[index], MASKDWORD, BB_backup[index]);
	}

	/* reload AFE default value */
	_phy_reload_adda_registers(p_adapter, AFE_REG, AFE_backup, IQK_ADDA_REG_NUM);

	/* reload RF path default value */
	for (path = 0; path < pathbound; path++) {
		odm_set_rf_reg(p_dm_odm, (enum odm_rf_radio_path_e)path, 0xd, MASKDWORD, reg_d[path]);
		if (path == ODM_RF_PATH_B) {
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_MODE1, MASKDWORD, 0x1000f);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, RF_MODE2, MASKDWORD, 0x20101);
		}

		/* note no index == 0 */
		if (APK_result[path][1] > 6)
			APK_result[path][1] = 6;
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("apk path %d result %d 0x%x \t", path, 1, APK_result[path][1]));
	}

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("\n"));

	for (path = 0; path < pathbound; path++) {
		odm_set_rf_reg(p_dm_odm, (enum odm_rf_radio_path_e)path, 0x3, MASKDWORD,
			((APK_result[path][1] << 15) | (APK_result[path][1] << 10) | (APK_result[path][1] << 5) | APK_result[path][1]));
		if (path == ODM_RF_PATH_A)
			odm_set_rf_reg(p_dm_odm, (enum odm_rf_radio_path_e)path, 0x4, MASKDWORD,
				((APK_result[path][1] << 15) | (APK_result[path][1] << 10) | (0x00 << 5) | 0x05));
		else
			odm_set_rf_reg(p_dm_odm, (enum odm_rf_radio_path_e)path, 0x4, MASKDWORD,
				((APK_result[path][1] << 15) | (APK_result[path][1] << 10) | (0x02 << 5) | 0x05));
		odm_set_rf_reg(p_dm_odm, (enum odm_rf_radio_path_e)path, RF_BS_PA_APSET_G9_G11, MASKDWORD,
			((0x08 << 15) | (0x08 << 10) | (0x08 << 5) | 0x08));
	}

	p_dm_odm->rf_calibrate_info.is_ap_kdone = true;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("<==_phy_ap_calibrate_8188e()\n"));
}

#define		DP_BB_REG_NUM		7
#define		DP_RF_REG_NUM		1
#define		DP_RETRY_LIMIT		10
#define		DP_PATH_NUM		2
#define		DP_DPK_NUM			3
#define		DP_DPK_VALUE_NUM	2

void
phy_iq_calibrate_8188e(
	struct _ADAPTER	*p_adapter,
	bool	is_recovery
)
{
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);

	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;

#if (MP_DRIVER == 1)
	PMPT_CONTEXT	p_mpt_ctx = &(p_adapter->mppriv.mpt_ctx);
#endif/* (MP_DRIVER == 1) */

	s32			result[4][8];	/* last is final result */
	u8			i, final_candidate, indexforchannel;
	u8          channel_to_iqk = 7;
	bool			is_patha_ok, is_pathb_ok;
	s32			rege94, rege9c, regea4, regeac, regeb4, regebc, regec4, regecc, reg_tmp = 0;
	bool			is12simular, is13simular, is23simular;
	bool		is_start_cont_tx = false, is_single_tone = false, is_carrier_suppression = false;
	u32			IQK_BB_REG_92C[IQK_BB_REG_NUM] = {
		REG_OFDM_0_XA_RX_IQ_IMBALANCE,	REG_OFDM_0_XB_RX_IQ_IMBALANCE,
		REG_OFDM_0_ECCA_THRESHOLD,	REG_OFDM_0_AGC_RSSI_TABLE,
		REG_OFDM_0_XA_TX_IQ_IMBALANCE,	REG_OFDM_0_XB_TX_IQ_IMBALANCE,
		REG_OFDM_0_XC_TX_AFE,			REG_OFDM_0_XD_TX_AFE,
		REG_OFDM_0_RX_IQ_EXT_ANTA
	};
	u32		start_time;
	s32		progressing_time;

	if (odm_check_power_status(p_adapter) == false)
		return;

	if (!(p_dm_odm->support_ability & ODM_RF_CALIBRATION))
		return;

#if MP_DRIVER == 1
	if (p_dm_odm->mp_mode == true) {
		is_start_cont_tx = p_mpt_ctx->is_start_cont_tx;
		is_single_tone = p_mpt_ctx->is_single_tone;
		is_carrier_suppression = p_mpt_ctx->is_carrier_suppression;
	}
#endif

	/* 20120213<Kordan> Turn on when continuous Tx to pass lab testing. (required by Edlu) */
	if (is_single_tone || is_carrier_suppression)
		return;

#if DISABLE_BB_RF
	return;
#endif

	if (p_dm_odm->rf_calibrate_info.is_iqk_in_progress)
		return;

	if (is_recovery) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_INIT, ODM_DBG_LOUD, ("phy_iq_calibrate_8188e: Return due to is_recovery!\n"));
		_phy_reload_adda_registers(p_adapter, IQK_BB_REG_92C, p_dm_odm->rf_calibrate_info.IQK_BB_backup_recover, 9);
		return;
	}

	odm_acquire_spin_lock(p_dm_odm, RT_IQK_SPINLOCK);
	p_dm_odm->rf_calibrate_info.is_iqk_in_progress = true;
	odm_release_spin_lock(p_dm_odm, RT_IQK_SPINLOCK);

	start_time = odm_get_current_time(p_dm_odm);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK:Start!!!\n"));

	for (i = 0; i < 8; i++) {
		result[0][i] = 0;
		result[1][i] = 0;
		result[2][i] = 0;
		result[3][i] = 0;
	}
	final_candidate = 0xff;
	is_patha_ok = false;
	is_pathb_ok = false;
	is12simular = false;
	is23simular = false;
	is13simular = false;

	for (i = 0; i < 3; i++) {
		/* For 88C 1T1R */
		_phy_iq_calibrate_8188e(p_adapter, result, i, false);

		if (i == 1) {
			is12simular = phy_simularity_compare_8188e(p_adapter, result, 0, 1);
			if (is12simular) {
				final_candidate = 0;
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK: is12simular final_candidate is %x\n", final_candidate));
				break;
			}
		}

		if (i == 2) {
			is13simular = phy_simularity_compare_8188e(p_adapter, result, 0, 2);
			if (is13simular) {
				final_candidate = 0;
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK: is13simular final_candidate is %x\n", final_candidate));

				break;
			}
			is23simular = phy_simularity_compare_8188e(p_adapter, result, 1, 2);
			if (is23simular) {
				final_candidate = 1;
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK: is23simular final_candidate is %x\n", final_candidate));
			} else {
				for (i = 0; i < 8; i++)
					reg_tmp += result[3][i];

				if (reg_tmp != 0)
					final_candidate = 3;
				else
					final_candidate = 0xFF;
			}
		}
	}
	/*	RT_TRACE(COMP_INIT,DBG_LOUD,("Release Mutex in IQCalibrate\n")); */

	for (i = 0; i < 4; i++) {
		rege94 = result[i][0];
		rege9c = result[i][1];
		regea4 = result[i][2];
		regeac = result[i][3];
		regeb4 = result[i][4];
		regebc = result[i][5];
		regec4 = result[i][6];
		regecc = result[i][7];
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK: rege94=%x rege9c=%x regea4=%x regeac=%x regeb4=%x regebc=%x regec4=%x regecc=%x\n ", rege94, rege9c, regea4, regeac, regeb4, regebc, regec4, regecc));
	}

	if (final_candidate != 0xff) {
		p_dm_odm->rf_calibrate_info.rege94 = rege94 = result[final_candidate][0];
		p_dm_odm->rf_calibrate_info.rege9c = rege9c = result[final_candidate][1];
		regea4 = result[final_candidate][2];
		regeac = result[final_candidate][3];
		p_dm_odm->rf_calibrate_info.regeb4 = regeb4 = result[final_candidate][4];
		p_dm_odm->rf_calibrate_info.regebc = regebc = result[final_candidate][5];
		regec4 = result[final_candidate][6];
		regecc = result[final_candidate][7];
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK: final_candidate is %x\n", final_candidate));
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK: rege94=%x rege9c=%x regea4=%x regeac=%x regeb4=%x regebc=%x regec4=%x regecc=%x\n ", rege94, rege9c, regea4, regeac, regeb4, regebc, regec4, regecc));
		is_patha_ok = is_pathb_ok = true;
	} else {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK: FAIL use default value\n"));

		p_dm_odm->rf_calibrate_info.rege94 = p_dm_odm->rf_calibrate_info.regeb4 = 0x100;	/* X default value */
		p_dm_odm->rf_calibrate_info.rege9c = p_dm_odm->rf_calibrate_info.regebc = 0x0;		/* Y default value */
	}

	if ((rege94 != 0)/*&&(regea4 != 0)*/) {
		_phy_path_a_fill_iqk_matrix(p_adapter, is_patha_ok, result, final_candidate, (regea4 == 0));
	}

	indexforchannel = odm_get_right_chnl_place_for_iqk(*p_dm_odm->p_channel);

	/* To Fix BSOD when final_candidate is 0xff
	 * by sherry 20120321 */
	if (final_candidate < 4) {
		for (i = 0; i < iqk_matrix_reg_num; i++)
			p_dm_odm->rf_calibrate_info.iqk_matrix_reg_setting[indexforchannel].value[0][i] = result[final_candidate][i];
		p_dm_odm->rf_calibrate_info.iqk_matrix_reg_setting[indexforchannel].is_iqk_done = true;
	}
	/* RT_DISP(FINIT, INIT_IQK, ("\nIQK OK indexforchannel %d.\n", indexforchannel)); */
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("\nIQK OK indexforchannel %d.\n", indexforchannel));

	_phy_save_adda_registers(p_adapter, IQK_BB_REG_92C, p_dm_odm->rf_calibrate_info.IQK_BB_backup_recover, 9);

	odm_acquire_spin_lock(p_dm_odm, RT_IQK_SPINLOCK);
	p_dm_odm->rf_calibrate_info.is_iqk_in_progress = false;
	odm_release_spin_lock(p_dm_odm, RT_IQK_SPINLOCK);

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK finished\n"));
	progressing_time = odm_get_progressing_time(p_dm_odm, start_time);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("IQK progressing_time = %d\n", progressing_time));

}

void
phy_lc_calibrate_8188e(
	void		*p_dm_void
)
{
	bool		is_start_cont_tx = false, is_single_tone = false, is_carrier_suppression = false;
	u32			timeout = 2000, timecount = 0;
	u32			start_time;
	s32			progressing_time;
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	struct _ADAPTER	*p_adapter = p_dm_odm->adapter;
	/* HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter); */

#if (MP_DRIVER == 1)
	PMPT_CONTEXT	p_mpt_ctx = &(p_adapter->mppriv.mpt_ctx);

	if (p_dm_odm->mp_mode == true) {
		is_start_cont_tx = p_mpt_ctx->is_start_cont_tx;
		is_single_tone = p_mpt_ctx->is_single_tone;
		is_carrier_suppression = p_mpt_ctx->is_carrier_suppression;
	}
#endif

#if DISABLE_BB_RF
	return;
#endif

	if (!(p_dm_odm->support_ability & ODM_RF_CALIBRATION))
		return;
	/* 20120213<Kordan> Turn on when continuous Tx to pass lab testing. (required by Edlu) */
	if (is_single_tone || is_carrier_suppression)
		return;

	start_time = odm_get_current_time(p_dm_odm);
	while (*(p_dm_odm->p_is_scan_in_process) && timecount < timeout) {
		ODM_delay_ms(50);
		timecount += 50;
	}

	p_dm_odm->rf_calibrate_info.is_lck_in_progress = true;

	/* ODM_RT_TRACE(p_dm_odm,ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LCK:Start!!!interface %d currentband %x delay %d ms\n", p_dm_odm->interface_index, p_hal_data->CurrentBandType92D, timecount)); */
	_phy_lc_calibrate_8188e(p_dm_odm, false);

	p_dm_odm->rf_calibrate_info.is_lck_in_progress = false;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LCK:Finish!!!interface %d\n", p_dm_odm->interface_index));
	progressing_time = odm_get_progressing_time(p_dm_odm, start_time);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_CALIBRATION, ODM_DBG_LOUD, ("LCK progressing_time = %d\n", progressing_time));
}

static void _phy_set_rf_path_switch_8188e(
	struct _ADAPTER	*p_adapter,
	bool		is_main,
	bool		is2T
)
{
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &p_hal_data->odmpriv;

	if (!rtw_is_hw_init_completed(p_adapter)) {
		u8	u1b_tmp;
		u1b_tmp = odm_read_1byte(p_dm_odm, REG_LEDCFG2) | BIT(7);
		odm_write_1byte(p_dm_odm, REG_LEDCFG2, u1b_tmp);
		/* odm_set_bb_reg(p_dm_odm, REG_LEDCFG0, BIT23, 0x01); */
		odm_set_bb_reg(p_dm_odm, REG_FPGA0_XAB_RF_PARAMETER, BIT(13), 0x01);
	}

	if (is2T) {	/* 92C */
		if (is_main)
			odm_set_bb_reg(p_dm_odm, REG_FPGA0_XB_RF_INTERFACE_OE, BIT(5) | BIT6, 0x1);	/* 92C_Path_A */
		else
			odm_set_bb_reg(p_dm_odm, REG_FPGA0_XB_RF_INTERFACE_OE, BIT(5) | BIT6, 0x2);	/* BT */
	} else {		/* 88C */

		/* <20120504, Kordan> [8188E] We should make AntDiversity controlled by HW (0x870[9:8] = 0), */
		/* otherwise the following action has no effect. (0x860[9:8] has the effect only if AntDiversity controlled by SW) */
		odm_set_bb_reg(p_dm_odm, REG_FPGA0_XAB_RF_INTERFACE_SW, BIT(8) | BIT(9), 0x0);
		odm_set_bb_reg(p_dm_odm, 0x914, MASKLWORD, 0x0201);		  			  /* Set up the ant mapping table */

		if (is_main) {
			/* odm_set_bb_reg(p_dm_odm, REG_FPGA0_XA_RF_INTERFACE_OE, BIT(8)|BIT9, 0x2);		  */ /* Tx Main (SW control)(The right antenna) */
			/* 4 [ Tx ] */
			odm_set_bb_reg(p_dm_odm, REG_FPGA0_XA_RF_INTERFACE_OE, BIT(14) | BIT13 | BIT12, 0x1); /* Tx Main (HW control)(The right antenna) */

			/* 4 [ Rx ] */
			odm_set_bb_reg(p_dm_odm, REG_FPGA0_XB_RF_INTERFACE_OE, BIT(5) | BIT4 | BIT3, 0x1); /* ant_div_type = TRDiv, right antenna */
			if (p_dm_odm->ant_div_type == CGCS_RX_HW_ANTDIV)
				odm_set_bb_reg(p_dm_odm, REG_CONFIG_RAM64X16, BIT(31), 0x1);				 /* RxCG, Default is RxCG. ant_div_type = 2RDiv, left antenna */

		} else {
			/* odm_set_bb_reg(p_dm_odm, REG_FPGA0_XA_RF_INTERFACE_OE, BIT(8)|BIT9, 0x1);		  */ /* Tx Aux (SW control)(The left antenna) */
			/* 4 [ Tx ] */
			odm_set_bb_reg(p_dm_odm, REG_FPGA0_XA_RF_INTERFACE_OE, BIT(14) | BIT13 | BIT12, 0x0);	 /* Tx Aux (HW control)(The left antenna) */

			/* 4 [ Rx ] */
			odm_set_bb_reg(p_dm_odm, REG_FPGA0_XB_RF_INTERFACE_OE, BIT(5) | BIT4 | BIT3, 0x0); /* ant_div_type = TRDiv, left antenna */
			if (p_dm_odm->ant_div_type == CGCS_RX_HW_ANTDIV)
				odm_set_bb_reg(p_dm_odm, REG_CONFIG_RAM64X16, BIT(31), 0x0);				 /* RxCS, ant_div_type = 2RDiv, right antenna */
		}

	}
}
void phy_set_rf_path_switch_8188e(
	struct _ADAPTER	*p_adapter,
	bool		is_main
)
{
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);

#if DISABLE_BB_RF
	return;
#endif

	{
		/* For 88C 1T1R */
		_phy_set_rf_path_switch_8188e(p_adapter, is_main, false);
	}
}


