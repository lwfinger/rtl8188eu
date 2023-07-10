// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#define _RTW_IOCTL_SET_C_

#include <drv_types.h>
#include <hal_data.h>
#include <rtw_mlme.h>

#define IS_MAC_ADDRESS_BROADCAST(addr) \
	(\
	 ((addr[0] == 0xff) && (addr[1] == 0xff) && \
	  (addr[2] == 0xff) && (addr[3] == 0xff) && \
	  (addr[4] == 0xff) && (addr[5] == 0xff)) ? true : false \
	)

u8 rtw_validate_bssid(u8 *bssid)
{
	u8 ret = true;

	if (is_zero_mac_addr(bssid)
	    || is_broadcast_mac_addr(bssid)
	    || is_multicast_mac_addr(bssid)
	   )
		ret = false;

	return ret;
}

u8 rtw_validate_ssid(NDIS_802_11_SSID *ssid)
{
	u8	 i;
	u8	ret = true;

	if (ssid->SsidLength > 32) {
		ret = false;
		goto exit;
	}

#ifdef CONFIG_VALIDATE_SSID
	for (i = 0; i < ssid->SsidLength; i++) {
		/* wifi, printable ascii code must be supported */
		if (!((ssid->Ssid[i] >= 0x20) && (ssid->Ssid[i] <= 0x7e))) {
			ret = false;
			break;
		}
	}
#endif /* CONFIG_VALIDATE_SSID */

exit:

	return ret;
}

u8 rtw_do_join(_adapter *padapter);
u8 rtw_do_join(_adapter *padapter)
{
	unsigned long	irqL;
	_list	*plist, *phead;
	u8 *pibss = NULL;
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	_queue	*queue	= &(pmlmepriv->scanned_queue);
	u8 ret = _SUCCESS;

	_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);
	phead = get_list_head(queue);
	plist = get_next(phead);

	pmlmepriv->cur_network.join_res = -2;

	set_fwstate(pmlmepriv, _FW_UNDER_LINKING);

	pmlmepriv->pscanned = plist;

	pmlmepriv->to_join = true;

	if (_rtw_queue_empty(queue) ) {
		_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);
		_clr_fwstate_(pmlmepriv, _FW_UNDER_LINKING);

		/* when set_ssid/set_bssid for rtw_do_join(), but scanning queue is empty */
		/* we try to issue sitesurvey firstly	 */

		if (pmlmepriv->LinkDetectInfo.bBusyTraffic == false
		    || rtw_to_roam(padapter) > 0
		   ) {
			/* submit site_survey_cmd */
			ret = rtw_sitesurvey_cmd(padapter, &pmlmepriv->assoc_ssid, 1, NULL, 0);
			if (_SUCCESS != ret) {
				pmlmepriv->to_join = false;
			}
		} else {
			pmlmepriv->to_join = false;
			ret = _FAIL;
		}

		goto exit;
	} else {
		int select_ret;
		_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);
		select_ret = rtw_select_and_join_from_scanned_queue(pmlmepriv);
		if (select_ret == _SUCCESS) {
			pmlmepriv->to_join = false;
			_set_timer(&pmlmepriv->assoc_timer, MAX_JOIN_TIMEOUT);
		} else {
			if (check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) ) {
				/* submit createbss_cmd to change to a ADHOC_MASTER */

				/* pmlmepriv->lock has been acquired by caller... */
				WLAN_BSSID_EX    *pdev_network = &(padapter->registrypriv.dev_network);

				/*pmlmepriv->fw_state = WIFI_ADHOC_MASTER_STATE;*/
				init_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE);

				pibss = padapter->registrypriv.dev_network.MacAddress;

				memset(&pdev_network->Ssid, 0, sizeof(NDIS_802_11_SSID));
				memcpy(&pdev_network->Ssid, &pmlmepriv->assoc_ssid, sizeof(NDIS_802_11_SSID));

				rtw_update_registrypriv_dev_network(padapter);

				rtw_generate_random_ibss(pibss);

				if (rtw_create_ibss_cmd(padapter, 0) != _SUCCESS) {
					ret =  false;
					goto exit;
				}

				pmlmepriv->to_join = false;

			} else {
				/* can't associate ; reset under-linking			 */
				_clr_fwstate_(pmlmepriv, _FW_UNDER_LINKING);

				/* when set_ssid/set_bssid for rtw_do_join(), but there are no desired bss in scanning queue */
				/* we try to issue sitesurvey firstly			 */
				if (pmlmepriv->LinkDetectInfo.bBusyTraffic == false
				    || rtw_to_roam(padapter) > 0
				   ) {
					/* RTW_INFO("rtw_do_join() when   no desired bss in scanning queue\n"); */
					ret = rtw_sitesurvey_cmd(padapter, &pmlmepriv->assoc_ssid, 1, NULL, 0);
					if (_SUCCESS != ret) {
						pmlmepriv->to_join = false;
					}
				} else {
					ret = _FAIL;
					pmlmepriv->to_join = false;
				}
			}

		}

	}

