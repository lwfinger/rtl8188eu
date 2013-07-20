/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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
#define _RTW_PWRCTRL_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <osdep_intf.h>
#include <linux/usb.h>

#ifdef CONFIG_BT_COEXIST
#include <rtl8723a_hal.h>
#endif

void ips_enter(_adapter * padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct xmit_priv *pxmit_priv = &padapter->xmitpriv;

#if (MP_DRIVER == 1)
	if (padapter->registrypriv.mp_mode == 1)
		return;
#endif

	if (pxmit_priv->free_xmitbuf_cnt != NR_XMITBUFF ||
		pxmit_priv->free_xmit_extbuf_cnt != NR_XMIT_EXTBUFF) {
		DBG_88E_LEVEL(_drv_info_, "There are some pkts to transmit\n");
		DBG_88E_LEVEL(_drv_info_, "free_xmitbuf_cnt: %d, free_xmit_extbuf_cnt: %d\n",
			pxmit_priv->free_xmitbuf_cnt, pxmit_priv->free_xmit_extbuf_cnt);
		return;
	}

	_enter_pwrlock(&pwrpriv->lock);

	pwrpriv->bips_processing = true;

	/*  syn ips_mode with request */
	pwrpriv->ips_mode = pwrpriv->ips_mode_req;

	pwrpriv->ips_enter_cnts++;
	DBG_88E("==>ips_enter cnts:%d\n",pwrpriv->ips_enter_cnts);
#ifdef CONFIG_BT_COEXIST
	BTDM_TurnOffBtCoexistBeforeEnterIPS(padapter);
#endif
	if (rf_off == pwrpriv->change_rfpwrstate )
	{
		pwrpriv->bpower_saving = true;
		DBG_88E_LEVEL(_drv_info_, "nolinked power save enter\n");

		if (pwrpriv->ips_mode == IPS_LEVEL_2)
			pwrpriv->bkeepfwalive = true;

		rtw_ips_pwr_down(padapter);
		pwrpriv->rf_pwrstate = rf_off;
	}
	pwrpriv->bips_processing = false;

	_exit_pwrlock(&pwrpriv->lock);

}

int ips_leave(_adapter * padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct security_priv* psecuritypriv=&(padapter->securitypriv);
	struct mlme_priv		*pmlmepriv = &(padapter->mlmepriv);
	int result = _SUCCESS;
	sint keyid;


	_enter_pwrlock(&pwrpriv->lock);

	if ((pwrpriv->rf_pwrstate == rf_off) &&(!pwrpriv->bips_processing))
	{
		pwrpriv->bips_processing = true;
		pwrpriv->change_rfpwrstate = rf_on;
		pwrpriv->ips_leave_cnts++;
		DBG_88E("==>ips_leave cnts:%d\n",pwrpriv->ips_leave_cnts);

		if ((result = rtw_ips_pwr_up(padapter)) == _SUCCESS) {
			pwrpriv->rf_pwrstate = rf_on;
		}
		DBG_88E_LEVEL(_drv_info_, "nolinked power save leave\n");

		if ((_WEP40_ == psecuritypriv->dot11PrivacyAlgrthm) ||(_WEP104_ == psecuritypriv->dot11PrivacyAlgrthm))
		{
			DBG_88E("==>%s,channel(%d),processing(%x)\n",__func__,padapter->mlmeextpriv.cur_channel,pwrpriv->bips_processing);
			set_channel_bwmode(padapter, padapter->mlmeextpriv.cur_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HT_CHANNEL_WIDTH_20);
			for (keyid=0;keyid<4;keyid++){
				if (pmlmepriv->key_mask & BIT(keyid)){
					if (keyid == psecuritypriv->dot11PrivacyKeyIndex)
						result=rtw_set_key(padapter,psecuritypriv, keyid, 1);
					else
						result=rtw_set_key(padapter,psecuritypriv, keyid, 0);
				}
			}
		}

		DBG_88E("==> ips_leave.....LED(0x%08x)...\n",rtw_read32(padapter,0x4c));
		pwrpriv->bips_processing = false;

		pwrpriv->bkeepfwalive = false;
		pwrpriv->bpower_saving = false;
	}

	_exit_pwrlock(&pwrpriv->lock);

	return result;
}

