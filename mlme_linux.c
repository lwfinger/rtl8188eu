// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */



#define _MLME_OSDEP_C_

#include <drv_types.h>
#include <rtw_mlme.h>

#ifdef RTK_DMP_PLATFORM
void Linkup_workitem_callback(struct work_struct *work)
{
	struct mlme_priv *pmlmepriv = container_of(work, struct mlme_priv, Linkup_workitem);
	_adapter *padapter = container_of(pmlmepriv, _adapter, mlmepriv);



#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 12))
	kobject_uevent(&padapter->pnetdev->dev.kobj, KOBJ_LINKUP);
#else
	kobject_hotplug(&padapter->pnetdev->class_dev.kobj, KOBJ_LINKUP);
#endif

}

void Linkdown_workitem_callback(struct work_struct *work)
{
	struct mlme_priv *pmlmepriv = container_of(work, struct mlme_priv, Linkdown_workitem);
	_adapter *padapter = container_of(pmlmepriv, _adapter, mlmepriv);



#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 12))
	kobject_uevent(&padapter->pnetdev->dev.kobj, KOBJ_LINKDOWN);
#else
	kobject_hotplug(&padapter->pnetdev->class_dev.kobj, KOBJ_LINKDOWN);
#endif

}
#endif


/*
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void sitesurvey_ctrl_handler(void *FunctionContext)
#else
void sitesurvey_ctrl_handler(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	_adapter *adapter = (_adapter *)FunctionContext;
#else
	_adapter *adapter = from_timer(adapter, t, sitesurveyctrl.sitesurvey_ctrl_timer);
#endif

	_sitesurvey_ctrl_handler(adapter);

	_set_timer(&adapter->mlmepriv.sitesurveyctrl.sitesurvey_ctrl_timer, 3000);
}
*/

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void rtw_join_timeout_handler(void *FunctionContext)
#else
void rtw_join_timeout_handler(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	_adapter *adapter = (_adapter *)FunctionContext;
#else
	_adapter *adapter = from_timer(adapter, t, mlmepriv.assoc_timer);
#endif

	_rtw_join_timeout_handler(adapter);
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void _rtw_scan_timeout_handler(void *FunctionContext)
#else
void _rtw_scan_timeout_handler(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	_adapter *adapter = (_adapter *)FunctionContext;
#else
	_adapter *adapter = from_timer(adapter, t, mlmepriv.scan_to_timer);
#endif

	rtw_scan_timeout_handler(adapter);
}


#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void _dynamic_check_timer_handler (void *FunctionContext)
#else
void _dynamic_check_timer_handler(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	struct dvobj_priv *pdvobj = (struct dvobj_priv *)FunctionContext;
#else
	struct dvobj_priv *pdvobj = from_timer(pdvobj, t, dynamic_chk_timer);
#endif
	_adapter *adapter = dvobj_get_primary_adapter(pdvobj);

#if (MP_DRIVER == 1)
	if (adapter->registrypriv.mp_mode == 1 && adapter->mppriv.mp_dm == 0) { /* for MP ODM dynamic Tx power tracking */
		/* RTW_INFO("_dynamic_check_timer_handler mp_dm =0 return\n"); */
		_set_timer(&pdvobj->dynamic_chk_timer, 2000);
		return;
	}
#endif

	rtw_dynamic_check_timer_handler(adapter);

	_set_timer(&pdvobj->dynamic_chk_timer, 2000);
}

#ifdef CONFIG_SET_SCAN_DENY_TIMER
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void _rtw_set_scan_deny_timer_hdl(void *FunctionContext)
#else
static void _rtw_set_scan_deny_timer_hdl(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	_adapter *adapter = (_adapter *)FunctionContext;
#else
	_adapter *adapter = from_timer(adapter, t, mlmepriv.set_scan_deny_timer);
#endif

	rtw_set_scan_deny_timer_hdl(adapter);
}
#endif

void rtw_init_mlme_timer(_adapter *padapter)
{
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	_init_timer(&pmlmepriv->assoc_timer, padapter->pnetdev, rtw_join_timeout_handler, padapter);
	/* _init_timer(&pmlmepriv->sitesurveyctrl.sitesurvey_ctrl_timer, padapter->pnetdev, sitesurvey_ctrl_handler, padapter); */
	_init_timer(&pmlmepriv->scan_to_timer, padapter->pnetdev, _rtw_scan_timeout_handler, padapter);

#ifdef CONFIG_DFS_MASTER
	_init_timer(&pmlmepriv->dfs_master_timer, padapter->pnetdev, rtw_dfs_master_timer_hdl, padapter);
#endif

#ifdef CONFIG_SET_SCAN_DENY_TIMER
	_init_timer(&pmlmepriv->set_scan_deny_timer, padapter->pnetdev, _rtw_set_scan_deny_timer_hdl, padapter);
#endif
#else
	timer_setup(&pmlmepriv->assoc_timer, rtw_join_timeout_handler, 0);
	/* timer_setup(&pmlmepriv->sitesurveyctrl.sitesurvey_ctrl_timer, sitesurvey_ctrl_handler, 0); */
	timer_setup(&pmlmepriv->scan_to_timer, _rtw_scan_timeout_handler, 0);

#ifdef CONFIG_DFS_MASTER
	timer_setup(&pmlmepriv->dfs_master_timer, rtw_dfs_master_timer_hdl, 0);
#endif

#ifdef CONFIG_SET_SCAN_DENY_TIMER
	timer_setup(&pmlmepriv->set_scan_deny_timer, _rtw_set_scan_deny_timer_hdl, 0);
#endif
#endif
#ifdef RTK_DMP_PLATFORM
	_init_workitem(&(pmlmepriv->Linkup_workitem), Linkup_workitem_callback, padapter);
	_init_workitem(&(pmlmepriv->Linkdown_workitem), Linkdown_workitem_callback, padapter);
#endif

}

void rtw_os_indicate_connect(_adapter *adapter)
{
	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);

#ifdef CONFIG_IOCTL_CFG80211
	if ((check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == true) ||
	    (check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == true))
		rtw_cfg80211_ibss_indicate_connect(adapter);
	else
		rtw_cfg80211_indicate_connect(adapter);
#endif /* CONFIG_IOCTL_CFG80211 */

	rtw_indicate_wx_assoc_event(adapter);
	netif_carrier_on(adapter->pnetdev);

	if (adapter->pid[2] != 0)
		rtw_signal_process(adapter->pid[2], SIGALRM);

#ifdef RTK_DMP_PLATFORM
	_set_workitem(&adapter->mlmepriv.Linkup_workitem);
#endif


}