exit:

	return ret;
}

u8 rtw_set_802_11_bssid(_adapter *padapter, u8 *bssid)
{
	unsigned long irqL;
	u8 status = _SUCCESS;

	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	RTW_PRINT("set bssid:%pM\n", bssid);

	if ((bssid[0] == 0x00 && bssid[1] == 0x00 && bssid[2] == 0x00 && bssid[3] == 0x00 && bssid[4] == 0x00 && bssid[5] == 0x00) ||
	    (bssid[0] == 0xFF && bssid[1] == 0xFF && bssid[2] == 0xFF && bssid[3] == 0xFF && bssid[4] == 0xFF && bssid[5] == 0xFF)) {
		status = _FAIL;
		goto exit;
	}

	_enter_critical_bh(&pmlmepriv->lock, &irqL);

	RTW_INFO("Set BSSID under fw_state=0x%08x\n", get_fwstate(pmlmepriv));
	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY) )
		goto handle_tkip_countermeasure;
	else if (check_fwstate(pmlmepriv, _FW_UNDER_LINKING) )
		goto release_mlme_lock;

	if (check_fwstate(pmlmepriv, _FW_LINKED | WIFI_ADHOC_MASTER_STATE) ) {

		if (!memcmp(&pmlmepriv->cur_network.network.MacAddress, bssid, ETH_ALEN) ) {
			if (check_fwstate(pmlmepriv, WIFI_STATION_STATE) == false)
				goto release_mlme_lock;/* it means driver is in WIFI_ADHOC_MASTER_STATE, we needn't create bss again. */
		} else {

			rtw_disassoc_cmd(padapter, 0, true);

			if (check_fwstate(pmlmepriv, _FW_LINKED) )
				rtw_indicate_disconnect(padapter, 0, false);

			rtw_free_assoc_resources(padapter, 1);

			if ((check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) )) {
				_clr_fwstate_(pmlmepriv, WIFI_ADHOC_MASTER_STATE);
//				set_fwstate(pmlmepriv, WIFI_ADHOC_STATE);
			}
		}
	}

handle_tkip_countermeasure:
	if (rtw_handle_tkip_countermeasure(padapter, __func__) == _FAIL) {
		status = _FAIL;
		goto release_mlme_lock;
	}

	memset(&pmlmepriv->assoc_ssid, 0, sizeof(NDIS_802_11_SSID));
	memcpy(&pmlmepriv->assoc_bssid, bssid, ETH_ALEN);
	pmlmepriv->assoc_by_bssid = true;

	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY) )
		pmlmepriv->to_join = true;
	else
		status = rtw_do_join(padapter);

release_mlme_lock:
	_exit_critical_bh(&pmlmepriv->lock, &irqL);

exit:

	return status;
}

u8 rtw_set_802_11_ssid(_adapter *padapter, NDIS_802_11_SSID *ssid)
{
	unsigned long irqL;
	u8 status = _SUCCESS;
	u32 cur_time = 0;

	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct wlan_network *pnetwork = &pmlmepriv->cur_network;

	RTW_PRINT("set ssid [%s] fw_state=0x%08x\n",
		  ssid->Ssid, get_fwstate(pmlmepriv));

	if (!rtw_is_hw_init_completed(padapter)) {
		status = _FAIL;
		goto exit;
	}

	_enter_critical_bh(&pmlmepriv->lock, &irqL);

	RTW_INFO("Set SSID under fw_state=0x%08x\n", get_fwstate(pmlmepriv));
	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY) )
		goto handle_tkip_countermeasure;
	else if (check_fwstate(pmlmepriv, _FW_UNDER_LINKING) )
		goto release_mlme_lock;

	if (check_fwstate(pmlmepriv, _FW_LINKED | WIFI_ADHOC_MASTER_STATE) ) {

		if ((pmlmepriv->assoc_ssid.SsidLength == ssid->SsidLength) &&
		    (!memcmp(&pmlmepriv->assoc_ssid.Ssid, ssid->Ssid, ssid->SsidLength) )) {
			if ((check_fwstate(pmlmepriv, WIFI_STATION_STATE) == false)) {

				if (rtw_is_same_ibss(padapter, pnetwork) == false) {
					/* if in WIFI_ADHOC_MASTER_STATE | WIFI_ADHOC_STATE, create bss or rejoin again */
					rtw_disassoc_cmd(padapter, 0, true);

					if (check_fwstate(pmlmepriv, _FW_LINKED) )
						rtw_indicate_disconnect(padapter, 0, false);

					rtw_free_assoc_resources(padapter, 1);

					if (check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) ) {
						_clr_fwstate_(pmlmepriv, WIFI_ADHOC_MASTER_STATE);
//						set_fwstate(pmlmepriv, WIFI_ADHOC_STATE);
					}
				} else {
					goto release_mlme_lock;/* it means driver is in WIFI_ADHOC_MASTER_STATE, we needn't create bss again. */
				}
			}
#ifdef CONFIG_LPS
			else
				rtw_lps_ctrl_wk_cmd(padapter, LPS_CTRL_JOINBSS, 1);
#endif
		} else {

			rtw_disassoc_cmd(padapter, 0, true);

			if (check_fwstate(pmlmepriv, _FW_LINKED) )
				rtw_indicate_disconnect(padapter, 0, false);

			rtw_free_assoc_resources(padapter, 1);

			if (check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) ) {
				_clr_fwstate_(pmlmepriv, WIFI_ADHOC_MASTER_STATE);
//				set_fwstate(pmlmepriv, WIFI_ADHOC_STATE);
			}
		}
	}

