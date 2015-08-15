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

/*  */
/*  include files */
/*  */

#include "odm_precomp.h"

void
ODM_DIG_LowerBound_88E(
		PDM_ODM_T		pDM_Odm
)
{
	pDIG_T		pDM_DigTable = &pDM_Odm->DM_DigTable;

	if (pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)
	{
		pDM_DigTable->rx_gain_range_min = (u8) pDM_DigTable->AntDiv_RSSI_max;
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_DIG_LowerBound_88E(): pDM_DigTable->AntDiv_RSSI_max=%d\n",pDM_DigTable->AntDiv_RSSI_max));
	}
	/* If only one Entry connected */



}

static void
odm_RX_HWAntDivInit(
		PDM_ODM_T		pDM_Odm
)
{
	u32	value32;
	struct adapter *	Adapter = pDM_Odm->Adapter;
        #if (MP_DRIVER == 1)
        if (*(pDM_Odm->mp_mode) == 1)
	{
		pDM_Odm->AntDivType = CGCS_RX_SW_ANTDIV;
		ODM_SetBBReg(pDM_Odm, ODM_REG_IGI_A_11N , BIT7, 0); /*  disable HW AntDiv */
		ODM_SetBBReg(pDM_Odm, ODM_REG_LNA_SWITCH_11N , BIT31, 1);  /*  1:CG, 0:CS */
		return;
        }
        #endif
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_RX_HWAntDivInit()\n"));

	/* MAC Setting */
	value32 = ODM_GetMACReg(pDM_Odm, ODM_REG_ANTSEL_PIN_11N, bMaskDWord);
	ODM_SetMACReg(pDM_Odm, ODM_REG_ANTSEL_PIN_11N, bMaskDWord, value32|(BIT23|BIT25)); /* Reg4C[25]=1, Reg4C[23]=1 for pin output */
	/* Pin Settings */
	ODM_SetBBReg(pDM_Odm, ODM_REG_PIN_CTRL_11N , BIT9|BIT8, 0);/* Reg870[8]=1'b0, Reg870[9]=1'b0antsel antselb by HW */
	ODM_SetBBReg(pDM_Odm, ODM_REG_RX_ANT_CTRL_11N , BIT10, 0);	/* Reg864[10]=1'b0	antsel2 by HW */
	ODM_SetBBReg(pDM_Odm, ODM_REG_LNA_SWITCH_11N , BIT22, 1);	/* Regb2c[22]=1'b0	disable CS/CG switch */
	ODM_SetBBReg(pDM_Odm, ODM_REG_LNA_SWITCH_11N , BIT31, 1);	/* Regb2c[31]=1'b1	output at CG only */
	/* OFDM Settings */
	ODM_SetBBReg(pDM_Odm, ODM_REG_ANTDIV_PARA1_11N , bMaskDWord, 0x000000a0);
	/* CCK Settings */
	ODM_SetBBReg(pDM_Odm, ODM_REG_BB_PWR_SAV4_11N , BIT7, 1); /* Fix CCK PHY status report issue */
	ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA2_11N , BIT4, 1); /* CCK complete HW AntDiv within 64 samples */
	ODM_UpdateRxIdleAnt_88E(pDM_Odm, MAIN_ANT);
	ODM_SetBBReg(pDM_Odm, ODM_REG_ANT_MAPPING1_11N , 0xFFFF, 0x0201);	/* antenna mapping table */

	/* ODM_SetBBReg(pDM_Odm, 0xc50 , BIT7, 1);	Enable HW AntDiv */
	/* ODM_SetBBReg(pDM_Odm, 0xa00 , BIT15, 1); Enable CCK AntDiv */
}

