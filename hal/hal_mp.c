// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#define _HAL_MP_C_

#include <drv_types.h>

#ifdef CONFIG_MP_INCLUDED

#ifdef RTW_HALMAC
	#include <hal_data.h>		/* struct HAL_DATA_TYPE, RF register definition and etc. */
#else /* !RTW_HALMAC */
	#include <rtl8188e_hal.h>
#endif /* !RTW_HALMAC */


static u8 MgntQuery_NssTxRate(u16 Rate)
{
	u8	NssNum = RF_TX_NUM_NONIMPLEMENT;

	if ((Rate >= MGN_MCS8 && Rate <= MGN_MCS15) ||
	    (Rate >= MGN_VHT2SS_MCS0 && Rate <= MGN_VHT2SS_MCS9))
		NssNum = RF_2TX;
	else if ((Rate >= MGN_MCS16 && Rate <= MGN_MCS23) ||
		 (Rate >= MGN_VHT3SS_MCS0 && Rate <= MGN_VHT3SS_MCS9))
		NssNum = RF_3TX;
	else if ((Rate >= MGN_MCS24 && Rate <= MGN_MCS31) ||
		 (Rate >= MGN_VHT4SS_MCS0 && Rate <= MGN_VHT4SS_MCS9))
		NssNum = RF_4TX;
	else
		NssNum = RF_1TX;

	return NssNum;
}

void hal_mpt_SwitchRfSetting(PADAPTER	pAdapter)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(pAdapter);
	PMPT_CONTEXT		pMptCtx = &(pAdapter->mppriv.mpt_ctx);
	u8				ChannelToSw = pMptCtx->MptChannelToSw;
	u32				ulRateIdx = pMptCtx->mpt_rate_index;
	u32				ulbandwidth = pMptCtx->MptBandWidth;

	/* <20120525, Kordan> Dynamic mechanism for APK, asked by Dennis.*/
	if (IS_HARDWARE_TYPE_8188E(pAdapter)) {
		phy_set_rf_reg(pAdapter, ODM_RF_PATH_A, RF_0x52, 0x000F0, pMptCtx->backup0x52_RF_A);
		phy_set_rf_reg(pAdapter, ODM_RF_PATH_B, RF_0x52, 0x000F0, pMptCtx->backup0x52_RF_B);
	}
}

s32 hal_mpt_SetPowerTracking(PADAPTER padapter, u8 enable)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct PHY_DM_STRUCT		*pDM_Odm = &(pHalData->odmpriv);


	if (!netif_running(padapter->pnetdev)) {
		return _FAIL;
	}

	if (check_fwstate(&padapter->mlmepriv, WIFI_MP_STATE) == false) {
		return _FAIL;
	}
	if (enable)
		pDM_Odm->rf_calibrate_info.txpowertrack_control = true;
	else
		pDM_Odm->rf_calibrate_info.txpowertrack_control = false;

	return _SUCCESS;
}

void hal_mpt_GetPowerTracking(PADAPTER padapter, u8 *enable)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct PHY_DM_STRUCT		*pDM_Odm = &(pHalData->odmpriv);


	*enable = pDM_Odm->rf_calibrate_info.txpowertrack_control;
}


void hal_mpt_CCKTxPowerAdjust(PADAPTER Adapter, bool bInCH14)
{
	u32		TempVal = 0, TempVal2 = 0, TempVal3 = 0;
	u32		CurrCCKSwingVal = 0, CCKSwingIndex = 12;
	u8		i;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	PMPT_CONTEXT		pMptCtx = &(Adapter->mppriv.mpt_ctx);
	u8				u1Channel = pHalData->current_channel;
	u32				ulRateIdx = pMptCtx->mpt_rate_index;
	u8				DataRate = 0xFF;

	DataRate = mpt_to_mgnt_rate(ulRateIdx);

	if (u1Channel == 14 && IS_CCK_RATE(DataRate))
		pHalData->bCCKinCH14 = true;
	else
		pHalData->bCCKinCH14 = false;

	/* get current cck swing value and check 0xa22 & 0xa23 later to match the table.*/
	CurrCCKSwingVal = read_bbreg(Adapter, rCCK0_TxFilter1, bMaskHWord);

	if (!pHalData->bCCKinCH14) {
		/* Readback the current bb cck swing value and compare with the table to */
		/* get the current swing index */
		for (i = 0; i < CCK_TABLE_SIZE; i++) {
			if (((CurrCCKSwingVal & 0xff) == (u32)cck_swing_table_ch1_ch13[i][0]) &&
			    (((CurrCCKSwingVal & 0xff00) >> 8) == (u32)cck_swing_table_ch1_ch13[i][1])) {
				CCKSwingIndex = i;
				break;
			}
		}

		/*Write 0xa22 0xa23*/
		TempVal = cck_swing_table_ch1_ch13[CCKSwingIndex][0] +
			(cck_swing_table_ch1_ch13[CCKSwingIndex][1] << 8);


		/*Write 0xa24 ~ 0xa27*/
		TempVal2 = 0;
		TempVal2 = cck_swing_table_ch1_ch13[CCKSwingIndex][2] +
			(cck_swing_table_ch1_ch13[CCKSwingIndex][3] << 8) +
			(cck_swing_table_ch1_ch13[CCKSwingIndex][4] << 16) +
			(cck_swing_table_ch1_ch13[CCKSwingIndex][5] << 24);

		/*Write 0xa28  0xa29*/
		TempVal3 = 0;
		TempVal3 = cck_swing_table_ch1_ch13[CCKSwingIndex][6] +
			(cck_swing_table_ch1_ch13[CCKSwingIndex][7] << 8);
	}  else {
		for (i = 0; i < CCK_TABLE_SIZE; i++) {
			if (((CurrCCKSwingVal & 0xff) == (u32)cck_swing_table_ch14[i][0]) &&
			    (((CurrCCKSwingVal & 0xff00) >> 8) == (u32)cck_swing_table_ch14[i][1])) {
				CCKSwingIndex = i;
				break;
			}
		}

		/*Write 0xa22 0xa23*/
		TempVal = cck_swing_table_ch14[CCKSwingIndex][0] +
			  (cck_swing_table_ch14[CCKSwingIndex][1] << 8);

		/*Write 0xa24 ~ 0xa27*/
		TempVal2 = 0;
		TempVal2 = cck_swing_table_ch14[CCKSwingIndex][2] +
			   (cck_swing_table_ch14[CCKSwingIndex][3] << 8) +
			(cck_swing_table_ch14[CCKSwingIndex][4] << 16) +
			   (cck_swing_table_ch14[CCKSwingIndex][5] << 24);

		/*Write 0xa28  0xa29*/
		TempVal3 = 0;
		TempVal3 = cck_swing_table_ch14[CCKSwingIndex][6] +
			   (cck_swing_table_ch14[CCKSwingIndex][7] << 8);
	}

	write_bbreg(Adapter, rCCK0_TxFilter1, bMaskHWord, TempVal);
	write_bbreg(Adapter, rCCK0_TxFilter2, bMaskDWord, TempVal2);
	write_bbreg(Adapter, rCCK0_DebugPort, bMaskLWord, TempVal3);
}

