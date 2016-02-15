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
#define _RTL8188E_PHYCFG_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <rtw_iol.h>
#include <rtl8188e_hal.h>

/*---------------------------Define Local Constant---------------------------*/
/* Channel switch:The size of command tables for switch channel*/
#define MAX_PRECMD_CNT 16
#define MAX_RFDEPENDCMD_CNT 16
#define MAX_POSTCMD_CNT 16

#define MAX_DOZE_WAITING_TIMES_9x 64

/*---------------------------Define Local Constant---------------------------*/


/*------------------------Define global variable-----------------------------*/

/*------------------------Define local variable------------------------------*/


/*--------------------Define export function prototype-----------------------*/
/*  Please refer to header file */
/*--------------------Define export function prototype-----------------------*/

/*----------------------------Function Body----------------------------------*/
/*  */
/*  1. BB register R/W API */
/*  */

/**
* Function:	phy_CalculateBitShift
*
* OverView:	Get shifted position of the BitMask
*
* Input:
*			u32		BitMask,
*
* Output:	none
* Return:		u32		Return the shift bit bit position of the mask
*/
static	u32
phy_CalculateBitShift(
	u32 BitMask
	)
{
	u32 i;

	for (i=0; i<=31; i++)
	{
		if ( ((BitMask>>i) &  0x1 ) == 1)
			break;
	}

	return (i);
}

#if (SIC_ENABLE == 1)
static bool
sic_IsSICReady(
	struct adapter *Adapter
	)
{
	bool		bRet=false;
	u32		retryCnt=0;
	u8		sic_cmd=0xff;

	while (1)
	{
		if (retryCnt++ >= SIC_MAX_POLL_CNT)
		{
			/* RTPRINT(FPHY, (PHY_SICR|PHY_SICW), ("[SIC], sic_IsSICReady() return false\n")); */
			return false;
		}

		/* if (RT_SDIO_CANNOT_IO(Adapter)) */
		/* 	return false; */

		sic_cmd = rtw_read8(Adapter, SIC_CMD_REG);
		/* sic_cmd = PlatformEFIORead1Byte(Adapter, SIC_CMD_REG); */
#if (SIC_HW_SUPPORT == 1)
		sic_cmd &= 0xf0;	/*  [7:4] */
#endif
		/* RTPRINT(FPHY, (PHY_SICR|PHY_SICW), ("[SIC], sic_IsSICReady(), readback 0x%x=0x%x\n", SIC_CMD_REG, sic_cmd)); */
		if (sic_cmd == SIC_CMD_READY)
			return true;
		else
		{
			rtw_msleep_os(1);
			/* delay_ms(1); */
		}
	}

	return bRet;
}

static u32
sic_Read4Byte(
	void *		Adapter,
	u32		offset
	)
{
	u32	u4ret=0xffffffff;
	u8	retry = 0;

	/* RTPRINT(FPHY, PHY_SICR, ("[SIC], sic_Read4Byte(): read offset(%#x)\n", offset)); */

	if (sic_IsSICReady(Adapter))
	{
#if (SIC_HW_SUPPORT == 1)
		rtw_write8(Adapter, SIC_CMD_REG, SIC_CMD_PREREAD);
		/* PlatformEFIOWrite1Byte(Adapter, SIC_CMD_REG, SIC_CMD_PREREAD); */
		/* RTPRINT(FPHY, PHY_SICR, ("write cmdreg 0x%x = 0x%x\n", SIC_CMD_REG, SIC_CMD_PREREAD)); */
#endif
		rtw_write8(Adapter, SIC_ADDR_REG, (u8)(offset&0xff));
		/* PlatformEFIOWrite1Byte(Adapter, SIC_ADDR_REG, (u8)(offset&0xff)); */
		/* RTPRINT(FPHY, PHY_SICR, ("write 0x%x = 0x%x\n", SIC_ADDR_REG, (u8)(offset&0xff))); */
		rtw_write8(Adapter, SIC_ADDR_REG+1, (u8)((offset&0xff00)>>8));
		/* PlatformEFIOWrite1Byte(Adapter, SIC_ADDR_REG+1, (u8)((offset&0xff00)>>8)); */
		/* RTPRINT(FPHY, PHY_SICR, ("write 0x%x = 0x%x\n", SIC_ADDR_REG+1, (u8)((offset&0xff00)>>8))); */
		rtw_write8(Adapter, SIC_CMD_REG, SIC_CMD_READ);
		/* PlatformEFIOWrite1Byte(Adapter, SIC_CMD_REG, SIC_CMD_READ); */
		/* RTPRINT(FPHY, PHY_SICR, ("write cmdreg 0x%x = 0x%x\n", SIC_CMD_REG, SIC_CMD_READ)); */

		retry = 4;
		while (retry--) {
			rtw_udelay_os(50);
			/* PlatformStallExecution(50); */
		}
		if (sic_IsSICReady(Adapter))
		{
			u4ret = rtw_read32(Adapter, SIC_DATA_REG);
			/* u4ret = PlatformEFIORead4Byte(Adapter, SIC_DATA_REG); */
			/* RTPRINT(FPHY, PHY_SICR, ("read 0x%x = 0x%x\n", SIC_DATA_REG, u4ret)); */
			/* DbgPrint("<===Read 0x%x = 0x%x\n", offset, u4ret); */
		}
	}

	return u4ret;
}

static void
sic_Write4Byte(
	void *		Adapter,
	u32		offset,
	u32		data
	)
{
	u8	retry = 6;

	if (sic_IsSICReady(Adapter))
	{
#if (SIC_HW_SUPPORT == 1)
		rtw_write8(Adapter, SIC_CMD_REG, SIC_CMD_PREWRITE);
#endif
		rtw_write8(Adapter, SIC_ADDR_REG, (u8)(offset&0xff));
		rtw_write8(Adapter, SIC_ADDR_REG+1, (u8)((offset&0xff00)>>8));
		rtw_write32(Adapter, SIC_DATA_REG, (u32)data);
		rtw_write8(Adapter, SIC_CMD_REG, SIC_CMD_WRITE);
		while (retry--) {
			rtw_udelay_os(50);
		}
	}
}
/*  */
/*  extern function */
/*  */
static void
SIC_SetBBReg(
	struct adapter *Adapter,
	u32		RegAddr,
	u32		BitMask,
	u32		Data
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u32			OriginalValue, BitShift;
	u16			BBWaitCounter = 0;

	/*  */
	/*  Critical section start */
	/*  */

	/* RTPRINT(FPHY, PHY_SICW, ("[SIC], SIC_SetBBReg(), mask=0x%x, addr[0x%x]=0x%x\n", BitMask, RegAddr, Data)); */

	if (BitMask!= bMaskDWord) {/* if not "double word" write */
		OriginalValue = sic_Read4Byte(Adapter, RegAddr);
		/* BitShift = sic_CalculateBitShift(BitMask); */
		BitShift = phy_CalculateBitShift(BitMask);
		Data = (((OriginalValue) & (~BitMask)) | (Data << BitShift));
	}

	sic_Write4Byte(Adapter, RegAddr, Data);

	/* PlatformAtomicExchange(&pHalData->bChangeBBInProgress, false); */
	/* RTPRINT(FPHY, PHY_SICW, ("[SIC], SIC_SetBBReg() end\n")); */
}

static u32
SIC_QueryBBReg(
	struct adapter *Adapter,
	u32		RegAddr,
	u32		BitMask
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u32			ReturnValue = 0, OriginalValue, BitShift;
	u16			BBWaitCounter = 0;

	/* RTPRINT(FPHY, PHY_SICR, ("[SIC], SIC_QueryBBReg() start\n")); */

	OriginalValue = sic_Read4Byte(Adapter, RegAddr);
	/* BitShift = sic_CalculateBitShift(BitMask); */
	BitShift = phy_CalculateBitShift(BitMask);
	ReturnValue = (OriginalValue & BitMask) >> BitShift;

	/* RTPRINT(FPHY, PHY_SICR, ("[SIC], SIC_QueryBBReg(), 0x%x=0x%x\n", RegAddr, OriginalValue)); */
	/* RTPRINT(FPHY, PHY_SICR, ("[SIC], SIC_QueryBBReg() end\n")); */

	/* PlatformAtomicExchange(&pHalData->bChangeBBInProgress, false); */
	return (ReturnValue);
}

void
SIC_Init(
	struct adapter *Adapter
	)
{
	/*  Here we need to write 0x1b8~0x1bf = 0 after fw is downloaded */
	/*  because for 8723E at beginning 0x1b8=0x1e, that will cause */
	/*  sic always not be ready */
#if (SIC_HW_SUPPORT == 1)
	/* RTPRINT(FPHY, PHY_SICR, ("[SIC], SIC_Init(), write 0x%x = 0x%x\n", */
	/* 	SIC_INIT_REG, SIC_INIT_VAL)); */
	rtw_write8(Adapter, SIC_INIT_REG, SIC_INIT_VAL);
	/* PlatformEFIOWrite1Byte(Adapter, SIC_INIT_REG, SIC_INIT_VAL); */
	/* RTPRINT(FPHY, PHY_SICR, ("[SIC], SIC_Init(), write 0x%x = 0x%x\n", */
	/* 	SIC_CMD_REG, SIC_CMD_INIT)); */
	rtw_write8(Adapter, SIC_CMD_REG, SIC_CMD_INIT);
	/* PlatformEFIOWrite1Byte(Adapter, SIC_CMD_REG, SIC_CMD_INIT); */
#else
	/* RTPRINT(FPHY, PHY_SICR, ("[SIC], SIC_Init(), write 0x1b8~0x1bf = 0x0\n")); */
	rtw_write32(Adapter, SIC_CMD_REG, 0);
	/* PlatformEFIOWrite4Byte(Adapter, SIC_CMD_REG, 0); */
	rtw_write32(Adapter, SIC_CMD_REG+4, 0);
	/* PlatformEFIOWrite4Byte(Adapter, SIC_CMD_REG+4, 0); */
#endif
}

static bool
SIC_LedOff(
	struct adapter *Adapter
	)
{
	/*  When SIC is enabled, led pin will be used as debug pin, */
	/*  so don't execute led function when SIC is enabled. */
	return true;
}
#endif

/**
* Function:	PHY_QueryBBReg
*
* OverView:	Read "sepcific bits" from BB register
*
* Input:
*			struct adapter *	Adapter,
*			u32			RegAddr,	 The target address to be readback
*			u32			BitMask		 The target bit position in the target address
*										to be readback
* Output:	None
* Return:		u32			Data		The readback register value
* Note:		This function is equal to "GetRegSetting" in PHY programming guide
*/
u32
rtl8188e_PHY_QueryBBReg(
	struct adapter *Adapter,
	u32		RegAddr,
	u32		BitMask
	)
{
	u32	ReturnValue = 0, OriginalValue, BitShift;
	u16	BBWaitCounter = 0;

#if (DISABLE_BB_RF == 1)
	return 0;
#endif

#if (SIC_ENABLE == 1)
	return SIC_QueryBBReg(Adapter, RegAddr, BitMask);
#endif

	/* RT_TRACE(COMP_RF, DBG_TRACE, ("--->PHY_QueryBBReg(): RegAddr(%#lx), BitMask(%#lx)\n", RegAddr, BitMask)); */

	OriginalValue = rtw_read32(Adapter, RegAddr);
	BitShift = phy_CalculateBitShift(BitMask);
	ReturnValue = (OriginalValue & BitMask) >> BitShift;
	return (ReturnValue);
}


