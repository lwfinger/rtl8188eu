/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
/******************************************************************************
 *
 *
 * Module:	rtl8192c_rf6052.c	( Source C File)
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

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>

#include <rtl8188e_hal.h>

/*---------------------------Define Local Constant---------------------------*/
/*  Define local structure for debug!!!!! */
typedef struct RF_Shadow_Compare_Map {
	/*  Shadow register value */
	u32		Value;
	/*  Compare or not flag */
	u8		Compare;
	/*  Record If it had ever modified unpredicted */
	u8		ErrorOrNot;
	/*  Recorver Flag */
	u8		Recorver;
	/*  */
	u8		Driver_Write;
}RF_SHADOW_T;
/*---------------------------Define Local Constant---------------------------*/


/*------------------------Define global variable-----------------------------*/
/*------------------------Define global variable-----------------------------*/


/*------------------------Define local variable------------------------------*/
/*  2008/11/20 MH For Debug only, RF */
/* static	RF_SHADOW_T	RF_Shadow[RF6052_MAX_PATH][RF6052_MAX_REG] = {0}; */
static	RF_SHADOW_T	RF_Shadow[RF6052_MAX_PATH][RF6052_MAX_REG];
/*------------------------Define local variable------------------------------*/


/*-----------------------------------------------------------------------------
 * Function:	RF_ChangeTxPath
 *
 * Overview:	For RL6052, we must change some RF settign for 1T or 2T.
 *
 * Input:		u16 DataRate		0x80-8f, 0x90-9f
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
void rtl8188e_RF_ChangeTxPath(	struct adapter *Adapter,
										u16		DataRate)
{
/*  We do not support gain table change inACUT now !!!! Delete later !!! */
}	/* RF_ChangeTxPath */


/*-----------------------------------------------------------------------------
 * Function:    PHY_RF6052SetBandwidth()
 *
 * Overview:    This function is called by SetBWModeCallback8190Pci() only
 *
 * Input:       struct adapter *			Adapter
 *			WIRELESS_BANDWIDTH_E	Bandwidth	20M or 40M
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Note:		For RF type 0222D
 *---------------------------------------------------------------------------*/
void
rtl8188e_PHY_RF6052SetBandwidth(
	struct adapter *			Adapter,
	enum HT_CHANNEL_WIDTH		Bandwidth)	/* 20M or 40M */
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	switch (Bandwidth)
	{
		case HT_CHANNEL_WIDTH_20:
			pHalData->RfRegChnlVal[0] = ((pHalData->RfRegChnlVal[0] & 0xfffff3ff) | BIT(10) | BIT(11));
			PHY_SetRFReg(Adapter, RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, pHalData->RfRegChnlVal[0]);
			break;

		case HT_CHANNEL_WIDTH_40:
			pHalData->RfRegChnlVal[0] = ((pHalData->RfRegChnlVal[0] & 0xfffff3ff)| BIT(10));
			PHY_SetRFReg(Adapter, RF_PATH_A, RF_CHNLBW, bRFRegOffsetMask, pHalData->RfRegChnlVal[0]);
			break;

		default:
			break;
	}

}


/*-----------------------------------------------------------------------------
 * Function:	PHY_RF6052SetCckTxPower
 *
 * Overview:
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/05/2008	MHC		Simulate 8192series..
 *
 *---------------------------------------------------------------------------*/