static void
odm_TRX_HWAntDivInit(
		PDM_ODM_T		pDM_Odm
)
{
	u32	value32;
	struct adapter *	Adapter = pDM_Odm->Adapter;

        #if (MP_DRIVER == 1)
	if (*(pDM_Odm->mp_mode) == 1)
        {
		pDM_Odm->AntDivType = CGCS_RX_SW_ANTDIV;
		ODM_SetBBReg(pDM_Odm, ODM_REG_IGI_A_11N , BIT7, 0); /*  disable HW AntDiv */
		ODM_SetBBReg(pDM_Odm, ODM_REG_RX_ANT_CTRL_11N , BIT5|BIT4|BIT3, 0); /* Default RX   (0/1) */
		return;
		}

        #endif

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_TRX_HWAntDivInit()\n"));

	/* MAC Setting */
	value32 = ODM_GetMACReg(pDM_Odm, ODM_REG_ANTSEL_PIN_11N, bMaskDWord);
	ODM_SetMACReg(pDM_Odm, ODM_REG_ANTSEL_PIN_11N, bMaskDWord, value32|(BIT23|BIT25)); /* Reg4C[25]=1, Reg4C[23]=1 for pin output */
	/* Pin Settings */
	ODM_SetBBReg(pDM_Odm, ODM_REG_PIN_CTRL_11N , BIT9|BIT8, 0);/* Reg870[8]=1'b0, Reg870[9]=1'b0		antsel antselb by HW */
	ODM_SetBBReg(pDM_Odm, ODM_REG_RX_ANT_CTRL_11N , BIT10, 0);	/* Reg864[10]=1'b0	antsel2 by HW */
	ODM_SetBBReg(pDM_Odm, ODM_REG_LNA_SWITCH_11N , BIT22, 0);	/* Regb2c[22]=1'b0	disable CS/CG switch */
	ODM_SetBBReg(pDM_Odm, ODM_REG_LNA_SWITCH_11N , BIT31, 1);	/* Regb2c[31]=1'b1	output at CG only */
	/* OFDM Settings */
	ODM_SetBBReg(pDM_Odm, ODM_REG_ANTDIV_PARA1_11N , bMaskDWord, 0x000000a0);
	/* CCK Settings */
	ODM_SetBBReg(pDM_Odm, ODM_REG_BB_PWR_SAV4_11N , BIT7, 1); /* Fix CCK PHY status report issue */
	ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA2_11N , BIT4, 1); /* CCK complete HW AntDiv within 64 samples */
	/* Tx Settings */
	ODM_SetBBReg(pDM_Odm, ODM_REG_TX_ANT_CTRL_11N , BIT21, 0); /* Reg80c[21]=1'b0		from TX Reg */
	ODM_UpdateRxIdleAnt_88E(pDM_Odm, MAIN_ANT);

	/* antenna mapping table */
	if (!pDM_Odm->bIsMPChip) /* testchip */
	{
		ODM_SetBBReg(pDM_Odm, ODM_REG_RX_DEFUALT_A_11N , BIT10|BIT9|BIT8, 1);	/* Reg858[10:8]=3'b001 */
		ODM_SetBBReg(pDM_Odm, ODM_REG_RX_DEFUALT_A_11N , BIT13|BIT12|BIT11, 2);	/* Reg858[13:11]=3'b010 */
	}
	else /* MPchip */
		ODM_SetBBReg(pDM_Odm, ODM_REG_ANT_MAPPING1_11N , bMaskDWord, 0x0201);	/* Reg914=3'b010, Reg915=3'b001 */

	/* ODM_SetBBReg(pDM_Odm, 0xc50 , BIT7, 1);	Enable HW AntDiv */
	/* ODM_SetBBReg(pDM_Odm, 0xa00 , BIT15, 1); Enable CCK AntDiv */
}

void
odm_FastAntTrainingInit(
		PDM_ODM_T		pDM_Odm
)
{
	u32	value32, i;
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	u32	AntCombination = 2;
	struct adapter *	Adapter = pDM_Odm->Adapter;


	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("odm_FastAntTrainingInit()\n"));

#if (MP_DRIVER == 1)
	if (*(pDM_Odm->mp_mode) == 1) {
		ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_LOUD, ("pDM_Odm->AntDivType: %d\n", pDM_Odm->AntDivType));
		return;
	}
