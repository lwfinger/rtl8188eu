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

//============================================================
// include files
//============================================================

#include "odm_precomp.h"
//
// ODM IO Relative API.
//

u1Byte
ODM_Read1Byte(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u4Byte			RegAddr
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	return rtw_read8(Adapter,RegAddr);
}


u2Byte
ODM_Read2Byte(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u4Byte			RegAddr
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	return rtw_read16(Adapter,RegAddr);
}

u4Byte
ODM_Read4Byte(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u4Byte			RegAddr
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	return rtw_read32(Adapter,RegAddr);
}

void
ODM_Write1Byte(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u4Byte			RegAddr,
	IN	u1Byte			Data
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	rtw_write8(Adapter,RegAddr, Data);
}

void
ODM_Write2Byte(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u4Byte			RegAddr,
	IN	u2Byte			Data
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	rtw_write16(Adapter,RegAddr, Data);
}

void
ODM_Write4Byte(
	IN	PDM_ODM_T		pDM_Odm,
	IN	u4Byte			RegAddr,
	IN	u4Byte			Data
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	rtw_write32(Adapter,RegAddr, Data);
}

void
ODM_SetMACReg(
	IN	PDM_ODM_T	pDM_Odm,
	IN	u4Byte		RegAddr,
	IN	u4Byte		BitMask,
	IN	u4Byte		Data
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
}

u4Byte
ODM_GetMACReg(
	IN	PDM_ODM_T	pDM_Odm,
	IN	u4Byte		RegAddr,
	IN	u4Byte		BitMask
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	return PHY_QueryBBReg(Adapter, RegAddr, BitMask);
}

void
ODM_SetBBReg(
	IN	PDM_ODM_T	pDM_Odm,
	IN	u4Byte		RegAddr,
	IN	u4Byte		BitMask,
	IN	u4Byte		Data
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
}


u4Byte
ODM_GetBBReg(
	IN	PDM_ODM_T	pDM_Odm,
	IN	u4Byte		RegAddr,
	IN	u4Byte		BitMask
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	return PHY_QueryBBReg(Adapter, RegAddr, BitMask);
}

void
ODM_SetRFReg(
	IN	PDM_ODM_T			pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E	eRFPath,
	IN	u4Byte				RegAddr,
	IN	u4Byte				BitMask,
	IN	u4Byte				Data
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	PHY_SetRFReg(Adapter, eRFPath, RegAddr, BitMask, Data);
}

u4Byte
ODM_GetRFReg(
	IN	PDM_ODM_T			pDM_Odm,
	IN	ODM_RF_RADIO_PATH_E	eRFPath,
	IN	u4Byte				RegAddr,
	IN	u4Byte				BitMask
	)
{
	struct adapter *	Adapter = pDM_Odm->Adapter;
	return PHY_QueryRFReg(Adapter, eRFPath, RegAddr, BitMask);
}

//
// ODM Memory relative API.
//
void
ODM_AllocateMemory(
	IN	PDM_ODM_T	pDM_Odm,
	OUT	void *		*pPtr,
	IN	u4Byte		length
	)
{
	*pPtr = rtw_zvmalloc(length);
}

// length could be ignored, used to detect memory leakage.
void
ODM_FreeMemory(
	IN	PDM_ODM_T	pDM_Odm,
	OUT	void *		pPtr,
	IN	u4Byte		length
	)
{
	rtw_vmfree(pPtr, length);
}

s4Byte ODM_CompareMemory(
	IN	PDM_ODM_T	pDM_Odm,
	IN	void *           pBuf1,
      IN	void *           pBuf2,
      IN	u4Byte          length
       )
{
	return _rtw_memcmp(pBuf1,pBuf2,length);
}

//
// ODM MISC relative API.
//
void
ODM_AcquireSpinLock(
	IN	PDM_ODM_T			pDM_Odm,
	IN	RT_SPINLOCK_TYPE	type
	)
{
}

void
ODM_ReleaseSpinLock(
	IN	PDM_ODM_T			pDM_Odm,
	IN	RT_SPINLOCK_TYPE	type
	)
{
}

//
// Work item relative API. FOr MP driver only~!
//
void
ODM_InitializeWorkItem(
	IN	PDM_ODM_T					pDM_Odm,
	IN	PRT_WORK_ITEM				pRtWorkItem,
	IN	RT_WORKITEM_CALL_BACK		RtWorkItemCallback,
	IN	void *						pContext,
	IN	const char*					szID
	)
{
}

void
ODM_StartWorkItem(
	IN	PRT_WORK_ITEM	pRtWorkItem
	)
{
}

void
ODM_StopWorkItem(
	IN	PRT_WORK_ITEM	pRtWorkItem
	)
{
}

void
ODM_FreeWorkItem(
	IN	PRT_WORK_ITEM	pRtWorkItem
	)
{
}

void
ODM_ScheduleWorkItem(
	IN	PRT_WORK_ITEM	pRtWorkItem
	)
{
}

void
ODM_IsWorkItemScheduled(
	IN	PRT_WORK_ITEM	pRtWorkItem
	)
{
}

//
// ODM Timer relative API.
//
void
ODM_StallExecution(
	IN	u4Byte	usDelay
	)
{
	rtw_udelay_os(usDelay);
}

void
ODM_delay_ms(IN u4Byte	ms)
{
	rtw_mdelay_os(ms);
}

void
ODM_delay_us(IN u4Byte	us)
{
	rtw_udelay_os(us);
}

void
ODM_sleep_ms(IN u4Byte	ms)
{
	rtw_msleep_os(ms);
}

void
ODM_sleep_us(IN u4Byte	us)
{
	rtw_usleep_os(us);
}

void
ODM_SetTimer(
	IN	PDM_ODM_T		pDM_Odm,
	IN	PRT_TIMER		pTimer,
	IN	u4Byte			msDelay
	)
{
	_set_timer(pTimer,msDelay ); //ms
}

void
ODM_InitializeTimer(
	IN	PDM_ODM_T			pDM_Odm,
	IN	PRT_TIMER			pTimer,
	IN	RT_TIMER_CALL_BACK	CallBackFunc,
	IN	void *				pContext,
	IN	const char*			szID
	)
{
	struct adapter *Adapter = pDM_Odm->Adapter;
	_init_timer(pTimer,Adapter->pnetdev,CallBackFunc,pDM_Odm);
}

void
ODM_CancelTimer(
	IN	PDM_ODM_T		pDM_Odm,
	IN	PRT_TIMER		pTimer
	)
{
	_cancel_timer_ex(pTimer);
}

void
ODM_ReleaseTimer(
	IN	PDM_ODM_T		pDM_Odm,
	IN	PRT_TIMER		pTimer
	)
{
}

//
// ODM FW relative API.
//
u4Byte
ODM_FillH2CCmd(
	IN	pu1Byte		pH2CBuffer,
	IN	u4Byte		H2CBufferLen,
	IN	u4Byte		CmdNum,
	IN	pu4Byte		pElementID,
	IN	pu4Byte		pCmdLen,
	IN	pu1Byte*		pCmbBuffer,
	IN	pu1Byte		CmdStartSeq
	)
{
	return	TRUE;
}