void
rtl8188e_PHY_RF6052SetCckTxPower(
	struct adapter *	Adapter,
	u8*			pPowerlevel)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	struct mlme_priv	*pmlmepriv = &Adapter->mlmepriv;
	struct dm_priv		*pdmpriv = &pHalData->dmpriv;
	struct mlme_ext_priv		*pmlmeext = &Adapter->mlmeextpriv;
	u32			TxAGC[2]={0, 0}, tmpval=0,pwrtrac_value;
	bool		TurboScanOff = false;
	u8			idx1, idx2;
	u8*			ptr;
	u8			direction;
	TurboScanOff = true;


	if (pmlmeext->sitesurvey_res.state == SCAN_PROCESS)
	{
		TxAGC[RF_PATH_A] = 0x3f3f3f3f;
		TxAGC[RF_PATH_B] = 0x3f3f3f3f;

		TurboScanOff = true;/* disable turbo scan */

		if (TurboScanOff)
		{
			for (idx1=RF_PATH_A; idx1<=RF_PATH_B; idx1++)
			{
				TxAGC[idx1] =
					pPowerlevel[idx1] | (pPowerlevel[idx1]<<8) |
					(pPowerlevel[idx1]<<16) | (pPowerlevel[idx1]<<24);
				/*  2010/10/18 MH For external PA module. We need to limit power index to be less than 0x20. */
				if (TxAGC[idx1] > 0x20 && pHalData->ExternalPA)
					TxAGC[idx1] = 0x20;
			}
		}
	}
	else
	{
/*  20100427 Joseph: Driver dynamic Tx power shall not affect Tx power. It shall be determined by power training mechanism. */
/*  Currently, we cannot fully disable driver dynamic tx power mechanism because it is referenced by BT coexist mechanism. */
/*  In the future, two mechanism shall be separated from each other and maintained independantly. Thanks for Lanhsin's reminder. */
		if (pdmpriv->DynamicTxHighPowerLvl == TxHighPwrLevel_Level1)
		{
			TxAGC[RF_PATH_A] = 0x10101010;
			TxAGC[RF_PATH_B] = 0x10101010;
		}
		else if (pdmpriv->DynamicTxHighPowerLvl == TxHighPwrLevel_Level2)
		{
			TxAGC[RF_PATH_A] = 0x00000000;
			TxAGC[RF_PATH_B] = 0x00000000;
		}
		else
		{
			for (idx1=RF_PATH_A; idx1<=RF_PATH_B; idx1++)
			{
				TxAGC[idx1] =
					pPowerlevel[idx1] | (pPowerlevel[idx1]<<8) |
					(pPowerlevel[idx1]<<16) | (pPowerlevel[idx1]<<24);
			}

			if (pHalData->EEPROMRegulatory== 0)
			{
				tmpval = (pHalData->MCSTxPowerLevelOriginalOffset[0][6]) +
						(pHalData->MCSTxPowerLevelOriginalOffset[0][7]<<8);
				TxAGC[RF_PATH_A] += tmpval;

				tmpval = (pHalData->MCSTxPowerLevelOriginalOffset[0][14]) +
						(pHalData->MCSTxPowerLevelOriginalOffset[0][15]<<24);
				TxAGC[RF_PATH_B] += tmpval;
			}
		}
	}


	ODM_TxPwrTrackAdjust88E(&pHalData->odmpriv, 1, &direction, &pwrtrac_value);
	/* printk("ODM_TxPwrTrackAdjust88E => direction:%02x, pwrtrac_value:%d\n", direction, pwrtrac_value); */
	/* printk(" ==> TxAGC:0x%08x\n",TxAGC[0] ); */

	if (direction == 1)			/*  Increase TX pwoer */
	{
		TxAGC[0] += pwrtrac_value;
		TxAGC[1] += pwrtrac_value;

	}
	else if (direction == 2)	/*  Decrease TX pwoer */
	{
		TxAGC[0] -=  pwrtrac_value;
		TxAGC[1] -=  pwrtrac_value;
	}

	for (idx1=RF_PATH_A; idx1<=RF_PATH_B; idx1++)
	{
		ptr = (u8*)(&(TxAGC[idx1]));
		for (idx2=0; idx2<4; idx2++)
		{
			if (*ptr > RF6052_MAX_TX_PWR)
				*ptr = RF6052_MAX_TX_PWR;
			ptr++;
		}
	}
	/* printk(" ==> TxAGC:0x%08x\n",TxAGC[0] ); */

	/*  rf-A cck tx power */
	tmpval = TxAGC[RF_PATH_A]&0xff;
	PHY_SetBBReg(Adapter, rTxAGC_A_CCK1_Mcs32, bMaskByte1, tmpval);
	/* printk("CCK PWR 1M (rf-A) = 0x%x (reg 0x%x)\n", tmpval, rTxAGC_A_CCK1_Mcs32); */


	tmpval = TxAGC[RF_PATH_A]>>8;
	PHY_SetBBReg(Adapter, rTxAGC_B_CCK11_A_CCK2_11, 0xffffff00, tmpval);
	/* printk("CCK PWR 2~11M (rf-A) = 0x%x (reg 0x%x)\n", tmpval, rTxAGC_B_CCK11_A_CCK2_11); */

}	/* PHY_RF6052SetCckTxPower */