/**
* Function:	PHY_SetBBReg
*
* OverView:	Write "Specific bits" to BB register (page 8~)
*
* Input:
*			struct adapter *	Adapter,
*			u32			RegAddr,	 The target address to be modified
*			u32			BitMask		 The target bit position in the target address
*										to be modified
*			u32			Data		The new register value in the target bit position
*										of the target address
*
* Output:	None
* Return:		None
* Note:		This function is equal to "PutRegSetting" in PHY programming guide
*/

void
rtl8188e_PHY_SetBBReg(
	struct adapter *Adapter,
	u32		RegAddr,
	u32		BitMask,
	u32		Data
	)
{
	HAL_DATA_TYPE	*pHalData		= GET_HAL_DATA(Adapter);
	/* u16			BBWaitCounter	= 0; */
	u32			OriginalValue, BitShift;

#if (DISABLE_BB_RF == 1)
	return;
#endif

#if (SIC_ENABLE == 1)
	SIC_SetBBReg(Adapter, RegAddr, BitMask, Data);
	return;
#endif

	if (BitMask!= bMaskDWord) {/* if not "double word" write */
		OriginalValue = rtw_read32(Adapter, RegAddr);
		BitShift = phy_CalculateBitShift(BitMask);
		Data = ((OriginalValue & (~BitMask)) | ((Data << BitShift) & BitMask));
	}

	rtw_write32(Adapter, RegAddr, Data);
}


/*  */
/*  2. RF register R/W API */
/*  */
/**
* Function:	phy_RFSerialRead
*
* OverView:	Read regster from RF chips
*
* Input:
*			struct adapter *	Adapter,
*			enum rf_radio_path	eRFPath,	Radio path of A/B/C/D
*			u32			Offset,		The target address to be read
*
* Output:	None
* Return:		u32			reback value
* Note:		Threre are three types of serial operations:
*			1. Software serial write
*			2. Hardware LSSI-Low Speed Serial Interface
*			3. Hardware HSSI-High speed
*			serial write. Driver need to implement (1) and (2).
*			This function is equal to the combination of RF_ReadReg() and  RFLSSIRead()
*/
static	u32
phy_RFSerialRead(
	struct adapter *		Adapter,
	enum rf_radio_path	eRFPath,
	u32				Offset
	)
{
	u32						retValue = 0;
	HAL_DATA_TYPE				*pHalData = GET_HAL_DATA(Adapter);
	BB_REGISTER_DEFINITION_T	*pPhyReg = &pHalData->PHYRegDef[eRFPath];
	u32						NewOffset;
	u32						tmplong,tmplong2;
	u8					RfPiEnable=0;

	/*  */
	/*  Make sure RF register offset is correct */
	/*  */
	Offset &= 0xff;

	/*  */
	/*  Switch page for 8256 RF IC */
	/*  */
	NewOffset = Offset;

	/*  2009/06/17 MH We can not execute IO for power save or other accident mode. */
	/* if (RT_CANNOT_IO(Adapter)) */
	/*  */
	/* 	RTPRINT(FPHY, PHY_RFR, ("phy_RFSerialRead return all one\n")); */
	/* 	return	0xFFFFFFFF; */
	/*  */

	/*  For 92S LSSI Read RFLSSIRead */
	/*  For RF A/B write 0x824/82c(does not work in the future) */
	/*  We must use 0x824 for RF A and B to execute read trigger */
	tmplong = PHY_QueryBBReg(Adapter, rFPGA0_XA_HSSIParameter2, bMaskDWord);
	if (eRFPath == RF_PATH_A)
		tmplong2 = tmplong;
	else
		tmplong2 = PHY_QueryBBReg(Adapter, pPhyReg->rfHSSIPara2, bMaskDWord);

	tmplong2 = (tmplong2 & (~bLSSIReadAddress)) | (NewOffset<<23) | bLSSIReadEdge;	/* T65 RF */

	PHY_SetBBReg(Adapter, rFPGA0_XA_HSSIParameter2, bMaskDWord, tmplong&(~bLSSIReadEdge));
	rtw_udelay_os(10);/*  PlatformStallExecution(10); */

	PHY_SetBBReg(Adapter, pPhyReg->rfHSSIPara2, bMaskDWord, tmplong2);
	rtw_udelay_os(100);/* PlatformStallExecution(100); */

	/* PHY_SetBBReg(Adapter, rFPGA0_XA_HSSIParameter2, bMaskDWord, tmplong|bLSSIReadEdge); */
	rtw_udelay_os(10);/* PlatformStallExecution(10); */

	if (eRFPath == RF_PATH_A)
		RfPiEnable = (u8)PHY_QueryBBReg(Adapter, rFPGA0_XA_HSSIParameter1, BIT8);
	else if (eRFPath == RF_PATH_B)
		RfPiEnable = (u8)PHY_QueryBBReg(Adapter, rFPGA0_XB_HSSIParameter1, BIT8);

	if (RfPiEnable)
	{	/*  Read from BBreg8b8, 12 bits for 8190, 20bits for T65 RF */
		retValue = PHY_QueryBBReg(Adapter, pPhyReg->rfLSSIReadBackPi, bLSSIReadBackData);
		/* DBG_8192C("Readback from RF-PI : 0x%x\n", retValue); */
	}
	else
	{	/* Read from BBreg8a0, 12 bits for 8190, 20 bits for T65 RF */
		retValue = PHY_QueryBBReg(Adapter, pPhyReg->rfLSSIReadBack, bLSSIReadBackData);
		/* DBG_8192C("Readback from RF-SI : 0x%x\n", retValue); */
	}
	/* DBG_8192C("RFR-%d Addr[0x%x]=0x%x\n", eRFPath, pPhyReg->rfLSSIReadBack, retValue); */

	return retValue;

}



/**
* Function:	phy_RFSerialWrite
*
* OverView:	Write data to RF register (page 8~)
*
* Input:
*			struct adapter *	Adapter,
*			enum rf_radio_path	eRFPath,	Radio path of A/B/C/D
*			u32			Offset,		The target address to be read
*			u32			Data		The new register Data in the target bit position
*										of the target to be read
*
* Output:	None
* Return:		None
* Note:		Threre are three types of serial operations:
*			1. Software serial write
*			2. Hardware LSSI-Low Speed Serial Interface
*			3. Hardware HSSI-High speed
*			serial write. Driver need to implement (1) and (2).
*			This function is equal to the combination of RF_ReadReg() and  RFLSSIRead()
 *
 * Note:		  For RF8256 only
 *			 The total count of RTL8256(Zebra4) register is around 36 bit it only employs
 *			 4-bit RF address. RTL8256 uses "register mode control bit" (Reg00[12], Reg00[10])
 *			 to access register address bigger than 0xf. See "Appendix-4 in PHY Configuration
 *			 programming guide" for more details.
 *			 Thus, we define a sub-finction for RTL8526 register address conversion
 *		       ===========================================================
 *			 Register Mode		RegCTL[1]		RegCTL[0]		Note
 *								(Reg00[12])		(Reg00[10])
 *		       ===========================================================
 *			 Reg_Mode0				0				x			Reg 0 ~15(0x0 ~ 0xf)
 *		       ------------------------------------------------------------------
 *			 Reg_Mode1				1				0			Reg 16 ~30(0x1 ~ 0xf)
 *		       ------------------------------------------------------------------
 *			 Reg_Mode2				1				1			Reg 31 ~ 45(0x1 ~ 0xf)
 *		       ------------------------------------------------------------------
 *
 *	2008/09/02	MH	Add 92S RF definition
 *
 *
 *
*/
static	void
phy_RFSerialWrite(
	struct adapter *		Adapter,
	enum rf_radio_path	eRFPath,
	u32				Offset,
	u32				Data
	)
{
	u32						DataAndAddr = 0;
	HAL_DATA_TYPE				*pHalData = GET_HAL_DATA(Adapter);
	BB_REGISTER_DEFINITION_T	*pPhyReg = &pHalData->PHYRegDef[eRFPath];
	u32						NewOffset;

	Offset &= 0xff;

	/*  */
	/*  Switch page for 8256 RF IC */
	/*  */
	NewOffset = Offset;

	/*  */
	/*  Put write addr in [5:0]  and write data in [31:16] */
	/*  */
	DataAndAddr = ((NewOffset<<20) | (Data&0x000fffff)) & 0x0fffffff;	/*  T65 RF */

	/*  */
	/*  Write Operation */
	/*  */
	PHY_SetBBReg(Adapter, pPhyReg->rf3wireOffset, bMaskDWord, DataAndAddr);
}

/**
* Function:	PHY_QueryRFReg
*
* OverView:	Query "Specific bits" to RF register (page 8~)
*
* Input:
*			struct adapter *	Adapter,
*			enum rf_radio_path	eRFPath,	Radio path of A/B/C/D
*			u32			RegAddr,	The target address to be read
*			u32			BitMask		The target bit position in the target address
*										to be read
*
* Output:	None
* Return:		u32			Readback value
* Note:		This function is equal to "GetRFRegSetting" in PHY programming guide
*/
u32 rtl8188e_PHY_QueryRFReg(struct adapter *Adapter, enum rf_radio_path eRFPath, u32 RegAddr, u32 BitMask)
{
	u32 Original_Value, Readback_Value, BitShift;

#if (DISABLE_BB_RF == 1)
	return 0;
#endif

	Original_Value = phy_RFSerialRead(Adapter, eRFPath, RegAddr);

	BitShift =  phy_CalculateBitShift(BitMask);
	Readback_Value = (Original_Value & BitMask) >> BitShift;

	return (Readback_Value);
}

/**
* Function:	PHY_SetRFReg
*
* OverView:	Write "Specific bits" to RF register (page 8~)
*
* Input:
*			struct adapter *	Adapter,
*			enum rf_radio_path	eRFPath,	Radio path of A/B/C/D
*			u32			RegAddr,	The target address to be modified
*			u32			BitMask		The target bit position in the target address
*								to be modified
*			u32			Data		The new register Data in the target bit position
*								of the target address
*
* Output:	None
* Return:		None
* Note:		This function is equal to "PutRFRegSetting" in PHY programming guide
*/
void
rtl8188e_PHY_SetRFReg(
	struct adapter *		Adapter,
	enum rf_radio_path	eRFPath,
	u32				RegAddr,
	u32				BitMask,
	u32				Data
	)
{

	/* HAL_DATA_TYPE	*pHalData		= GET_HAL_DATA(Adapter); */
	/* u8			RFWaitCounter	= 0; */
	u32		Original_Value, BitShift;
	/* _irqL	irqL; */

#if (DISABLE_BB_RF == 1)
	return;
#endif

	/*  RF data is 12 bits only */
	if (BitMask != bRFRegOffsetMask)
	{
		Original_Value = phy_RFSerialRead(Adapter, eRFPath, RegAddr);
		BitShift =  phy_CalculateBitShift(BitMask);
		Data = ((Original_Value & (~BitMask)) | (Data<< BitShift));
	}

	phy_RFSerialWrite(Adapter, eRFPath, RegAddr, Data);
}