void hal_mpt_SetChannel(PADAPTER pAdapter)
{
	u8 eRFPath;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	struct PHY_DM_STRUCT		*pDM_Odm = &(pHalData->odmpriv);
	struct mp_priv	*pmp = &pAdapter->mppriv;
	u8		channel = pmp->channel;
	u8		bandwidth = pmp->bandwidth;

	hal_mpt_SwitchRfSetting(pAdapter);

	pHalData->bSwChnl = true;
	pHalData->bSetChnlBW = true;
	rtw_hal_set_chnl_bw(pAdapter, channel, bandwidth, 0, 0);

	hal_mpt_CCKTxPowerAdjust(pAdapter, pHalData->bCCKinCH14);

}

/*
 * Notice
 *	Switch bandwitdth may change center frequency(channel)
 */
void hal_mpt_SetBandwidth(PADAPTER pAdapter)
{
	struct mp_priv *pmp = &pAdapter->mppriv;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	u8		channel = pmp->channel;
	u8		bandwidth = pmp->bandwidth;

	pHalData->bSwChnl = true;
	pHalData->bSetChnlBW = true;
	rtw_hal_set_chnl_bw(pAdapter, channel, bandwidth, 0, 0);

	hal_mpt_SwitchRfSetting(pAdapter);
}

static void mpt_SetTxPower_Old(PADAPTER pAdapter, MPT_TXPWR_DEF Rate, u8 *pTxPower)
{
	switch (Rate) {
	case MPT_CCK: {
		u32	TxAGC = 0, pwr = 0;
		u8	rf;

		pwr = pTxPower[ODM_RF_PATH_A];
		if (pwr < 0x3f) {
			TxAGC = (pwr << 16) | (pwr << 8) | (pwr);
			phy_set_bb_reg(pAdapter, rTxAGC_A_CCK1_Mcs32, bMaskByte1, pTxPower[ODM_RF_PATH_A]);
			phy_set_bb_reg(pAdapter, rTxAGC_B_CCK11_A_CCK2_11, 0xffffff00, TxAGC);
		}
		pwr = pTxPower[ODM_RF_PATH_B];
		if (pwr < 0x3f) {
			TxAGC = (pwr << 16) | (pwr << 8) | (pwr);
			phy_set_bb_reg(pAdapter, rTxAGC_B_CCK11_A_CCK2_11, bMaskByte0, pTxPower[ODM_RF_PATH_B]);
			phy_set_bb_reg(pAdapter, rTxAGC_B_CCK1_55_Mcs32, 0xffffff00, TxAGC);
		}
	}
	break;

	case MPT_OFDM_AND_HT: {
		u32	TxAGC = 0;
		u8	pwr = 0, rf;

		pwr = pTxPower[0];
		if (pwr < 0x3f) {
			TxAGC |= ((pwr << 24) | (pwr << 16) | (pwr << 8) | pwr);
			RTW_INFO("HT Tx-rf(A) Power = 0x%x\n", TxAGC);
			phy_set_bb_reg(pAdapter, rTxAGC_A_Rate18_06, bMaskDWord, TxAGC);
			phy_set_bb_reg(pAdapter, rTxAGC_A_Rate54_24, bMaskDWord, TxAGC);
			phy_set_bb_reg(pAdapter, rTxAGC_A_Mcs03_Mcs00, bMaskDWord, TxAGC);
			phy_set_bb_reg(pAdapter, rTxAGC_A_Mcs07_Mcs04, bMaskDWord, TxAGC);
			phy_set_bb_reg(pAdapter, rTxAGC_A_Mcs11_Mcs08, bMaskDWord, TxAGC);
			phy_set_bb_reg(pAdapter, rTxAGC_A_Mcs15_Mcs12, bMaskDWord, TxAGC);
		}
		TxAGC = 0;
		pwr = pTxPower[1];
		if (pwr < 0x3f) {
			TxAGC |= ((pwr << 24) | (pwr << 16) | (pwr << 8) | pwr);
			RTW_INFO("HT Tx-rf(B) Power = 0x%x\n", TxAGC);
			phy_set_bb_reg(pAdapter, rTxAGC_B_Rate18_06, bMaskDWord, TxAGC);
			phy_set_bb_reg(pAdapter, rTxAGC_B_Rate54_24, bMaskDWord, TxAGC);
			phy_set_bb_reg(pAdapter, rTxAGC_B_Mcs03_Mcs00, bMaskDWord, TxAGC);
			phy_set_bb_reg(pAdapter, rTxAGC_B_Mcs07_Mcs04, bMaskDWord, TxAGC);
			phy_set_bb_reg(pAdapter, rTxAGC_B_Mcs11_Mcs08, bMaskDWord, TxAGC);
			phy_set_bb_reg(pAdapter, rTxAGC_B_Mcs15_Mcs12, bMaskDWord, TxAGC);
		}
	}
	break;

	default:
		break;
	}
	RTW_INFO("<===mpt_SetTxPower_Old()\n");
}