#endif

	for (i=0; i<6; i++) {
		pDM_FatTable->Bssid[i] = 0;
		pDM_FatTable->antSumRSSI[i] = 0;
		pDM_FatTable->antRSSIcnt[i] = 0;
		pDM_FatTable->antAveRSSI[i] = 0;
	}
	pDM_FatTable->TrainIdx = 0;
	pDM_FatTable->FAT_State = FAT_NORMAL_STATE;

	/* MAC Setting */
	value32 = ODM_GetMACReg(pDM_Odm, 0x4c, bMaskDWord);
	ODM_SetMACReg(pDM_Odm, 0x4c, bMaskDWord, value32|(BIT23|BIT25)); /* Reg4C[25]=1, Reg4C[23]=1 for pin output */
	value32 = ODM_GetMACReg(pDM_Odm,  0x7B4, bMaskDWord);
	ODM_SetMACReg(pDM_Odm, 0x7b4, bMaskDWord, value32|(BIT16|BIT17)); /* Reg7B4[16]=1 enable antenna training, Reg7B4[17]=1 enable A2 match */
	/* value32 = PlatformEFIORead4Byte(Adapter, 0x7B4); */
	/* PlatformEFIOWrite4Byte(Adapter, 0x7b4, value32|BIT18);	append MACID in reponse packet */

	/* Match MAC ADDR */
	ODM_SetMACReg(pDM_Odm, 0x7b4, 0xFFFF, 0);
	ODM_SetMACReg(pDM_Odm, 0x7b0, bMaskDWord, 0);

	ODM_SetBBReg(pDM_Odm, 0x870 , BIT9|BIT8, 0);/* Reg870[8]=1'b0, Reg870[9]=1'b0		antsel antselb by HW */
	ODM_SetBBReg(pDM_Odm, 0x864 , BIT10, 0);	/* Reg864[10]=1'b0	antsel2 by HW */
	ODM_SetBBReg(pDM_Odm, 0xb2c , BIT22, 0);	/* Regb2c[22]=1'b0	disable CS/CG switch */
	ODM_SetBBReg(pDM_Odm, 0xb2c , BIT31, 1);	/* Regb2c[31]=1'b1	output at CG only */
	ODM_SetBBReg(pDM_Odm, 0xca4 , bMaskDWord, 0x000000a0);

	/* antenna mapping table */
	if (AntCombination == 2)
	{
		if (!pDM_Odm->bIsMPChip) /* testchip */
		{
			ODM_SetBBReg(pDM_Odm, 0x858 , BIT10|BIT9|BIT8, 1);	/* Reg858[10:8]=3'b001 */
			ODM_SetBBReg(pDM_Odm, 0x858 , BIT13|BIT12|BIT11, 2);	/* Reg858[13:11]=3'b010 */
		}
		else /* MPchip */
		{
			ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte0, 1);
			ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte1, 2);
		}
	}
	else if (AntCombination == 7)
	{
		if (!pDM_Odm->bIsMPChip) /* testchip */
		{
			ODM_SetBBReg(pDM_Odm, 0x858 , BIT10|BIT9|BIT8, 0);	/* Reg858[10:8]=3'b000 */
			ODM_SetBBReg(pDM_Odm, 0x858 , BIT13|BIT12|BIT11, 1);	/* Reg858[13:11]=3'b001 */
			ODM_SetBBReg(pDM_Odm, 0x878 , BIT16, 0);
			ODM_SetBBReg(pDM_Odm, 0x858 , BIT15|BIT14, 2);	/* Reg878[0],Reg858[14:15])=3'b010 */
			ODM_SetBBReg(pDM_Odm, 0x878 , BIT19|BIT18|BIT17, 3);/* Reg878[3:1]=3b'011 */
			ODM_SetBBReg(pDM_Odm, 0x878 , BIT22|BIT21|BIT20, 4);/* Reg878[6:4]=3b'100 */
			ODM_SetBBReg(pDM_Odm, 0x878 , BIT25|BIT24|BIT23, 5);/* Reg878[9:7]=3b'101 */
			ODM_SetBBReg(pDM_Odm, 0x878 , BIT28|BIT27|BIT26, 6);/* Reg878[12:10]=3b'110 */
			ODM_SetBBReg(pDM_Odm, 0x878 , BIT31|BIT30|BIT29, 7);/* Reg878[15:13]=3b'111 */
		}
		else /* MPchip */
		{
			ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte0, 0);
			ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte1, 1);
			ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte2, 2);
			ODM_SetBBReg(pDM_Odm, 0x914 , bMaskByte3, 3);
			ODM_SetBBReg(pDM_Odm, 0x918 , bMaskByte0, 4);
			ODM_SetBBReg(pDM_Odm, 0x918 , bMaskByte1, 5);
			ODM_SetBBReg(pDM_Odm, 0x918 , bMaskByte2, 6);
			ODM_SetBBReg(pDM_Odm, 0x918 , bMaskByte3, 7);
		}
	}

	/* Default Ant Setting when no fast training */
	ODM_SetBBReg(pDM_Odm, 0x80c , BIT21, 1); /* Reg80c[21]=1'b1		from TX Info */
	ODM_SetBBReg(pDM_Odm, 0x864 , BIT5|BIT4|BIT3, 0);	/* Default RX */
	ODM_SetBBReg(pDM_Odm, 0x864 , BIT8|BIT7|BIT6, 1);	/* Optional RX */

	/* Enter Traing state */
	ODM_SetBBReg(pDM_Odm, 0x864 , BIT2|BIT1|BIT0, (AntCombination-1));	/* Reg864[2:0]=3'd6	ant combination=reg864[2:0]+1 */
	ODM_SetBBReg(pDM_Odm, 0xc50 , BIT7, 1);	/* RegC50[7]=1'b1		enable HW AntDiv */

	/* SW Control */
	/* PHY_SetBBReg(Adapter, 0x864 , BIT10, 1); */
	/* PHY_SetBBReg(Adapter, 0x870 , BIT9, 1); */
	/* PHY_SetBBReg(Adapter, 0x870 , BIT8, 1); */
	/* PHY_SetBBReg(Adapter, 0x864 , BIT11, 1); */
	/* PHY_SetBBReg(Adapter, 0x860 , BIT9, 0); */
	/* PHY_SetBBReg(Adapter, 0x860 , BIT8, 0); */
}