/*  */
/*  3. Initial MAC/BB/RF config by reading MAC/BB/RF txt. */
/*  */

/*-----------------------------------------------------------------------------
 * Function:    phy_ConfigMACWithParaFile()
 *
 * Overview:    This function read BB parameters from general file format, and do register
 *			  Read/Write
 *
 * Input:	struct adapter *	Adapter
 *			s8 *				pFileName
 *
 * Output:      NONE
 *
 * Return:      RT_STATUS_SUCCESS: configuration file exist
 *
 * Note:		The format of MACPHY_REG.txt is different from PHY and RF.
 *			[Register][Mask][Value]
 *---------------------------------------------------------------------------*/
static	int
phy_ConfigMACWithParaFile(
	struct adapter *	Adapter,
	u8*			pFileName
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	int		rtStatus = _FAIL;

	return rtStatus;
}

/*-----------------------------------------------------------------------------
 * Function:    PHY_MACConfig8192C
 *
 * Overview:	Condig MAC by header file or parameter file.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 *  When		Who		Remark
 *  08/12/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
s32 PHY_MACConfig8188E(struct adapter *Adapter)
{
	int		rtStatus = _SUCCESS;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	s8			*pszMACRegFile;
	s8			sz8188EMACRegFile[] = RTL8188E_PHY_MACREG;

	pszMACRegFile = sz8188EMACRegFile;

	/*  Config MAC */
	if (HAL_STATUS_FAILURE == ODM_ConfigMACWithHeaderFile(&pHalData->odmpriv))
		rtStatus = _FAIL;

	/*  2010.07.13 AMPDU aggregation number B */
	rtw_write8(Adapter, REG_MAX_AGGR_NUM, MAX_AGGR_NUM);
	/* rtw_write8(Adapter, REG_MAX_AGGR_NUM, 0x0B); */

	return rtStatus;

}


/**
* Function:	phy_InitBBRFRegisterDefinition
*
* OverView:	Initialize Register definition offset for Radio Path A/B/C/D
*
* Input:
*			struct adapter *	Adapter,
*
* Output:	None
* Return:		None
* Note:		The initialization value is constant and it should never be changes
*/
static	void
phy_InitBBRFRegisterDefinition(
	struct adapter *	Adapter
)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);

	/*  RF Interface Sowrtware Control */
	pHalData->PHYRegDef[RF_PATH_A].rfintfs = rFPGA0_XAB_RFInterfaceSW; /*  16 LSBs if read 32-bit from 0x870 */
	pHalData->PHYRegDef[RF_PATH_B].rfintfs = rFPGA0_XAB_RFInterfaceSW; /*  16 MSBs if read 32-bit from 0x870 (16-bit for 0x872) */
	pHalData->PHYRegDef[RF_PATH_C].rfintfs = rFPGA0_XCD_RFInterfaceSW;/*  16 LSBs if read 32-bit from 0x874 */
	pHalData->PHYRegDef[RF_PATH_D].rfintfs = rFPGA0_XCD_RFInterfaceSW;/*  16 MSBs if read 32-bit from 0x874 (16-bit for 0x876) */

	/*  RF Interface Readback Value */
	pHalData->PHYRegDef[RF_PATH_A].rfintfi = rFPGA0_XAB_RFInterfaceRB; /*  16 LSBs if read 32-bit from 0x8E0 */
	pHalData->PHYRegDef[RF_PATH_B].rfintfi = rFPGA0_XAB_RFInterfaceRB;/*  16 MSBs if read 32-bit from 0x8E0 (16-bit for 0x8E2) */
	pHalData->PHYRegDef[RF_PATH_C].rfintfi = rFPGA0_XCD_RFInterfaceRB;/*  16 LSBs if read 32-bit from 0x8E4 */
	pHalData->PHYRegDef[RF_PATH_D].rfintfi = rFPGA0_XCD_RFInterfaceRB;/*  16 MSBs if read 32-bit from 0x8E4 (16-bit for 0x8E6) */

	/*  RF Interface Output (and Enable) */
	pHalData->PHYRegDef[RF_PATH_A].rfintfo = rFPGA0_XA_RFInterfaceOE; /*  16 LSBs if read 32-bit from 0x860 */
	pHalData->PHYRegDef[RF_PATH_B].rfintfo = rFPGA0_XB_RFInterfaceOE; /*  16 LSBs if read 32-bit from 0x864 */

	/*  RF Interface (Output and)  Enable */
	pHalData->PHYRegDef[RF_PATH_A].rfintfe = rFPGA0_XA_RFInterfaceOE; /*  16 MSBs if read 32-bit from 0x860 (16-bit for 0x862) */
	pHalData->PHYRegDef[RF_PATH_B].rfintfe = rFPGA0_XB_RFInterfaceOE; /*  16 MSBs if read 32-bit from 0x864 (16-bit for 0x866) */

	/* Addr of LSSI. Wirte RF register by driver */
	pHalData->PHYRegDef[RF_PATH_A].rf3wireOffset = rFPGA0_XA_LSSIParameter; /* LSSI Parameter */
	pHalData->PHYRegDef[RF_PATH_B].rf3wireOffset = rFPGA0_XB_LSSIParameter;

	/*  RF parameter */
	pHalData->PHYRegDef[RF_PATH_A].rfLSSI_Select = rFPGA0_XAB_RFParameter;  /* BB Band Select */
	pHalData->PHYRegDef[RF_PATH_B].rfLSSI_Select = rFPGA0_XAB_RFParameter;
	pHalData->PHYRegDef[RF_PATH_C].rfLSSI_Select = rFPGA0_XCD_RFParameter;
	pHalData->PHYRegDef[RF_PATH_D].rfLSSI_Select = rFPGA0_XCD_RFParameter;

	/*  Tx AGC Gain Stage (same for all path. Should we remove this?) */
	pHalData->PHYRegDef[RF_PATH_A].rfTxGainStage = rFPGA0_TxGainStage; /* Tx gain stage */
	pHalData->PHYRegDef[RF_PATH_B].rfTxGainStage = rFPGA0_TxGainStage; /* Tx gain stage */
	pHalData->PHYRegDef[RF_PATH_C].rfTxGainStage = rFPGA0_TxGainStage; /* Tx gain stage */
	pHalData->PHYRegDef[RF_PATH_D].rfTxGainStage = rFPGA0_TxGainStage; /* Tx gain stage */

	/*  Tranceiver A~D HSSI Parameter-1 */
	pHalData->PHYRegDef[RF_PATH_A].rfHSSIPara1 = rFPGA0_XA_HSSIParameter1;  /* wire control parameter1 */
	pHalData->PHYRegDef[RF_PATH_B].rfHSSIPara1 = rFPGA0_XB_HSSIParameter1;  /* wire control parameter1 */

	/*  Tranceiver A~D HSSI Parameter-2 */
	pHalData->PHYRegDef[RF_PATH_A].rfHSSIPara2 = rFPGA0_XA_HSSIParameter2;  /* wire control parameter2 */
	pHalData->PHYRegDef[RF_PATH_B].rfHSSIPara2 = rFPGA0_XB_HSSIParameter2;  /* wire control parameter2 */

	/*  RF switch Control */
	pHalData->PHYRegDef[RF_PATH_A].rfSwitchControl = rFPGA0_XAB_SwitchControl; /* TR/Ant switch control */
	pHalData->PHYRegDef[RF_PATH_B].rfSwitchControl = rFPGA0_XAB_SwitchControl;
	pHalData->PHYRegDef[RF_PATH_C].rfSwitchControl = rFPGA0_XCD_SwitchControl;
	pHalData->PHYRegDef[RF_PATH_D].rfSwitchControl = rFPGA0_XCD_SwitchControl;

	/*  AGC control 1 */
	pHalData->PHYRegDef[RF_PATH_A].rfAGCControl1 = rOFDM0_XAAGCCore1;
	pHalData->PHYRegDef[RF_PATH_B].rfAGCControl1 = rOFDM0_XBAGCCore1;
	pHalData->PHYRegDef[RF_PATH_C].rfAGCControl1 = rOFDM0_XCAGCCore1;
	pHalData->PHYRegDef[RF_PATH_D].rfAGCControl1 = rOFDM0_XDAGCCore1;

	/*  AGC control 2 */
	pHalData->PHYRegDef[RF_PATH_A].rfAGCControl2 = rOFDM0_XAAGCCore2;
	pHalData->PHYRegDef[RF_PATH_B].rfAGCControl2 = rOFDM0_XBAGCCore2;
	pHalData->PHYRegDef[RF_PATH_C].rfAGCControl2 = rOFDM0_XCAGCCore2;
	pHalData->PHYRegDef[RF_PATH_D].rfAGCControl2 = rOFDM0_XDAGCCore2;

	/*  RX AFE control 1 */
	pHalData->PHYRegDef[RF_PATH_A].rfRxIQImbalance = rOFDM0_XARxIQImbalance;
	pHalData->PHYRegDef[RF_PATH_B].rfRxIQImbalance = rOFDM0_XBRxIQImbalance;
	pHalData->PHYRegDef[RF_PATH_C].rfRxIQImbalance = rOFDM0_XCRxIQImbalance;
	pHalData->PHYRegDef[RF_PATH_D].rfRxIQImbalance = rOFDM0_XDRxIQImbalance;

	/*  RX AFE control 1 */
	pHalData->PHYRegDef[RF_PATH_A].rfRxAFE = rOFDM0_XARxAFE;
	pHalData->PHYRegDef[RF_PATH_B].rfRxAFE = rOFDM0_XBRxAFE;
	pHalData->PHYRegDef[RF_PATH_C].rfRxAFE = rOFDM0_XCRxAFE;
	pHalData->PHYRegDef[RF_PATH_D].rfRxAFE = rOFDM0_XDRxAFE;

	/*  Tx AFE control 1 */
	pHalData->PHYRegDef[RF_PATH_A].rfTxIQImbalance = rOFDM0_XATxIQImbalance;
	pHalData->PHYRegDef[RF_PATH_B].rfTxIQImbalance = rOFDM0_XBTxIQImbalance;
	pHalData->PHYRegDef[RF_PATH_C].rfTxIQImbalance = rOFDM0_XCTxIQImbalance;
	pHalData->PHYRegDef[RF_PATH_D].rfTxIQImbalance = rOFDM0_XDTxIQImbalance;

	/*  Tx AFE control 2 */
	pHalData->PHYRegDef[RF_PATH_A].rfTxAFE = rOFDM0_XATxAFE;
	pHalData->PHYRegDef[RF_PATH_B].rfTxAFE = rOFDM0_XBTxAFE;
	pHalData->PHYRegDef[RF_PATH_C].rfTxAFE = rOFDM0_XCTxAFE;
	pHalData->PHYRegDef[RF_PATH_D].rfTxAFE = rOFDM0_XDTxAFE;

	/*  Tranceiver LSSI Readback SI mode */
	pHalData->PHYRegDef[RF_PATH_A].rfLSSIReadBack = rFPGA0_XA_LSSIReadBack;
	pHalData->PHYRegDef[RF_PATH_B].rfLSSIReadBack = rFPGA0_XB_LSSIReadBack;
	pHalData->PHYRegDef[RF_PATH_C].rfLSSIReadBack = rFPGA0_XC_LSSIReadBack;
	pHalData->PHYRegDef[RF_PATH_D].rfLSSIReadBack = rFPGA0_XD_LSSIReadBack;

	/*  Tranceiver LSSI Readback PI mode */
	pHalData->PHYRegDef[RF_PATH_A].rfLSSIReadBackPi = TransceiverA_HSPI_Readback;
	pHalData->PHYRegDef[RF_PATH_B].rfLSSIReadBackPi = TransceiverB_HSPI_Readback;
	/* pHalData->PHYRegDef[RF_PATH_C].rfLSSIReadBackPi = rFPGA0_XC_LSSIReadBack; */
	/* pHalData->PHYRegDef[RF_PATH_D].rfLSSIReadBackPi = rFPGA0_XD_LSSIReadBack; */

}