static void
mpt_SetTxPower(
	PADAPTER		pAdapter,
	MPT_TXPWR_DEF	Rate,
	u8 *	pTxPower
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	u8 path = 0 , i = 0, MaxRate = MGN_6M;
	u8 StartPath = ODM_RF_PATH_A, EndPath = ODM_RF_PATH_B;

	switch (Rate) {
	case MPT_CCK: {
		u8 rate[] = {MGN_1M, MGN_2M, MGN_5_5M, MGN_11M};

		for (path = StartPath; path <= EndPath; path++)
			for (i = 0; i < sizeof(rate); ++i)
				PHY_SetTxPowerIndex(pAdapter, pTxPower[path], path, rate[i]);
	}
	break;
	case MPT_OFDM: {
		u8 rate[] = {
			MGN_6M, MGN_9M, MGN_12M, MGN_18M,
			MGN_24M, MGN_36M, MGN_48M, MGN_54M,
		};

		for (path = StartPath; path <= EndPath; path++)
			for (i = 0; i < sizeof(rate); ++i)
				PHY_SetTxPowerIndex(pAdapter, pTxPower[path], path, rate[i]);
	}
	break;
	case MPT_HT: {
		u8 rate[] = {
			MGN_MCS0, MGN_MCS1, MGN_MCS2, MGN_MCS3, MGN_MCS4,
			MGN_MCS5, MGN_MCS6, MGN_MCS7, MGN_MCS8, MGN_MCS9,
			MGN_MCS10, MGN_MCS11, MGN_MCS12, MGN_MCS13, MGN_MCS14,
			MGN_MCS15, MGN_MCS16, MGN_MCS17, MGN_MCS18, MGN_MCS19,
			MGN_MCS20, MGN_MCS21, MGN_MCS22, MGN_MCS23, MGN_MCS24,
			MGN_MCS25, MGN_MCS26, MGN_MCS27, MGN_MCS28, MGN_MCS29,
			MGN_MCS30, MGN_MCS31,
		};
		if (pHalData->rf_type == RF_3T3R)
			MaxRate = MGN_MCS23;
		else if (pHalData->rf_type == RF_2T2R)
			MaxRate = MGN_MCS15;
		else
			MaxRate = MGN_MCS7;
		for (path = StartPath; path <= EndPath; path++) {
			for (i = 0; i < sizeof(rate); ++i) {
				if (rate[i] > MaxRate)
					break;
				PHY_SetTxPowerIndex(pAdapter, pTxPower[path], path, rate[i]);
			}
		}
	}
	break;
	case MPT_VHT: {
		u8 rate[] = {
			MGN_VHT1SS_MCS0, MGN_VHT1SS_MCS1, MGN_VHT1SS_MCS2, MGN_VHT1SS_MCS3, MGN_VHT1SS_MCS4,
			MGN_VHT1SS_MCS5, MGN_VHT1SS_MCS6, MGN_VHT1SS_MCS7, MGN_VHT1SS_MCS8, MGN_VHT1SS_MCS9,
			MGN_VHT2SS_MCS0, MGN_VHT2SS_MCS1, MGN_VHT2SS_MCS2, MGN_VHT2SS_MCS3, MGN_VHT2SS_MCS4,
			MGN_VHT2SS_MCS5, MGN_VHT2SS_MCS6, MGN_VHT2SS_MCS7, MGN_VHT2SS_MCS8, MGN_VHT2SS_MCS9,
			MGN_VHT3SS_MCS0, MGN_VHT3SS_MCS1, MGN_VHT3SS_MCS2, MGN_VHT3SS_MCS3, MGN_VHT3SS_MCS4,
			MGN_VHT3SS_MCS5, MGN_VHT3SS_MCS6, MGN_VHT3SS_MCS7, MGN_VHT3SS_MCS8, MGN_VHT3SS_MCS9,
			MGN_VHT4SS_MCS0, MGN_VHT4SS_MCS1, MGN_VHT4SS_MCS2, MGN_VHT4SS_MCS3, MGN_VHT4SS_MCS4,
			MGN_VHT4SS_MCS5, MGN_VHT4SS_MCS6, MGN_VHT4SS_MCS7, MGN_VHT4SS_MCS8, MGN_VHT4SS_MCS9,
		};
		if (pHalData->rf_type == RF_3T3R)
			MaxRate = MGN_VHT3SS_MCS9;
		else if (pHalData->rf_type == RF_2T2R || pHalData->rf_type == RF_2T4R)
			MaxRate = MGN_VHT2SS_MCS9;
		else
			MaxRate = MGN_VHT1SS_MCS9;

		for (path = StartPath; path <= EndPath; path++) {
			for (i = 0; i < sizeof(rate); ++i) {
				if (rate[i] > MaxRate)
					break;
				PHY_SetTxPowerIndex(pAdapter, pTxPower[path], path, rate[i]);
			}
		}
	}
	break;
	default:
		RTW_INFO("<===mpt_SetTxPower: Illegal channel!!\n");
		break;
	}
}