#ifdef CONFIG_AUTOSUSPEND
extern void autosuspend_enter(_adapter* padapter);
extern int autoresume_enter(_adapter* padapter);
#endif

static bool rtw_pwr_unassociated_idle(_adapter *adapter)
{
	_adapter *buddy = adapter->pbuddy_adapter;
	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &(adapter->wdinfo);
#ifdef CONFIG_IOCTL_CFG80211
	struct cfg80211_wifidirect_info *pcfg80211_wdinfo = &adapter->cfg80211_wdinfo;
#endif
#endif

	bool ret = false;

	if (adapter->pwrctrlpriv.ips_deny_time >= rtw_get_current_time()) {
		/* DBG_88E("%s ips_deny_time\n", __func__); */
		goto exit;
	}

	if (check_fwstate(pmlmepriv, WIFI_ASOC_STATE|WIFI_SITE_MONITOR)
		|| check_fwstate(pmlmepriv, WIFI_UNDER_LINKING|WIFI_UNDER_WPS)
		|| check_fwstate(pmlmepriv, WIFI_AP_STATE)
		|| check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE|WIFI_ADHOC_STATE)
		#if defined(CONFIG_P2P) && defined(CONFIG_IOCTL_CFG80211) && defined(CONFIG_P2P_IPS)
		|| pcfg80211_wdinfo->is_ro_ch
		#elif defined(CONFIG_P2P)
		|| !rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)
		#endif
	) {
		goto exit;
	}

	/* consider buddy, if exist */
	if (buddy) {
		struct mlme_priv *b_pmlmepriv = &(buddy->mlmepriv);
		#ifdef CONFIG_P2P
		struct wifidirect_info *b_pwdinfo = &(buddy->wdinfo);
		#ifdef CONFIG_IOCTL_CFG80211
		struct cfg80211_wifidirect_info *b_pcfg80211_wdinfo = &buddy->cfg80211_wdinfo;
		#endif
		#endif

		if (check_fwstate(b_pmlmepriv, WIFI_ASOC_STATE|WIFI_SITE_MONITOR)
			|| check_fwstate(b_pmlmepriv, WIFI_UNDER_LINKING|WIFI_UNDER_WPS)
			|| check_fwstate(b_pmlmepriv, WIFI_AP_STATE)
			|| check_fwstate(b_pmlmepriv, WIFI_ADHOC_MASTER_STATE|WIFI_ADHOC_STATE)
			#if defined(CONFIG_P2P) && defined(CONFIG_IOCTL_CFG80211) && defined(CONFIG_P2P_IPS)
			|| b_pcfg80211_wdinfo->is_ro_ch
			#elif defined(CONFIG_P2P)
			|| !rtw_p2p_chk_state(b_pwdinfo, P2P_STATE_NONE)
			#endif
		) {
			goto exit;
		}
	}
	ret = true;

exit:
	return ret;
}