/*-----------------------------------------------------------------------------
 * Function:    phy_ConfigBBWithParaFile()
 *
 * Overview:    This function read BB parameters from general file format, and do register
 *			  Read/Write
 *
 * Input:	struct adapter *	Adapter
 *			s8 *				pFileName
 *
 * Output:      NONE
 *
 * Return:      RT_STATUS_SUCCESS: configuration file exist
 *	2008/11/06	MH	For 92S we do not support silent reset now. Disable
 *					parameter file compare!!!!!!??
 *
 *---------------------------------------------------------------------------*/
static	int
phy_ConfigBBWithParaFile(
	struct adapter *	Adapter,
	u8*			pFileName
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	int		rtStatus = _SUCCESS;

	return rtStatus;
}



/*  */
/*  The following is for High Power PA */
/*  */
static void
phy_ConfigBBExternalPA(
	struct adapter *		Adapter
)
{
}

void
storePwrIndexDiffRateOffset(
	struct adapter *Adapter,
	u32		RegAddr,
	u32		BitMask,
	u32		Data
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	if (RegAddr == rTxAGC_A_Rate18_06)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][0] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][0]-TxAGC_A_Rate18_06 = 0x%x\n", pHalData->pwrGroupCnt, */
		/* 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][0]); */
	}
	if (RegAddr == rTxAGC_A_Rate54_24)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][1] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][1]-TxAGC_A_Rate54_24 = 0x%x\n", pHalData->pwrGroupCnt, */
		/* 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][1]); */
	}
	if (RegAddr == rTxAGC_A_CCK1_Mcs32)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][6] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][6]-TxAGC_A_CCK1_Mcs32 = 0x%x\n", pHalData->pwrGroupCnt, */
		/* 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][6]); */
	}
	if (RegAddr == rTxAGC_B_CCK11_A_CCK2_11 && BitMask == 0xffffff00)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][7] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][7]-TxAGC_B_CCK11_A_CCK2_11 = 0x%x\n", pHalData->pwrGroupCnt, */
		/* 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][7]); */
	}
	if (RegAddr == rTxAGC_A_Mcs03_Mcs00)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][2] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][2]-TxAGC_A_Mcs03_Mcs00 = 0x%x\n", pHalData->pwrGroupCnt, */
		/* 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][2]); */
	}
	if (RegAddr == rTxAGC_A_Mcs07_Mcs04)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][3] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][3]-TxAGC_A_Mcs07_Mcs04 = 0x%x\n", pHalData->pwrGroupCnt, */
		/* 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][3]); */
	}
	if (RegAddr == rTxAGC_A_Mcs11_Mcs08)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][4] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][4]-TxAGC_A_Mcs11_Mcs08 = 0x%x\n", pHalData->pwrGroupCnt, */
		/* 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][4]); */
	}
	if (RegAddr == rTxAGC_A_Mcs15_Mcs12)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][5] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][5]-TxAGC_A_Mcs15_Mcs12 = 0x%x\n", pHalData->pwrGroupCnt,pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][5]); */
		if (pHalData->rf_type== RF_1T1R)
		{
			/* printk("pwrGroupCnt = %d\n", pHalData->pwrGroupCnt); */
			pHalData->pwrGroupCnt++;
		}
	}
	if (RegAddr == rTxAGC_B_Rate18_06)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][8] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][8]-TxAGC_B_Rate18_06 = 0x%x\n", pHalData->pwrGroupCnt, */
		/* 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][8]); */
	}
	if (RegAddr == rTxAGC_B_Rate54_24)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][9] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][9]-TxAGC_B_Rate54_24 = 0x%x\n", pHalData->pwrGroupCnt, */
		/* 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][9]); */
	}
	if (RegAddr == rTxAGC_B_CCK1_55_Mcs32)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][14] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][14]-TxAGC_B_CCK1_55_Mcs32 = 0x%x\n", pHalData->pwrGroupCnt, */
		/* 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][14]); */
	}
	if (RegAddr == rTxAGC_B_CCK11_A_CCK2_11 && BitMask == 0x000000ff)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][15] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][15]-TxAGC_B_CCK11_A_CCK2_11 = 0x%x\n", pHalData->pwrGroupCnt, */
		/* 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][15]); */
	}
	if (RegAddr == rTxAGC_B_Mcs03_Mcs00)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][10] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][10]-TxAGC_B_Mcs03_Mcs00 = 0x%x\n", pHalData->pwrGroupCnt, */
		/* 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][10]); */
	}
	if (RegAddr == rTxAGC_B_Mcs07_Mcs04)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][11] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][11]-TxAGC_B_Mcs07_Mcs04 = 0x%x\n", pHalData->pwrGroupCnt, */
		/* 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][11]); */
	}
	if (RegAddr == rTxAGC_B_Mcs11_Mcs08)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][12] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][12]-TxAGC_B_Mcs11_Mcs08 = 0x%x\n", pHalData->pwrGroupCnt, */
		/* 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][12]); */
	}
	if (RegAddr == rTxAGC_B_Mcs15_Mcs12)
	{
		pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][13] = Data;
		/* printk("MCSTxPowerLevelOriginalOffset[%d][13]-TxAGC_B_Mcs15_Mcs12 = 0x%x\n", pHalData->pwrGroupCnt, */
		/* 	pHalData->MCSTxPowerLevelOriginalOffset[pHalData->pwrGroupCnt][13]); */

		if (pHalData->rf_type != RF_1T1R)
		{
			/* printk("pwrGroupCnt = %d\n", pHalData->pwrGroupCnt); */
			pHalData->pwrGroupCnt++;
		}
	}
}
/*-----------------------------------------------------------------------------
 * Function:	phy_ConfigBBWithPgParaFile
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
 * 11/06/2008	MHC		Create Version 0.
 * 2009/07/29	tynli		(porting from 92SE branch)2009/03/11 Add copy parameter file to buffer for silent reset
 *---------------------------------------------------------------------------*/
static	int
phy_ConfigBBWithPgParaFile(
	struct adapter *	Adapter,
	u8*			pFileName)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	int		rtStatus = _SUCCESS;


	return rtStatus;

}	/* phy_ConfigBBWithPgParaFile */

static void
phy_BB8192C_Config_1T(
	struct adapter *Adapter
	)
{
	/* for path - B */
	PHY_SetBBReg(Adapter, rFPGA0_TxInfo, 0x3, 0x2);
	PHY_SetBBReg(Adapter, rFPGA1_TxInfo, 0x300033, 0x200022);

	/*  20100519 Joseph: Add for 1T2R config. Suggested by Kevin, Jenyu and Yunan. */
	PHY_SetBBReg(Adapter, rCCK0_AFESetting, bMaskByte3, 0x45);
	PHY_SetBBReg(Adapter, rOFDM0_TRxPathEnable, bMaskByte0, 0x23);
	PHY_SetBBReg(Adapter, rOFDM0_AGCParameter1, 0x30, 0x1);	/*  B path first AGC */

	PHY_SetBBReg(Adapter, 0xe74, 0x0c000000, 0x2);
	PHY_SetBBReg(Adapter, 0xe78, 0x0c000000, 0x2);
	PHY_SetBBReg(Adapter, 0xe7c, 0x0c000000, 0x2);
	PHY_SetBBReg(Adapter, 0xe80, 0x0c000000, 0x2);
	PHY_SetBBReg(Adapter, 0xe88, 0x0c000000, 0x2);


}

/*  Joseph test: new initialize order!! */
/*  Test only!! This part need to be re-organized. */
/*  Now it is just for 8256. */
static	int
phy_BB8190_Config_HardCode(
	struct adapter *Adapter
	)
{
	/* RT_ASSERT(false, ("This function is not implement yet!!\n")); */
	return _SUCCESS;
}

