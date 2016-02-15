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
#define _IOCTL_LINUX_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <wlan_bssdef.h>
#include <rtw_debug.h>
#include <wifi.h>
#include <rtw_mlme.h>
#include <rtw_mlme_ext.h>
#include <rtw_ioctl.h>
#include <rtw_ioctl_set.h>
#include <rtw_ioctl_query.h>
#include <mlme_osdep.h>
#include <rtw_mp_ioctl.h>
#include <usb_ops.h>
#include <rtw_version.h>
#include <rtl8188e_hal.h>
#include <rtw_iol.h>

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,27))
#define  iwe_stream_add_event(a, b, c, d, e)  iwe_stream_add_event(b, c, d, e)
#define  iwe_stream_add_point(a, b, c, d, e)  iwe_stream_add_point(b, c, d, e)
#endif


#define RTL_IOCTL_WPA_SUPPLICANT	SIOCIWFIRSTPRIV+30

#define SCAN_ITEM_SIZE 768
#define MAX_CUSTOM_LEN 64
#define RATE_COUNT 4

/*  combo scan */
#define WEXT_CSCAN_AMOUNT 9
#define WEXT_CSCAN_BUF_LEN		360
#define WEXT_CSCAN_HEADER		"CSCAN S\x01\x00\x00S\x00"
#define WEXT_CSCAN_HEADER_SIZE		12
#define WEXT_CSCAN_SSID_SECTION		'S'
#define WEXT_CSCAN_CHANNEL_SECTION	'C'
#define WEXT_CSCAN_NPROBE_SECTION	'N'
#define WEXT_CSCAN_ACTV_DWELL_SECTION	'A'
#define WEXT_CSCAN_PASV_DWELL_SECTION	'P'
#define WEXT_CSCAN_HOME_DWELL_SECTION	'H'
#define WEXT_CSCAN_TYPE_SECTION		'T'


extern u8 key_2char2num(u8 hch, u8 lch);
extern u8 str_2char2num(u8 hch, u8 lch);

static u32 rtw_rates[] = {1000000, 2000000, 5500000, 11000000,
	6000000, 9000000, 12000000, 18000000, 24000000, 36000000,
	48000000, 54000000};

static const char *const iw_operation_mode[] =
{
	"Auto", "Ad-Hoc", "Managed",  "Master", "Repeater", "Secondary", "Monitor"
};

static int hex2num_i(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	return -1;
}

static int hex2byte_i(const char *hex)
{
	int a, b;
	a = hex2num_i(*hex++);
	if (a < 0)
		return -1;
	b = hex2num_i(*hex++);
	if (b < 0)
		return -1;
	return (a << 4) | b;
}

/**
 * hwaddr_aton - Convert ASCII string to MAC address
 * @txt: MAC address as a string (e.g., "00:11:22:33:44:55")
 * @addr: Buffer for the MAC address (ETH_ALEN = 6 bytes)
 * Returns: 0 on success, -1 on failure (e.g., string not a MAC address)
 */
static int hwaddr_aton_i(const char *txt, u8 *addr)
{
	int i;

	for (i = 0; i < 6; i++) {
		int a, b;

		a = hex2num_i(*txt++);
		if (a < 0)
			return -1;
		b = hex2num_i(*txt++);
		if (b < 0)
			return -1;
		*addr++ = (a << 4) | b;
		if (i < 5 && *txt++ != ':')
			return -1;
	}

	return 0;
}

static void indicate_wx_custom_event(struct adapter *padapter, char *msg)
{
	u8 *buff, *p;
	union iwreq_data wrqu;

	if (strlen(msg) > IW_CUSTOM_MAX) {
		DBG_88E("%s strlen(msg):%zu > IW_CUSTOM_MAX:%u\n", __FUNCTION__ , strlen(msg), IW_CUSTOM_MAX);
		return;
	}

	buff = rtw_zmalloc(IW_CUSTOM_MAX+1);
	if (!buff)
		return;

	memcpy(buff, msg, strlen(msg));

	memset(&wrqu,0,sizeof(wrqu));
	wrqu.data.length = strlen(msg);

	DBG_88E("%s %s\n", __FUNCTION__, buff);

	rtw_mfree(buff, IW_CUSTOM_MAX+1);
}

static void request_wps_pbc_event(struct adapter *padapter)
{
	u8 *buff, *p;
	union iwreq_data wrqu;


	buff = rtw_malloc(IW_CUSTOM_MAX);
	if (!buff)
		return;

	memset(buff, 0, IW_CUSTOM_MAX);

	p=buff;

	p+=sprintf(p, "WPS_PBC_START.request=true");

	memset(&wrqu,0,sizeof(wrqu));

	wrqu.data.length = p-buff;

	wrqu.data.length = (wrqu.data.length<IW_CUSTOM_MAX) ? wrqu.data.length:IW_CUSTOM_MAX;

	DBG_88E("%s\n", __FUNCTION__);

	if (buff)
	{
		rtw_mfree(buff, IW_CUSTOM_MAX);
	}

}


void indicate_wx_scan_complete_event(struct adapter *padapter)
{
	union iwreq_data wrqu;
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;

	memset(&wrqu, 0, sizeof(union iwreq_data));

	/* DBG_88E("+rtw_indicate_wx_scan_complete_event\n"); */
}


void rtw_indicate_wx_assoc_event(struct adapter *padapter)
{
	union iwreq_data wrqu;
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct wlan_bssid_ex		*pnetwork = (struct wlan_bssid_ex*)(&(pmlmeinfo->network));

	memset(&wrqu, 0, sizeof(union iwreq_data));

	wrqu.ap_addr.sa_family = ARPHRD_ETHER;

	if (check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE)==true )
		memcpy(wrqu.ap_addr.sa_data, pnetwork->MacAddress, ETH_ALEN);
	else
		memcpy(wrqu.ap_addr.sa_data, pmlmepriv->cur_network.network.MacAddress, ETH_ALEN);

	DBG_88E_LEVEL(_drv_always_, "assoc success\n");
}

void rtw_indicate_wx_disassoc_event(struct adapter *padapter)
{
	union iwreq_data wrqu;

	memset(&wrqu, 0, sizeof(union iwreq_data));

	wrqu.ap_addr.sa_family = ARPHRD_ETHER;
	memset(wrqu.ap_addr.sa_data, 0, ETH_ALEN);
}

static char *translate_scan(struct adapter *padapter,
				struct iw_request_info* info, struct wlan_network *pnetwork,
				char *start, char *stop)
{
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	u8 ss, sq;
	struct iw_event iwe;
	u16 cap;
	__le16 le_cap;
	u32 ht_ielen = 0;
	char *custom;
	char *p;
	u16 max_rate=0, rate, ht_cap=false;
	u32 i = 0;
	char	*current_val;
	long rssi;
	u8 bw_40MHz=0, short_GI=0;
	u16 mcs_rate=0;
	struct registry_priv *pregpriv = &padapter->registrypriv;
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &padapter->wdinfo;
#endif /* CONFIG_P2P */

#ifdef CONFIG_P2P
#ifdef CONFIG_P2P
	if ( SCAN_RESULT_ALL == pwdinfo->wfd_info->scan_result_type )
	{

	}
	else if ( ( SCAN_RESULT_P2P_ONLY == pwdinfo->wfd_info->scan_result_type ) ||
		      ( SCAN_RESULT_WFD_TYPE == pwdinfo->wfd_info->scan_result_type ) )
#endif /*  CONFIG_P2P */
	{
		if (!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
		{
			u32	blnGotP2PIE = false;

			/* 	User is doing the P2P device discovery */
			/* 	The prefix of SSID should be "DIRECT-" and the IE should contains the P2P IE. */
			/* 	If not, the driver should ignore this AP and go to the next AP. */

			/* 	Verifying the SSID */
			if ( _rtw_memcmp( pnetwork->network.Ssid.Ssid, pwdinfo->p2p_wildcard_ssid, P2P_WILDCARD_SSID_LEN ) )
			{
				u32	p2pielen = 0;

				if (pnetwork->network.Reserved[0] == 2) { /*  Probe Request */
					/* 	Verifying the P2P IE */
					if ( rtw_get_p2p_ie( pnetwork->network.IEs, pnetwork->network.IELength, NULL, &p2pielen) )
						blnGotP2PIE = true;
				} else { /*  Beacon or Probe Respones */
					/* 	Verifying the P2P IE */
					if ( rtw_get_p2p_ie( &pnetwork->network.IEs[12], pnetwork->network.IELength - 12, NULL, &p2pielen) )
						blnGotP2PIE = true;
				}
			}

			if ( blnGotP2PIE == false )
				return start;
		}
	}

#ifdef CONFIG_P2P
	if ( SCAN_RESULT_WFD_TYPE == pwdinfo->wfd_info->scan_result_type )
	{
		u32	blnGotWFD = false;
		u8	*wfd_ie;
		uint	wfd_ielen = 0;

		wfd_ie = kzalloc(128, GFP_ATOMIC);
		if (!wfd_ie)
			return start;
		if ( rtw_get_wfd_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength,  wfd_ie, &wfd_ielen, pnetwork->network.Reserved[0]) )
		{
			u8	wfd_devinfo[ 6 ] = { 0x00 };
			uint	wfd_devlen = 6;

			if ( rtw_get_wfd_attr_content( wfd_ie, wfd_ielen, WFD_ATTR_DEVICE_INFO, wfd_devinfo, &wfd_devlen) )
			{
				if ( pwdinfo->wfd_info->wfd_device_type == WFD_DEVINFO_PSINK )
				{
					/* 	the first two bits will indicate the WFD device type */
					if ( ( wfd_devinfo[ 1 ] & 0x03 ) == WFD_DEVINFO_SOURCE )
					{
						/* 	If this device is Miracast PSink device, the scan reuslt should just provide the Miracast source. */
						blnGotWFD = true;
					}
				}
				else if ( pwdinfo->wfd_info->wfd_device_type == WFD_DEVINFO_SOURCE )
				{
					/* 	the first two bits will indicate the WFD device type */
					if ( ( wfd_devinfo[ 1 ] & 0x03 ) == WFD_DEVINFO_PSINK )
					{
						/* 	If this device is Miracast source device, the scan reuslt should just provide the Miracast PSink. */
						/* 	Todo: How about the SSink?! */
						blnGotWFD = true;
					}
				}
			}
		}
		kfree(wfd_ie);
		if ( blnGotWFD == false )
			return start;
	}
#endif /*  CONFIG_P2P */

#endif /* CONFIG_P2P */

	/*  AP MAC address  */
	iwe.cmd = SIOCGIWAP;
	iwe.u.ap_addr.sa_family = ARPHRD_ETHER;

	memcpy(iwe.u.ap_addr.sa_data, pnetwork->network.MacAddress, ETH_ALEN);
	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_ADDR_LEN);

	/* Add the ESSID */
	iwe.cmd = SIOCGIWESSID;
	iwe.u.data.flags = 1;
	iwe.u.data.length = min((u16)pnetwork->network.Ssid.SsidLength, (u16)32);
	start = iwe_stream_add_point(info, start, stop, &iwe, pnetwork->network.Ssid.Ssid);

	/* parsing HT_CAP_IE */
	p = rtw_get_ie(&pnetwork->network.IEs[12], _HT_CAPABILITY_IE_, &ht_ielen, pnetwork->network.IELength-12);

	if (p && ht_ielen>0) {
		struct ieee80211_ht_cap *pht_capie;
		ht_cap = true;
		pht_capie = (struct ieee80211_ht_cap *)(p+2);
		memcpy(&mcs_rate , pht_capie->mcs.rx_mask, 2);
		bw_40MHz = (le16_to_cpu(pht_capie->cap_info) & IEEE80211_HT_CAP_SUP_WIDTH) ? 1 : 0;
		short_GI = (le16_to_cpu(pht_capie->cap_info) & (IEEE80211_HT_CAP_SGI_20 | IEEE80211_HT_CAP_SGI_40)) ? 1 : 0;
	}

	/* Add the protocol name */
	iwe.cmd = SIOCGIWNAME;
	if ((rtw_is_cckratesonly_included((u8*)&pnetwork->network.SupportedRates)) == true) {
		if (ht_cap == true)
			snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11bn");
		else
			snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11b");
	} else if ((rtw_is_cckrates_included((u8*)&pnetwork->network.SupportedRates)) == true) {
		if (ht_cap == true)
			snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11bgn");
		else
			snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11bg");
	} else {
		if (pnetwork->network.Configuration.DSConfig > 14) {
			if (ht_cap == true)
				snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11an");
			else
				snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11a");
		} else {
			if (ht_cap == true)
				snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11gn");
			else
				snprintf(iwe.u.name, IFNAMSIZ, "IEEE 802.11g");
		}
	}

	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_CHAR_LEN);

	  /* Add mode */
        iwe.cmd = SIOCGIWMODE;
	memcpy((u8 *)&le_cap, rtw_get_capability_from_ie(pnetwork->network.IEs), 2);


	cap = le16_to_cpu(le_cap);

	if (cap & (WLAN_CAPABILITY_IBSS |WLAN_CAPABILITY_BSS)) {
		if (cap & WLAN_CAPABILITY_BSS)
			iwe.u.mode = IW_MODE_MASTER;
		else
			iwe.u.mode = IW_MODE_ADHOC;

		start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_UINT_LEN);
	}

	if (pnetwork->network.Configuration.DSConfig<1 /*|| pnetwork->network.Configuration.DSConfig>14*/)
		pnetwork->network.Configuration.DSConfig = 1;

	 /* Add frequency/channel */
	iwe.cmd = SIOCGIWFREQ;
	iwe.u.freq.m = rtw_ch2freq(pnetwork->network.Configuration.DSConfig) * 100000;
	iwe.u.freq.e = 1;
	iwe.u.freq.i = pnetwork->network.Configuration.DSConfig;
	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_FREQ_LEN);

	/* Add encryption capability */
	iwe.cmd = SIOCGIWENCODE;
	if (cap & WLAN_CAPABILITY_PRIVACY)
		iwe.u.data.flags = IW_ENCODE_ENABLED | IW_ENCODE_NOKEY;
	else
		iwe.u.data.flags = IW_ENCODE_DISABLED;
	iwe.u.data.length = 0;
	start = iwe_stream_add_point(info, start, stop, &iwe, pnetwork->network.Ssid.Ssid);

	/*Add basic and extended rates */
	max_rate = 0;
	custom = kzalloc(MAX_CUSTOM_LEN, GFP_ATOMIC);
	if (!custom)
		return start;
	p = custom;
	p += snprintf(p, MAX_CUSTOM_LEN - (p - custom), " Rates (Mb/s): ");
	while (pnetwork->network.SupportedRates[i]!=0) {
		rate = pnetwork->network.SupportedRates[i]&0x7F;
		if (rate > max_rate)
			max_rate = rate;
		p += snprintf(p, MAX_CUSTOM_LEN - (p - custom),
			      "%d%s ", rate >> 1, (rate & 1) ? ".5" : "");
		i++;
	}

	if (ht_cap == true) {
		if (mcs_rate&0x8000) { /* MCS15 */
			max_rate = (bw_40MHz) ? ((short_GI)?300:270):((short_GI)?144:130);

		} else if (mcs_rate&0x0080) { /* MCS7 */
			max_rate = (bw_40MHz) ? ((short_GI)?150:135):((short_GI)?72:65);
		} else { /* default MCS7 */
			/* DBG_88E("wx_get_scan, mcs_rate_bitmap=0x%x\n", mcs_rate); */
			max_rate = (bw_40MHz) ? ((short_GI)?150:135):((short_GI)?72:65);
		}

		max_rate = max_rate*2;/* Mbps/2; */
	}

	iwe.cmd = SIOCGIWRATE;
	iwe.u.bitrate.fixed = iwe.u.bitrate.disabled = 0;
	iwe.u.bitrate.value = max_rate * 500000;
	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_PARAM_LEN);

	/* parsing WPA/WPA2 IE */
	{
		u8 *buf;
		u8 *wpa_ie, *rsn_ie;
		u16 wpa_len=0,rsn_len=0;
		u8 *p;
		sint out_len=0;

		buf = kzalloc(MAX_WPA_IE_LEN, GFP_ATOMIC);
		if (!buf)
			goto exit;
		wpa_ie = kzalloc(255, GFP_ATOMIC);
		if (!wpa_ie) {
			kfree(buf);
			goto exit;
		}
		rsn_ie = kzalloc(255, GFP_ATOMIC);
		if (!rsn_ie) {
			kfree(buf);
			kfree(wpa_ie);
			goto exit;
		}
		out_len=rtw_get_sec_ie(pnetwork->network.IEs ,pnetwork->network.IELength,rsn_ie,&rsn_len,wpa_ie,&wpa_len);
		RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_scan: ssid=%s\n",pnetwork->network.Ssid.Ssid));
		RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_scan: wpa_len=%d rsn_len=%d\n",wpa_len,rsn_len));

		if (wpa_len > 0) {
			p=buf;
			memset(buf, 0, MAX_WPA_IE_LEN);
			p += sprintf(p, "wpa_ie=");
			for (i = 0; i < wpa_len; i++)
				p += sprintf(p, "%02x", wpa_ie[i]);

			memset(&iwe, 0, sizeof(iwe));
			iwe.cmd = IWEVCUSTOM;
			iwe.u.data.length = strlen(buf);
			start = iwe_stream_add_point(info, start, stop, &iwe,buf);

			memset(&iwe, 0, sizeof(iwe));
			iwe.cmd =IWEVGENIE;
			iwe.u.data.length = wpa_len;
			start = iwe_stream_add_point(info, start, stop, &iwe, wpa_ie);
		}
		if (rsn_len > 0) {
			p = buf;
			memset(buf, 0, MAX_WPA_IE_LEN);
			p += sprintf(p, "rsn_ie=");
			for (i = 0; i < rsn_len; i++)
				p += sprintf(p, "%02x", rsn_ie[i]);
			memset(&iwe, 0, sizeof(iwe));
			iwe.cmd = IWEVCUSTOM;
			iwe.u.data.length = strlen(buf);
			start = iwe_stream_add_point(info, start, stop, &iwe,buf);

			memset(&iwe, 0, sizeof(iwe));
			iwe.cmd =IWEVGENIE;
			iwe.u.data.length = rsn_len;
			start = iwe_stream_add_point(info, start, stop, &iwe, rsn_ie);
		}
		kfree(buf);
		kfree(wpa_ie);
		kfree(rsn_ie);
	}

	{ /* parsing WPS IE */
		uint cnt = 0,total_ielen;
		u8 *wpsie_ptr= NULL;
		uint wps_ielen = 0;

		u8 *ie_ptr = pnetwork->network.IEs +_FIXED_IE_LENGTH_;
		total_ielen= pnetwork->network.IELength - _FIXED_IE_LENGTH_;

		if (pnetwork->network.Reserved[0] == 2) /*  Probe Request */
		{
			ie_ptr = pnetwork->network.IEs;
			total_ielen = pnetwork->network.IELength;
		}
		else     /*  Beacon or Probe Respones */
		{
			ie_ptr = pnetwork->network.IEs + _FIXED_IE_LENGTH_;
			total_ielen = pnetwork->network.IELength - _FIXED_IE_LENGTH_;
		}

		while (cnt < total_ielen) {
			if (rtw_is_wps_ie(&ie_ptr[cnt], &wps_ielen) && (wps_ielen>2)) {
				wpsie_ptr = &ie_ptr[cnt];
				iwe.cmd =IWEVGENIE;
				iwe.u.data.length = (u16)wps_ielen;
				start = iwe_stream_add_point(info, start, stop, &iwe, wpsie_ptr);
			}
			cnt+=ie_ptr[cnt+1]+2; /* goto next */
		}
	}

	/* Add quality statistics */
	iwe.cmd = IWEVQUAL;
	iwe.u.qual.updated = IW_QUAL_QUAL_UPDATED | IW_QUAL_LEVEL_UPDATED | IW_QUAL_NOISE_INVALID;

	if ( check_fwstate(pmlmepriv, _FW_LINKED)== true &&
		is_same_network(&pmlmepriv->cur_network.network, &pnetwork->network, 0)) {
		ss = padapter->recvpriv.signal_strength;
		sq = padapter->recvpriv.signal_qual;
	} else {
		ss = pnetwork->network.PhyInfo.SignalStrength;
		sq = pnetwork->network.PhyInfo.SignalQuality;
	}


	iwe.u.qual.level = (u8)ss;/*  */
	#ifdef CONFIG_BT_COEXIST
	BT_SignalCompensation(padapter, &iwe.u.qual.level, NULL);
	#endif /*  CONFIG_BT_COEXIST */

	iwe.u.qual.qual = (u8)sq;   /*  signal quality */

	iwe.u.qual.noise = 0; /*  noise level */

	start = iwe_stream_add_event(info, start, stop, &iwe, IW_EV_QUAL_LEN);

	{
		u8 *buf;
		u8 * p,*pos;
		int len;

		buf = kzalloc(MAX_WPA_IE_LEN, GFP_ATOMIC);
		if (!buf)
			goto exit;
		p = buf;
		pos = pnetwork->network.Reserved;
		memset(buf, 0, MAX_WPA_IE_LEN);
		p += sprintf(p, "fm=%02X%02X", pos[1], pos[0]);
		memset(&iwe, 0, sizeof(iwe));
		iwe.cmd = IWEVCUSTOM;
		iwe.u.data.length = strlen(buf);
		start = iwe_stream_add_point(info, start, stop, &iwe, buf);
		kfree(buf);
	}
exit:
	kfree(custom);
	return start;
}

static int wpa_set_auth_algs(struct net_device *dev, u32 value)
{
	struct adapter *padapter = (struct adapter *) rtw_netdev_priv(dev);
	int ret = 0;

	if ((value & AUTH_ALG_SHARED_KEY)&&(value & AUTH_ALG_OPEN_SYSTEM))
	{
		DBG_88E("wpa_set_auth_algs, AUTH_ALG_SHARED_KEY and  AUTH_ALG_OPEN_SYSTEM [value:0x%x]\n",value);
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
		padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeAutoSwitch;
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
	}
	else if (value & AUTH_ALG_SHARED_KEY)
	{
		DBG_88E("wpa_set_auth_algs, AUTH_ALG_SHARED_KEY  [value:0x%x]\n",value);
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;

		padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeShared;
		padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Shared;
	}
	else if (value & AUTH_ALG_OPEN_SYSTEM)
	{
		DBG_88E("wpa_set_auth_algs, AUTH_ALG_OPEN_SYSTEM\n");
		/* padapter->securitypriv.ndisencryptstatus = Ndis802_11EncryptionDisabled; */
		if (padapter->securitypriv.ndisauthtype < Ndis802_11AuthModeWPAPSK)
		{
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeOpen;
			padapter->securitypriv.dot11AuthAlgrthm = dot11AuthAlgrthm_Open;
		}

	}
	else if (value & AUTH_ALG_LEAP)
	{
		DBG_88E("wpa_set_auth_algs, AUTH_ALG_LEAP\n");
	}
	else
	{
		DBG_88E("wpa_set_auth_algs, error!\n");
		ret = -EINVAL;
	}

	return ret;

}

static int wpa_set_encryption(struct net_device *dev, struct ieee_param *param, u32 param_len)
{
	int ret = 0;
	u32 wep_key_idx, wep_key_len,wep_total_len;
	struct ndis_802_11_wep	 *pwep = NULL;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
#ifdef CONFIG_P2P
	struct wifidirect_info* pwdinfo = &padapter->wdinfo;
#endif /* CONFIG_P2P */

;

	param->u.crypt.err = 0;
	param->u.crypt.alg[IEEE_CRYPT_ALG_NAME_LEN - 1] = '\0';

	if (param_len < (u32) ((u8 *) param->u.crypt.key - (u8 *) param) + param->u.crypt.key_len)
	{
		ret =  -EINVAL;
		goto exit;
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff)
	{

		if (param->u.crypt.idx >= WEP_KEYS
#ifdef CONFIG_IEEE80211W
			&& param->u.crypt.idx > BIP_MAX_KEYID
#endif /* CONFIG_IEEE80211W */
			)
		{
			ret = -EINVAL;
			goto exit;
		}
	} else {
		ret = -EINVAL;
		goto exit;
	}

	if (strcmp(param->u.crypt.alg, "WEP") == 0) {
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("wpa_set_encryption, crypt.alg = WEP\n"));
		DBG_88E("wpa_set_encryption, crypt.alg = WEP\n");

		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
		padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
		padapter->securitypriv.dot118021XGrpPrivacy=_WEP40_;

		wep_key_idx = param->u.crypt.idx;
		wep_key_len = param->u.crypt.key_len;

		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("(1)wep_key_idx=%d\n", wep_key_idx));
		DBG_88E("(1)wep_key_idx=%d\n", wep_key_idx);

		if (wep_key_idx > WEP_KEYS)
			return -EINVAL;

		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("(2)wep_key_idx=%d\n", wep_key_idx));

		if (wep_key_len > 0)
		{
			wep_key_len = wep_key_len <= 5 ? 5 : 13;
			wep_total_len = wep_key_len + FIELD_OFFSET(struct ndis_802_11_wep, KeyMaterial);
			pwep =(struct ndis_802_11_wep	 *) rtw_malloc(wep_total_len);
			if (pwep == NULL) {
				RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,(" wpa_set_encryption: pwep allocate fail !!!\n"));
				goto exit;
			}

			memset(pwep, 0, wep_total_len);

			pwep->KeyLength = wep_key_len;
			pwep->Length = wep_total_len;

			if (wep_key_len==13)
			{
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;
				padapter->securitypriv.dot118021XGrpPrivacy=_WEP104_;
			}
		}
		else {
			ret = -EINVAL;
			goto exit;
		}

		pwep->KeyIndex = wep_key_idx;
		pwep->KeyIndex |= 0x80000000;

		memcpy(pwep->KeyMaterial,  param->u.crypt.key, pwep->KeyLength);

		if (param->u.crypt.set_tx)
		{
			DBG_88E("wep, set_tx=1\n");

			if (rtw_set_802_11_add_wep(padapter, pwep) == (u8)_FAIL)
			{
				ret = -EOPNOTSUPP ;
			}
		} else {
			DBG_88E("wep, set_tx=0\n");

			if (wep_key_idx >= WEP_KEYS) {
				ret = -EOPNOTSUPP ;
				goto exit;
			}
			memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), pwep->KeyMaterial, pwep->KeyLength);
			psecuritypriv->dot11DefKeylen[wep_key_idx]=pwep->KeyLength;
			rtw_set_key(padapter, psecuritypriv, wep_key_idx, 0,true);
		}

		goto exit;
	}

	if (padapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X) /*  802_1x */
	{
		struct sta_info * psta,*pbcmc_sta;
		struct sta_priv * pstapriv = &padapter->stapriv;

		if (check_fwstate(pmlmepriv, WIFI_STATION_STATE | WIFI_MP_STATE) == true) /* sta mode */
		{
			psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));
			if (psta == NULL) {
				/* DEBUG_ERR( ("Set wpa_set_encryption: Obtain Sta_info fail\n")); */
			}
			else
			{
				/* Jeff: don't disable ieee8021x_blocked while clearing key */
				if (strcmp(param->u.crypt.alg, "none") != 0)
					psta->ieee8021x_blocked = false;

				if ((padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption2Enabled)||
						(padapter->securitypriv.ndisencryptstatus ==  Ndis802_11Encryption3Enabled))
				{
					psta->dot118021XPrivacy = padapter->securitypriv.dot11PrivacyAlgrthm;
				}

				if (param->u.crypt.set_tx ==1)/* pairwise key */
				{
					memcpy(psta->dot118021x_UncstKey.skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

					if (strcmp(param->u.crypt.alg, "TKIP") == 0)/* set mic key */
					{
						memcpy(psta->dot11tkiptxmickey.skey, &(param->u.crypt.key[16]), 8);
						memcpy(psta->dot11tkiprxmickey.skey, &(param->u.crypt.key[24]), 8);

						padapter->securitypriv.busetkipkey=false;
					}

					DBG_88E(" ~~~~set sta key:unicastkey\n");

					rtw_setstakey_cmd(padapter, (unsigned char *)psta, true, true);
				}
				else/* group key */
				{
					if (strcmp(param->u.crypt.alg, "TKIP") == 0 || strcmp(param->u.crypt.alg, "CCMP") == 0)
					{
						printk("[%s]  GTK key_len=%d @@@@@@@@@@@@@@@@@@\n", param->u.crypt.alg, param->u.crypt.key_len);
						memcpy(padapter->securitypriv.dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key,(param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
						/* only TKIP group key need to install this */
						if (param->u.crypt.key_len > 16)
						{
							memcpy(padapter->securitypriv.dot118021XGrptxmickey[param->u.crypt.idx].skey,&(param->u.crypt.key[16]),8);
							memcpy(padapter->securitypriv.dot118021XGrprxmickey[param->u.crypt.idx].skey,&(param->u.crypt.key[24]),8);
						}
						padapter->securitypriv.binstallGrpkey = true;
						DBG_88E(" ~~~~set sta key:groupkey\n");

						padapter->securitypriv.dot118021XGrpKeyid = param->u.crypt.idx;

						rtw_set_key(padapter,&padapter->securitypriv,param->u.crypt.idx, 1,true);
					}
#ifdef CONFIG_IEEE80211W
					else if (strcmp(param->u.crypt.alg, "BIP") == 0)
					{
						int no;
						/* save the IGTK key, length 16 bytes */
						memcpy(padapter->securitypriv.dot11wBIPKey[param->u.crypt.idx].skey,  param->u.crypt.key,(param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
						padapter->securitypriv.dot11wBIPKeyid = param->u.crypt.idx;
						padapter->securitypriv.binstallBIPkey = true;
						DBG_88E(" ~~~~set sta key:IGKT\n");
					}
#endif /* CONFIG_IEEE80211W */

#ifdef CONFIG_P2P
					if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_PROVISIONING_ING))
						rtw_p2p_set_state(pwdinfo, P2P_STATE_PROVISIONING_DONE);
#endif /* CONFIG_P2P */

				}
			}

			pbcmc_sta=rtw_get_bcmc_stainfo(padapter);
			if (pbcmc_sta== NULL)
			{
				/* DEBUG_ERR( ("Set OID_802_11_ADD_KEY: bcmc stainfo is null\n")); */
			}
			else
			{
				/* Jeff: don't disable ieee8021x_blocked while clearing key */
				if (strcmp(param->u.crypt.alg, "none") != 0)
					pbcmc_sta->ieee8021x_blocked = false;

				if ((padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption2Enabled)||
						(padapter->securitypriv.ndisencryptstatus ==  Ndis802_11Encryption3Enabled))
				{
					pbcmc_sta->dot118021XPrivacy = padapter->securitypriv.dot11PrivacyAlgrthm;
				}
			}
		}
	}

exit:

	if (pwep)
		rtw_mfree((u8 *)pwep, wep_total_len);
	return ret;
}

static int rtw_set_wpa_ie(struct adapter *padapter, char *pie, unsigned short ielen)
{
	u8 *buf= NULL, *pos= NULL;
	u32 left;
	int group_cipher = 0, pairwise_cipher = 0;
	int ret = 0;
	u8 null_addr[]= {0,0,0,0,0,0};
#ifdef CONFIG_P2P
	struct wifidirect_info* pwdinfo = &padapter->wdinfo;
#endif /* CONFIG_P2P */

	if ((ielen > MAX_WPA_IE_LEN) || (pie == NULL)) {
		_clr_fwstate_(&padapter->mlmepriv, WIFI_UNDER_WPS);
		if (pie == NULL)
			return ret;
		else
			return -EINVAL;
	}

	if (ielen) {
		buf = rtw_zmalloc(ielen);
		if (buf == NULL) {
			ret =  -ENOMEM;
			goto exit;
		}

		memcpy(buf, pie , ielen);

		/* dump */
		{
			int i;
			DBG_88E("\n wpa_ie(length:%d):\n", ielen);
			for (i=0;i<ielen;i=i+8)
				DBG_88E("0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n",buf[i],buf[i+1],buf[i+2],buf[i+3],buf[i+4],buf[i+5],buf[i+6],buf[i+7]);
		}

		pos = buf;
		if (ielen < RSN_HEADER_LEN) {
			RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("Ie len too short %d\n", ielen));
			ret  = -1;
			goto exit;
		}

		if (rtw_parse_wpa_ie(buf, ielen, &group_cipher, &pairwise_cipher, NULL) == _SUCCESS)
		{
			padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
			padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeWPAPSK;
			memcpy(padapter->securitypriv.supplicant_ie, &buf[0], ielen);
		}

		if (rtw_parse_wpa2_ie(buf, ielen, &group_cipher, &pairwise_cipher, NULL) == _SUCCESS)
		{
			padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
			padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeWPA2PSK;
			memcpy(padapter->securitypriv.supplicant_ie, &buf[0], ielen);
		}

		if (group_cipher == 0)
		{
			group_cipher = WPA_CIPHER_NONE;
		}
		if (pairwise_cipher == 0)
		{
			pairwise_cipher = WPA_CIPHER_NONE;
		}

		switch (group_cipher)
		{
			case WPA_CIPHER_NONE:
				padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
				padapter->securitypriv.ndisencryptstatus=Ndis802_11EncryptionDisabled;
				break;
			case WPA_CIPHER_WEP40:
				padapter->securitypriv.dot118021XGrpPrivacy=_WEP40_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
			case WPA_CIPHER_TKIP:
				padapter->securitypriv.dot118021XGrpPrivacy=_TKIP_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
				break;
			case WPA_CIPHER_CCMP:
				padapter->securitypriv.dot118021XGrpPrivacy=_AES_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
				break;
			case WPA_CIPHER_WEP104:
				padapter->securitypriv.dot118021XGrpPrivacy=_WEP104_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
		}

		switch (pairwise_cipher)
		{
			case WPA_CIPHER_NONE:
				padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
				padapter->securitypriv.ndisencryptstatus=Ndis802_11EncryptionDisabled;
				break;
			case WPA_CIPHER_WEP40:
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
			case WPA_CIPHER_TKIP:
				padapter->securitypriv.dot11PrivacyAlgrthm=_TKIP_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
				break;
			case WPA_CIPHER_CCMP:
				padapter->securitypriv.dot11PrivacyAlgrthm=_AES_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
				break;
			case WPA_CIPHER_WEP104:
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;
				padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
				break;
		}

		_clr_fwstate_(&padapter->mlmepriv, WIFI_UNDER_WPS);
		{/* set wps_ie */
			u16 cnt = 0;
			u8 eid, wps_oui[4]={0x0,0x50,0xf2,0x04};

			while ( cnt < ielen )
			{
				eid = buf[cnt];

				if ((eid==_VENDOR_SPECIFIC_IE_)&&(_rtw_memcmp(&buf[cnt+2], wps_oui, 4)==true))
				{
					DBG_88E("SET WPS_IE\n");

					padapter->securitypriv.wps_ie_len = ( (buf[cnt+1]+2) < (MAX_WPA_IE_LEN<<2)) ? (buf[cnt+1]+2):(MAX_WPA_IE_LEN<<2);

					memcpy(padapter->securitypriv.wps_ie, &buf[cnt], padapter->securitypriv.wps_ie_len);

					set_fwstate(&padapter->mlmepriv, WIFI_UNDER_WPS);

#ifdef CONFIG_P2P
					if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_GONEGO_OK))
					{
						rtw_p2p_set_state(pwdinfo, P2P_STATE_PROVISIONING_ING);
					}
#endif /* CONFIG_P2P */
					cnt += buf[cnt+1]+2;

					break;
				} else {
					cnt += buf[cnt+1]+2; /* goto next */
				}
			}
		}
	}

	/* TKIP and AES disallow multicast packets until installing group key */
        if (padapter->securitypriv.dot11PrivacyAlgrthm == _TKIP_
                || padapter->securitypriv.dot11PrivacyAlgrthm == _TKIP_WTMIC_
                || padapter->securitypriv.dot11PrivacyAlgrthm == _AES_)
                /* WPS open need to enable multicast */
                /*  check_fwstate(&padapter->mlmepriv, WIFI_UNDER_WPS) == true) */
                rtw_hal_set_hwreg(padapter, HW_VAR_OFF_RCR_AM, null_addr);

	RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
		 ("rtw_set_wpa_ie: pairwise_cipher=0x%08x padapter->securitypriv.ndisencryptstatus=%d padapter->securitypriv.ndisauthtype=%d\n",
		  pairwise_cipher, padapter->securitypriv.ndisencryptstatus, padapter->securitypriv.ndisauthtype));

exit:

	if (buf) rtw_mfree(buf, ielen);

	return ret;
}

static int rtw_wx_get_name(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	u16 cap;
	u32 ht_ielen = 0;
	char *p;
	u8 ht_cap=false;
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct wlan_bssid_ex  *pcur_bss = &pmlmepriv->cur_network.network;
	u8 *prates;

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("cmd_code=%x\n", info->cmd));
	if (check_fwstate(pmlmepriv, _FW_LINKED|WIFI_ADHOC_MASTER_STATE)) {
		/* parsing HT_CAP_IE */
		p = rtw_get_ie(&pcur_bss->IEs[12], _HT_CAPABILITY_IE_, &ht_ielen, pcur_bss->IELength-12);
		if (p && ht_ielen>0)
			ht_cap = true;

		prates = pcur_bss->SupportedRates;

		if (rtw_is_cckratesonly_included((u8*)prates)) {
			if (ht_cap == true)
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11bn");
			else
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11b");
		} else if ((rtw_is_cckrates_included((u8*)prates))) {
			if (ht_cap == true)
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11bgn");
			else
				snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11bg");
		} else {
			if (pcur_bss->Configuration.DSConfig > 14) {
				if (ht_cap == true)
					snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11an");
				else
					snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11a");
			} else {
				if (ht_cap == true)
					snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11gn");
				else
					snprintf(wrqu->name, IFNAMSIZ, "IEEE 802.11g");
			}
		}
	} else {
		snprintf(wrqu->name, IFNAMSIZ, "unassociated");
	}
	return 0;
}

static int rtw_wx_set_freq(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	;

	RT_TRACE(_module_rtl871x_mlme_c_, _drv_notice_, ("+rtw_wx_set_freq\n"));

	;

	return 0;
}

static int rtw_wx_get_freq(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct wlan_bssid_ex  *pcur_bss = &pmlmepriv->cur_network.network;

	if (check_fwstate(pmlmepriv, _FW_LINKED) == true)
	{
		/* wrqu->freq.m = ieee80211_wlan_frequencies[pcur_bss->Configuration.DSConfig-1] * 100000; */
		wrqu->freq.m = rtw_ch2freq(pcur_bss->Configuration.DSConfig) * 100000;
		wrqu->freq.e = 1;
		wrqu->freq.i = pcur_bss->Configuration.DSConfig;

	}
	else {
		wrqu->freq.m = rtw_ch2freq(padapter->mlmeextpriv.cur_channel) * 100000;
		wrqu->freq.e = 1;
		wrqu->freq.i = padapter->mlmeextpriv.cur_channel;
	}

	return 0;
}

