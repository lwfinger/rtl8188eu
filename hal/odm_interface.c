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
		PDM_ODM_T		pDM_Odm,
		u4Byte			RegAddr
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	prtl8192cd_priv	priv	= pDM_Odm->priv;
	return	RTL_R8(RegAddr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return rtw_read8(Adapter,RegAddr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return	PlatformEFIORead1Byte(Adapter, RegAddr);
#endif

}


u2Byte
ODM_Read2Byte(
		PDM_ODM_T		pDM_Odm,
		u4Byte			RegAddr
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	prtl8192cd_priv	priv	= pDM_Odm->priv;
	return	RTL_R16(RegAddr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return rtw_read16(Adapter,RegAddr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return	PlatformEFIORead2Byte(Adapter, RegAddr);
#endif

}


u4Byte
ODM_Read4Byte(
		PDM_ODM_T		pDM_Odm,
		u4Byte			RegAddr
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	prtl8192cd_priv	priv	= pDM_Odm->priv;
	return	RTL_R32(RegAddr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return rtw_read32(Adapter,RegAddr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return	PlatformEFIORead4Byte(Adapter, RegAddr);
#endif

}


void
ODM_Write1Byte(
		PDM_ODM_T		pDM_Odm,
		u4Byte			RegAddr,
		u1Byte			Data
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	prtl8192cd_priv	priv	= pDM_Odm->priv;
	RTL_W8(RegAddr, Data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	rtw_write8(Adapter,RegAddr, Data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PlatformEFIOWrite1Byte(Adapter, RegAddr, Data);
#endif

}


void
ODM_Write2Byte(
		PDM_ODM_T		pDM_Odm,
		u4Byte			RegAddr,
		u2Byte			Data
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	prtl8192cd_priv	priv	= pDM_Odm->priv;
	RTL_W16(RegAddr, Data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	rtw_write16(Adapter,RegAddr, Data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PlatformEFIOWrite2Byte(Adapter, RegAddr, Data);
#endif

}


void
ODM_Write4Byte(
		PDM_ODM_T		pDM_Odm,
		u4Byte			RegAddr,
		u4Byte			Data
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	prtl8192cd_priv	priv	= pDM_Odm->priv;
	RTL_W32(RegAddr, Data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	rtw_write32(Adapter,RegAddr, Data);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PlatformEFIOWrite4Byte(Adapter, RegAddr, Data);
#endif

}


void
ODM_SetMACReg(
		PDM_ODM_T	pDM_Odm,
		u4Byte		RegAddr,
		u4Byte		BitMask,
		u4Byte		Data
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	PHY_SetBBReg(pDM_Odm->priv, RegAddr, BitMask, Data);
#elif (DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_MP))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
#endif
}


u4Byte
ODM_GetMACReg(
		PDM_ODM_T	pDM_Odm,
		u4Byte		RegAddr,
		u4Byte		BitMask
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	return PHY_QueryBBReg(pDM_Odm->priv, RegAddr, BitMask);
#elif (DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_MP))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return PHY_QueryBBReg(Adapter, RegAddr, BitMask);
#endif
}


void
ODM_SetBBReg(
		PDM_ODM_T	pDM_Odm,
		u4Byte		RegAddr,
		u4Byte		BitMask,
		u4Byte		Data
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	PHY_SetBBReg(pDM_Odm->priv, RegAddr, BitMask, Data);
#elif (DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_MP))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PHY_SetBBReg(Adapter, RegAddr, BitMask, Data);
#endif
}


u4Byte
ODM_GetBBReg(
		PDM_ODM_T	pDM_Odm,
		u4Byte		RegAddr,
		u4Byte		BitMask
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	return PHY_QueryBBReg(pDM_Odm->priv, RegAddr, BitMask);
#elif (DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_MP))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return PHY_QueryBBReg(Adapter, RegAddr, BitMask);
#endif
}


void
ODM_SetRFReg(
		PDM_ODM_T			pDM_Odm,
		ODM_RF_RADIO_PATH_E	eRFPath,
		u4Byte				RegAddr,
		u4Byte				BitMask,
		u4Byte				Data
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	PHY_SetRFReg(pDM_Odm->priv, eRFPath, RegAddr, BitMask, Data);
#elif (DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_MP))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PHY_SetRFReg(Adapter, eRFPath, RegAddr, BitMask, Data);
#endif
}


u4Byte
ODM_GetRFReg(
		PDM_ODM_T			pDM_Odm,
		ODM_RF_RADIO_PATH_E	eRFPath,
		u4Byte				RegAddr,
		u4Byte				BitMask
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	return PHY_QueryRFReg(pDM_Odm->priv, eRFPath, RegAddr, BitMask, 1);
#elif (DM_ODM_SUPPORT_TYPE & (ODM_CE|ODM_MP))
	PADAPTER		Adapter = pDM_Odm->Adapter;
	return PHY_QueryRFReg(Adapter, eRFPath, RegAddr, BitMask);
#endif
}




//
// ODM Memory relative API.
//
void
ODM_AllocateMemory(
		PDM_ODM_T	pDM_Odm,
		void *		*pPtr,
		u4Byte		length
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	*pPtr = kmalloc(length, GFP_ATOMIC);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE )
	*pPtr = rtw_zvmalloc(length);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PlatformAllocateMemory(Adapter, pPtr, length);
#endif
}

// length could be ignored, used to detect memory leakage.
void
ODM_FreeMemory(
		PDM_ODM_T	pDM_Odm,
		void *		pPtr,
		u4Byte		length
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	kfree(pPtr);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE )
	rtw_vmfree(pPtr, length);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	//PADAPTER    Adapter = pDM_Odm->Adapter;
	PlatformFreeMemory(pPtr, length);
#endif
}
s4Byte ODM_CompareMemory(
		PDM_ODM_T	pDM_Odm,
		void *           pBuf1,
      	void *           pBuf2,
      	u4Byte          length
       )
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	return memcmp(pBuf1,pBuf2,length);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE )
	return _rtw_memcmp(pBuf1,pBuf2,length);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	return PlatformCompareMemory(pBuf1,pBuf2,length);
#endif
}



//
// ODM MISC relative API.
//
void
ODM_AcquireSpinLock(
		PDM_ODM_T			pDM_Odm,
		RT_SPINLOCK_TYPE	type
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE )

#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PlatformAcquireSpinLock(Adapter, type);
#endif
}
void
ODM_ReleaseSpinLock(
		PDM_ODM_T			pDM_Odm,
		RT_SPINLOCK_TYPE	type
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE )

#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PlatformReleaseSpinLock(Adapter, type);
#endif
}

//
// Work item relative API. FOr MP driver only~!
//
void
ODM_InitializeWorkItem(
		PDM_ODM_T					pDM_Odm,
		PRT_WORK_ITEM				pRtWorkItem,
		RT_WORKITEM_CALL_BACK		RtWorkItemCallback,
		void *						pContext,
		const char*					szID
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PlatformInitializeWorkItem(Adapter, pRtWorkItem, RtWorkItemCallback, pContext, szID);
#endif
}


void
ODM_StartWorkItem(
		PRT_WORK_ITEM	pRtWorkItem
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PlatformStartWorkItem(pRtWorkItem);
#endif
}


void
ODM_StopWorkItem(
		PRT_WORK_ITEM	pRtWorkItem
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PlatformStopWorkItem(pRtWorkItem);
#endif
}


void
ODM_FreeWorkItem(
		PRT_WORK_ITEM	pRtWorkItem
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PlatformFreeWorkItem(pRtWorkItem);
#endif
}


void
ODM_ScheduleWorkItem(
		PRT_WORK_ITEM	pRtWorkItem
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PlatformScheduleWorkItem(pRtWorkItem);
#endif
}


void
ODM_IsWorkItemScheduled(
		PRT_WORK_ITEM	pRtWorkItem
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PlatformIsWorkItemScheduled(pRtWorkItem);
#endif
}



//
// ODM Timer relative API.
//
void
ODM_StallExecution(
		u4Byte	usDelay
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_udelay_os(usDelay);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PlatformStallExecution(usDelay);
#endif
}

void
ODM_delay_ms(u4Byte	ms)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	delay_ms(ms);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_mdelay_os(ms);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	delay_ms(ms);
#endif
}

void
ODM_delay_us(u4Byte	us)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	delay_us(us);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_udelay_os(us);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PlatformStallExecution(us);
#endif
}

void
ODM_sleep_ms(u4Byte	ms)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_msleep_os(ms);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
#endif
}

void
ODM_sleep_us(u4Byte	us)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	rtw_usleep_os(us);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
#endif
}