void rtw_ps_processor(_adapter*padapter)
{
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
#endif /* CONFIG_P2P */
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
#ifdef SUPPORT_HW_RFOFF_DETECTED
	rt_rf_power_state rfpwrstate;
#endif /* SUPPORT_HW_RFOFF_DETECTED */

	pwrpriv->ps_processing = true;

#ifdef SUPPORT_HW_RFOFF_DETECTED
	if (pwrpriv->bips_processing == true)
		goto exit;

	/* DBG_88E("==> fw report state(0x%x)\n",rtw_read8(padapter,0x1ca)); */
	if (padapter->pwrctrlpriv.bHWPwrPindetect)
	{
	#ifdef CONFIG_AUTOSUSPEND
		if (padapter->registrypriv.usbss_enable)
		{
			if (pwrpriv->rf_pwrstate == rf_on)
			{
				if (padapter->net_closed == true)
					pwrpriv->ps_flag = true;

				rfpwrstate = RfOnOffDetect(padapter);
				DBG_88E("@@@@- #1  %s==> rfstate:%s\n",__func__,(rfpwrstate==rf_on)?"rf_on":"rf_off");
				if (rfpwrstate!= pwrpriv->rf_pwrstate)
				{
					if (rfpwrstate == rf_off)
					{
						pwrpriv->change_rfpwrstate = rf_off;

						pwrpriv->bkeepfwalive = true;
						pwrpriv->brfoffbyhw = true;

						autosuspend_enter(padapter);
					}
				}
			}
		}
		else
	#endif /* CONFIG_AUTOSUSPEND */
		{
			rfpwrstate = RfOnOffDetect(padapter);
			DBG_88E("@@@@- #2  %s==> rfstate:%s\n",__func__,(rfpwrstate==rf_on)?"rf_on":"rf_off");

			if (rfpwrstate!= pwrpriv->rf_pwrstate)
			{
				if (rfpwrstate == rf_off)
				{
					pwrpriv->change_rfpwrstate = rf_off;
					pwrpriv->brfoffbyhw = true;
					padapter->bCardDisableWOHSM = true;
					rtw_hw_suspend(padapter );
				}
				else
				{
					pwrpriv->change_rfpwrstate = rf_on;
					rtw_hw_resume(padapter );
				}
				DBG_88E("current rf_pwrstate(%s)\n",(pwrpriv->rf_pwrstate == rf_off)?"rf_off":"rf_on");
			}
		}
		pwrpriv->pwr_state_check_cnts ++;
	}
#endif /* SUPPORT_HW_RFOFF_DETECTED */

	if (pwrpriv->ips_mode_req == IPS_NONE)
		goto exit;

	if (rtw_pwr_unassociated_idle(padapter) == false)
		goto exit;

	if ((pwrpriv->rf_pwrstate == rf_on) && ((pwrpriv->pwr_state_check_cnts%4)==0))
	{
		DBG_88E("==>%s .fw_state(%x)\n",__func__,get_fwstate(pmlmepriv));
		#if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND)
		#else
		pwrpriv->change_rfpwrstate = rf_off;
		#endif
		#ifdef CONFIG_AUTOSUSPEND
		if (padapter->registrypriv.usbss_enable)
		{
			if (pwrpriv->bHWPwrPindetect)
				pwrpriv->bkeepfwalive = true;

			if (padapter->net_closed == true)
				pwrpriv->ps_flag = true;

			#if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND)
			if (true==pwrpriv->bInternalAutoSuspend) {
				DBG_88E("<==%s .pwrpriv->bInternalAutoSuspend)(%x)\n",__func__,pwrpriv->bInternalAutoSuspend);
			} else {
				pwrpriv->change_rfpwrstate = rf_off;
				padapter->bCardDisableWOHSM = true;
				DBG_88E("<==%s .pwrpriv->bInternalAutoSuspend)(%x) call autosuspend_enter\n",__func__,pwrpriv->bInternalAutoSuspend);
				autosuspend_enter(padapter);
			}
			#else
			padapter->bCardDisableWOHSM = true;
			autosuspend_enter(padapter);
			#endif	/* if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND) */
		}
		else if (pwrpriv->bHWPwrPindetect)
		{
		}
		else
		#endif /* CONFIG_AUTOSUSPEND */
		{
			#if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND)
			pwrpriv->change_rfpwrstate = rf_off;
			#endif	/* defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND) */

			ips_enter(padapter);
		}
	}
exit:
	rtw_set_pwr_state_check_timer(&padapter->pwrctrlpriv);
	pwrpriv->ps_processing = false;
	return;
}

void pwr_state_check_handler(void *FunctionContext)
{
	_adapter *padapter = (_adapter *)FunctionContext;
	rtw_ps_cmd(padapter);
}

/*
 *
 * Parameters
 *	padapter
 *	pslv			power state level, only could be PS_STATE_S0 ~ PS_STATE_S4
 *
 */