/*  powerbase0 for OFDM rates */
/*  powerbase1 for HT MCS rates */
static void getPowerBase88E(
	struct adapter *Adapter,
	u8*			pPowerLevelOFDM,
	u8*			pPowerLevelBW20,
	u8*			pPowerLevelBW40,
	u8			Channel,
	u32*		OfdmBase,
	u32*		MCSBase
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u32			powerBase0, powerBase1;
	u8			Legacy_pwrdiff=0;
	s8			HT20_pwrdiff=0;
	u8			i, powerlevel[2];

	for (i=0; i<2; i++)
	{
		powerBase0 = pPowerLevelOFDM[i];

		powerBase0 = (powerBase0<<24) | (powerBase0<<16) |(powerBase0<<8) |powerBase0;
		*(OfdmBase+i) = powerBase0;
		/* DBG_88E(" [OFDM power base index rf(%c) = 0x%x]\n", ((i== 0)?'A':'B'), *(OfdmBase+i)); */
	}

	for (i=0; i<pHalData->NumTotalRFPath; i++)
	{
		/* Check HT20 to HT40 diff */
		if (pHalData->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
		{
			powerlevel[i] = pPowerLevelBW20[i];
		}
		else
		{
			powerlevel[i] = pPowerLevelBW40[i];
		}
		powerBase1 = powerlevel[i];
		powerBase1 = (powerBase1<<24) | (powerBase1<<16) |(powerBase1<<8) |powerBase1;
		*(MCSBase+i) = powerBase1;
		/* DBG_88E(" [MCS power base index rf(%c) = 0x%x]\n", ((i== 0)?'A':'B'), *(MCSBase+i)); */
	}
}

static void getTxPowerWriteValByRegulatory88E(
		struct adapter *Adapter,
		u8			Channel,
		u8			index,
		u32*		powerBase0,
		u32*		powerBase1,
		u32*		pOutWriteVal
	)
{

	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	struct dm_priv	*pdmpriv = &pHalData->dmpriv;
	u8			i, chnlGroup=0, pwr_diff_limit[4], customer_pwr_limit;
	s8			pwr_diff=0;
	u32			writeVal, customer_limit, rf;
	u8			Regulatory = pHalData->EEPROMRegulatory;

	/*  */
	/*  Index 0 & 1= legacy OFDM, 2-5=HT_MCS rate */
	/*  */
	for (rf=0; rf<2; rf++) {
		switch (Regulatory) {
			case 0:	/*  Realtek better performance */
					/*  increase power diff defined by Realtek for large power */
				chnlGroup = 0;
				/* RTPRINT(FPHY, PHY_TXPWR, ("MCSTxPowerLevelOriginalOffset[%d][%d] = 0x%x\n", */
				/* 	chnlGroup, index, pHalData->MCSTxPowerLevelOriginalOffset[chnlGroup][index+(rf?8:0)])); */
				writeVal = pHalData->MCSTxPowerLevelOriginalOffset[chnlGroup][index+(rf?8:0)] +
					((index<2)?powerBase0[rf]:powerBase1[rf]);
				/* RTPRINT(FPHY, PHY_TXPWR, ("RTK better performance, writeVal(%c) = 0x%x\n", ((rf== 0)?'A':'B'), writeVal)); */
				break;
			case 1:	/*  Realtek regulatory */
					/*  increase power diff defined by Realtek for regulatory */
				{
					if (pHalData->pwrGroupCnt == 1)
						chnlGroup = 0;
					/* if (pHalData->pwrGroupCnt >= pHalData->PGMaxGroup) */
					{
						if (Channel < 3)			/*  Chanel 1-2 */
							chnlGroup = 0;
						else if (Channel < 6)		/*  Channel 3-5 */
							chnlGroup = 1;
						else	 if (Channel <9)		/*  Channel 6-8 */
							chnlGroup = 2;
						else if (Channel <12)		/*  Channel 9-11 */
							chnlGroup = 3;
						else if (Channel <14)		/*  Channel 12-13 */
							chnlGroup = 4;
						else if (Channel ==14)		/*  Channel 14 */
							chnlGroup = 4;

						if (pHalData->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
							chnlGroup++;
						else
							chnlGroup+=6;
					}
					writeVal = pHalData->MCSTxPowerLevelOriginalOffset[chnlGroup][index+(rf?8:0)] +
							((index<2)?powerBase0[rf]:powerBase1[rf]);
				}
				break;
			case 2:	/*  Better regulatory */
					/*  don't increase any power diff */
				writeVal = ((index<2)?powerBase0[rf]:powerBase1[rf]);
				break;
			case 3:	/*  Customer defined power diff. */
					/*  increase power diff defined by customer. */
				chnlGroup = 0;
				if (index < 2)
					pwr_diff = pHalData->TxPwrLegacyHtDiff[rf][Channel-1];
				else if (pHalData->CurrentChannelBW == HT_CHANNEL_WIDTH_20)
					pwr_diff = pHalData->TxPwrHt20Diff[rf][Channel-1];

				if (pHalData->CurrentChannelBW == HT_CHANNEL_WIDTH_40)
					customer_pwr_limit = pHalData->PwrGroupHT40[rf][Channel-1];
				else
					customer_pwr_limit = pHalData->PwrGroupHT20[rf][Channel-1];

				if (pwr_diff >= customer_pwr_limit)
					pwr_diff = 0;
				else
					pwr_diff = customer_pwr_limit - pwr_diff;

				for (i=0; i<4; i++)
					{
					pwr_diff_limit[i] = (u8)((pHalData->MCSTxPowerLevelOriginalOffset[chnlGroup][index+(rf?8:0)]&(0x7f<<(i*8)))>>(i*8));

					if (pwr_diff_limit[i] > pwr_diff)
						pwr_diff_limit[i] = pwr_diff;
				}
				customer_limit = (pwr_diff_limit[3]<<24) | (pwr_diff_limit[2]<<16) |
								(pwr_diff_limit[1]<<8) | (pwr_diff_limit[0]);
				/* RTPRINT(FPHY, PHY_TXPWR, ("Customer's limit rf(%c) = 0x%x\n", ((rf== 0)?'A':'B'), customer_limit)); */
				writeVal = customer_limit + ((index<2)?powerBase0[rf]:powerBase1[rf]);
				/* RTPRINT(FPHY, PHY_TXPWR, ("Customer, writeVal rf(%c)= 0x%x\n", ((rf== 0)?'A':'B'), writeVal)); */
				break;
			default:
				chnlGroup = 0;
				writeVal = pHalData->MCSTxPowerLevelOriginalOffset[chnlGroup][index+(rf?8:0)] +
						((index<2)?powerBase0[rf]:powerBase1[rf]);
				/* RTPRINT(FPHY, PHY_TXPWR, ("RTK better performance, writeVal rf(%c) = 0x%x\n", ((rf== 0)?'A':'B'), writeVal)); */
				break;
		}

/*  20100427 Joseph: Driver dynamic Tx power shall not affect Tx power. It shall be determined by power training mechanism. */
/*  Currently, we cannot fully disable driver dynamic tx power mechanism because it is referenced by BT coexist mechanism. */
/*  In the future, two mechanism shall be separated from each other and maintained independantly. Thanks for Lanhsin's reminder. */
		/* 92d do not need this */
		if (pdmpriv->DynamicTxHighPowerLvl == TxHighPwrLevel_Level1)
			writeVal = 0x14141414;
		else if (pdmpriv->DynamicTxHighPowerLvl == TxHighPwrLevel_Level2)
			writeVal = 0x00000000;

		/*  20100628 Joseph: High power mode for BT-Coexist mechanism. */
		/*  This mechanism is only applied when Driver-Highpower-Mechanism is OFF. */
		if (pdmpriv->DynamicTxHighPowerLvl == TxHighPwrLevel_BT1)
		{
			/* RTPRINT(FBT, BT_TRACE, ("Tx Power (-6)\n")); */
			writeVal = writeVal - 0x06060606;
		}
		else if (pdmpriv->DynamicTxHighPowerLvl == TxHighPwrLevel_BT2)
		{
			/* RTPRINT(FBT, BT_TRACE, ("Tx Power (-0)\n")); */
			writeVal = writeVal ;
		}
		*(pOutWriteVal+rf) = writeVal;
	}
}

static void writeOFDMPowerReg88E(
		struct adapter *Adapter,
		u8		index,
		u32*		pValue
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u16 RegOffset_A[6] = {	rTxAGC_A_Rate18_06, rTxAGC_A_Rate54_24,
							rTxAGC_A_Mcs03_Mcs00, rTxAGC_A_Mcs07_Mcs04,
							rTxAGC_A_Mcs11_Mcs08, rTxAGC_A_Mcs15_Mcs12};
	u16 RegOffset_B[6] = {	rTxAGC_B_Rate18_06, rTxAGC_B_Rate54_24,
							rTxAGC_B_Mcs03_Mcs00, rTxAGC_B_Mcs07_Mcs04,
							rTxAGC_B_Mcs11_Mcs08, rTxAGC_B_Mcs15_Mcs12};
	u8 i, rf, pwr_val[4];
	u32 writeVal;
	u16 RegOffset;

	for (rf=0; rf<2; rf++)
	{
		writeVal = pValue[rf];
		for (i=0; i<4; i++)
		{
			pwr_val[i] = (u8)((writeVal & (0x7f<<(i*8)))>>(i*8));
			if (pwr_val[i]  > RF6052_MAX_TX_PWR)
				pwr_val[i]  = RF6052_MAX_TX_PWR;
		}
		writeVal = (pwr_val[3]<<24) | (pwr_val[2]<<16) |(pwr_val[1]<<8) |pwr_val[0];

		if (rf == 0)
			RegOffset = RegOffset_A[index];
		else
			RegOffset = RegOffset_B[index];

		PHY_SetBBReg(Adapter, RegOffset, bMaskDWord, writeVal);
		/* printk("Set OFDM tx pwr- 0x%x = %08x\n", RegOffset, writeVal); */

		/*  201005115 Joseph: Set Tx Power diff for Tx power training mechanism. */
		if (((pHalData->rf_type == RF_2T2R) &&
				(RegOffset == rTxAGC_A_Mcs15_Mcs12 || RegOffset == rTxAGC_B_Mcs15_Mcs12))||
		     ((pHalData->rf_type != RF_2T2R) &&
				(RegOffset == rTxAGC_A_Mcs07_Mcs04 || RegOffset == rTxAGC_B_Mcs07_Mcs04))	)
		{
			writeVal = pwr_val[3];
			if (RegOffset == rTxAGC_A_Mcs15_Mcs12 || RegOffset == rTxAGC_A_Mcs07_Mcs04)
				RegOffset = 0xc90;
			if (RegOffset == rTxAGC_B_Mcs15_Mcs12 || RegOffset == rTxAGC_B_Mcs07_Mcs04)
				RegOffset = 0xc98;
			for (i=0; i<3; i++)
			{
				if (i!=2)
					writeVal = (writeVal>8)?(writeVal-8):0;
				else
					writeVal = (writeVal>6)?(writeVal-6):0;
				rtw_write8(Adapter, (u32)(RegOffset+i), (u8)writeVal);
			}
		}
	}
}


/*-----------------------------------------------------------------------------
 * Function:	PHY_RF6052SetOFDMTxPower
 *
 * Overview:	For legacy and HY OFDM, we must read EEPROM TX power index for
 *			different channel and read original value in TX power register area from
 *			0xe00. We increase offset and original value to be correct tx pwr.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/05/2008	MHC		Simulate 8192 series method.
 * 01/06/2009	MHC		1. Prevent Path B tx power overflow or underflow dure to
 *						A/B pwr difference or legacy/HT pwr diff.
 *						2. We concern with path B legacy/HT OFDM difference.
 * 01/22/2009	MHC		Support new EPRO format from SD3.
 *
 *---------------------------------------------------------------------------*/

void
rtl8188e_PHY_RF6052SetOFDMTxPower(
	struct adapter *Adapter,
	u8*		pPowerLevelOFDM,
	u8*		pPowerLevelBW20,
	u8*		pPowerLevelBW40,
	u8		Channel)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u32 writeVal[2], powerBase0[2], powerBase1[2], pwrtrac_value;
	u8 direction;
	u8 index = 0;


	/* DBG_88E("PHY_RF6052SetOFDMTxPower, channel(%d)\n", Channel); */

	getPowerBase88E(Adapter, pPowerLevelOFDM,pPowerLevelBW20,pPowerLevelBW40, Channel, &powerBase0[0], &powerBase1[0]);

	/*  */
	/*  2012/04/23 MH According to power tracking value, we need to revise OFDM tx power. */
	/*  This is ued to fix unstable power tracking mode. */
	/*  */
	ODM_TxPwrTrackAdjust88E(&pHalData->odmpriv, 0, &direction, &pwrtrac_value);

	for (index=0; index<6; index++)
	{
		getTxPowerWriteValByRegulatory88E(Adapter, Channel, index,
			&powerBase0[0], &powerBase1[0], &writeVal[0]);

		if (direction == 1)
		{
			writeVal[0] += pwrtrac_value;
			writeVal[1] += pwrtrac_value;
		}
		else if (direction == 2)
		{
			writeVal[0] -= pwrtrac_value;
			writeVal[1] -= pwrtrac_value;
		}

		writeOFDMPowerReg88E(Adapter, index, &writeVal[0]);
	}
}


static void
phy_RF6052_Config_HardCode(
	struct adapter *	Adapter
	)
{

	/*  Set Default Bandwidth to 20M */
	/* Adapter->HalFunc	.SetBWModeHandler(Adapter, enum HT_CHANNEL_WIDTH_20); */

	/*  TODO: Set Default Channel to channel one for RTL8225 */

}

static int
phy_RF6052_Config_ParaFile(
	struct adapter *	Adapter
	)
{
	u32					u4RegValue;
	u8					eRFPath;
	BB_REGISTER_DEFINITION_T	*pPhyReg;

	int					rtStatus = _SUCCESS;
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);

	static char			sz88eRadioAFile[] = RTL8188E_PHY_RADIO_A;
	static char			sz88eRadioBFile[] = RTL8188E_PHY_RADIO_B;
	char					*pszRadioAFile, *pszRadioBFile;



	pszRadioAFile = sz88eRadioAFile;
	pszRadioBFile = sz88eRadioBFile;


	/* 3----------------------------------------------------------------- */
	/* 3 <2> Initialize RF */
	/* 3----------------------------------------------------------------- */
	/* for (eRFPath = RF_PATH_A; eRFPath <pHalData->NumTotalRFPath; eRFPath++) */
	for (eRFPath = 0; eRFPath <pHalData->NumTotalRFPath; eRFPath++)
	{

		pPhyReg = &pHalData->PHYRegDef[eRFPath];

		/*----Store original RFENV control type----*/
		switch (eRFPath)
		{
		case RF_PATH_A:
		case RF_PATH_C:
			u4RegValue = PHY_QueryBBReg(Adapter, pPhyReg->rfintfs, bRFSI_RFENV);
			break;
		case RF_PATH_B :
		case RF_PATH_D:
			u4RegValue = PHY_QueryBBReg(Adapter, pPhyReg->rfintfs, bRFSI_RFENV<<16);
			break;
		}

		/*----Set RF_ENV enable----*/
		PHY_SetBBReg(Adapter, pPhyReg->rfintfe, bRFSI_RFENV<<16, 0x1);
		rtw_udelay_os(1);/* PlatformStallExecution(1); */

		/*----Set RF_ENV output high----*/
		PHY_SetBBReg(Adapter, pPhyReg->rfintfo, bRFSI_RFENV, 0x1);
		rtw_udelay_os(1);/* PlatformStallExecution(1); */

		/* Set bit number of Address and Data for RF register */
		PHY_SetBBReg(Adapter, pPhyReg->rfHSSIPara2, b3WireAddressLength, 0x0);	/*  Set 1 to 4 bits for 8255 */
		rtw_udelay_os(1);/* PlatformStallExecution(1); */

		PHY_SetBBReg(Adapter, pPhyReg->rfHSSIPara2, b3WireDataLength, 0x0);	/*  Set 0 to 12  bits for 8255 */
		rtw_udelay_os(1);/* PlatformStallExecution(1); */

		/*----Initialize RF fom connfiguration file----*/
		switch (eRFPath)
		{
		case RF_PATH_A:
			if (HAL_STATUS_FAILURE ==ODM_ConfigRFWithHeaderFile(&pHalData->odmpriv,(enum rf_radio_path)eRFPath, (enum rf_radio_path)eRFPath))
				rtStatus= _FAIL;
			break;
		case RF_PATH_B:
			if (HAL_STATUS_FAILURE ==ODM_ConfigRFWithHeaderFile(&pHalData->odmpriv,(enum rf_radio_path)eRFPath, (enum rf_radio_path)eRFPath))
				rtStatus= _FAIL;
			break;
		case RF_PATH_C:
			break;
		case RF_PATH_D:
			break;
		}

		/*----Restore RFENV control type----*/;
		switch (eRFPath)
		{
		case RF_PATH_A:
		case RF_PATH_C:
			PHY_SetBBReg(Adapter, pPhyReg->rfintfs, bRFSI_RFENV, u4RegValue);
			break;
		case RF_PATH_B :
		case RF_PATH_D:
			PHY_SetBBReg(Adapter, pPhyReg->rfintfs, bRFSI_RFENV<<16, u4RegValue);
			break;
		}

		if (rtStatus != _SUCCESS) {
			/* RT_TRACE(COMP_FPGA, DBG_LOUD, ("phy_RF6052_Config_ParaFile():Radio[%d] Fail!!", eRFPath)); */
			goto phy_RF6052_Config_ParaFile_Fail;
		}

	}

	/* RT_TRACE(COMP_INIT, DBG_LOUD, ("<---phy_RF6052_Config_ParaFile()\n")); */
	return rtStatus;

phy_RF6052_Config_ParaFile_Fail:
	return rtStatus;
}