extern void indicate_wx_scan_complete_event(_adapter *padapter);
void rtw_os_indicate_scan_done(_adapter *padapter, bool aborted)
{
#ifdef CONFIG_IOCTL_CFG80211
	rtw_cfg80211_indicate_scan_done(padapter, aborted);
#endif
	indicate_wx_scan_complete_event(padapter);
}

static RT_PMKID_LIST   backupPMKIDList[NUM_PMKID_CACHE];
void rtw_reset_securitypriv(_adapter *adapter)
{
	u8	backupPMKIDIndex = 0;
	u8	backupTKIPCountermeasure = 0x00;
	u32	backupTKIPcountermeasure_time = 0;
	/* add for CONFIG_IEEE80211W, none 11w also can use */
	unsigned long irqL;
	struct mlme_ext_priv	*pmlmeext = &adapter->mlmeextpriv;

	_enter_critical_bh(&adapter->security_key_mutex, &irqL);

	if (adapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X) { /* 802.1x */
		/* Added by Albert 2009/02/18 */
		/* We have to backup the PMK information for WiFi PMK Caching test item. */
		/*  */
		/* Backup the btkip_countermeasure information. */
		/* When the countermeasure is trigger, the driver have to disconnect with AP for 60 seconds. */

		memset(&backupPMKIDList[0], 0x00, sizeof(RT_PMKID_LIST) * NUM_PMKID_CACHE);

		memcpy(&backupPMKIDList[0], &adapter->securitypriv.PMKIDList[0], sizeof(RT_PMKID_LIST) * NUM_PMKID_CACHE);
		backupPMKIDIndex = adapter->securitypriv.PMKIDIndex;
		backupTKIPCountermeasure = adapter->securitypriv.btkip_countermeasure;
		backupTKIPcountermeasure_time = adapter->securitypriv.btkip_countermeasure_time;
#ifdef CONFIG_IEEE80211W
		/* reset RX BIP packet number */
		pmlmeext->mgnt_80211w_IPN_rx = 0;
#endif /* CONFIG_IEEE80211W */
		memset((unsigned char *)&adapter->securitypriv, 0, sizeof(struct security_priv));
		/* _init_timer(&(adapter->securitypriv.tkip_timer),adapter->pnetdev, rtw_use_tkipkey_handler, adapter); */

		/* Added by Albert 2009/02/18 */
		/* Restore the PMK information to securitypriv structure for the following connection. */
		memcpy(&adapter->securitypriv.PMKIDList[0], &backupPMKIDList[0], sizeof(RT_PMKID_LIST) * NUM_PMKID_CACHE);
		adapter->securitypriv.PMKIDIndex = backupPMKIDIndex;
		adapter->securitypriv.btkip_countermeasure = backupTKIPCountermeasure;
		adapter->securitypriv.btkip_countermeasure_time = backupTKIPcountermeasure_time;

		adapter->securitypriv.ndisauthtype = Ndis802_11AuthModeOpen;
		adapter->securitypriv.ndisencryptstatus = Ndis802_11WEPDisabled;

	} else { /* reset values in securitypriv */
		/* if(adapter->mlmepriv.fw_state & WIFI_STATION_STATE) */
		/* { */
		struct security_priv *psec_priv = &adapter->securitypriv;

		psec_priv->dot11AuthAlgrthm = dot11AuthAlgrthm_Open; /* open system */
		psec_priv->dot11PrivacyAlgrthm = _NO_PRIVACY_;
		psec_priv->dot11PrivacyKeyIndex = 0;

		psec_priv->dot118021XGrpPrivacy = _NO_PRIVACY_;
		psec_priv->dot118021XGrpKeyid = 1;

		psec_priv->ndisauthtype = Ndis802_11AuthModeOpen;
		psec_priv->ndisencryptstatus = Ndis802_11WEPDisabled;
		/* } */
	}
	/* add for CONFIG_IEEE80211W, none 11w also can use */
	_exit_critical_bh(&adapter->security_key_mutex, &irqL);

	RTW_INFO(FUNC_ADPT_FMT" - End to Disconnect\n", FUNC_ADPT_ARG(adapter));
}