void hal_mpt_SetTxPower(PADAPTER pAdapter)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);
	PMPT_CONTEXT		pMptCtx = &(pAdapter->mppriv.mpt_ctx);
	struct PHY_DM_STRUCT		*pDM_Odm = &pHalData->odmpriv;

	if (pHalData->rf_chip < RF_TYPE_MAX) {
		if (IS_HARDWARE_TYPE_8188E(pAdapter)) {
			u8 path = (pHalData->antenna_tx_path == ANTENNA_A) ? (ODM_RF_PATH_A) : (ODM_RF_PATH_B);

			RTW_INFO("===> MPT_ProSetTxPower: Old\n");

			mpt_SetTxPower_Old(pAdapter, MPT_CCK, pMptCtx->TxPwrLevel);
			mpt_SetTxPower_Old(pAdapter, MPT_OFDM_AND_HT, pMptCtx->TxPwrLevel);

		} else {
			RTW_INFO("===> MPT_ProSetTxPower: Jaguar/Jaguar2\n");
			mpt_SetTxPower(pAdapter, MPT_CCK, pMptCtx->TxPwrLevel);
			mpt_SetTxPower(pAdapter, MPT_OFDM, pMptCtx->TxPwrLevel);
			mpt_SetTxPower(pAdapter, MPT_HT, pMptCtx->TxPwrLevel);
			mpt_SetTxPower(pAdapter, MPT_VHT, pMptCtx->TxPwrLevel);
		}
	} else
		RTW_INFO("RFChipID < RF_TYPE_MAX, the RF chip is not supported - %d\n", pHalData->rf_chip);

	odm_clear_txpowertracking_state(pDM_Odm);
}

void hal_mpt_SetDataRate(PADAPTER pAdapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	PMPT_CONTEXT		pMptCtx = &(pAdapter->mppriv.mpt_ctx);
	u32 DataRate;

	DataRate = mpt_to_mgnt_rate(pMptCtx->mpt_rate_index);

	hal_mpt_SwitchRfSetting(pAdapter);

	hal_mpt_CCKTxPowerAdjust(pAdapter, pHalData->bCCKinCH14);
}

#define RF_PATH_AB	22

