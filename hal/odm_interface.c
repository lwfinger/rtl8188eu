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
/*  */
/*  ODM IO Relative API. */
/*  */

u1Byte ODM_Read1Byte(PDM_ODM_T pDM_Odm, u4Byte RegAddr)
{
	PADAPTER Adapter = pDM_Odm->Adapter;
	return rtw_read8(Adapter,RegAddr);
}

u2Byte ODM_Read2Byte(PDM_ODM_T pDM_Odm, u4Byte RegAddr)
{
	PADAPTER Adapter = pDM_Odm->Adapter;
	return rtw_read16(Adapter,RegAddr);
}

u4Byte ODM_Read4Byte(PDM_ODM_T pDM_Odm, u4Byte RegAddr)
{
	PADAPTER Adapter = pDM_Odm->Adapter;
	return rtw_read32(Adapter,RegAddr);
}

void ODM_Write1Byte(PDM_ODM_T pDM_Odm, u4Byte RegAddr, u1Byte Data)
{
	PADAPTER Adapter = pDM_Odm->Adapter;
	rtw_write8(Adapter,RegAddr, Data);
}

void ODM_Write2Byte(PDM_ODM_T pDM_Odm, u4Byte RegAddr, u2Byte Data)
{
	PADAPTER Adapter = pDM_Odm->Adapter;
	rtw_write16(Adapter,RegAddr, Data);
}

void ODM_Write4Byte(PDM_ODM_T pDM_Odm, u4Byte RegAddr, u4Byte Data)
{
	PADAPTER Adapter = pDM_Odm->Adapter;
	rtw_write32(Adapter,RegAddr, Data);
}

void ODM_SetMACReg(PDM_ODM_T pDM_Odm, u4Byte RegAddr, u4Byte BitMask, u4Byte Data)
{
	PADAPTER Adapter = pDM_Odm->Adapter;
	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
}

u4Byte
ODM_GetMACReg(
		PDM_ODM_T pDM_Odm,
		u4Byte RegAddr,
		u4Byte BitMask
	)
{
	PADAPTER Adapter = pDM_Odm->Adapter;
	return PHY_QueryBBReg(Adapter, RegAddr, BitMask);
}

void
ODM_SetBBReg(
		PDM_ODM_T pDM_Odm,
		u4Byte RegAddr,
		u4Byte BitMask,
		u4Byte Data
	)
{
	PADAPTER Adapter = pDM_Odm->Adapter;
	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
}

u4Byte
ODM_GetBBReg(
		PDM_ODM_T pDM_Odm,
		u4Byte RegAddr,
		u4Byte BitMask
	)
{
	PADAPTER Adapter = pDM_Odm->Adapter;
	return PHY_QueryBBReg(Adapter, RegAddr, BitMask);
}


void
ODM_SetRFReg(
		PDM_ODM_T pDM_Odm,
		ODM_RF_RADIO_PATH_E	eRFPath,
		u4Byte RegAddr,
		u4Byte BitMask,
		u4Byte Data
	)
{
	PADAPTER Adapter = pDM_Odm->Adapter;
	PHY_SetRFReg(Adapter, (enum _RF_RADIO_PATH)eRFPath, RegAddr, BitMask, Data);
}


u4Byte
ODM_GetRFReg(
		PDM_ODM_T pDM_Odm,
		ODM_RF_RADIO_PATH_E	eRFPath,
		u4Byte RegAddr,
		u4Byte BitMask
	)
{
	PADAPTER Adapter = pDM_Odm->Adapter;
	return PHY_QueryRFReg(Adapter, (enum _RF_RADIO_PATH)eRFPath, RegAddr, BitMask);
}




/*  */
/*  ODM Memory relative API. */
/*  */
void
ODM_AllocateMemory(
		PDM_ODM_T pDM_Odm,
		void *		*pPtr,
		u4Byte length
	)
{
	*pPtr = rtw_zvmalloc(length);
}

/*  length could be ignored, used to detect memory leakage. */
void
ODM_FreeMemory(
		PDM_ODM_T pDM_Odm,
		void *		pPtr,
		u4Byte length
	)
{
	rtw_vmfree(pPtr, length);
}

s4Byte ODM_CompareMemory(
		PDM_ODM_T pDM_Odm,
		void *           pBuf1,
      	void *           pBuf2,
      	u4Byte          length
       )
{
	return _rtw_memcmp(pBuf1,pBuf2,length);
}

/*  */
/*  ODM MISC relative API. */
/*  */
void
ODM_AcquireSpinLock(
		PDM_ODM_T pDM_Odm,
		RT_SPINLOCK_TYPE	type
	)
{
}

void
ODM_ReleaseSpinLock(
		PDM_ODM_T pDM_Odm,
		RT_SPINLOCK_TYPE	type
	)
{
}

/*  */
/*  Work item relative API. FOr MP driver only~! */
/*  */
void ODM_InitializeWorkItem(PDM_ODM_T pDM_Odm, void *pRtWorkItem,
			    RT_WORKITEM_CALL_BACK RtWorkItemCallback,
			    void *pContext, const char*szID)
{
}

void ODM_StartWorkItem(void *pRtWorkItem)
{
}

void ODM_StopWorkItem(void *pRtWorkItem)
{
}

void ODM_FreeWorkItem(void *pRtWorkItem)
{
}

void ODM_ScheduleWorkItem(void *pRtWorkItem)
{
}

void ODM_IsWorkItemScheduled(void *pRtWorkItem)
{
}

/*  */
/*  ODM Timer relative API. */
/*  */
void ODM_StallExecution(u4Byte usDelay)
{
	rtw_udelay_os(usDelay);
}

void ODM_delay_ms(u4Byte ms)
{
	rtw_mdelay_os(ms);
}

void ODM_delay_us(u4Byte us)
{
	rtw_udelay_os(us);
}

void ODM_sleep_ms(u4Byte ms)
{
	rtw_msleep_os(ms);
}

void ODM_sleep_us(u4Byte us)
{
	rtw_usleep_os(us);
}

void ODM_SetTimer(PDM_ODM_T pDM_Odm, PRT_TIMER pTimer, u4Byte msDelay)
{
	_set_timer(pTimer,msDelay ); /* ms */
}

void ODM_InitializeTimer(PDM_ODM_T pDM_Odm, PRT_TIMER pTimer,
			 RT_TIMER_CALL_BACK CallBackFunc, void *pContext,
			 const char *szID)
{
	PADAPTER Adapter = pDM_Odm->Adapter;
	_init_timer(pTimer,Adapter->pnetdev,CallBackFunc,pDM_Odm);
}

void ODM_CancelTimer(PDM_ODM_T pDM_Odm, PRT_TIMER pTimer)
{
	_cancel_timer_ex(pTimer);
}

void ODM_ReleaseTimer(PDM_ODM_T pDM_Odm, PRT_TIMER pTimer)
{
}

/*  */
/*  ODM FW relative API. */
/*  */
u4Byte
ODM_FillH2CCmd(
		pu1Byte pH2CBuffer,
		u4Byte H2CBufferLen,
		u4Byte CmdNum,
		pu4Byte pElementID,
		pu4Byte pCmdLen,
		pu1Byte*		pCmbBuffer,
		pu1Byte CmdStartSeq
	)
{
	return	true;
}