static int rtw_wx_set_mode(struct net_device *dev, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	enum NDIS_802_11_NETWORK_INFRASTRUCTURE networkType ;
	int ret = 0;

	;

	if (_FAIL == rtw_pwr_wakeup(padapter)) {
		ret= -EPERM;
		goto exit;
	}

	if (padapter->hw_init_completed==false) {
		ret = -EPERM;
		goto exit;
	}

	switch (wrqu->mode)
	{
		case IW_MODE_AUTO:
			networkType = Ndis802_11AutoUnknown;
			DBG_88E("set_mode = IW_MODE_AUTO\n");
			break;
		case IW_MODE_ADHOC:
			networkType = Ndis802_11IBSS;
			DBG_88E("set_mode = IW_MODE_ADHOC\n");
			break;
		case IW_MODE_MASTER:
			networkType = Ndis802_11APMode;
			DBG_88E("set_mode = IW_MODE_MASTER\n");
                        /* rtw_setopmode_cmd(padapter, networkType,true); */
			break;
		case IW_MODE_INFRA:
			networkType = Ndis802_11Infrastructure;
			DBG_88E("set_mode = IW_MODE_INFRA\n");
			break;

		default :
			ret = -EINVAL;;
			RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("\n Mode: %s is not supported \n", iw_operation_mode[wrqu->mode]));
			goto exit;
	}

	if (rtw_set_802_11_infrastructure_mode(padapter, networkType) ==false) {

		ret = -EPERM;
		goto exit;

	}

	rtw_setopmode_cmd(padapter, networkType,true);

exit:

	;

	return ret;

}

static int rtw_wx_get_mode(struct net_device *dev, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *b)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,(" rtw_wx_get_mode\n"));

	if (check_fwstate(pmlmepriv, WIFI_STATION_STATE) == true)
		wrqu->mode = IW_MODE_INFRA;
	else if  ((check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == true) ||
		       (check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == true))
		wrqu->mode = IW_MODE_ADHOC;
	else if (check_fwstate(pmlmepriv, WIFI_AP_STATE) == true)
		wrqu->mode = IW_MODE_MASTER;
	else
		wrqu->mode = IW_MODE_AUTO;
	return 0;
}

static int rtw_wx_set_pmkid(struct net_device *dev,
	                     struct iw_request_info *a,
			     union iwreq_data *wrqu, char *extra)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	u8          j,blInserted = false;
	int         intReturn = false;
	struct mlme_priv  *pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
        struct iw_pmksa*  pPMK = ( struct iw_pmksa* ) extra;
        u8     strZeroMacAddress[ ETH_ALEN ] = { 0x00 };
        u8     strIssueBssid[ ETH_ALEN ] = { 0x00 };

	memcpy( strIssueBssid, pPMK->bssid.sa_data, ETH_ALEN);
        if ( pPMK->cmd == IW_PMKSA_ADD ) {
		DBG_88E( "[rtw_wx_set_pmkid] IW_PMKSA_ADD!\n" );
		if ( _rtw_memcmp( strIssueBssid, strZeroMacAddress, ETH_ALEN ) == true )
			return( intReturn );
		else
			intReturn = true;
		blInserted = false;

		/* overwrite PMKID */
		for (j=0 ; j<NUM_PMKID_CACHE; j++) {
			if ( _rtw_memcmp( psecuritypriv->PMKIDList[j].Bssid, strIssueBssid, ETH_ALEN) ==true )
			{ /*  BSSID is matched, the same AP => rewrite with new PMKID. */

                                DBG_88E( "[rtw_wx_set_pmkid] BSSID exists in the PMKList.\n" );

				memcpy( psecuritypriv->PMKIDList[j].PMKID, pPMK->pmkid, IW_PMKID_LEN);
                                psecuritypriv->PMKIDList[ j ].bUsed = true;
				psecuritypriv->PMKIDIndex = j+1;
				blInserted = true;
				break;
			}
	        }

	        if (!blInserted) {
		    /*  Find a new entry */
                    DBG_88E( "[rtw_wx_set_pmkid] Use the new entry index = %d for this PMKID.\n",
                            psecuritypriv->PMKIDIndex );

	            memcpy(psecuritypriv->PMKIDList[psecuritypriv->PMKIDIndex].Bssid, strIssueBssid, ETH_ALEN);
		    memcpy(psecuritypriv->PMKIDList[psecuritypriv->PMKIDIndex].PMKID, pPMK->pmkid, IW_PMKID_LEN);

                    psecuritypriv->PMKIDList[ psecuritypriv->PMKIDIndex ].bUsed = true;
		    psecuritypriv->PMKIDIndex++ ;
		    if (psecuritypriv->PMKIDIndex==16)
		        psecuritypriv->PMKIDIndex =0;
		}
        } else if ( pPMK->cmd == IW_PMKSA_REMOVE ) {
                DBG_88E( "[rtw_wx_set_pmkid] IW_PMKSA_REMOVE!\n" );
                intReturn = true;
		for (j=0 ; j<NUM_PMKID_CACHE; j++) {
			if ( _rtw_memcmp( psecuritypriv->PMKIDList[j].Bssid, strIssueBssid, ETH_ALEN) ==true )
			{ /*  BSSID is matched, the same AP => Remove this PMKID information and reset it. */
                                memset( psecuritypriv->PMKIDList[ j ].Bssid, 0x00, ETH_ALEN );
                                psecuritypriv->PMKIDList[ j ].bUsed = false;
				break;
			}
	        }
        } else if ( pPMK->cmd == IW_PMKSA_FLUSH ) {
            DBG_88E( "[rtw_wx_set_pmkid] IW_PMKSA_FLUSH!\n" );
            memset( &psecuritypriv->PMKIDList[ 0 ], 0x00, sizeof( RT_PMKID_LIST ) * NUM_PMKID_CACHE );
            psecuritypriv->PMKIDIndex = 0;
            intReturn = true;
        }
	return( intReturn );
}

static int rtw_wx_get_sens(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	wrqu->sens.value = 0;
	wrqu->sens.fixed = 0;	/* no auto select */
	wrqu->sens.disabled = 1;
	return 0;
}

static int rtw_wx_get_range(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	struct iw_range *range = (struct iw_range *)extra;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;

	u16 val;
	int i;

	;

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_range. cmd_code=%x\n", info->cmd));

	wrqu->data.length = sizeof(*range);
	memset(range, 0, sizeof(*range));

	/* Let's try to keep this struct in the same order as in
	 * linux/include/wireless.h
	 */

	/* TODO: See what values we can set, and remove the ones we can't
	 * set, or fill them with some default data.
	 */

	/* ~5 Mb/s real (802.11b) */
	range->throughput = 5 * 1000 * 1000;


	/* signal level threshold range */

	/* percent values between 0 and 100. */
	range->max_qual.qual = 100;
	range->max_qual.level = 100;
	range->max_qual.noise = 100;
	range->max_qual.updated = 7; /* Updated all three */


	range->avg_qual.qual = 92; /* > 8% missed beacons is 'bad' */
	/* TODO: Find real 'good' to 'bad' threshol value for RSSI */
	range->avg_qual.level = 178; /* equals -78 */
	range->avg_qual.noise = 0;
	range->avg_qual.updated = 7; /* Updated all three */

	range->num_bitrates = RATE_COUNT;

	for (i = 0; i < RATE_COUNT && i < IW_MAX_BITRATES; i++) {
		range->bitrate[i] = rtw_rates[i];
	}

	range->min_frag = MIN_FRAG_THRESHOLD;
	range->max_frag = MAX_FRAG_THRESHOLD;

	range->pm_capa = 0;

	range->we_version_compiled = WIRELESS_EXT;
	range->we_version_source = 16;

	for (i = 0, val = 0; i < MAX_CHANNEL_NUM; i++) {

		/*  Include only legal frequencies for some countries */
		if (pmlmeext->channel_set[i].ChannelNum != 0)
		{
			range->freq[val].i = pmlmeext->channel_set[i].ChannelNum;
			range->freq[val].m = rtw_ch2freq(pmlmeext->channel_set[i].ChannelNum) * 100000;
			range->freq[val].e = 1;
			val++;
		}

		if (val == IW_MAX_FREQUENCIES)
			break;
	}

	range->num_channels = val;
	range->num_frequency = val;

/*  Commented by Albert 2009/10/13 */
/*  The following code will proivde the security capability to network manager. */
/*  If the driver doesn't provide this capability to network manager, */
/*  the WPA/WPA2 routers can't be choosen in the network manager. */

/*
#define IW_SCAN_CAPA_NONE		0x00
#define IW_SCAN_CAPA_ESSID		0x01
#define IW_SCAN_CAPA_BSSID		0x02
#define IW_SCAN_CAPA_CHANNEL	0x04
#define IW_SCAN_CAPA_MODE		0x08
#define IW_SCAN_CAPA_RATE		0x10
#define IW_SCAN_CAPA_TYPE		0x20
#define IW_SCAN_CAPA_TIME		0x40
*/

#if WIRELESS_EXT > 17
	range->enc_capa = IW_ENC_CAPA_WPA|IW_ENC_CAPA_WPA2|
			  IW_ENC_CAPA_CIPHER_TKIP|IW_ENC_CAPA_CIPHER_CCMP;
#endif

#ifdef IW_SCAN_CAPA_ESSID /* WIRELESS_EXT > 21 */
	range->scan_capa = IW_SCAN_CAPA_ESSID | IW_SCAN_CAPA_TYPE |IW_SCAN_CAPA_BSSID|
					IW_SCAN_CAPA_CHANNEL|IW_SCAN_CAPA_MODE|IW_SCAN_CAPA_RATE;
#endif


	;

	return 0;

}

/* set bssid flow */
/* s1. rtw_set_802_11_infrastructure_mode() */
/* s2. rtw_set_802_11_authentication_mode() */
/* s3. set_802_11_encryption_mode() */
/* s4. rtw_set_802_11_bssid() */
static int rtw_wx_set_wap(struct net_device *dev,
			 struct iw_request_info *info,
			 union iwreq_data *awrq,
			 char *extra)
{
	unsigned long	irqL;
	uint ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct sockaddr *temp = (struct sockaddr *)awrq;
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct list_head *phead;
	u8 *dst_bssid, *src_bssid;
	struct  __queue	*queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network	*pnetwork = NULL;
	enum NDIS_802_11_AUTHENTICATION_MODE	authmode;

	if (_FAIL == rtw_pwr_wakeup(padapter))
	{
		ret= -1;
		goto exit;
	}

	if (!padapter->bup) {
		ret = -1;
		goto exit;
	}


	if (temp->sa_family != ARPHRD_ETHER) {
		ret = -EINVAL;
		goto exit;
	}

	authmode = padapter->securitypriv.ndisauthtype;
	spin_lock_bh(&queue->lock);
	phead = get_list_head(queue);
	pmlmepriv->pscanned = get_next(phead);

	while (1) {

		if ((rtw_end_of_queue_search(phead, pmlmepriv->pscanned)) == true)
			break;

		pnetwork = LIST_CONTAINOR(pmlmepriv->pscanned, struct wlan_network, list);

		pmlmepriv->pscanned = get_next(pmlmepriv->pscanned);

		dst_bssid = pnetwork->network.MacAddress;

		src_bssid = temp->sa_data;

		if ((_rtw_memcmp(dst_bssid, src_bssid, ETH_ALEN)) == true)
		{
			if (!rtw_set_802_11_infrastructure_mode(padapter, pnetwork->network.InfrastructureMode))
			{
				ret = -1;
				spin_unlock_bh(&queue->lock);
				goto exit;
			}

				break;
		}

	}
	spin_unlock_bh(&queue->lock);

	rtw_set_802_11_authentication_mode(padapter, authmode);
	/* set_802_11_encryption_mode(padapter, padapter->securitypriv.ndisencryptstatus); */
	if (rtw_set_802_11_bssid(padapter, temp->sa_data) == false) {
		ret = -1;
		goto exit;
	}

exit:

	;

	return ret;
}

static int rtw_wx_get_wap(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu, char *extra)
{

	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct wlan_bssid_ex  *pcur_bss = &pmlmepriv->cur_network.network;

	wrqu->ap_addr.sa_family = ARPHRD_ETHER;

	memset(wrqu->ap_addr.sa_data, 0, ETH_ALEN);

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_wap\n"));

	;

	if  ( ((check_fwstate(pmlmepriv, _FW_LINKED)) == true) ||
			((check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE)) == true) ||
			((check_fwstate(pmlmepriv, WIFI_AP_STATE)) == true) )
	{

		memcpy(wrqu->ap_addr.sa_data, pcur_bss->MacAddress, ETH_ALEN);
	}
	else
	{
		memset(wrqu->ap_addr.sa_data, 0, ETH_ALEN);
	}

	;

	return 0;

}

static int rtw_wx_set_mlme(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	int ret=0;
	u16 reason;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_mlme *mlme = (struct iw_mlme *) extra;


	if (mlme== NULL)
		return -1;

	DBG_88E("%s\n", __FUNCTION__);

	reason = mlme->reason_code;

	DBG_88E("%s, cmd=%d, reason=%d\n", __FUNCTION__, mlme->cmd, reason);

	switch (mlme->cmd) {
	case IW_MLME_DEAUTH:
		if (!rtw_set_802_11_disassociate(padapter))
			ret = -1;
		break;
	case IW_MLME_DISASSOC:
		if (!rtw_set_802_11_disassociate(padapter))
				ret = -1;
		break;
	default:
		return -EOPNOTSUPP;
	}

	return ret;
}

static int rtw_wx_set_scan(struct net_device *dev, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *extra)
{
	u8 _status = false;
	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv= &padapter->mlmepriv;
	struct ndis_802_11_ssid ssid[RTW_SSID_SCAN_AMOUNT];
	unsigned long	irqL;
#ifdef CONFIG_P2P
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
#endif /* CONFIG_P2P */
	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_set_scan\n"));

;

	#ifdef DBG_IOCTL
	DBG_88E("DBG_IOCTL %s:%d\n",__FUNCTION__, __LINE__);
	#endif
	if (_FAIL == rtw_pwr_wakeup(padapter))
	{
		ret= -1;
		goto exit;
	}

	if (padapter->bDriverStopped) {
		DBG_88E("bDriverStopped=%d\n", padapter->bDriverStopped);
		ret= -1;
		goto exit;
	}

	if (!padapter->bup) {
		ret = -1;
		goto exit;
	}

	if (padapter->hw_init_completed==false) {
		ret = -1;
		goto exit;
	}

	/*  When Busy Traffic, driver do not site survey. So driver return success. */
	/*  wpa_supplicant will not issue SIOCSIWSCAN cmd again after scan timeout. */
	/*  modify by thomas 2011-02-22. */
	if (pmlmepriv->LinkDetectInfo.bBusyTraffic == true) {
		indicate_wx_scan_complete_event(padapter);
		goto exit;
	}

	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY|_FW_UNDER_LINKING) == true)
	{
		indicate_wx_scan_complete_event(padapter);
		goto exit;
	}

#ifdef CONFIG_BT_COEXIST
	{
		u32 curr_time, delta_time;

		/*  under DHCP(Special packet) */
		curr_time = jiffies;
		delta_time = curr_time - adapter_to_pwrctl(padapter)->DelayLPSLastTimeStamp;
		delta_time = rtw_systime_to_ms(delta_time);
		if (delta_time < 500) /*  500ms */
		{
			DBG_88E("%s: send DHCP pkt before %d ms, Skip scan\n", __FUNCTION__, delta_time);
			ret = -1;
			goto exit;
		}
	}
#endif

/* 	Mareded by Albert 20101103 */
/* 	For the DMP WiFi Display project, the driver won't to scan because */
/* 	the pmlmepriv->scan_interval is always equal to 3. */
/* 	So, the wpa_supplicant won't find out the WPS SoftAP. */

#ifdef CONFIG_P2P
	if ( pwdinfo->p2p_state != P2P_STATE_NONE )
	{
		rtw_p2p_set_pre_state( pwdinfo, rtw_p2p_state( pwdinfo ) );
		rtw_p2p_set_state(pwdinfo, P2P_STATE_FIND_PHASE_SEARCH);
		rtw_p2p_findphase_ex_set(pwdinfo, P2P_FINDPHASE_EX_FULL);
		rtw_free_network_queue(padapter, true);
	}
#endif /* CONFIG_P2P */

	memset(ssid, 0, sizeof(struct ndis_802_11_ssid)*RTW_SSID_SCAN_AMOUNT);

#if WIRELESS_EXT >= 17
	if (wrqu->data.length == sizeof(struct iw_scan_req))
	{
		struct iw_scan_req *req = (struct iw_scan_req *)extra;

		if (wrqu->data.flags & IW_SCAN_THIS_ESSID)
		{
			int len = min((int)req->essid_len, IW_ESSID_MAX_SIZE);

			memcpy(ssid[0].Ssid, req->essid, len);
			ssid[0].SsidLength = len;

			DBG_88E("IW_SCAN_THIS_ESSID, ssid=%s, len=%d\n", req->essid, req->essid_len);

			spin_lock_bh(&pmlmepriv->lock);

			_status = rtw_sitesurvey_cmd(padapter, ssid, 1, NULL, 0);

			spin_unlock_bh(&pmlmepriv->lock);

		}
		else if (req->scan_type == IW_SCAN_TYPE_PASSIVE)
		{
			DBG_88E("rtw_wx_set_scan, req->scan_type == IW_SCAN_TYPE_PASSIVE\n");
		}

	}
	else
#endif

	if (	wrqu->data.length >= WEXT_CSCAN_HEADER_SIZE
		&& _rtw_memcmp(extra, WEXT_CSCAN_HEADER, WEXT_CSCAN_HEADER_SIZE) == true
	)
	{
		int len = wrqu->data.length -WEXT_CSCAN_HEADER_SIZE;
		char *pos = extra+WEXT_CSCAN_HEADER_SIZE;
		char section;
		char sec_len;
		int ssid_index = 0;

		while (len >= 1) {
			section = *(pos++); len-=1;

			switch (section) {
				case WEXT_CSCAN_SSID_SECTION:
					/* DBG_88E("WEXT_CSCAN_SSID_SECTION\n"); */
					if (len < 1) {
						len = 0;
						break;
					}

					sec_len = *(pos++); len-=1;

					if (sec_len>0 && sec_len<=len) {
						ssid[ssid_index].SsidLength = sec_len;
						memcpy(ssid[ssid_index].Ssid, pos, ssid[ssid_index].SsidLength);
						ssid_index++;
					}

					pos+=sec_len; len-=sec_len;
					break;


				case WEXT_CSCAN_CHANNEL_SECTION:
					pos+=1; len-=1;
					break;
				case WEXT_CSCAN_ACTV_DWELL_SECTION:
					pos+=2; len-=2;
					break;
				case WEXT_CSCAN_PASV_DWELL_SECTION:
					pos+=2; len-=2;
					break;
				case WEXT_CSCAN_HOME_DWELL_SECTION:
					pos+=2; len-=2;
					break;
				case WEXT_CSCAN_TYPE_SECTION:
					pos+=1; len-=1;
					break;
				default:
					len = 0; /*  stop parsing */
			}
		}

		/* jeff: it has still some scan paramater to parse, we only do this now... */
		_status = rtw_set_802_11_bssid_list_scan(padapter, ssid, RTW_SSID_SCAN_AMOUNT);

	} else

	{
		_status = rtw_set_802_11_bssid_list_scan(padapter, NULL, 0);
	}

	if (_status == false)
		ret = -1;

exit:
	#ifdef DBG_IOCTL
	DBG_88E("DBG_IOCTL %s:%d return %d\n",__FUNCTION__, __LINE__, ret);
	#endif

;

	return ret;
}

static int rtw_wx_get_scan(struct net_device *dev, struct iw_request_info *a,
			     union iwreq_data *wrqu, char *extra)
{
	unsigned long	irqL;
	struct list_head *plist, *phead;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct  __queue *queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network	*pnetwork = NULL;
	char *ev = extra;
	char *stop = ev + wrqu->data.length;
	u32 ret = 0;
	u32 cnt=0;
	u32 wait_for_surveydone;
	sint wait_status;
#ifdef CONFIG_P2P
	struct	wifidirect_info*	pwdinfo = &padapter->wdinfo;
#endif /* CONFIG_P2P */
	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_scan\n"));
	RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_, (" Start of Query SIOCGIWSCAN .\n"));

	;

	#ifdef DBG_IOCTL
	DBG_88E("DBG_IOCTL %s:%d\n",__FUNCTION__, __LINE__);
	#endif

	if (adapter_to_pwrctl(padapter)->brfoffbyhw && padapter->bDriverStopped)
	{
		ret = -EINVAL;
		goto exit;
	}

#ifdef CONFIG_P2P
	if (!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
	{
		/* 	P2P is enabled */
		if ( padapter->chip_type == RTL8192D )
			wait_for_surveydone = 300;	/* 	Because the 8192du supports more channels. */
		else
			wait_for_surveydone = 200;
	}
	else
	{
		/* 	P2P is disabled */
		wait_for_surveydone = 100;
	}
#else
	{
		wait_for_surveydone = 100;
	}
#endif /* CONFIG_P2P */

	wait_status = _FW_UNDER_SURVEY
#ifndef CONFIG_ANDROID
		| _FW_UNDER_LINKING
#endif
	;

	if (check_fwstate(pmlmepriv, wait_status))
		return -EAGAIN;

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1)
	{
		if (rtw_end_of_queue_search(phead,plist)== true)
			break;

		if ((stop - ev) < SCAN_ITEM_SIZE) {
			ret = -E2BIG;
			break;
		}

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

		/* report network only if the current channel set contains the channel to which this network belongs */
		if (rtw_ch_set_search_ch(padapter->mlmeextpriv.channel_set, pnetwork->network.Configuration.DSConfig) >= 0
			&& true == rtw_validate_ssid(&(pnetwork->network.Ssid))
		)
		{
			ev=translate_scan(padapter, a, pnetwork, ev, stop);
		}

		plist = get_next(plist);

	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

	wrqu->data.length = ev-extra;
	wrqu->data.flags = 0;

exit:

	;

	#ifdef DBG_IOCTL
	DBG_88E("DBG_IOCTL %s:%d return %d\n",__FUNCTION__, __LINE__, ret);
	#endif

	return ret ;

}

/* set ssid flow */
/* s1. rtw_set_802_11_infrastructure_mode() */
/* s2. set_802_11_authenticaion_mode() */
/* s3. set_802_11_encryption_mode() */
/* s4. rtw_set_802_11_ssid() */
static int rtw_wx_set_essid(struct net_device *dev,
			      struct iw_request_info *a,
			      union iwreq_data *wrqu, char *extra)
{
	unsigned long irqL;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct  __queue *queue = &pmlmepriv->scanned_queue;
	struct list_head *phead;
	s8 status = true;
	struct wlan_network *pnetwork = NULL;
	enum NDIS_802_11_AUTHENTICATION_MODE authmode;
	struct ndis_802_11_ssid ndis_ssid;
	u8 *dst_ssid, *src_ssid;

	uint ret = 0, len;

	;

	#ifdef DBG_IOCTL
	DBG_88E("DBG_IOCTL %s:%d\n",__FUNCTION__, __LINE__);
	#endif

	RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
		 ("+rtw_wx_set_essid: fw_state=0x%08x\n", get_fwstate(pmlmepriv)));
	if (_FAIL == rtw_pwr_wakeup(padapter))
	{
		ret = -1;
		goto exit;
	}

	if (!padapter->bup) {
		ret = -1;
		goto exit;
	}

#if WIRELESS_EXT <= 20
	if ((wrqu->essid.length-1) > IW_ESSID_MAX_SIZE) {
#else
	if (wrqu->essid.length > IW_ESSID_MAX_SIZE) {
#endif
		ret= -E2BIG;
		goto exit;
	}

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
		ret = -1;
		goto exit;
	}

	authmode = padapter->securitypriv.ndisauthtype;
	DBG_88E("=>%s\n",__FUNCTION__);
	if (wrqu->essid.flags && wrqu->essid.length)
	{
		/*  Commented by Albert 20100519 */
		/*  We got the codes in "set_info" function of iwconfig source code. */
		/* 	========================================= */
		/* 	wrq.u.essid.length = strlen(essid) + 1; */
		/* 	if (we_kernel_version > 20) */
		/* 		wrq.u.essid.length--; */
		/* 	========================================= */
		/* 	That means, if the WIRELESS_EXT less than or equal to 20, the correct ssid len should subtract 1. */
#if WIRELESS_EXT <= 20
		len = ((wrqu->essid.length-1) < IW_ESSID_MAX_SIZE) ? (wrqu->essid.length-1) : IW_ESSID_MAX_SIZE;
#else
		len = (wrqu->essid.length < IW_ESSID_MAX_SIZE) ? wrqu->essid.length : IW_ESSID_MAX_SIZE;
#endif

		if ( wrqu->essid.length != 33 )
			DBG_88E("ssid=%s, len=%d\n", extra, wrqu->essid.length);

		memset(&ndis_ssid, 0, sizeof(struct ndis_802_11_ssid));
		ndis_ssid.SsidLength = len;
		memcpy(ndis_ssid.Ssid, extra, len);
		src_ssid = ndis_ssid.Ssid;

		RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_, ("rtw_wx_set_essid: ssid=[%s]\n", src_ssid));
		spin_lock_bh(&queue->lock);
		phead = get_list_head(queue);
		pmlmepriv->pscanned = get_next(phead);

		while (1) {
			if (rtw_end_of_queue_search(phead, pmlmepriv->pscanned) == true) {
			        RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_warning_,
					 ("rtw_wx_set_essid: scan_q is empty, set ssid to check if scanning again!\n"));

				break;
			}

			pnetwork = LIST_CONTAINOR(pmlmepriv->pscanned, struct wlan_network, list);

			pmlmepriv->pscanned = get_next(pmlmepriv->pscanned);

			dst_ssid = pnetwork->network.Ssid.Ssid;

			RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
				 ("rtw_wx_set_essid: dst_ssid=%s\n",
				  pnetwork->network.Ssid.Ssid));

			if ((_rtw_memcmp(dst_ssid, src_ssid, ndis_ssid.SsidLength) == true) &&
				(pnetwork->network.Ssid.SsidLength==ndis_ssid.SsidLength))
			{
				RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
					 ("rtw_wx_set_essid: find match, set infra mode\n"));

				if (check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == true)
				{
					if (pnetwork->network.InfrastructureMode != pmlmepriv->cur_network.network.InfrastructureMode)
						continue;
				}

				if (rtw_set_802_11_infrastructure_mode(padapter, pnetwork->network.InfrastructureMode) == false)
				{
					ret = -1;
					spin_unlock_bh(&queue->lock);
					goto exit;
				}

				break;
			}
		}
		spin_unlock_bh(&queue->lock);
		RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
			 ("set ssid: set_802_11_auth. mode=%d\n", authmode));
		rtw_set_802_11_authentication_mode(padapter, authmode);
		if (rtw_set_802_11_ssid(padapter, &ndis_ssid) == false) {
			ret = -1;
			goto exit;
		}
	}

exit:

	DBG_88E("<=%s, ret %d\n",__FUNCTION__, ret);

	#ifdef DBG_IOCTL
	DBG_88E("DBG_IOCTL %s:%d return %d\n",__FUNCTION__, __LINE__, ret);
	#endif

	;

	return ret;
}

static int rtw_wx_get_essid(struct net_device *dev,
			      struct iw_request_info *a,
			      union iwreq_data *wrqu, char *extra)
{
	u32 len,ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct wlan_bssid_ex  *pcur_bss = &pmlmepriv->cur_network.network;

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,("rtw_wx_get_essid\n"));

	;

	if ( (check_fwstate(pmlmepriv, _FW_LINKED) == true) ||
	      (check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == true))
	{
		len = pcur_bss->Ssid.SsidLength;

		wrqu->essid.length = len;

		memcpy(extra, pcur_bss->Ssid.Ssid, len);

		wrqu->essid.flags = 1;
	}
	else
	{
		ret = -1;
		goto exit;
	}

exit:

	;

	return ret;

}

static int rtw_wx_set_rate(struct net_device *dev,
			      struct iw_request_info *a,
			      union iwreq_data *wrqu, char *extra)
{
	int	i, ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	u8	datarates[NumRates];
	u32	target_rate = wrqu->bitrate.value;
	u32	fixed = wrqu->bitrate.fixed;
	u32	ratevalue = 0;
	u8 mpdatarate[NumRates]={11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0, 0xff};

	RT_TRACE(_module_rtl871x_mlme_c_,_drv_info_,(" rtw_wx_set_rate\n"));
	RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("target_rate = %d, fixed = %d\n",target_rate,fixed));

	if (target_rate == -1) {
		ratevalue = 11;
		goto set_rate;
	}
	target_rate = target_rate/100000;

	switch (target_rate) {
		case 10:
			ratevalue = 0;
			break;
		case 20:
			ratevalue = 1;
			break;
		case 55:
			ratevalue = 2;
			break;
		case 60:
			ratevalue = 3;
			break;
		case 90:
			ratevalue = 4;
			break;
		case 110:
			ratevalue = 5;
			break;
		case 120:
			ratevalue = 6;
			break;
		case 180:
			ratevalue = 7;
			break;
		case 240:
			ratevalue = 8;
			break;
		case 360:
			ratevalue = 9;
			break;
		case 480:
			ratevalue = 10;
			break;
		case 540:
			ratevalue = 11;
			break;
		default:
			ratevalue = 11;
			break;
	}

set_rate:

	for (i=0; i<NumRates; i++)
	{
		if (ratevalue==mpdatarate[i])
		{
			datarates[i] = mpdatarate[i];
			if (fixed == 0)
				break;
		}
		else {
			datarates[i] = 0xff;
		}

		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("datarate_inx=%d\n",datarates[i]));
	}

	if ( rtw_setdatarate_cmd(padapter, datarates) !=_SUCCESS) {
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("rtw_wx_set_rate Fail!!!\n"));
		ret = -1;
	}

;

	return ret;
}

static int rtw_wx_get_rate(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	u16 max_rate = 0;

	max_rate = rtw_get_cur_max_rate((struct adapter *)rtw_netdev_priv(dev));

	if (max_rate == 0)
		return -EPERM;

	wrqu->bitrate.fixed = 0;	/* no auto select */
	wrqu->bitrate.value = max_rate * 100000;

	return 0;
}

static int rtw_wx_set_rts(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);

	;

	if (wrqu->rts.disabled)
		padapter->registrypriv.rts_thresh = 2347;
	else {
		if (wrqu->rts.value < 0 ||
		    wrqu->rts.value > 2347)
			return -EINVAL;

		padapter->registrypriv.rts_thresh = wrqu->rts.value;
	}

	DBG_88E("%s, rts_thresh=%d\n", __func__, padapter->registrypriv.rts_thresh);

	;

	return 0;

}

static int rtw_wx_get_rts(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);

	;

	DBG_88E("%s, rts_thresh=%d\n", __func__, padapter->registrypriv.rts_thresh);

	wrqu->rts.value = padapter->registrypriv.rts_thresh;
	wrqu->rts.fixed = 0;	/* no auto select */
	/* wrqu->rts.disabled = (wrqu->rts.value == DEFAULT_RTS_THRESHOLD); */
	return 0;
}

static int rtw_wx_set_frag(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);

	if (wrqu->frag.disabled)
		padapter->xmitpriv.frag_len = MAX_FRAG_THRESHOLD;
	else {
		if (wrqu->frag.value < MIN_FRAG_THRESHOLD ||
		    wrqu->frag.value > MAX_FRAG_THRESHOLD)
			return -EINVAL;

		padapter->xmitpriv.frag_len = wrqu->frag.value & ~0x1;
	}

	DBG_88E("%s, frag_len=%d\n", __func__, padapter->xmitpriv.frag_len);
	return 0;
}

static int rtw_wx_get_frag(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);

	;

	DBG_88E("%s, frag_len=%d\n", __func__, padapter->xmitpriv.frag_len);

	wrqu->frag.value = padapter->xmitpriv.frag_len;
	wrqu->frag.fixed = 0;	/* no auto select */
	return 0;
}

static int rtw_wx_get_retry(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	wrqu->retry.value = 7;
	wrqu->retry.fixed = 0;	/* no auto select */
	wrqu->retry.disabled = 1;

	return 0;
}

static int rtw_wx_set_enc(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu, char *keybuf)
{
	u32 key, ret = 0;
	u32 keyindex_provided;
	struct ndis_802_11_wep	 wep;
	enum NDIS_802_11_AUTHENTICATION_MODE authmode;

	struct iw_point *erq = &(wrqu->encoding);
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	DBG_88E("+rtw_wx_set_enc, flags=0x%x\n", erq->flags);

	memset(&wep, 0, sizeof(struct ndis_802_11_wep));

	key = erq->flags & IW_ENCODE_INDEX;

	if (erq->flags & IW_ENCODE_DISABLED) {
		DBG_88E("EncryptionDisabled\n");
		padapter->securitypriv.ndisencryptstatus = Ndis802_11EncryptionDisabled;
		padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
		padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open; /* open system */
		authmode = Ndis802_11AuthModeOpen;
		padapter->securitypriv.ndisauthtype=authmode;

		goto exit;
	}

	if (key) {
		if (key > WEP_KEYS)
			return -EINVAL;
		key--;
		keyindex_provided = 1;
	} else {
		keyindex_provided = 0;
		key = padapter->securitypriv.dot11PrivacyKeyIndex;
		DBG_88E("rtw_wx_set_enc, key=%d\n", key);
	}

	/* set authentication mode */
	if (erq->flags & IW_ENCODE_OPEN) {
		DBG_88E("rtw_wx_set_enc():IW_ENCODE_OPEN\n");
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;/* Ndis802_11EncryptionDisabled; */

		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open;

		padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
		padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
		authmode = Ndis802_11AuthModeOpen;
		padapter->securitypriv.ndisauthtype=authmode;
	} else if (erq->flags & IW_ENCODE_RESTRICTED) {
		DBG_88E("rtw_wx_set_enc():IW_ENCODE_RESTRICTED\n");
		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;

		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Shared;

		padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
		padapter->securitypriv.dot118021XGrpPrivacy=_WEP40_;
		authmode = Ndis802_11AuthModeShared;
		padapter->securitypriv.ndisauthtype=authmode;
	} else {
		DBG_88E("rtw_wx_set_enc():erq->flags=0x%x\n", erq->flags);

		padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;/* Ndis802_11EncryptionDisabled; */
		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open; /* open system */
		padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
		padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
		authmode = Ndis802_11AuthModeOpen;
		padapter->securitypriv.ndisauthtype=authmode;
	}

	wep.KeyIndex = key;
	if (erq->length > 0) {
		wep.KeyLength = erq->length <= 5 ? 5 : 13;

		wep.Length = wep.KeyLength + FIELD_OFFSET(struct ndis_802_11_wep, KeyMaterial);
	} else {
		wep.KeyLength = 0 ;

		if (keyindex_provided == 1)/*  set key_id only, no given KeyMaterial(erq->length== 0). */
		{
			padapter->securitypriv.dot11PrivacyKeyIndex = key;

			DBG_88E("(keyindex_provided == 1), keyid=%d, key_len=%d\n", key, padapter->securitypriv.dot11DefKeylen[key]);

			switch (padapter->securitypriv.dot11DefKeylen[key]) {
			case 5:
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
				break;
			case 13:
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;
				break;
			default:
				padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
				break;
			}
			goto exit;
		}
	}
	wep.KeyIndex |= 0x80000000;

	memcpy(wep.KeyMaterial, keybuf, wep.KeyLength);

	if (rtw_set_802_11_add_wep(padapter, &wep) == false) {
		if (rf_on == pwrpriv->rf_pwrstate )
			ret = -EOPNOTSUPP;
		goto exit;
	}

exit:

	return ret;
}

static int rtw_wx_get_enc(struct net_device *dev,
			    struct iw_request_info *info,
			    union iwreq_data *wrqu, char *keybuf)
{
	uint key, ret =0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *erq = &(wrqu->encoding);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

	;

	if (check_fwstate(pmlmepriv, _FW_LINKED) != true)
	{
		 if (check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) != true)
		 {
		erq->length = 0;
		erq->flags |= IW_ENCODE_DISABLED;
		return 0;
	}
	}


	key = erq->flags & IW_ENCODE_INDEX;

	if (key) {
		if (key > WEP_KEYS)
			return -EINVAL;
		key--;
	} else
	{
		key = padapter->securitypriv.dot11PrivacyKeyIndex;
	}

	erq->flags = key + 1;

	switch (padapter->securitypriv.ndisencryptstatus) {
	case Ndis802_11EncryptionNotSupported:
	case Ndis802_11EncryptionDisabled:
		erq->length = 0;
		erq->flags |= IW_ENCODE_DISABLED;
		break;
	case Ndis802_11Encryption1Enabled:
		erq->length = padapter->securitypriv.dot11DefKeylen[key];
		if (erq->length) {
			memcpy(keybuf, padapter->securitypriv.dot11DefKey[key].skey, padapter->securitypriv.dot11DefKeylen[key]);

			erq->flags |= IW_ENCODE_ENABLED;

			if (padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeOpen)
				erq->flags |= IW_ENCODE_OPEN;
			else if (padapter->securitypriv.ndisauthtype == Ndis802_11AuthModeShared)
				erq->flags |= IW_ENCODE_RESTRICTED;
		} else {
			erq->length = 0;
			erq->flags |= IW_ENCODE_DISABLED;
		}
		break;
	case Ndis802_11Encryption2Enabled:
	case Ndis802_11Encryption3Enabled:
		erq->length = 16;
		erq->flags |= (IW_ENCODE_ENABLED | IW_ENCODE_OPEN | IW_ENCODE_NOKEY);
		break;
	default:
		erq->length = 0;
		erq->flags |= IW_ENCODE_DISABLED;
		break;
	}
	return ret;
}

static int rtw_wx_get_power(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	wrqu->power.value = 0;
	wrqu->power.fixed = 0;	/* no auto select */
	wrqu->power.disabled = 1;

	return 0;
}

static int rtw_wx_set_gen_ie(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);

	return rtw_set_wpa_ie(padapter, extra, wrqu->data.length);
}

static int rtw_wx_set_auth(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_param *param = (struct iw_param*)&(wrqu->param);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u32 value = param->value;
	int ret = 0;

	switch (param->flags & IW_AUTH_INDEX) {

	case IW_AUTH_WPA_VERSION:
		break;
	case IW_AUTH_CIPHER_PAIRWISE:

		break;
	case IW_AUTH_CIPHER_GROUP:

		break;
	case IW_AUTH_KEY_MGMT:
		/*
		 *  ??? does not use these parameters
		 */
		break;

	case IW_AUTH_TKIP_COUNTERMEASURES:
        {
	    if ( param->value )
            {  /*  wpa_supplicant is enabling the tkip countermeasure. */
               padapter->securitypriv.btkip_countermeasure = true;
            }
            else
            {  /*  wpa_supplicant is disabling the tkip countermeasure. */
               padapter->securitypriv.btkip_countermeasure = false;
            }
		break;
        }
	case IW_AUTH_DROP_UNENCRYPTED:
		{
			/* HACK:
			 *
			 * wpa_supplicant calls set_wpa_enabled when the driver
			 * is loaded and unloaded, regardless of if WPA is being
			 * used.  No other calls are made which can be used to
			 * determine if encryption will be used or not prior to
			 * association being expected.  If encryption is not being
			 * used, drop_unencrypted is set to false, else true -- we
			 * can use this to determine if the CAP_PRIVACY_ON bit should
			 * be set.
			 */

			if (padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption1Enabled)
			{
				break;/* it means init value, or using wep, ndisencryptstatus = Ndis802_11Encryption1Enabled, */
						/*  then it needn't reset it; */
			}

			if (param->value) {
				padapter->securitypriv.ndisencryptstatus = Ndis802_11EncryptionDisabled;
				padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
				padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
				padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_Open; /* open system */
				padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeOpen;
			}

			break;
		}

	case IW_AUTH_80211_AUTH_ALG:

		#if defined(CONFIG_ANDROID) || 1
		/*
		 *  It's the starting point of a link layer connection using wpa_supplicant
		*/
		if (check_fwstate(&padapter->mlmepriv, _FW_LINKED)) {
			LeaveAllPowerSaveMode(padapter);
			rtw_disassoc_cmd(padapter, 500, false);
			DBG_88E("%s...call rtw_indicate_disconnect\n ",__FUNCTION__);
			rtw_indicate_disconnect(padapter);
			rtw_free_assoc_resources(padapter, 1);
		}
		#endif


		ret = wpa_set_auth_algs(dev, (u32)param->value);

		break;

	case IW_AUTH_WPA_ENABLED:
		break;
	case IW_AUTH_RX_UNENCRYPTED_EAPOL:
		break;
	case IW_AUTH_PRIVACY_INVOKED:
		break;
	default:
		return -EOPNOTSUPP;
	}
	return ret;
}

