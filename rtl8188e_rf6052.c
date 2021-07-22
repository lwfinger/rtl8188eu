// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

/******************************************************************************
 *
 *
 * Module:	rtl8188e_rf6052.c	( Source C File)
 *
 * Note:	Provide RF 6052 series relative API.
 *
 * Function:
 *
 * Export:
 *
 * Abbrev:
 *
 * History:
 * Data			Who		Remark
 *
 * 09/25/2008	MHC		Create initial version.
 * 11/05/2008	MHC		Add API for tw power setting.
 *
 *
******************************************************************************/

#define _RTL8188E_RF6052_C_

#include <drv_types.h>
#include <rtl8188e_hal.h>

/*---------------------------Define Local Constant---------------------------*/

/*---------------------------Define Local Constant---------------------------*/


/*------------------------Define global variable-----------------------------*/
/*------------------------Define global variable-----------------------------*/


/*------------------------Define local variable------------------------------*/

/*------------------------Define local variable------------------------------*/


/*-----------------------------------------------------------------------------
 * Function:	RF_ChangeTxPath
 *
 * Overview:	For RL6052, we must change some RF settign for 1T or 2T.
 *
 * Input:		u16 DataRate		// 0x80-8f, 0x90-9f
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 09/25/2008	MHC		Create Version 0.
 *						Firmwaer support the utility later.
 *
 *---------------------------------------------------------------------------*/
void rtl8188e_RF_ChangeTxPath(PADAPTER	Adapter,
			      u16		DataRate)
{
	/* We do not support gain table change inACUT now !!!! Delete later !!! */
}	/* RF_ChangeTxPath */

/*-----------------------------------------------------------------------------
 * Function:    PHY_RF6052SetBandwidth()
 *
 * Overview:    This function is called by SetBWModeCallback8190Pci() only
 *
 * Input:       PADAPTER				Adapter
 *			WIRELESS_BANDWIDTH_E	Bandwidth	//20M or 40M
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Note:		For RF type 0222D
 *---------------------------------------------------------------------------*/
void
rtl8188e_PHY_RF6052SetBandwidth(
	PADAPTER				Adapter,
	CHANNEL_WIDTH		Bandwidth)	/* 20M or 40M */
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	switch (Bandwidth) {
	case CHANNEL_WIDTH_20:
		pHalData->RfRegChnlVal[0] = ((pHalData->RfRegChnlVal[0] & 0xfffff3ff) | BIT(10) | BIT(11));
		phy_set_rf_reg(Adapter, RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, pHalData->RfRegChnlVal[0]);
		break;

	case CHANNEL_WIDTH_40:
		pHalData->RfRegChnlVal[0] = ((pHalData->RfRegChnlVal[0] & 0xfffff3ff) | BIT(10));
		phy_set_rf_reg(Adapter, RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, pHalData->RfRegChnlVal[0]);
		break;

	default:
		break;
	}

}

