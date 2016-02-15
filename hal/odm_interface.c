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
#include "Hal8188EPhyCfg.h"

/*  ODM IO Relative API. */

u8
ODM_Read1Byte(
	PDM_ODM_T		pDM_Odm,
	u32			RegAddr
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	return rtw_read8(Adapter,RegAddr);
}


u16
ODM_Read2Byte(
	PDM_ODM_T		pDM_Odm,
	u32			RegAddr
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	return rtw_read16(Adapter,RegAddr);
}

u32
ODM_Read4Byte(
	PDM_ODM_T		pDM_Odm,
	u32			RegAddr
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	return rtw_read32(Adapter,RegAddr);
}

void
ODM_Write1Byte(
	PDM_ODM_T		pDM_Odm,
	u32			RegAddr,
	u8			Data
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	rtw_write8(Adapter,RegAddr, Data);
}

void
ODM_Write2Byte(
	PDM_ODM_T		pDM_Odm,
	u32			RegAddr,
	u16			Data
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	rtw_write16(Adapter,RegAddr, Data);
}

void
ODM_Write4Byte(
	PDM_ODM_T		pDM_Odm,
	u32			RegAddr,
	u32			Data
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	rtw_write32(Adapter,RegAddr, Data);
}

void
ODM_SetMACReg(
	PDM_ODM_T	pDM_Odm,
	u32		RegAddr,
	u32		BitMask,
	u32		Data
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
}

u32
ODM_GetMACReg(
	PDM_ODM_T	pDM_Odm,
	u32		RegAddr,
	u32		BitMask
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	return PHY_QueryBBReg(Adapter, RegAddr, BitMask);
}

void
ODM_SetBBReg(
	PDM_ODM_T	pDM_Odm,
	u32		RegAddr,
	u32		BitMask,
	u32		Data
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
}


u32
ODM_GetBBReg(
	PDM_ODM_T	pDM_Odm,
	u32		RegAddr,
	u32		BitMask
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	return PHY_QueryBBReg(Adapter, RegAddr, BitMask);
}

void
ODM_SetRFReg(
	PDM_ODM_T			pDM_Odm,
	enum rf_radio_path	eRFPath,
	u32				RegAddr,
	u32				BitMask,
	u32				Data
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	PHY_SetRFReg(Adapter, (enum rf_radio_path)eRFPath, RegAddr, BitMask, Data);
}

u32
ODM_GetRFReg(
	PDM_ODM_T			pDM_Odm,
	enum rf_radio_path	eRFPath,
	u32				RegAddr,
	u32				BitMask
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;

	return PHY_QueryRFReg(Adapter, (enum rf_radio_path)eRFPath,
			      RegAddr, BitMask);
}

/*  */
/*  ODM Memory relative API. */
/*  */
void
ODM_AllocateMemory(
	PDM_ODM_T	pDM_Odm,
	void *		*pPtr,
	u32		length
	)
{
	*pPtr = rtw_zvmalloc(length);
}

/*  length could be ignored, used to detect memory leakage. */
void
ODM_FreeMemory(
	PDM_ODM_T	pDM_Odm,
	void *		pPtr,
	u32		length
	)
{
	rtw_vmfree(pPtr, length);
}

s32 ODM_CompareMemory(
	PDM_ODM_T	pDM_Odm,
	void *           pBuf1,
      void *           pBuf2,
      u32          length
       )
{
	return _rtw_memcmp(pBuf1,pBuf2,length);
}

/*  */
/*  ODM MISC relative API. */
/*  */
void
ODM_AcquireSpinLock(
	PDM_ODM_T			pDM_Odm,
	RT_SPINLOCK_TYPE	type
	)
{
}

void
ODM_ReleaseSpinLock(
	PDM_ODM_T			pDM_Odm,
	RT_SPINLOCK_TYPE	type
	)
{
}

/*  */
/*  Work item relative API. FOr MP driver only~! */
/*  */
void
ODM_InitializeWorkItem(
	PDM_ODM_T					pDM_Odm,
	PRT_WORK_ITEM				pRtWorkItem,
	RT_WORKITEM_CALL_BACK		RtWorkItemCallback,
	void *						pContext,
	const char*					szID
	)
{
}

void
ODM_StartWorkItem(
	PRT_WORK_ITEM	pRtWorkItem
	)
{
}

void
ODM_StopWorkItem(
	PRT_WORK_ITEM	pRtWorkItem
	)
{
}

void
ODM_FreeWorkItem(
	PRT_WORK_ITEM	pRtWorkItem
	)
{
}

void
ODM_ScheduleWorkItem(
	PRT_WORK_ITEM	pRtWorkItem
	)
{
}

void
ODM_IsWorkItemScheduled(
	PRT_WORK_ITEM	pRtWorkItem
	)
{
}

/*  */
/*  ODM Timer relative API. */
/*  */
void
ODM_StallExecution(
	u32	usDelay
	)
{
	rtw_udelay_os(usDelay);
}

void
ODM_delay_ms(u32	ms)
{
	rtw_mdelay_os(ms);
}

void
ODM_delay_us(u32	us)
{
	rtw_udelay_os(us);
}

void
ODM_sleep_ms(u32	ms)
{
	rtw_msleep_os(ms);
}

void
ODM_sleep_us(u32	us)
{
	rtw_usleep_os(us);
}

void
ODM_SetTimer(
	PDM_ODM_T		pDM_Odm,
	PRT_TIMER		pTimer,
	u32			msDelay
	)
{
	_set_timer(pTimer,msDelay ); /* ms */
}

void
ODM_InitializeTimer(
	PDM_ODM_T			pDM_Odm,
	PRT_TIMER			pTimer,
	RT_TIMER_CALL_BACK	CallBackFunc,
	void *				pContext,
	const char*			szID
	)
{
	struct adapter *Adapter = pDM_Odm->Adapter;
	_init_timer(pTimer,Adapter->pnetdev,CallBackFunc,pDM_Odm);
}

void
ODM_CancelTimer(
	PDM_ODM_T		pDM_Odm,
	PRT_TIMER		pTimer
	)
{
	_cancel_timer_ex(pTimer);
}

void
ODM_ReleaseTimer(
	PDM_ODM_T		pDM_Odm,
	PRT_TIMER		pTimer
	)
{
}

/*  */
/*  ODM FW relative API. */
/*  */
u32
ODM_FillH2CCmd(
	u8 *		pH2CBuffer,
	u32		H2CBufferLen,
	u32		CmdNum,
	u32 *		pElementID,
	u32 *		pCmdLen,
	u8 **		pCmbBuffer,
	u8 *		CmdStartSeq
	)
{
	return	true;
}