static	int
phy_BB8188E_Config_ParaFile(
	struct adapter *Adapter
	)
{
	struct eeprom_priv *pEEPROM = GET_EEPROM_EFUSE_PRIV(Adapter);
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	int			rtStatus = _SUCCESS;

	u8	sz8188EBBRegFile[] = RTL8188E_PHY_REG;
	u8	sz8188EAGCTableFile[] = RTL8188E_AGC_TAB;
	u8	sz8188EBBRegPgFile[] = RTL8188E_PHY_REG_PG;
	u8	sz8188EBBRegMpFile[] = RTL8188E_PHY_REG_MP;

	u8	*pszBBRegFile = NULL, *pszAGCTableFile = NULL, *pszBBRegPgFile = NULL, *pszBBRegMpFile= NULL;


	/* RT_TRACE(COMP_INIT, DBG_TRACE, ("==>phy_BB8192S_Config_ParaFile\n")); */

	pszBBRegFile = sz8188EBBRegFile ;
	pszAGCTableFile = sz8188EAGCTableFile;
	pszBBRegPgFile = sz8188EBBRegPgFile;
	pszBBRegMpFile = sz8188EBBRegMpFile;

	/*  */
	/*  1. Read PHY_REG.TXT BB INIT!! */
	/*  We will seperate as 88C / 92C according to chip version */
	/*  */
	if (HAL_STATUS_FAILURE ==ODM_ConfigBBWithHeaderFile(&pHalData->odmpriv, CONFIG_BB_PHY_REG))
		rtStatus = _FAIL;

	if (rtStatus != _SUCCESS) {
		/* RT_TRACE(COMP_INIT, DBG_SERIOUS, ("phy_BB8192S_Config_ParaFile():Write BB Reg Fail!!")); */
		goto phy_BB8190_Config_ParaFile_Fail;
	}

	/*  */
	/*  20100318 Joseph: Config 2T2R to 1T2R if necessary. */
	/*  */
	/* if (pHalData->rf_type == RF_1T2R) */
	/*  */
		/* phy_BB8192C_Config_1T(Adapter); */
		/* DBG_8192C("phy_BB8188E_Config_ParaFile():Config to 1T!!\n"); */
	/*  */

	/*  */
	/*  2. If EEPROM or EFUSE autoload OK, We must config by PHY_REG_PG.txt */
	/*  */
	if (pEEPROM->bautoload_fail_flag == false) {
		pHalData->pwrGroupCnt = 0;

		if (HAL_STATUS_FAILURE ==ODM_ConfigBBWithHeaderFile(&pHalData->odmpriv, CONFIG_BB_PHY_REG_PG))
			rtStatus = _FAIL;
	}

	if (rtStatus != _SUCCESS) {
		/* RT_TRACE(COMP_INIT, DBG_SERIOUS, ("phy_BB8192S_Config_ParaFile():BB_PG Reg Fail!!")); */
		goto phy_BB8190_Config_ParaFile_Fail;
	}

	/*  3. BB AGC table Initialization */
	if (HAL_STATUS_FAILURE ==ODM_ConfigBBWithHeaderFile(&pHalData->odmpriv,  CONFIG_BB_AGC_TAB))
		rtStatus = _FAIL;
	if (rtStatus != _SUCCESS) {
		/* RT_TRACE(COMP_FPGA, DBG_SERIOUS, ("phy_BB8192S_Config_ParaFile():AGC Table Fail\n")); */
		goto phy_BB8190_Config_ParaFile_Fail;
	}


phy_BB8190_Config_ParaFile_Fail:

	return rtStatus;
}


int
PHY_BBConfig8188E(
	struct adapter *Adapter
	)
{
	int	rtStatus = _SUCCESS;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u32	RegVal;
	u8	TmpU1B=0;
	u8	value8,CrystalCap;

	phy_InitBBRFRegisterDefinition(Adapter);


	/*  Enable BB and RF */
	RegVal = rtw_read16(Adapter, REG_SYS_FUNC_EN);
	rtw_write16(Adapter, REG_SYS_FUNC_EN, (u16)(RegVal|BIT13|BIT0|BIT1));

	/*  20090923 Joseph: Advised by Steven and Jenyu. Power sequence before init RF. */
	/* rtw_write8(Adapter, REG_AFE_PLL_CTRL, 0x83); */
	/* rtw_write8(Adapter, REG_AFE_PLL_CTRL+1, 0xdb); */

	rtw_write8(Adapter, REG_RF_CTRL, RF_EN|RF_RSTB|RF_SDMRSTB);

	rtw_write8(Adapter, REG_SYS_FUNC_EN, FEN_USBA | FEN_USBD | FEN_BB_GLB_RSTn | FEN_BBRSTB);

	/*  */
	/*  Config BB and AGC */
	/*  */
	rtStatus = phy_BB8188E_Config_ParaFile(Adapter);

	/*  write 0x24[16:11] = 0x24[22:17] = CrystalCap */
	CrystalCap = pHalData->CrystalCap & 0x3F;
	PHY_SetBBReg(Adapter, REG_AFE_XTAL_CTRL, 0x7ff800, (CrystalCap | (CrystalCap << 6)));

	return rtStatus;

}


int
PHY_RFConfig8188E(
	struct adapter *Adapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	int		rtStatus = _SUCCESS;

	/*  */
	/*  RF config */
	/*  */
	rtStatus = PHY_RF6052_Config8188E(Adapter);
	return rtStatus;
}


/*-----------------------------------------------------------------------------
 * Function:    PHY_ConfigRFWithParaFile()
 *
 * Overview:    This function read RF parameters from general file format, and do RF 3-wire
 *
 * Input:	struct adapter *		Adapter
 *			s8 *					pFileName
 *			enum rf_radio_path	eRFPath
 *
 * Output:      NONE
 *
 * Return:      RT_STATUS_SUCCESS: configuration file exist
 *
 * Note:		Delay may be required for RF configuration
 *---------------------------------------------------------------------------*/
int
rtl8188e_PHY_ConfigRFWithParaFile(
	struct adapter *		Adapter,
	u8*				pFileName,
	enum rf_radio_path		eRFPath
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	int	rtStatus = _SUCCESS;


	return rtStatus;

}

/*  */
/*  The following is for High Power PA */
/*  */
#define HighPowerRadioAArrayLen 22
/* This is for High power PA */
static u32 Rtl8192S_HighPower_RadioA_Array[HighPowerRadioAArrayLen] = {
0x013,0x00029ea4,
0x013,0x00025e74,
0x013,0x00020ea4,
0x013,0x0001ced0,
0x013,0x00019f40,
0x013,0x00014e70,
0x013,0x000106a0,
0x013,0x0000c670,
0x013,0x000082a0,
0x013,0x00004270,
0x013,0x00000240,
};

static int
PHY_ConfigRFExternalPA(
	struct adapter *		Adapter,
	enum rf_radio_path		eRFPath
)
{
	int	rtStatus = _SUCCESS;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u16 i=0;

	if (!pHalData->ExternalPA)
		return rtStatus;
	return rtStatus;
}

/*-----------------------------------------------------------------------------
 * Function:    PHY_CheckBBAndRFOK()
 *
 * Overview:    This function is write register and then readback to make sure whether
 *			  BB[PHY0, PHY1], RF[Patha, path b, path c, path d] is Ok
 *
 * Input:	struct adapter *		Adapter
 *			HW90_BLOCK_E		CheckBlock
 *			enum rf_radio_path	eRFPath		it is used only when CheckBlock is HW90_BLOCK_RF
 *
 * Output:      NONE
 *
 * Return:      RT_STATUS_SUCCESS: PHY is OK
 *
 * Note:		This function may be removed in the ASIC
 *---------------------------------------------------------------------------*/
static int
PHY_CheckBBAndRFOK(
	struct adapter *		Adapter,
	HW90_BLOCK_E		CheckBlock,
	enum rf_radio_path	eRFPath
	)
{
	int			rtStatus = _SUCCESS;

	u32				i, CheckTimes = 4,ulRegRead = 0;

	u32				WriteAddr[4];
	u32				WriteData[] = {0xfffff027, 0xaa55a02f, 0x00000027, 0x55aa502f};

	/*  Initialize register address offset to be checked */
	WriteAddr[HW90_BLOCK_MAC] = 0x100;
	WriteAddr[HW90_BLOCK_PHY0] = 0x900;
	WriteAddr[HW90_BLOCK_PHY1] = 0x800;
	WriteAddr[HW90_BLOCK_RF] = 0x3;

	for (i=0 ; i < CheckTimes ; i++)
	{

		/*  */
		/*  Write Data to register and readback */
		/*  */
		switch (CheckBlock)
		{
		case HW90_BLOCK_MAC:
			/* RT_ASSERT(false, ("PHY_CheckBBRFOK(): Never Write 0x100 here!")); */
			/* RT_TRACE(COMP_INIT, DBG_LOUD, ("PHY_CheckBBRFOK(): Never Write 0x100 here!\n")); */
			break;

		case HW90_BLOCK_PHY0:
		case HW90_BLOCK_PHY1:
			rtw_write32(Adapter, WriteAddr[CheckBlock], WriteData[i]);
			ulRegRead = rtw_read32(Adapter, WriteAddr[CheckBlock]);
			break;

		case HW90_BLOCK_RF:
			/*  When initialization, we want the delay function(delay_ms(), delay_us() */
			/*  ==> actually we call PlatformStallExecution()) to do NdisStallExecution() */
			/*  [busy wait] instead of NdisMSleep(). So we acquire RT_INITIAL_SPINLOCK */
			/*  to run at Dispatch level to achive it. */
			/* cosa PlatformAcquireSpinLock(Adapter, RT_INITIAL_SPINLOCK); */
			WriteData[i] &= 0xfff;
			PHY_SetRFReg(Adapter, eRFPath, WriteAddr[HW90_BLOCK_RF], bRFRegOffsetMask, WriteData[i]);
			/*  TODO: we should not delay for such a long time. Ask SD3 */
			rtw_mdelay_os(10);
			ulRegRead = PHY_QueryRFReg(Adapter, eRFPath, WriteAddr[HW90_BLOCK_RF], bMaskDWord);
			rtw_mdelay_os(10);
			/* cosa PlatformReleaseSpinLock(Adapter, RT_INITIAL_SPINLOCK); */
			break;

		default:
			rtStatus = _FAIL;
			break;
		}


		/*  */
		/*  Check whether readback data is correct */
		/*  */
		if (ulRegRead != WriteData[i])
		{
			/* RT_TRACE(COMP_FPGA, DBG_LOUD, ("ulRegRead: %lx, WriteData: %lx\n", ulRegRead, WriteData[i])); */
			rtStatus = _FAIL;
			break;
		}
	}

	return rtStatus;
}


void
rtl8192c_PHY_GetHWRegOriginalValue(
	struct adapter *	Adapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	/*  read rx initial gain */
	pHalData->DefaultInitialGain[0] = (u8)PHY_QueryBBReg(Adapter, rOFDM0_XAAGCCore1, bMaskByte0);
	pHalData->DefaultInitialGain[1] = (u8)PHY_QueryBBReg(Adapter, rOFDM0_XBAGCCore1, bMaskByte0);
	pHalData->DefaultInitialGain[2] = (u8)PHY_QueryBBReg(Adapter, rOFDM0_XCAGCCore1, bMaskByte0);
	pHalData->DefaultInitialGain[3] = (u8)PHY_QueryBBReg(Adapter, rOFDM0_XDAGCCore1, bMaskByte0);
	/* RT_TRACE(COMP_INIT, DBG_LOUD, */
	/* Default initial gain (c50=0x%x, c58=0x%x, c60=0x%x, c68=0x%x)\n", */
	/* pHalData->DefaultInitialGain[0], pHalData->DefaultInitialGain[1], */
	/* pHalData->DefaultInitialGain[2], pHalData->DefaultInitialGain[3])); */

	/*  read framesync */
	pHalData->framesync = (u8)PHY_QueryBBReg(Adapter, rOFDM0_RxDetector3, bMaskByte0);
	pHalData->framesyncC34 = PHY_QueryBBReg(Adapter, rOFDM0_RxDetector2, bMaskDWord);
	/* RT_TRACE(COMP_INIT, DBG_LOUD, ("Default framesync (0x%x) = 0x%x\n", */
	/* 	rOFDM0_RxDetector3, pHalData->framesync)); */
}