void rtw_set_rpwm(PADAPTER padapter, u8 pslv)
{
	u8	rpwm;
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;

_func_enter_;

	pslv = PS_STATE(pslv);


	if (true == pwrpriv->btcoex_rfon)
	{
		if (pslv < PS_STATE_S4)
			pslv = PS_STATE_S3;
	}

	if ( (pwrpriv->rpwm == pslv)) {
		RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,
			("%s: Already set rpwm[0x%02X], new=0x%02X!\n", __func__, pwrpriv->rpwm, pslv));
		return;
	}

	if ((padapter->bSurpriseRemoved == true) ||
		(padapter->hw_init_completed == false))
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_err_,
				 ("%s: SurpriseRemoved(%d) hw_init_completed(%d)\n",
				  __func__, padapter->bSurpriseRemoved, padapter->hw_init_completed));

		pwrpriv->cpwm = PS_STATE_S4;

		return;
	}

	if (padapter->bDriverStopped == true)
	{
		RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_err_,
				 ("%s: change power state(0x%02X) when DriverStopped\n", __func__, pslv));

		if (pslv < PS_STATE_S2) {
			RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_err_,
					 ("%s: Reject to enter PS_STATE(0x%02X) lower than S2 when DriverStopped!!\n", __func__, pslv));
			return;
		}
	}

	rpwm = pslv | pwrpriv->tog;
	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("rtw_set_rpwm: rpwm=0x%02x cpwm=0x%02x\n", rpwm, pwrpriv->cpwm));

	pwrpriv->rpwm = pslv;

	rtw_hal_set_hwreg(padapter, HW_VAR_SET_RPWM, (u8 *)(&rpwm));

	pwrpriv->tog += 0x80;
	pwrpriv->cpwm = pslv;

_func_exit_;
}

u8 PS_RDY_CHECK(_adapter * padapter);
u8 PS_RDY_CHECK(_adapter * padapter)
{
	u32 curr_time, delta_time;
	struct pwrctrl_priv	*pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);


	curr_time = rtw_get_current_time();
	delta_time = curr_time -pwrpriv->DelayLPSLastTimeStamp;

	if (delta_time < LPS_DELAY_TIME)
	{
		return false;
	}

	if ((check_fwstate(pmlmepriv, _FW_LINKED) == false) ||
		(check_fwstate(pmlmepriv, _FW_UNDER_SURVEY) == true) ||
		(check_fwstate(pmlmepriv, WIFI_AP_STATE) == true) ||
		(check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == true) ||
		(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == true) )
		return false;
#ifdef CONFIG_WOWLAN
	if (true == pwrpriv->bInSuspend && pwrpriv->wowlan_mode)
		return true;
	else
		return false;
#else
	if (true == pwrpriv->bInSuspend )
		return false;
#endif
	if ( (padapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X) && (padapter->securitypriv.binstallGrpkey == false) )
	{
		DBG_88E("Group handshake still in progress !!!\n");
		return false;
	}
#ifdef CONFIG_IOCTL_CFG80211
	if (!rtw_cfg80211_pwr_mgmt(padapter))
		return false;
#endif

	return true;
}

void rtw_set_ps_mode(PADAPTER padapter, u8 ps_mode, u8 smart_ps, u8 bcn_ant_mode)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
#endif /* CONFIG_P2P */

