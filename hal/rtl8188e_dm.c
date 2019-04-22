// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

/* ************************************************************
 * Description:
 *
 * This file is for 92CE/92CU dynamic mechanism only
 *
 *
 * ************************************************************ */
#define _RTL8188E_DM_C_

/* ************************************************************
 * include files
 * ************************************************************ */
#include <drv_types.h>
#include <rtl8188e_hal.h>

/* ************************************************************
 * Global var
 * ************************************************************ */


static void
dm_CheckProtection(
	PADAPTER	Adapter
)
{
}

static void
dm_CheckStatistics(
	PADAPTER	Adapter
)
{
}

#ifdef CONFIG_SUPPORT_HW_WPS_PBC
static void dm_CheckPbcGPIO(_adapter *padapter)
{
	u8	tmp1byte;
	u8	bPbcPressed = false;

	if (!padapter->registrypriv.hw_wps_pbc)
		return;

	tmp1byte = rtw_read8(padapter, GPIO_IO_SEL);
	tmp1byte |= (HAL_8188E_HW_GPIO_WPS_BIT);
	rtw_write8(padapter, GPIO_IO_SEL, tmp1byte);	/* enable GPIO[2] as output mode */

	tmp1byte &= ~(HAL_8188E_HW_GPIO_WPS_BIT);
	rtw_write8(padapter,  GPIO_IN, tmp1byte);		/* reset the floating voltage level */

	tmp1byte = rtw_read8(padapter, GPIO_IO_SEL);
	tmp1byte &= ~(HAL_8188E_HW_GPIO_WPS_BIT);
	rtw_write8(padapter, GPIO_IO_SEL, tmp1byte);	/* enable GPIO[2] as input mode */

	tmp1byte = rtw_read8(padapter, GPIO_IN);

	if (tmp1byte == 0xff)
		return ;

	if (tmp1byte & HAL_8188E_HW_GPIO_WPS_BIT)
		bPbcPressed = true;

	if (bPbcPressed) {
		/* Here we only set bPbcPressed to true */
		/* After trigger PBC, the variable will be set to false */
		RTW_INFO("CheckPbcGPIO - PBC is pressed\n");
		rtw_request_wps_pbc_event(padapter);
	}
}
#endif/* #ifdef CONFIG_SUPPORT_HW_WPS_PBC */

/* Initialize GPIO setting registers */
static void
dm_InitGPIOSetting(
	PADAPTER	Adapter
)
{
	PHAL_DATA_TYPE		pHalData = GET_HAL_DATA(Adapter);

	u8	tmp1byte;

	tmp1byte = rtw_read8(Adapter, REG_GPIO_MUXCFG);
	tmp1byte &= (GPIOSEL_GPIO | ~GPIOSEL_ENBT);

	rtw_write8(Adapter, REG_GPIO_MUXCFG, tmp1byte);

}

/* ************************************************************
 * functions
 * ************************************************************ */
static void Init_ODM_ComInfo_88E(PADAPTER	Adapter)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	struct PHY_DM_STRUCT		*pDM_Odm = &(pHalData->odmpriv);
	u32  SupportAbility = 0;
	u8	cut_ver, fab_ver;

	Init_ODM_ComInfo(Adapter);

	fab_ver = ODM_TSMC;
	cut_ver = ODM_CUT_A;

	if (IS_VENDOR_8188E_I_CUT_SERIES(Adapter))
		cut_ver = ODM_CUT_I;

	odm_cmn_info_init(pDM_Odm, ODM_CMNINFO_FAB_VER, fab_ver);
	odm_cmn_info_init(pDM_Odm, ODM_CMNINFO_CUT_VER, cut_ver);

#ifdef CONFIG_DISABLE_ODM
	SupportAbility = 0;
#else
	SupportAbility =	ODM_RF_CALIBRATION |
				ODM_RF_TX_PWR_TRACK
				;
#endif

	odm_cmn_info_update(pDM_Odm, ODM_CMNINFO_ABILITY, SupportAbility);

}
static void Update_ODM_ComInfo_88E(PADAPTER	Adapter)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	struct PHY_DM_STRUCT		*pDM_Odm = &(pHalData->odmpriv);
	u32  SupportAbility = 0;
	int i;

	SupportAbility = 0
			 | ODM_BB_DIG
			 | ODM_BB_RA_MASK
			 | ODM_BB_DYNAMIC_TXPWR
			 | ODM_BB_FA_CNT
			 | ODM_BB_RSSI_MONITOR
			 | ODM_BB_CCK_PD
			 /* | ODM_BB_PWR_SAVE	 */
			 | ODM_BB_CFO_TRACKING
			 | ODM_RF_CALIBRATION
			 | ODM_RF_TX_PWR_TRACK
			 | ODM_BB_NHM_CNT
			 | ODM_BB_PRIMARY_CCA
			 /*		| ODM_BB_PWR_TRAIN */
			 ;

	if (rtw_odm_adaptivity_needed(Adapter) == true) {
		rtw_odm_adaptivity_config_msg(RTW_DBGDUMP, Adapter);
		SupportAbility |= ODM_BB_ADAPTIVITY;
	}

	if (!Adapter->registrypriv.qos_opt_enable)
		SupportAbility |= ODM_MAC_EDCA_TURBO;