void rtw_os_indicate_disconnect(_adapter *adapter,  u16 reason, u8 locally_generated)
{
	/* RT_PMKID_LIST   backupPMKIDList[NUM_PMKID_CACHE]; */


	netif_carrier_off(adapter->pnetdev); /* Do it first for tx broadcast pkt after disconnection issue! */

#ifdef CONFIG_IOCTL_CFG80211
	rtw_cfg80211_indicate_disconnect(adapter,  reason, locally_generated);
#endif /* CONFIG_IOCTL_CFG80211 */

	rtw_indicate_wx_disassoc_event(adapter);

#ifdef RTK_DMP_PLATFORM
	_set_workitem(&adapter->mlmepriv.Linkdown_workitem);
#endif
	/* modify for CONFIG_IEEE80211W, none 11w also can use the same command */
	rtw_reset_securitypriv_cmd(adapter);


}

void rtw_report_sec_ie(_adapter *adapter, u8 authmode, u8 *sec_ie)
{
	uint	len;
	u8	*buff, *p, i;
	union iwreq_data wrqu;



	buff = NULL;
	if (authmode == _WPA_IE_ID_) {

		buff = rtw_zmalloc(IW_CUSTOM_MAX);
		if (NULL == buff) {
			RTW_INFO(FUNC_ADPT_FMT ": alloc memory FAIL!!\n",
				 FUNC_ADPT_ARG(adapter));
			return;
		}
		p = buff;

		p += sprintf(p, "ASSOCINFO(ReqIEs=");

		len = sec_ie[1] + 2;
		len = (len < IW_CUSTOM_MAX) ? len : IW_CUSTOM_MAX;

		for (i = 0; i < len; i++)
			p += sprintf(p, "%02x", sec_ie[i]);

		p += sprintf(p, ")");

		memset(&wrqu, 0, sizeof(wrqu));

		wrqu.data.length = p - buff;

		wrqu.data.length = (wrqu.data.length < IW_CUSTOM_MAX) ? wrqu.data.length : IW_CUSTOM_MAX;

#ifndef CONFIG_IOCTL_CFG80211
		wireless_send_event(adapter->pnetdev, IWEVCUSTOM, &wrqu, buff);
#endif

		rtw_mfree(buff, IW_CUSTOM_MAX);
	}


}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void _survey_timer_hdl(void *FunctionContext)
#else
static void _survey_timer_hdl(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	_adapter *padapter = (_adapter *)FunctionContext;
#else
	_adapter *padapter = from_timer(padapter, t, mlmeextpriv.survey_timer);
#endif

	survey_timer_hdl(padapter);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void _link_timer_hdl(void *FunctionContext)
#else
static void _link_timer_hdl(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	_adapter *padapter = (_adapter *)FunctionContext;
#else
	_adapter *padapter = from_timer(padapter, t, mlmeextpriv.link_timer);
#endif

	link_timer_hdl(padapter);
}

#ifdef CONFIG_RTW_80211R
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void _ft_link_timer_hdl(void *FunctionContext)
#else
void _ft_link_timer_hdl(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	_adapter *padapter = (_adapter *)FunctionContext;
#else
	_adapter *padapter = from_timer(ipadapter, t, ft_link_timer);
#endif

	ft_link_timer_hdl(padapter);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void _ft_roam_timer_hdl(void *FunctionContext)
#else
void _ft_roam_timer_hdl(struct timer_list *t);
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	_adapter *padapter = (_adapter *)FunctionContext;
#else
	_adapter *padapter = from_timer(adapter, t, ft_roam_timer);
#endif

	ft_roam_timer_hdl(padapter);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
static void _addba_timer_hdl(void *FunctionContext)
#else
static void _addba_timer_hdl(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	struct sta_info *psta = (struct sta_info *)FunctionContext;
#else
	struct sta_info *psta = from_timer(psta, t, addba_retry_timer);
#endif

	addba_timer_hdl(psta);
}

#ifdef CONFIG_IEEE80211W


#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void _sa_query_timer_hdl(void *FunctionContext)
#else
void _sa_query_timer_hdl(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	struct sta_info *psta = (struct sta_info *)FunctionContext;
#else
	struct sta_info *psta = from_timer(psta, t, dot11w_expire_timer);
#endif

	sa_query_timer_hdl(psta);
}

void init_dot11w_expire_timer(_adapter *padapter, struct sta_info *psta)
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	_init_timer(&psta->dot11w_expire_timer, padapter->pnetdev, _sa_query_timer_hdl, psta);
#else
	timer_setip(&psta->dot11w_expire_timer, _sa_query_timer_hdl, 0);
#endif
}

#endif /* CONFIG_IEEE80211W */

void init_addba_retry_timer(_adapter *padapter, struct sta_info *psta)
{

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	_init_timer(&psta->addba_retry_timer, padapter->pnetdev, _addba_timer_hdl, psta);
#else
	timer_setup(&psta->addba_retry_timer, _addba_timer_hdl, 0);
#endif
}

/*
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void _reauth_timer_hdl(void *FunctionContext)
#else
void _reauth_timer_hdl(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	_adapter *padapter = (_adapter *)FunctionContext;
#else
	_adapter *padapter = from_timer(adapter, t, reauth_timer);
#endif

	reauth_timer_hdl(padapter);
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void _reassoc_timer_hdl(void *FunctionContext)
#else
void _reassoc_timer_hdl(struct timer_list *t)
#endif
{
#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	_adapter *padapter = (_adapter *)FunctionContext;
#else
	_adapter *padapter = from_timer(adapter, t, reassoc_timer);
#endif

`	reassoc_timer_hdl(padapter);
}
*/

void init_mlme_ext_timer(_adapter *padapter)
{
	struct	mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
	_init_timer(&pmlmeext->survey_timer, padapter->pnetdev, _survey_timer_hdl, padapter);
	_init_timer(&pmlmeext->link_timer, padapter->pnetdev, _link_timer_hdl, padapter);
#ifdef CONFIG_RTW_80211R
	_init_timer(&pmlmeext->ft_link_timer, padapter->pnetdev, _ft_link_timer_hdl, padapter);
	_init_timer(&pmlmeext->ft_roam_timer, padapter->pnetdev, _ft_roam_timer_hdl, padapter);
#endif

	/* _init_timer(&pmlmeext->ADDBA_timer, padapter->pnetdev, _addba_timer_hdl, padapter); */

	/* _init_timer(&pmlmeext->reauth_timer, padapter->pnetdev, _reauth_timer_hdl, padapter); */
	/* _init_timer(&pmlmeext->reassoc_timer, padapter->pnetdev, _reassoc_timer_hdl, padapter); */
#else
	timer_setup(&pmlmeext->survey_timer, _survey_timer_hdl, 0);
	timer_setup(&pmlmeext->link_timer, _link_timer_hdl, 0);
#ifdef CONFIG_RTW_80211R
	timer_setup(&pmlmeext->ft_link_timer, _ft_link_timer_hdl, 0);
	timer_setup(&pmlmeext->ft_roam_timer, _ft_roam_timer_hdl, 0);
#endif

	/* timer_setup(&pmlmeext->ADDBA_timer, _addba_timer_hdl, 0); */

	/* timer_setup(&pmlmeext->reauth_timer, _reauth_timer_hdl, 0); */
	/* timer_setup(&pmlmeext->reassoc_timer, _reassoc_timer_hdl, 0); */
#endif
}

#ifdef CONFIG_AP_MODE

void rtw_indicate_sta_assoc_event(_adapter *padapter, struct sta_info *psta)
{
	union iwreq_data wrqu;
	struct sta_priv *pstapriv = &padapter->stapriv;

	if (psta == NULL)
		return;

	if (psta->aid > NUM_STA)
		return;

	if (pstapriv->sta_aid[psta->aid - 1] != psta)
		return;


	wrqu.addr.sa_family = ARPHRD_ETHER;

	memcpy(wrqu.addr.sa_data, psta->hwaddr, ETH_ALEN);

	RTW_INFO("+rtw_indicate_sta_assoc_event\n");

#ifndef CONFIG_IOCTL_CFG80211
	wireless_send_event(padapter->pnetdev, IWEVREGISTERED, &wrqu, NULL);
#endif

}

void rtw_indicate_sta_disassoc_event(_adapter *padapter, struct sta_info *psta)
{
	union iwreq_data wrqu;
	struct sta_priv *pstapriv = &padapter->stapriv;

	if (psta == NULL)
		return;

	if (psta->aid > NUM_STA)
		return;

	if (pstapriv->sta_aid[psta->aid - 1] != psta)
		return;


	wrqu.addr.sa_family = ARPHRD_ETHER;

	memcpy(wrqu.addr.sa_data, psta->hwaddr, ETH_ALEN);

	RTW_INFO("+rtw_indicate_sta_disassoc_event\n");

#ifndef CONFIG_IOCTL_CFG80211
	wireless_send_event(padapter->pnetdev, IWEVEXPIRED, &wrqu, NULL);
#endif

}


#ifdef CONFIG_HOSTAPD_MLME

static int mgnt_xmit_entry(struct sk_buff *skb, struct net_device *pnetdev)
{
	struct hostapd_priv *phostapdpriv = rtw_netdev_priv(pnetdev);
	_adapter *padapter = (_adapter *)phostapdpriv->padapter;

	/* RTW_INFO("%s\n", __func__); */

	return rtw_hal_hostap_mgnt_xmit_entry(padapter, skb);
}

static int mgnt_netdev_open(struct net_device *pnetdev)
{
	struct hostapd_priv *phostapdpriv = rtw_netdev_priv(pnetdev);

	RTW_INFO("mgnt_netdev_open: MAC Address:" MAC_FMT "\n", MAC_ARG(pnetdev->dev_addr));


	init_usb_anchor(&phostapdpriv->anchored);

	rtw_netif_wake_queue(pnetdev);

	netif_carrier_on(pnetdev);

	/* rtw_write16(phostapdpriv->padapter, 0x0116, 0x0100); */ /* only excluding beacon */

	return 0;
}
static int mgnt_netdev_close(struct net_device *pnetdev)
{
	struct hostapd_priv *phostapdpriv = rtw_netdev_priv(pnetdev);

	RTW_INFO("%s\n", __func__);

	usb_kill_anchored_urbs(&phostapdpriv->anchored);

	netif_carrier_off(pnetdev);

	rtw_netif_stop_queue(pnetdev);

	/* rtw_write16(phostapdpriv->padapter, 0x0116, 0x3f3f); */

	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))