int
PHY_RF6052_Config8188E(
	struct adapter *	Adapter)
{
	HAL_DATA_TYPE				*pHalData = GET_HAL_DATA(Adapter);
	int					rtStatus = _SUCCESS;

	/*  */
	/*  Initialize general global value */
	/*  */
	/*  TODO: Extend RF_PATH_C and RF_PATH_D in the future */
	if (pHalData->rf_type == RF_1T1R)
		pHalData->NumTotalRFPath = 1;
	else
		pHalData->NumTotalRFPath = 2;

	/*  */
	/*  Config BB and RF */
	/*  */
	rtStatus = phy_RF6052_Config_ParaFile(Adapter);
	return rtStatus;
}


/*  */
/*  ==> RF shadow Operation API Code Section!!! */
/*  */
/*-----------------------------------------------------------------------------
 * Function:	PHY_RFShadowRead
 *				PHY_RFShadowWrite
 *				PHY_RFShadowCompare
 *				PHY_RFShadowRecorver
 *				PHY_RFShadowCompareAll
 *				PHY_RFShadowRecorverAll
 *				PHY_RFShadowCompareFlagSet
 *				PHY_RFShadowRecorverFlagSet
 *
 * Overview:	When we set RF register, we must write shadow at first.
 *			When we are running, we must compare shadow abd locate error addr.
 *			Decide to recorver or not.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/20/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static u32
PHY_RFShadowRead(
	struct adapter *		Adapter,
	enum rf_radio_path	eRFPath,
	u32				Offset)
{
	return	RF_Shadow[eRFPath][Offset].Value;

}	/* PHY_RFShadowRead */