static void mpt_SetRFPath_819X(PADAPTER	pAdapter)
{
	HAL_DATA_TYPE			*pHalData	= GET_HAL_DATA(pAdapter);
	PMPT_CONTEXT		pMptCtx = &(pAdapter->mppriv.mpt_ctx);
	u32			ulAntennaTx, ulAntennaRx;
	R_ANTENNA_SELECT_OFDM	*p_ofdm_tx;	/* OFDM Tx register */
	R_ANTENNA_SELECT_CCK	*p_cck_txrx;
	u8		r_rx_antenna_ofdm = 0, r_ant_select_cck_val = 0;
	u8		chgTx = 0, chgRx = 0;
	u32		r_ant_sel_cck_val = 0, r_ant_select_ofdm_val = 0, r_ofdm_tx_en_val = 0;

	ulAntennaTx = pHalData->antenna_tx_path;
	ulAntennaRx = pHalData->AntennaRxPath;

	p_ofdm_tx = (R_ANTENNA_SELECT_OFDM *)&r_ant_select_ofdm_val;
	p_cck_txrx = (R_ANTENNA_SELECT_CCK *)&r_ant_select_cck_val;

	p_ofdm_tx->r_ant_ht1			= 0x1;
	p_ofdm_tx->r_ant_ht2			= 0x2;/*Second TX RF path is A*/
	p_ofdm_tx->r_ant_non_ht			= 0x3;/*/ 0x1+0x2=0x3 */

	switch (ulAntennaTx) {
	case ANTENNA_A:
		p_ofdm_tx->r_tx_antenna		= 0x1;
		r_ofdm_tx_en_val		= 0x1;
		p_ofdm_tx->r_ant_l		= 0x1;
		p_ofdm_tx->r_ant_ht_s1		= 0x1;
		p_ofdm_tx->r_ant_non_ht_s1	= 0x1;
		p_cck_txrx->r_ccktx_enable	= 0x8;
		chgTx = 1;
		/*/ From SD3 Willis suggestion !!! Set RF A=TX and B as standby*/
		{
			phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter2, 0xe, 2);
			phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter2, 0xe, 1);
			r_ofdm_tx_en_val			= 0x3;
			/*/ Power save*/
			/*/cosa r_ant_select_ofdm_val = 0x11111111;*/
			/*/ We need to close RFB by SW control*/
			if (pHalData->rf_type == RF_2T2R) {
				phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFInterfaceSW, BIT10, 0);
				phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFInterfaceSW, BIT26, 1);
				phy_set_bb_reg(pAdapter, rFPGA0_XB_RFInterfaceOE, BIT10, 0);
				phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFParameter, BIT1, 1);
				phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFParameter, BIT17, 0);
			}
		}
		pMptCtx->mpt_rf_path = ODM_RF_PATH_A;
		break;
	case ANTENNA_B:
		p_ofdm_tx->r_tx_antenna		= 0x2;
		r_ofdm_tx_en_val		= 0x2;
		p_ofdm_tx->r_ant_l		= 0x2;
		p_ofdm_tx->r_ant_ht_s1		= 0x2;
		p_ofdm_tx->r_ant_non_ht_s1	= 0x2;
		p_cck_txrx->r_ccktx_enable	= 0x4;
		chgTx = 1;
		/*/ From SD3 Willis suggestion !!! Set RF A as standby*/
		{
			phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter2, 0xe, 1);
			phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter2, 0xe, 2);

			/*/ 2008/10/31 MH From SD3 Willi's suggestion. We must read RF 1T table.*/
			/*/ 2009/01/08 MH From Sd3 Willis. We need to close RFA by SW control*/
			if (pHalData->rf_type == RF_2T2R || pHalData->rf_type == RF_1T2R) {
				phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFInterfaceSW, BIT10, 1);
				phy_set_bb_reg(pAdapter, rFPGA0_XA_RFInterfaceOE, BIT10, 0);
				phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFInterfaceSW, BIT26, 0);
				/*/phy_set_bb_reg(pAdapter, rFPGA0_XB_RFInterfaceOE, BIT10, 0);*/
				phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFParameter, BIT1, 0);
				phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFParameter, BIT17, 1);
			}
		}
		pMptCtx->mpt_rf_path = ODM_RF_PATH_B;
		break;
	case ANTENNA_AB:/*/ For 8192S*/
		p_ofdm_tx->r_tx_antenna		= 0x3;
		r_ofdm_tx_en_val		= 0x3;
		p_ofdm_tx->r_ant_l		= 0x3;
		p_ofdm_tx->r_ant_ht_s1		= 0x3;
		p_ofdm_tx->r_ant_non_ht_s1	= 0x3;
		p_cck_txrx->r_ccktx_enable	= 0xC;
		chgTx = 1;
		/*/ From SD3Willis suggestion !!! Set RF B as standby*/
		{
			phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter2, 0xe, 2);
			phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter2, 0xe, 2);
			/* Disable Power save*/
			/*cosa r_ant_select_ofdm_val = 0x3321333;*/
			/* 2009/01/08 MH From Sd3 Willis. We need to enable RFA/B by SW control*/
			if (pHalData->rf_type == RF_2T2R) {
				phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFInterfaceSW, BIT10, 0);

				phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFInterfaceSW, BIT26, 0);
				/*/phy_set_bb_reg(pAdapter, rFPGA0_XB_RFInterfaceOE, BIT10, 0);*/
				phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFParameter, BIT1, 1);
				phy_set_bb_reg(pAdapter, rFPGA0_XAB_RFParameter, BIT17, 1);
			}
		}
		pMptCtx->mpt_rf_path = ODM_RF_PATH_AB;
		break;
	default:
		break;
	}
	switch (ulAntennaRx) {
	case ANTENNA_A:
		r_rx_antenna_ofdm		= 0x1;	/* A*/
		p_cck_txrx->r_cckrx_enable	= 0x0;	/* default: A*/
		p_cck_txrx->r_cckrx_enable_2	= 0x0;	/* option: A*/
		chgRx = 1;
		break;
	case ANTENNA_B:
		r_rx_antenna_ofdm			= 0x2;	/*/ B*/
		p_cck_txrx->r_cckrx_enable	= 0x1;	/*/ default: B*/
		p_cck_txrx->r_cckrx_enable_2	= 0x1;	/*/ option: B*/
		chgRx = 1;
		break;
	case ANTENNA_AB:/*/ For 8192S and 8192E/U...*/
		r_rx_antenna_ofdm		= 0x3;/*/ AB*/
		p_cck_txrx->r_cckrx_enable	= 0x0;/*/ default:A*/
		p_cck_txrx->r_cckrx_enable_2	= 0x1;/*/ option:B*/
		chgRx = 1;
		break;
	default:
		break;
	}


	if (chgTx && chgRx) {
		switch (pHalData->rf_chip) {
		case RF_8225:
		case RF_8256:
		case RF_6052:
			/*/r_ant_sel_cck_val = r_ant_select_cck_val;*/
			phy_set_bb_reg(pAdapter, rFPGA1_TxInfo, 0x7fffffff, r_ant_select_ofdm_val);		/*/OFDM Tx*/
			phy_set_bb_reg(pAdapter, rFPGA0_TxInfo, 0x0000000f, r_ofdm_tx_en_val);		/*/OFDM Tx*/
			phy_set_bb_reg(pAdapter, rOFDM0_TRxPathEnable, 0x0000000f, r_rx_antenna_ofdm);	/*/OFDM Rx*/
			phy_set_bb_reg(pAdapter, rOFDM1_TRxPathEnable, 0x0000000f, r_rx_antenna_ofdm);	/*/OFDM Rx*/
			phy_set_bb_reg(pAdapter, rCCK0_AFESetting, bMaskByte3, r_ant_select_cck_val);/*/r_ant_sel_cck_val); /CCK TxRx*/
			break;

		default:
			RTW_INFO("Unsupported RFChipID for switching antenna.\n");
			break;
		}
	}
}	/* MPT_ProSetRFPath */