static const struct net_device_ops rtl871x_mgnt_netdev_ops = {
	.ndo_open = mgnt_netdev_open,
	.ndo_stop = mgnt_netdev_close,
	.ndo_start_xmit = mgnt_xmit_entry,
};
#endif

int hostapd_mode_init(_adapter *padapter)
{
	unsigned char mac[ETH_ALEN];
	struct hostapd_priv *phostapdpriv;
	struct net_device *pnetdev;

	pnetdev = rtw_alloc_etherdev(sizeof(struct hostapd_priv));
	if (!pnetdev)
		return -ENOMEM;

	/* SET_MODULE_OWNER(pnetdev); */
	ether_setup(pnetdev);

	/* pnetdev->type = ARPHRD_IEEE80211; */

	phostapdpriv = rtw_netdev_priv(pnetdev);
	phostapdpriv->pmgnt_netdev = pnetdev;
	phostapdpriv->padapter = padapter;
	padapter->phostapdpriv = phostapdpriv;

	/* pnetdev->init = NULL; */

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 29))

	RTW_INFO("register rtl871x_mgnt_netdev_ops to netdev_ops\n");

	pnetdev->netdev_ops = &rtl871x_mgnt_netdev_ops;

#else

	pnetdev->open = mgnt_netdev_open;

	pnetdev->stop = mgnt_netdev_close;

	pnetdev->hard_start_xmit = mgnt_xmit_entry;

	/* pnetdev->set_mac_address = r871x_net_set_mac_address; */

	/* pnetdev->get_stats = r871x_net_get_stats; */

	/* pnetdev->do_ioctl = r871x_mp_ioctl; */