_func_enter_;

	RT_TRACE(_module_rtl871x_pwrctrl_c_, _drv_notice_,
			 ("%s: PowerMode=%d Smart_PS=%d\n",
			  __func__, ps_mode, smart_ps));

	if (ps_mode > PM_Card_Disable) {
		RT_TRACE(_module_rtl871x_pwrctrl_c_,_drv_err_,("ps_mode:%d error\n", ps_mode));
		return;
	}

	if (pwrpriv->pwr_mode == ps_mode)
	{
		if (PS_MODE_ACTIVE == ps_mode) return;

		if ((pwrpriv->smart_ps == smart_ps) &&
			(pwrpriv->bcn_ant_mode == bcn_ant_mode))
		{
			return;
		}
	}

	/* if (pwrpriv->pwr_mode == PS_MODE_ACTIVE) */
	if (ps_mode == PS_MODE_ACTIVE) {
		if (pwdinfo->opp_ps == 0) {
			DBG_88E("rtw_set_ps_mode: Leave 802.11 power save\n");
			pwrpriv->pwr_mode = ps_mode;
			rtw_set_rpwm(padapter, PS_STATE_S4);
			rtw_hal_set_hwreg(padapter, HW_VAR_H2C_FW_PWRMODE, (u8 *)(&ps_mode));
			pwrpriv->bFwCurrentInPSMode = false;
		}
	}
	else
	{
		if (PS_RDY_CHECK(padapter)
#ifdef CONFIG_BT_COEXIST
			|| (BT_1Ant(padapter) == true)
#endif
			)
		{
			DBG_88E("%s: Enter 802.11 power save\n", __func__);
			pwrpriv->bFwCurrentInPSMode = true;
			pwrpriv->pwr_mode = ps_mode;
			pwrpriv->smart_ps = smart_ps;
			pwrpriv->bcn_ant_mode = bcn_ant_mode;
			rtw_hal_set_hwreg(padapter, HW_VAR_H2C_FW_PWRMODE, (u8 *)(&ps_mode));

			/*  Set CTWindow after LPS */
			if (pwdinfo->opp_ps == 1)
				p2p_ps_wk_cmd(padapter, P2P_PS_ENABLE, 0);

			rtw_set_rpwm(padapter, PS_STATE_S2);
		}
	}

_func_exit_;
}

/*
 * Return:
 *	0:	Leave OK
 *	-1:	Timeout
 *	-2:	Other error
 */
s32 LPS_RF_ON_check(PADAPTER padapter, u32 delay_ms)
{
	u32 start_time;
	u8 bAwake = false;
	s32 err = 0;


	start_time = rtw_get_current_time();
	while (1)
	{
		rtw_hal_get_hwreg(padapter, HW_VAR_FWLPS_RF_ON, &bAwake);
		if (true == bAwake)
			break;

		if (true == padapter->bSurpriseRemoved)
		{
			err = -2;
			DBG_88E("%s: device surprise removed!!\n", __func__);
			break;
		}

		if (rtw_get_passing_time_ms(start_time) > delay_ms)
		{
			err = -1;
			DBG_88E("%s: Wait for FW LPS leave more than %u ms!!!\n", __func__, delay_ms);
			break;
		}
		rtw_usleep_os(100);
	}

	return err;
}

/*  */
/* 	Description: */
/* 		Enter the leisure power save mode. */
/*  */
void LPS_Enter(PADAPTER padapter)
{
	struct pwrctrl_priv	*pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	_adapter *buddy = padapter->pbuddy_adapter;

_func_enter_;

	if (PS_RDY_CHECK(padapter) == false)
		return;

	if (true == pwrpriv->bLeisurePs)
	{
		/*  Idle for a while if we connect to AP a while ago. */
		if (pwrpriv->LpsIdleCount >= 2) /*   4 Sec */
		{
			if (pwrpriv->pwr_mode == PS_MODE_ACTIVE)
			{
				pwrpriv->bpower_saving = true;
				DBG_88E("%s smart_ps:%d\n", __func__, pwrpriv->smart_ps);
				/* For Tenda W311R IOT issue */
				rtw_set_ps_mode(padapter, pwrpriv->power_mgnt, pwrpriv->smart_ps, 0);
			}
		}
		else
			pwrpriv->LpsIdleCount++;
	}

/* 	DBG_88E("-LeisurePSEnter\n"); */

_func_exit_;
}

/*  */
/* 	Description: */
/* 		Leave the leisure power save mode. */
/*  */
void LPS_Leave(PADAPTER padapter)
{
#define LPS_LEAVE_TIMEOUT_MS 100

	struct pwrctrl_priv	*pwrpriv = &padapter->pwrctrlpriv;
	u32 start_time;
	u8 bAwake = false;

_func_enter_;

	if (pwrpriv->bLeisurePs)
	{
		if (pwrpriv->pwr_mode != PS_MODE_ACTIVE)
		{
			rtw_set_ps_mode(padapter, PS_MODE_ACTIVE, 0, 0);

			if (pwrpriv->pwr_mode == PS_MODE_ACTIVE)
				LPS_RF_ON_check(padapter, LPS_LEAVE_TIMEOUT_MS);
		}
	}

	pwrpriv->bpower_saving = false;

/* 	DBG_88E("-LeisurePSLeave\n"); */

_func_exit_;
}