void hal_mpt_SetAntenna(PADAPTER	pAdapter)

{
	RTW_INFO("Do %s\n", __func__);
	mpt_SetRFPath_819X(pAdapter);
	RTW_INFO("mpt_SetRFPath_819X Do %s\n", __func__);
}

s32 hal_mpt_SetThermalMeter(PADAPTER pAdapter, u8 target_ther)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);

	if (!netif_running(pAdapter->pnetdev)) {
		return _FAIL;
	}


	if (check_fwstate(&pAdapter->mlmepriv, WIFI_MP_STATE) == false) {
		return _FAIL;
	}


	target_ther &= 0xff;
	if (target_ther < 0x07)
		target_ther = 0x07;
	else if (target_ther > 0x1d)
		target_ther = 0x1d;

	pHalData->eeprom_thermal_meter = target_ther;

	return _SUCCESS;
}


void hal_mpt_TriggerRFThermalMeter(PADAPTER pAdapter)
{
	phy_set_rf_reg(pAdapter, ODM_RF_PATH_A, 0x42, BIT17 | BIT16, 0x03);

}


u8 hal_mpt_ReadRFThermalMeter(PADAPTER pAdapter)

{
	u32 ThermalValue = 0;

	ThermalValue = (u8)phy_query_rf_reg(pAdapter, ODM_RF_PATH_A, 0x42, 0xfc00);	/*0x42: RF Reg[15:10]*/
	return (u8)ThermalValue;

}

void hal_mpt_GetThermalMeter(PADAPTER pAdapter, u8 *value)
{
	hal_mpt_TriggerRFThermalMeter(pAdapter);
	rtw_msleep_os(1000);
	*value = hal_mpt_ReadRFThermalMeter(pAdapter);
}

void hal_mpt_SetSingleCarrierTx(PADAPTER pAdapter, u8 bStart)
{
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(pAdapter);

	pAdapter->mppriv.mpt_ctx.bSingleCarrier = bStart;

	if (bStart) {/*/ Start Single Carrier.*/
		/*/ Start Single Carrier.*/
		/*/ 1. if OFDM block on?*/
		if (!phy_query_bb_reg(pAdapter, rFPGA0_RFMOD, bOFDMEn))
			phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bOFDMEn, 1); /*set OFDM block on*/

		/*/ 2. set CCK test mode off, set to CCK normal mode*/
		phy_set_bb_reg(pAdapter, rCCK0_System, bCCKBBMode, 0);

		/*/ 3. turn on scramble setting*/
		phy_set_bb_reg(pAdapter, rCCK0_System, bCCKScramble, 1);

		/*/ 4. Turn On Continue Tx and turn off the other test modes.*/
		phy_set_bb_reg(pAdapter, rOFDM1_LSTF, BIT30 | BIT29 | BIT28, OFDM_SingleCarrier);
	} else {
		/*/ Stop Single Carrier.*/
		/*/ Stop Single Carrier.*/
		/*/ Turn off all test modes.*/
		phy_set_bb_reg(pAdapter, rOFDM1_LSTF, BIT30 | BIT29 | BIT28, OFDM_ALL_OFF);

		rtw_msleep_os(10);
		/*/BB Reset*/
		phy_set_bb_reg(pAdapter, rPMAC_Reset, bBBResetB, 0x0);
		phy_set_bb_reg(pAdapter, rPMAC_Reset, bBBResetB, 0x1);
	}
}