static void
PHY_RFShadowWrite(
	struct adapter *		Adapter,
	enum rf_radio_path	eRFPath,
	u32				Offset,
	u32				Data)
{
	RF_Shadow[eRFPath][Offset].Value = (Data & bRFRegOffsetMask);
	RF_Shadow[eRFPath][Offset].Driver_Write = true;

}	/* PHY_RFShadowWrite */


static bool
PHY_RFShadowCompare(
	struct adapter *		Adapter,
	enum rf_radio_path	eRFPath,
	u32				Offset)
{
	u32	reg;
	/*  Check if we need to check the register */
	if (RF_Shadow[eRFPath][Offset].Compare == true)
	{
		reg = PHY_QueryRFReg(Adapter, eRFPath, Offset, bRFRegOffsetMask);
		/*  Compare shadow and real rf register for 20bits!! */
		if (RF_Shadow[eRFPath][Offset].Value != reg)
		{
			/*  Locate error position. */
			RF_Shadow[eRFPath][Offset].ErrorOrNot = true;
			/* RT_TRACE(COMP_INIT, DBG_LOUD, */
			/* PHY_RFShadowCompare RF-%d Addr%02lx Err = %05lx\n", */
			/* eRFPath, Offset, reg)); */
		}
		return RF_Shadow[eRFPath][Offset].ErrorOrNot ;
	}
	return false;
}	/* PHY_RFShadowCompare */