void
ODM_AntennaDiversityInit_88E(
		PDM_ODM_T		pDM_Odm
)
{
	if (pDM_Odm->SupportICType != ODM_RTL8188E)
		return;

	/* ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("pDM_Odm->AntDivType=%d, pHalData->AntDivCfg=%d\n", */
	/* 	pDM_Odm->AntDivType, pHalData->AntDivCfg)); */
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("pDM_Odm->AntDivType=%d\n",pDM_Odm->AntDivType));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("pDM_Odm->bIsMPChip=%s\n",(pDM_Odm->bIsMPChip?"true":"false")));

	if (pDM_Odm->AntDivType == CGCS_RX_HW_ANTDIV)
		odm_RX_HWAntDivInit(pDM_Odm);
	else if (pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)
		odm_TRX_HWAntDivInit(pDM_Odm);
	else if (pDM_Odm->AntDivType == CG_TRX_SMART_ANTDIV)
		odm_FastAntTrainingInit(pDM_Odm);
}


void
ODM_UpdateRxIdleAnt_88E(PDM_ODM_T pDM_Odm, u8 Ant)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	u32	DefaultAnt, OptionalAnt;

	if (pDM_FatTable->RxIdleAnt != Ant)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Need to Update Rx Idle Ant\n"));
		if (Ant == MAIN_ANT)
		{
			DefaultAnt = (pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)?MAIN_ANT_CG_TRX:MAIN_ANT_CGCS_RX;
			OptionalAnt = (pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)?AUX_ANT_CG_TRX:AUX_ANT_CGCS_RX;
		}
		else
		{
			DefaultAnt = (pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)?AUX_ANT_CG_TRX:AUX_ANT_CGCS_RX;
			OptionalAnt = (pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)?MAIN_ANT_CG_TRX:MAIN_ANT_CGCS_RX;
		}

		if (pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)
		{
			ODM_SetBBReg(pDM_Odm, ODM_REG_RX_ANT_CTRL_11N , BIT5|BIT4|BIT3, DefaultAnt);	/* Default RX */
			ODM_SetBBReg(pDM_Odm, ODM_REG_RX_ANT_CTRL_11N , BIT8|BIT7|BIT6, OptionalAnt);		/* Optional RX */
			ODM_SetBBReg(pDM_Odm, ODM_REG_ANTSEL_CTRL_11N , BIT14|BIT13|BIT12, DefaultAnt);	/* Default TX */
			ODM_SetMACReg(pDM_Odm, ODM_REG_RESP_TX_11N , BIT6|BIT7, DefaultAnt);	/* Resp Tx */

		}
		else if (pDM_Odm->AntDivType == CGCS_RX_HW_ANTDIV)
		{
			ODM_SetBBReg(pDM_Odm, ODM_REG_RX_ANT_CTRL_11N , BIT5|BIT4|BIT3, DefaultAnt);	/* Default RX */
			ODM_SetBBReg(pDM_Odm, ODM_REG_RX_ANT_CTRL_11N , BIT8|BIT7|BIT6, OptionalAnt);		/* Optional RX */
		}
	}
	pDM_FatTable->RxIdleAnt = Ant;
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("RxIdleAnt=%s\n",(Ant==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
	printk("RxIdleAnt=%s\n",(Ant==MAIN_ANT)?"MAIN_ANT":"AUX_ANT");
}


static void
odm_UpdateTxAnt_88E(PDM_ODM_T pDM_Odm, u8 Ant, u32 MacId)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	u8	TargetAnt;

	if (Ant == MAIN_ANT)
		TargetAnt = MAIN_ANT_CG_TRX;
	else
		TargetAnt = AUX_ANT_CG_TRX;

	pDM_FatTable->antsel_a[MacId] = TargetAnt&BIT0;
	pDM_FatTable->antsel_b[MacId] = (TargetAnt&BIT1)>>1;
	pDM_FatTable->antsel_c[MacId] = (TargetAnt&BIT2)>>2;

	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Tx from TxInfo, TargetAnt=%s\n",
						(Ant==MAIN_ANT)?"MAIN_ANT":"AUX_ANT"));
	ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD,("antsel_tr_mux=3'b%d%d%d\n",
							pDM_FatTable->antsel_c[MacId] , pDM_FatTable->antsel_b[MacId] , pDM_FatTable->antsel_a[MacId] ));
}