handle_tkip_countermeasure:
	if (rtw_handle_tkip_countermeasure(padapter, __func__) == _FAIL) {
		status = _FAIL;
		goto release_mlme_lock;
	}

	if (rtw_validate_ssid(ssid) == false) {
		status = _FAIL;
		goto release_mlme_lock;
	}

	memcpy(&pmlmepriv->assoc_ssid, ssid, sizeof(NDIS_802_11_SSID));
	pmlmepriv->assoc_by_bssid = false;

	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY) )
		pmlmepriv->to_join = true;
	else
		status = rtw_do_join(padapter);

release_mlme_lock:
	_exit_critical_bh(&pmlmepriv->lock, &irqL);

exit:

	return status;

}

u8 rtw_set_802_11_connect(_adapter *padapter, u8 *bssid, NDIS_802_11_SSID *ssid)
{
	unsigned long irqL;
	u8 status = _SUCCESS;
	u32 cur_time = 0;
	bool bssid_valid = true;
	bool ssid_valid = true;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	if (!ssid || rtw_validate_ssid(ssid) == false)
		ssid_valid = false;

	if (!bssid || rtw_validate_bssid(bssid) == false)
		bssid_valid = false;

	if (ssid_valid == false && bssid_valid == false) {
		RTW_INFO(FUNC_ADPT_FMT" ssid:%p, ssid_valid:%d, bssid:%p, bssid_valid:%d\n",
			FUNC_ADPT_ARG(padapter), ssid, ssid_valid, bssid, bssid_valid);
		status = _FAIL;
		goto exit;
	}

	if (!rtw_is_hw_init_completed(padapter)) {
		status = _FAIL;
		goto exit;
	}

	_enter_critical_bh(&pmlmepriv->lock, &irqL);

	RTW_PRINT(FUNC_ADPT_FMT"  fw_state=0x%08x\n",
		  FUNC_ADPT_ARG(padapter), get_fwstate(pmlmepriv));

	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY) )
		goto handle_tkip_countermeasure;
	else if (check_fwstate(pmlmepriv, _FW_UNDER_LINKING) )
		goto release_mlme_lock;

handle_tkip_countermeasure:
	if (rtw_handle_tkip_countermeasure(padapter, __func__) == _FAIL) {
		status = _FAIL;
		goto release_mlme_lock;
	}

	if (ssid && ssid_valid)
		memcpy(&pmlmepriv->assoc_ssid, ssid, sizeof(NDIS_802_11_SSID));
	else
		memset(&pmlmepriv->assoc_ssid, 0, sizeof(NDIS_802_11_SSID));

	if (bssid && bssid_valid) {
		memcpy(&pmlmepriv->assoc_bssid, bssid, ETH_ALEN);
		pmlmepriv->assoc_by_bssid = true;
	} else
		pmlmepriv->assoc_by_bssid = false;

	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY) )
		pmlmepriv->to_join = true;
	else
		status = rtw_do_join(padapter);

release_mlme_lock:
	_exit_critical_bh(&pmlmepriv->lock, &irqL);

exit:

	return status;
}