/*  */
/*  Description: Leave all power save mode: LPS, FwLPS, IPS if needed. */
/*  Move code to function by tynli. 2010.03.26. */
/*  */
void LeaveAllPowerSaveMode(PADAPTER Adapter)
{
	struct mlme_priv	*pmlmepriv = &(Adapter->mlmepriv);
	u8	enqueue = 0;

_func_enter_;

	if (check_fwstate(pmlmepriv, _FW_LINKED) == true)
	{ /* connect */
		p2p_ps_wk_cmd(Adapter, P2P_PS_DISABLE, enqueue);

		rtw_lps_ctrl_wk_cmd(Adapter, LPS_CTRL_LEAVE, enqueue);
	} else {
		if (Adapter->pwrctrlpriv.rf_pwrstate== rf_off)
		{
			#ifdef CONFIG_AUTOSUSPEND
			if (Adapter->registrypriv.usbss_enable)
			{
				#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
				usb_disable_autosuspend(adapter_to_dvobj(Adapter)->pusbdev);
				#elif (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22) && LINUX_VERSION_CODE<=KERNEL_VERSION(2,6,34))
				adapter_to_dvobj(Adapter)->pusbdev->autosuspend_disabled = Adapter->bDisableAutosuspend;/* autosuspend disabled by the user */
				#endif
			}
			else
			#endif
			{
			}
		}
	}

_func_exit_;
}

#ifdef CONFIG_RESUME_IN_WORKQUEUE
static void resume_workitem_callback(struct work_struct *work);
#endif /* CONFIG_RESUME_IN_WORKQUEUE */

void rtw_init_pwrctrl_priv(PADAPTER padapter)
{
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;

_func_enter_;

	_init_pwrlock(&pwrctrlpriv->lock);
	pwrctrlpriv->rf_pwrstate = rf_on;
	pwrctrlpriv->ips_enter_cnts=0;
	pwrctrlpriv->ips_leave_cnts=0;
	pwrctrlpriv->bips_processing = false;

	pwrctrlpriv->ips_mode = padapter->registrypriv.ips_mode;
	pwrctrlpriv->ips_mode_req = padapter->registrypriv.ips_mode;

	pwrctrlpriv->pwr_state_check_interval = RTW_PWR_STATE_CHK_INTERVAL;
	pwrctrlpriv->pwr_state_check_cnts = 0;
	pwrctrlpriv->bInternalAutoSuspend = false;
	pwrctrlpriv->bInSuspend = false;
	pwrctrlpriv->bkeepfwalive = false;

#ifdef CONFIG_AUTOSUSPEND
#ifdef SUPPORT_HW_RFOFF_DETECTED
	pwrctrlpriv->pwr_state_check_interval = (pwrctrlpriv->bHWPwrPindetect) ?1000:2000;
#endif
#endif

	pwrctrlpriv->LpsIdleCount = 0;
	if (padapter->registrypriv.mp_mode == 1)
		pwrctrlpriv->power_mgnt =PS_MODE_ACTIVE ;
	else
		pwrctrlpriv->power_mgnt =padapter->registrypriv.power_mgnt;/*  PS_MODE_MIN; */
	pwrctrlpriv->bLeisurePs = (PS_MODE_ACTIVE != pwrctrlpriv->power_mgnt)?true:false;

	pwrctrlpriv->bFwCurrentInPSMode = false;

	pwrctrlpriv->rpwm = 0;
	pwrctrlpriv->cpwm = PS_STATE_S4;

	pwrctrlpriv->pwr_mode = PS_MODE_ACTIVE;
	pwrctrlpriv->smart_ps = padapter->registrypriv.smart_ps;
	pwrctrlpriv->bcn_ant_mode = 0;

	pwrctrlpriv->tog = 0x80;

	pwrctrlpriv->btcoex_rfon = false;

	_init_timer(&(pwrctrlpriv->pwr_state_check_timer), padapter->pnetdev, pwr_state_check_handler, (u8 *)padapter);

	#ifdef CONFIG_RESUME_IN_WORKQUEUE
	_init_workitem(&pwrctrlpriv->resume_work, resume_workitem_callback, NULL);
	pwrctrlpriv->rtw_workqueue = create_singlethread_workqueue("rtw_workqueue");
	#endif /* CONFIG_RESUME_IN_WORKQUEUE */

_func_exit_;
}