void
ODM_SetTxAntByTxInfo_88E(
		PDM_ODM_T		pDM_Odm,
		u8 *			pDesc,
		u8			macId
	)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;

	if ((pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)||(pDM_Odm->AntDivType == CG_TRX_SMART_ANTDIV))
	{
		SET_TX_DESC_ANTSEL_A_88E(pDesc, pDM_FatTable->antsel_a[macId]);
		SET_TX_DESC_ANTSEL_B_88E(pDesc, pDM_FatTable->antsel_b[macId]);
		SET_TX_DESC_ANTSEL_C_88E(pDesc, pDM_FatTable->antsel_c[macId]);
		/* ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_SetTxAntByTxInfo_88E_WIN(): MacID=%d, antsel_tr_mux=3'b%d%d%d\n", */
		/* 	macId, pDM_FatTable->antsel_c[macId], pDM_FatTable->antsel_b[macId], pDM_FatTable->antsel_a[macId])); */
	}
}

void
ODM_AntselStatistics_88E(
		PDM_ODM_T		pDM_Odm,
		u8			antsel_tr_mux,
		u32			MacId,
		u8			RxPWDBAll
)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	if (pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)
	{
		if (antsel_tr_mux == MAIN_ANT_CG_TRX)
		{

			pDM_FatTable->MainAnt_Sum[MacId]+=RxPWDBAll;
			pDM_FatTable->MainAnt_Cnt[MacId]++;
		}
		else
		{
			pDM_FatTable->AuxAnt_Sum[MacId]+=RxPWDBAll;
			pDM_FatTable->AuxAnt_Cnt[MacId]++;

		}
	}
	else if (pDM_Odm->AntDivType == CGCS_RX_HW_ANTDIV)
	{
		if (antsel_tr_mux == MAIN_ANT_CGCS_RX)
		{

			pDM_FatTable->MainAnt_Sum[MacId]+=RxPWDBAll;
			pDM_FatTable->MainAnt_Cnt[MacId]++;
		}
		else
		{
			pDM_FatTable->AuxAnt_Sum[MacId]+=RxPWDBAll;
			pDM_FatTable->AuxAnt_Cnt[MacId]++;

		}
	}
}

