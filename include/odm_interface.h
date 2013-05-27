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


#ifndef	__ODM_INTERFACE_H__
#define __ODM_INTERFACE_H__



//
// =========== Constant/Structure/Enum/... Define
//



//
// =========== Macro Define
//

#define _reg_all(_name)			ODM_##_name
#define _reg_ic(_name, _ic)		ODM_##_name##_ic
#define _bit_all(_name)			BIT_##_name
#define _bit_ic(_name, _ic)		BIT_##_name##_ic

// _cat: implemented by Token-Pasting Operator.

/*===================================

#define ODM_REG_DIG_11N		0xC50
#define ODM_REG_DIG_11AC	0xDDD

ODM_REG(DIG,_pDM_Odm)
=====================================*/

#define _reg_11N(_name)			ODM_REG_##_name##_11N
#define _reg_11AC(_name)		ODM_REG_##_name##_11AC
#define _bit_11N(_name)			ODM_BIT_##_name##_11N
#define _bit_11AC(_name)		ODM_BIT_##_name##_11AC

#if 1 //TODO: enable it if we need to support run-time to differentiate between 92C_SERIES and JAGUAR_SERIES.
#define _cat(_name, _ic_type, _func)									\
	(															\
		((_ic_type) & ODM_IC_11N_SERIES)? _func##_11N(_name):		\
		_func##_11AC(_name)									\
	)
#endif

// _name: name of register or bit.
// Example: "ODM_REG(R_A_AGC_CORE1, pDM_Odm)"
//        gets "ODM_R_A_AGC_CORE1" or "ODM_R_A_AGC_CORE1_8192C", depends on SupportICType.
#define ODM_REG(_name, _pDM_Odm)	_cat(_name, _pDM_Odm->SupportICType, _reg)
#define ODM_BIT(_name, _pDM_Odm)	_cat(_name, _pDM_Odm->SupportICType, _bit)

typedef enum _ODM_H2C_CMD
{
	ODM_H2C_RSSI_REPORT = 0,
	ODM_H2C_PSD_RESULT=1,
	ODM_H2C_PathDiv = 2,
	ODM_MAX_H2CCMD
}ODM_H2C_CMD;


//
// 2012/02/17 MH For non-MP compile pass only. Linux does not support workitem.
// Suggest HW team to use thread instead of workitem. Windows also support the feature.
//
#if (DM_ODM_SUPPORT_TYPE != ODM_MP)
typedef  void *PRT_WORK_ITEM ;
typedef  void RT_WORKITEM_HANDLE,*PRT_WORKITEM_HANDLE;
typedef void (*RT_WORKITEM_CALL_BACK)(void * pContext);
#endif

//
// =========== Extern Variable ??? It should be forbidden.
//


//
// =========== EXtern Function Prototype
//


u1Byte
ODM_Read1Byte(
		PDM_ODM_T		pDM_Odm,
		u4Byte			RegAddr
	);

u2Byte
ODM_Read2Byte(
		PDM_ODM_T		pDM_Odm,
		u4Byte			RegAddr
	);

u4Byte
ODM_Read4Byte(
		PDM_ODM_T		pDM_Odm,
		u4Byte			RegAddr
	);

void
ODM_Write1Byte(
		PDM_ODM_T		pDM_Odm,
		u4Byte			RegAddr,
		u1Byte			Data
	);

void
ODM_Write2Byte(
		PDM_ODM_T		pDM_Odm,
		u4Byte			RegAddr,
		u2Byte			Data
	);

void
ODM_Write4Byte(
		PDM_ODM_T		pDM_Odm,
		u4Byte			RegAddr,
		u4Byte			Data
	);

void
ODM_SetMACReg(
		PDM_ODM_T	pDM_Odm,
		u4Byte		RegAddr,
		u4Byte		BitMask,
		u4Byte		Data
	);

u4Byte
ODM_GetMACReg(
		PDM_ODM_T	pDM_Odm,
		u4Byte		RegAddr,
		u4Byte		BitMask
	);

void
ODM_SetBBReg(
		PDM_ODM_T	pDM_Odm,
		u4Byte		RegAddr,
		u4Byte		BitMask,
		u4Byte		Data
	);

u4Byte
ODM_GetBBReg(
		PDM_ODM_T	pDM_Odm,
		u4Byte		RegAddr,
		u4Byte		BitMask
	);

void
ODM_SetRFReg(
		PDM_ODM_T				pDM_Odm,
		ODM_RF_RADIO_PATH_E	eRFPath,
		u4Byte					RegAddr,
		u4Byte					BitMask,
		u4Byte					Data
	);

u4Byte
ODM_GetRFReg(
		PDM_ODM_T				pDM_Odm,
		ODM_RF_RADIO_PATH_E	eRFPath,
		u4Byte					RegAddr,
		u4Byte					BitMask
	);


//
// Memory Relative Function.
//
void
ODM_AllocateMemory(
		PDM_ODM_T	pDM_Odm,
		void *		*pPtr,
		u4Byte		length
	);
void
ODM_FreeMemory(
		PDM_ODM_T	pDM_Odm,
		void *		pPtr,
		u4Byte		length
	);

s4Byte ODM_CompareMemory(
		PDM_ODM_T	pDM_Odm,
		void *           pBuf1,
      	void *           pBuf2,
      	u4Byte          length
       );

//
// ODM MISC-spin lock relative API.
//
void
ODM_AcquireSpinLock(
		PDM_ODM_T			pDM_Odm,
		RT_SPINLOCK_TYPE	type
	);

void
ODM_ReleaseSpinLock(
		PDM_ODM_T			pDM_Odm,
		RT_SPINLOCK_TYPE	type
	);


//
// ODM MISC-workitem relative API.
//
void
ODM_InitializeWorkItem(
		PDM_ODM_T					pDM_Odm,
		PRT_WORK_ITEM				pRtWorkItem,
		RT_WORKITEM_CALL_BACK		RtWorkItemCallback,
		void *						pContext,
		const char*					szID
	);

void
ODM_StartWorkItem(
		PRT_WORK_ITEM	pRtWorkItem
	);

void
ODM_StopWorkItem(
		PRT_WORK_ITEM	pRtWorkItem
	);

void
ODM_FreeWorkItem(
		PRT_WORK_ITEM	pRtWorkItem
	);

void
ODM_ScheduleWorkItem(
		PRT_WORK_ITEM	pRtWorkItem
	);

void
ODM_IsWorkItemScheduled(
		PRT_WORK_ITEM	pRtWorkItem
	);

//
// ODM Timer relative API.
//
void
ODM_StallExecution(
		u4Byte	usDelay
	);

void
ODM_delay_ms(u4Byte	ms);


void
ODM_delay_us(u4Byte	us);

void
ODM_sleep_ms(u4Byte	ms);

void
ODM_sleep_us(u4Byte	us);

void
ODM_SetTimer(
		PDM_ODM_T		pDM_Odm,
		PRT_TIMER		pTimer,
		u4Byte			msDelay
	);

void
ODM_InitializeTimer(
		PDM_ODM_T			pDM_Odm,
		PRT_TIMER			pTimer,
		RT_TIMER_CALL_BACK	CallBackFunc,
		void *				pContext,
		const char*			szID
	);

void
ODM_CancelTimer(
		PDM_ODM_T		pDM_Odm,
		PRT_TIMER		pTimer
	);

void
ODM_ReleaseTimer(
		PDM_ODM_T		pDM_Odm,
		PRT_TIMER		pTimer
	);


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
);
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
	);
#endif
#endif	// __ODM_INTERFACE_H__