#endif

	pnetdev->watchdog_timeo = HZ; /* 1 second timeout */

	/* pnetdev->wireless_handlers = NULL; */

#ifdef CONFIG_TCP_CSUM_OFFLOAD_TX
	pnetdev->features |= NETIF_F_IP_CSUM;
#endif



	if (dev_alloc_name(pnetdev, "mgnt.wlan%d") < 0)
		RTW_INFO("hostapd_mode_init(): dev_alloc_name, fail!\n");


	/* SET_NETDEV_DEV(pnetdev, pintfpriv->udev); */


	mac[0] = 0x00;
	mac[1] = 0xe0;
	mac[2] = 0x4c;
	mac[3] = 0x87;
	mac[4] = 0x11;
	mac[5] = 0x12;

	memcpy(pnetdev->dev_addr, mac, ETH_ALEN);


	netif_carrier_off(pnetdev);


	/* Tell the network stack we exist */
	if (register_netdev(pnetdev) != 0) {
		RTW_INFO("hostapd_mode_init(): register_netdev fail!\n");

		if (pnetdev)
			rtw_free_netdev(pnetdev);
	}

	return 0;

}

void hostapd_mode_unload(_adapter *padapter)
{
	struct hostapd_priv *phostapdpriv = padapter->phostapdpriv;
	struct net_device *pnetdev = phostapdpriv->pmgnt_netdev;

	unregister_netdev(pnetdev);
	rtw_free_netdev(pnetdev);

}

#endif
#endif