void rtw_free_pwrctrl_priv(PADAPTER adapter)
{
	struct pwrctrl_priv *pwrctrlpriv = &adapter->pwrctrlpriv;

_func_enter_;

	/* _rtw_memset((unsigned char *)pwrctrlpriv, 0, sizeof(struct pwrctrl_priv)); */


	#ifdef CONFIG_RESUME_IN_WORKQUEUE
	if (pwrctrlpriv->rtw_workqueue) {
		flush_workqueue(pwrctrlpriv->rtw_workqueue);
		destroy_workqueue(pwrctrlpriv->rtw_workqueue);
	}
	#endif

	_free_pwrlock(&pwrctrlpriv->lock);

_func_exit_;
}

#ifdef CONFIG_RESUME_IN_WORKQUEUE
extern int rtw_resume_process(_adapter *padapter);
static void resume_workitem_callback(struct work_struct *work)
{
	struct pwrctrl_priv *pwrpriv = container_of(work, struct pwrctrl_priv, resume_work);
	_adapter *adapter = container_of(pwrpriv, _adapter, pwrctrlpriv);

	DBG_88E("%s\n",__func__);

	rtw_resume_process(adapter);
}

void rtw_resume_in_workqueue(struct pwrctrl_priv *pwrpriv)
{
	/*  accquire system's suspend lock preventing from falliing asleep while resume in workqueue */
	rtw_lock_suspend();

	#if 1
	queue_work(pwrpriv->rtw_workqueue, &pwrpriv->resume_work);
	#else
	_set_workitem(&pwrpriv->resume_work);
	#endif
}
#endif /* CONFIG_RESUME_IN_WORKQUEUE */

u8 rtw_interface_ps_func(_adapter *padapter,HAL_INTF_PS_FUNC efunc_id,u8* val)
{
	u8 bResult = true;
	rtw_hal_intf_ps_func(padapter,efunc_id,val);

	return bResult;
}


inline void rtw_set_ips_deny(_adapter *padapter, u32 ms)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	pwrpriv->ips_deny_time = rtw_get_current_time() + rtw_ms_to_systime(ms);
}

/*
* rtw_pwr_wakeup - Wake the NIC up from: 1)IPS. 2)USB autosuspend
* @adapter: pointer to _adapter structure
* @ips_deffer_ms: the ms wiil prevent from falling into IPS after wakeup
* Return _SUCCESS or _FAIL
*/

int _rtw_pwr_wakeup(_adapter *padapter, u32 ips_deffer_ms, const char *caller)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	int ret = _SUCCESS;

	if (pwrpriv->ips_deny_time < rtw_get_current_time() + rtw_ms_to_systime(ips_deffer_ms))
		pwrpriv->ips_deny_time = rtw_get_current_time() + rtw_ms_to_systime(ips_deffer_ms);