static int rtw_wx_set_enc_ext(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	char *alg_name;
	u32 param_len;
	struct ieee_param *param = NULL;
	struct iw_point *pencoding = &wrqu->encoding;
	struct iw_encode_ext *pext = (struct iw_encode_ext *)extra;
	int ret=0;

	param_len = sizeof(struct ieee_param) + pext->key_len;
	param = (struct ieee_param *)rtw_malloc(param_len);
	if (param == NULL)
		return -1;

	memset(param, 0, param_len);

	param->cmd = IEEE_CMD_SET_ENCRYPTION;
	memset(param->sta_addr, 0xff, ETH_ALEN);


	switch (pext->alg) {
	case IW_ENCODE_ALG_NONE:
		/* todo: remove key */
		/* remove = 1; */
		alg_name = "none";
		break;
	case IW_ENCODE_ALG_WEP:
		alg_name = "WEP";
		break;
	case IW_ENCODE_ALG_TKIP:
		alg_name = "TKIP";
		break;
	case IW_ENCODE_ALG_CCMP:
		alg_name = "CCMP";
		break;
#ifdef CONFIG_IEEE80211W
	case IW_ENCODE_ALG_AES_CMAC:
		alg_name = "BIP";
		break;
#endif /* CONFIG_IEEE80211W */
	default:
		return -1;
	}

	strncpy((char *)param->u.crypt.alg, alg_name, IEEE_CRYPT_ALG_NAME_LEN);

	if (pext->ext_flags & IW_ENCODE_EXT_SET_TX_KEY)
	{
		param->u.crypt.set_tx = 1;
	}

	/* cliW: WEP does not have group key
	 * just not checking GROUP key setting
	 */
	if ((pext->alg != IW_ENCODE_ALG_WEP) &&
		((pext->ext_flags & IW_ENCODE_EXT_GROUP_KEY)
#ifdef CONFIG_IEEE80211W
		|| (pext->ext_flags & IW_ENCODE_ALG_AES_CMAC)
#endif /* CONFIG_IEEE80211W */
	))
	{
		param->u.crypt.set_tx = 0;
	}

	param->u.crypt.idx = (pencoding->flags&0x00FF) -1 ;

	if (pext->ext_flags & IW_ENCODE_EXT_RX_SEQ_VALID)
		memcpy(param->u.crypt.seq, pext->rx_seq, 8);

	if (pext->key_len) {
		param->u.crypt.key_len = pext->key_len;
		/* memcpy(param + 1, pext + 1, pext->key_len); */
		memcpy(param->u.crypt.key, pext + 1, pext->key_len);
	}

	if (pencoding->flags & IW_ENCODE_DISABLED)
	{
		/* todo: remove key */
		/* remove = 1; */
	}

	ret =  wpa_set_encryption(dev, param, param_len);

	if (param)
	{
		rtw_mfree((u8*)param, param_len);
	}

	return ret;
}


static int rtw_wx_get_nick(struct net_device *dev,
			     struct iw_request_info *info,
			     union iwreq_data *wrqu, char *extra)
{
	if (extra) {
		wrqu->data.length = 14;
		wrqu->data.flags = 1;
		memcpy(extra, "<WIFI@REALTEK>", 14);
	}
	return 0;
}

static int rtw_wx_read32(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	struct adapter *padapter;
	struct iw_point *p;
	u16 len;
	u32 addr;
	u32 data32;
	u32 bytes;
	u8 *ptmp;


	padapter = (struct adapter *)rtw_netdev_priv(dev);
	p = &wrqu->data;
	len = p->length;
	ptmp = (u8*)rtw_malloc(len);
	if (NULL == ptmp)
		return -ENOMEM;

	if (copy_from_user(ptmp, p->pointer, len)) {
		rtw_mfree(ptmp, len);
		return -EFAULT;
	}

	bytes = 0;
	addr = 0;
	sscanf(ptmp, "%d,%x", &bytes, &addr);

	switch (bytes) {
		case 1:
			data32 = rtw_read8(padapter, addr);
			sprintf(extra, "0x%02X", data32);
			break;
		case 2:
			data32 = rtw_read16(padapter, addr);
			sprintf(extra, "0x%04X", data32);
			break;
		case 4:
			data32 = rtw_read32(padapter, addr);
			sprintf(extra, "0x%08X", data32);
			break;
		default:
			DBG_88E(KERN_INFO "%s: usage> read [bytes],[address(hex)]\n", __func__);
			return -EINVAL;
	}
	DBG_88E(KERN_INFO "%s: addr=0x%08X data=%s\n", __func__, addr, extra);

	rtw_mfree(ptmp, len);

	return 0;
}

static int rtw_wx_write32(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);

	u32 addr;
	u32 data32;
	u32 bytes;


	bytes = 0;
	addr = 0;
	data32 = 0;
	sscanf(extra, "%d,%x,%x", &bytes, &addr, &data32);

	switch (bytes) {
		case 1:
			rtw_write8(padapter, addr, (u8)data32);
			DBG_88E(KERN_INFO "%s: addr=0x%08X data=0x%02X\n", __func__, addr, (u8)data32);
			break;
		case 2:
			rtw_write16(padapter, addr, (u16)data32);
			DBG_88E(KERN_INFO "%s: addr=0x%08X data=0x%04X\n", __func__, addr, (u16)data32);
			break;
		case 4:
			rtw_write32(padapter, addr, data32);
			DBG_88E(KERN_INFO "%s: addr=0x%08X data=0x%08X\n", __func__, addr, data32);
			break;
		default:
			DBG_88E(KERN_INFO "%s: usage> write [bytes],[address(hex)],[data(hex)]\n", __func__);
			return -EINVAL;
	}

	return 0;
}

static int rtw_wx_read_rf(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	u32 path, addr, data32;


	path = *(u32*)extra;
	addr = *((u32*)extra + 1);
	data32 = rtw_hal_read_rfreg(padapter, path, addr, 0xFFFFF);
/* 	DBG_88E("%s: path=%d addr=0x%02x data=0x%05x\n", __func__, path, addr, data32); */
	/*
	 * IMPORTANT!!
	 * Only when wireless private ioctl is at odd order,
	 * "extra" would be copied to user space.
	 */
	sprintf(extra, "0x%05x", data32);

	return 0;
}

static int rtw_wx_write_rf(struct net_device *dev,
                            struct iw_request_info *info,
                            union iwreq_data *wrqu, char *extra)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	u32 path, addr, data32;


	path = *(u32*)extra;
	addr = *((u32*)extra + 1);
	data32 = *((u32*)extra + 2);
/* 	DBG_88E("%s: path=%d addr=0x%02x data=0x%05x\n", __func__, path, addr, data32); */
	rtw_hal_write_rfreg(padapter, path, addr, 0xFFFFF, data32);

	return 0;
}

static int rtw_wx_priv_null(struct net_device *dev, struct iw_request_info *a,
		 union iwreq_data *wrqu, char *b)
{
	return -1;
}

static int dummy(struct net_device *dev, struct iw_request_info *a,
		 union iwreq_data *wrqu, char *b)
{
	return -1;
}

static int rtw_wx_set_channel_plan(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	extern int rtw_channel_plan;
	u8 channel_plan_req = (u8) (*((int *)wrqu));

	if ( _SUCCESS == rtw_set_chplan_cmd(padapter, channel_plan_req, 1) )
		DBG_88E("%s set channel_plan = 0x%02X\n", __func__, pmlmepriv->ChannelPlan);
	else
		return -EPERM;
	return 0;
}

static int rtw_wx_set_mtk_wps_probe_ie(struct net_device *dev,
		struct iw_request_info *a,
		union iwreq_data *wrqu, char *b)
{
	return 0;
}

static int rtw_wx_get_sensitivity(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *buf)
{
	return 0;
}

static int rtw_wx_set_mtk_wps_ie(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	return 0;
}

/*
 *	For all data larger than 16 octets, we need to use a
 *	pointer to memory allocated in user space.
 */
static  int rtw_drvext_hdl(struct net_device *dev, struct iw_request_info *info,
						union iwreq_data *wrqu, char *extra)
{
#ifdef CONFIG_DRVEXT_MODULE
	u8 res;
	struct drvext_handler *phandler;
	struct drvext_oidparam *poidparam;
	int ret;
	u16 len;
	u8 *pparmbuf, bset;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *p = &wrqu->data;

	if ( (!p->length) || (!p->pointer)) {
		ret = -EINVAL;
		goto _rtw_drvext_hdl_exit;
	}


	bset = (u8)(p->flags&0xFFFF);
	len = p->length;
	pparmbuf = (u8*)rtw_malloc(len);
	if (pparmbuf == NULL) {
		ret = -ENOMEM;
		goto _rtw_drvext_hdl_exit;
	}

	if (bset)/* set info */
	{
		if (copy_from_user(pparmbuf, p->pointer,len)) {
			rtw_mfree(pparmbuf, len);
			ret = -EFAULT;
			goto _rtw_drvext_hdl_exit;
		}
	}
	else/* query info */
	{

	}


	/*  */
	poidparam = (struct drvext_oidparam *)pparmbuf;

	RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("drvext set oid subcode [%d], len[%d], InformationBufferLength[%d]\r\n",
						 poidparam->subcode, poidparam->len, len));


	/* check subcode */
	if ( poidparam->subcode >= MAX_DRVEXT_HANDLERS)
	{
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("no matching drvext handlers\r\n"));
		ret = -EINVAL;
		goto _rtw_drvext_hdl_exit;
	}


	if ( poidparam->subcode >= MAX_DRVEXT_OID_SUBCODES)
	{
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("no matching drvext subcodes\r\n"));
		ret = -EINVAL;
		goto _rtw_drvext_hdl_exit;
	}


	phandler = drvextoidhandlers + poidparam->subcode;

	if (poidparam->len != phandler->parmsize)
	{
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("no matching drvext param size %d vs %d\r\n",
						poidparam->len , phandler->parmsize));
		ret = -EINVAL;
		goto _rtw_drvext_hdl_exit;
	}


	res = phandler->handler(&padapter->drvextpriv, bset, poidparam->data);

	if (res== 0)
	{
		ret = 0;

		if (bset == 0x00) {/* query info */
			/* memcpy(p->pointer, pparmbuf, len); */
			if (copy_to_user(p->pointer, pparmbuf, len))
				ret = -EFAULT;
		}
	}
	else
		ret = -EFAULT;


_rtw_drvext_hdl_exit:

	return ret;

#endif

	return 0;

}

static void rtw_dbg_mode_hdl(struct adapter *padapter, u32 id, u8 *pdata, u32 len)
{
	struct mp_rw_reg *		RegRWStruct;
	struct rf_reg_param *prfreg;
	u8 path;
	u8 offset;
	u32 value;

	DBG_88E("%s\n", __FUNCTION__);

	switch (id)
	{
		case GEN_MP_IOCTL_SUBCODE(MP_START):
			DBG_88E("871x_driver is only for normal mode, can't enter mp mode\n");
			break;
		case GEN_MP_IOCTL_SUBCODE(READ_REG):
			RegRWStruct = (struct mp_rw_reg *)pdata;
			switch (RegRWStruct->width)
			{
				case 1:
					RegRWStruct->value = rtw_read8(padapter, RegRWStruct->offset);
					break;
				case 2:
					RegRWStruct->value = rtw_read16(padapter, RegRWStruct->offset);
					break;
				case 4:
					RegRWStruct->value = rtw_read32(padapter, RegRWStruct->offset);
					break;
				default:
					break;
			}

			break;
		case GEN_MP_IOCTL_SUBCODE(WRITE_REG):
			RegRWStruct = (struct mp_rw_reg *)pdata;
			switch (RegRWStruct->width)
			{
				case 1:
					rtw_write8(padapter, RegRWStruct->offset, (u8)RegRWStruct->value);
					break;
				case 2:
					rtw_write16(padapter, RegRWStruct->offset, (u16)RegRWStruct->value);
					break;
				case 4:
					rtw_write32(padapter, RegRWStruct->offset, (u32)RegRWStruct->value);
					break;
				default:
				break;
			}

			break;
		case GEN_MP_IOCTL_SUBCODE(READ_RF_REG):

			prfreg = (struct rf_reg_param *)pdata;

			path = (u8)prfreg->path;
			offset = (u8)prfreg->offset;

			value = rtw_hal_read_rfreg(padapter, path, offset, 0xffffffff);

			prfreg->value = value;

			break;
		case GEN_MP_IOCTL_SUBCODE(WRITE_RF_REG):

			prfreg = (struct rf_reg_param *)pdata;

			path = (u8)prfreg->path;
			offset = (u8)prfreg->offset;
			value = prfreg->value;

			rtw_hal_write_rfreg(padapter, path, offset, 0xffffffff, value);

			break;
                case GEN_MP_IOCTL_SUBCODE(TRIGGER_GPIO):
			DBG_88E("==> trigger gpio 0\n");
			rtw_hal_set_hwreg(padapter, HW_VAR_TRIGGER_GPIO_0, NULL);
			break;
#ifdef CONFIG_BT_COEXIST
		case GEN_MP_IOCTL_SUBCODE(SET_DM_BT):
			DBG_88E("==> set dm_bt_coexist:%x\n",*(u8 *)pdata);
			rtw_hal_set_hwreg(padapter, HW_VAR_BT_SET_COEXIST, pdata);
			break;
		case GEN_MP_IOCTL_SUBCODE(DEL_BA):
			DBG_88E("==> delete ba:%x\n",*(u8 *)pdata);
			rtw_hal_set_hwreg(padapter, HW_VAR_BT_ISSUE_DELBA, pdata);
			break;
#endif
		case GEN_MP_IOCTL_SUBCODE(GET_WIFI_STATUS):
			*pdata = rtw_hal_sreset_get_wifi_status(padapter);
			break;
		default:
			break;
	}
}

static int rtw_mp_ioctl_hdl(struct net_device *dev, struct iw_request_info *info,
						union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	u32 BytesRead, BytesWritten, BytesNeeded;
	struct oid_par_priv	oid_par;
	struct mp_ioctl_handler	*phandler;
	struct mp_ioctl_param	*poidparam;
	uint status=0;
	u16 len;
	u8 *pparmbuf = NULL, bset;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *p = &wrqu->data;

	/* DBG_88E("+rtw_mp_ioctl_hdl\n"); */

	/* mutex_lock(&ioctl_mutex); */

	if ((!p->length) || (!p->pointer)) {
		ret = -EINVAL;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	pparmbuf = NULL;
	bset = (u8)(p->flags & 0xFFFF);
	len = p->length;
	pparmbuf = (u8*)rtw_malloc(len);
	if (pparmbuf == NULL) {
		ret = -ENOMEM;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	if (copy_from_user(pparmbuf, p->pointer, len)) {
		ret = -EFAULT;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	poidparam = (struct mp_ioctl_param *)pparmbuf;
	RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
		 ("rtw_mp_ioctl_hdl: subcode [%d], len[%d], buffer_len[%d]\r\n",
		  poidparam->subcode, poidparam->len, len));

	if (poidparam->subcode >= MAX_MP_IOCTL_SUBCODE) {
		RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_err_, ("no matching drvext subcodes\r\n"));
		ret = -EINVAL;
		goto _rtw_mp_ioctl_hdl_exit;
	}

	/* DBG_88E("%s: %d\n", __func__, poidparam->subcode); */
	rtw_dbg_mode_hdl(padapter, poidparam->subcode, poidparam->data, poidparam->len);

	if (bset == 0x00) {/* query info */
		if (copy_to_user(p->pointer, pparmbuf, len))
			ret = -EFAULT;
	}

	if (status) {
		ret = -EFAULT;
		goto _rtw_mp_ioctl_hdl_exit;
	}

_rtw_mp_ioctl_hdl_exit:

	if (pparmbuf)
		rtw_mfree(pparmbuf, len);

	/* mutex_unlock(&ioctl_mutex); */

	return ret;
}

static int rtw_get_ap_info(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	int bssid_match, ret = 0;
	u32 cnt=0, wpa_ielen;
	unsigned long	irqL;
	struct list_head *plist, *phead;
	unsigned char *pbuf;
	u8 bssid[ETH_ALEN];
	char data[32];
	struct wlan_network *pnetwork = NULL;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct  __queue *queue = &(pmlmepriv->scanned_queue);
	struct iw_point *pdata = &wrqu->data;

	DBG_88E("+rtw_get_aplist_info\n");

	if ((padapter->bDriverStopped) || (pdata== NULL))
	{
		ret= -EINVAL;
		goto exit;
	}

	while ((check_fwstate(pmlmepriv, (_FW_UNDER_SURVEY|_FW_UNDER_LINKING))) == true)
	{
		rtw_msleep_os(30);
		cnt++;
		if (cnt > 100)
			break;
	}


	/* pdata->length = 0; */
	pdata->flags = 0;
	if (pdata->length>=32)
	{
		if (copy_from_user(data, pdata->pointer, 32))
		{
			ret= -EINVAL;
			goto exit;
		}
	}
	else
	{
		ret= -EINVAL;
		goto exit;
	}

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1)
	{
		if (rtw_end_of_queue_search(phead,plist)== true)
			break;


		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

		/* if (hwaddr_aton_i(pdata->pointer, bssid)) */
		if (hwaddr_aton_i(data, bssid))
		{
			DBG_88E("Invalid BSSID '%s'.\n", (u8*)data);
			spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));
			return -EINVAL;
		}


		if (_rtw_memcmp(bssid, pnetwork->network.MacAddress, ETH_ALEN) == true)/* BSSID match, then check if supporting wpa/wpa2 */
		{
			DBG_88E("BSSID:" MAC_FMT "\n", MAC_ARG(bssid));

			pbuf = rtw_get_wpa_ie(&pnetwork->network.IEs[12], &wpa_ielen, pnetwork->network.IELength-12);
			if (pbuf && (wpa_ielen>0))
			{
				pdata->flags = 1;
				break;
			}

			pbuf = rtw_get_wpa2_ie(&pnetwork->network.IEs[12], &wpa_ielen, pnetwork->network.IELength-12);
			if (pbuf && (wpa_ielen>0))
			{
				pdata->flags = 2;
				break;
			}

		}

		plist = get_next(plist);

	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

	if (pdata->length>=34) {
		if (copy_to_user((u8 __user *)pdata->pointer+32, (u8*)&pdata->flags, 1)) {
			ret= -EINVAL;
			goto exit;
		}
	}

exit:

	return ret;

}

static int rtw_set_pid(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter *padapter = rtw_netdev_priv(dev);
	int *pdata = (int *)wrqu;
	int selector;

	if ((padapter->bDriverStopped) || (pdata== NULL))
	{
		ret= -EINVAL;
		goto exit;
	}

	selector = *pdata;
	if (selector < 3 && selector >=0) {
		padapter->pid[selector] = *(pdata+1);
		ui_pid[selector] = *(pdata+1);
		DBG_88E("%s set pid[%d]=%d\n", __FUNCTION__, selector ,padapter->pid[selector]);
	} else {
		DBG_88E("%s selector %d error\n", __FUNCTION__, selector);
	}
exit:

	return ret;

}

static int rtw_wps_start(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	u32   u32wps_start = 0;
        unsigned int uintRet = 0;

        uintRet = copy_from_user( ( void* ) &u32wps_start, pdata->pointer, 4 );

	if ((padapter->bDriverStopped) || (pdata== NULL))
	{
		ret= -EINVAL;
		goto exit;
	}

	if ( u32wps_start == 0 )
	{
		u32wps_start = *extra;
	}

	DBG_88E( "[%s] wps_start = %d\n", __FUNCTION__, u32wps_start );

	if ( u32wps_start == 1 ) /*  WPS Start */
	{
		rtw_led_control(padapter, LED_CTL_START_WPS);
	}
	else if ( u32wps_start == 2 ) /*  WPS Stop because of wps success */
	{
		rtw_led_control(padapter, LED_CTL_STOP_WPS);
	}
	else if ( u32wps_start == 3 ) /*  WPS Stop because of wps fail */
	{
		rtw_led_control(padapter, LED_CTL_STOP_WPS_FAIL);
	}

exit:

	return ret;
}

#ifdef CONFIG_P2P
static int rtw_wext_p2p_enable(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	enum P2P_ROLE init_role = P2P_ROLE_DISABLE;

	if (*extra == '0' )
		init_role = P2P_ROLE_DISABLE;
	else if (*extra == '1')
		init_role = P2P_ROLE_DEVICE;
	else if (*extra == '2')
		init_role = P2P_ROLE_CLIENT;
	else if (*extra == '3')
		init_role = P2P_ROLE_GO;

	if (_FAIL == rtw_p2p_enable(padapter, init_role))
	{
		ret = -EFAULT;
		goto exit;
	}

	/* set channel/bandwidth */
	if (init_role != P2P_ROLE_DISABLE)
	{
		u8 channel, ch_offset;
		u16 bwmode;

		if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_LISTEN))
		{
			/* 	Stay at the listen state and wait for discovery. */
			channel = pwdinfo->listen_channel;
			pwdinfo->operating_channel = pwdinfo->listen_channel;
			ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
			bwmode = HT_CHANNEL_WIDTH_20;
		} else {
			pwdinfo->operating_channel = pmlmeext->cur_channel;

			channel = pwdinfo->operating_channel;
			ch_offset = pmlmeext->cur_ch_offset;
			bwmode = pmlmeext->cur_bwmode;
		}

		set_channel_bwmode(padapter, channel, ch_offset, bwmode);
	}

exit:
	return ret;

}

static int rtw_p2p_set_go_nego_ssid(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);

	DBG_88E( "[%s] ssid = %s, len = %zu\n", __FUNCTION__, extra, strlen( extra ) );
	memcpy( pwdinfo->nego_ssid, extra, strlen( extra ) );
	pwdinfo->nego_ssidlen = strlen( extra );

	return ret;

}


static int rtw_p2p_set_intent(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	int							ret = 0;
	struct adapter						*padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info			*pwdinfo= &(padapter->wdinfo);
	u8							intent = pwdinfo->intent;

	extra[ wrqu->data.length ] = 0x00;

	intent = rtw_atoi( extra );

	if ( intent <= 15 )
	{
		pwdinfo->intent= intent;
	}
	else
	{
		ret = -1;
	}

	DBG_88E( "[%s] intent = %d\n", __FUNCTION__, intent);

	return ret;

}

static int rtw_p2p_set_listen_ch(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	u8	listen_ch = pwdinfo->listen_channel;	/* 	Listen channel number */

	extra[ wrqu->data.length ] = 0x00;
	listen_ch = rtw_atoi( extra );

	if ( ( listen_ch == 1 ) || ( listen_ch == 6 ) || ( listen_ch == 11 ) )
	{
		pwdinfo->listen_channel = listen_ch;
		set_channel_bwmode(padapter, pwdinfo->listen_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HT_CHANNEL_WIDTH_20);
	}
	else
	{
		ret = -1;
	}

	DBG_88E( "[%s] listen_ch = %d\n", __FUNCTION__, pwdinfo->listen_channel );

	return ret;

}

static int rtw_p2p_set_op_ch(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
/* 	Commented by Albert 20110524 */
/* 	This function is used to set the operating channel if the driver will become the group owner */

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	u8	op_ch = pwdinfo->operating_channel;	/* 	Operating channel number */

	extra[ wrqu->data.length ] = 0x00;

	op_ch = ( u8 ) rtw_atoi( extra );
	if ( op_ch > 0 )
	{
		pwdinfo->operating_channel = op_ch;
	}
	else
	{
		ret = -1;
	}

	DBG_88E( "[%s] op_ch = %d\n", __FUNCTION__, pwdinfo->operating_channel );

	return ret;

}


static int rtw_p2p_profilefound(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);

	/* 	Comment by Albert 2010/10/13 */
	/* 	Input data format: */
	/* 	Ex:  0 */
	/* 	Ex:  1XX:XX:XX:XX:XX:XXYYSSID */
	/* 	0 => Reflush the profile record list. */
	/* 	1 => Add the profile list */
	/* 	XX:XX:XX:XX:XX:XX => peer's MAC Address ( ex: 00:E0:4C:00:00:01 ) */
	/* 	YY => SSID Length */
	/* 	SSID => SSID for persistence group */

	DBG_88E( "[%s] In value = %s, len = %d\n", __FUNCTION__, extra, wrqu->data.length -1);


	/* 	The upper application should pass the SSID to driver by using this rtw_p2p_profilefound function. */
	if (!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
	{
		if ( extra[ 0 ] == '0' )
		{
			/* 	Remove all the profile information of wifidirect_info structure. */
			memset( &pwdinfo->profileinfo[ 0 ], 0x00, sizeof( struct profile_info ) * P2P_MAX_PERSISTENT_GROUP_NUM );
			pwdinfo->profileindex = 0;
		}
		else
		{
			if ( pwdinfo->profileindex >= P2P_MAX_PERSISTENT_GROUP_NUM )
		{
				ret = -1;
		}
		else
		{
				int jj, kk;

				/* 	Add this profile information into pwdinfo->profileinfo */
				/* 	Ex:  1XX:XX:XX:XX:XX:XXYYSSID */
				for ( jj = 0, kk = 1; jj < ETH_ALEN; jj++, kk += 3 )
				{
					pwdinfo->profileinfo[ pwdinfo->profileindex ].peermac[ jj ] = key_2char2num(extra[ kk ], extra[ kk+ 1 ]);
				}

				/* pwdinfo->profileinfo[ pwdinfo->profileindex ].ssidlen = ( extra[18] - '0' ) * 10 + ( extra[ 19 ] - '0' ); */
				/* memcpy( pwdinfo->profileinfo[ pwdinfo->profileindex ].ssid, &extra[ 20 ], pwdinfo->profileinfo[ pwdinfo->profileindex ].ssidlen ); */
				pwdinfo->profileindex++;
			}
		}
	}

	return ret;

}

static int rtw_p2p_setDN(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);


	DBG_88E( "[%s] %s %d\n", __FUNCTION__, extra, wrqu->data.length -1  );
	memset( pwdinfo->device_name, 0x00, WPS_MAX_DEVICE_NAME_LEN );
	memcpy( pwdinfo->device_name, extra, wrqu->data.length - 1 );
	pwdinfo->device_name_len = wrqu->data.length - 1;

	return ret;

}


static int rtw_p2p_get_status(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	if ( padapter->bShowGetP2PState )
	{
		DBG_88E( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __FUNCTION__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo),
				pwdinfo->p2p_peer_interface_addr[ 0 ], pwdinfo->p2p_peer_interface_addr[ 1 ], pwdinfo->p2p_peer_interface_addr[ 2 ],
				pwdinfo->p2p_peer_interface_addr[ 3 ], pwdinfo->p2p_peer_interface_addr[ 4 ], pwdinfo->p2p_peer_interface_addr[ 5 ]);
	}

	/* 	Commented by Albert 2010/10/12 */
	/* 	Because of the output size limitation, I had removed the "Role" information. */
	/* 	About the "Role" information, we will use the new private IOCTL to get the "Role" information. */
	sprintf( extra, "\n\nStatus=%.2d\n", rtw_p2p_state(pwdinfo) );
	wrqu->data.length = strlen( extra );

	return ret;

}

/* 	Commented by Albert 20110520 */
/* 	This function will return the config method description */
/* 	This config method description will show us which config method the remote P2P device is intented to use */
/* 	by sending the provisioning discovery request frame. */

static int rtw_p2p_get_req_cm(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	sprintf( extra, "\n\nCM=%s\n", pwdinfo->rx_prov_disc_info.strconfig_method_desc_of_prov_disc_req );
	wrqu->data.length = strlen( extra );
	return ret;

}


static int rtw_p2p_get_role(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );


	DBG_88E( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __FUNCTION__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo),
			pwdinfo->p2p_peer_interface_addr[ 0 ], pwdinfo->p2p_peer_interface_addr[ 1 ], pwdinfo->p2p_peer_interface_addr[ 2 ],
			pwdinfo->p2p_peer_interface_addr[ 3 ], pwdinfo->p2p_peer_interface_addr[ 4 ], pwdinfo->p2p_peer_interface_addr[ 5 ]);

	sprintf( extra, "\n\nRole=%.2d\n", rtw_p2p_role(pwdinfo) );
	wrqu->data.length = strlen( extra );
	return ret;

}


static int rtw_p2p_get_peer_ifaddr(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );


	DBG_88E( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __FUNCTION__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo),
			pwdinfo->p2p_peer_interface_addr[ 0 ], pwdinfo->p2p_peer_interface_addr[ 1 ], pwdinfo->p2p_peer_interface_addr[ 2 ],
			pwdinfo->p2p_peer_interface_addr[ 3 ], pwdinfo->p2p_peer_interface_addr[ 4 ], pwdinfo->p2p_peer_interface_addr[ 5 ]);

	sprintf( extra, "\nMAC %.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
			pwdinfo->p2p_peer_interface_addr[ 0 ], pwdinfo->p2p_peer_interface_addr[ 1 ], pwdinfo->p2p_peer_interface_addr[ 2 ],
			pwdinfo->p2p_peer_interface_addr[ 3 ], pwdinfo->p2p_peer_interface_addr[ 4 ], pwdinfo->p2p_peer_interface_addr[ 5 ]);
	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_peer_devaddr(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)

{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	DBG_88E( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __FUNCTION__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo),
			pwdinfo->rx_prov_disc_info.peerDevAddr[ 0 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 1 ],
			pwdinfo->rx_prov_disc_info.peerDevAddr[ 2 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 3 ],
			pwdinfo->rx_prov_disc_info.peerDevAddr[ 4 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 5 ]);
	sprintf( extra, "\n%.2X%.2X%.2X%.2X%.2X%.2X",
			pwdinfo->rx_prov_disc_info.peerDevAddr[ 0 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 1 ],
			pwdinfo->rx_prov_disc_info.peerDevAddr[ 2 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 3 ],
			pwdinfo->rx_prov_disc_info.peerDevAddr[ 4 ], pwdinfo->rx_prov_disc_info.peerDevAddr[ 5 ]);
	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_peer_devaddr_by_invitation(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)

{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	DBG_88E( "[%s] Role = %d, Status = %d, peer addr = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\n", __FUNCTION__, rtw_p2p_role(pwdinfo), rtw_p2p_state(pwdinfo),
			pwdinfo->p2p_peer_device_addr[ 0 ], pwdinfo->p2p_peer_device_addr[ 1 ],
			pwdinfo->p2p_peer_device_addr[ 2 ], pwdinfo->p2p_peer_device_addr[ 3 ],
			pwdinfo->p2p_peer_device_addr[ 4 ], pwdinfo->p2p_peer_device_addr[ 5 ]);
	sprintf( extra, "\nMAC %.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
			pwdinfo->p2p_peer_device_addr[ 0 ], pwdinfo->p2p_peer_device_addr[ 1 ],
			pwdinfo->p2p_peer_device_addr[ 2 ], pwdinfo->p2p_peer_device_addr[ 3 ],
			pwdinfo->p2p_peer_device_addr[ 4 ], pwdinfo->p2p_peer_device_addr[ 5 ]);
	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_groupid(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)

{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	sprintf( extra, "\n%.2X:%.2X:%.2X:%.2X:%.2X:%.2X %s",
			pwdinfo->groupid_info.go_device_addr[ 0 ], pwdinfo->groupid_info.go_device_addr[ 1 ],
			pwdinfo->groupid_info.go_device_addr[ 2 ], pwdinfo->groupid_info.go_device_addr[ 3 ],
			pwdinfo->groupid_info.go_device_addr[ 4 ], pwdinfo->groupid_info.go_device_addr[ 5 ],
			pwdinfo->groupid_info.ssid);
	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_op_ch(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)

{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );


	DBG_88E( "[%s] Op_ch = %02x\n", __FUNCTION__, pwdinfo->operating_channel);

	sprintf( extra, "\n\nOp_ch=%.2d\n", pwdinfo->operating_channel );
	wrqu->data.length = strlen( extra );
	return ret;

}

inline static void macstr2num(u8 *dst, u8 *src)
{
	int	jj, kk;
	for (jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3)
	{
		dst[jj] = key_2char2num(src[kk], src[kk + 1]);
	}
}

static int rtw_p2p_get_wps_configmethod(struct net_device *dev,
										struct iw_request_info *info,
										union iwreq_data *wrqu, char *extra, char *subcmd)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	u8 peerMAC[ETH_ALEN] = { 0x00 };
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	unsigned long irqL;
	struct list_head *plist,*phead;
	struct  __queue *queue = &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork = NULL;
	u8 blnMatch = 0;
	__be16	be_attr_content = 0;
	u16	attr_content = 0;
	uint attr_contentlen = 0;
	u8	attr_content_str[P2P_PRIVATE_IOCTL_SET_LEN] = { 0x00 };

	/* 	Commented by Albert 20110727 */
	/* 	The input data is the MAC address which the application wants to know its WPS config method. */
	/* 	After knowing its WPS config method, the application can decide the config method for provisioning discovery. */
	/* 	Format: iwpriv wlanx p2p_get_wpsCM 00:E0:4C:00:00:05 */

	DBG_88E("[%s] data = %s\n", __FUNCTION__, subcmd);

	macstr2num(peerMAC, subcmd);

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1)
	{
		if (rtw_end_of_queue_search(phead, plist) == true) break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if (_rtw_memcmp(pnetwork->network.MacAddress, peerMAC, ETH_ALEN))
		{
			u8 *wpsie;
			uint	wpsie_len = 0;

			/* 	The mac address is matched. */

			if ( (wpsie=rtw_get_wps_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &wpsie_len, pnetwork->network.Reserved[0])) )
			{
				rtw_get_wps_attr_content(wpsie, wpsie_len, WPS_ATTR_CONF_METHOD, (u8 *)&be_attr_content, &attr_contentlen);
				if (attr_contentlen) {
					attr_content = be16_to_cpu(be_attr_content);
					sprintf(attr_content_str, "\n\nM=%.4d", attr_content);
					blnMatch = 1;
				}
			}

			break;
		}

		plist = get_next(plist);

	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

	if (!blnMatch)
		sprintf(attr_content_str, "\n\nM=0000");

	wrqu->data.length = strlen(attr_content_str);
	memcpy(extra, attr_content_str, wrqu->data.length);

	return ret;
}

#ifdef CONFIG_P2P
static int rtw_p2p_get_peer_wfd_port(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	DBG_88E( "[%s] p2p_state = %d\n", __FUNCTION__, rtw_p2p_state(pwdinfo) );

	sprintf( extra, "\n\nPort=%d\n", pwdinfo->wfd_info->peer_rtsp_ctrlport );
	DBG_88E( "[%s] remote port = %d\n", __FUNCTION__, pwdinfo->wfd_info->peer_rtsp_ctrlport );

	wrqu->data.length = strlen( extra );
	return ret;

}

static int rtw_p2p_get_peer_wfd_preferred_connection(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	sprintf( extra, "\n\nwfd_pc=%d\n", pwdinfo->wfd_info->wfd_pc );
	DBG_88E( "[%s] wfd_pc = %d\n", __FUNCTION__, pwdinfo->wfd_info->wfd_pc );

	wrqu->data.length = strlen( extra );
	pwdinfo->wfd_info->wfd_pc = false;	/* 	Reset the WFD preferred connection to P2P */
	return ret;

}

static int rtw_p2p_get_peer_wfd_session_available(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );

	sprintf( extra, "\n\nwfd_sa=%d\n", pwdinfo->wfd_info->peer_session_avail );
	DBG_88E( "[%s] wfd_sa = %d\n", __FUNCTION__, pwdinfo->wfd_info->peer_session_avail );

	wrqu->data.length = strlen( extra );
	pwdinfo->wfd_info->peer_session_avail = true;	/* 	Reset the WFD session available */
	return ret;

}

#endif /*  CONFIG_P2P */

static int rtw_p2p_get_go_device_address(struct net_device *dev,
										 struct iw_request_info *info,
										 union iwreq_data *wrqu, char *extra, char *subcmd)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	u8 peerMAC[ETH_ALEN] = { 0x00 };
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	unsigned long irqL;
	struct list_head *plist, *phead;
	struct  __queue *queue	= &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork = NULL;
	u8 blnMatch = 0;
	u8 *p2pie;
	uint p2pielen = 0, attr_contentlen = 0;
	u8 attr_content[100] = { 0x00 };
	u8 go_devadd_str[P2P_PRIVATE_IOCTL_SET_LEN] = { 0x00 };

	/* 	Commented by Albert 20121209 */
	/* 	The input data is the GO's interface address which the application wants to know its device address. */
	/* 	Format: iwpriv wlanx p2p_get2 go_devadd=00:E0:4C:00:00:05 */

	DBG_88E("[%s] data = %s\n", __FUNCTION__, subcmd);

	macstr2num(peerMAC, subcmd);

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1)
	{
		if (rtw_end_of_queue_search(phead, plist) == true) break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if (_rtw_memcmp(pnetwork->network.MacAddress, peerMAC, ETH_ALEN))
		{
			/* 	Commented by Albert 2011/05/18 */
			/* 	Match the device address located in the P2P IE */
			/* 	This is for the case that the P2P device address is not the same as the P2P interface address. */

			if ((p2pie = rtw_get_p2p_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &p2pielen, pnetwork->network.Reserved[0])))
			{
				while (p2pie)
				{
					/* 	The P2P Device ID attribute is included in the Beacon frame. */
					/* 	The P2P Device Info attribute is included in the probe response frame. */

					memset(attr_content, 0x00, 100);
					if (rtw_get_p2p_attr_content(p2pie, p2pielen, P2P_ATTR_DEVICE_ID, attr_content, &attr_contentlen))
					{
						/* 	Handle the P2P Device ID attribute of Beacon first */
						blnMatch = 1;
						break;

					} else if (rtw_get_p2p_attr_content(p2pie, p2pielen, P2P_ATTR_DEVICE_INFO, attr_content, &attr_contentlen))
					{
						/* 	Handle the P2P Device Info attribute of probe response */
						blnMatch = 1;
						break;
					}

					/* Get the next P2P IE */
					p2pie = rtw_get_p2p_ie(p2pie + p2pielen, pnetwork->network.IELength - 12 - (p2pie - &pnetwork->network.IEs[12] + p2pielen), NULL, &p2pielen);
				}
			}
		}

		plist = get_next(plist);

	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

	if (!blnMatch) {
		sprintf(go_devadd_str, "\n\ndev_add= NULL");
	} else {
		sprintf(go_devadd_str, "\n\ndev_add=%.2X:%.2X:%.2X:%.2X:%.2X:%.2X",
				attr_content[0], attr_content[1], attr_content[2], attr_content[3], attr_content[4], attr_content[5]);
	}

	wrqu->data.length = strlen(go_devadd_str);
	memcpy(extra, go_devadd_str, wrqu->data.length);

	return ret;

}