static void
PHY_RFShadowRecorver(
	struct adapter *		Adapter,
	enum rf_radio_path	eRFPath,
	u32				Offset)
{
	/*  Check if the address is error */
	if (RF_Shadow[eRFPath][Offset].ErrorOrNot == true)
	{
		/*  Check if we need to recorver the register. */
		if (RF_Shadow[eRFPath][Offset].Recorver == true)
		{
			PHY_SetRFReg(Adapter, eRFPath, Offset, bRFRegOffsetMask,
							RF_Shadow[eRFPath][Offset].Value);
			/* RT_TRACE(COMP_INIT, DBG_LOUD, */
			/* PHY_RFShadowRecorver RF-%d Addr%02lx=%05lx", */
			/* eRFPath, Offset, RF_Shadow[eRFPath][Offset].Value)); */
		}
	}

}	/* PHY_RFShadowRecorver */


static void
PHY_RFShadowCompareAll(
	struct adapter *		Adapter)
{
	u32		eRFPath;
	u32		Offset;

	for (eRFPath = 0; eRFPath < RF6052_MAX_PATH; eRFPath++)
	{
		for (Offset = 0; Offset <= RF6052_MAX_REG; Offset++)
		{
			PHY_RFShadowCompare(Adapter, (enum rf_radio_path)eRFPath, Offset);
		}
	}

}	/* PHY_RFShadowCompareAll */