u8 rtw_set_802_11_infrastructure_mode(_adapter *padapter,
			      NDIS_802_11_NETWORK_INFRASTRUCTURE networktype)
{
	unsigned long irqL;
	struct	mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct	wlan_network	*cur_network = &pmlmepriv->cur_network;
	NDIS_802_11_NETWORK_INFRASTRUCTURE *pold_state = &(cur_network->network.InfrastructureMode);
	u8 ap2sta_mode = false;

	if (*pold_state != networktype) {
		/* RTW_INFO("change mode, old_mode=%d, new_mode=%d, fw_state=0x%x\n", *pold_state, networktype, get_fwstate(pmlmepriv)); */

		if (*pold_state == Ndis802_11APMode) {
			/* change to other mode from Ndis802_11APMode			 */
			cur_network->join_res = -1;
			ap2sta_mode = true;
#ifdef CONFIG_NATIVEAP_MLME
			stop_ap_mode(padapter);
#endif
		}

		_enter_critical_bh(&pmlmepriv->lock, &irqL);

		if ((check_fwstate(pmlmepriv, _FW_LINKED) ) || (*pold_state == Ndis802_11IBSS))
			rtw_disassoc_cmd(padapter, 0, true);

		if ((check_fwstate(pmlmepriv, _FW_LINKED) ) ||
		    (check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) ))
			rtw_free_assoc_resources(padapter, 1);

		if ((*pold_state == Ndis802_11Infrastructure) || (*pold_state == Ndis802_11IBSS)) {
			if (check_fwstate(pmlmepriv, _FW_LINKED) ) {
				rtw_indicate_disconnect(padapter, 0, false); /*will clr Linked_state; before this function, we must have checked whether issue dis-assoc_cmd or not*/
			}
		}

		*pold_state = networktype;

		_clr_fwstate_(pmlmepriv, ~WIFI_NULL_STATE);

		switch (networktype) {
		case Ndis802_11IBSS:
//			set_fwstate(pmlmepriv, WIFI_ADHOC_STATE);
			break;

		case Ndis802_11Infrastructure:
			set_fwstate(pmlmepriv, WIFI_STATION_STATE);

			if (ap2sta_mode)
				rtw_init_bcmc_stainfo(padapter);
			break;

		case Ndis802_11APMode:
			set_fwstate(pmlmepriv, WIFI_AP_STATE);
#ifdef CONFIG_NATIVEAP_MLME
			start_ap_mode(padapter);
			/* rtw_indicate_connect(padapter); */
#endif

			break;

		case Ndis802_11AutoUnknown:
		case Ndis802_11InfrastructureMax:
			break;
		case Ndis802_11Monitor:
			set_fwstate(pmlmepriv, WIFI_MONITOR_STATE);
			break;
		}

		/* SecClearAllKeys(adapter); */

		_exit_critical_bh(&pmlmepriv->lock, &irqL);
	}

	return true;
}

u8 rtw_set_802_11_disassociate(_adapter *padapter)
{
	unsigned long irqL;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	_enter_critical_bh(&pmlmepriv->lock, &irqL);

	if (check_fwstate(pmlmepriv, _FW_LINKED) ) {

		rtw_disassoc_cmd(padapter, 0, true);
		rtw_indicate_disconnect(padapter, 0, false);
		/* modify for CONFIG_IEEE80211W, none 11w can use it */
		rtw_free_assoc_resources_cmd(padapter);
		if (_FAIL == rtw_pwr_wakeup(padapter))
			RTW_INFO("%s(): rtw_pwr_wakeup fail !!!\n", __func__);
	}

	_exit_critical_bh(&pmlmepriv->lock, &irqL);

	return true;
}

u8 rtw_set_802_11_bssid_list_scan(_adapter *padapter, NDIS_802_11_SSID *pssid, int ssid_max_num, struct rtw_ieee80211_channel *ch, int ch_num)
{
	unsigned long	irqL;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	u8	res = true;

	_enter_critical_bh(&pmlmepriv->lock, &irqL);
	res = rtw_sitesurvey_cmd(padapter, pssid, ssid_max_num, ch, ch_num);
	_exit_critical_bh(&pmlmepriv->lock, &irqL);

	return res;
}

u8 rtw_set_802_11_authentication_mode(_adapter *padapter, NDIS_802_11_AUTHENTICATION_MODE authmode)
{
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	int res;
	u8 ret;

	psecuritypriv->ndisauthtype = authmode;

	if (psecuritypriv->ndisauthtype > 3)
		psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_8021X;

#ifdef CONFIG_WAPI_SUPPORT
	if (psecuritypriv->ndisauthtype == 6)
		psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_WAPI;
#endif

	res = rtw_set_auth(padapter, psecuritypriv);

	if (res == _SUCCESS)
		ret = true;
	else
		ret = false;

	return ret;
}