static int rtw_p2p_get_device_type(struct net_device *dev,
								   struct iw_request_info *info,
								   union iwreq_data *wrqu, char *extra, char *subcmd)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	u8 peerMAC[ETH_ALEN] = { 0x00 };
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	unsigned long irqL;
	struct list_head *plist, *phead;
	struct  __queue *queue = &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork = NULL;
	u8 blnMatch = 0;
	u8 dev_type[8] = { 0x00 };
	uint dev_type_len = 0;
	u8 dev_type_str[P2P_PRIVATE_IOCTL_SET_LEN] = { 0x00 };    /*  +9 is for the str "dev_type=", we have to clear it at wrqu->data.pointer */

	/* 	Commented by Albert 20121209 */
	/* 	The input data is the MAC address which the application wants to know its device type. */
	/* 	Such user interface could know the device type. */
	/* 	Format: iwpriv wlanx p2p_get2 dev_type=00:E0:4C:00:00:05 */

	DBG_88E("[%s] data = %s\n", __FUNCTION__, subcmd);

	macstr2num(peerMAC, subcmd);

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1)
	{
		if (rtw_end_of_queue_search(phead, plist) == true) break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if (_rtw_memcmp(pnetwork->network.MacAddress, peerMAC, ETH_ALEN))
		{
			u8 *wpsie;
			uint	wpsie_len = 0;

			/* 	The mac address is matched. */

			if ( (wpsie=rtw_get_wps_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &wpsie_len, pnetwork->network.Reserved[0])) )
			{
				rtw_get_wps_attr_content(wpsie, wpsie_len, WPS_ATTR_PRIMARY_DEV_TYPE, dev_type, &dev_type_len);
				if (dev_type_len) {
					__be16 be_type;
					u16 type;

					memcpy(&be_type, dev_type, 2);
					type = be16_to_cpu(be_type);
					sprintf(dev_type_str, "\n\nN=%.2d", type);
					blnMatch = 1;
				}
			}
			break;
		}

		plist = get_next(plist);

	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

	if (!blnMatch)
		sprintf(dev_type_str, "\n\nN=00");

	wrqu->data.length = strlen(dev_type_str);
	memcpy(extra, dev_type_str, wrqu->data.length);

	return ret;
}

static int rtw_p2p_get_device_name(struct net_device *dev,
								   struct iw_request_info *info,
								   union iwreq_data *wrqu, char *extra, char *subcmd)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	u8 peerMAC[ETH_ALEN] = { 0x00 };
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	unsigned long irqL;
	struct list_head *plist, *phead;
	struct  __queue *queue = &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork = NULL;
	u8 blnMatch = 0;
	u8 dev_name[WPS_MAX_DEVICE_NAME_LEN] = { 0x00 };
	uint dev_len = 0;
	u8 dev_name_str[P2P_PRIVATE_IOCTL_SET_LEN] = { 0x00 };

	/* 	Commented by Albert 20121225 */
	/* 	The input data is the MAC address which the application wants to know its device name. */
	/* 	Such user interface could show peer device's device name instead of ssid. */
	/* 	Format: iwpriv wlanx p2p_get2 devN=00:E0:4C:00:00:05 */

	DBG_88E("[%s] data = %s\n", __FUNCTION__, subcmd);

	macstr2num(peerMAC, subcmd);

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1)
	{
		if (rtw_end_of_queue_search(phead, plist) == true) break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if (_rtw_memcmp(pnetwork->network.MacAddress, peerMAC, ETH_ALEN))
		{
			u8 *wpsie;
			uint	wpsie_len = 0;

			/* 	The mac address is matched. */

			if ( (wpsie=rtw_get_wps_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &wpsie_len, pnetwork->network.Reserved[0])) )
			{
				rtw_get_wps_attr_content(wpsie, wpsie_len, WPS_ATTR_DEVICE_NAME, dev_name, &dev_len);
				if (dev_len)
				{
					sprintf(dev_name_str, "\n\nN=%s", dev_name);
					blnMatch = 1;
				}
			}
			break;
		}

		plist = get_next(plist);

	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

	if (!blnMatch)
		sprintf(dev_name_str, "\n\nN=0000");

	wrqu->data.length = strlen(dev_name_str);
	memcpy(extra, dev_name_str, wrqu->data.length);

	return ret;
}

static int rtw_p2p_get_invitation_procedure(struct net_device *dev,
											struct iw_request_info *info,
											union iwreq_data *wrqu, char *extra, char *subcmd)
{

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	u8 peerMAC[ETH_ALEN] = { 0x00 };
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	unsigned long irqL;
	struct list_head *plist, *phead;
	struct  __queue *queue	= &(pmlmepriv->scanned_queue);
	struct wlan_network *pnetwork = NULL;
	u8 blnMatch = 0;
	u8 *p2pie;
	uint p2pielen = 0, attr_contentlen = 0;
	u8 attr_content[2] = { 0x00 };
	u8 inv_proc_str[P2P_PRIVATE_IOCTL_SET_LEN] = { 0x00 };

	/* 	Commented by Ouden 20121226 */
	/* 	The application wants to know P2P initation procedure is support or not. */
	/* 	Format: iwpriv wlanx p2p_get2 InvProc=00:E0:4C:00:00:05 */

	DBG_88E("[%s] data = %s\n", __FUNCTION__, subcmd);

	macstr2num(peerMAC, subcmd);

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1)
	{
		if (rtw_end_of_queue_search(phead, plist) == true) break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if (_rtw_memcmp(pnetwork->network.MacAddress, peerMAC, ETH_ALEN))
		{
			/* 	Commented by Albert 20121226 */
			/* 	Match the device address located in the P2P IE */
			/* 	This is for the case that the P2P device address is not the same as the P2P interface address. */

			if ((p2pie = rtw_get_p2p_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &p2pielen, pnetwork->network.Reserved[0])))
			{
				while (p2pie)
				{
					/* memset( attr_content, 0x00, 2); */
					if (rtw_get_p2p_attr_content(p2pie, p2pielen, P2P_ATTR_CAPABILITY, attr_content, &attr_contentlen))
					{
						/* 	Handle the P2P capability attribute */
						blnMatch = 1;
						break;

					}

					/* Get the next P2P IE */
					p2pie = rtw_get_p2p_ie(p2pie + p2pielen, pnetwork->network.IELength - 12 - (p2pie - &pnetwork->network.IEs[12] + p2pielen), NULL, &p2pielen);
				}
			}
		}

		plist = get_next(plist);

	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

	if (!blnMatch) {
		sprintf(inv_proc_str, "\nIP=-1");
	} else {
		if (attr_content[0] & 0x20)
			sprintf(inv_proc_str, "\nIP=1");
		else
			sprintf(inv_proc_str, "\nIP=0");
	}

	wrqu->data.length = strlen(inv_proc_str);
	memcpy(extra, inv_proc_str, wrqu->data.length);

	return ret;
}

static int rtw_p2p_connect(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter				*padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
	u8					peerMAC[ ETH_ALEN ] = { 0x00 };
	int					jj,kk;
	u8					peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	unsigned long				irqL;
	struct list_head *plist, *phead;
	struct  __queue *queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network	*pnetwork = NULL;
	uint					uintPeerChannel = 0;

	/* 	Commented by Albert 20110304 */
	/* 	The input data contains two informations. */
	/* 	1. First information is the MAC address which wants to formate with */
	/* 	2. Second information is the WPS PINCode or "pbc" string for push button method */
	/* 	Format: 00:E0:4C:00:00:05 */
	/* 	Format: 00:E0:4C:00:00:05 */

	DBG_88E( "[%s] data = %s\n", __FUNCTION__, extra );

	if ( pwdinfo->p2p_state == P2P_STATE_NONE )
	{
		DBG_88E( "[%s] WiFi Direct is disable!\n", __FUNCTION__ );
		return ret;
	}

	if ( pwdinfo->ui_got_wps_info == P2P_NO_WPSINFO )
	{
		return -1;
	}

	for ( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3 )
	{
		peerMAC[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
	}

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1)
	{
		if (rtw_end_of_queue_search(phead,plist)== true)
			break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);
		if ( _rtw_memcmp( pnetwork->network.MacAddress, peerMAC, ETH_ALEN ) )
		{
			uintPeerChannel = pnetwork->network.Configuration.DSConfig;
			break;
		}

		plist = get_next(plist);

	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

	if ( uintPeerChannel ) {
		memset( &pwdinfo->nego_req_info, 0x00, sizeof( struct tx_nego_req_info ) );
		memset( &pwdinfo->groupid_info, 0x00, sizeof( struct group_id_info ) );

		pwdinfo->nego_req_info.peer_channel_num[ 0 ] = uintPeerChannel;
		memcpy( pwdinfo->nego_req_info.peerDevAddr, pnetwork->network.MacAddress, ETH_ALEN );
		pwdinfo->nego_req_info.benable = true;

		_cancel_timer_ex( &pwdinfo->restore_p2p_state_timer );
		if ( rtw_p2p_state(pwdinfo) != P2P_STATE_GONEGO_OK )
		{
			/* 	Restore to the listen state if the current p2p state is not nego OK */
			rtw_p2p_set_state(pwdinfo, P2P_STATE_LISTEN );
		}

		rtw_p2p_set_pre_state(pwdinfo, rtw_p2p_state(pwdinfo));
		rtw_p2p_set_state(pwdinfo, P2P_STATE_GONEGO_ING);

		DBG_88E( "[%s] Start PreTx Procedure!\n", __FUNCTION__ );
		_set_timer( &pwdinfo->pre_tx_scan_timer, P2P_TX_PRESCAN_TIMEOUT );
		_set_timer( &pwdinfo->restore_p2p_state_timer, P2P_GO_NEGO_TIMEOUT );
	} else {
		DBG_88E( "[%s] Not Found in Scanning Queue~\n", __FUNCTION__ );
		ret = -1;
	}
exit:
	return ret;
}

static int rtw_p2p_invite_req(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter					*padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point				*pdata = &wrqu->data;
	struct wifidirect_info		*pwdinfo = &( padapter->wdinfo );
	int						jj,kk;
	u8						peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv			*pmlmepriv = &padapter->mlmepriv;
	struct list_head *plist, *phead;
	struct  __queue *queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network		*pnetwork = NULL;
	uint						uintPeerChannel = 0;
	u8						attr_content[50] = { 0x00 }, _status = 0;
	u8						*p2pie;
	uint						p2pielen = 0, attr_contentlen = 0;
	unsigned long					irqL;
	struct tx_invite_req_info*	pinvite_req_info = &pwdinfo->invitereq_info;

#ifdef CONFIG_P2P
	struct wifi_display_info*	pwfd_info = pwdinfo->wfd_info;
#endif /*  CONFIG_P2P */

	/* 	Commented by Albert 20120321 */
	/* 	The input data contains two informations. */
	/* 	1. First information is the P2P device address which you want to send to. */
	/* 	2. Second information is the group id which combines with GO's mac address, space and GO's ssid. */
	/* 	Command line sample: iwpriv wlan0 p2p_set invite="00:11:22:33:44:55 00:E0:4C:00:00:05 DIRECT-xy" */
	/* 	Format: 00:11:22:33:44:55 00:E0:4C:00:00:05 DIRECT-xy */

	DBG_88E( "[%s] data = %s\n", __FUNCTION__, extra );

	if ( wrqu->data.length <=  37 )
	{
		DBG_88E( "[%s] Wrong format!\n", __FUNCTION__ );
		return ret;
	}

	if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
	{
		DBG_88E( "[%s] WiFi Direct is disable!\n", __FUNCTION__ );
		return ret;
	}
	else
	{
		/* 	Reset the content of struct tx_invite_req_info */
		pinvite_req_info->benable = false;
		memset( pinvite_req_info->go_bssid, 0x00, ETH_ALEN );
		memset( pinvite_req_info->go_ssid, 0x00, WLAN_SSID_MAXLEN );
		pinvite_req_info->ssidlen = 0x00;
		pinvite_req_info->operating_ch = pwdinfo->operating_channel;
		memset( pinvite_req_info->peer_macaddr, 0x00, ETH_ALEN );
		pinvite_req_info->token = 3;
	}

	for ( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3 )
	{
		pinvite_req_info->peer_macaddr[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
	}

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1)
	{
		if (rtw_end_of_queue_search(phead,plist)== true)
			break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

		/* 	Commented by Albert 2011/05/18 */
		/* 	Match the device address located in the P2P IE */
		/* 	This is for the case that the P2P device address is not the same as the P2P interface address. */

		if ( (p2pie=rtw_get_p2p_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &p2pielen, pnetwork->network.Reserved[0])))
		{
			/* 	The P2P Device ID attribute is included in the Beacon frame. */
			/* 	The P2P Device Info attribute is included in the probe response frame. */

			if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_ID, attr_content, &attr_contentlen) )
			{
				/* 	Handle the P2P Device ID attribute of Beacon first */
				if ( _rtw_memcmp( attr_content, pinvite_req_info->peer_macaddr, ETH_ALEN ) )
				{
					uintPeerChannel = pnetwork->network.Configuration.DSConfig;
					break;
				}
			}
			else if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_INFO, attr_content, &attr_contentlen) )
			{
				/* 	Handle the P2P Device Info attribute of probe response */
				if ( _rtw_memcmp( attr_content, pinvite_req_info->peer_macaddr, ETH_ALEN ) )
				{
					uintPeerChannel = pnetwork->network.Configuration.DSConfig;
					break;
				}
			}

		}

		plist = get_next(plist);

	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

#ifdef CONFIG_P2P
	if ( uintPeerChannel )
	{
		u8	wfd_ie[ 128 ] = { 0x00 };
		uint	wfd_ielen = 0;

		if ( rtw_get_wfd_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength,  wfd_ie, &wfd_ielen, pnetwork->network.Reserved[0]) )
		{
			u8	wfd_devinfo[ 6 ] = { 0x00 };
			uint	wfd_devlen = 6;

			DBG_88E( "[%s] Found WFD IE!\n", __FUNCTION__ );
			if ( rtw_get_wfd_attr_content( wfd_ie, wfd_ielen, WFD_ATTR_DEVICE_INFO, wfd_devinfo, &wfd_devlen ) )
			{
				u16	wfd_devinfo_field = 0;

				/* 	Commented by Albert 20120319 */
				/* 	The first two bytes are the WFD device information field of WFD device information subelement. */
				/* 	In big endian format. */
				wfd_devinfo_field = RTW_GET_BE16(wfd_devinfo);
				if ( wfd_devinfo_field & WFD_DEVINFO_SESSION_AVAIL )
				{
					pwfd_info->peer_session_avail = true;
				}
				else
				{
					pwfd_info->peer_session_avail = false;
				}
			}
		}

		if ( false == pwfd_info->peer_session_avail )
		{
			DBG_88E( "[%s] WFD Session not avaiable!\n", __FUNCTION__ );
			goto exit;
		}
	}
#endif /*  CONFIG_P2P */

	if ( uintPeerChannel ) {
		/* 	Store the GO's bssid */
		for ( jj = 0, kk = 18; jj < ETH_ALEN; jj++, kk += 3 )
		{
			pinvite_req_info->go_bssid[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
		}

		/* 	Store the GO's ssid */
		pinvite_req_info->ssidlen = wrqu->data.length - 36;
		memcpy( pinvite_req_info->go_ssid, &extra[ 36 ], (u32) pinvite_req_info->ssidlen );
		pinvite_req_info->benable = true;
		pinvite_req_info->peer_ch = uintPeerChannel;

		rtw_p2p_set_pre_state(pwdinfo, rtw_p2p_state(pwdinfo));
		rtw_p2p_set_state(pwdinfo, P2P_STATE_TX_INVITE_REQ);

		set_channel_bwmode(padapter, uintPeerChannel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HT_CHANNEL_WIDTH_20);

		_set_timer( &pwdinfo->pre_tx_scan_timer, P2P_TX_PRESCAN_TIMEOUT );

		_set_timer( &pwdinfo->restore_p2p_state_timer, P2P_INVITE_TIMEOUT );
	} else {
		DBG_88E( "[%s] NOT Found in the Scanning Queue!\n", __FUNCTION__ );
	}
exit:

	return ret;

}

static int rtw_p2p_set_persistent(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter					*padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point				*pdata = &wrqu->data;
	struct wifidirect_info		*pwdinfo = &( padapter->wdinfo );
	int						jj,kk;
	u8						peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv			*pmlmepriv = &padapter->mlmepriv;
	struct list_head *plist, *phead;
	struct  __queue *queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network		*pnetwork = NULL;
	uint						uintPeerChannel = 0;
	u8						attr_content[50] = { 0x00 }, _status = 0;
	u8						*p2pie;
	uint						p2pielen = 0, attr_contentlen = 0;
	unsigned long					irqL;
	struct tx_invite_req_info*	pinvite_req_info = &pwdinfo->invitereq_info;

#ifdef CONFIG_P2P
	struct wifi_display_info*	pwfd_info = pwdinfo->wfd_info;
#endif /*  CONFIG_P2P */

	/* 	Commented by Albert 20120328 */
	/* 	The input data is 0 or 1 */
	/* 	0: disable persistent group functionality */
	/* 	1: enable persistent group founctionality */

	DBG_88E( "[%s] data = %s\n", __FUNCTION__, extra );

	if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
	{
		DBG_88E( "[%s] WiFi Direct is disable!\n", __FUNCTION__ );
		return ret;
	}
	else
	{
		if ( extra[ 0 ] == '0' )	/* 	Disable the persistent group function. */
		{
			pwdinfo->persistent_supported = false;
		}
		else if ( extra[ 0 ] == '1' )	/* 	Enable the persistent group function. */
		{
			pwdinfo->persistent_supported = true;
		}
		else
		{
			pwdinfo->persistent_supported = false;
		}
	}
	printk( "[%s] persistent_supported = %d\n", __FUNCTION__, pwdinfo->persistent_supported );

exit:

	return ret;

}

static int hexstr2bin(const char *hex, u8 *buf, size_t len)
{
	size_t i;
	int a;
	const char *ipos = hex;
	u8 *opos = buf;

	for (i = 0; i < len; i++) {
		a = hex2byte_i(ipos);
		if (a < 0)
			return -1;
		*opos++ = a;
		ipos += 2;
	}
	return 0;
}

static int uuid_str2bin(const char *str, u8 *bin)
{
	const char *pos;
	u8 *opos;

	pos = str;
	opos = bin;

	if (hexstr2bin(pos, opos, 4))
		return -1;
	pos += 8;
	opos += 4;

	if (*pos++ != '-' || hexstr2bin(pos, opos, 2))
		return -1;
	pos += 4;
	opos += 2;

	if (*pos++ != '-' || hexstr2bin(pos, opos, 2))
		return -1;
	pos += 4;
	opos += 2;

	if (*pos++ != '-' || hexstr2bin(pos, opos, 2))
		return -1;
	pos += 4;
	opos += 2;

	if (*pos++ != '-' || hexstr2bin(pos, opos, 6))
		return -1;

	return 0;
}

static int rtw_p2p_set_wps_uuid(struct net_device *dev,
	struct iw_request_info *info,
	union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter				*padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info			*pwdinfo = &(padapter->wdinfo);

	DBG_88E("[%s] data = %s\n", __FUNCTION__, extra);

	if ((36 == strlen(extra)) && (uuid_str2bin(extra, pwdinfo->uuid) == 0))
	{
		pwdinfo->external_uuid = 1;
	} else {
		pwdinfo->external_uuid = 0;
		ret = -EINVAL;
	}

	return ret;

}
#ifdef CONFIG_P2P
static int rtw_p2p_set_pc(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter				*padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point			*pdata = &wrqu->data;
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
	u8					peerMAC[ ETH_ALEN ] = { 0x00 };
	int					jj,kk;
	u8					peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct list_head *plist, *phead;
	struct  __queue *queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network	*pnetwork = NULL;
	u8					attr_content[50] = { 0x00 }, _status = 0;
	u8 *p2pie;
	uint					p2pielen = 0, attr_contentlen = 0;
	unsigned long				irqL;
	uint					uintPeerChannel = 0;
	struct wifi_display_info*	pwfd_info = pwdinfo->wfd_info;

	/* 	Commented by Albert 20120512 */
	/* 	1. Input information is the MAC address which wants to know the Preferred Connection bit (PC bit) */
	/* 	Format: 00:E0:4C:00:00:05 */

	DBG_88E( "[%s] data = %s\n", __FUNCTION__, extra );

	if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
	{
		DBG_88E( "[%s] WiFi Direct is disable!\n", __FUNCTION__ );
		return ret;
	}

	for ( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3 )
	{
		peerMAC[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );
	}

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1)
	{
		if (rtw_end_of_queue_search(phead,plist)== true)
			break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

		/* 	Commented by Albert 2011/05/18 */
		/* 	Match the device address located in the P2P IE */
		/* 	This is for the case that the P2P device address is not the same as the P2P interface address. */

		if ( (p2pie=rtw_get_p2p_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &p2pielen, pnetwork->network.Reserved[0])))
		{
			/* 	The P2P Device ID attribute is included in the Beacon frame. */
			/* 	The P2P Device Info attribute is included in the probe response frame. */
			printk( "[%s] Got P2P IE\n", __FUNCTION__ );
			if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_ID, attr_content, &attr_contentlen) )
			{
				/* 	Handle the P2P Device ID attribute of Beacon first */
				printk( "[%s] P2P_ATTR_DEVICE_ID\n", __FUNCTION__ );
				if ( _rtw_memcmp( attr_content, peerMAC, ETH_ALEN ) )
				{
					uintPeerChannel = pnetwork->network.Configuration.DSConfig;
					break;
				}
			}
			else if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_INFO, attr_content, &attr_contentlen) )
			{
				/* 	Handle the P2P Device Info attribute of probe response */
				printk( "[%s] P2P_ATTR_DEVICE_INFO\n", __FUNCTION__ );
				if ( _rtw_memcmp( attr_content, peerMAC, ETH_ALEN ) )
				{
					uintPeerChannel = pnetwork->network.Configuration.DSConfig;
					break;
				}
			}

		}

		plist = get_next(plist);

	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));
	printk( "[%s] channel = %d\n", __FUNCTION__, uintPeerChannel );

	if ( uintPeerChannel )
	{
		u8	wfd_ie[ 128 ] = { 0x00 };
		uint	wfd_ielen = 0;

		if ( rtw_get_wfd_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength,  wfd_ie, &wfd_ielen, pnetwork->network.Reserved[0]) )
		{
			u8	wfd_devinfo[ 6 ] = { 0x00 };
			uint	wfd_devlen = 6;

			DBG_88E( "[%s] Found WFD IE!\n", __FUNCTION__ );
			if ( rtw_get_wfd_attr_content( wfd_ie, wfd_ielen, WFD_ATTR_DEVICE_INFO, wfd_devinfo, &wfd_devlen ) )
			{
				u16	wfd_devinfo_field = 0;

				/* 	Commented by Albert 20120319 */
				/* 	The first two bytes are the WFD device information field of WFD device information subelement. */
				/* 	In big endian format. */
				wfd_devinfo_field = RTW_GET_BE16(wfd_devinfo);
				if ( wfd_devinfo_field & WFD_DEVINFO_PC_TDLS )
				{
					pwfd_info->wfd_pc = true;
				}
				else
				{
					pwfd_info->wfd_pc = false;
				}
			}
		}
	}
	else
	{
		DBG_88E( "[%s] NOT Found in the Scanning Queue!\n", __FUNCTION__ );
	}

exit:

	return ret;

}

static int rtw_p2p_set_wfd_device_type(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter					*padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point				*pdata = &wrqu->data;
	struct wifidirect_info		*pwdinfo = &( padapter->wdinfo );
	struct wifi_display_info*	pwfd_info = pwdinfo->wfd_info;

	/* 	Commented by Albert 20120328 */
	/* 	The input data is 0 or 1 */
	/* 	0: specify to Miracast source device */
	/* 	1 or others: specify to Miracast sink device (display device) */

	DBG_88E( "[%s] data = %s\n", __FUNCTION__, extra );

	if ( extra[ 0 ] == '0' )	/* 	Set to Miracast source device. */
	{
		pwfd_info->wfd_device_type = WFD_DEVINFO_SOURCE;
	}
	else					/* 	Set to Miracast sink device. */
	{
		pwfd_info->wfd_device_type = WFD_DEVINFO_PSINK;
	}

exit:

	return ret;

}

static int rtw_p2p_set_scan_result_type(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter					*padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point				*pdata = &wrqu->data;
	struct wifidirect_info		*pwdinfo = &( padapter->wdinfo );
	struct wifi_display_info		*pwfd_info = pwdinfo->wfd_info;

	/* 	Commented by Albert 20120328 */
	/* 	The input data is 0 , 1 , 2 */
	/* 	0: when the P2P is enabled, the scan result will return all the found P2P device. */
	/* 	1: when the P2P is enabled, the scan result will return all the found P2P device and AP. */
	/* 	2: when the P2P is enabled, the scan result will show up the found Miracast devices base on... */
	/* 	It will show up all the Miracast source device if this device is sink. */
	/* 	It will show up all the Miracast sink device if this device is source. */

	DBG_88E( "[%s] data = %s\n", __FUNCTION__, extra );

	if ( extra[ 0 ] == '0' )
	{
		pwfd_info->scan_result_type = SCAN_RESULT_P2P_ONLY;
	}
	else if ( extra[ 0 ] == '1' )
	{
		pwfd_info->scan_result_type = SCAN_RESULT_ALL;
	}
	else if ( extra[ 0 ] == '2' )
	{
		pwfd_info->scan_result_type = SCAN_RESULT_WFD_TYPE;
	}
	else
	{
		pwfd_info->scan_result_type = SCAN_RESULT_P2P_ONLY;
	}

exit:

	return ret;

}

static int rtw_p2p_set_wfd_enable(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
/* 	Commented by Kurt 20121206 */
/* 	This function is used to set wfd enabled */

	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);

	if (*extra == '0' )
		pwdinfo->wfd_info->wfd_enable = false;
	else if (*extra == '1')
		pwdinfo->wfd_info->wfd_enable = true;

	DBG_88E( "[%s] wfd_enable = %d\n", __FUNCTION__, pwdinfo->wfd_info->wfd_enable );

	return ret;

}

static int rtw_p2p_set_driver_iface(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
/* 	Commented by Kurt 20121206 */
/* 	This function is used to set driver iface is WEXT or CFG80211 */
	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);

	if (*extra == '1' )
	{
		pwdinfo->driver_interface = DRIVER_WEXT;
		DBG_88E( "[%s] driver_interface = WEXT\n", __FUNCTION__);
	}
	else if (*extra == '2')
	{
		pwdinfo->driver_interface = DRIVER_CFG80211;
		DBG_88E( "[%s] driver_interface = CFG80211\n", __FUNCTION__);
	}

	return ret;

}

/* 	To set the WFD session available to enable or disable */
static int rtw_p2p_set_sa(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter					*padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point				*pdata = &wrqu->data;
	struct wifidirect_info		*pwdinfo = &( padapter->wdinfo );
	struct wifi_display_info		*pwfd_info = pwdinfo->wfd_info;

	DBG_88E( "[%s] data = %s\n", __FUNCTION__, extra );

	if ( 0 )
	{
		DBG_88E( "[%s] WiFi Direct is disable!\n", __FUNCTION__ );
		return ret;
	}
	else
	{
		if ( extra[ 0 ] == '0' )	/* 	Disable the session available. */
		{
			pwdinfo->session_available = false;
		}
		else if ( extra[ 0 ] == '1' )	/* 	Enable the session available. */
		{
			pwdinfo->session_available = true;
		}
		else
		{
			pwdinfo->session_available = false;
		}
	}
	printk( "[%s] session available = %d\n", __FUNCTION__, pwdinfo->session_available );

exit:

	return ret;

}
#endif /*  CONFIG_P2P */

static int rtw_p2p_prov_disc(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	struct adapter				*padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );
	u8					peerMAC[ ETH_ALEN ] = { 0x00 };
	int					jj,kk;
	u8					peerMACStr[ ETH_ALEN * 2 ] = { 0x00 };
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct list_head *plist, *phead;
	struct  __queue *queue	= &(pmlmepriv->scanned_queue);
	struct	wlan_network	*pnetwork = NULL;
	uint					uintPeerChannel = 0;
	u8					attr_content[100] = { 0x00 }, _status = 0;
	u8 *p2pie;
	uint					p2pielen = 0, attr_contentlen = 0;
	unsigned long				irqL;
	u8					ie_offset;

#ifdef CONFIG_P2P
	struct wifi_display_info*	pwfd_info = pwdinfo->wfd_info;
#endif /*  CONFIG_P2P */

	/* 	Commented by Albert 20110301 */
	/* 	The input data contains two informations. */
	/* 	1. First information is the MAC address which wants to issue the provisioning discovery request frame. */
	/* 	2. Second information is the WPS configuration method which wants to discovery */
	/* 	Format: 00:E0:4C:00:00:05_display */
	/* 	Format: 00:E0:4C:00:00:05_keypad */
	/* 	Format: 00:E0:4C:00:00:05_pbc */
	/* 	Format: 00:E0:4C:00:00:05_label */

	DBG_88E( "[%s] data = %s\n", __FUNCTION__, extra );

	if ( pwdinfo->p2p_state == P2P_STATE_NONE ) {
		DBG_88E( "[%s] WiFi Direct is disable!\n", __FUNCTION__ );
		return ret;
	} else {
		/* 	Reset the content of struct tx_provdisc_req_info excluded the wps_config_method_request. */
		memset( pwdinfo->tx_prov_disc_info.peerDevAddr, 0x00, ETH_ALEN );
		memset( pwdinfo->tx_prov_disc_info.peerIFAddr, 0x00, ETH_ALEN );
		memset( &pwdinfo->tx_prov_disc_info.ssid, 0x00, sizeof( struct ndis_802_11_ssid ) );
		pwdinfo->tx_prov_disc_info.peer_channel_num[ 0 ] = 0;
		pwdinfo->tx_prov_disc_info.peer_channel_num[ 1 ] = 0;
		pwdinfo->tx_prov_disc_info.benable = false;
	}

	for ( jj = 0, kk = 0; jj < ETH_ALEN; jj++, kk += 3 )
		peerMAC[ jj ] = key_2char2num( extra[kk], extra[kk+ 1] );

	if ( _rtw_memcmp( &extra[ 18 ], "display", 7 ) )
	{
		pwdinfo->tx_prov_disc_info.wps_config_method_request = WPS_CM_DISPLYA;
	}
	else if ( _rtw_memcmp( &extra[ 18 ], "keypad", 7 ) )
	{
		pwdinfo->tx_prov_disc_info.wps_config_method_request = WPS_CM_KEYPAD;
	}
	else if ( _rtw_memcmp( &extra[ 18 ], "pbc", 3 ) )
	{
		pwdinfo->tx_prov_disc_info.wps_config_method_request = WPS_CM_PUSH_BUTTON;
	}
	else if ( _rtw_memcmp( &extra[ 18 ], "label", 5 ) )
	{
		pwdinfo->tx_prov_disc_info.wps_config_method_request = WPS_CM_LABEL;
	}
	else
	{
		DBG_88E( "[%s] Unknown WPS config methodn", __FUNCTION__ );
		return( ret );
	}

	spin_lock_bh(&(pmlmepriv->scanned_queue.lock));

	phead = get_list_head(queue);
	plist = get_next(phead);

	while (1)
	{
		if (rtw_end_of_queue_search(phead,plist)== true)
			break;

		if ( uintPeerChannel != 0 )
			break;

		pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

		/* 	Commented by Albert 2011/05/18 */
		/* 	Match the device address located in the P2P IE */
		/* 	This is for the case that the P2P device address is not the same as the P2P interface address. */

		if (pnetwork->network.Reserved[0] == 2) { /*  Probe Request */
			ie_offset = 0;
		} else { /*  Beacon or Probe Respones */
			ie_offset = 12;
		}
		if ( (p2pie=rtw_get_p2p_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength, NULL, &p2pielen, pnetwork->network.Reserved[0])))
		{
			while ( p2pie )
			{
				/* 	The P2P Device ID attribute is included in the Beacon frame. */
				/* 	The P2P Device Info attribute is included in the probe response frame. */

				if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_ID, attr_content, &attr_contentlen) )
				{
					/* 	Handle the P2P Device ID attribute of Beacon first */
					if ( _rtw_memcmp( attr_content, peerMAC, ETH_ALEN ) )
					{
						uintPeerChannel = pnetwork->network.Configuration.DSConfig;
						break;
					}
				}
				else if ( rtw_get_p2p_attr_content( p2pie, p2pielen, P2P_ATTR_DEVICE_INFO, attr_content, &attr_contentlen) )
				{
					/* 	Handle the P2P Device Info attribute of probe response */
					if ( _rtw_memcmp( attr_content, peerMAC, ETH_ALEN ) )
					{
						uintPeerChannel = pnetwork->network.Configuration.DSConfig;
						break;
					}
				}

				/* Get the next P2P IE */
				p2pie = rtw_get_p2p_ie(p2pie+p2pielen, pnetwork->network.IELength - ie_offset -(p2pie -&pnetwork->network.IEs[ie_offset] + p2pielen), NULL, &p2pielen);
			}
		}
		plist = get_next(plist);
	}

	spin_unlock_bh(&(pmlmepriv->scanned_queue.lock));

	if ( uintPeerChannel )
	{
#ifdef CONFIG_P2P
		{
			u8	wfd_ie[ 128 ] = { 0x00 };
			uint	wfd_ielen = 0;

			if ( rtw_get_wfd_ie_from_scan_queue( &pnetwork->network.IEs[0], pnetwork->network.IELength,  wfd_ie, &wfd_ielen, pnetwork->network.Reserved[0]) )
			{
				u8	wfd_devinfo[ 6 ] = { 0x00 };
				uint	wfd_devlen = 6;

				DBG_88E( "[%s] Found WFD IE!\n", __FUNCTION__ );
				if ( rtw_get_wfd_attr_content( wfd_ie, wfd_ielen, WFD_ATTR_DEVICE_INFO, wfd_devinfo, &wfd_devlen ) )
				{
					u16	wfd_devinfo_field = 0;

					/* 	Commented by Albert 20120319 */
					/* 	The first two bytes are the WFD device information field of WFD device information subelement. */
					/* 	In big endian format. */
					wfd_devinfo_field = RTW_GET_BE16(wfd_devinfo);
					if ( wfd_devinfo_field & WFD_DEVINFO_SESSION_AVAIL )
					{
						pwfd_info->peer_session_avail = true;
					}
					else
					{
						pwfd_info->peer_session_avail = false;
					}
				}
			}

			if ( false == pwfd_info->peer_session_avail )
			{
				DBG_88E( "[%s] WFD Session not avaiable!\n", __FUNCTION__ );
				goto exit;
			}
		}
#endif /*  CONFIG_P2P */

		DBG_88E( "[%s] peer channel: %d!\n", __FUNCTION__, uintPeerChannel );
		memcpy( pwdinfo->tx_prov_disc_info.peerIFAddr, pnetwork->network.MacAddress, ETH_ALEN );
		memcpy( pwdinfo->tx_prov_disc_info.peerDevAddr, peerMAC, ETH_ALEN );
		pwdinfo->tx_prov_disc_info.peer_channel_num[0] = ( u16 ) uintPeerChannel;
		pwdinfo->tx_prov_disc_info.benable = true;
		rtw_p2p_set_pre_state(pwdinfo, rtw_p2p_state(pwdinfo));
		rtw_p2p_set_state(pwdinfo, P2P_STATE_TX_PROVISION_DIS_REQ);

		if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_CLIENT))
		{
			memcpy( &pwdinfo->tx_prov_disc_info.ssid, &pnetwork->network.Ssid, sizeof( struct ndis_802_11_ssid ) );
		}
		else if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_DEVICE) || rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO))
		{
			memcpy( pwdinfo->tx_prov_disc_info.ssid.Ssid, pwdinfo->p2p_wildcard_ssid, P2P_WILDCARD_SSID_LEN );
			pwdinfo->tx_prov_disc_info.ssid.SsidLength= P2P_WILDCARD_SSID_LEN;
		}

		set_channel_bwmode(padapter, uintPeerChannel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, HT_CHANNEL_WIDTH_20);

		_set_timer( &pwdinfo->pre_tx_scan_timer, P2P_TX_PRESCAN_TIMEOUT );

		_set_timer( &pwdinfo->restore_p2p_state_timer, P2P_PROVISION_TIMEOUT );
	} else {
		DBG_88E( "[%s] NOT Found in the Scanning Queue!\n", __FUNCTION__ );
	}
exit:

	return ret;

}

/* 	Added by Albert 20110328 */
/* 	This function is used to inform the driver the user had specified the pin code value or pbc */
/* 	to application. */

static int rtw_p2p_got_wpsinfo(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
	struct adapter				*padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct wifidirect_info	*pwdinfo = &( padapter->wdinfo );


	DBG_88E( "[%s] data = %s\n", __FUNCTION__, extra );
	/* 	Added by Albert 20110328 */
	/* 	if the input data is P2P_NO_WPSINFO -> reset the wpsinfo */
	/* 	if the input data is P2P_GOT_WPSINFO_PEER_DISPLAY_PIN -> the utility just input the PIN code got from the peer P2P device. */
	/* 	if the input data is P2P_GOT_WPSINFO_SELF_DISPLAY_PIN -> the utility just got the PIN code from itself. */
	/* 	if the input data is P2P_GOT_WPSINFO_PBC -> the utility just determine to use the PBC */

	if ( *extra == '0' )
	{
		pwdinfo->ui_got_wps_info = P2P_NO_WPSINFO;
	}
	else if ( *extra == '1' )
	{
		pwdinfo->ui_got_wps_info = P2P_GOT_WPSINFO_PEER_DISPLAY_PIN;
	}
	else if ( *extra == '2' )
	{
		pwdinfo->ui_got_wps_info = P2P_GOT_WPSINFO_SELF_DISPLAY_PIN;
	}
	else if ( *extra == '3' )
	{
		pwdinfo->ui_got_wps_info = P2P_GOT_WPSINFO_PBC;
	}
	else
	{
		pwdinfo->ui_got_wps_info = P2P_NO_WPSINFO;
	}

	return ret;

}

#endif /* CONFIG_P2P */