void hal_mpt_SetSingleToneTx(PADAPTER pAdapter, u8 bStart)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);
	PMPT_CONTEXT		pMptCtx = &(pAdapter->mppriv.mpt_ctx);
	u32			ulAntennaTx = pHalData->antenna_tx_path;
	static u32		regRF = 0, regBB0 = 0, regBB1 = 0, regBB2 = 0, regBB3 = 0;
	u8 rfPath;

	switch (ulAntennaTx) {
	case ANTENNA_B:
		rfPath = ODM_RF_PATH_B;
		break;
	case ANTENNA_C:
		rfPath = ODM_RF_PATH_C;
		break;
	case ANTENNA_D:
		rfPath = ODM_RF_PATH_D;
		break;
	case ANTENNA_A:
	default:
		rfPath = ODM_RF_PATH_A;
		break;
	}

	pAdapter->mppriv.mpt_ctx.is_single_tone = bStart;
	if (bStart) {
		/*/ Start Single Tone.*/
		/*/ <20120326, Kordan> To amplify the power of tone for Xtal calibration. (asked by Edlu)*/
		if (IS_HARDWARE_TYPE_8188E(pAdapter)) {
			regRF = phy_query_rf_reg(pAdapter, rfPath, lna_low_gain_3, bRFRegOffsetMask);
			phy_set_rf_reg(pAdapter, ODM_RF_PATH_A, lna_low_gain_3, BIT1, 0x1); /*/ RF LO enabled*/
			phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bCCKEn, 0x0);
			phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bOFDMEn, 0x0);
		} else {	/*/ Turn On SingleTone and turn off the other test modes.*/
			phy_set_bb_reg(pAdapter, rOFDM1_LSTF, BIT30 | BIT29 | BIT28, OFDM_SingleTone);
		}
		write_bbreg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000500);
		write_bbreg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000500);
	} else {/*/ Stop Single Ton e.*/
		if (IS_HARDWARE_TYPE_8188E(pAdapter)) {
			phy_set_rf_reg(pAdapter, ODM_RF_PATH_A, lna_low_gain_3, bRFRegOffsetMask, regRF);
			phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bCCKEn, 0x1);
			phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bOFDMEn, 0x1);
		}
		write_bbreg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000100);
		write_bbreg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000100);
	}
}

void hal_mpt_SetCarrierSuppressionTx(PADAPTER pAdapter, u8 bStart)
{
	u8 Rate;

	pAdapter->mppriv.mpt_ctx.is_carrier_suppression = bStart;

	Rate = HwRateToMPTRate(pAdapter->mppriv.rateidx);
	if (bStart) {/* Start Carrier Suppression.*/
		if (Rate <= MPT_RATE_11M) {
			/*/ 1. if CCK block on?*/
			if (!read_bbreg(pAdapter, rFPGA0_RFMOD, bCCKEn))
				write_bbreg(pAdapter, rFPGA0_RFMOD, bCCKEn, bEnable);/*set CCK block on*/

			/*/Turn Off All Test Mode*/
			phy_set_bb_reg(pAdapter, rOFDM1_LSTF, BIT30 | BIT29 | BIT28, OFDM_ALL_OFF);

			write_bbreg(pAdapter, rCCK0_System, bCCKBBMode, 0x2);    /*/transmit mode*/
			write_bbreg(pAdapter, rCCK0_System, bCCKScramble, 0x0);  /*/turn off scramble setting*/

			/*/Set CCK Tx Test Rate*/
			write_bbreg(pAdapter, rCCK0_System, bCCKTxRate, 0x0);    /*/Set FTxRate to 1Mbps*/
		}

		/*Set for dynamic set Power index*/
		write_bbreg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000500);
		write_bbreg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000500);

	} else {/* Stop Carrier Suppression.*/

		if (Rate <= MPT_RATE_11M) {
			write_bbreg(pAdapter, rCCK0_System, bCCKBBMode, 0x0);    /*normal mode*/
			write_bbreg(pAdapter, rCCK0_System, bCCKScramble, 0x1);  /*turn on scramble setting*/

			/*BB Reset*/
			write_bbreg(pAdapter, rPMAC_Reset, bBBResetB, 0x0);
			write_bbreg(pAdapter, rPMAC_Reset, bBBResetB, 0x1);
		}
		/*Stop for dynamic set Power index*/
		write_bbreg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000100);
		write_bbreg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000100);
	}
	RTW_INFO("\n MPT_ProSetCarrierSupp() is finished.\n");
}

u32 hal_mpt_query_phytxok(PADAPTER	pAdapter)
{
	PMPT_CONTEXT pMptCtx = &(pAdapter->mppriv.mpt_ctx);
	RT_PMAC_TX_INFO PMacTxInfo = pMptCtx->PMacTxInfo;
	u16 count = 0;

	if (IS_MPT_CCK_RATE(PMacTxInfo.TX_RATE))
		count = phy_query_bb_reg(pAdapter, 0xF50, bMaskLWord); /* [15:0]*/
	else
		count = phy_query_bb_reg(pAdapter, 0xF50, bMaskHWord); /* [31:16]*/

	if (count > 50000) {
		rtw_reset_phy_trx_ok_counters(pAdapter);
		pAdapter->mppriv.tx.sended += count;
		count = 0;
	}

	return pAdapter->mppriv.tx.sended + count;

}

static	void mpt_StopCckContTx(
	PADAPTER	pAdapter
)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(pAdapter);
	PMPT_CONTEXT	pMptCtx = &(pAdapter->mppriv.mpt_ctx);
	u8			u1bReg;

	pMptCtx->bCckContTx = false;
	pMptCtx->bOfdmContTx = false;

	phy_set_bb_reg(pAdapter, rCCK0_System, bCCKBBMode, 0x0);	/*normal mode*/
	phy_set_bb_reg(pAdapter, rCCK0_System, bCCKScramble, 0x1);	/*turn on scramble setting*/

	/*BB Reset*/
	phy_set_bb_reg(pAdapter, rPMAC_Reset, bBBResetB, 0x0);
	phy_set_bb_reg(pAdapter, rPMAC_Reset, bBBResetB, 0x1);

	phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000100);
	phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000100);

}	/* mpt_StopCckContTx */