/*  */
/* 	Description: */
/* 		Map dBm into Tx power index according to */
/* 		current HW model, for example, RF and PA, and */
/* 		current wireless mode. */
/* 	By Bruce, 2008-01-29. */
/*  */
static	u8
phy_DbmToTxPwrIdx(
	struct adapter *	Adapter,
	WIRELESS_MODE	WirelessMode,
	int			PowerInDbm
	)
{
	u8				TxPwrIdx = 0;
	int				Offset = 0;


	/*  */
	/*  Tested by MP, we found that CCK Index 0 equals to 8dbm, OFDM legacy equals to */
	/*  3dbm, and OFDM HT equals to 0dbm repectively. */
	/*  Note: */
	/* 	The mapping may be different by different NICs. Do not use this formula for what needs accurate result. */
	/*  By Bruce, 2008-01-29. */
	/*  */
	switch (WirelessMode)
	{
	case WIRELESS_MODE_B:
		Offset = -7;
		break;

	case WIRELESS_MODE_G:
	case WIRELESS_MODE_N_24G:
		Offset = -8;
		break;
	default:
		Offset = -8;
		break;
	}

	if ((PowerInDbm - Offset) > 0)
	{
		TxPwrIdx = (u8)((PowerInDbm - Offset) * 2);
	}
	else
	{
		TxPwrIdx = 0;
	}

	/*  Tx Power Index is too large. */
	if (TxPwrIdx > MAX_TXPWR_IDX_NMODE_92S)
		TxPwrIdx = MAX_TXPWR_IDX_NMODE_92S;

	return TxPwrIdx;
}

/*  */
/* 	Description: */
/* 		Map Tx power index into dBm according to */
/* 		current HW model, for example, RF and PA, and */
/* 		current wireless mode. */
/* 	By Bruce, 2008-01-29. */
/*  */
static int
phy_TxPwrIdxToDbm(
	struct adapter *	Adapter,
	WIRELESS_MODE	WirelessMode,
	u8			TxPwrIdx
	)
{
	int				Offset = 0;
	int				PwrOutDbm = 0;

	/*  */
	/*  Tested by MP, we found that CCK Index 0 equals to -7dbm, OFDM legacy equals to -8dbm. */
	/*  Note: */
	/* 	The mapping may be different by different NICs. Do not use this formula for what needs accurate result. */
	/*  By Bruce, 2008-01-29. */
	/*  */
	switch (WirelessMode) {
	case WIRELESS_MODE_B:
		Offset = -7;
		break;
	case WIRELESS_MODE_G:
	case WIRELESS_MODE_N_24G:
	default:
		Offset = -8;
		break;
	}

	PwrOutDbm = TxPwrIdx / 2 + Offset; /*  Discard the decimal part. */

	return PwrOutDbm;
}


/*-----------------------------------------------------------------------------
 * Function:    GetTxPowerLevel8190()
 *
 * Overview:    This function is export to "common" moudule
 *
 * Input:       struct adapter *	Adapter
 *			psByte			Power Level
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 *---------------------------------------------------------------------------*/
void
PHY_GetTxPowerLevel8188E(
	struct adapter *	Adapter,
	u32*		powerlevel
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u8			TxPwrLevel = 0;
	int			TxPwrDbm;

	/*  */
	/*  Because the Tx power indexes are different, we report the maximum of them to */
	/*  meet the CCX TPC request. By Bruce, 2008-01-31. */
	/*  */

	/*  CCK */
	TxPwrLevel = pHalData->CurrentCckTxPwrIdx;
	TxPwrDbm = phy_TxPwrIdxToDbm(Adapter, WIRELESS_MODE_B, TxPwrLevel);

	/*  Legacy OFDM */
	TxPwrLevel = pHalData->CurrentOfdm24GTxPwrIdx + pHalData->LegacyHTTxPowerDiff;

	/*  Compare with Legacy OFDM Tx power. */
	if (phy_TxPwrIdxToDbm(Adapter, WIRELESS_MODE_G, TxPwrLevel) > TxPwrDbm)
		TxPwrDbm = phy_TxPwrIdxToDbm(Adapter, WIRELESS_MODE_G, TxPwrLevel);

	/*  HT OFDM */
	TxPwrLevel = pHalData->CurrentOfdm24GTxPwrIdx;

	/*  Compare with HT OFDM Tx power. */
	if (phy_TxPwrIdxToDbm(Adapter, WIRELESS_MODE_N_24G, TxPwrLevel) > TxPwrDbm)
		TxPwrDbm = phy_TxPwrIdxToDbm(Adapter, WIRELESS_MODE_N_24G, TxPwrLevel);

	*powerlevel = TxPwrDbm;
}

static void getTxPowerIndex88E(
	struct adapter *	Adapter,
	u8				channel,
	u8*			cckPowerLevel,
	u8*			ofdmPowerLevel,
	u8*			BW20PowerLevel,
	u8*			BW40PowerLevel
	)
{

	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	u8				index = (channel -1);
	u8				TxCount=0,path_nums;


	if ((RF_1T2R == pHalData->rf_type) ||(RF_1T1R ==pHalData->rf_type ))
		path_nums = 1;
	else
		path_nums = 2;

	for (TxCount=0;TxCount< path_nums ;TxCount++)
	{
		if (TxCount==RF_PATH_A)
		{
			/*  1. CCK */
			cckPowerLevel[TxCount]		= pHalData->Index24G_CCK_Base[TxCount][index];
			/* 2. OFDM */
			ofdmPowerLevel[TxCount]		= pHalData->Index24G_BW40_Base[RF_PATH_A][index]+
				pHalData->OFDM_24G_Diff[TxCount][RF_PATH_A];
			/*  1. BW20 */
			BW20PowerLevel[TxCount]	= pHalData->Index24G_BW40_Base[RF_PATH_A][index]+
				pHalData->BW20_24G_Diff[TxCount][RF_PATH_A];
			/* 2. BW40 */
			BW40PowerLevel[TxCount]	= pHalData->Index24G_BW40_Base[TxCount][index];
			/* RTPRINT(FPHY, PHY_TXPWR, ("getTxPowerIndex88E(): 40MBase=0x%x  20Mdiff=%d  20MBase=0x%x!!\n", */
			/* 	pHalData->Index24G_BW40_Base[RF_PATH_A][index], */
			/* 	pHalData->BW20_24G_Diff[TxCount][RF_PATH_A], */
			/* 	BW20PowerLevel[TxCount])); */
		}
		else if (TxCount==RF_PATH_B)
		{
			/*  1. CCK */
			cckPowerLevel[TxCount]		= pHalData->Index24G_CCK_Base[TxCount][index];
			/* 2. OFDM */
			ofdmPowerLevel[TxCount]		= pHalData->Index24G_BW40_Base[RF_PATH_A][index]+
			pHalData->BW20_24G_Diff[RF_PATH_A][index]+
			pHalData->BW20_24G_Diff[TxCount][index];
			/*  1. BW20 */
			BW20PowerLevel[TxCount]	= pHalData->Index24G_BW40_Base[RF_PATH_A][index]+
			pHalData->BW20_24G_Diff[TxCount][RF_PATH_A]+
			pHalData->BW20_24G_Diff[TxCount][index];
			/* 2. BW40 */
			BW40PowerLevel[TxCount]	= pHalData->Index24G_BW40_Base[TxCount][index];
		}
	}
}

static void phy_PowerIndexCheck88E(
	struct adapter *Adapter,
	u8			channel,
	u8 *		cckPowerLevel,
	u8 *		ofdmPowerLevel,
	u8 *		BW20PowerLevel,
	u8 *		BW40PowerLevel
	)
{

	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);

	pHalData->CurrentCckTxPwrIdx = cckPowerLevel[0];
	pHalData->CurrentOfdm24GTxPwrIdx = ofdmPowerLevel[0];
	pHalData->CurrentBW2024GTxPwrIdx = BW20PowerLevel[0];
	pHalData->CurrentBW4024GTxPwrIdx = BW40PowerLevel[0];
}

/*-----------------------------------------------------------------------------
 * Function:    SetTxPowerLevel8190()
 *
 * Overview:    This function is export to "HalCommon" moudule
 *			We must consider RF path later!!!!!!!
 *
 * Input:       struct adapter *	Adapter
 *			u8		channel
 *
 * Output:      NONE
 *
 * Return:      NONE
 *	2008/11/04	MHC		We remove EEPROM_93C56.
 *						We need to move CCX relative code to independet file.
 *	2009/01/21	MHC		Support new EEPROM format from SD3 requirement.
 *
 *---------------------------------------------------------------------------*/
void
PHY_SetTxPowerLevel8188E(
	struct adapter *	Adapter,
	u8				channel
	)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);

	u8	cckPowerLevel[MAX_TX_COUNT], ofdmPowerLevel[MAX_TX_COUNT];/*  [0]:RF-A, [1]:RF-B */
	u8	BW20PowerLevel[MAX_TX_COUNT], BW40PowerLevel[MAX_TX_COUNT];
	u8	i=0;
/*
#if (MP_DRIVER == 1)
	if (Adapter->registrypriv.mp_mode == 1)
	return;
#endif
*/
	/* getTxPowerIndex(Adapter, channel, &cckPowerLevel[0], &ofdmPowerLevel[0]); */
	getTxPowerIndex88E(Adapter, channel, &cckPowerLevel[0], &ofdmPowerLevel[0],&BW20PowerLevel[0],&BW40PowerLevel[0]);

	/* printk("Channel-%d, cckPowerLevel =  0x%x,   ofdmPowerLeve = 0x%x, BW20PowerLevel  =  0x%x,   BW40PowerLevel = 0x%x,\n", */
	/* 		channel, cckPowerLevel[0],  ofdmPowerLevel[0], BW20PowerLevel[0] ,BW40PowerLevel[0]); */

	/* RTPRINT(FPHY, PHY_TXPWR, ("Channel-%d, cckPowerLevel (A / B) = 0x%x / 0x%x,   ofdmPowerLevel (A / B) = 0x%x / 0x%x\n", */
	/* 	channel, cckPowerLevel[0], cckPowerLevel[1], ofdmPowerLevel[0], ofdmPowerLevel[1])); */

	/* ccxPowerIndexCheck(Adapter, channel, &cckPowerLevel[0], &ofdmPowerLevel[0]); */
	phy_PowerIndexCheck88E(Adapter, channel, &cckPowerLevel[0], &ofdmPowerLevel[0],&BW20PowerLevel[0],&BW40PowerLevel[0]);

	rtl8188e_PHY_RF6052SetCckTxPower(Adapter, &cckPowerLevel[0]);
	rtl8188e_PHY_RF6052SetOFDMTxPower(Adapter, &ofdmPowerLevel[0],&BW20PowerLevel[0],&BW40PowerLevel[0], channel);
}