u8 rtw_set_802_11_add_wep(_adapter *padapter, NDIS_802_11_WEP *wep)
{

	u8		bdefaultkey;
	u8		btransmitkey;
	sint		keyid, res;
	struct security_priv *psecuritypriv = &(padapter->securitypriv);
	u8		ret = _SUCCESS;

	bdefaultkey = (wep->KeyIndex & 0x40000000) > 0 ? false : true; /* for ??? */
	btransmitkey = (wep->KeyIndex & 0x80000000) > 0 ? true  : false;	/* for ??? */
	keyid = wep->KeyIndex & 0x3fffffff;

	if (keyid >= 4) {
		ret = false;
		goto exit;
	}

	switch (wep->KeyLength) {
	case 5:
		psecuritypriv->dot11PrivacyAlgrthm = _WEP40_;
		break;
	case 13:
		psecuritypriv->dot11PrivacyAlgrthm = _WEP104_;
		break;
	default:
		psecuritypriv->dot11PrivacyAlgrthm = _NO_PRIVACY_;
		break;
	}

	memcpy(&(psecuritypriv->dot11DefKey[keyid].skey[0]), &(wep->KeyMaterial), wep->KeyLength);

	psecuritypriv->dot11DefKeylen[keyid] = wep->KeyLength;

	psecuritypriv->dot11PrivacyKeyIndex = keyid;

	res = rtw_set_key(padapter, psecuritypriv, keyid, 1, true);

	if (res == _FAIL)
		ret = false;
exit:

	return ret;

}

u8 rtw_set_802_11_remove_wep(_adapter *padapter, u32 keyindex)
{

	u8 ret = _SUCCESS;

	if (keyindex >= 0x80000000 || padapter == NULL) {

		ret = false;
		goto exit;

	} else {
		int res;
		struct security_priv *psecuritypriv = &(padapter->securitypriv);
		if (keyindex < 4) {

			memset(&psecuritypriv->dot11DefKey[keyindex], 0, 16);

			res = rtw_set_key(padapter, psecuritypriv, keyindex, 0, true);

			psecuritypriv->dot11DefKeylen[keyindex] = 0;

			if (res == _FAIL)
				ret = _FAIL;

		} else
			ret = _FAIL;

	}

exit:

	return ret;

}