static void
PHY_RFShadowRecorverAll(
	struct adapter *		Adapter)
{
	u32		eRFPath;
	u32		Offset;

	for (eRFPath = 0; eRFPath < RF6052_MAX_PATH; eRFPath++)
	{
		for (Offset = 0; Offset <= RF6052_MAX_REG; Offset++)
		{
			PHY_RFShadowRecorver(Adapter, (enum rf_radio_path)eRFPath, Offset);
		}
	}

}	/* PHY_RFShadowRecorverAll */


static void
PHY_RFShadowCompareFlagSet(
	struct adapter *		Adapter,
	enum rf_radio_path	eRFPath,
	u32				Offset,
	u8				Type)
{
	/*  Set True or False!!! */
	RF_Shadow[eRFPath][Offset].Compare = Type;

}	/* PHY_RFShadowCompareFlagSet */


static void
PHY_RFShadowRecorverFlagSet(
	struct adapter *		Adapter,
	enum rf_radio_path	eRFPath,
	u32				Offset,
	u8				Type)
{
	/*  Set True or False!!! */
	RF_Shadow[eRFPath][Offset].Recorver= Type;

}	/* PHY_RFShadowRecorverFlagSet */


static void
PHY_RFShadowCompareFlagSetAll(
	struct adapter *		Adapter)
{
	u32		eRFPath;
	u32		Offset;