/*  */
/* 	Description: */
/* 		Update transmit power level of all channel supported. */
/*  */
/* 	TODO: */
/* 		A mode. */
/* 	By Bruce, 2008-02-04. */
/*  */
bool
PHY_UpdateTxPowerDbm8188E(
	struct adapter *Adapter,
	int		powerInDbm
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u8				idx;
	u8			rf_path;

	/*  TODO: A mode Tx power. */
	u8	CckTxPwrIdx = phy_DbmToTxPwrIdx(Adapter, WIRELESS_MODE_B, powerInDbm);
	u8	OfdmTxPwrIdx = phy_DbmToTxPwrIdx(Adapter, WIRELESS_MODE_N_24G, powerInDbm);

	if (OfdmTxPwrIdx - pHalData->LegacyHTTxPowerDiff > 0)
		OfdmTxPwrIdx -= pHalData->LegacyHTTxPowerDiff;
	else
		OfdmTxPwrIdx = 0;

	/* RT_TRACE(COMP_TXAGC, DBG_LOUD, ("PHY_UpdateTxPowerDbm8192S(): %ld dBm , CckTxPwrIdx = %d, OfdmTxPwrIdx = %d\n", powerInDbm, CckTxPwrIdx, OfdmTxPwrIdx)); */

	for (idx = 0; idx < 14; idx++)
	{
		for (rf_path = 0; rf_path < 2; rf_path++)
		{
			pHalData->TxPwrLevelCck[rf_path][idx] = CckTxPwrIdx;
			pHalData->TxPwrLevelHT40_1S[rf_path][idx] =
			pHalData->TxPwrLevelHT40_2S[rf_path][idx] = OfdmTxPwrIdx;
		}
	}
	return true;
}


/*
	Description:
		When beacon interval is changed, the values of the
		hw registers should be modified.
	By tynli, 2008.10.24.

*/


static void
rtl8192c_PHY_SetBeaconHwReg(
	struct adapter *	Adapter,
	u16			BeaconInterval
	)
{

}


void
PHY_ScanOperationBackup8188E(
	struct adapter *Adapter,
	u8		Operation
	)
{
}

/*-----------------------------------------------------------------------------
 * Function:    PHY_SetBWModeCallback8192C()
 *
 * Overview:    Timer callback function for SetSetBWMode
 *
 * Input:		PRT_TIMER		pTimer
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Note:		(1) We do not take j mode into consideration now
 *			(2) Will two workitem of "switch channel" and "switch channel bandwidth" run
 *			     concurrently?
 *---------------------------------------------------------------------------*/
static void
_PHY_SetBWMode92C(
	struct adapter *Adapter
)
{
	HAL_DATA_TYPE		*pHalData = GET_HAL_DATA(Adapter);
	u8				regBwOpMode;
	u8				regRRSR_RSC;

	if (pHalData->rf_chip == RF_PSEUDO_11N)
	{
		return;
	}

	/*  There is no 40MHz mode in RF_8225. */
	if (pHalData->rf_chip==RF_8225)
		return;

	if (Adapter->bDriverStopped)
		return;

	regBwOpMode = rtw_read8(Adapter, REG_BWOPMODE);
	regRRSR_RSC = rtw_read8(Adapter, REG_RRSR+2);

	switch (pHalData->CurrentChannelBW) {
		case HT_CHANNEL_WIDTH_20:
			regBwOpMode |= BW_OPMODE_20MHZ;
			   /*  2007/02/07 Mark by Emily becasue we have not verify whether this register works */
			rtw_write8(Adapter, REG_BWOPMODE, regBwOpMode);
			break;

		case HT_CHANNEL_WIDTH_40:
			regBwOpMode &= ~BW_OPMODE_20MHZ;
				/*  2007/02/07 Mark by Emily becasue we have not verify whether this register works */
			rtw_write8(Adapter, REG_BWOPMODE, regBwOpMode);

			regRRSR_RSC = (regRRSR_RSC&0x90) |(pHalData->nCur40MhzPrimeSC<<5);
			rtw_write8(Adapter, REG_RRSR+2, regRRSR_RSC);
			break;

		default:
			/*RT_TRACE(COMP_DBG, DBG_LOUD, ("PHY_SetBWModeCallback8192C():
						unknown Bandwidth: %#X\n",pHalData->CurrentChannelBW));*/
			break;
	}

	/* 3 */
	/* 3<2>Set PHY related register */
	/* 3 */
	switch (pHalData->CurrentChannelBW)
	{
		/* 20 MHz channel*/
		case HT_CHANNEL_WIDTH_20:
			PHY_SetBBReg(Adapter, rFPGA0_RFMOD, bRFMOD, 0x0);
			PHY_SetBBReg(Adapter, rFPGA1_RFMOD, bRFMOD, 0x0);
			/* PHY_SetBBReg(Adapter, rFPGA0_AnalogParameter2, BIT10, 1); */

			break;


		/* 40 MHz channel*/
		case HT_CHANNEL_WIDTH_40:
			PHY_SetBBReg(Adapter, rFPGA0_RFMOD, bRFMOD, 0x1);
			PHY_SetBBReg(Adapter, rFPGA1_RFMOD, bRFMOD, 0x1);

			/*  Set Control channel to upper or lower. These settings are required only for 40MHz */
			PHY_SetBBReg(Adapter, rCCK0_System, bCCKSideBand, (pHalData->nCur40MhzPrimeSC>>1));
			PHY_SetBBReg(Adapter, rOFDM1_LSTF, 0xC00, pHalData->nCur40MhzPrimeSC);
			/* PHY_SetBBReg(Adapter, rFPGA0_AnalogParameter2, BIT10, 0); */

			PHY_SetBBReg(Adapter, 0x818, (BIT26|BIT27), (pHalData->nCur40MhzPrimeSC==HAL_PRIME_CHNL_OFFSET_LOWER)?2:1);

			break;



		default:
			/*RT_TRACE(COMP_DBG, DBG_LOUD, ("PHY_SetBWModeCallback8192C(): unknown Bandwidth: %#X\n"\
						,pHalData->CurrentChannelBW));*/
			break;

	}
	/* Skip over setting of J-mode in BB register here. Default value is "None J mode". Emily 20070315 */

	/*  Added it for 20/40 mhz switch time evaluation by guangan 070531 */
	/* NowL = PlatformEFIORead4Byte(Adapter, TSFR); */
	/* NowH = PlatformEFIORead4Byte(Adapter, TSFR+4); */
	/* EndTime = ((u64)NowH << 32) + NowL; */
	/* RT_TRACE(COMP_SCAN, DBG_LOUD, ("SetBWModeCallback8190Pci: time of SetBWMode = %I64d us!\n", (EndTime - BeginTime))); */

	/* 3<3>Set RF related register */
	switch (pHalData->rf_chip)
	{
		case RF_8225:
			/* PHY_SetRF8225Bandwidth(Adapter, pHalData->CurrentChannelBW); */
			break;

		case RF_8256:
			/*  Please implement this function in Hal8190PciPhy8256.c */
			/* PHY_SetRF8256Bandwidth(Adapter, pHalData->CurrentChannelBW); */
			break;

		case RF_8258:
			/*  Please implement this function in Hal8190PciPhy8258.c */
			/*  PHY_SetRF8258Bandwidth(); */
			break;

		case RF_PSEUDO_11N:
			/*  Do Nothing */
			break;

		case RF_6052:
			rtl8188e_PHY_RF6052SetBandwidth(Adapter, pHalData->CurrentChannelBW);
			break;

		default:
			/* RT_ASSERT(false, ("Unknown RFChipID: %d\n", pHalData->RFChipID)); */
			break;
	}

	/* pHalData->SetBWModeInProgress= false; */

	/* RT_TRACE(COMP_SCAN, DBG_LOUD, ("<==PHY_SetBWModeCallback8192C()\n" )); */
}


 /*-----------------------------------------------------------------------------
 * Function:   SetBWMode8190Pci()
 *
 * Overview:  This function is export to "HalCommon" moudule
 *
 * Input:		struct adapter *		Adapter
 *			enum HT_CHANNEL_WIDTH	Bandwidth	20M or 40M
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Note:		We do not take j mode into consideration now
 *---------------------------------------------------------------------------*/
void
PHY_SetBWMode8188E(
	struct adapter *				Adapter,
	enum HT_CHANNEL_WIDTH	Bandwidth,	/*  20M or 40M */
	unsigned char	Offset		/*  Upper, Lower, or Don't care */
)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	enum HT_CHANNEL_WIDTH	tmpBW= pHalData->CurrentChannelBW;

	pHalData->CurrentChannelBW = Bandwidth;

	pHalData->nCur40MhzPrimeSC = Offset;

	if ((!Adapter->bDriverStopped) && (!Adapter->bSurpriseRemoved))
		_PHY_SetBWMode92C(Adapter);
	else
		pHalData->CurrentChannelBW = tmpBW;

}