void
ODM_SetTimer(
		PDM_ODM_T		pDM_Odm,
		PRT_TIMER		pTimer,
		u4Byte			msDelay
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	mod_timer(pTimer, jiffies + (msDelay+9)/10);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	_set_timer(pTimer,msDelay ); //ms
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PADAPTER		Adapter = pDM_Odm->Adapter;
	PlatformSetTimer(Adapter, pTimer, msDelay);
#endif

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
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	pTimer->function = CallBackFunc;
	pTimer->data = (unsigned long)pDM_Odm;
	init_timer(pTimer);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	PADAPTER Adapter = pDM_Odm->Adapter;
	_init_timer(pTimer,Adapter->pnetdev,CallBackFunc,pDM_Odm);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PADAPTER Adapter = pDM_Odm->Adapter;
	PlatformInitializeTimer(Adapter, pTimer, CallBackFunc,pContext,szID);
#endif
}


void
ODM_CancelTimer(
		PDM_ODM_T		pDM_Odm,
		PRT_TIMER		pTimer
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))
	del_timer_sync(pTimer);
#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)
	_cancel_timer_ex(pTimer);
#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	PADAPTER Adapter = pDM_Odm->Adapter;
	PlatformCancelTimer(Adapter, pTimer);
#endif
}


void
ODM_ReleaseTimer(
		PDM_ODM_T		pDM_Odm,
		PRT_TIMER		pTimer
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)

	PADAPTER Adapter = pDM_Odm->Adapter;

    // <20120301, Kordan> If the initilization fails, InitializeAdapterXxx will return regardless of InitHalDm.
    // Hence, uninitialized timers cause BSOD when the driver releases resources since the init fail.
    if (pTimer == 0)
    {
        ODM_RT_TRACE(pDM_Odm, ODM_COMP_INIT, ODM_DBG_SERIOUS, ("=====>ODM_ReleaseTimer(), The timer is NULL! Please check it!\n"));
        return;
    }

	PlatformReleaseTimer(Adapter, pTimer);