	for (eRFPath = 0; eRFPath < RF6052_MAX_PATH; eRFPath++)
	{
		for (Offset = 0; Offset <= RF6052_MAX_REG; Offset++)
		{
			/*  2008/11/20 MH For S3S4 test, we only check reg 26/27 now!!!! */
			if (Offset != 0x26 && Offset != 0x27)
				PHY_RFShadowCompareFlagSet(Adapter, (enum rf_radio_path)eRFPath, Offset, false);
			else
				PHY_RFShadowCompareFlagSet(Adapter, (enum rf_radio_path)eRFPath, Offset, true);
		}
	}

}	/* PHY_RFShadowCompareFlagSetAll */


static void
PHY_RFShadowRecorverFlagSetAll(
	struct adapter *		Adapter)
{
	u32		eRFPath;
	u32		Offset;

	for (eRFPath = 0; eRFPath < RF6052_MAX_PATH; eRFPath++)
	{
		for (Offset = 0; Offset <= RF6052_MAX_REG; Offset++)
		{
			/*  2008/11/20 MH For S3S4 test, we only check reg 26/27 now!!!! */
			if (Offset != 0x26 && Offset != 0x27)
				PHY_RFShadowRecorverFlagSet(Adapter, (enum rf_radio_path)eRFPath, Offset, false);
			else
				PHY_RFShadowRecorverFlagSet(Adapter, (enum rf_radio_path)eRFPath, Offset, true);
		}
	}

}	/* PHY_RFShadowCompareFlagSetAll */

static void
PHY_RFShadowRefresh(
	struct adapter *		Adapter)
{
	u32		eRFPath;
	u32		Offset;

	for (eRFPath = 0; eRFPath < RF6052_MAX_PATH; eRFPath++)
	{
		for (Offset = 0; Offset <= RF6052_MAX_REG; Offset++)
		{
			RF_Shadow[eRFPath][Offset].Value = 0;
			RF_Shadow[eRFPath][Offset].Compare = false;
			RF_Shadow[eRFPath][Offset].Recorver  = false;
			RF_Shadow[eRFPath][Offset].ErrorOrNot = false;
			RF_Shadow[eRFPath][Offset].Driver_Write = false;
		}
	}

}	/* PHY_RFShadowRead */

/* End of HalRf6052.c */