static	void mpt_StopOfdmContTx(
	PADAPTER	pAdapter
)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(pAdapter);
	PMPT_CONTEXT	pMptCtx = &(pAdapter->mppriv.mpt_ctx);
	u8			u1bReg;
	u32			data;

	pMptCtx->bCckContTx = false;
	pMptCtx->bOfdmContTx = false;

	phy_set_bb_reg(pAdapter, rOFDM1_LSTF, BIT30 | BIT29 | BIT28, OFDM_ALL_OFF);

	rtw_mdelay_os(10);

	/*BB Reset*/
	phy_set_bb_reg(pAdapter, rPMAC_Reset, bBBResetB, 0x0);
	phy_set_bb_reg(pAdapter, rPMAC_Reset, bBBResetB, 0x1);

	phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000100);
	phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000100);
}	/* mpt_StopOfdmContTx */


static	void mpt_StartCckContTx(
	PADAPTER		pAdapter
)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(pAdapter);
	PMPT_CONTEXT	pMptCtx = &(pAdapter->mppriv.mpt_ctx);
	u32			cckrate;

	/* 1. if CCK block on */
	if (!phy_query_bb_reg(pAdapter, rFPGA0_RFMOD, bCCKEn))
		phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bCCKEn, 1);/*set CCK block on*/

	/*Turn Off All Test Mode*/
	phy_set_bb_reg(pAdapter, rOFDM1_LSTF, BIT30 | BIT29 | BIT28, OFDM_ALL_OFF);

	cckrate  = pAdapter->mppriv.rateidx;

	phy_set_bb_reg(pAdapter, rCCK0_System, bCCKTxRate, cckrate);

	phy_set_bb_reg(pAdapter, rCCK0_System, bCCKBBMode, 0x2);	/*transmit mode*/
	phy_set_bb_reg(pAdapter, rCCK0_System, bCCKScramble, 0x1);	/*turn on scramble setting*/

	phy_set_bb_reg(pAdapter, 0xa14, 0x300, 0x3);			/* 0xa15[1:0] = 11 force cck rxiq = 0*/
	phy_set_bb_reg(pAdapter, rOFDM0_TRMuxPar, 0x10000, 0x1);		/* 0xc08[16] = 1 force ofdm rxiq = ofdm txiq*/
	phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter2, BIT14, 1);
	phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter2, BIT14, 1);
	phy_set_bb_reg(pAdapter, 0x0B34, BIT14, 1);

	phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000500);
	phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000500);

	pMptCtx->bCckContTx = true;
	pMptCtx->bOfdmContTx = false;

}	/* mpt_StartCckContTx */


static	void mpt_StartOfdmContTx(
	PADAPTER		pAdapter
)
{
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(pAdapter);
	PMPT_CONTEXT	pMptCtx = &(pAdapter->mppriv.mpt_ctx);

	/* 1. if OFDM block on?*/
	if (!phy_query_bb_reg(pAdapter, rFPGA0_RFMOD, bOFDMEn))
		phy_set_bb_reg(pAdapter, rFPGA0_RFMOD, bOFDMEn, 1);/*set OFDM block on*/

	/* 2. set CCK test mode off, set to CCK normal mode*/
	phy_set_bb_reg(pAdapter, rCCK0_System, bCCKBBMode, 0);

	/* 3. turn on scramble setting*/
	phy_set_bb_reg(pAdapter, rCCK0_System, bCCKScramble, 1);

	phy_set_bb_reg(pAdapter, 0xa14, 0x300, 0x3);			/* 0xa15[1:0] = 2b'11*/
	phy_set_bb_reg(pAdapter, rOFDM0_TRMuxPar, 0x10000, 0x1);		/* 0xc08[16] = 1*/

	/* 4. Turn On Continue Tx and turn off the other test modes.*/
	phy_set_bb_reg(pAdapter, rOFDM1_LSTF, BIT30 | BIT29 | BIT28, OFDM_ContinuousTx);

	phy_set_bb_reg(pAdapter, rFPGA0_XA_HSSIParameter1, bMaskDWord, 0x01000500);
	phy_set_bb_reg(pAdapter, rFPGA0_XB_HSSIParameter1, bMaskDWord, 0x01000500);

	pMptCtx->bCckContTx = false;
	pMptCtx->bOfdmContTx = true;
}	/* mpt_StartOfdmContTx */

void hal_mpt_SetContinuousTx(PADAPTER pAdapter, u8 bStart)
{
	u8 Rate;

	RT_TRACE(_module_mp_, _drv_info_,
		 ("SetContinuousTx: rate:%d\n", pAdapter->mppriv.rateidx));
	Rate = HwRateToMPTRate(pAdapter->mppriv.rateidx);
	pAdapter->mppriv.mpt_ctx.is_start_cont_tx = bStart;

	if (Rate <= MPT_RATE_11M) {
		if (bStart)
			mpt_StartCckContTx(pAdapter);
		else
			mpt_StopCckContTx(pAdapter);

	} else if (Rate >= MPT_RATE_6M) {
		if (bStart)
			mpt_StartOfdmContTx(pAdapter);
		else
			mpt_StopOfdmContTx(pAdapter);
	}
}

#endif /* CONFIG_MP_INCLUDE*/