#endif
}


//
// ODM FW relative API.
//
#if (DM_ODM_SUPPORT_TYPE & ODM_MP)
void
ODM_FillH2CCmd(
		PADAPTER		Adapter,
		u1Byte	ElementID,
		u4Byte	CmdLen,
		pu1Byte	pCmdBuffer
)
{
	if (IS_HARDWARE_TYPE_JAGUAR(Adapter))
	{
		switch (ElementID)
		{
		case ODM_H2C_RSSI_REPORT:
			FillH2CCmd8812(Adapter, H2C_8812_RSSI_REPORT, CmdLen, pCmdBuffer);
		default:
			break;
		}

	}
	else if (IS_HARDWARE_TYPE_8188E(Adapter))
	{
		switch (ElementID)
		{
		case ODM_H2C_PSD_RESULT:
			FillH2CCmd88E(Adapter, H2C_88E_PSD_RESULT, CmdLen, pCmdBuffer);
		default:
			break;
		}
	}
	else
	{
		switch (ElementID)
		{
		case ODM_H2C_RSSI_REPORT:
			FillH2CCmd92C(Adapter, H2C_RSSI_REPORT, CmdLen, pCmdBuffer);
		case ODM_H2C_PSD_RESULT:
			FillH2CCmd92C(Adapter, H2C_92C_PSD_RESULT, CmdLen, pCmdBuffer);
		default:
			break;
		}
	}
}
#else
u4Byte
ODM_FillH2CCmd(
		pu1Byte		pH2CBuffer,
		u4Byte		H2CBufferLen,
		u4Byte		CmdNum,
		pu4Byte		pElementID,
		pu4Byte		pCmdLen,
		pu1Byte*		pCmbBuffer,
		pu1Byte		CmdStartSeq
	)
{
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP|ODM_ADSL))

#elif (DM_ODM_SUPPORT_TYPE & ODM_CE)

#elif (DM_ODM_SUPPORT_TYPE & ODM_MP)
	//FillH2CCmd(pH2CBuffer, H2CBufferLen, CmdNum, pElementID, pCmdLen, pCmbBuffer, CmdStartSeq);
	return	FALSE;
#endif

	return	TRUE;
}
#endif