{
	u32 start = rtw_get_current_time();
	if (pwrpriv->ps_processing) {
		DBG_88E("%s wait ps_processing...\n", __func__);
		while (pwrpriv->ps_processing && rtw_get_passing_time_ms(start) <= 3000)
			rtw_msleep_os(10);
		if (pwrpriv->ps_processing)
			DBG_88E("%s wait ps_processing timeout\n", __func__);
		else
			DBG_88E("%s wait ps_processing done\n", __func__);
	}
}

	/* System suspend is not allowed to wakeup */
	if ((pwrpriv->bInternalAutoSuspend == false) && (true == pwrpriv->bInSuspend )){
		ret = _FAIL;
		goto exit;
	}

	/* block??? */
	if ((pwrpriv->bInternalAutoSuspend == true)  && (padapter->net_closed == true)) {
		ret = _FAIL;
		goto exit;
	}

	/* I think this should be check in IPS, LPS, autosuspend functions... */
	if (check_fwstate(pmlmepriv, _FW_LINKED) == true)
	{
#if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND)
		if (true==pwrpriv->bInternalAutoSuspend){
			if (0==pwrpriv->autopm_cnt){
			#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))
				if (usb_autopm_get_interface(adapter_to_dvobj(padapter)->pusbintf) < 0)
				{
					DBG_88E( "can't get autopm:\n");
				}
			#elif (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,20))
				usb_autopm_disable(adapter_to_dvobj(padapter)->pusbintf);
			#else
				usb_autoresume_device(adapter_to_dvobj(padapter)->pusbdev, 1);
			#endif
			pwrpriv->autopm_cnt++;
			}
#endif	/* if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND) */
		ret = _SUCCESS;
		goto exit;
#if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND)
		}
#endif	/* if defined (CONFIG_BT_COEXIST)&& defined (CONFIG_AUTOSUSPEND) */
	}
	if (rf_off == pwrpriv->rf_pwrstate )
	{
#ifdef CONFIG_AUTOSUSPEND
		 if (pwrpriv->brfoffbyhw==true)
		{
			DBG_88E("hw still in rf_off state ...........\n");
			ret = _FAIL;
			goto exit;
		}
		else if (padapter->registrypriv.usbss_enable)
		{
			DBG_88E("%s call autoresume_enter....\n",__func__);
			if (_FAIL ==  autoresume_enter(padapter))
			{
				DBG_88E("======> autoresume fail.............\n");
				ret = _FAIL;
				goto exit;
			}
		}
		else
#endif
		{
			DBG_88E("%s call ips_leave....\n",__func__);
			if (_FAIL ==  ips_leave(padapter))
			{
				DBG_88E("======> ips_leave fail.............\n");
				ret = _FAIL;
				goto exit;
			}
		}
	}

	/* TODO: the following checking need to be merged... */
	if (padapter->bDriverStopped
		|| !padapter->bup
		|| !padapter->hw_init_completed
	){
		DBG_88E("%s: bDriverStopped=%d, bup=%d, hw_init_completed=%u\n"
			, caller
			, padapter->bDriverStopped
			, padapter->bup
			, padapter->hw_init_completed);
		ret= false;
		goto exit;
	}

exit:
	if (pwrpriv->ips_deny_time < rtw_get_current_time() + rtw_ms_to_systime(ips_deffer_ms))
		pwrpriv->ips_deny_time = rtw_get_current_time() + rtw_ms_to_systime(ips_deffer_ms);
	return ret;

}

int rtw_pm_set_lps(_adapter *padapter, u8 mode)
{
	int	ret = 0;
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;

	if ( mode < PS_MODE_NUM )
	{
		if (pwrctrlpriv->power_mgnt !=mode)
		{
			if (PS_MODE_ACTIVE == mode)
			{
				LeaveAllPowerSaveMode(padapter);
			}
			else
			{
				pwrctrlpriv->LpsIdleCount = 2;
			}
			pwrctrlpriv->power_mgnt = mode;
			pwrctrlpriv->bLeisurePs = (PS_MODE_ACTIVE != pwrctrlpriv->power_mgnt)?true:false;
		}
	}
	else
	{
		ret = -EINVAL;
	}

	return ret;
}

int rtw_pm_set_ips(_adapter *padapter, u8 mode)
{
	struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv;

	if ( mode == IPS_NORMAL || mode == IPS_LEVEL_2 ) {
		rtw_ips_mode_req(pwrctrlpriv, mode);
		DBG_88E("%s %s\n", __func__, mode == IPS_NORMAL?"IPS_NORMAL":"IPS_LEVEL_2");
		return 0;
	}
	else if (mode ==IPS_NONE){
		rtw_ips_mode_req(pwrctrlpriv, mode);
		DBG_88E("%s %s\n", __func__, "IPS_NONE");
		if ((padapter->bSurpriseRemoved ==0)&&(_FAIL == rtw_pwr_wakeup(padapter)) )
			return -EFAULT;
	}
	else {
		return -EINVAL;
	}
	return 0;
}