#define	TX_BY_REG	0
static void
odm_HWAntDiv(
		PDM_ODM_T		pDM_Odm
)
{
	u32	i, MinRSSI=0xFF, AntDivMaxRSSI=0, MaxRSSI=0, LocalMinRSSI, LocalMaxRSSI;
	u32	Main_RSSI, Aux_RSSI;
	u8	RxIdleAnt=0, TargetAnt=7;
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	pDIG_T	pDM_DigTable = &pDM_Odm->DM_DigTable;
	bool	bMatchBSSID;
	bool	bPktFilterMacth = false;
	PSTA_INFO_T	pEntry;

	for (i=0; i<ODM_ASSOCIATE_ENTRY_NUM; i++)
	{
		pEntry = pDM_Odm->pODM_StaInfo[i];
		if (IS_STA_VALID(pEntry))
		{
			/* 2 Caculate RSSI per Antenna */
			Main_RSSI = (pDM_FatTable->MainAnt_Cnt[i]!=0)?(pDM_FatTable->MainAnt_Sum[i]/pDM_FatTable->MainAnt_Cnt[i]):0;
			Aux_RSSI = (pDM_FatTable->AuxAnt_Cnt[i]!=0)?(pDM_FatTable->AuxAnt_Sum[i]/pDM_FatTable->AuxAnt_Cnt[i]):0;
			TargetAnt = (Main_RSSI>=Aux_RSSI)?MAIN_ANT:AUX_ANT;
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("MacID=%d, MainAnt_Sum=%d, MainAnt_Cnt=%d\n", i, pDM_FatTable->MainAnt_Sum[i], pDM_FatTable->MainAnt_Cnt[i]));
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("MacID=%d, AuxAnt_Sum=%d, AuxAnt_Cnt=%d\n",i, pDM_FatTable->AuxAnt_Sum[i], pDM_FatTable->AuxAnt_Cnt[i]));
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("MacID=%d, Main_RSSI= %d, Aux_RSSI= %d\n", i, Main_RSSI, Aux_RSSI));

			/* 2 Select MaxRSSI for DIG */
			LocalMaxRSSI = (Main_RSSI>Aux_RSSI)?Main_RSSI:Aux_RSSI;
			if ((LocalMaxRSSI > AntDivMaxRSSI) && (LocalMaxRSSI < 40))
				AntDivMaxRSSI = LocalMaxRSSI;
			if (LocalMaxRSSI > MaxRSSI)
				MaxRSSI = LocalMaxRSSI;

			/* 2 Select RX Idle Antenna */
			if ((pDM_FatTable->RxIdleAnt == MAIN_ANT) && (Main_RSSI == 0))
				Main_RSSI = Aux_RSSI;
			else if ((pDM_FatTable->RxIdleAnt == AUX_ANT) && (Aux_RSSI == 0))
				Aux_RSSI = Main_RSSI;

			LocalMinRSSI = (Main_RSSI>Aux_RSSI)?Aux_RSSI:Main_RSSI;
			if (LocalMinRSSI < MinRSSI)
			{
				MinRSSI = LocalMinRSSI;
				RxIdleAnt = TargetAnt;
			}
#if TX_BY_REG

#else
			/* 2 Select TRX Antenna */
			if (pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)
				odm_UpdateTxAnt_88E(pDM_Odm, TargetAnt, i);
#endif
		}
		pDM_FatTable->MainAnt_Sum[i] = 0;
		pDM_FatTable->AuxAnt_Sum[i] = 0;
		pDM_FatTable->MainAnt_Cnt[i] = 0;
		pDM_FatTable->AuxAnt_Cnt[i] = 0;
	}

	/* 2 Set RX Idle Antenna */
	ODM_UpdateRxIdleAnt_88E(pDM_Odm, RxIdleAnt);

	pDM_DigTable->AntDiv_RSSI_max = AntDivMaxRSSI;
	pDM_DigTable->RSSI_max = MaxRSSI;
}