u8 rtw_set_802_11_add_key(_adapter *padapter, NDIS_802_11_KEY *key)
{

	uint	encryptionalgo;
	u8 *pbssid;
	struct sta_info *stainfo;
	u8	bgroup = false;
	u8	bgrouptkey = false;/* can be remove later */
	u8	ret = _SUCCESS;

	if (((key->KeyIndex & 0x80000000) == 0) && ((key->KeyIndex & 0x40000000) > 0)) {

		/* It is invalid to clear bit 31 and set bit 30. If the miniport driver encounters this combination, */
		/* it must fail the request and return NDIS_STATUS_INVALID_DATA. */
		ret = _FAIL;
		goto exit;
	}

	if (key->KeyIndex & 0x40000000) {
		/* Pairwise key */

		pbssid = get_bssid(&padapter->mlmepriv);
		stainfo = rtw_get_stainfo(&padapter->stapriv, pbssid);

		if ((stainfo != NULL) && (padapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X)) {
			encryptionalgo = stainfo->dot118021XPrivacy;
		} else {
			encryptionalgo = padapter->securitypriv.dot11PrivacyAlgrthm;
		}

		if (key->KeyIndex & 0x000000FF) {
			/* The key index is specified in the lower 8 bits by values of zero to 255. */
			/* The key index should be set to zero for a Pairwise key, and the driver should fail with */
			/* NDIS_STATUS_INVALID_DATA if the lower 8 bits is not zero */
			ret = _FAIL;
			goto exit;
		}

		/* check BSSID */
		if (IS_MAC_ADDRESS_BROADCAST(key->BSSID) ) {

			ret = false;
			goto exit;
		}

		/* Check key length for TKIP. */
		/* if(encryptionAlgorithm == RT_ENC_TKIP_ENCRYPTION && key->KeyLength != 32) */
		if ((encryptionalgo == _TKIP_) && (key->KeyLength != 32)) {
			ret = _FAIL;
			goto exit;

		}

		/* Check key length for AES. */
		if ((encryptionalgo == _AES_) && (key->KeyLength != 16)) {
			/* For our supplicant, EAPPkt9x.vxd, cannot differentiate TKIP and AES case. */
			if (key->KeyLength == 32)
				key->KeyLength = 16;
			else {
				ret = _FAIL;
				goto exit;
			}
		}

		/* Check key length for WEP. For NDTEST, 2005.01.27, by rcnjko. -> modify checking condition*/
		if (((encryptionalgo == _WEP40_) && (key->KeyLength != 5)) || ((encryptionalgo == _WEP104_) && (key->KeyLength != 13))) {
			ret = _FAIL;
			goto exit;
		}

		bgroup = false;

		/* Check the pairwise key. Added by Annie, 2005-07-06. */

	} else {
		/* Group key - KeyIndex(BIT30==0) */

		/* when add wep key through add key and didn't assigned encryption type before */
		if ((padapter->securitypriv.ndisauthtype <= 3) && (padapter->securitypriv.dot118021XGrpPrivacy == 0)) {

			switch (key->KeyLength) {
			case 5:
				padapter->securitypriv.dot11PrivacyAlgrthm = _WEP40_;
				break;
			case 13:
				padapter->securitypriv.dot11PrivacyAlgrthm = _WEP104_;
				break;
			default:
				padapter->securitypriv.dot11PrivacyAlgrthm = _NO_PRIVACY_;
				break;
			}

			encryptionalgo = padapter->securitypriv.dot11PrivacyAlgrthm;

		} else {
			encryptionalgo = padapter->securitypriv.dot118021XGrpPrivacy;

		}

		if ((check_fwstate(&padapter->mlmepriv, WIFI_ADHOC_STATE) ) && (IS_MAC_ADDRESS_BROADCAST(key->BSSID) == false)) {
			ret = _FAIL;
			goto exit;
		}

		/* Check key length for TKIP */
		if ((encryptionalgo == _TKIP_) && (key->KeyLength != 32)) {

			ret = _FAIL;
			goto exit;

		} else if (encryptionalgo == _AES_ && (key->KeyLength != 16 && key->KeyLength != 32)) {

			/* Check key length for AES */
			/* For NDTEST, we allow keylen=32 in this case. 2005.01.27, by rcnjko. */
			ret = _FAIL;
			goto exit;
		}

		/* Change the key length for EAPPkt9x.vxd. Added by Annie, 2005-11-03. */
		if ((encryptionalgo ==  _AES_) && (key->KeyLength == 32)) {
			key->KeyLength = 16;
		}

		if (key->KeyIndex & 0x8000000) /* error ??? 0x8000_0000 */
			bgrouptkey = true;

		if ((check_fwstate(&padapter->mlmepriv, WIFI_ADHOC_STATE) ) && (check_fwstate(&padapter->mlmepriv, _FW_LINKED) == true))
			bgrouptkey = true;

		bgroup = true;

	}

	/* If WEP encryption algorithm, just call rtw_set_802_11_add_wep(). */
	if ((padapter->securitypriv.dot11AuthAlgrthm != dot11AuthAlgrthm_8021X) && (encryptionalgo == _WEP40_  || encryptionalgo == _WEP104_)) {
		u8 ret;
		u32 keyindex;
		u32 len = FIELD_OFFSET(NDIS_802_11_KEY, KeyMaterial) + key->KeyLength;
		NDIS_802_11_WEP *wep = &padapter->securitypriv.ndiswep;

		wep->Length = len;
		keyindex = key->KeyIndex & 0x7fffffff;
		wep->KeyIndex = keyindex ;
		wep->KeyLength = key->KeyLength;

		memcpy(wep->KeyMaterial, key->KeyMaterial, key->KeyLength);
		memcpy(&(padapter->securitypriv.dot11DefKey[keyindex].skey[0]), key->KeyMaterial, key->KeyLength);

		padapter->securitypriv.dot11DefKeylen[keyindex] = key->KeyLength;
		padapter->securitypriv.dot11PrivacyKeyIndex = keyindex;

		ret = rtw_set_802_11_add_wep(padapter, wep);

		goto exit;

	}

	if (key->KeyIndex & 0x20000000) {
		/* SetRSC */
		if (bgroup ) {
			NDIS_802_11_KEY_RSC keysrc = key->KeyRSC & 0x00FFFFFFFFFFFFULL;
			memcpy(&padapter->securitypriv.dot11Grprxpn, &keysrc, 8);
		} else {
			NDIS_802_11_KEY_RSC keysrc = key->KeyRSC & 0x00FFFFFFFFFFFFULL;
			memcpy(&padapter->securitypriv.dot11Grptxpn, &keysrc, 8);
		}

	}

	/* Indicate this key idx is used for TX */
	/* Save the key in KeyMaterial */
	if (bgroup ) { /* Group transmit key */
		int res;

		if (bgrouptkey )
			padapter->securitypriv.dot118021XGrpKeyid = (u8)key->KeyIndex;

		if ((key->KeyIndex & 0x3) == 0) {
			ret = _FAIL;
			goto exit;
		}

		memset(&padapter->securitypriv.dot118021XGrpKey[(u8)((key->KeyIndex) & 0x03)], 0, 16);
		memset(&padapter->securitypriv.dot118021XGrptxmickey[(u8)((key->KeyIndex) & 0x03)], 0, 16);
		memset(&padapter->securitypriv.dot118021XGrprxmickey[(u8)((key->KeyIndex) & 0x03)], 0, 16);

		if ((key->KeyIndex & 0x10000000)) {
			memcpy(&padapter->securitypriv.dot118021XGrptxmickey[(u8)((key->KeyIndex) & 0x03)], key->KeyMaterial + 16, 8);
			memcpy(&padapter->securitypriv.dot118021XGrprxmickey[(u8)((key->KeyIndex) & 0x03)], key->KeyMaterial + 24, 8);

		} else {
			memcpy(&padapter->securitypriv.dot118021XGrptxmickey[(u8)((key->KeyIndex) & 0x03)], key->KeyMaterial + 24, 8);
			memcpy(&padapter->securitypriv.dot118021XGrprxmickey[(u8)((key->KeyIndex) & 0x03)], key->KeyMaterial + 16, 8);

		}

		/* set group key by index */
		memcpy(&padapter->securitypriv.dot118021XGrpKey[(u8)((key->KeyIndex) & 0x03)], key->KeyMaterial, key->KeyLength);

		key->KeyIndex = key->KeyIndex & 0x03;

		padapter->securitypriv.binstallGrpkey = true;

		padapter->securitypriv.bcheck_grpkey = false;

		res = rtw_set_key(padapter, &padapter->securitypriv, key->KeyIndex, 1, true);

		if (res == _FAIL)
			ret = _FAIL;

		goto exit;

	} else { /* Pairwise Key */
		u8 res;

		pbssid = get_bssid(&padapter->mlmepriv);
		stainfo = rtw_get_stainfo(&padapter->stapriv , pbssid);

		if (stainfo != NULL) {
			memset(&stainfo->dot118021x_UncstKey, 0, 16); /* clear keybuffer */

			memcpy(&stainfo->dot118021x_UncstKey, key->KeyMaterial, 16);

			if (encryptionalgo == _TKIP_) {
				padapter->securitypriv.busetkipkey = false;

				/* _set_timer(&padapter->securitypriv.tkip_timer, 50); */

				/* if TKIP, save the Receive/Transmit MIC key in KeyMaterial[128-255] */
				if ((key->KeyIndex & 0x10000000)) {
					memcpy(&stainfo->dot11tkiptxmickey, key->KeyMaterial + 16, 8);
					memcpy(&stainfo->dot11tkiprxmickey, key->KeyMaterial + 24, 8);

				} else {
					memcpy(&stainfo->dot11tkiptxmickey, key->KeyMaterial + 24, 8);
					memcpy(&stainfo->dot11tkiprxmickey, key->KeyMaterial + 16, 8);

				}

			} else if (encryptionalgo == _AES_) {

			}

			/* Set key to CAM through H2C command */

			res = rtw_setstakey_cmd(padapter, stainfo, UNICAST_KEY, true);

			if (res == false)
				ret = _FAIL;
		}
	}

exit:

	return ret;
}