static int rtw_p2p_set(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;
#ifdef CONFIG_P2P

	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;

	DBG_88E( "[%s] extra = %s\n", __FUNCTION__, extra );

	if ( _rtw_memcmp( extra, "enable=", 7 ) )
	{
		rtw_wext_p2p_enable( dev, info, wrqu, &extra[7] );
	}
	else if ( _rtw_memcmp( extra, "setDN=", 6 ) )
	{
		wrqu->data.length -= 6;
		rtw_p2p_setDN( dev, info, wrqu, &extra[6] );
	}
	else if ( _rtw_memcmp( extra, "profilefound=", 13 ) )
	{
		wrqu->data.length -= 13;
		rtw_p2p_profilefound( dev, info, wrqu, &extra[13] );
	}
	else if ( _rtw_memcmp( extra, "prov_disc=", 10 ) )
	{
		wrqu->data.length -= 10;
		rtw_p2p_prov_disc( dev, info, wrqu, &extra[10] );
	}
	else if ( _rtw_memcmp( extra, "nego=", 5 ) )
	{
		wrqu->data.length -= 5;
		rtw_p2p_connect( dev, info, wrqu, &extra[5] );
	}
	else if ( _rtw_memcmp( extra, "intent=", 7 ) )
	{
		/* 	Commented by Albert 2011/03/23 */
		/* 	The wrqu->data.length will include the null character */
		/* 	So, we will decrease 7 + 1 */
		wrqu->data.length -= 8;
		rtw_p2p_set_intent( dev, info, wrqu, &extra[7] );
	}
	else if ( _rtw_memcmp( extra, "ssid=", 5 ) )
	{
		wrqu->data.length -= 5;
		rtw_p2p_set_go_nego_ssid( dev, info, wrqu, &extra[5] );
	}
	else if ( _rtw_memcmp( extra, "got_wpsinfo=", 12 ) )
	{
		wrqu->data.length -= 12;
		rtw_p2p_got_wpsinfo( dev, info, wrqu, &extra[12] );
	}
	else if ( _rtw_memcmp( extra, "listen_ch=", 10 ) )
	{
		/* 	Commented by Albert 2011/05/24 */
		/* 	The wrqu->data.length will include the null character */
		/* 	So, we will decrease (10 + 1) */
		wrqu->data.length -= 11;
		rtw_p2p_set_listen_ch( dev, info, wrqu, &extra[10] );
	}
	else if ( _rtw_memcmp( extra, "op_ch=", 6 ) )
	{
		/* 	Commented by Albert 2011/05/24 */
		/* 	The wrqu->data.length will include the null character */
		/* 	So, we will decrease (6 + 1) */
		wrqu->data.length -= 7;
		rtw_p2p_set_op_ch( dev, info, wrqu, &extra[6] );
	}
	else if ( _rtw_memcmp( extra, "invite=", 7 ) )
	{
		wrqu->data.length -= 8;
		rtw_p2p_invite_req( dev, info, wrqu, &extra[7] );
	}
	else if ( _rtw_memcmp( extra, "persistent=", 11 ) )
	{
		wrqu->data.length -= 11;
		rtw_p2p_set_persistent( dev, info, wrqu, &extra[11] );
	}
	else if ( _rtw_memcmp ( extra, "uuid=", 5) )
	{
		wrqu->data.length -= 5;
		ret = rtw_p2p_set_wps_uuid( dev, info, wrqu, &extra[5] );
	}
#ifdef CONFIG_P2P
	else if ( _rtw_memcmp( extra, "sa=", 3 ) )
	{
		/* 	sa: WFD Session Available information */
		wrqu->data.length -= 3;
		rtw_p2p_set_sa( dev, info, wrqu, &extra[3] );
	}
	else if ( _rtw_memcmp( extra, "pc=", 3 ) )
	{
		/* 	pc: WFD Preferred Connection */
		wrqu->data.length -= 3;
		rtw_p2p_set_pc( dev, info, wrqu, &extra[3] );
	}
	else if ( _rtw_memcmp( extra, "wfd_type=", 9 ) )
	{
		/* 	Specify this device is Mircast source or sink */
		wrqu->data.length -= 9;
		rtw_p2p_set_wfd_device_type( dev, info, wrqu, &extra[9] );
	}
	else if ( _rtw_memcmp( extra, "scan_type=", 10 ) )
	{
		wrqu->data.length -= 10;
		rtw_p2p_set_scan_result_type( dev, info, wrqu, &extra[10] );
	}
	else if ( _rtw_memcmp( extra, "wfd_enable=", 11 ) )
	{
		wrqu->data.length -= 11;
		rtw_p2p_set_wfd_enable( dev, info, wrqu, &extra[11] );
	}
	else if ( _rtw_memcmp( extra, "driver_iface=", 13 ) )
	{
		wrqu->data.length -= 13;
		rtw_p2p_set_driver_iface( dev, info, wrqu, &extra[13] );
	}
#endif /* CONFIG_P2P */
#endif /* CONFIG_P2P */

	return ret;

}

static int rtw_p2p_get(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{

	int ret = 0;

#ifdef CONFIG_P2P

	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct iw_point *pdata = &wrqu->data;
	struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;

	if ( padapter->bShowGetP2PState )
		DBG_88E( "[%s] extra = %s\n", __FUNCTION__, (__force char *)wrqu->data.pointer );

	if ( _rtw_memcmp((__force void *)wrqu->data.pointer, "status", 6 ) ) {
		rtw_p2p_get_status( dev, info, wrqu, extra );
	} else if ( _rtw_memcmp((__force void *)wrqu->data.pointer, "role", 4 ) ) {
		rtw_p2p_get_role( dev, info, wrqu, extra);
	} else if ( _rtw_memcmp((__force void *)wrqu->data.pointer, "peer_ifa", 8 ) ) {
		rtw_p2p_get_peer_ifaddr(dev, info, wrqu, extra);
	} else if ( _rtw_memcmp((__force void *)wrqu->data.pointer, "req_cm", 6 ) ) {
		rtw_p2p_get_req_cm( dev, info, wrqu, extra);
	} else if ( _rtw_memcmp((__force void *)wrqu->data.pointer, "peer_deva", 9 ) ) {
		/* 	Get the P2P device address when receiving the provision discovery request frame. */
		rtw_p2p_get_peer_devaddr((__force void *)dev, info, wrqu, extra);
	} else if ( _rtw_memcmp((__force void *)wrqu->data.pointer, "group_id", 8 ) ) {
		rtw_p2p_get_groupid( dev, info, wrqu, extra);
	} else if ( _rtw_memcmp((__force void *)wrqu->data.pointer, "inv_peer_deva", 13 ) ) {
		/* 	Get the P2P device address when receiving the P2P Invitation request frame. */
		rtw_p2p_get_peer_devaddr_by_invitation( dev, info, wrqu, extra);
	} else if ( _rtw_memcmp((__force void *)wrqu->data.pointer, "op_ch", 5 ) ) {
		rtw_p2p_get_op_ch( dev, info, wrqu, extra);
	}
#ifdef CONFIG_P2P
	else if ( _rtw_memcmp((__force void *)wrqu->data.pointer, "peer_port", 9 ) ) {
		rtw_p2p_get_peer_wfd_port( dev, info, wrqu, extra );
	} else if ( _rtw_memcmp((__force void *)wrqu->data.pointer, "wfd_sa", 6 ) ) {
		rtw_p2p_get_peer_wfd_session_available( dev, info, wrqu, extra );
	} else if ( _rtw_memcmp((__force void *)wrqu->data.pointer, "wfd_pc", 6 ) ) {
		rtw_p2p_get_peer_wfd_preferred_connection( dev, info, wrqu, extra );
	}
#endif /*  CONFIG_P2P */

#endif /* CONFIG_P2P */

	return ret;

}

static int rtw_p2p_get2(struct net_device *dev,
						struct iw_request_info *info,
						union iwreq_data *wrqu, char *extra)
{

	int ret = 0;

#ifdef CONFIG_P2P

	int length = wrqu->data.length;
	char *buffer = (u8 *)rtw_malloc(length);

	if (buffer == NULL)
	{
		ret = -ENOMEM;
		goto bad;
	}

	if (copy_from_user(buffer, wrqu->data.pointer, wrqu->data.length))
	{
		ret = -EFAULT;
		goto bad;
	}

	DBG_88E("[%s] buffer = %s\n", __FUNCTION__, buffer);

	if (_rtw_memcmp(buffer, "wpsCM=", 6))
	{
		ret = rtw_p2p_get_wps_configmethod(dev, info, wrqu, extra, &buffer[6]);
	} else if (_rtw_memcmp(buffer, "devN=", 5))
	{
		ret = rtw_p2p_get_device_name(dev, info, wrqu, extra, &buffer[5]);
	} else if (_rtw_memcmp(buffer, "dev_type=", 9))
	{
		ret = rtw_p2p_get_device_type(dev, info, wrqu, extra, &buffer[9]);
	} else if (_rtw_memcmp(buffer, "go_devadd=", 10))
	{
		ret = rtw_p2p_get_go_device_address(dev, info, wrqu, extra, &buffer[10]);
	} else if (_rtw_memcmp(buffer, "InvProc=", 8))
	{
		ret = rtw_p2p_get_invitation_procedure(dev, info, wrqu, extra, &buffer[8]);
	} else
	{
		snprintf(extra, sizeof("Command not found."), "Command not found.");
		wrqu->data.length = strlen(extra);
	}

bad:
	if (buffer)
	{
		rtw_mfree(buffer, length);
	}

#endif /* CONFIG_P2P */

	return ret;

}

static int rtw_cta_test_start(struct net_device *dev,
							   struct iw_request_info *info,
							   union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	struct adapter	*padapter = (struct adapter *)rtw_netdev_priv(dev);
	DBG_88E("%s %s\n", __func__, extra);
	if (!strcmp(extra, "1"))
		padapter->in_cta_test = 1;
	else
		padapter->in_cta_test = 0;

	if (padapter->in_cta_test)
	{
		u32 v = rtw_read32(padapter, REG_RCR);
		v &= ~(RCR_CBSSID_DATA | RCR_CBSSID_BCN );/*  RCR_ADF */
		rtw_write32(padapter, REG_RCR, v);
		DBG_88E("enable RCR_ADF\n");
	}
	else
	{
		u32 v = rtw_read32(padapter, REG_RCR);
		v |= RCR_CBSSID_DATA | RCR_CBSSID_BCN ;/*  RCR_ADF */
		rtw_write32(padapter, REG_RCR, v);
		DBG_88E("disable RCR_ADF\n");
	}
	return ret;
}


extern int rtw_change_ifname(struct adapter *padapter, const char *ifname);
static int rtw_rereg_nd_name(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	struct adapter *padapter = rtw_netdev_priv(dev);
	struct rereg_nd_name_data *rereg_priv = &padapter->rereg_nd_name_priv;
	char new_ifname[IFNAMSIZ];

	if (rereg_priv->old_ifname[0] == 0) {
		char *reg_ifname;
		reg_ifname = padapter->registrypriv.ifname;

		strncpy(rereg_priv->old_ifname, reg_ifname, IFNAMSIZ);
		rereg_priv->old_ifname[IFNAMSIZ-1] = 0;
	}

	if (wrqu->data.length > IFNAMSIZ)
		return -EFAULT;

	if ( copy_from_user(new_ifname, wrqu->data.pointer, IFNAMSIZ) ) {
		return -EFAULT;
	}

	if ( 0 == strcmp(rereg_priv->old_ifname, new_ifname) ) {
		return ret;
	}

	DBG_88E("%s new_ifname:%s\n", __FUNCTION__, new_ifname);
	if ( 0 != (ret = rtw_change_ifname(padapter, new_ifname)) ) {
		goto exit;
	}

	if (_rtw_memcmp(rereg_priv->old_ifname, "disable%d", 9) == true) {
		padapter->ledpriv.bRegUseLed= rereg_priv->old_bRegUseLed;
		rtw_hal_sw_led_init(padapter);
		/* rtw_ips_mode_req(&padapter->pwrctrlpriv, rereg_priv->old_ips_mode); */
	}

	strncpy(rereg_priv->old_ifname, new_ifname, IFNAMSIZ);
	rereg_priv->old_ifname[IFNAMSIZ-1] = 0;

	if (_rtw_memcmp(new_ifname, "disable%d", 9) == true) {

		DBG_88E("%s disable\n", __FUNCTION__);
		/*  free network queue for Android's timming issue */
		rtw_free_network_queue(padapter, true);

		/*  close led */
		rtw_led_control(padapter, LED_CTL_POWER_OFF);
		rereg_priv->old_bRegUseLed = padapter->ledpriv.bRegUseLed;
		padapter->ledpriv.bRegUseLed= false;
		rtw_hal_sw_led_deinit(padapter);
	}
exit:
	return ret;

}

static void mac_reg_dump(struct adapter *padapter)
{
	int i,j=1;
	printk("\n======= MAC REG =======\n");
	for (i=0x0;i<0x300;i+=4)
	{
		if (j%4==1)	printk("0x%02x",i);
		printk(" 0x%08x ",rtw_read32(padapter,i));
		if ((j++)%4 == 0)	printk("\n");
	}
	for (i=0x400;i<0x800;i+=4)
	{
		if (j%4==1)	printk("0x%02x",i);
		printk(" 0x%08x ",rtw_read32(padapter,i));
		if ((j++)%4 == 0)	printk("\n");
	}
}

static void bb_reg_dump(struct adapter *padapter)
{
	int i,j=1;
	printk("\n======= BB REG =======\n");
	for (i=0x800;i<0x1000;i+=4)
	{
		if (j%4==1) printk("0x%02x",i);

		printk(" 0x%08x ",rtw_read32(padapter,i));
		if ((j++)%4 == 0)	printk("\n");
	}
}

static void rf_reg_dump(struct adapter *padapter)
{
	int i,j=1,path;
	u32 value;
	u8 rf_type,path_nums = 0;
	rtw_hal_get_hwreg(padapter, HW_VAR_RF_TYPE, (u8 *)(&rf_type));

	printk("\n======= RF REG =======\n");
	if ((RF_1T2R == rf_type) ||(RF_1T1R ==rf_type ))
		path_nums = 1;
	else
		path_nums = 2;

	for (path=0;path<path_nums;path++)
	{
		printk("\nRF_Path(%x)\n",path);
		for (i=0;i<0x100;i++)
		{
			/* value = PHY_QueryRFReg(padapter, (enum rf_radio_path)path,i, bMaskDWord); */
			value = rtw_hal_read_rfreg(padapter, path, i, 0xffffffff);
			if (j%4==1)	printk("0x%02x ",i);
			printk(" 0x%08x ",value);
			if ((j++)%4== 0)	printk("\n");
		}
	}
}

static int rtw_dbg_port(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	unsigned long irqL;
	int ret = 0;
	u8 major_cmd, minor_cmd;
	u16 arg;
	u32 extra_arg, *pdata, val32;
	struct sta_info *psta;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct wlan_network *cur_network = &(pmlmepriv->cur_network);
	struct sta_priv *pstapriv = &padapter->stapriv;


	pdata = (u32*)&wrqu->data;

	val32 = *pdata;
	arg = (u16)(val32&0x0000ffff);
	major_cmd = (u8)(val32>>24);
	minor_cmd = (u8)((val32>>16)&0x00ff);

	extra_arg = *(pdata+1);

	switch (major_cmd)
	{
		case 0x70:/* read_reg */
			switch (minor_cmd)
			{
				case 1:
					DBG_88E("rtw_read8(0x%x)=0x%02x\n", arg, rtw_read8(padapter, arg));
					break;
				case 2:
					DBG_88E("rtw_read16(0x%x)=0x%04x\n", arg, rtw_read16(padapter, arg));
					break;
				case 4:
					DBG_88E("rtw_read32(0x%x)=0x%08x\n", arg, rtw_read32(padapter, arg));
					break;
			}
			break;
		case 0x71:/* write_reg */
			switch (minor_cmd)
			{
				case 1:
					rtw_write8(padapter, arg, extra_arg);
					DBG_88E("rtw_write8(0x%x)=0x%02x\n", arg, rtw_read8(padapter, arg));
					break;
				case 2:
					rtw_write16(padapter, arg, extra_arg);
					DBG_88E("rtw_write16(0x%x)=0x%04x\n", arg, rtw_read16(padapter, arg));
					break;
				case 4:
					rtw_write32(padapter, arg, extra_arg);
					DBG_88E("rtw_write32(0x%x)=0x%08x\n", arg, rtw_read32(padapter, arg));
					break;
			}
			break;
		case 0x72:/* read_bb */
			DBG_88E("read_bbreg(0x%x)=0x%x\n", arg, rtw_hal_read_bbreg(padapter, arg, 0xffffffff));
			break;
		case 0x73:/* write_bb */
			rtw_hal_write_bbreg(padapter, arg, 0xffffffff, extra_arg);
			DBG_88E("write_bbreg(0x%x)=0x%x\n", arg, rtw_hal_read_bbreg(padapter, arg, 0xffffffff));
			break;
		case 0x74:/* read_rf */
			DBG_88E("read RF_reg path(0x%02x),offset(0x%x),value(0x%08x)\n",minor_cmd,arg,rtw_hal_read_rfreg(padapter, minor_cmd, arg, 0xffffffff));
			break;
		case 0x75:/* write_rf */
			rtw_hal_write_rfreg(padapter, minor_cmd, arg, 0xffffffff, extra_arg);
			DBG_88E("write RF_reg path(0x%02x),offset(0x%x),value(0x%08x)\n",minor_cmd,arg, rtw_hal_read_rfreg(padapter, minor_cmd, arg, 0xffffffff));
			break;
		case 0x76:
			switch (minor_cmd) {
			case 0x00: /* normal mode, */
				padapter->recvpriv.is_signal_dbg = 0;
				break;
			case 0x01: /* dbg mode */
				padapter->recvpriv.is_signal_dbg = 1;
				extra_arg = extra_arg > 100 ? 100 : extra_arg;
				padapter->recvpriv.signal_strength_dbg=extra_arg;
				break;
			}
			break;
		case 0x78: /* IOL test */
			switch (minor_cmd)
			{
				case 0x04: /* LLT table initialization test */
				{
					u8 page_boundary = 0xf9;
					{
						struct xmit_frame	*xmit_frame;

						if ((xmit_frame=rtw_IOL_accquire_xmit_frame(padapter)) == NULL) {
							ret = -ENOMEM;
							break;
						}

						rtw_IOL_append_LLT_cmd(xmit_frame, page_boundary);


						if (_SUCCESS != rtw_IOL_exec_cmds_sync(padapter, xmit_frame, 500,0) )
							ret = -EPERM;
					}
				}
					break;
				case 0x05: /* blink LED test */
				{
					u16 reg = 0x4c;
					u32 blink_num = 50;
					u32 blink_delay_ms = 200;
					int i;

					{
						struct xmit_frame	*xmit_frame;

						if ((xmit_frame=rtw_IOL_accquire_xmit_frame(padapter)) == NULL) {
							ret = -ENOMEM;
							break;
						}

						for (i=0;i<blink_num;i++) {
							rtw_IOL_append_WB_cmd(xmit_frame, reg, 0x00,0xff);
							rtw_IOL_append_DELAY_MS_cmd(xmit_frame, blink_delay_ms);
							rtw_IOL_append_WB_cmd(xmit_frame, reg, 0x08,0xff);
							rtw_IOL_append_DELAY_MS_cmd(xmit_frame, blink_delay_ms);
						}
						if (_SUCCESS != rtw_IOL_exec_cmds_sync(padapter, xmit_frame, (blink_delay_ms*blink_num*2)+200,0) )
							ret = -EPERM;
					}
				}
					break;
				case 0x06: /* continuous wirte byte test */
				{
					u16 reg = arg;
					u16 start_value = 0;
					u32 write_num = extra_arg;
					int i;
					u8 final;

					{
						struct xmit_frame	*xmit_frame;

						if ((xmit_frame=rtw_IOL_accquire_xmit_frame(padapter)) == NULL) {
							ret = -ENOMEM;
							break;
						}

						for (i=0;i<write_num;i++) {
							rtw_IOL_append_WB_cmd(xmit_frame, reg, i+start_value,0xFF);
						}
						if (_SUCCESS != rtw_IOL_exec_cmds_sync(padapter, xmit_frame, 5000,0))
							ret = -EPERM;
					}

					if (start_value+write_num-1 == (final=rtw_read8(padapter, reg)) ) {
						DBG_88E("continuous IOL_CMD_WB_REG to 0x%x %u times Success, start:%u, final:%u\n", reg, write_num, start_value, final);
					} else {
						DBG_88E("continuous IOL_CMD_WB_REG to 0x%x %u times Fail, start:%u, final:%u\n", reg, write_num, start_value, final);
					}
				}
					break;

				case 0x07: /* continuous wirte word test */
				{
					u16 reg = arg;
					u16 start_value = 200;
					u32 write_num = extra_arg;

					int i;
					u16 final;

					{
						struct xmit_frame	*xmit_frame;

						if ((xmit_frame=rtw_IOL_accquire_xmit_frame(padapter)) == NULL) {
							ret = -ENOMEM;
							break;
						}

						for (i=0;i<write_num;i++) {
							rtw_IOL_append_WW_cmd(xmit_frame, reg, i+start_value,0xFFFF);
						}
						if (_SUCCESS !=rtw_IOL_exec_cmds_sync(padapter, xmit_frame, 5000,0))
							ret = -EPERM;
					}

					if (start_value+write_num-1 == (final=rtw_read16(padapter, reg)) ) {
						DBG_88E("continuous IOL_CMD_WW_REG to 0x%x %u times Success, start:%u, final:%u\n", reg, write_num, start_value, final);
					} else {
						DBG_88E("continuous IOL_CMD_WW_REG to 0x%x %u times Fail, start:%u, final:%u\n", reg, write_num, start_value, final);
					}
				}
					break;

				case 0x08: /* continuous wirte dword test */
				{
					u16 reg = arg;
					u32 start_value = 0x110000c7;
					u32 write_num = extra_arg;

					int i;
					u32 final;

					{
						struct xmit_frame	*xmit_frame;

						if ((xmit_frame=rtw_IOL_accquire_xmit_frame(padapter)) == NULL) {
							ret = -ENOMEM;
							break;
						}

						for (i=0;i<write_num;i++) {
							rtw_IOL_append_WD_cmd(xmit_frame, reg, i+start_value,0xFFFFFFFF);
						}
						if (_SUCCESS !=rtw_IOL_exec_cmds_sync(padapter, xmit_frame, 5000,0))
							ret = -EPERM;

					}

					if (start_value+write_num-1 == (final=rtw_read32(padapter, reg)) ) {
						DBG_88E("continuous IOL_CMD_WD_REG to 0x%x %u times Success, start:%u, final:%u\n", reg, write_num, start_value, final);
					} else {
						DBG_88E("continuous IOL_CMD_WD_REG to 0x%x %u times Fail, start:%u, final:%u\n", reg, write_num, start_value, final);
					}
				}
					break;
			}
			break;
		case 0x79:
			{
				/*
				* dbg 0x79000000 [value], set RESP_TXAGC to + value, value:0~15
				* dbg 0x79010000 [value], set RESP_TXAGC to - value, value:0~15
				*/
				u8 value =  extra_arg & 0x0f;
				u8 sign = minor_cmd;
				u16 write_value = 0;

				DBG_88E("%s set RESP_TXAGC to %s %u\n", __func__, sign?"minus":"plus", value);

				if (sign)
					value = value | 0x10;

				write_value = value | (value << 5);
				rtw_write16(padapter, 0x6d9, write_value);
			}
			break;
		case 0x7a:
			receive_disconnect(padapter, pmlmeinfo->network.MacAddress
				, WLAN_REASON_EXPIRATION_CHK);
			break;
		case 0x7F:
			switch (minor_cmd)
			{
				case 0x0:
					DBG_88E("fwstate=0x%x\n", get_fwstate(pmlmepriv));
					break;
				case 0x01:
					DBG_88E("auth_alg=0x%x, enc_alg=0x%x, auth_type=0x%x, enc_type=0x%x\n",
						psecuritypriv->dot11AuthAlgrthm, psecuritypriv->dot11PrivacyAlgrthm,
						psecuritypriv->ndisauthtype, psecuritypriv->ndisencryptstatus);
					break;
				case 0x02:
					DBG_88E("pmlmeinfo->state=0x%x\n", pmlmeinfo->state);
					break;
				case 0x03:
					DBG_88E("qos_option=%d\n", pmlmepriv->qospriv.qos_option);
					DBG_88E("ht_option=%d\n", pmlmepriv->htpriv.ht_option);
					break;
				case 0x04:
					DBG_88E("cur_ch=%d\n", pmlmeext->cur_channel);
					DBG_88E("cur_bw=%d\n", pmlmeext->cur_bwmode);
					DBG_88E("cur_ch_off=%d\n", pmlmeext->cur_ch_offset);
					break;
				case 0x05:
					psta = rtw_get_stainfo(pstapriv, cur_network->network.MacAddress);
					if (psta)
					{
						int i;
						struct recv_reorder_ctrl *preorder_ctrl;

						DBG_88E("SSID=%s\n", cur_network->network.Ssid.Ssid);
						DBG_88E("sta's macaddr:" MAC_FMT "\n", MAC_ARG(psta->hwaddr));
						DBG_88E("cur_channel=%d, cur_bwmode=%d, cur_ch_offset=%d\n", pmlmeext->cur_channel, pmlmeext->cur_bwmode, pmlmeext->cur_ch_offset);
						DBG_88E("rtsen=%d, cts2slef=%d\n", psta->rtsen, psta->cts2self);
						DBG_88E("state=0x%x, aid=%d, macid=%d, raid=%d\n", psta->state, psta->aid, psta->mac_id, psta->raid);
						DBG_88E("qos_en=%d, ht_en=%d, init_rate=%d\n", psta->qos_option, psta->htpriv.ht_option, psta->init_rate);
						DBG_88E("bwmode=%d, ch_offset=%d, sgi=%d\n", psta->htpriv.bwmode, psta->htpriv.ch_offset, psta->htpriv.sgi);
						DBG_88E("ampdu_enable = %d\n", psta->htpriv.ampdu_enable);
						DBG_88E("agg_enable_bitmap=%x, candidate_tid_bitmap=%x\n", psta->htpriv.agg_enable_bitmap, psta->htpriv.candidate_tid_bitmap);

						for (i=0;i<16;i++)
						{
							preorder_ctrl = &psta->recvreorder_ctrl[i];
							if (preorder_ctrl->enable)
							{
								DBG_88E("tid=%d, indicate_seq=%d\n", i, preorder_ctrl->indicate_seq);
							}
						}

					}
					else
					{
						DBG_88E("can't get sta's macaddr, cur_network's macaddr:" MAC_FMT "\n", MAC_ARG(cur_network->network.MacAddress));
					}
					break;
				case 0x06:
					{
						u32	ODMFlag;
						rtw_hal_get_hwreg(padapter, HW_VAR_DM_FLAG, (u8*)(&ODMFlag));
						DBG_88E("(B)DMFlag=0x%x, arg=0x%x\n", ODMFlag, arg);
						ODMFlag = (u32)(0x0f&arg);
						DBG_88E("(A)DMFlag=0x%x\n", ODMFlag);
						rtw_hal_set_hwreg(padapter, HW_VAR_DM_FLAG, (u8 *)(&ODMFlag));
					}
					break;
				case 0x07:
					DBG_88E("bSurpriseRemoved=%d, bDriverStopped=%d\n",
						padapter->bSurpriseRemoved, padapter->bDriverStopped);
					break;
				case 0x08:
					{
						struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
						struct recv_priv  *precvpriv = &padapter->recvpriv;

						DBG_88E("free_xmitbuf_cnt=%d, free_xmitframe_cnt=%d"
							", free_xmit_extbuf_cnt=%d, free_xframe_ext_cnt=%d"
							", free_recvframe_cnt=%d\n",
							pxmitpriv->free_xmitbuf_cnt, pxmitpriv->free_xmitframe_cnt,
							pxmitpriv->free_xmit_extbuf_cnt, pxmitpriv->free_xframe_ext_cnt,
							precvpriv->free_recvframe_cnt);
						DBG_88E("rx_urb_pending_cn=%d\n", precvpriv->rx_pending_cnt);
					}
					break;
				case 0x09:
					{
						int i, j;
						struct list_head *plist, *phead;
						struct recv_reorder_ctrl *preorder_ctrl;

#ifdef CONFIG_AP_MODE
						DBG_88E("sta_dz_bitmap=0x%x, tim_bitmap=0x%x\n", pstapriv->sta_dz_bitmap, pstapriv->tim_bitmap);
#endif
						spin_lock_bh(&pstapriv->sta_hash_lock);

						for (i=0; i< NUM_STA; i++)
						{
							phead = &(pstapriv->sta_hash[i]);
							plist = get_next(phead);

							while ((rtw_end_of_queue_search(phead, plist)) == false)
							{
								psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);

								plist = get_next(plist);

								if (extra_arg == psta->aid)
								{
									DBG_88E("sta's macaddr:" MAC_FMT "\n", MAC_ARG(psta->hwaddr));
									DBG_88E("rtsen=%d, cts2slef=%d\n", psta->rtsen, psta->cts2self);
									DBG_88E("state=0x%x, aid=%d, macid=%d, raid=%d\n", psta->state, psta->aid, psta->mac_id, psta->raid);
									DBG_88E("qos_en=%d, ht_en=%d, init_rate=%d\n", psta->qos_option, psta->htpriv.ht_option, psta->init_rate);
									DBG_88E("bwmode=%d, ch_offset=%d, sgi=%d\n", psta->htpriv.bwmode, psta->htpriv.ch_offset, psta->htpriv.sgi);
									DBG_88E("ampdu_enable = %d\n", psta->htpriv.ampdu_enable);
									DBG_88E("agg_enable_bitmap=%x, candidate_tid_bitmap=%x\n", psta->htpriv.agg_enable_bitmap, psta->htpriv.candidate_tid_bitmap);

#ifdef CONFIG_AP_MODE
									DBG_88E("capability=0x%x\n", psta->capability);
									DBG_88E("flags=0x%x\n", psta->flags);
									DBG_88E("wpa_psk=0x%x\n", psta->wpa_psk);
									DBG_88E("wpa2_group_cipher=0x%x\n", psta->wpa2_group_cipher);
									DBG_88E("wpa2_pairwise_cipher=0x%x\n", psta->wpa2_pairwise_cipher);
									DBG_88E("qos_info=0x%x\n", psta->qos_info);
#endif
									DBG_88E("dot118021XPrivacy=0x%x\n", psta->dot118021XPrivacy);



									for (j=0;j<16;j++)
									{
										preorder_ctrl = &psta->recvreorder_ctrl[j];
										if (preorder_ctrl->enable)
										{
											DBG_88E("tid=%d, indicate_seq=%d\n", j, preorder_ctrl->indicate_seq);
										}
									}

								}

							}
						}

						spin_unlock_bh(&pstapriv->sta_hash_lock);

					}
					break;
					case 0x0a:
					{
						#ifdef DBG_TRX_STA_PKTS
						int i, j;
						struct list_head *plist, *phead;

						spin_lock_bh(&pstapriv->sta_hash_lock);

						for (i=0; i< NUM_STA; i++)
						{
							phead = &(pstapriv->sta_hash[i]);
							plist = get_next(phead);

							while ((rtw_end_of_queue_search(phead, plist)) == false)
							{
								psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);

								plist = get_next(plist);
								if (arg == 0xff) {
									psta->rx_bk_cnt =0;
									psta->rx_be_cnt =0;
									psta->rx_vo_cnt =0;
									psta->rx_vi_cnt =0;
									psta->tx_bk_cnt =0;
									psta->tx_be_cnt =0;
									psta->tx_vo_cnt =0;
									psta->tx_vi_cnt =0;
								}
								else {
									if (extra_arg == psta->mac_id)
									{
										DBG_88E("=== sta's macaddr:" MAC_FMT "===\n", MAC_ARG(psta->hwaddr));
										DBG_88E("rx_bk_cnt =%d\n", psta->rx_bk_cnt);
										DBG_88E("rx_be_cnt =%d\n", psta->rx_be_cnt);
										DBG_88E("rx_vo_cnt =%d\n", psta->rx_vo_cnt);
										DBG_88E("rx_vi_cnt =%d\n\n", psta->rx_vi_cnt);

										DBG_88E("tx_bk_cnt =%d\n", psta->tx_bk_cnt);
										DBG_88E("tx_be_cnt =%d\n", psta->tx_be_cnt);
										DBG_88E("tx_vo_cnt =%d\n", psta->tx_vo_cnt);
										DBG_88E("tx_vi_cnt =%dn\n", psta->tx_vi_cnt);
									}
								}

							}
						}
						spin_unlock_bh(&pstapriv->sta_hash_lock);
						#endif
					}
					break;

				case 0x0c:/* dump rx/tx packet */
					{
						if (arg == 0) {
							DBG_88E("dump rx packet (%d)\n",extra_arg);
							/* pHalData->bDumpRxPkt =extra_arg; */
							rtw_hal_set_def_var(padapter, HAL_DEF_DBG_DUMP_RXPKT, &(extra_arg));
						}
						else if (arg==1) {
							DBG_88E("dump tx packet (%d)\n",extra_arg);
							rtw_hal_set_def_var(padapter, HAL_DEF_DBG_DUMP_TXPKT, &(extra_arg));
						}
					}
					break;
				case 0x0f:
						{
							if (extra_arg == 0) {
								DBG_88E("###### silent reset test.......#####\n");
								rtw_hal_sreset_reset(padapter);
							} else {
								HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
								struct sreset_priv *psrtpriv = &pHalData->srestpriv;
								psrtpriv->dbg_trigger_point = extra_arg;
							}

						}
				break;
				case 0x15:
					{
						struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
						DBG_88E("==>silent resete cnts:%d\n",pwrpriv->ips_enter_cnts);
					}
					break;
				case 0x10:/*  driver version display */
					DBG_88E("rtw driver version=%s\n", DRIVERVERSION);
					break;
				case 0x11:
					DBG_88E("turn %s Rx RSSI display function\n",(extra_arg==1)?"on":"off");
					padapter->bRxRSSIDisplay = extra_arg;
					rtw_hal_set_def_var(padapter, HW_DEF_FA_CNT_DUMP, &(padapter->bRxRSSIDisplay));
					break;
				case 0x12: /* set rx_stbc */
				{
					struct registry_priv	*pregpriv = &padapter->registrypriv;
					/*  0: disable, bit(0):enable 2.4g, bit(1):enable 5g, 0x3: enable both 2.4g and 5g */
					/* default is set to enable 2.4GHZ for IOT issue with bufflao's AP at 5GHZ */
					if ( pregpriv && (extra_arg == 0 || extra_arg == 1|| extra_arg == 2 || extra_arg == 3))
					{
						pregpriv->rx_stbc= extra_arg;
						DBG_88E("set rx_stbc=%d\n",pregpriv->rx_stbc);
					}
					else
						DBG_88E("get rx_stbc=%d\n",pregpriv->rx_stbc);

				}
				break;
				case 0x13: /* set ampdu_enable */
				{
					struct registry_priv	*pregpriv = &padapter->registrypriv;
					/*  0: disable, 0x1:enable (but wifi_spec should be 0), 0x2: force enable (don't care wifi_spec) */
					if ( pregpriv && extra_arg >= 0 && extra_arg < 3 )
					{
						pregpriv->ampdu_enable= extra_arg;
						DBG_88E("set ampdu_enable=%d\n",pregpriv->ampdu_enable);
					}
					else
						DBG_88E("get ampdu_enable=%d\n",pregpriv->ampdu_enable);

				}
				break;
				case 0x14: /* get wifi_spec */
				{
					struct registry_priv	*pregpriv = &padapter->registrypriv;
					DBG_88E("get wifi_spec=%d\n",pregpriv->wifi_spec);

				}
				break;
				case 0x16:
				{
					if (arg == 0xff) {
						rtw_odm_dbg_comp_msg(padapter);
					}
					else {
						u64 dbg_comp = (u64)extra_arg;
						rtw_odm_dbg_comp_set(padapter, dbg_comp);
					}
				}
					break;
#ifdef DBG_FIXED_CHAN
				case 0x17:
					{
						struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
						printk("===>  Fixed channel to %d\n",extra_arg);
						pmlmeext->fixed_chan = extra_arg;

					}
					break;
#endif
				case 0x20:
					{
						rtw_hal_get_hwreg(padapter, HW_VAR_READ_LLT_TAB,(u8 *)&extra_arg);
					}
					break;
				case 0x23:
					{
						DBG_88E("turn %s the bNotifyChannelChange Variable\n",(extra_arg==1)?"on":"off");
						padapter->bNotifyChannelChange = extra_arg;
						break;
					}
				case 0x24:
					{
#ifdef CONFIG_P2P
						DBG_88E("turn %s the bShowGetP2PState Variable\n",(extra_arg==1)?"on":"off");
						padapter->bShowGetP2PState = extra_arg;
#endif /*  CONFIG_P2P */
						break;
					}
				case 0xaa:
					{
						if (extra_arg> 0x13) extra_arg = 0xFF;
						DBG_88E("chang data rate to :0x%02x\n",extra_arg);
						padapter->fix_rate = extra_arg;
					}
					break;
				case 0xdd:/* registers dump , 0 for mac reg,1 for bb reg, 2 for rf reg */
					{
						if (extra_arg== 0) {
							mac_reg_dump(padapter);
						}
						else if (extra_arg==1) {
							bb_reg_dump(padapter);
						}
						else if (extra_arg==2) {
							rf_reg_dump(padapter);
						}

					}
					break;

				case 0xee:/* turn on/off dynamic funcs */
					{
						u32 odm_flag;

						if (0xf==extra_arg) {
							rtw_hal_get_def_var(padapter, HAL_DEF_DBG_DM_FUNC,&odm_flag);
							DBG_88E(" === DMFlag(0x%08x) ===\n",odm_flag);
							DBG_88E("extra_arg = 0  - disable all dynamic func\n");
							DBG_88E("extra_arg = 1  - disable DIG- BIT(0)\n");
							DBG_88E("extra_arg = 2  - disable High power - BIT(1)\n");
							DBG_88E("extra_arg = 3  - disable tx power tracking - BIT(2)\n");
							DBG_88E("extra_arg = 4  - disable BT coexistence - BIT(3)\n");
							DBG_88E("extra_arg = 5  - disable antenna diversity - BIT(4)\n");
							DBG_88E("extra_arg = 6  - enable all dynamic func\n");
						}
						else {
							/*	extra_arg = 0  - disable all dynamic func
								extra_arg = 1  - disable DIG
								extra_arg = 2  - disable tx power tracking
								extra_arg = 3  - turn on all dynamic func
							*/
							rtw_hal_set_def_var(padapter, HAL_DEF_DBG_DM_FUNC, &(extra_arg));
							rtw_hal_get_def_var(padapter, HAL_DEF_DBG_DM_FUNC,&odm_flag);
							DBG_88E(" === DMFlag(0x%08x) ===\n",odm_flag);
						}
					}
					break;

				case 0xfd:
					rtw_write8(padapter, 0xc50, arg);
					DBG_88E("wr(0xc50)=0x%x\n", rtw_read8(padapter, 0xc50));
					rtw_write8(padapter, 0xc58, arg);
					DBG_88E("wr(0xc58)=0x%x\n", rtw_read8(padapter, 0xc58));
					break;
				case 0xfe:
					DBG_88E("rd(0xc50)=0x%x\n", rtw_read8(padapter, 0xc50));
					DBG_88E("rd(0xc58)=0x%x\n", rtw_read8(padapter, 0xc58));
					break;
				case 0xff:
					{
						DBG_88E("dbg(0x210)=0x%x\n", rtw_read32(padapter, 0x210));
						DBG_88E("dbg(0x608)=0x%x\n", rtw_read32(padapter, 0x608));
						DBG_88E("dbg(0x280)=0x%x\n", rtw_read32(padapter, 0x280));
						DBG_88E("dbg(0x284)=0x%x\n", rtw_read32(padapter, 0x284));
						DBG_88E("dbg(0x288)=0x%x\n", rtw_read32(padapter, 0x288));

						DBG_88E("dbg(0x664)=0x%x\n", rtw_read32(padapter, 0x664));


						DBG_88E("\n");

						DBG_88E("dbg(0x430)=0x%x\n", rtw_read32(padapter, 0x430));
						DBG_88E("dbg(0x438)=0x%x\n", rtw_read32(padapter, 0x438));

						DBG_88E("dbg(0x440)=0x%x\n", rtw_read32(padapter, 0x440));

						DBG_88E("dbg(0x458)=0x%x\n", rtw_read32(padapter, 0x458));

						DBG_88E("dbg(0x484)=0x%x\n", rtw_read32(padapter, 0x484));
						DBG_88E("dbg(0x488)=0x%x\n", rtw_read32(padapter, 0x488));

						DBG_88E("dbg(0x444)=0x%x\n", rtw_read32(padapter, 0x444));
						DBG_88E("dbg(0x448)=0x%x\n", rtw_read32(padapter, 0x448));
						DBG_88E("dbg(0x44c)=0x%x\n", rtw_read32(padapter, 0x44c));
						DBG_88E("dbg(0x450)=0x%x\n", rtw_read32(padapter, 0x450));
					}
					break;
			}
			break;
		default:
			DBG_88E("error dbg cmd!\n");
			break;
	}


	return ret;

}