#ifdef CONFIG_ANTENNA_DIVERSITY
	if (pHalData->AntDivCfg)
		SupportAbility |= ODM_BB_ANT_DIV;
#endif

#if (MP_DRIVER == 1)
	if (Adapter->registrypriv.mp_mode == 1) {
		SupportAbility = 0
				 | ODM_RF_CALIBRATION
				 | ODM_RF_TX_PWR_TRACK
				 ;
	}
#endif/* (MP_DRIVER==1) */

#ifdef CONFIG_DISABLE_ODM
	SupportAbility = 0;
#endif/* CONFIG_DISABLE_ODM */

	odm_cmn_info_update(pDM_Odm, ODM_CMNINFO_ABILITY, SupportAbility);
}

void
rtl8188e_InitHalDm(
	PADAPTER	Adapter
)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	struct PHY_DM_STRUCT		*pDM_Odm = &(pHalData->odmpriv);
	u8	i;

	dm_InitGPIOSetting(Adapter);

	pHalData->DM_Type = dm_type_by_driver;

	Update_ODM_ComInfo_88E(Adapter);
	odm_dm_init(pDM_Odm);
}


void
rtl8188e_HalDmWatchDog(
	PADAPTER	Adapter
)
{
	bool		bFwCurrentInPSMode = false;
	bool		bFwPSAwake = true;
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	struct PHY_DM_STRUCT		*pDM_Odm = &(pHalData->odmpriv);


	if (!rtw_is_hw_init_completed(Adapter))
		goto skip_dm;

#ifdef CONFIG_LPS
	bFwCurrentInPSMode = adapter_to_pwrctl(Adapter)->bFwCurrentInPSMode;
	rtw_hal_get_hwreg(Adapter, HW_VAR_FWLPS_RF_ON, (u8 *)(&bFwPSAwake));
#endif

#ifdef CONFIG_P2P_PS
	/* Fw is under p2p powersaving mode, driver should stop dynamic mechanism. */
	/* modifed by thomas. 2011.06.11. */
	if (Adapter->wdinfo.p2p_ps_mode)
		bFwPSAwake = false;
#endif /* CONFIG_P2P_PS */

	if ((rtw_is_hw_init_completed(Adapter))
	    && ((!bFwCurrentInPSMode) && bFwPSAwake)) {
		/* Calculate Tx/Rx statistics. */
		dm_CheckStatistics(Adapter);

		rtw_hal_check_rxfifo_full(Adapter);
	}

	/* ODM */
	if (rtw_is_hw_init_completed(Adapter)) {
		u8	bLinked = false;
		u8	bsta_state = false;
#ifdef CONFIG_DISABLE_ODM
		pHalData->odmpriv.support_ability = 0;
#endif

		if (rtw_mi_check_status(Adapter, MI_ASSOC)) {
			bLinked = true;
			if (rtw_mi_check_status(Adapter, MI_STA_LINKED))
				bsta_state = true;
		}

		odm_cmn_info_update(&pHalData->odmpriv , ODM_CMNINFO_LINK, bLinked);
		odm_cmn_info_update(&pHalData->odmpriv , ODM_CMNINFO_STATION_STATE, bsta_state);


		odm_dm_watchdog(&pHalData->odmpriv);

	}

skip_dm:

#ifdef CONFIG_SUPPORT_HW_WPS_PBC
	/* Check GPIO to determine current Pbc status. */
	dm_CheckPbcGPIO(Adapter);
#endif
	return;
}

void rtl8188e_init_dm_priv(PADAPTER Adapter)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	struct PHY_DM_STRUCT		*podmpriv = &pHalData->odmpriv;

	/* spin_lock_init(&(pHalData->odm_stainfo_lock)); */
	Init_ODM_ComInfo_88E(Adapter);
	odm_init_all_timers(podmpriv);
	
}

void rtl8188e_deinit_dm_priv(PADAPTER Adapter)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(Adapter);
	struct PHY_DM_STRUCT		*podmpriv = &pHalData->odmpriv;
	odm_cancel_all_timers(podmpriv);
}