void
ODM_AntennaDiversity_88E(
		PDM_ODM_T		pDM_Odm
)
{
	pFAT_T	pDM_FatTable = &pDM_Odm->DM_FatTable;
	if ((pDM_Odm->SupportICType != ODM_RTL8188E) || (!(pDM_Odm->SupportAbility & ODM_BB_ANT_DIV)))
	{
		/* ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_AntennaDiversity_88E: Not Support 88E AntDiv\n")); */
		return;
	}

	if (!pDM_Odm->bLinked)
	{
		ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("ODM_AntennaDiversity_88E(): No Link.\n"));
		if (pDM_FatTable->bBecomeLinked == true)
		{
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Need to Turn off HW AntDiv\n"));
			ODM_SetBBReg(pDM_Odm, ODM_REG_IGI_A_11N , BIT7, 0);	/* RegC50[7]=1'b1	enable HW AntDiv */
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA1_11N , BIT15, 0); /* Enable CCK AntDiv */
			if (pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)
				ODM_SetBBReg(pDM_Odm, ODM_REG_TX_ANT_CTRL_11N , BIT21, 0); /* Reg80c[21]=1'b0	from TX Reg */
			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;
		}
		return;
	} else {
		if (pDM_FatTable->bBecomeLinked ==false) {
			ODM_RT_TRACE(pDM_Odm,ODM_COMP_ANT_DIV, ODM_DBG_LOUD, ("Need to Turn on HW AntDiv\n"));
			/* Because HW AntDiv is disabled before Link, we enable HW AntDiv after link */
			ODM_SetBBReg(pDM_Odm, ODM_REG_IGI_A_11N , BIT7, 1);	/* RegC50[7]=1'b1	enable HW AntDiv */
			ODM_SetBBReg(pDM_Odm, ODM_REG_CCK_ANTDIV_PARA1_11N , BIT15, 1); /* Enable CCK AntDiv */
			if (pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV) {
#if TX_BY_REG
				ODM_SetBBReg(pDM_Odm, ODM_REG_TX_ANT_CTRL_11N , BIT21, 0); /* Reg80c[21]=1'b0	from Reg */
#else
				ODM_SetBBReg(pDM_Odm, ODM_REG_TX_ANT_CTRL_11N , BIT21, 1); /* Reg80c[21]=1'b1	from TX Info */
#endif
			}
			pDM_FatTable->bBecomeLinked = pDM_Odm->bLinked;
		}
	}



	if ((pDM_Odm->AntDivType == CG_TRX_HW_ANTDIV)||(pDM_Odm->AntDivType == CGCS_RX_HW_ANTDIV))
		odm_HWAntDiv(pDM_Odm);
}

/* 3============================================================ */
/* 3 Dynamic Primary CCA */
/* 3============================================================ */

void
odm_PrimaryCCA_Init(
		PDM_ODM_T		pDM_Odm)
{
	pPri_CCA_T		PrimaryCCA = &(pDM_Odm->DM_PriCCA);
	PrimaryCCA->DupRTS_flag = 0;
	PrimaryCCA->intf_flag = 0;
	PrimaryCCA->intf_type = 0;
	PrimaryCCA->Monitor_flag = 0;
	PrimaryCCA->PriCCA_flag = 0;
}

bool
ODM_DynamicPrimaryCCA_DupRTS(
		PDM_ODM_T		pDM_Odm
	)
{
	pPri_CCA_T		PrimaryCCA = &(pDM_Odm->DM_PriCCA);

	return	PrimaryCCA->DupRTS_flag;
}

void
odm_DynamicPrimaryCCA(
		PDM_ODM_T		pDM_Odm
	)
{
	struct adapter *Adapter =  pDM_Odm->Adapter;	/*  for NIC */
	prtl8192cd_priv	priv		= pDM_Odm->priv;	/*  for AP */
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	Pfalse_ALARM_STATISTICS		FalseAlmCnt = &(pDM_Odm->FalseAlmCnt);
	pPri_CCA_T		PrimaryCCA = &(pDM_Odm->DM_PriCCA);
	bool		Is40MHz;
	bool		Client_40MHz = false, Client_tmp = false;      /*  connected client BW */
	bool		bConnected = false;		/*  connected or not */
	static u8	Client_40MHz_pre = 0;
	static u64	lastTxOkCnt = 0;
	static u64	lastRxOkCnt = 0;
	static u32	Counter = 0;
	static u8	Delay = 1;
	u64		curTxOkCnt;
	u64		curRxOkCnt;
	u8		SecCHOffset;
	u8		i;

	return;
}