static int wpa_set_param(struct net_device *dev, u8 name, u32 value)
{
	uint ret=0;
	u32 flags;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);

	switch (name) {
	case IEEE_PARAM_WPA_ENABLED:
		padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X; /* 802.1x */

		switch ((value)&0xff) {
		case 1 : /* WPA */
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeWPAPSK; /* WPA_PSK */
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
			break;
		case 2: /* WPA2 */
			padapter->securitypriv.ndisauthtype = Ndis802_11AuthModeWPA2PSK; /* WPA2_PSK */
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
			break;
		}
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_info_,("wpa_set_param:padapter->securitypriv.ndisauthtype=%d\n", padapter->securitypriv.ndisauthtype));
		break;
	case IEEE_PARAM_TKIP_COUNTERMEASURES:
		/* ieee->tkip_countermeasures=value; */
		break;
	case IEEE_PARAM_DROP_UNENCRYPTED:
		/* HACK:
		 *
		 * wpa_supplicant calls set_wpa_enabled when the driver
		 * is loaded and unloaded, regardless of if WPA is being
		 * used.  No other calls are made which can be used to
		 * determine if encryption will be used or not prior to
		 * association being expected.  If encryption is not being
		 * used, drop_unencrypted is set to false, else true -- we
		 * can use this to determine if the CAP_PRIVACY_ON bit should
		 * be set.
		 */

		break;
	case IEEE_PARAM_PRIVACY_INVOKED:
		break;
	case IEEE_PARAM_AUTH_ALGS:
		ret = wpa_set_auth_algs(dev, value);
		break;
	case IEEE_PARAM_IEEE_802_1X:
		break;
	case IEEE_PARAM_WPAX_SELECT:
		break;
	default:
		ret = -EOPNOTSUPP;
		break;
	}
	return ret;
}

static int wpa_mlme(struct net_device *dev, u32 command, u32 reason)
{
	int ret = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);

	switch (command)
	{
		case IEEE_MLME_STA_DEAUTH:

			if (!rtw_set_802_11_disassociate(padapter))
				ret = -1;

			break;

		case IEEE_MLME_STA_DISASSOC:

			if (!rtw_set_802_11_disassociate(padapter))
				ret = -1;

			break;

		default:
			ret = -EOPNOTSUPP;
			break;
	}

	return ret;

}

static int wpa_supplicant_ioctl(struct net_device *dev, struct iw_point *p)
{
	struct ieee_param *param;
	uint ret=0;

	/* down(&ieee->wx_sem); */

	if (p->length < sizeof(struct ieee_param) || !p->pointer) {
		ret = -EINVAL;
		goto out;
	}

	param = (struct ieee_param *)rtw_malloc(p->length);
	if (param == NULL)
	{
		ret = -ENOMEM;
		goto out;
	}

	if (copy_from_user(param, p->pointer, p->length))
	{
		rtw_mfree((u8*)param, p->length);
		ret = -EFAULT;
		goto out;
	}

	switch (param->cmd) {

	case IEEE_CMD_SET_WPA_PARAM:
		ret = wpa_set_param(dev, param->u.wpa_param.name, param->u.wpa_param.value);
		break;

	case IEEE_CMD_SET_WPA_IE:
		/* ret = wpa_set_wpa_ie(dev, param, p->length); */
		ret =  rtw_set_wpa_ie((struct adapter *)rtw_netdev_priv(dev), (char*)param->u.wpa_ie.data, (u16)param->u.wpa_ie.len);
		break;

	case IEEE_CMD_SET_ENCRYPTION:
		ret = wpa_set_encryption(dev, param, p->length);
		break;

	case IEEE_CMD_MLME:
		ret = wpa_mlme(dev, param->u.mlme.command, param->u.mlme.reason_code);
		break;

	default:
		DBG_88E("Unknown WPA supplicant request: %d\n", param->cmd);
		ret = -EOPNOTSUPP;
		break;

	}

	if (ret == 0 && copy_to_user(p->pointer, param, p->length))
		ret = -EFAULT;

	rtw_mfree((u8 *)param, p->length);

out:

	/* up(&ieee->wx_sem); */

	return ret;

}

#ifdef CONFIG_AP_MODE
static u8 set_pairwise_key(struct adapter *padapter, struct sta_info *psta)
{
	struct cmd_obj*			ph2c;
	struct set_stakey_parm	*psetstakey_para;
	struct cmd_priv				*pcmdpriv=&padapter->cmdpriv;
	u8	res=_SUCCESS;

	ph2c = (struct cmd_obj*)rtw_zmalloc(sizeof(struct cmd_obj));
	if ( ph2c == NULL) {
		res= _FAIL;
		goto exit;
	}

	psetstakey_para = (struct set_stakey_parm*)rtw_zmalloc(sizeof(struct set_stakey_parm));
	if (psetstakey_para== NULL) {
		rtw_mfree((u8 *) ph2c, sizeof(struct cmd_obj));
		res=_FAIL;
		goto exit;
	}

	init_h2fwcmd_w_parm_no_rsp(ph2c, psetstakey_para, _SetStaKey_CMD_);


	psetstakey_para->algorithm = (u8)psta->dot118021XPrivacy;

	memcpy(psetstakey_para->addr, psta->hwaddr, ETH_ALEN);

	memcpy(psetstakey_para->key, &psta->dot118021x_UncstKey, 16);


	res = rtw_enqueue_cmd(pcmdpriv, ph2c);

exit:

	return res;

}

static int set_group_key(struct adapter *padapter, u8 *key, u8 alg, int keyid)
{
	u8 keylen;
	struct cmd_obj* pcmd;
	struct setkey_parm *psetkeyparm;
	struct cmd_priv	*pcmdpriv=&(padapter->cmdpriv);
	int res=_SUCCESS;

	DBG_88E("%s\n", __FUNCTION__);

	pcmd = (struct cmd_obj*)rtw_zmalloc(sizeof(struct	cmd_obj));
	if (pcmd== NULL) {
		res= _FAIL;
		goto exit;
	}
	psetkeyparm=(struct setkey_parm*)rtw_zmalloc(sizeof(struct setkey_parm));
	if (psetkeyparm== NULL) {
		rtw_mfree((unsigned char *)pcmd, sizeof(struct cmd_obj));
		res= _FAIL;
		goto exit;
	}

	memset(psetkeyparm, 0, sizeof(struct setkey_parm));

	psetkeyparm->keyid=(u8)keyid;
	if (is_wep_enc(alg))
		padapter->securitypriv.key_mask |= BIT(psetkeyparm->keyid);
	psetkeyparm->algorithm = alg;

	psetkeyparm->set_tx = 1;

	switch (alg)
	{
		case _WEP40_:
			keylen = 5;
			break;
		case _WEP104_:
			keylen = 13;
			break;
		case _TKIP_:
		case _TKIP_WTMIC_:
		case _AES_:
		default:
			keylen = 16;
	}

	memcpy(&(psetkeyparm->key[0]), key, keylen);

	pcmd->cmdcode = _SetKey_CMD_;
	pcmd->parmbuf = (u8 *)psetkeyparm;
	pcmd->cmdsz =  (sizeof(struct setkey_parm));
	pcmd->rsp = NULL;
	pcmd->rspsz = 0;


	_rtw_init_listhead(&pcmd->list);

	res = rtw_enqueue_cmd(pcmdpriv, pcmd);

exit:

	return res;


}

static int set_wep_key(struct adapter *padapter, u8 *key, u8 keylen, int keyid)
{
	u8 alg;

	switch (keylen)
	{
		case 5:
			alg =_WEP40_;
			break;
		case 13:
			alg =_WEP104_;
			break;
		default:
			alg =_NO_PRIVACY_;
	}

	return set_group_key(padapter, key, alg, keyid);

}


static int rtw_set_encryption(struct net_device *dev, struct ieee_param *param, u32 param_len)
{
	int ret = 0;
	u32 wep_key_idx, wep_key_len,wep_total_len;
	struct ndis_802_11_wep	 *pwep = NULL;
	struct sta_info *psta = NULL, *pbcmc_sta = NULL;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct security_priv* psecuritypriv=&(padapter->securitypriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_88E("%s\n", __FUNCTION__);

	param->u.crypt.err = 0;
	param->u.crypt.alg[IEEE_CRYPT_ALG_NAME_LEN - 1] = '\0';

	/* sizeof(struct ieee_param) = 64 bytes; */
	/* if (param_len !=  (u32) ((u8 *) param->u.crypt.key - (u8 *) param) + param->u.crypt.key_len) */
	if (param_len !=  sizeof(struct ieee_param) + param->u.crypt.key_len)
	{
		ret =  -EINVAL;
		goto exit;
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff)
	{
		if (param->u.crypt.idx >= WEP_KEYS)
		{
			ret = -EINVAL;
			goto exit;
		}
	}
	else
	{
		psta = rtw_get_stainfo(pstapriv, param->sta_addr);
		if (!psta)
		{
			/* ret = -EINVAL; */
			DBG_88E("rtw_set_encryption(), sta has already been removed or never been added\n");
			goto exit;
		}
	}

	if (strcmp(param->u.crypt.alg, "none") == 0 && (psta== NULL))
	{
		/* todo:clear default encryption keys */

		DBG_88E("clear default encryption keys, keyid=%d\n", param->u.crypt.idx);

		goto exit;
	}


	if (strcmp(param->u.crypt.alg, "WEP") == 0 && (psta== NULL))
	{
		DBG_88E("r871x_set_encryption, crypt.alg = WEP\n");

		wep_key_idx = param->u.crypt.idx;
		wep_key_len = param->u.crypt.key_len;

		DBG_88E("r871x_set_encryption, wep_key_idx=%d, len=%d\n", wep_key_idx, wep_key_len);

		if ((wep_key_idx >= WEP_KEYS) || (wep_key_len<=0))
		{
			ret = -EINVAL;
			goto exit;
		}


		if (wep_key_len > 0)
		{
			wep_key_len = wep_key_len <= 5 ? 5 : 13;
			wep_total_len = wep_key_len + FIELD_OFFSET(struct ndis_802_11_wep, KeyMaterial);
			pwep =(struct ndis_802_11_wep *)rtw_malloc(wep_total_len);
			if (pwep == NULL) {
				DBG_88E(" r871x_set_encryption: pwep allocate fail !!!\n");
				goto exit;
			}

			memset(pwep, 0, wep_total_len);

			pwep->KeyLength = wep_key_len;
			pwep->Length = wep_total_len;

		}

		pwep->KeyIndex = wep_key_idx;

		memcpy(pwep->KeyMaterial,  param->u.crypt.key, pwep->KeyLength);

		if (param->u.crypt.set_tx)
		{
			DBG_88E("wep, set_tx=1\n");

			psecuritypriv->ndisencryptstatus = Ndis802_11Encryption1Enabled;
			psecuritypriv->dot11PrivacyAlgrthm=_WEP40_;
			psecuritypriv->dot118021XGrpPrivacy=_WEP40_;

			if (pwep->KeyLength==13) {
				psecuritypriv->dot11PrivacyAlgrthm=_WEP104_;
				psecuritypriv->dot118021XGrpPrivacy=_WEP104_;
			}
			psecuritypriv->dot11PrivacyKeyIndex = wep_key_idx;

			memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), pwep->KeyMaterial, pwep->KeyLength);

			psecuritypriv->dot11DefKeylen[wep_key_idx]=pwep->KeyLength;

			set_wep_key(padapter, pwep->KeyMaterial, pwep->KeyLength, wep_key_idx);
		} else {
			DBG_88E("wep, set_tx=0\n");

			/* don't update "psecuritypriv->dot11PrivacyAlgrthm" and */
			/* psecuritypriv->dot11PrivacyKeyIndex=keyid", but can rtw_set_key to cam */

			memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), pwep->KeyMaterial, pwep->KeyLength);

			psecuritypriv->dot11DefKeylen[wep_key_idx] = pwep->KeyLength;

			set_wep_key(padapter, pwep->KeyMaterial, pwep->KeyLength, wep_key_idx);

		}

		goto exit;

	}


	if (!psta && check_fwstate(pmlmepriv, WIFI_AP_STATE)) /*  group key */
	{
		if (param->u.crypt.set_tx ==1)
		{
			if (strcmp(param->u.crypt.alg, "WEP") == 0)
			{
				DBG_88E("%s, set group_key, WEP\n", __FUNCTION__);

				memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

				psecuritypriv->dot118021XGrpPrivacy = _WEP40_;
				if (param->u.crypt.key_len==13)
				{
						psecuritypriv->dot118021XGrpPrivacy = _WEP104_;
				}

			}
			else if (strcmp(param->u.crypt.alg, "TKIP") == 0)
			{
				DBG_88E("%s, set group_key, TKIP\n", __FUNCTION__);

				psecuritypriv->dot118021XGrpPrivacy = _TKIP_;

				memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

				/* DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len); */
				/* set mic key */
				memcpy(psecuritypriv->dot118021XGrptxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[16]), 8);
				memcpy(psecuritypriv->dot118021XGrprxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[24]), 8);

				psecuritypriv->busetkipkey = true;

			}
			else if (strcmp(param->u.crypt.alg, "CCMP") == 0)
			{
				DBG_88E("%s, set group_key, CCMP\n", __FUNCTION__);

				psecuritypriv->dot118021XGrpPrivacy = _AES_;

				memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
			}
			else
			{
				DBG_88E("%s, set group_key, none\n", __FUNCTION__);

				psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
			}

			psecuritypriv->dot118021XGrpKeyid = param->u.crypt.idx;

			psecuritypriv->binstallGrpkey = true;

			psecuritypriv->dot11PrivacyAlgrthm = psecuritypriv->dot118021XGrpPrivacy;/*  */

			set_group_key(padapter, param->u.crypt.key, psecuritypriv->dot118021XGrpPrivacy, param->u.crypt.idx);

			pbcmc_sta=rtw_get_bcmc_stainfo(padapter);
			if (pbcmc_sta)
			{
				pbcmc_sta->ieee8021x_blocked = false;
				pbcmc_sta->dot118021XPrivacy= psecuritypriv->dot118021XGrpPrivacy;/* rx will use bmc_sta's dot118021XPrivacy */
			}

		}

		goto exit;

	}

	if (psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_8021X && psta) /*  psk/802_1x */
	{
		if (check_fwstate(pmlmepriv, WIFI_AP_STATE))
		{
			if (param->u.crypt.set_tx ==1)
			{
				memcpy(psta->dot118021x_UncstKey.skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

				if (strcmp(param->u.crypt.alg, "WEP") == 0)
				{
					DBG_88E("%s, set pairwise key, WEP\n", __FUNCTION__);

					psta->dot118021XPrivacy = _WEP40_;
					if (param->u.crypt.key_len==13)
					{
						psta->dot118021XPrivacy = _WEP104_;
					}
				}
				else if (strcmp(param->u.crypt.alg, "TKIP") == 0)
				{
					DBG_88E("%s, set pairwise key, TKIP\n", __FUNCTION__);

					psta->dot118021XPrivacy = _TKIP_;

					/* DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len); */
					/* set mic key */
					memcpy(psta->dot11tkiptxmickey.skey, &(param->u.crypt.key[16]), 8);
					memcpy(psta->dot11tkiprxmickey.skey, &(param->u.crypt.key[24]), 8);

					psecuritypriv->busetkipkey = true;

				}
				else if (strcmp(param->u.crypt.alg, "CCMP") == 0)
				{

					DBG_88E("%s, set pairwise key, CCMP\n", __FUNCTION__);

					psta->dot118021XPrivacy = _AES_;
				}
				else
				{
					DBG_88E("%s, set pairwise key, none\n", __FUNCTION__);

					psta->dot118021XPrivacy = _NO_PRIVACY_;
				}

				set_pairwise_key(padapter, psta);

				psta->ieee8021x_blocked = false;

			}
			else/* group key??? */
			{
				if (strcmp(param->u.crypt.alg, "WEP") == 0)
				{
					memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

					psecuritypriv->dot118021XGrpPrivacy = _WEP40_;
					if (param->u.crypt.key_len==13)
					{
						psecuritypriv->dot118021XGrpPrivacy = _WEP104_;
					}
				}
				else if (strcmp(param->u.crypt.alg, "TKIP") == 0)
				{
					psecuritypriv->dot118021XGrpPrivacy = _TKIP_;

					memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

					/* DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len); */
					/* set mic key */
					memcpy(psecuritypriv->dot118021XGrptxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[16]), 8);
					memcpy(psecuritypriv->dot118021XGrprxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[24]), 8);

					psecuritypriv->busetkipkey = true;

				}
				else if (strcmp(param->u.crypt.alg, "CCMP") == 0)
				{
					psecuritypriv->dot118021XGrpPrivacy = _AES_;

					memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
				}
				else
				{
					psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
				}

				psecuritypriv->dot118021XGrpKeyid = param->u.crypt.idx;

				psecuritypriv->binstallGrpkey = true;

				psecuritypriv->dot11PrivacyAlgrthm = psecuritypriv->dot118021XGrpPrivacy;/*  */

				set_group_key(padapter, param->u.crypt.key, psecuritypriv->dot118021XGrpPrivacy, param->u.crypt.idx);

				pbcmc_sta=rtw_get_bcmc_stainfo(padapter);
				if (pbcmc_sta)
				{
					pbcmc_sta->ieee8021x_blocked = false;
					pbcmc_sta->dot118021XPrivacy= psecuritypriv->dot118021XGrpPrivacy;/* rx will use bmc_sta's dot118021XPrivacy */
				}

			}

		}

	}

exit:

	if (pwep)
	{
		rtw_mfree((u8 *)pwep, wep_total_len);
	}

	return ret;

}

static int rtw_set_beacon(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;
	unsigned char *pbuf = param->u.bcn_ie.buf;


	DBG_88E("%s, len=%d\n", __FUNCTION__, len);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) != true)
		return -EINVAL;

	memcpy(&pstapriv->max_num_sta, param->u.bcn_ie.reserved, 2);

	if ((pstapriv->max_num_sta>NUM_STA) || (pstapriv->max_num_sta<=0))
		pstapriv->max_num_sta = NUM_STA;


	if (rtw_check_beacon_data(padapter, pbuf,  (len-12-2)) == _SUCCESS)/*  12 = param header, 2:no packed */
		ret = 0;
	else
		ret = -EINVAL;


	return ret;

}

static int rtw_hostapd_sta_flush(struct net_device *dev)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	int ret=0;

	DBG_88E("%s\n", __FUNCTION__);

	flush_all_cam_entry(padapter);	/* clear CAM */

	ret = rtw_sta_flush(padapter);

	return ret;

}

static int rtw_add_sta(struct net_device *dev, struct ieee_param *param)
{
	unsigned long irqL;
	int ret=0;
	struct sta_info *psta = NULL;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_88E("rtw_add_sta(aid=%d)=" MAC_FMT "\n", param->u.add_sta.aid, MAC_ARG(param->sta_addr));

	if (check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) != true)
	{
		return -EINVAL;
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff)
	{
		return -EINVAL;
	}

	psta = rtw_get_stainfo(pstapriv, param->sta_addr);
	if (psta)
	{
		int flags = param->u.add_sta.flags;

		psta->aid = param->u.add_sta.aid;/* aid=1~2007 */
		memcpy(psta->bssrateset, param->u.add_sta.tx_supp_rates, 16);

		/* check wmm cap. */
		if (WLAN_STA_WME&flags)
			psta->qos_option = 1;
		else
			psta->qos_option = 0;

		if (pmlmepriv->qospriv.qos_option == 0)
			psta->qos_option = 0;


		/* chec 802.11n ht cap. */
		if (WLAN_STA_HT&flags)
		{
			psta->htpriv.ht_option = true;
			psta->qos_option = 1;
			memcpy((void*)&psta->htpriv.ht_cap, (void*)&param->u.add_sta.ht_cap, sizeof(struct ieee80211_ht_cap));
		}
		else
		{
			psta->htpriv.ht_option = false;
		}

		if (pmlmepriv->htpriv.ht_option == false)
			psta->htpriv.ht_option = false;

		update_sta_info_apmode(padapter, psta);
	}
	else
	{
		ret = -ENOMEM;
	}

	return ret;

}

static int rtw_del_sta(struct net_device *dev, struct ieee_param *param)
{
	unsigned long irqL;
	int ret=0;
	struct sta_info *psta = NULL;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_88E("rtw_del_sta=" MAC_FMT "\n", MAC_ARG(param->sta_addr));

	if (check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) != true)
	{
		return -EINVAL;
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff)
	{
		return -EINVAL;
	}

	psta = rtw_get_stainfo(pstapriv, param->sta_addr);
	if (psta)
	{
		u8 updated;

		spin_lock_bh(&pstapriv->asoc_list_lock);
		if (rtw_is_list_empty(&psta->asoc_list)==false)
		{
			rtw_list_delete(&psta->asoc_list);
			pstapriv->asoc_list_cnt--;
			updated = ap_free_sta(padapter, psta, true, WLAN_REASON_DEAUTH_LEAVING);
		}
		spin_unlock_bh(&pstapriv->asoc_list_lock);

		associated_clients_update(padapter, updated);

		psta = NULL;

	}
	else
	{
		DBG_88E("rtw_del_sta(), sta has already been removed or never been added\n");

		/* ret = -1; */
	}


	return ret;

}

static int rtw_ioctl_get_sta_data(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	struct sta_info *psta = NULL;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct ieee_param_ex *param_ex = (struct ieee_param_ex *)param;
	struct sta_data *psta_data = (struct sta_data *)param_ex->data;

	DBG_88E("rtw_ioctl_get_sta_info, sta_addr: " MAC_FMT "\n", MAC_ARG(param_ex->sta_addr));

	if (check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) != true)
	{
		return -EINVAL;
	}

	if (param_ex->sta_addr[0] == 0xff && param_ex->sta_addr[1] == 0xff &&
	    param_ex->sta_addr[2] == 0xff && param_ex->sta_addr[3] == 0xff &&
	    param_ex->sta_addr[4] == 0xff && param_ex->sta_addr[5] == 0xff)
	{
		return -EINVAL;
	}

	psta = rtw_get_stainfo(pstapriv, param_ex->sta_addr);
	if (psta)
	{
		psta_data->aid = (u16)psta->aid;
		psta_data->capability = psta->capability;
		psta_data->flags = psta->flags;
		psta_data->sta_set =((psta->nonerp_set) |
				(psta->no_short_slot_time_set <<1) |
				(psta->no_short_preamble_set <<2) |
				(psta->no_ht_gf_set <<3) |
				(psta->no_ht_set <<4) |
				(psta->ht_20mhz_set <<5));

		psta_data->tx_supp_rates_len =  psta->bssratelen;
		memcpy(psta_data->tx_supp_rates, psta->bssrateset, psta->bssratelen);
		memcpy(&psta_data->ht_cap, &psta->htpriv.ht_cap, sizeof(struct ieee80211_ht_cap));
		psta_data->rx_pkts = psta->sta_stats.rx_data_pkts;
		psta_data->rx_bytes = psta->sta_stats.rx_bytes;
		psta_data->rx_drops = psta->sta_stats.rx_drops;

		psta_data->tx_pkts = psta->sta_stats.tx_pkts;
		psta_data->tx_bytes = psta->sta_stats.tx_bytes;
		psta_data->tx_drops = psta->sta_stats.tx_drops;


	}
	else
	{
		ret = -1;
	}

	return ret;

}

static int rtw_get_sta_wpaie(struct net_device *dev, struct ieee_param *param)
{
	int ret=0;
	struct sta_info *psta = NULL;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_88E("rtw_get_sta_wpaie, sta_addr: " MAC_FMT "\n", MAC_ARG(param->sta_addr));

	if (check_fwstate(pmlmepriv, (_FW_LINKED|WIFI_AP_STATE)) != true)
	{
		return -EINVAL;
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff)
	{
		return -EINVAL;
	}

	psta = rtw_get_stainfo(pstapriv, param->sta_addr);
	if (psta)
	{
		if ((psta->wpa_ie[0] == WLAN_EID_RSN) || (psta->wpa_ie[0] == WLAN_EID_GENERIC))
		{
			int wpa_ie_len;
			int copy_len;

			wpa_ie_len = psta->wpa_ie[1];

			copy_len = ((wpa_ie_len+2) > sizeof(psta->wpa_ie)) ? (sizeof(psta->wpa_ie)):(wpa_ie_len+2);

			param->u.wpa_ie.len = copy_len;

			memcpy(param->u.wpa_ie.reserved, psta->wpa_ie, copy_len);
		}
		else
		{
			/* ret = -1; */
			DBG_88E("sta's wpa_ie is NONE\n");
		}
	}
	else
	{
		ret = -1;
	}

	return ret;

}

static int rtw_set_wps_beacon(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	unsigned char wps_oui[4]={0x0,0x50,0xf2,0x04};
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	int ie_len;

	DBG_88E("%s, len=%d\n", __FUNCTION__, len);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) != true)
		return -EINVAL;

	ie_len = len-12-2;/*  12 = param header, 2:no packed */


	if (pmlmepriv->wps_beacon_ie)
	{
		rtw_mfree(pmlmepriv->wps_beacon_ie, pmlmepriv->wps_beacon_ie_len);
		pmlmepriv->wps_beacon_ie = NULL;
	}

	if (ie_len>0)
	{
		pmlmepriv->wps_beacon_ie = rtw_malloc(ie_len);
		pmlmepriv->wps_beacon_ie_len = ie_len;
		if ( pmlmepriv->wps_beacon_ie == NULL) {
			DBG_88E("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
			return -EINVAL;
		}

		memcpy(pmlmepriv->wps_beacon_ie, param->u.bcn_ie.buf, ie_len);

		update_beacon(padapter, _VENDOR_SPECIFIC_IE_, wps_oui, true);

		pmlmeext->bstart_bss = true;

	}


	return ret;

}

static int rtw_set_wps_probe_resp(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	int ie_len;

	DBG_88E("%s, len=%d\n", __FUNCTION__, len);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) != true)
		return -EINVAL;

	ie_len = len-12-2;/*  12 = param header, 2:no packed */


	if (pmlmepriv->wps_probe_resp_ie)
	{
		rtw_mfree(pmlmepriv->wps_probe_resp_ie, pmlmepriv->wps_probe_resp_ie_len);
		pmlmepriv->wps_probe_resp_ie = NULL;
	}

	if (ie_len>0)
	{
		pmlmepriv->wps_probe_resp_ie = rtw_malloc(ie_len);
		pmlmepriv->wps_probe_resp_ie_len = ie_len;
		if ( pmlmepriv->wps_probe_resp_ie == NULL) {
			DBG_88E("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
			return -EINVAL;
		}
		memcpy(pmlmepriv->wps_probe_resp_ie, param->u.bcn_ie.buf, ie_len);
	}


	return ret;

}

static int rtw_set_wps_assoc_resp(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	int ie_len;

	DBG_88E("%s, len=%d\n", __FUNCTION__, len);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) != true)
		return -EINVAL;

	ie_len = len-12-2;/*  12 = param header, 2:no packed */


	if (pmlmepriv->wps_assoc_resp_ie)
	{
		rtw_mfree(pmlmepriv->wps_assoc_resp_ie, pmlmepriv->wps_assoc_resp_ie_len);
		pmlmepriv->wps_assoc_resp_ie = NULL;
	}

	if (ie_len>0)
	{
		pmlmepriv->wps_assoc_resp_ie = rtw_malloc(ie_len);
		pmlmepriv->wps_assoc_resp_ie_len = ie_len;
		if ( pmlmepriv->wps_assoc_resp_ie == NULL) {
			DBG_88E("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
			return -EINVAL;
		}

		memcpy(pmlmepriv->wps_assoc_resp_ie, param->u.bcn_ie.buf, ie_len);
	}


	return ret;

}

static int rtw_set_hidden_ssid(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	struct adapter *adapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *mlmepriv = &(adapter->mlmepriv);
	struct mlme_ext_priv	*mlmeext = &(adapter->mlmeextpriv);
	struct mlme_ext_info	*mlmeinfo = &(mlmeext->mlmext_info);
	int ie_len;
	u8 *ssid_ie;
	char ssid[NDIS_802_11_LENGTH_SSID + 1];
	sint ssid_len;
	u8 ignore_broadcast_ssid;

	if (check_fwstate(mlmepriv, WIFI_AP_STATE) != true)
		return -EPERM;

	if (param->u.bcn_ie.reserved[0] != 0xea)
		return -EINVAL;

	mlmeinfo->hidden_ssid_mode = ignore_broadcast_ssid = param->u.bcn_ie.reserved[1];

	ie_len = len-12-2;/*  12 = param header, 2:no packed */
	ssid_ie = rtw_get_ie(param->u.bcn_ie.buf,  WLAN_EID_SSID, &ssid_len, ie_len);

	if (ssid_ie && ssid_len) {
		struct wlan_bssid_ex *pbss_network = &mlmepriv->cur_network.network;
		struct wlan_bssid_ex *pbss_network_ext = &mlmeinfo->network;

		memcpy(ssid, ssid_ie+2, ssid_len);
		ssid[ssid_len>NDIS_802_11_LENGTH_SSID?NDIS_802_11_LENGTH_SSID:ssid_len] = 0x0;

		memcpy(pbss_network->Ssid.Ssid, (void *)ssid, ssid_len);
		pbss_network->Ssid.SsidLength = ssid_len;
		memcpy(pbss_network_ext->Ssid.Ssid, (void *)ssid, ssid_len);
		pbss_network_ext->Ssid.SsidLength = ssid_len;
	}

	DBG_88E(FUNC_ADPT_FMT" ignore_broadcast_ssid:%d, %s,%d\n", FUNC_ADPT_ARG(adapter),
		ignore_broadcast_ssid, ssid, ssid_len);

	return ret;
}

static int rtw_ioctl_acl_remove_sta(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) != true)
		return -EINVAL;

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff)
	{
		return -EINVAL;
	}

	ret = rtw_acl_remove_sta(padapter, param->sta_addr);

	return ret;

}

static int rtw_ioctl_acl_add_sta(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) != true)
		return -EINVAL;

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff)
	{
		return -EINVAL;
	}

	ret = rtw_acl_add_sta(padapter, param->sta_addr);

	return ret;

}

static int rtw_ioctl_set_macaddr_acl(struct net_device *dev, struct ieee_param *param, int len)
{
	int ret=0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) != true)
		return -EINVAL;

	rtw_set_macaddr_acl(padapter, param->u.mlme.command);

	return ret;
}