static void _PHY_SwChnl8192C(struct adapter *Adapter, u8 channel)
{
	u8 eRFPath;
	u32 param1, param2;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	if ( Adapter->bNotifyChannelChange )
	{
		DBG_88E( "[%s] ch = %d\n", __FUNCTION__, channel );
	}

	/* s1. pre common command - CmdID_SetTxPowerLevel */
	PHY_SetTxPowerLevel8188E(Adapter, channel);

	/* s2. RF dependent command - CmdID_RF_WriteReg, param1=RF_CHNLBW, param2=channel */
	param1 = RF_CHNLBW;
	param2 = channel;
	for (eRFPath = 0; eRFPath <pHalData->NumTotalRFPath; eRFPath++)
	{
		pHalData->RfRegChnlVal[eRFPath] = ((pHalData->RfRegChnlVal[eRFPath] & 0xfffffc00) | param2);
		PHY_SetRFReg(Adapter, (enum rf_radio_path)eRFPath, param1, bRFRegOffsetMask, pHalData->RfRegChnlVal[eRFPath]);
	}


	/* s3. post common command - CmdID_End, None */

}
/*  <20130708, James> A workaround to eliminate the 2480MHz spur for 8188E I-Cut */
static void phy_SpurCalibration_8188E(
	struct adapter *		Adapter
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	/* DbgPrint("===> phy_SpurCalibration_8188E  CurrentChannelBW = %d, CurrentChannel = %d\n", pHalData->CurrentChannelBW, pHalData->CurrentChannel); */
	if (pHalData->CurrentChannelBW == 0 && pHalData->CurrentChannel == 13) {
		PHY_SetBBReg(Adapter, rOFDM1_CFOTracking, BIT(28), 0x1); /* enable CSI Mask */
		PHY_SetBBReg(Adapter, rOFDM1_csi_fix_mask, BIT(26)|BIT(25), 0x3); /* Fix CSI Mask Tone */
	}
	else {
		PHY_SetBBReg(Adapter, rOFDM1_CFOTracking, BIT(28), 0x0); /* disable CSI Mask */
		PHY_SetBBReg(Adapter, rOFDM1_csi_fix_mask, BIT(26)|BIT(25), 0x0);
	}

}
void
PHY_SwChnl8188E(	/*  Call after initialization */
	struct adapter *Adapter,
	u8		channel
	)
{
	/* struct adapter *Adapter =  ADJUST_TO_ADAPTIVE_ADAPTER(pAdapter, true); */
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);
	u8	tmpchannel = pHalData->CurrentChannel;
	bool  bResult = true;

	if (pHalData->rf_chip == RF_PSEUDO_11N)
	{
		/* pHalData->SwChnlInProgress=false; */
		return;									/* return immediately if it is peudo-phy */
	}

	/* if (pHalData->SwChnlInProgress) */
	/* 	return; */

	/* if (pHalData->SetBWModeInProgress) */
	/* 	return; */

	/*  */
	switch (pHalData->CurrentWirelessMode)
	{
		case WIRELESS_MODE_A:
		case WIRELESS_MODE_N_5G:
			/* RT_ASSERT((channel>14), ("WIRELESS_MODE_A but channel<=14")); */
			break;

		case WIRELESS_MODE_B:
			/* RT_ASSERT((channel<=14), ("WIRELESS_MODE_B but channel>14")); */
			break;

		case WIRELESS_MODE_G:
		case WIRELESS_MODE_N_24G:
			/* RT_ASSERT((channel<=14), ("WIRELESS_MODE_G but channel>14")); */
			break;

		default:
			/* RT_ASSERT(false, ("Invalid WirelessMode(%#x)!!\n", pHalData->CurrentWirelessMode)); */
			break;
	}
	/*  */

	/* pHalData->SwChnlInProgress = true; */
	if (channel == 0)
		channel = 1;

	pHalData->CurrentChannel=channel;

	/* pHalData->SwChnlStage=0; */
	/* pHalData->SwChnlStep=0; */

	if ((!Adapter->bDriverStopped) && (!Adapter->bSurpriseRemoved))
	{
		_PHY_SwChnl8192C(Adapter, channel);
		if (IS_VENDOR_8188E_I_CUT_SERIES(Adapter))
			phy_SpurCalibration_8188E( Adapter);
		if (bResult)
		{
			/* RT_TRACE(COMP_SCAN, DBG_LOUD, ("PHY_SwChnl8192C SwChnlInProgress true schdule workitem done\n")); */
		}
		else
		{
			/* RT_TRACE(COMP_SCAN, DBG_LOUD, ("PHY_SwChnl8192C SwChnlInProgress false schdule workitem error\n")); */
			/* if (IS_HARDWARE_TYPE_8192SU(Adapter)) */
			/*  */
			/* 	pHalData->SwChnlInProgress = false; */
				pHalData->CurrentChannel = tmpchannel;
			/*  */
		}

	}
	else
	{
		/* RT_TRACE(COMP_SCAN, DBG_LOUD, ("PHY_SwChnl8192C SwChnlInProgress false driver sleep or unload\n")); */
		/* if (IS_HARDWARE_TYPE_8192SU(Adapter)) */
		/*  */
		/* 	pHalData->SwChnlInProgress = false; */
			pHalData->CurrentChannel = tmpchannel;
		/*  */
	}
}


static	bool
phy_SwChnlStepByStep(
	struct adapter *Adapter,
	u8		channel,
	u8		*stage,
	u8		*step,
	u32		*delay
	)
{
	return true;
}


static	bool
phy_SetSwChnlCmdArray(
	SwChnlCmd*		CmdTable,
	u32			CmdTableIdx,
	u32			CmdTableSz,
	SwChnlCmdID		CmdID,
	u32			Para1,
	u32			Para2,
	u32			msDelay
	)
{
	SwChnlCmd* pCmd;

	if (CmdTable == NULL)
	{
		/* RT_ASSERT(false, ("phy_SetSwChnlCmdArray(): CmdTable cannot be NULL.\n")); */
		return false;
	}
	if (CmdTableIdx >= CmdTableSz)
	{
		/* RT_ASSERT(false, */
		/* 		("phy_SetSwChnlCmdArray(): Access invalid index, please check size of the table, CmdTableIdx:%ld, CmdTableSz:%ld\n", */
		/* 		CmdTableIdx, CmdTableSz)); */
		return false;
	}

	pCmd = CmdTable + CmdTableIdx;
	pCmd->CmdID = CmdID;
	pCmd->Para1 = Para1;
	pCmd->Para2 = Para2;
	pCmd->msDelay = msDelay;

	return true;
}


static	void
phy_FinishSwChnlNow(	/*  We should not call this function directly */
		struct adapter *Adapter,
		u8		channel
		)
{
}

/*  */
/*  Description: */
/* 	Switch channel synchronously. Called by SwChnlByDelayHandler. */
/*  */
/*  Implemented by Bruce, 2008-02-14. */
/*  The following procedure is operted according to SwChanlCallback8190Pci(). */
/*  However, this procedure is performed synchronously  which should be running under */
/*  passive level. */
/*  */
void
PHY_SwChnlPhy8192C(	/*  Only called during initialize */
	struct adapter *Adapter,
	u8		channel
	)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(Adapter);

	/* RT_TRACE(COMP_SCAN | COMP_RM, DBG_LOUD, ("==>PHY_SwChnlPhy8192S(), switch from channel %d to channel %d.\n", pHalData->CurrentChannel, channel)); */

	/*  Cannot IO. */
	/* if (RT_CANNOT_IO(Adapter)) */
	/* 	return; */

	/*  Channel Switching is in progress. */
	/* if (pHalData->SwChnlInProgress) */
	/* 	return; */

	/* return immediately if it is peudo-phy */
	if (pHalData->rf_chip == RF_PSEUDO_11N)
	{
		/* pHalData->SwChnlInProgress=false; */
		return;
	}

	/* pHalData->SwChnlInProgress = true; */
	if ( channel == 0)
		channel = 1;

	pHalData->CurrentChannel=channel;

	/* pHalData->SwChnlStage = 0; */
	/* pHalData->SwChnlStep = 0; */

	phy_FinishSwChnlNow(Adapter,channel);

	/* pHalData->SwChnlInProgress = false; */
}


/*  */
/* 	Description: */
/* 		Configure H/W functionality to enable/disable Monitor mode. */
/* 		Note, because we possibly need to configure BB and RF in this function, */
/* 		so caller should in PASSIVE_LEVEL. 080118, by rcnjko. */
/*  */
void
PHY_SetMonitorMode8192C(
	struct adapter *		pAdapter,
	bool				bEnableMonitorMode
	)
{
}


/*-----------------------------------------------------------------------------
 * Function:	PHYCheckIsLegalRfPath8190Pci()
 *
 * Overview:	Check different RF type to execute legal judgement. If RF Path is illegal
 *			We will return false.
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	11/15/2007	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
bool
PHY_CheckIsLegalRfPath8192C(
	struct adapter *pAdapter,
	u32	eRFPath)
{
	bool				rtValue = true;

	/*  NOt check RF Path now.! */
	return	rtValue;

}	/* PHY_CheckIsLegalRfPath8192C */

static void _PHY_SetRFPathSwitch(
	struct adapter *pAdapter,
	bool		bMain,
	bool		is2T
	)
{
	u8	u1bTmp;

	if (!pAdapter->hw_init_completed)
	{
		u1bTmp = rtw_read8(pAdapter, REG_LEDCFG2) | BIT7;
		rtw_write8(pAdapter, REG_LEDCFG2, u1bTmp);
		/* PHY_SetBBReg(pAdapter, REG_LEDCFG0, BIT23, 0x01); */
		PHY_SetBBReg(pAdapter, rFPGA0_XAB_RFParameter, BIT13, 0x01);
	}

	if (is2T)
	{
		if (bMain)
			PHY_SetBBReg(pAdapter, rFPGA0_XB_RFInterfaceOE, BIT5|BIT6, 0x1);	/* 92C_Path_A */
		else
			PHY_SetBBReg(pAdapter, rFPGA0_XB_RFInterfaceOE, BIT5|BIT6, 0x2);	/* BT */
	}
	else
	{

		if (bMain)
			PHY_SetBBReg(pAdapter, rFPGA0_XA_RFInterfaceOE, 0x300, 0x2);	/* Main */
		else
			PHY_SetBBReg(pAdapter, rFPGA0_XA_RFInterfaceOE, 0x300, 0x1);	/* Aux */
	}

}

/* return value true => Main; false => Aux */

static bool _PHY_QueryRFPathSwitch(
	struct adapter *pAdapter,
	bool		is2T
	)
{
/* 	if (is2T) */
/* 		return true; */

	if (!pAdapter->hw_init_completed)
	{
		PHY_SetBBReg(pAdapter, REG_LEDCFG0, BIT23, 0x01);
		PHY_SetBBReg(pAdapter, rFPGA0_XAB_RFParameter, BIT13, 0x01);
	}

	if (is2T)
	{
		if (PHY_QueryBBReg(pAdapter, rFPGA0_XB_RFInterfaceOE, BIT5|BIT6) == 0x01)
			return true;
		else
			return false;
	}
	else
	{
		if (PHY_QueryBBReg(pAdapter, rFPGA0_XA_RFInterfaceOE, 0x300) == 0x02)
			return true;
		else
			return false;
	}
}


static void
_PHY_DumpRFReg(struct adapter *pAdapter)
{
	u32 rfRegValue,rfRegOffset;

	/* RTPRINT(FINIT, INIT_RF, ("PHY_DumpRFReg()====>\n")); */

	for (rfRegOffset = 0x00;rfRegOffset<=0x30;rfRegOffset++) {
		rfRegValue = PHY_QueryRFReg(pAdapter,RF_PATH_A, rfRegOffset, bMaskDWord);
		/* RTPRINT(FINIT, INIT_RF, (" 0x%02x = 0x%08x\n",rfRegOffset,rfRegValue)); */
	}
	/* RTPRINT(FINIT, INIT_RF, ("<===== PHY_DumpRFReg()\n")); */
}


/*  */
/*  Move from phycfg.c to gen.c to be code independent later */
/*  */
/* Move to other DIR later----------------------------*/

/*  */
/* 	Description: */
/* 		To dump all Tx FIFO LLT related link-list table. */
/* 		Added by Roger, 2009.03.10. */
/*  */
static void DumpBBDbgPort_92CU(
	struct adapter *		Adapter
	)
{
	PHY_SetBBReg(Adapter, 0x0908, 0xffff, 0x0000);
	PHY_SetBBReg(Adapter, 0x0908, 0xffff, 0x0803);
	PHY_SetBBReg(Adapter, 0x0908, 0xffff, 0x0a06);
	PHY_SetBBReg(Adapter, 0x0908, 0xffff, 0x0007);
	PHY_SetBBReg(Adapter, 0x0908, 0xffff, 0x0100);
	PHY_SetBBReg(Adapter, 0x0a28, 0x00ff0000, 0x000f0000);
	PHY_SetBBReg(Adapter, 0x0908, 0xffff, 0x0100);
	PHY_SetBBReg(Adapter, 0x0a28, 0x00ff0000, 0x00150000);
}