u8 rtw_set_802_11_remove_key(_adapter	*padapter, NDIS_802_11_REMOVE_KEY *key)
{

	uint				encryptionalgo;
	u8 *pbssid;
	struct sta_info *stainfo;
	u8	bgroup = (key->KeyIndex & 0x4000000) > 0 ? false : true;
	u8	keyIndex = (u8)key->KeyIndex & 0x03;
	u8	ret = _SUCCESS;

	if ((key->KeyIndex & 0xbffffffc) > 0) {
		ret = _FAIL;
		goto exit;
	}

	if (bgroup ) {
		encryptionalgo = padapter->securitypriv.dot118021XGrpPrivacy;
		/* clear group key by index */
		/* NdisZeroMemory(Adapter->MgntInfo.SecurityInfo.KeyBuf[keyIndex], MAX_WEP_KEY_LEN); */
		/* Adapter->MgntInfo.SecurityInfo.KeyLen[keyIndex] = 0; */

		memset(&padapter->securitypriv.dot118021XGrpKey[keyIndex], 0, 16);

		/* ! \todo Send a H2C Command to Firmware for removing this Key in CAM Entry. */

	} else {

		pbssid = get_bssid(&padapter->mlmepriv);
		stainfo = rtw_get_stainfo(&padapter->stapriv , pbssid);
		if (stainfo != NULL) {
			encryptionalgo = stainfo->dot118021XPrivacy;

			/* clear key by BSSID */
			memset(&stainfo->dot118021x_UncstKey, 0, 16);

			/* ! \todo Send a H2C Command to Firmware for disable this Key in CAM Entry. */

		} else {
			ret = _FAIL;
			goto exit;
		}
	}

exit:

	return true;

}