static int rtw_hostapd_ioctl(struct net_device *dev, struct iw_point *p)
{
	struct ieee_param *param;
	int ret=0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);

	/* DBG_88E("%s\n", __FUNCTION__); */

	/*
	* this function is expect to call in master mode, which allows no power saving
	* so, we just check hw_init_completed
	*/

	if (padapter->hw_init_completed==false) {
		ret = -EPERM;
		goto out;
	}


	/* if (p->length < sizeof(struct ieee_param) || !p->pointer) { */
	if (!p->pointer) {
		ret = -EINVAL;
		goto out;
	}

	param = (struct ieee_param *)rtw_malloc(p->length);
	if (param == NULL)
	{
		ret = -ENOMEM;
		goto out;
	}

	if (copy_from_user(param, p->pointer, p->length))
	{
		rtw_mfree((u8*)param, p->length);
		ret = -EFAULT;
		goto out;
	}

	/* DBG_88E("%s, cmd=%d\n", __FUNCTION__, param->cmd); */

	switch (param->cmd)
	{
		case RTL871X_HOSTAPD_FLUSH:

			ret = rtw_hostapd_sta_flush(dev);

			break;

		case RTL871X_HOSTAPD_ADD_STA:

			ret = rtw_add_sta(dev, param);

			break;

		case RTL871X_HOSTAPD_REMOVE_STA:

			ret = rtw_del_sta(dev, param);

			break;

		case RTL871X_HOSTAPD_SET_BEACON:

			ret = rtw_set_beacon(dev, param, p->length);

			break;

		case RTL871X_SET_ENCRYPTION:

			ret = rtw_set_encryption(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_GET_WPAIE_STA:

			ret = rtw_get_sta_wpaie(dev, param);

			break;

		case RTL871X_HOSTAPD_SET_WPS_BEACON:

			ret = rtw_set_wps_beacon(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_SET_WPS_PROBE_RESP:

			ret = rtw_set_wps_probe_resp(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_SET_WPS_ASSOC_RESP:

			ret = rtw_set_wps_assoc_resp(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_SET_HIDDEN_SSID:

			ret = rtw_set_hidden_ssid(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_GET_INFO_STA:

			ret = rtw_ioctl_get_sta_data(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_SET_MACADDR_ACL:

			ret = rtw_ioctl_set_macaddr_acl(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_ACL_ADD_STA:

			ret = rtw_ioctl_acl_add_sta(dev, param, p->length);

			break;

		case RTL871X_HOSTAPD_ACL_REMOVE_STA:

			ret = rtw_ioctl_acl_remove_sta(dev, param, p->length);

			break;

		default:
			DBG_88E("Unknown hostapd request: %d\n", param->cmd);
			ret = -EOPNOTSUPP;
			break;

	}

	if (ret == 0 && copy_to_user(p->pointer, param, p->length))
		ret = -EFAULT;


	rtw_mfree((u8 *)param, p->length);

out:

	return ret;

}
#endif

#include <rtw_android.h>
static int rtw_wx_set_priv(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *awrq,
				char *extra)
{

#ifdef CONFIG_DEBUG_RTW_WX_SET_PRIV
	char *ext_dbg;
#endif

	int ret = 0;
	int len = 0;
	char *ext;
	int i;

	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_point *dwrq = (struct iw_point*)awrq;

	/* RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_notice_, ("+rtw_wx_set_priv\n")); */
	if (dwrq->length == 0)
		return -EFAULT;

	len = dwrq->length;
	if (!(ext = rtw_vmalloc(len)))
		return -ENOMEM;

	if (copy_from_user(ext, dwrq->pointer, len)) {
		rtw_vmfree(ext, len);
		return -EFAULT;
	}


	/* RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_notice_, */
	/* 	 ("rtw_wx_set_priv: %s req=%s\n", */
	/* 	  dev->name, ext)); */

	#ifdef CONFIG_DEBUG_RTW_WX_SET_PRIV
	if (!(ext_dbg = rtw_vmalloc(len)))
	{
		rtw_vmfree(ext, len);
		return -ENOMEM;
	}

	memcpy(ext_dbg, ext, len);
	#endif

	/* added for wps2.0 @20110524 */
	if (dwrq->flags == 0x8766 && len > 8)
	{
		u32 cp_sz;
		struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
		u8 *probereq_wpsie = ext;
		int probereq_wpsie_len = len;
		u8 wps_oui[4]={0x0,0x50,0xf2,0x04};

		if ((_VENDOR_SPECIFIC_IE_ == probereq_wpsie[0]) &&
			(_rtw_memcmp(&probereq_wpsie[2], wps_oui, 4) ==true))
		{
			cp_sz = probereq_wpsie_len>MAX_WPS_IE_LEN ? MAX_WPS_IE_LEN:probereq_wpsie_len;

			/* memcpy(pmlmepriv->probereq_wpsie, probereq_wpsie, cp_sz); */
			/* pmlmepriv->probereq_wpsie_len = cp_sz; */
			if (pmlmepriv->wps_probe_req_ie)
			{
				u32 free_len = pmlmepriv->wps_probe_req_ie_len;
				pmlmepriv->wps_probe_req_ie_len = 0;
				rtw_mfree(pmlmepriv->wps_probe_req_ie, free_len);
				pmlmepriv->wps_probe_req_ie = NULL;
			}

			pmlmepriv->wps_probe_req_ie = rtw_malloc(cp_sz);
			if ( pmlmepriv->wps_probe_req_ie == NULL) {
				printk("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
				ret =  -EINVAL;
				goto FREE_EXT;

			}

			memcpy(pmlmepriv->wps_probe_req_ie, probereq_wpsie, cp_sz);
			pmlmepriv->wps_probe_req_ie_len = cp_sz;

		}

		goto FREE_EXT;

	}

	if (	len >= WEXT_CSCAN_HEADER_SIZE
		&& _rtw_memcmp(ext, WEXT_CSCAN_HEADER, WEXT_CSCAN_HEADER_SIZE) == true
	) {
		ret = rtw_wx_set_scan(dev, info, awrq, ext);
		goto FREE_EXT;
	}

#ifdef CONFIG_ANDROID
	/* DBG_88E("rtw_wx_set_priv: %s req=%s\n", dev->name, ext); */

	i = rtw_android_cmdstr_to_num(ext);

	switch (i) {
		case ANDROID_WIFI_CMD_START :
			indicate_wx_custom_event(padapter, "START");
			break;
		case ANDROID_WIFI_CMD_STOP :
			indicate_wx_custom_event(padapter, "STOP");
			break;
		case ANDROID_WIFI_CMD_RSSI :
			{
				struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
				struct	wlan_network	*pcur_network = &pmlmepriv->cur_network;

				if (check_fwstate(pmlmepriv, _FW_LINKED) == true) {
					sprintf(ext, "%s rssi %d", pcur_network->network.Ssid.Ssid, padapter->recvpriv.rssi);
				} else {
					sprintf(ext, "OK");
				}
			}
			break;
		case ANDROID_WIFI_CMD_LINKSPEED :
			{
				u16 mbps = rtw_get_cur_max_rate(padapter)/10;
				sprintf(ext, "LINKSPEED %d", mbps);
			}
			break;
		case ANDROID_WIFI_CMD_MACADDR :
			sprintf(ext, "MACADDR = " MAC_FMT, MAC_ARG(dev->dev_addr));
			break;
		case ANDROID_WIFI_CMD_SCAN_ACTIVE :
			{
				/* rtw_set_scan_mode(padapter, SCAN_ACTIVE); */
				sprintf(ext, "OK");
			}
			break;
		case ANDROID_WIFI_CMD_SCAN_PASSIVE :
			{
				/* rtw_set_scan_mode(padapter, SCAN_PASSIVE); */
				sprintf(ext, "OK");
			}
			break;

		case ANDROID_WIFI_CMD_COUNTRY :
			{
				char country_code[10];
				sscanf(ext, "%*s %s", country_code);
				rtw_set_country(padapter, country_code);
				sprintf(ext, "OK");
			}
			break;
		default :
			#ifdef  CONFIG_DEBUG_RTW_WX_SET_PRIV
			DBG_88E("%s: %s unknowned req=%s\n", __FUNCTION__,
				dev->name, ext_dbg);
			#endif

			sprintf(ext, "OK");

	}

	if (copy_to_user(dwrq->pointer, ext, min(dwrq->length, (u16)(strlen(ext)+1)) ) )
		ret = -EFAULT;

	#ifdef CONFIG_DEBUG_RTW_WX_SET_PRIV
	DBG_88E("%s: %s req=%s rep=%s dwrq->length=%d, strlen(ext)+1=%d\n", __FUNCTION__,
		dev->name, ext_dbg ,ext, dwrq->length, (u16)(strlen(ext)+1));
	#endif
#endif /* end of CONFIG_ANDROID */


FREE_EXT:

	rtw_vmfree(ext, len);
	#ifdef CONFIG_DEBUG_RTW_WX_SET_PRIV
	rtw_vmfree(ext_dbg, len);
	#endif

	/* DBG_88E("rtw_wx_set_priv: (SIOCSIWPRIV) %s ret=%d\n", */
	/* 		dev->name, ret); */

	return ret;

}

static int rtw_pm_set(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	int ret = 0;
	unsigned	mode = 0;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);

	DBG_88E( "[%s] extra = %s\n", __FUNCTION__, extra );

	if ( _rtw_memcmp( extra, "lps=", 4 ) )
	{
		sscanf(extra+4, "%u", &mode);
		ret = rtw_pm_set_lps(padapter,mode);
	}
	else if ( _rtw_memcmp( extra, "ips=", 4 ) )
	{
		sscanf(extra+4, "%u", &mode);
		ret = rtw_pm_set_ips(padapter,mode);
	}
	else {
		ret = -EINVAL;
	}

	return ret;
}

static int rtw_mp_efuse_get(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wdata, char *extra)
{

	struct adapter *padapter = rtw_netdev_priv(dev);
	struct eeprom_priv *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);
	PEFUSE_HAL pEfuseHal;
	struct iw_point *wrqu;

	u8	*PROMContent = pEEPROM->efuse_eeprom_data;
	struct pwrctrl_priv *pwrctrlpriv ;

	u8 *data = NULL;
	u8 *rawdata = NULL;
	char *pch, *ptmp, *token, *tmp[3]={NULL, NULL, NULL};
	u8 ips_mode,lps_mode;
	u16 i=0, j=0, mapLen=0, addr=0, cnts=0;
	u16 max_available_size=0, raw_cursize=0, raw_maxsize=0;
	int err;
	u8 org_fw_iol = padapter->registrypriv.fw_iol;/*  0:Disable, 1:enable, 2:by usb speed */

	wrqu = (struct iw_point*)wdata;
	pwrctrlpriv = adapter_to_pwrctl(padapter);
	pEfuseHal = &pHalData->EfuseHal;

	err = 0;
	data = rtw_zmalloc(EFUSE_BT_MAX_MAP_LEN);
	if (data == NULL)
	{
		err = -ENOMEM;
		goto exit;
	}
	rawdata = rtw_zmalloc(EFUSE_BT_MAX_MAP_LEN);
	if (rawdata == NULL)
	{
		err = -ENOMEM;
		goto exit;
	}

	if (copy_from_user(extra, wrqu->pointer, wrqu->length))
	{
		err = -EFAULT;
		goto exit;
	}
	lps_mode = pwrctrlpriv->power_mgnt;/* keep org value */
	rtw_pm_set_lps(padapter,PS_MODE_ACTIVE);

	ips_mode = pwrctrlpriv->ips_mode;/* keep org value */
	rtw_pm_set_ips(padapter,IPS_NONE);

	pch = extra;
	DBG_88E("%s: in=%s\n", __FUNCTION__, extra);

	i = 0;
	/* mac 16 "00e04c871200" rmap,00,2 */
	while ((token = strsep(&pch, ",")) != NULL)
	{
		if (i > 2) break;
		tmp[i] = token;
		i++;
	}
	padapter->registrypriv.fw_iol = 0;/*  0:Disable, 1:enable, 2:by usb speed */

	if (strcmp(tmp[0], "status") == 0) {
		sprintf(extra, "Load File efuse=%s,Load File MAC=%s",(pEEPROM->bloadfile_fail_flag? "FAIL" : "OK"),(pEEPROM->bloadmac_fail_flag? "FAIL" : "OK"));
		goto exit;
	}
	else if (strcmp(tmp[0], "drvmap") == 0)
	{
		mapLen = EFUSE_MAP_SIZE;

		sprintf(extra, "\n");
		for (i = 0; i < EFUSE_MAP_SIZE; i += 16)
		{
			sprintf(extra, "%s0x%02x\t", extra, i);
			for (j=0; j<8; j++) {
				sprintf(extra, "%s%02X ", extra, PROMContent[i+j]);
			}
			sprintf(extra, "%s\t", extra);
			for (; j<16; j++) {
				sprintf(extra, "%s%02X ", extra, PROMContent[i+j]);
			}
			sprintf(extra,"%s\n",extra);
		}
	}
	else if (strcmp(tmp[0], "realmap") == 0)
	{
		mapLen = EFUSE_MAP_SIZE;
		if (rtw_efuse_map_read(padapter, 0, mapLen, pEfuseHal->fakeEfuseInitMap) == _FAIL)
		{
			DBG_88E("%s: read realmap Fail!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

		sprintf(extra, "\n");
		for (i = 0; i < EFUSE_MAP_SIZE; i += 16)
		{
			sprintf(extra, "%s0x%02x\t", extra, i);
			for (j=0; j<8; j++) {
				if (i + j >= EFUSE_MAX_MAP_LEN)
					continue;
				sprintf(extra, "%s%02X ", extra, pEfuseHal->fakeEfuseInitMap[i+j]);
			}
			sprintf(extra, "%s\t", extra);
			for (; j<16; j++) {
				if (i + j >= EFUSE_MAX_MAP_LEN)
					continue;
				sprintf(extra, "%s%02X ", extra, pEfuseHal->fakeEfuseInitMap[i+j]);
			}
			sprintf(extra,"%s\n",extra);
		}
	}
	else if (strcmp(tmp[0], "rmap") == 0)
	{
		if ((tmp[1]== NULL) || (tmp[2]== NULL))
		{
			DBG_88E("%s: rmap Fail!! Parameters error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}

		/*  rmap addr cnts */
		addr = simple_strtoul(tmp[1], &ptmp, 16);
		DBG_88E("%s: addr=%x\n", __FUNCTION__, addr);

		cnts = simple_strtoul(tmp[2], &ptmp, 10);
		if (cnts == 0)
		{
			DBG_88E("%s: rmap Fail!! cnts error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}
		DBG_88E("%s: cnts=%d\n", __FUNCTION__, cnts);

		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_available_size, false);
		if ((addr + cnts) > max_available_size)
		{
			DBG_88E("%s: addr(0x%X)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EINVAL;
			goto exit;
		}

		if (rtw_efuse_map_read(padapter, addr, cnts, data) == _FAIL)
		{
			DBG_88E("%s: rtw_efuse_map_read error!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

		*extra = 0;
		for (i=0; i<cnts; i++) {
			sprintf(extra, "%s0x%02X ", extra, data[i]);
		}
	}
	else if (strcmp(tmp[0], "realraw") == 0)
	{
		addr = 0;
		mapLen = EFUSE_MAX_SIZE;
		if (rtw_efuse_access(padapter, false, addr, mapLen, rawdata) == _FAIL)
		{
			DBG_88E("%s: rtw_efuse_access Fail!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

		sprintf(extra, "\n");
		for (i=0; i<mapLen; i++)
		{
			sprintf(extra, "%s%02X", extra, rawdata[i]);

			if ((i & 0xF) == 0xF) {
				sprintf(extra, "%s\n", extra);
			}
			else if ((i & 0x7) == 0x7) {
				sprintf(extra, "%s\t", extra);
			} else {
				sprintf(extra, "%s ", extra);
			}
		}
	}
	else if (strcmp(tmp[0], "mac") == 0)
	{
		addr = EEPROM_MAC_ADDR_88EU;

		cnts = 6;

		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_available_size, false);
		if ((addr + cnts) > max_available_size) {
			DBG_88E("%s: addr(0x%02x)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EFAULT;
			goto exit;
		}

		if (rtw_efuse_map_read(padapter, addr, cnts, data) == _FAIL)
		{
			DBG_88E("%s: rtw_efuse_map_read error!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

		*extra = 0;
		for (i=0; i<cnts; i++)
		{
			sprintf(extra, "%s%02X", extra, data[i]);
			if (i != (cnts-1))
				sprintf(extra,"%s:",extra);
		}
	}
	else if (strcmp(tmp[0], "vidpid") == 0)
	{
		cnts = 4;

		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_available_size, false);
		if ((addr + cnts) > max_available_size)
		{
			DBG_88E("%s: addr(0x%02x)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EFAULT;
			goto exit;
		}
		if (rtw_efuse_map_read(padapter, addr, cnts, data) == _FAIL)
		{
			DBG_88E("%s: rtw_efuse_access error!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

		*extra = 0;
		for (i=0; i<cnts; i++)
		{
			sprintf(extra, "%s0x%02X", extra, data[i]);
			if (i != (cnts-1))
				sprintf(extra,"%s,",extra);
		}
	}
	else if (strcmp(tmp[0], "ableraw") == 0)
	{
		efuse_GetCurrentSize(padapter,&raw_cursize);
		raw_maxsize = efuse_GetMaxSize(padapter);
		sprintf(extra, "[available raw size]= %d bytes", raw_maxsize-raw_cursize);
	}
	else if (strcmp(tmp[0], "btfmap") == 0)
	{
		mapLen = EFUSE_BT_MAX_MAP_LEN;
		if (rtw_BT_efuse_map_read(padapter, 0, mapLen, pEfuseHal->BTEfuseInitMap) == _FAIL)
		{
			DBG_88E("%s: rtw_BT_efuse_map_read Fail!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

		sprintf(extra, "\n");
		for (i=0; i<512; i+=16) /*  set 512 because the iwpriv's extra size have limit 0x7FF */
		{
			sprintf(extra, "%s0x%03x\t", extra, i);
			for (j=0; j<8; j++)
				sprintf(extra, "%s%02X ", extra, pEfuseHal->BTEfuseInitMap[i+j]);
			sprintf(extra,"%s\t",extra);
			for (; j<16; j++)
				sprintf(extra, "%s%02X ", extra, pEfuseHal->BTEfuseInitMap[i+j]);
			sprintf(extra, "%s\n", extra);
		}
	}
	else if (strcmp(tmp[0],"btbmap") == 0)
	{
		mapLen = EFUSE_BT_MAX_MAP_LEN;
		if (rtw_BT_efuse_map_read(padapter, 0, mapLen, pEfuseHal->BTEfuseInitMap) == _FAIL)
		{
			DBG_88E("%s: rtw_BT_efuse_map_read Fail!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

		sprintf(extra, "\n");
		for (i=512; i<1024 ; i+=16)
		{
			sprintf(extra, "%s0x%03x\t", extra, i);
			for (j=0; j<8; j++)
				sprintf(extra, "%s%02X ", extra, pEfuseHal->BTEfuseInitMap[i+j]);
			sprintf(extra,"%s\t",extra);
			for (; j<16; j++)
				sprintf(extra, "%s%02X ", extra, pEfuseHal->BTEfuseInitMap[i+j]);
			sprintf(extra, "%s\n", extra);
		}
	}
	else if (strcmp(tmp[0],"btrmap") == 0)
	{
		if ((tmp[1]== NULL) || (tmp[2]== NULL))
		{
			err = -EINVAL;
			goto exit;
		}

		/*  rmap addr cnts */
		addr = simple_strtoul(tmp[1], &ptmp, 16);
		DBG_88E("%s: addr=0x%X\n", __FUNCTION__, addr);

		cnts = simple_strtoul(tmp[2], &ptmp, 10);
		if (cnts == 0)
		{
			DBG_88E("%s: btrmap Fail!! cnts error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}
		DBG_88E("%s: cnts=%d\n", __FUNCTION__, cnts);

		EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_available_size, false);
		if ((addr + cnts) > max_available_size)
		{
			DBG_88E("%s: addr(0x%X)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EFAULT;
			goto exit;
		}

		if (rtw_BT_efuse_map_read(padapter, addr, cnts, data) == _FAIL)
		{
			DBG_88E("%s: rtw_BT_efuse_map_read error!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}

		*extra = 0;
/* 		DBG_88E("%s: bt efuse data={", __FUNCTION__); */
		for (i=0; i<cnts; i++)
		{
/* 			DBG_88E("0x%02x ", data[i]); */
			sprintf(extra, "%s 0x%02X ", extra, data[i]);
		}
/* 		DBG_88E("}\n"); */
	}
	else if (strcmp(tmp[0], "btffake") == 0)
	{
/* 		DBG_88E("OFFSET\tVALUE(hex)\n"); */
		sprintf(extra, "\n");
		for (i=0; i<512; i+=16)
		{
/* 			DBG_88E("0x%03x\t", i); */
			sprintf(extra, "%s0x%03x\t", extra, i);
			for (j=0; j<8; j++) {
/* 				DBG_88E("%02X ", pEfuseHal->fakeBTEfuseModifiedMap[i+j]); */
				sprintf(extra, "%s%02X ", extra, pEfuseHal->fakeBTEfuseModifiedMap[i+j]);
			}
/* 			DBG_88E("\t"); */
			sprintf(extra, "%s\t", extra);
			for (; j<16; j++) {
/* 				DBG_88E("%02X ", pEfuseHal->fakeBTEfuseModifiedMap[i+j]); */
				sprintf(extra, "%s%02X ", extra, pEfuseHal->fakeBTEfuseModifiedMap[i+j]);
			}
/* 			DBG_88E("\n"); */
			sprintf(extra, "%s\n", extra);
		}
/* 		DBG_88E("\n"); */
	}
	else if (strcmp(tmp[0],"btbfake") == 0)
	{
/* 		DBG_88E("OFFSET\tVALUE(hex)\n"); */
		sprintf(extra, "\n");
		for (i=512; i<1024; i+=16)
		{
/* 			DBG_88E("0x%03x\t", i); */
			sprintf(extra, "%s0x%03x\t", extra, i);
			for (j=0; j<8; j++) {
				if (i + j >= EFUSE_BT_MAX_MAP_LEN)
					continue;
/* 				DBG_88E("%02X ", pEfuseHal->fakeBTEfuseModifiedMap[i+j]); */
				sprintf(extra, "%s%02X ", extra, pEfuseHal->fakeBTEfuseModifiedMap[i+j]);
			}
/* 			DBG_88E("\t"); */
			sprintf(extra, "%s\t", extra);
			for (; j<16; j++) {
				if (i + j >= EFUSE_BT_MAX_MAP_LEN)
					continue;
/* 				DBG_88E("%02X ", pEfuseHal->fakeBTEfuseModifiedMap[i+j]); */
				sprintf(extra, "%s%02X ", extra, pEfuseHal->fakeBTEfuseModifiedMap[i+j]);
			}
/* 			DBG_88E("\n"); */
			sprintf(extra, "%s\n", extra);
		}
/* 		DBG_88E("\n"); */
	}
	else if (strcmp(tmp[0],"wlrfkmap")== 0)
	{
/* 		DBG_88E("OFFSET\tVALUE(hex)\n"); */
		sprintf(extra, "\n");
		for (i = 0; i < EFUSE_MAP_SIZE; i += 16) {
/* 			DBG_88E("\t0x%02x\t", i); */
			sprintf(extra, "%s0x%02x\t", extra, i);
			for (j=0; j<8; j++) {
				if (i + j >= EFUSE_MAX_MAP_LEN)
					continue;
/* 				DBG_88E("%02X ", pEfuseHal->fakeEfuseModifiedMap[i+j]); */
				sprintf(extra, "%s%02X ", extra, pEfuseHal->fakeEfuseModifiedMap[i+j]);
			}
/* 			DBG_88E("\t"); */
			sprintf(extra, "%s\t", extra);
			for (; j<16; j++) {
				if (i + j >= EFUSE_MAX_MAP_LEN)
					continue;
/* 				DBG_88E("%02X ", pEfuseHal->fakeEfuseModifiedMap[i+j]); */
				sprintf(extra, "%s %02X", extra, pEfuseHal->fakeEfuseModifiedMap[i+j]);
			}
/* 			DBG_88E("\n"); */
			sprintf(extra, "%s\n", extra);
		}
/* 		DBG_88E("\n"); */

	} else if (strcmp(tmp[0],"wlrfkrmap")== 0) {
		if ((tmp[1]== NULL) || (tmp[2]== NULL)) {
			DBG_88E("%s: rmap Fail!! Parameters error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}
		/*  rmap addr cnts */
		addr = simple_strtoul(tmp[1], &ptmp, 16);
		DBG_88E("%s: addr=%x\n", __FUNCTION__, addr);

		cnts = simple_strtoul(tmp[2], &ptmp, 10);
		if (cnts == 0) {
			DBG_88E("%s: rmap Fail!! cnts error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}
		DBG_88E("%s: cnts=%d\n", __FUNCTION__, cnts);

		*extra = 0;
		for (i=0; i<cnts; i++) {
			DBG_88E("wlrfkrmap = 0x%02x\n", pEfuseHal->fakeEfuseModifiedMap[addr+i]);
			sprintf(extra, "%s0x%02X ", extra, pEfuseHal->fakeEfuseModifiedMap[addr+i]);
		}
	} else if (strcmp(tmp[0],"btrfkrmap")== 0) {
		if ((tmp[1]== NULL) || (tmp[2]== NULL)) {
			DBG_88E("%s: rmap Fail!! Parameters error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}
		/*  rmap addr cnts */
		addr = simple_strtoul(tmp[1], &ptmp, 16);
		DBG_88E("%s: addr=%x\n", __FUNCTION__, addr);

		cnts = simple_strtoul(tmp[2], &ptmp, 10);
		if (cnts == 0) {
			DBG_88E("%s: rmap Fail!! cnts error!\n", __FUNCTION__);
			err = -EINVAL;
			goto exit;
		}
		DBG_88E("%s: cnts=%d\n", __FUNCTION__, cnts);

		*extra = 0;
		for (i=0; i<cnts; i++) {
				DBG_88E("wlrfkrmap = 0x%02x\n", pEfuseHal->fakeBTEfuseModifiedMap[addr+i]);
				sprintf(extra, "%s0x%02X ", extra, pEfuseHal->fakeBTEfuseModifiedMap[addr+i]);
		}
	} else {
		 sprintf(extra, "Command not found!");
	}

exit:
	if (data)
		rtw_mfree(data, EFUSE_BT_MAX_MAP_LEN);
	if (rawdata)
		rtw_mfree(rawdata, EFUSE_BT_MAX_MAP_LEN);
	if (!err)
		wrqu->length = strlen(extra);

	rtw_pm_set_ips(padapter, ips_mode);
	rtw_pm_set_lps(padapter, lps_mode);
	padapter->registrypriv.fw_iol = org_fw_iol;/*  0:Disable, 1:enable, 2:by usb speed */
	return err;
}

static int rtw_mp_efuse_set(struct net_device *dev,
			struct iw_request_info *info,
			union iwreq_data *wdata, char *extra)
{
	struct iw_point *wrqu;
	struct adapter *padapter;
	struct pwrctrl_priv *pwrctrlpriv ;
	PHAL_DATA_TYPE pHalData;
	PEFUSE_HAL pEfuseHal;

	u8 ips_mode,lps_mode;
	u32 i, jj, kk;
	u8 *setdata = NULL;
	u8 *ShadowMapBT = NULL;
	u8 *ShadowMapWiFi = NULL;
	u8 *setrawdata = NULL;
	char *pch, *ptmp, *token, *tmp[3]={NULL, NULL, NULL};
	u16 addr=0, cnts=0, max_available_size=0;
	int err;


	wrqu = (struct iw_point*)wdata;
	padapter = rtw_netdev_priv(dev);
	pwrctrlpriv = adapter_to_pwrctl(padapter);
	pHalData = GET_HAL_DATA(padapter);
	pEfuseHal = &pHalData->EfuseHal;
	err = 0;
	setdata = rtw_zmalloc(1024);
	if (setdata == NULL)
	{
		err = -ENOMEM;
		goto exit;
	}
	ShadowMapBT = rtw_malloc(EFUSE_BT_MAX_MAP_LEN);
	if (ShadowMapBT == NULL)
	{
		err = -ENOMEM;
		goto exit;
	}
	ShadowMapWiFi = rtw_malloc(EFUSE_MAP_SIZE);
	if (ShadowMapWiFi == NULL)
	{
		err = -ENOMEM;
		goto exit;
	}
	setrawdata = rtw_malloc(EFUSE_MAX_SIZE);
	if (setrawdata == NULL)
	{
		err = -ENOMEM;
		goto exit;
	}

	lps_mode = pwrctrlpriv->power_mgnt;/* keep org value */
	rtw_pm_set_lps(padapter,PS_MODE_ACTIVE);

	ips_mode = pwrctrlpriv->ips_mode;/* keep org value */
	rtw_pm_set_ips(padapter,IPS_NONE);

	pch = extra;
	DBG_88E("%s: in=%s\n", __FUNCTION__, extra);

	i = 0;
	while ((token = strsep(&pch, ",")) != NULL)
	{
		if (i > 2) break;
		tmp[i] = token;
		i++;
	}

	/*  tmp[0],[1],[2] */
	/*  wmap,addr,00e04c871200 */
	if (strcmp(tmp[0], "wmap") == 0)
	{
		if ((tmp[1]== NULL) || (tmp[2]== NULL))
		{
			err = -EINVAL;
			goto exit;
		}

		addr = simple_strtoul(tmp[1], &ptmp, 16);
		addr &= 0xFFF;

		cnts = strlen(tmp[2]);
		if (cnts%2)
		{
			err = -EINVAL;
			goto exit;
		}
		cnts /= 2;
		if (cnts == 0)
		{
			err = -EINVAL;
			goto exit;
		}

		DBG_88E("%s: addr=0x%X\n", __FUNCTION__, addr);
		DBG_88E("%s: cnts=%d\n", __FUNCTION__, cnts);
		DBG_88E("%s: map data=%s\n", __FUNCTION__, tmp[2]);

		for (jj=0, kk=0; jj<cnts; jj++, kk+=2)
		{
			setdata[jj] = key_2char2num(tmp[2][kk], tmp[2][kk+1]);
		}
		/* Change to check TYPE_EFUSE_MAP_LEN ,beacuse 8188E raw 256,logic map over 256. */
		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&max_available_size, false);
		if ((addr+cnts) > max_available_size)
		{
			DBG_88E("%s: addr(0x%X)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EFAULT;
			goto exit;
		}

		if (rtw_efuse_map_write(padapter, addr, cnts, setdata) == _FAIL)
		{
			DBG_88E("%s: rtw_efuse_map_write error!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}
	}
	else if (strcmp(tmp[0], "wraw") == 0)
	{
		if ((tmp[1]== NULL) || (tmp[2]== NULL))
		{
			err = -EINVAL;
			goto exit;
		}

		addr = simple_strtoul( tmp[1], &ptmp, 16 );
		addr &= 0xFFF;

		cnts = strlen(tmp[2]);
		if (cnts%2)
		{
			err = -EINVAL;
			goto exit;
		}
		cnts /= 2;
		if (cnts == 0)
		{
			err = -EINVAL;
			goto exit;
		}

		DBG_88E("%s: addr=0x%X\n", __FUNCTION__, addr);
		DBG_88E("%s: cnts=%d\n", __FUNCTION__, cnts);
		DBG_88E("%s: raw data=%s\n", __FUNCTION__, tmp[2]);

		for (jj=0, kk=0; jj<cnts; jj++, kk+=2)
		{
			setrawdata[jj] = key_2char2num(tmp[2][kk], tmp[2][kk+1]);
		}

		if (rtw_efuse_access(padapter, true, addr, cnts, setrawdata) == _FAIL)
		{
			DBG_88E("%s: rtw_efuse_access error!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}
	}
	else if (strcmp(tmp[0], "mac") == 0)
	{
		if (tmp[1]== NULL)
		{
			err = -EINVAL;
			goto exit;
		}

		addr = EEPROM_MAC_ADDR_88EU;

		cnts = strlen(tmp[1]);
		if (cnts%2)
		{
			err = -EINVAL;
			goto exit;
		}
		cnts /= 2;
		if (cnts == 0)
		{
			err = -EINVAL;
			goto exit;
		}
		if (cnts > 6)
		{
			DBG_88E("%s: error data for mac addr=\"%s\"\n", __FUNCTION__, tmp[1]);
			err = -EFAULT;
			goto exit;
		}

		DBG_88E("%s: addr=0x%X\n", __FUNCTION__, addr);
		DBG_88E("%s: cnts=%d\n", __FUNCTION__, cnts);
		DBG_88E("%s: MAC address=%s\n", __FUNCTION__, tmp[1]);

		for (jj=0, kk=0; jj<cnts; jj++, kk+=2)
		{
			setdata[jj] = key_2char2num(tmp[1][kk], tmp[1][kk+1]);
		}
		/* Change to check TYPE_EFUSE_MAP_LEN ,beacuse 8188E raw 256,logic map over 256. */
		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&max_available_size, false);
		if ((addr+cnts) > max_available_size)
		{
			DBG_88E("%s: addr(0x%X)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EFAULT;
			goto exit;
		}

		if (rtw_efuse_map_write(padapter, addr, cnts, setdata) == _FAIL)
		{
			DBG_88E("%s: rtw_efuse_map_write error!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}
	}
	else if (strcmp(tmp[0], "vidpid") == 0)
	{
		if (tmp[1]== NULL)
		{
			err = -EINVAL;
			goto exit;
		}

		addr = EEPROM_VID_88EE;
		cnts = strlen(tmp[1]);
		if (cnts%2) {
			err = -EINVAL;
			goto exit;
		}
		cnts /= 2;
		if (cnts == 0) {
			err = -EINVAL;
			goto exit;
		}

		DBG_88E("%s: addr=0x%X\n", __FUNCTION__, addr);
		DBG_88E("%s: cnts=%d\n", __FUNCTION__, cnts);
		DBG_88E("%s: VID/PID=%s\n", __FUNCTION__, tmp[1]);

		for (jj=0, kk=0; jj<cnts; jj++, kk+=2)
		{
			setdata[jj] = key_2char2num(tmp[1][kk], tmp[1][kk+1]);
		}

		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_available_size, false);
		if ((addr+cnts) > max_available_size)
		{
			DBG_88E("%s: addr(0x%X)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EFAULT;
			goto exit;
		}

		if (rtw_efuse_map_write(padapter, addr, cnts, setdata) == _FAIL)
		{
			DBG_88E("%s: rtw_efuse_map_write error!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}
	}
    else if (strcmp(tmp[0], "wldumpfake") == 0)
	{
		if (rtw_efuse_map_read(padapter, 0, EFUSE_MAP_SIZE,  pEfuseHal->fakeEfuseModifiedMap) == _SUCCESS) {
			DBG_88E("%s: WiFi hw efuse dump to Fake map success\n", __FUNCTION__);
		} else {
			DBG_88E("%s: WiFi hw efuse dump to Fake map Fail\n", __FUNCTION__);
			err = -EFAULT;
		}
	}
	else if (strcmp(tmp[0], "btwmap") == 0)
	{
		if ((tmp[1]== NULL) || (tmp[2]== NULL))
		{
			err = -EINVAL;
			goto exit;
		}

		addr = simple_strtoul(tmp[1], &ptmp, 16);
		addr &= 0xFFF;

		cnts = strlen(tmp[2]);
		if (cnts%2)
		{
			err = -EINVAL;
			goto exit;
		}
		cnts /= 2;
		if (cnts == 0)
		{
			err = -EINVAL;
			goto exit;
		}

		DBG_88E("%s: addr=0x%X\n", __FUNCTION__, addr);
		DBG_88E("%s: cnts=%d\n", __FUNCTION__, cnts);
		DBG_88E("%s: BT data=%s\n", __FUNCTION__, tmp[2]);

		for (jj=0, kk=0; jj<cnts; jj++, kk+=2)
		{
			setdata[jj] = key_2char2num(tmp[2][kk], tmp[2][kk+1]);
		}

		EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_available_size, false);
		if ((addr+cnts) > max_available_size)
		{
			DBG_88E("%s: addr(0x%X)+cnts(%d) parameter error!\n", __FUNCTION__, addr, cnts);
			err = -EFAULT;
			goto exit;
		}

		if (rtw_BT_efuse_map_write(padapter, addr, cnts, setdata) == _FAIL)
		{
			DBG_88E("%s: rtw_BT_efuse_map_write error!!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}
	}
	else if (strcmp(tmp[0], "btwfake") == 0)
	{
		if ((tmp[1]== NULL) || (tmp[2]== NULL))
		{
			err = -EINVAL;
			goto exit;
		}

		addr = simple_strtoul(tmp[1], &ptmp, 16);
		addr &= 0xFFF;

		cnts = strlen(tmp[2]);
		if (cnts%2)
		{
			err = -EINVAL;
			goto exit;
		}
		cnts /= 2;
		if (cnts == 0)
		{
			err = -EINVAL;
			goto exit;
		}

		DBG_88E("%s: addr=0x%X\n", __FUNCTION__, addr);
		DBG_88E("%s: cnts=%d\n", __FUNCTION__, cnts);
		DBG_88E("%s: BT tmp data=%s\n", __FUNCTION__, tmp[2]);

		for (jj=0, kk=0; jj<cnts; jj++, kk+=2)
		{
			pEfuseHal->fakeBTEfuseModifiedMap[addr+jj] = key_2char2num(tmp[2][kk], tmp[2][kk+1]);
		}
	}
	else if (strcmp(tmp[0], "btdumpfake") == 0)
	{
		if (rtw_BT_efuse_map_read(padapter, 0, EFUSE_BT_MAX_MAP_LEN, pEfuseHal->fakeBTEfuseModifiedMap) == _SUCCESS) {
			DBG_88E("%s: BT read all map success\n", __FUNCTION__);
		} else {
			DBG_88E("%s: BT read all map Fail!\n", __FUNCTION__);
			err = -EFAULT;
		}
	}
	else if (strcmp(tmp[0], "btfk2map") == 0)
	{
		memcpy(pEfuseHal->BTEfuseModifiedMap, pEfuseHal->fakeBTEfuseModifiedMap, EFUSE_BT_MAX_MAP_LEN);

		EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_available_size, false);
		if (max_available_size < 1)
		{
			err = -EFAULT;
			goto exit;
		}

		if (rtw_BT_efuse_map_write(padapter, 0x00, EFUSE_BT_MAX_MAP_LEN, pEfuseHal->fakeBTEfuseModifiedMap) == _FAIL)
		{
			DBG_88E("%s: rtw_BT_efuse_map_write error!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}
	}
	else if (strcmp(tmp[0], "wlfk2map") == 0)
	{
		EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_available_size, false);
		if (max_available_size < 1)
		{
			err = -EFAULT;
			goto exit;
		}

		if (rtw_efuse_map_write(padapter, 0x00, EFUSE_MAP_SIZE, pEfuseHal->fakeEfuseModifiedMap) == _FAIL)
		{
			DBG_88E("%s: rtw_efuse_map_write fakeEfuseModifiedMap error!\n", __FUNCTION__);
			err = -EFAULT;
			goto exit;
		}
	}
	else if (strcmp(tmp[0], "wlwfake") == 0)
	{
		if ((tmp[1]== NULL) || (tmp[2]== NULL))
		{
			err = -EINVAL;
			goto exit;
		}

		addr = simple_strtoul(tmp[1], &ptmp, 16);
		addr &= 0xFFF;

		cnts = strlen(tmp[2]);
		if (cnts%2)
		{
			err = -EINVAL;
			goto exit;
		}
		cnts /= 2;
		if (cnts == 0)
		{
			err = -EINVAL;
			goto exit;
		}

		DBG_88E("%s: addr=0x%X\n", __FUNCTION__, addr);
		DBG_88E("%s: cnts=%d\n", __FUNCTION__, cnts);
		DBG_88E("%s: map tmp data=%s\n", __FUNCTION__, tmp[2]);

		for (jj=0, kk=0; jj<cnts; jj++, kk+=2)
		{
			pEfuseHal->fakeEfuseModifiedMap[addr+jj] = key_2char2num(tmp[2][kk], tmp[2][kk+1]);
		}
	}

exit:
	if (setdata)
		rtw_mfree(setdata, 1024);
	if (ShadowMapBT)
		rtw_mfree(ShadowMapBT, EFUSE_BT_MAX_MAP_LEN);
	if (ShadowMapWiFi)
		rtw_mfree(ShadowMapWiFi, EFUSE_MAP_SIZE);
	if (setrawdata)
		rtw_mfree(setrawdata, EFUSE_MAX_SIZE);

	rtw_pm_set_ips(padapter, ips_mode);
	rtw_pm_set_lps(padapter, lps_mode);
	return err;
}

static int rtw_wfd_tdls_enable(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	return 0;
}

static int rtw_tdls_weaksec(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	return 0;
}


static int rtw_tdls_enable(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	return 0;
}

static int rtw_tdls_setup(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	return 0;
}

static int rtw_tdls_teardown(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	return 0;
}

static int rtw_tdls_discovery(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	return 0;
}

static int rtw_tdls_ch_switch (struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	return 0;
}

static int rtw_tdls_pson(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	return 0;
}

static int rtw_tdls_psoff(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	return 0;
}

static int rtw_tdls_setip(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	return 0;
}

static int rtw_tdls_getip(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	return 0;
}

static int rtw_tdls_getport(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	return 0;
}

/* WFDTDLS, for sigma test */
static int rtw_tdls_dis_result(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	return 0;
}

/* WFDTDLS, for sigma test */
static int rtw_wfd_tdls_status(struct net_device *dev,
                               struct iw_request_info *info,
                               union iwreq_data *wrqu, char *extra)
{
	return 0;
}

static int rtw_tdls_ch_switch_off(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	return 0;
}

static int rtw_tdls(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	return 0;
}


static int rtw_tdls_get(struct net_device *dev,
				struct iw_request_info *info,
				union iwreq_data *wrqu, char *extra)
{
	int ret = 0;

#ifdef CONFIG_P2P

	DBG_88E( "[%s] extra = %s\n", __FUNCTION__, (__force char *)wrqu->data.pointer );

	if ( _rtw_memcmp((__force void *)wrqu->data.pointer, "ip", 2 ) )
	{
		rtw_tdls_getip( dev, info, wrqu, extra );
	}
	if ( _rtw_memcmp((__force void *)wrqu->data.pointer, "port", 4 ) )
	{
		rtw_tdls_getport( dev, info, wrqu, extra );
	}
	/* WFDTDLS, for sigma test */
	if ( _rtw_memcmp((__force void *)wrqu->data.pointer, "dis", 3 ) )
	{
		rtw_tdls_dis_result( dev, info, wrqu, extra );
	}
	if (_rtw_memcmp((__force void *)wrqu->data.pointer, "status", 6))
		rtw_wfd_tdls_status( dev, info, wrqu, extra );

#endif /* CONFIG_P2P */

	return ret;
}

#ifdef CONFIG_MAC_LOOPBACK_DRIVER

#include <rtl8188e_hal.h>
extern void rtl8188e_cal_txdesc_chksum(struct tx_desc *ptxdesc);
#define cal_txdesc_chksum rtl8188e_cal_txdesc_chksum

static s32 initLoopback(struct adapter *padapter)
{
	PLOOPBACKDATA ploopback;


	if (padapter->ploopback == NULL) {
		ploopback = (PLOOPBACKDATA)rtw_zmalloc(sizeof(LOOPBACKDATA));
		if (ploopback == NULL) return -ENOMEM;

		_rtw_init_sema(&ploopback->sema, 0);
		ploopback->bstop = true;
		ploopback->cnt = 0;
		ploopback->size = 300;
		memset(ploopback->msg, 0, sizeof(ploopback->msg));

		padapter->ploopback = ploopback;
	}

	return 0;
}

static void freeLoopback(struct adapter *padapter)
{
	PLOOPBACKDATA ploopback;


	ploopback = padapter->ploopback;
	if (ploopback) {
		rtw_mfree((u8*)ploopback, sizeof(LOOPBACKDATA));
		padapter->ploopback = NULL;
	}
}

static s32 initpseudoadhoc(struct adapter *padapter)
{
	enum NDIS_802_11_NETWORK_INFRASTRUCTURE networkType;
	s32 err;

	networkType = Ndis802_11IBSS;
	err = rtw_set_802_11_infrastructure_mode(padapter, networkType);
	if (err == false) return _FAIL;

	err = rtw_setopmode_cmd(padapter, networkType,true);
	if (err == _FAIL) return _FAIL;

	return _SUCCESS;
}

static s32 createpseudoadhoc(struct adapter *padapter)
{
	enum NDIS_802_11_AUTHENTICATION_MODE authmode;
	struct mlme_priv *pmlmepriv;
	struct ndis_802_11_ssid *passoc_ssid;
	struct wlan_bssid_ex *pdev_network;
	u8 *pibss;
	u8 ssid[] = "pseduo_ad-hoc";
	s32 err;
	unsigned long irqL;


	pmlmepriv = &padapter->mlmepriv;

	authmode = Ndis802_11AuthModeOpen;
	err = rtw_set_802_11_authentication_mode(padapter, authmode);
	if (err == false) return _FAIL;

	passoc_ssid = &pmlmepriv->assoc_ssid;
	memset(passoc_ssid, 0, sizeof(struct ndis_802_11_ssid));
	passoc_ssid->SsidLength = sizeof(ssid) - 1;
	memcpy(passoc_ssid->Ssid, ssid, passoc_ssid->SsidLength);

	pdev_network = &padapter->registrypriv.dev_network;
	pibss = padapter->registrypriv.dev_network.MacAddress;
	memcpy(&pdev_network->Ssid, passoc_ssid, sizeof(struct ndis_802_11_ssid));

	rtw_update_registrypriv_dev_network(padapter);
	rtw_generate_random_ibss(pibss);

	spin_lock_bh(&pmlmepriv->lock);
	pmlmepriv->fw_state = WIFI_ADHOC_MASTER_STATE;
	spin_unlock_bh(&pmlmepriv->lock);

{
	struct wlan_network *pcur_network;
	struct sta_info *psta;

	/* 3  create a new psta */
	pcur_network = &pmlmepriv->cur_network;

	/* clear psta in the cur_network, if any */
	psta = rtw_get_stainfo(&padapter->stapriv, pcur_network->network.MacAddress);
	if (psta) rtw_free_stainfo(padapter, psta);

	psta = rtw_alloc_stainfo(&padapter->stapriv, pibss);
	if (psta == NULL) return _FAIL;

	/* 3  join psudo AdHoc */
	pcur_network->join_res = 1;
	pcur_network->aid = psta->aid = 1;
	memcpy(&pcur_network->network, pdev_network, get_wlan_bssid_ex_sz(pdev_network));
	{
		u8 val8;

		val8 = rtw_read8(padapter, MSR);
		val8 &= 0xFC; /*  clear NETYPE0 */
		val8 |= WIFI_FW_ADHOC_STATE & 0x3;
		rtw_write8(padapter, MSR, val8);
	}
}

	return _SUCCESS;
}

static struct xmit_frame* createloopbackpkt(struct adapter *padapter, u32 size)
{
	struct xmit_priv *pxmitpriv;
	struct xmit_frame *pframe;
	struct xmit_buf *pxmitbuf;
	struct pkt_attrib *pattrib;
	struct tx_desc *desc;
	u8 *pkt_start, *pkt_end, *ptr;
	struct rtw_ieee80211_hdr *hdr;
	s32 bmcast;
	unsigned long irqL;


	if ((TXDESC_SIZE + WLANHDR_OFFSET + size) > MAX_XMITBUF_SZ) return NULL;

	pxmitpriv = &padapter->xmitpriv;
	pframe = NULL;

	/* 2 1. allocate xmit frame */
	pframe = rtw_alloc_xmitframe(pxmitpriv);
	if (pframe == NULL) return NULL;
	pframe->padapter = padapter;

	/* 2 2. allocate xmit buffer */
	spin_lock_bh(&pxmitpriv->lock);
	pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);
	spin_unlock_bh(&pxmitpriv->lock);
	if (pxmitbuf == NULL) {
		rtw_free_xmitframe(pxmitpriv, pframe);
		return NULL;
	}

	pframe->pxmitbuf = pxmitbuf;
	pframe->buf_addr = pxmitbuf->pbuf;
	pxmitbuf->priv_data = pframe;

	/* 2 3. update_attrib() */
	pattrib = &pframe->attrib;

	/*  init xmitframe attribute */
	memset(pattrib, 0, sizeof(struct pkt_attrib));

	pattrib->ether_type = 0x8723;
	memcpy(pattrib->src, padapter->eeprompriv.mac_addr, ETH_ALEN);
	memcpy(pattrib->ta, pattrib->src, ETH_ALEN);
	memset(pattrib->dst, 0xFF, ETH_ALEN);
	memcpy(pattrib->ra, pattrib->dst, ETH_ALEN);
/* 	pattrib->pctrl = 0; */
/* 	pattrib->dhcp_pkt = 0; */
/* 	pattrib->pktlen = 0; */
	pattrib->ack_policy = 0;
/* 	pattrib->pkt_hdrlen = ETH_HLEN; */
	pattrib->hdrlen = WLAN_HDR_A3_LEN;
	pattrib->subtype = WIFI_DATA;
	pattrib->priority = 0;
	pattrib->qsel = pattrib->priority;
	pattrib->nr_frags = 1;
	pattrib->encrypt = 0;
	pattrib->bswenc = false;
	pattrib->qos_en = false;

	bmcast = IS_MCAST(pattrib->ra);
	if (bmcast) {
		pattrib->mac_id = 1;
		pattrib->psta = rtw_get_bcmc_stainfo(padapter);
	} else {
		pattrib->mac_id = 0;
		pattrib->psta = rtw_get_stainfo(&padapter->stapriv, get_bssid(&padapter->mlmepriv));
	}

	pattrib->pktlen = size;
	pattrib->last_txcmdsz = pattrib->hdrlen + pattrib->pktlen;

	/* 2 4. fill TX descriptor */
	desc = (struct tx_desc*)pframe->buf_addr;
	memset(desc, 0, TXDESC_SIZE);

	fill_default_txdesc(pframe, (u8*)desc);

	/*  Hw set sequence number */
	((PTXDESC)desc)->hwseq_en = 0; /*  HWSEQ_EN, 0:disable, 1:enable */

	((PTXDESC)desc)->disdatafb = 1;

	/*  convert to little endian */
	desc->txdw0 = cpu_to_le32(desc->txdw0);
	desc->txdw1 = cpu_to_le32(desc->txdw1);
	desc->txdw2 = cpu_to_le32(desc->txdw2);
	desc->txdw3 = cpu_to_le32(desc->txdw3);
	desc->txdw4 = cpu_to_le32(desc->txdw4);
	desc->txdw5 = cpu_to_le32(desc->txdw5);
	desc->txdw6 = cpu_to_le32(desc->txdw6);
	desc->txdw7 = cpu_to_le32(desc->txdw7);

	cal_txdesc_chksum(desc);

	/* 2 5. coalesce */
	pkt_start = pframe->buf_addr + TXDESC_SIZE;
	pkt_end = pkt_start + pattrib->last_txcmdsz;

	/* 3 5.1. make wlan header, make_wlanhdr() */
	hdr = (struct rtw_ieee80211_hdr *)pkt_start;
	SetFrameSubType(&hdr->frame_ctl, pattrib->subtype);
	memcpy(hdr->addr1, pattrib->dst, ETH_ALEN); /*  DA */
	memcpy(hdr->addr2, pattrib->src, ETH_ALEN); /*  SA */
	memcpy(hdr->addr3, get_bssid(&padapter->mlmepriv), ETH_ALEN); /*  RA, BSSID */

	/* 3 5.2. make payload */
	ptr = pkt_start + pattrib->hdrlen;
	get_random_bytes(ptr, pkt_end - ptr);

	pxmitbuf->len = TXDESC_SIZE + pattrib->last_txcmdsz;
	pxmitbuf->ptail += pxmitbuf->len;

	return pframe;
}

static void freeloopbackpkt(struct adapter *padapter, struct xmit_frame *pframe)
{
	struct xmit_priv *pxmitpriv;
	struct xmit_buf *pxmitbuf;


	pxmitpriv = &padapter->xmitpriv;
	pxmitbuf = pframe->pxmitbuf;

	rtw_free_xmitframe(pxmitpriv, pframe);
	rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
}

static void printdata(u8 *pbuf, u32 len)
{
	u32 i, val;


	for (i = 0; (i+4) <= len; i+=4) {
		printk("%08X", *(u32*)(pbuf + i));
		if ((i+4) & 0x1F) printk(" ");
		else printk("\n");
	}

	if (i < len)
	{
#ifdef __BIG_ENDIAN
		for (; i < len, i++)
			printk("%02X", pbuf+i);
#else /*  __LITTLE_ENDIAN */
		u8 str[9];
		u8 n;
		val = 0;
		n = len - i;
		memcpy(&val, pbuf+i, n);
		sprintf(str, "%08X", val);
		n = (4 - n) * 2;
		printk("%8s", str+n);
#endif /*  __LITTLE_ENDIAN */
	}
	printk("\n");
}

static u8 pktcmp(struct adapter *padapter, u8 *txbuf, u32 txsz, u8 *rxbuf, u32 rxsz)
{
	PHAL_DATA_TYPE phal;
	struct recv_stat *prxstat;
	struct recv_stat report;
	PRXREPORT prxreport;
	u32 drvinfosize;
	u32 rxpktsize;
	u8 fcssize;
	u8 ret = false;

	prxstat = (struct recv_stat*)rxbuf;
	report.rxdw0 = le32_to_cpu(prxstat->rxdw0);
	report.rxdw1 = le32_to_cpu(prxstat->rxdw1);
	report.rxdw2 = le32_to_cpu(prxstat->rxdw2);
	report.rxdw3 = le32_to_cpu(prxstat->rxdw3);
	report.rxdw4 = le32_to_cpu(prxstat->rxdw4);
	report.rxdw5 = le32_to_cpu(prxstat->rxdw5);

	prxreport = (PRXREPORT)&report;
	drvinfosize = prxreport->drvinfosize << 3;
	rxpktsize = prxreport->pktlen;

	phal = GET_HAL_DATA(padapter);
	if (phal->ReceiveConfig & RCR_APPFCS) fcssize = IEEE80211_FCS_LEN;
	else fcssize = 0;

	if ((txsz - TXDESC_SIZE) != (rxpktsize - fcssize)) {
		DBG_8192C("%s: ERROR! size not match tx/rx=%d/%d !\n",
			__func__, txsz - TXDESC_SIZE, rxpktsize - fcssize);
		ret = false;
	} else {
		ret = _rtw_memcmp(txbuf + TXDESC_SIZE,\
						  rxbuf + RXDESC_SIZE + drvinfosize,\
						  txsz - TXDESC_SIZE);
		if (ret == false) {
			DBG_8192C("%s: ERROR! pkt content mismatch!\n", __func__);
		}
	}

	if (ret == false)
	{
		DBG_8192C("\n%s: TX PKT total=%d, desc=%d, content=%d\n",
			__func__, txsz, TXDESC_SIZE, txsz - TXDESC_SIZE);
		DBG_8192C("%s: TX DESC size=%d\n", __func__, TXDESC_SIZE);
		printdata(txbuf, TXDESC_SIZE);
		DBG_8192C("%s: TX content size=%d\n", __func__, txsz - TXDESC_SIZE);
		printdata(txbuf + TXDESC_SIZE, txsz - TXDESC_SIZE);

		DBG_8192C("\n%s: RX PKT read=%d offset=%d(%d,%d) content=%d\n",
			__func__, rxsz, RXDESC_SIZE + drvinfosize, RXDESC_SIZE, drvinfosize, rxpktsize);
		if (rxpktsize != 0)
		{
			DBG_8192C("%s: RX DESC size=%d\n", __func__, RXDESC_SIZE);
			printdata(rxbuf, RXDESC_SIZE);
			DBG_8192C("%s: RX drvinfo size=%d\n", __func__, drvinfosize);
			printdata(rxbuf + RXDESC_SIZE, drvinfosize);
			DBG_8192C("%s: RX content size=%d\n", __func__, rxpktsize);
			printdata(rxbuf + RXDESC_SIZE + drvinfosize, rxpktsize);
		} else {
			DBG_8192C("%s: RX data size=%d\n", __func__, rxsz);
			printdata(rxbuf, rxsz);
		}
	}

	return ret;
}

int lbk_thread(void * context)
{
	s32 err;
	struct adapter *padapter;
	PLOOPBACKDATA ploopback;
	struct xmit_frame *pxmitframe;
	u32 cnt, ok, fail, headerlen;
	u32 pktsize;
	u32 ff_hwaddr;


	padapter = (struct adapter *)context;
	ploopback = padapter->ploopback;
	if (ploopback == NULL) return -1;
	cnt = 0;
	ok = 0;
	fail = 0;

	daemonize("%s", "RTW_LBK_THREAD");
	allow_signal(SIGTERM);

	do {
		if (ploopback->size == 0) {
			get_random_bytes(&pktsize, 4);
			pktsize = (pktsize % 1535) + 1; /*  1~1535 */
		} else
			pktsize = ploopback->size;

		pxmitframe = createloopbackpkt(padapter, pktsize);
		if (pxmitframe == NULL) {
			sprintf(ploopback->msg, "loopback FAIL! 3. create Packet FAIL!");
			break;
		}

		ploopback->txsize = TXDESC_SIZE + pxmitframe->attrib.last_txcmdsz;
		memcpy(ploopback->txbuf, pxmitframe->buf_addr, ploopback->txsize);
		ff_hwaddr = rtw_get_ff_hwaddr(pxmitframe);
		cnt++;
		DBG_8192C("%s: wirte port cnt=%d size=%d\n", __func__, cnt, ploopback->txsize);
		pxmitframe->pxmitbuf->pdata = ploopback->txbuf;
		rtw_write_port(padapter, ff_hwaddr, ploopback->txsize, (u8 *)pxmitframe->pxmitbuf);

		/*  wait for rx pkt */
		_rtw_down_sema(&ploopback->sema);

		err = pktcmp(padapter, ploopback->txbuf, ploopback->txsize, ploopback->rxbuf, ploopback->rxsize);
		if (err == true)
			ok++;
		else
			fail++;

		ploopback->txsize = 0;
		memset(ploopback->txbuf, 0, 0x8000);
		ploopback->rxsize = 0;
		memset(ploopback->rxbuf, 0, 0x8000);

		freeloopbackpkt(padapter, pxmitframe);
		pxmitframe = NULL;

		if (signal_pending(current)) {
			flush_signals(current);
		}

		if ((ploopback->bstop == true) ||
			((ploopback->cnt != 0) && (ploopback->cnt == cnt)))
		{
			u32 ok_rate, fail_rate, all;
			all = cnt;
			ok_rate = (ok*100)/all;
			fail_rate = (fail*100)/all;
			sprintf(ploopback->msg,\
					"loopback result: ok=%d%%(%d/%d),error=%d%%(%d/%d)",\
					ok_rate, ok, all, fail_rate, fail, all);
			break;
		}
	} while (1);

	ploopback->bstop = true;

	thread_exit();
}

static void loopbackTest(struct adapter *padapter, u32 cnt, u32 size, u8* pmsg)
{
	PLOOPBACKDATA ploopback;
	u32 len;
	s32 err;


	ploopback = padapter->ploopback;

	if (ploopback)
	{
		if (ploopback->bstop == false) {
			ploopback->bstop = true;
			_rtw_up_sema(&ploopback->sema);
		}
		len = 0;
		do {
			len = strlen(ploopback->msg);
			if (len) break;
			rtw_msleep_os(1);
		} while (1);
		memcpy(pmsg, ploopback->msg, len+1);
		freeLoopback(padapter);

		return;
	}

	/*  disable dynamic algorithm */
	{
	u32 DMFlag = DYNAMIC_FUNC_DISABLE;
	rtw_hal_get_hwreg(padapter, HW_VAR_DM_FLAG, (u8*)&DMFlag);
	}

	/*  create pseudo ad-hoc connection */
	err = initpseudoadhoc(padapter);
	if (err == _FAIL) {
		sprintf(pmsg, "loopback FAIL! 1.1 init ad-hoc FAIL!");
		return;
	}

	err = createpseudoadhoc(padapter);
	if (err == _FAIL) {
		sprintf(pmsg, "loopback FAIL! 1.2 create ad-hoc master FAIL!");
		return;
	}

	err = initLoopback(padapter);
	if (err) {
		sprintf(pmsg, "loopback FAIL! 2. init FAIL! error code=%d", err);
		return;
	}

	ploopback = padapter->ploopback;

	ploopback->bstop = false;
	ploopback->cnt = cnt;
	ploopback->size = size;
	ploopback->lbkthread = kthread_run(lbk_thread, padapter, "RTW_LBK_THREAD");
	if (IS_ERR(padapter->lbkthread))
	{
		freeLoopback(padapter);
		sprintf(pmsg, "loopback start FAIL! cnt=%d", cnt);
		return;
	}

	sprintf(pmsg, "loopback start! cnt=%d", cnt);
}
#endif /*  CONFIG_MAC_LOOPBACK_DRIVER */

static int rtw_test(
	struct net_device *dev,
	struct iw_request_info *info,
	union iwreq_data *wrqu, char *extra)
{
	u32 len;
	u8 *pbuf, *pch;
	char *ptmp;
	u8 *delim = ",";
	struct adapter *padapter = rtw_netdev_priv(dev);


	DBG_88E("+%s\n", __func__);
	len = wrqu->data.length;

	pbuf = (u8*)rtw_zmalloc(len);
	if (pbuf == NULL) {
		DBG_88E("%s: no memory!\n", __func__);
		return -ENOMEM;
	}

	if (copy_from_user(pbuf, wrqu->data.pointer, len)) {
		rtw_mfree(pbuf, len);
		DBG_88E("%s: copy from user fail!\n", __func__);
		return -EFAULT;
	}
	DBG_88E("%s: string=\"%s\"\n", __func__, pbuf);

	ptmp = (char*)pbuf;
	pch = strsep(&ptmp, delim);
	if ((pch == NULL) || (strlen(pch) == 0)) {
		rtw_mfree(pbuf, len);
		DBG_88E("%s: parameter error(level 1)!\n", __func__);
		return -EFAULT;
	}

#ifdef CONFIG_MAC_LOOPBACK_DRIVER
	if (strcmp(pch, "loopback") == 0)
	{
		s32 cnt = 0;
		u32 size = 64;

		pch = strsep(&ptmp, delim);
		if ((pch == NULL) || (strlen(pch) == 0)) {
			rtw_mfree(pbuf, len);
			DBG_88E("%s: parameter error(level 2)!\n", __func__);
			return -EFAULT;
		}

		sscanf(pch, "%d", &cnt);
		DBG_88E("%s: loopback cnt=%d\n", __func__, cnt);

		pch = strsep(&ptmp, delim);
		if ((pch == NULL) || (strlen(pch) == 0)) {
			rtw_mfree(pbuf, len);
			DBG_88E("%s: parameter error(level 2)!\n", __func__);
			return -EFAULT;
		}

		sscanf(pch, "%d", &size);
		DBG_88E("%s: loopback size=%d\n", __func__, size);

		loopbackTest(padapter, cnt, size, extra);
		wrqu->data.length = strlen(extra) + 1;

		rtw_mfree(pbuf, len);
		return 0;
	}
#endif

	rtw_mfree(pbuf, len);
	return 0;
}

static iw_handler rtw_handlers[] =
{
	NULL,					/* SIOCSIWCOMMIT */
	rtw_wx_get_name,		/* SIOCGIWNAME */
	dummy,					/* SIOCSIWNWID */
	dummy,					/* SIOCGIWNWID */
	rtw_wx_set_freq,		/* SIOCSIWFREQ */
	rtw_wx_get_freq,		/* SIOCGIWFREQ */
	rtw_wx_set_mode,		/* SIOCSIWMODE */
	rtw_wx_get_mode,		/* SIOCGIWMODE */
	dummy,					/* SIOCSIWSENS */
	rtw_wx_get_sens,		/* SIOCGIWSENS */
	NULL,					/* SIOCSIWRANGE */
	rtw_wx_get_range,		/* SIOCGIWRANGE */
	rtw_wx_set_priv,		/* SIOCSIWPRIV */
	NULL,					/* SIOCGIWPRIV */
	NULL,					/* SIOCSIWSTATS */
	NULL,					/* SIOCGIWSTATS */
	dummy,					/* SIOCSIWSPY */
	dummy,					/* SIOCGIWSPY */
	NULL,					/* SIOCGIWTHRSPY */
	NULL,					/* SIOCWIWTHRSPY */
	rtw_wx_set_wap,		/* SIOCSIWAP */
	rtw_wx_get_wap,		/* SIOCGIWAP */
	rtw_wx_set_mlme,		/* request MLME operation; uses struct iw_mlme */
	dummy,					/* SIOCGIWAPLIST -- depricated */
	rtw_wx_set_scan,		/* SIOCSIWSCAN */
	rtw_wx_get_scan,		/* SIOCGIWSCAN */
	rtw_wx_set_essid,		/* SIOCSIWESSID */
	rtw_wx_get_essid,		/* SIOCGIWESSID */
	dummy,					/* SIOCSIWNICKN */
	rtw_wx_get_nick,		/* SIOCGIWNICKN */
	NULL,					/* -- hole -- */
	NULL,					/* -- hole -- */
	rtw_wx_set_rate,		/* SIOCSIWRATE */
	rtw_wx_get_rate,		/* SIOCGIWRATE */
	rtw_wx_set_rts,			/* SIOCSIWRTS */
	rtw_wx_get_rts,			/* SIOCGIWRTS */
	rtw_wx_set_frag,		/* SIOCSIWFRAG */
	rtw_wx_get_frag,		/* SIOCGIWFRAG */
	dummy,					/* SIOCSIWTXPOW */
	dummy,					/* SIOCGIWTXPOW */
	dummy,					/* SIOCSIWRETRY */
	rtw_wx_get_retry,		/* SIOCGIWRETRY */
	rtw_wx_set_enc,			/* SIOCSIWENCODE */
	rtw_wx_get_enc,			/* SIOCGIWENCODE */
	dummy,					/* SIOCSIWPOWER */
	rtw_wx_get_power,		/* SIOCGIWPOWER */
	NULL,					/*---hole---*/
	NULL,					/*---hole---*/
	rtw_wx_set_gen_ie,		/* SIOCSIWGENIE */
	NULL,					/* SIOCGWGENIE */
	rtw_wx_set_auth,		/* SIOCSIWAUTH */
	NULL,					/* SIOCGIWAUTH */
	rtw_wx_set_enc_ext,		/* SIOCSIWENCODEEXT */
	NULL,					/* SIOCGIWENCODEEXT */
	rtw_wx_set_pmkid,		/* SIOCSIWPMKSA */
	NULL,					/*---hole---*/
};

static const struct iw_priv_args rtw_private_args[] = {
	{
		SIOCIWFIRSTPRIV + 0x0,
		IW_PRIV_TYPE_CHAR | 0x7FF, 0, "write"
	},
	{
		SIOCIWFIRSTPRIV + 0x1,
		IW_PRIV_TYPE_CHAR | 0x7FF,
		IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | IFNAMSIZ, "read"
	},
	{
		SIOCIWFIRSTPRIV + 0x2, 0, 0, "driver_ext"
	},
	{
		SIOCIWFIRSTPRIV + 0x3, 0, 0, "mp_ioctl"
	},
	{
		SIOCIWFIRSTPRIV + 0x4,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "apinfo"
	},
	{
		SIOCIWFIRSTPRIV + 0x5,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "setpid"
	},
	{
		SIOCIWFIRSTPRIV + 0x6,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wps_start"
	},
/* for PLATFORM_MT53XX */
	{
		SIOCIWFIRSTPRIV + 0x7,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "get_sensitivity"
	},
	{
		SIOCIWFIRSTPRIV + 0x8,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wps_prob_req_ie"
	},
	{
		SIOCIWFIRSTPRIV + 0x9,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "wps_assoc_req_ie"
	},

/* for RTK_DMP_PLATFORM */
	{
		SIOCIWFIRSTPRIV + 0xA,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 1, 0, "channel_plan"
	},

	{
		SIOCIWFIRSTPRIV + 0xB,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, 0, "dbg"
	},
	{
		SIOCIWFIRSTPRIV + 0xC,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 3, 0, "rfw"
	},
	{
		SIOCIWFIRSTPRIV + 0xD,
		IW_PRIV_TYPE_INT | IW_PRIV_SIZE_FIXED | 2, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | IFNAMSIZ, "rfr"
	},
	{
		SIOCIWFIRSTPRIV + 0x10,
		IW_PRIV_TYPE_CHAR | 1024, 0, "p2p_set"
	},
	{
		SIOCIWFIRSTPRIV + 0x11,
		IW_PRIV_TYPE_CHAR | 1024, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK , "p2p_get"
	},
	{
		SIOCIWFIRSTPRIV + 0x12, 0, 0, "NULL"
	},
	{
		SIOCIWFIRSTPRIV + 0x13,
		IW_PRIV_TYPE_CHAR | 64, IW_PRIV_TYPE_CHAR | 64 , "p2p_get2"
	},
	{
		SIOCIWFIRSTPRIV + 0x14,
		IW_PRIV_TYPE_CHAR  | 64, 0, "tdls"
	},
	{
		SIOCIWFIRSTPRIV + 0x15,
		IW_PRIV_TYPE_CHAR | P2P_PRIVATE_IOCTL_SET_LEN, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_FIXED | P2P_PRIVATE_IOCTL_SET_LEN , "tdls_get"
	},
	{
		SIOCIWFIRSTPRIV + 0x16,
		IW_PRIV_TYPE_CHAR | 64, 0, "pm_set"
	},

	{SIOCIWFIRSTPRIV + 0x18, IW_PRIV_TYPE_CHAR | IFNAMSIZ , 0 , "rereg_nd_name"},

	{SIOCIWFIRSTPRIV + 0x1A, IW_PRIV_TYPE_CHAR | 1024, 0, "efuse_set"},
	{SIOCIWFIRSTPRIV + 0x1B, IW_PRIV_TYPE_CHAR | 128, IW_PRIV_TYPE_CHAR | IW_PRIV_SIZE_MASK, "efuse_get"},
	{
		SIOCIWFIRSTPRIV + 0x1D,
		IW_PRIV_TYPE_CHAR | 40, IW_PRIV_TYPE_CHAR | 0x7FF, "test"
	},
};

static iw_handler rtw_private_handler[] =
{
	rtw_wx_write32,					/* 0x00 */
	rtw_wx_read32,					/* 0x01 */
	rtw_drvext_hdl,					/* 0x02 */
	rtw_mp_ioctl_hdl,				/* 0x03 */

/*  for MM DTV platform */
	rtw_get_ap_info,					/* 0x04 */

	rtw_set_pid,						/* 0x05 */
	rtw_wps_start,					/* 0x06 */

/*  for PLATFORM_MT53XX */
	rtw_wx_get_sensitivity,			/* 0x07 */
	rtw_wx_set_mtk_wps_probe_ie,	/* 0x08 */
	rtw_wx_set_mtk_wps_ie,			/* 0x09 */

/*  for RTK_DMP_PLATFORM */
/*  Set Channel depend on the country code */
	rtw_wx_set_channel_plan,		/* 0x0A */

	rtw_dbg_port,					/* 0x0B */
	rtw_wx_write_rf,					/* 0x0C */
	rtw_wx_read_rf,					/* 0x0D */

	rtw_wx_priv_null,				/* 0x0E */
	rtw_wx_priv_null,				/* 0x0F */
	rtw_p2p_set,					/* 0x10 */
	rtw_p2p_get,					/* 0x11 */
	NULL,							/* 0x12 */
	rtw_p2p_get2,					/* 0x13 */

	rtw_tdls,						/* 0x14 */
	rtw_tdls_get,					/* 0x15 */

	rtw_pm_set,						/* 0x16 */
	rtw_wx_priv_null,				/* 0x17 */
	rtw_rereg_nd_name,				/* 0x18 */
	rtw_wx_priv_null,				/* 0x19 */

	rtw_mp_efuse_set,				/* 0x1A */
	rtw_mp_efuse_get,				/* 0x1B */
	NULL,							/*  0x1C is reserved for hostapd */
	rtw_test,						/*  0x1D */
};

#if WIRELESS_EXT >= 17
static struct iw_statistics *rtw_get_wireless_stats(struct net_device *dev)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(dev);
	struct iw_statistics *piwstats=&padapter->iwstats;
	int tmp_level = 0;
	int tmp_qual = 0;
	int tmp_noise = 0;

	if (check_fwstate(&padapter->mlmepriv, _FW_LINKED) != true) {
		piwstats->qual.qual = 0;
		piwstats->qual.level = 0;
		piwstats->qual.noise = 0;
	} else {
		tmp_level = padapter->recvpriv.signal_strength;
		#ifdef CONFIG_BT_COEXIST
		{
			u8 signal = (u8)tmp_level;
			BT_SignalCompensation(padapter, &signal, NULL);
			tmp_level= signal;
		}
		#endif /*  CONFIG_BT_COEXIST */

		tmp_qual = padapter->recvpriv.signal_qual;
		tmp_noise =padapter->recvpriv.noise;

		piwstats->qual.level = tmp_level;
		piwstats->qual.qual = tmp_qual;
		piwstats->qual.noise = tmp_noise;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,14))
	piwstats->qual.updated = IW_QUAL_ALL_UPDATED ;/* IW_QUAL_DBM; */
#else
	piwstats->qual.updated = 0x0f;
#endif

	return &padapter->iwstats;
}
#endif

#ifdef CONFIG_WIRELESS_EXT
struct iw_handler_def rtw_handlers_def =
{
	.standard = rtw_handlers,
	.num_standard = sizeof(rtw_handlers) / sizeof(iw_handler),
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,33)) || defined(CONFIG_WEXT_PRIV)
	.private = rtw_private_handler,
	.private_args = (struct iw_priv_args *)rtw_private_args,
	.num_private = sizeof(rtw_private_handler) / sizeof(iw_handler),
	.num_private_args = sizeof(rtw_private_args) / sizeof(struct iw_priv_args),
#endif
#if WIRELESS_EXT >= 17
	.get_wireless_stats = rtw_get_wireless_stats,
#endif
};
#endif

/*  copy from net/wireless/wext.c start */
/* ---------------------------------------------------------------- */
/*
 * Calculate size of private arguments
 */
static const char iw_priv_type_size[] = {
	0,                              /* IW_PRIV_TYPE_NONE */
	1,                              /* IW_PRIV_TYPE_BYTE */
	1,                              /* IW_PRIV_TYPE_CHAR */
	0,                              /* Not defined */
	sizeof(__u32),                  /* IW_PRIV_TYPE_INT */
	sizeof(struct iw_freq),         /* IW_PRIV_TYPE_FLOAT */
	sizeof(struct sockaddr),        /* IW_PRIV_TYPE_ADDR */
	0,                              /* Not defined */
};

static int get_priv_size(__u16 args)
{
	int num = args & IW_PRIV_SIZE_MASK;
	int type = (args & IW_PRIV_TYPE_MASK) >> 12;

	return num * iw_priv_type_size[type];
}
/*  copy from net/wireless/wext.c end */

static int rtw_ioctl_wext_private(struct net_device *dev, union iwreq_data *wrq_data)
{
	int err = 0;
	u8 *input = NULL;
	u32 input_len = 0;
	const char delim[] = " ";
	u8 *output = NULL;
	u32 output_len = 0;
	u32 count = 0;
	u8 *buffer= NULL;
	u32 buffer_len = 0;
	char *ptr = NULL;
	u8 cmdname[17] = {0}; /*  IFNAMSIZ+1 */
	u32 cmdlen;
	s32 len;
	u8 *extra = NULL;
	u32 extra_size = 0;

	s32 k;
	const iw_handler *priv;		/* Private ioctl */
	const struct iw_priv_args *priv_args;	/* Private ioctl description */
	u32 num_priv;				/* Number of ioctl */
	u32 num_priv_args;			/* Number of descriptions */
	iw_handler handler;
	int temp;
	int subcmd = 0;				/* sub-ioctl index */
	int offset = 0;				/* Space for sub-ioctl index */

	union iwreq_data wdata;


	memcpy(&wdata, wrq_data, sizeof(wdata));

	input_len = wdata.data.length;
	if (input_len == 0)
		return -EFAULT;
	input = rtw_zmalloc(input_len);
	if (NULL == input)
		return -ENOMEM;
	if (copy_from_user(input, wdata.data.pointer, input_len)) {
		err = -EFAULT;
		goto exit;
	}
	ptr = input;
	len = input_len;

	sscanf(ptr, "%16s", cmdname);
	cmdlen = strlen(cmdname);
	DBG_8192C("%s: cmd=%s\n", __func__, cmdname);

	/*  skip command string */
	if (cmdlen > 0)
		cmdlen += 1; /*  skip one space */
	ptr += cmdlen;
	len -= cmdlen;
	DBG_8192C("%s: parameters=%s\n", __func__, ptr);

	priv = rtw_private_handler;
	priv_args = rtw_private_args;
	num_priv = sizeof(rtw_private_handler) / sizeof(iw_handler);
	num_priv_args = sizeof(rtw_private_args) / sizeof(struct iw_priv_args);

	if (num_priv_args == 0) {
		err = -EOPNOTSUPP;
		goto exit;
	}

	/* Search the correct ioctl */
	k = -1;
	while ((++k < num_priv_args) && strcmp(priv_args[k].name, cmdname));

	/* If not found... */
	if (k == num_priv_args) {
		err = -EOPNOTSUPP;
		goto exit;
	}

	/* Watch out for sub-ioctls ! */
	if (priv_args[k].cmd < SIOCDEVPRIVATE)
	{
		int j = -1;

		/* Find the matching *real* ioctl */
		while ((++j < num_priv_args) && ((priv_args[j].name[0] != '\0') ||
			(priv_args[j].set_args != priv_args[k].set_args) ||
			(priv_args[j].get_args != priv_args[k].get_args)));

		/* If not found... */
		if (j == num_priv_args) {
			err = -EINVAL;
			goto exit;
		}

		/* Save sub-ioctl number */
		subcmd = priv_args[k].cmd;
		/* Reserve one int (simplify alignment issues) */
		offset = sizeof(__u32);
		/* Use real ioctl definition from now on */
		k = j;
	}

	buffer = rtw_zmalloc(4096);
	if (NULL == buffer) {
		err = -ENOMEM;
		goto exit;
	}

	/* If we have to set some data */
	if ((priv_args[k].set_args & IW_PRIV_TYPE_MASK) &&
		(priv_args[k].set_args & IW_PRIV_SIZE_MASK))
	{
		u8 *str;

		switch (priv_args[k].set_args & IW_PRIV_TYPE_MASK)
		{
			case IW_PRIV_TYPE_BYTE:
				/* Fetch args */
				count = 0;
				do {
					str = strsep(&ptr, delim);
					if (NULL == str) break;
					sscanf(str, "%i", &temp);
					buffer[count++] = (u8)temp;
				} while (1);
				buffer_len = count;

				/* Number of args to fetch */
				wdata.data.length = count;
				if (wdata.data.length > (priv_args[k].set_args & IW_PRIV_SIZE_MASK))
					wdata.data.length = priv_args[k].set_args & IW_PRIV_SIZE_MASK;

				break;

			case IW_PRIV_TYPE_INT:
				/* Fetch args */
				count = 0;
				do {
					str = strsep(&ptr, delim);
					if (NULL == str) break;
					sscanf(str, "%i", &temp);
					((s32*)buffer)[count++] = (s32)temp;
				} while (1);
				buffer_len = count * sizeof(s32);

				/* Number of args to fetch */
				wdata.data.length = count;
				if (wdata.data.length > (priv_args[k].set_args & IW_PRIV_SIZE_MASK))
					wdata.data.length = priv_args[k].set_args & IW_PRIV_SIZE_MASK;

				break;

			case IW_PRIV_TYPE_CHAR:
				if (len > 0)
				{
					/* Size of the string to fetch */
					wdata.data.length = len;
					if (wdata.data.length > (priv_args[k].set_args & IW_PRIV_SIZE_MASK))
						wdata.data.length = priv_args[k].set_args & IW_PRIV_SIZE_MASK;

					/* Fetch string */
					memcpy(buffer, ptr, wdata.data.length);
				}
				else
				{
					wdata.data.length = 1;
					buffer[0] = '\0';
				}
				buffer_len = wdata.data.length;
				break;

			default:
				DBG_8192C("%s: Not yet implemented...\n", __func__);
				err = -1;
				goto exit;
		}

		if ((priv_args[k].set_args & IW_PRIV_SIZE_FIXED) &&
			(wdata.data.length != (priv_args[k].set_args & IW_PRIV_SIZE_MASK)))
		{
			DBG_8192C("%s: The command %s needs exactly %d argument(s)...\n",
					__func__, cmdname, priv_args[k].set_args & IW_PRIV_SIZE_MASK);
			err = -EINVAL;
			goto exit;
		}
	}   /* if args to set */
	else
	{
		wdata.data.length = 0L;
	}

	/* Those two tests are important. They define how the driver
	* will have to handle the data */
	if ((priv_args[k].set_args & IW_PRIV_SIZE_FIXED) &&
		((get_priv_size(priv_args[k].set_args) + offset) <= IFNAMSIZ))
	{
		/* First case : all SET args fit within wrq */
		if (offset)
			wdata.mode = subcmd;
		memcpy(wdata.name + offset, buffer, IFNAMSIZ - offset);
	}
	else
	{
		if ((priv_args[k].set_args == 0) &&
			(priv_args[k].get_args & IW_PRIV_SIZE_FIXED) &&
			(get_priv_size(priv_args[k].get_args) <= IFNAMSIZ))
		{
			/* Second case : no SET args, GET args fit within wrq */
			if (offset)
				wdata.mode = subcmd;
		}
		else
		{
			/* Third case : args won't fit in wrq, or variable number of args */
			if (copy_to_user(wdata.data.pointer, buffer, buffer_len)) {
				err = -EFAULT;
				goto exit;
			}
			wdata.data.flags = subcmd;
		}
	}

	rtw_mfree(input, input_len);
	input = NULL;

	extra_size = 0;
	if (IW_IS_SET(priv_args[k].cmd))
	{
		/* Size of set arguments */
		extra_size = get_priv_size(priv_args[k].set_args);

		/* Does it fits in iwr ? */
		if ((priv_args[k].set_args & IW_PRIV_SIZE_FIXED) &&
			((extra_size + offset) <= IFNAMSIZ))
			extra_size = 0;
	} else {
		/* Size of get arguments */
		extra_size = get_priv_size(priv_args[k].get_args);

		/* Does it fits in iwr ? */
		if ((priv_args[k].get_args & IW_PRIV_SIZE_FIXED) &&
			(extra_size <= IFNAMSIZ))
			extra_size = 0;
	}

	if (extra_size == 0) {
		extra = (u8*)&wdata;
		rtw_mfree(buffer, 4096);
		buffer = NULL;
	} else
		extra = buffer;

	handler = priv[priv_args[k].cmd - SIOCIWFIRSTPRIV];
	err = handler(dev, NULL, &wdata, extra);

	/* If we have to get some data */
	if ((priv_args[k].get_args & IW_PRIV_TYPE_MASK) &&
		(priv_args[k].get_args & IW_PRIV_SIZE_MASK))
	{
		int j;
		int n = 0;	/* number of args */
		u8 str[20] = {0};

		/* Check where is the returned data */
		if ((priv_args[k].get_args & IW_PRIV_SIZE_FIXED) &&
			(get_priv_size(priv_args[k].get_args) <= IFNAMSIZ))
			n = priv_args[k].get_args & IW_PRIV_SIZE_MASK;
		else
			n = wdata.data.length;

		output = rtw_zmalloc(4096);
		if (NULL == output) {
			err =  -ENOMEM;
			goto exit;
		}

		switch (priv_args[k].get_args & IW_PRIV_TYPE_MASK)
		{
			case IW_PRIV_TYPE_BYTE:
				/* Display args */
				for (j = 0; j < n; j++)
				{
					sprintf(str, "%d  ", extra[j]);
					len = strlen(str);
					output_len = strlen(output);
					if ((output_len + len + 1) > 4096) {
						err = -E2BIG;
						goto exit;
					}
					memcpy(output+output_len, str, len);
				}
				break;

			case IW_PRIV_TYPE_INT:
				/* Display args */
				for (j = 0; j < n; j++)
				{
					sprintf(str, "%d  ", ((__s32*)extra)[j]);
					len = strlen(str);
					output_len = strlen(output);
					if ((output_len + len + 1) > 4096) {
						err = -E2BIG;
						goto exit;
					}
					memcpy(output+output_len, str, len);
				}
				break;

			case IW_PRIV_TYPE_CHAR:
				/* Display args */
				memcpy(output, extra, n);
				break;

			default:
				DBG_8192C("%s: Not yet implemented...\n", __func__);
				err = -1;
				goto exit;
		}

		output_len = strlen(output) + 1;
		wrq_data->data.length = output_len;
		if (copy_to_user(wrq_data->data.pointer, output, output_len)) {
			err = -EFAULT;
			goto exit;
		}
	}   /* if args to set */
	else
	{
		wrq_data->data.length = 0;
	}

exit:
	if (input)
		rtw_mfree(input, input_len);
	if (buffer)
		rtw_mfree(buffer, 4096);
	if (output)
		rtw_mfree(output, 4096);

	return err;
}

#include <rtw_android.h>
int rtw_ioctl(struct net_device *dev, struct ifreq *rq, int cmd)
{
	struct iwreq *wrq = (struct iwreq *)rq;
	int ret=0;

	switch (cmd)
	{
		case RTL_IOCTL_WPA_SUPPLICANT:
			ret = wpa_supplicant_ioctl(dev, &wrq->u.data);
			break;
#ifdef CONFIG_AP_MODE
		case RTL_IOCTL_HOSTAPD:
			ret = rtw_hostapd_ioctl(dev, &wrq->u.data);
			break;
#ifdef CONFIG_NO_WIRELESS_HANDLERS
		case SIOCSIWMODE:
			ret = rtw_wx_set_mode(dev, NULL, &wrq->u, NULL);
			break;
#endif
#endif /*  CONFIG_AP_MODE */
		case SIOCDEVPRIVATE:
			ret = rtw_ioctl_wext_private(dev, &wrq->u);
			break;
		case (SIOCDEVPRIVATE+1):
			ret = rtw_android_priv_cmd(dev, rq, cmd);
			break;
		default:
			ret = -EOPNOTSUPP;
			break;
	}

	return ret;
}