static int
phy_RF6052_Config_ParaFile(
	PADAPTER		Adapter
)
{
	u32					u4RegValue = 0;
	u8					eRFPath;
	BB_REGISTER_DEFINITION_T	*pPhyReg;

	int					rtStatus = _SUCCESS;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);

	/* 3 */ /* ----------------------------------------------------------------- */
	/* 3 */ /* <2> Initialize RF */
	/* 3 */ /* ----------------------------------------------------------------- */
	/* for(eRFPath = RF_PATH_A; eRFPath <pHalData->NumTotalRFPath; eRFPath++) */
	for (eRFPath = 0; eRFPath < pHalData->NumTotalRFPath; eRFPath++) {

		pPhyReg = &pHalData->PHYRegDef[eRFPath];

		/*----Store original RFENV control type----*/
		switch (eRFPath) {
		case RF_PATH_A:
		case RF_PATH_C:
			u4RegValue = phy_query_bb_reg(Adapter, pPhyReg->rfintfs, bRFSI_RFENV);
			break;
		case RF_PATH_B:
		case RF_PATH_D:
			u4RegValue = phy_query_bb_reg(Adapter, pPhyReg->rfintfs, bRFSI_RFENV << 16);
			break;
		}

		/*----Set RF_ENV enable----*/
		phy_set_bb_reg(Adapter, pPhyReg->rfintfe, bRFSI_RFENV << 16, 0x1);
		rtw_udelay_os(1);/* PlatformStallExecution(1); */

		/*----Set RF_ENV output high----*/
		phy_set_bb_reg(Adapter, pPhyReg->rfintfo, bRFSI_RFENV, 0x1);
		rtw_udelay_os(1);/* PlatformStallExecution(1); */

		/* Set bit number of Address and Data for RF register */
		phy_set_bb_reg(Adapter, pPhyReg->rfHSSIPara2, b3WireAddressLength, 0x0);	/* Set 1 to 4 bits for 8255 */
		rtw_udelay_os(1);/* PlatformStallExecution(1); */

		phy_set_bb_reg(Adapter, pPhyReg->rfHSSIPara2, b3WireDataLength, 0x0);	/* Set 0 to 12  bits for 8255 */
		rtw_udelay_os(1);/* PlatformStallExecution(1); */

		/*----Initialize RF fom connfiguration file----*/
		switch (eRFPath) {
		case RF_PATH_A:
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
			if (PHY_ConfigRFWithParaFile(Adapter, PHY_FILE_RADIO_A, eRFPath) == _FAIL)
#endif
			{
#ifdef CONFIG_EMBEDDED_FWIMG
				if (HAL_STATUS_FAILURE == odm_config_rf_with_header_file(&pHalData->odmpriv, CONFIG_RF_RADIO, (enum odm_rf_radio_path_e)eRFPath))
					rtStatus = _FAIL;
#endif
			}
			break;
		case RF_PATH_B:
#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
			if (PHY_ConfigRFWithParaFile(Adapter, PHY_FILE_RADIO_B, eRFPath) == _FAIL)
#endif
			{
#ifdef CONFIG_EMBEDDED_FWIMG
				if (HAL_STATUS_FAILURE == odm_config_rf_with_header_file(&pHalData->odmpriv, CONFIG_RF_RADIO, (enum odm_rf_radio_path_e)eRFPath))
					rtStatus = _FAIL;
#endif
			}
			break;
		case RF_PATH_C:
			break;
		case RF_PATH_D:
			break;
		}

		/*----Restore RFENV control type----*/;
		switch (eRFPath) {
		case RF_PATH_A:
		case RF_PATH_C:
			phy_set_bb_reg(Adapter, pPhyReg->rfintfs, bRFSI_RFENV, u4RegValue);
			break;
		case RF_PATH_B:
		case RF_PATH_D:
			phy_set_bb_reg(Adapter, pPhyReg->rfintfs, bRFSI_RFENV << 16, u4RegValue);
			break;
		}

		if (rtStatus != _SUCCESS) {
			goto phy_RF6052_Config_ParaFile_Fail;
		}

	}


	/* 3 ----------------------------------------------------------------- */
	/* 3 Configuration of Tx Power Tracking */
	/* 3 ----------------------------------------------------------------- */

#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
	if (PHY_ConfigRFWithTxPwrTrackParaFile(Adapter, PHY_FILE_TXPWR_TRACK) == _FAIL)
#endif
	{
#ifdef CONFIG_EMBEDDED_FWIMG
		odm_config_rf_with_tx_pwr_track_header_file(&pHalData->odmpriv);
#endif
	}

	return rtStatus;

phy_RF6052_Config_ParaFile_Fail:
	return rtStatus;
}


int
PHY_RF6052_Config8188E(
	PADAPTER		Adapter)
{
	HAL_DATA_TYPE				*pHalData = GET_HAL_DATA(Adapter);
	int					rtStatus = _SUCCESS;

	/*  */
	/* Initialize general global value */
	/*  */
	/* TODO: Extend RF_PATH_C and RF_PATH_D in the future */
	if (pHalData->rf_type == RF_1T1R)
		pHalData->NumTotalRFPath = 1;
	else
		pHalData->NumTotalRFPath = 2;

	/*  */
	/* Config BB and RF */
	/*  */
	rtStatus = phy_RF6052_Config_ParaFile(Adapter);
	return rtStatus;
}

/* End of HalRf6052.c */