/*
* rtw_get_cur_max_rate -
* @adapter: pointer to _adapter structure
*
* Return 0 or 100Kbps
*/
u16 rtw_get_cur_max_rate(_adapter *adapter)
{
	int	i = 0;
	u16	rate = 0, max_rate = 0;
	struct mlme_priv	*pmlmepriv = &adapter->mlmepriv;
	WLAN_BSSID_EX	*pcur_bss = &pmlmepriv->cur_network.network;
	struct sta_info *psta = NULL;
	u8	short_GI = 0;
	u8	rf_type = 0;

#ifdef CONFIG_MP_INCLUDED
	if (adapter->registrypriv.mp_mode == 1) {
		if (check_fwstate(pmlmepriv, WIFI_MP_STATE) )
			return 0;
	}
#endif

	if ((check_fwstate(pmlmepriv, _FW_LINKED) != true)
	    && (check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) != true))
		return 0;

	psta = rtw_get_stainfo(&adapter->stapriv, get_bssid(pmlmepriv));
	if (psta == NULL)
		return 0;

	short_GI = query_ra_short_GI(psta, psta->bw_mode);

	if (is_supported_ht(psta->wireless_mode)) {
		rtw_hal_get_hwreg(adapter, HW_VAR_RF_TYPE, (u8 *)(&rf_type));
		max_rate = rtw_mcs_rate(rf_type
			, (psta->bw_mode == CHANNEL_WIDTH_40) ? 1 : 0
			, short_GI
			, psta->htpriv.ht_cap.supp_mcs_set
		);
	} else {
		while ((pcur_bss->SupportedRates[i] != 0) && (pcur_bss->SupportedRates[i] != 0xFF)) {
			rate = pcur_bss->SupportedRates[i] & 0x7F;
			if (rate > max_rate)
				max_rate = rate;
			i++;
		}

		max_rate = max_rate * 10 / 2;
	}

	return max_rate;
}

/*
* rtw_set_scan_mode -
* @adapter: pointer to _adapter structure
* @scan_mode:
*
* Return _SUCCESS or _FAIL
*/
int rtw_set_scan_mode(_adapter *adapter, RT_SCAN_TYPE scan_mode)
{
	if (scan_mode != SCAN_ACTIVE && scan_mode != SCAN_PASSIVE)
		return _FAIL;

	adapter->mlmepriv.scan_mode = scan_mode;

	return _SUCCESS;
}

/*
* rtw_set_channel_plan -
* @adapter: pointer to _adapter structure
* @channel_plan:
*
* Return _SUCCESS or _FAIL
*/
int rtw_set_channel_plan(_adapter *adapter, u8 channel_plan)
{
	struct registry_priv *pregistrypriv = &adapter->registrypriv;
	struct mlme_priv *pmlmepriv = &adapter->mlmepriv;

	/* handle by cmd_thread to sync with scan operation */
	return rtw_set_chplan_cmd(adapter, RTW_CMDF_WAIT_ACK, channel_plan, 1);
}

/*
* rtw_set_country -
* @adapter: pointer to _adapter structure
* @country_code: string of country code
*
* Return _SUCCESS or _FAIL
*/
int rtw_set_country(_adapter *adapter, const char *country_code)
{
#ifdef CONFIG_RTW_IOCTL_SET_COUNTRY
	return rtw_set_country_cmd(adapter, RTW_CMDF_WAIT_ACK, country_code, 1);
#else
	return _FAIL;
#endif
}

/*
* rtw_set_band -
* @adapter: pointer to _adapter structure
* @band: band to set
*
* Return _SUCCESS or _FAIL
*/
int rtw_set_band(_adapter *adapter, u8 band)
{
	if (rtw_band_valid(band)) {
		RTW_INFO(FUNC_ADPT_FMT" band:%d\n", FUNC_ADPT_ARG(adapter), band);
		adapter->setband = band;
		return _SUCCESS;
	}

	RTW_PRINT(FUNC_ADPT_FMT" band:%d fail\n", FUNC_ADPT_ARG(adapter), band);
	return _FAIL;
}
