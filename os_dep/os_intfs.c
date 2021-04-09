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
#define _OS_INTFS_C_

#include <drv_conf.h>

#include <osdep_service.h>
#include <osdep_intf.h>
#include <drv_types.h>
#include <xmit_osdep.h>
#include <recv_osdep.h>
#include <hal_intf.h>
#include <rtw_ioctl.h>
#include <rtw_version.h>
#include <rtw_br_ext.h>
#include <usb_hal.h>
#include <usb_osintf.h>

#ifdef CONFIG_BR_EXT
#include <rtw_br_ext.h>
#endif /* CONFIG_BR_EXT */

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Realtek Wireless Lan Driver");
MODULE_AUTHOR("Realtek Semiconductor Corp.");
MODULE_VERSION(DRIVERVERSION);

/* module param defaults */
static int rtw_rfintfs = HWPI;
static int rtw_lbkmode = 0;/* RTL8712_AIR_TRX; */


static int rtw_network_mode = Ndis802_11IBSS;/* Ndis802_11Infrastructure;infra, ad-hoc, auto */
/* NDIS_802_11_SSID	ssid; */
static int rtw_channel = 1;/* ad-hoc support requirement */
static int rtw_wireless_mode = WIRELESS_11BG_24N;
static int rtw_vrtl_carrier_sense = AUTO_VCS;
static int rtw_vcs_type = RTS_CTS;/*  */
static int rtw_rts_thresh = 2347;/*  */
static int rtw_frag_thresh = 2346;/*  */
static int rtw_preamble = PREAMBLE_LONG;/* long, short, auto */
static int rtw_scan_mode = 1;/* active, passive */
static int rtw_adhoc_tx_pwr = 1;
static int rtw_soft_ap = 0;
/* int smart_ps = 1; */
#ifdef CONFIG_POWER_SAVING
static int rtw_power_mgnt = 1;
static int rtw_ips_mode = IPS_NORMAL;
#else
static int rtw_power_mgnt = PS_MODE_ACTIVE;
static int rtw_ips_mode = IPS_NONE;
#endif

static int rtw_smart_ps = 2;

module_param(rtw_ips_mode, int, 0644);
MODULE_PARM_DESC(rtw_ips_mode,"The default IPS mode");

static int rtw_debug = 1;
static int rtw_radio_enable = 1;
static int rtw_long_retry_lmt = 7;
static int rtw_short_retry_lmt = 7;
static int rtw_busy_thresh = 40;
static int rtw_ack_policy = NORMAL_ACK;

static int rtw_mp_mode = 0;

static int rtw_software_encrypt = 0;
static int rtw_software_decrypt = 0;

static int rtw_acm_method = 0;/*  0:By SW 1:By HW. */

static int rtw_wmm_enable = 1;/*  default is set to enable the wmm. */
static int rtw_uapsd_enable = 0;
static int rtw_uapsd_max_sp = NO_LIMIT;
static int rtw_uapsd_acbk_en = 0;
static int rtw_uapsd_acbe_en = 0;
static int rtw_uapsd_acvi_en = 0;
static int rtw_uapsd_acvo_en = 0;

int rtw_ht_enable = 1;
int rtw_cbw40_enable = 3; /*  0 :diable, bit(0): enable 2.4g, bit(1): enable 5g */
int rtw_ampdu_enable = 1;/* for enable tx_ampdu */
static int rtw_rx_stbc = 1;/*  0: disable, bit(0):enable 2.4g, bit(1):enable 5g, default is set to enable 2.4GHZ for IOT issue with bufflao's AP at 5GHZ */
static int rtw_ampdu_amsdu = 0;/*  0: disabled, 1:enabled, 2:auto */

static int rtw_lowrate_two_xmit = 1;/* Use 2 path Tx to transmit MCS0~7 and legacy mode */

/* int rf_config = RF_1T2R;   1T2R */
static int rtw_rf_config = RF_819X_MAX_TYPE;  /* auto */
static int rtw_low_power = 0;
static int rtw_wifi_spec = 0;
static int rtw_channel_plan = RT_CHANNEL_DOMAIN_MAX;

#ifdef CONFIG_BT_COEXIST
static int rtw_btcoex_enable = 1;
static int rtw_bt_iso = 2;/*  0:Low, 1:High, 2:From Efuse */
static int rtw_bt_sco = 3;/*  0:Idle, 1:None-SCO, 2:SCO, 3:From Counter, 4.Busy, 5.OtherBusy */
static int rtw_bt_ampdu =1 ;/*  0:Disable BT control A-MPDU, 1:Enable BT control A-MPDU. */
#endif

static int rtw_AcceptAddbaReq = true;/*  0:Reject AP's Add BA req, 1:Accept AP's Add BA req. */

static int rtw_antdiv_cfg = 2; /*  0:OFF , 1:ON, 2:decide by Efuse config */
static int rtw_antdiv_type = 0 ; /* 0:decide by efuse  1: for 88EE, 1Tx and 1RxCG are diversity.(2 Ant with SPDT), 2:  for 88EE, 1Tx and 2Rx are diversity.( 2 Ant, Tx and RxCG are both on aux port, RxCS is on main port ), 3: for 88EE, 1Tx and 1RxCG are fixed.(1Ant, Tx and RxCG are both on aux port) */


#ifdef CONFIG_USB_AUTOSUSPEND
static int rtw_enusbss = 1;/* 0:disable,1:enable */
#else
static int rtw_enusbss = 0;/* 0:disable,1:enable */
#endif

static int rtw_hwpdn_mode=2;/* 0:disable,1:enable,2: by EFUSE config */

static int rtw_hwpwrp_detect = 0; /* HW power  ping detect 0:disable , 1:enable */

static int rtw_hw_wps_pbc = 1;

int rtw_mc2u_disable = 0;

#ifdef CONFIG_80211D
static int rtw_80211d = 0;
#endif

static int rtw_regulatory_id =2;
module_param(rtw_regulatory_id, int, 0644);

#ifdef CONFIG_QOS_OPTIMIZATION
static int rtw_qos_opt_enable=1;/* 0: disable,1:enable */
#else
static int rtw_qos_opt_enable=0;/* 0: disable,1:enable */
#endif
module_param(rtw_qos_opt_enable,int,0644);

static char* ifname = "wlan%d";
module_param(ifname, charp, 0644);
MODULE_PARM_DESC(ifname, "The default name to allocate for first interface");

char* rtw_initmac = NULL;  /*  temp mac address if users want to use instead of the mac address in Efuse */

module_param(rtw_initmac, charp, 0644);
module_param(rtw_channel_plan, int, 0644);
module_param(rtw_rfintfs, int, 0644);
module_param(rtw_lbkmode, int, 0644);
module_param(rtw_network_mode, int, 0644);
module_param(rtw_channel, int, 0644);
module_param(rtw_mp_mode, int, 0644);
module_param(rtw_wmm_enable, int, 0644);
module_param(rtw_vrtl_carrier_sense, int, 0644);
module_param(rtw_vcs_type, int, 0644);
module_param(rtw_busy_thresh, int, 0644);
module_param(rtw_ht_enable, int, 0644);
module_param(rtw_cbw40_enable, int, 0644);
module_param(rtw_ampdu_enable, int, 0644);
module_param(rtw_rx_stbc, int, 0644);
module_param(rtw_ampdu_amsdu, int, 0644);

module_param(rtw_lowrate_two_xmit, int, 0644);

module_param(rtw_rf_config, int, 0644);
module_param(rtw_power_mgnt, int, 0644);
module_param(rtw_smart_ps, int, 0644);
module_param(rtw_low_power, int, 0644);
module_param(rtw_wifi_spec, int, 0644);

module_param(rtw_antdiv_cfg, int, 0644);
module_param(rtw_antdiv_type, int, 0644);

module_param(rtw_enusbss, int, 0644);
module_param(rtw_hwpdn_mode, int, 0644);
module_param(rtw_hwpwrp_detect, int, 0644);

module_param(rtw_hw_wps_pbc, int, 0644);

static uint rtw_max_roaming_times=2;
module_param(rtw_max_roaming_times, uint, 0644);
MODULE_PARM_DESC(rtw_max_roaming_times,"The max roaming times to try");

static int rtw_fw_iol=1;/*  0:Disable, 1:enable, 2:by usb speed */
module_param(rtw_fw_iol, int, 0644);
MODULE_PARM_DESC(rtw_fw_iol,"FW IOL");

#ifdef CONFIG_FILE_FWIMG
static char *rtw_fw_file_path= "";
module_param(rtw_fw_file_path, charp, 0644);
MODULE_PARM_DESC(rtw_fw_file_path, "The path of fw image");
#endif /* CONFIG_FILE_FWIMG */

module_param(rtw_mc2u_disable, int, 0644);

#ifdef CONFIG_80211D
module_param(rtw_80211d, int, 0644);
MODULE_PARM_DESC(rtw_80211d, "Enable 802.11d mechanism");
#endif

#ifdef CONFIG_BT_COEXIST
module_param(rtw_btcoex_enable, int, 0644);
MODULE_PARM_DESC(rtw_btcoex_enable, "Enable BT co-existence mechanism");
#endif

static uint rtw_notch_filter = RTW_NOTCH_FILTER;
module_param(rtw_notch_filter, uint, 0644);
MODULE_PARM_DESC(rtw_notch_filter, "0:Disable, 1:Enable, 2:Enable only for P2P");
module_param_named(debug, rtw_debug, int, 0444);
MODULE_PARM_DESC(debug, "Set debug level (1-9) (default 1)");

static uint loadparam(struct adapter *padapter, struct  net_device * pnetdev);
int _netdev_open(struct net_device *pnetdev);
int netdev_open (struct net_device *pnetdev);
static int netdev_close (struct net_device *pnetdev);

#ifdef CONFIG_PROC_DEBUG
#define RTL8192C_PROC_NAME "rtl819xC"
#define RTL8192D_PROC_NAME "rtl819xD"
static char rtw_proc_name[IFNAMSIZ];
static struct proc_dir_entry *rtw_proc = NULL;
static int	rtw_proc_cnt = 0;

#define RTW_PROC_NAME DRV_NAME

void rtw_proc_init_one(struct net_device *dev)
{
}

void rtw_proc_remove_one(struct net_device *dev)
{
}
#endif

static uint loadparam( struct adapter *padapter,  struct  net_device *	pnetdev)
{

	uint status = _SUCCESS;
	struct registry_priv  *registry_par = &padapter->registrypriv;

	GlobalDebugLevel = rtw_debug;
	registry_par->rfintfs = (u8)rtw_rfintfs;
	registry_par->lbkmode = (u8)rtw_lbkmode;
	/* registry_par->hci = (u8)hci; */
	registry_par->network_mode  = (u8)rtw_network_mode;

	memcpy(registry_par->ssid.Ssid, "ANY", 3);
	registry_par->ssid.SsidLength = 3;

	registry_par->channel = (u8)rtw_channel;
	registry_par->wireless_mode = (u8)rtw_wireless_mode;
	registry_par->vrtl_carrier_sense = (u8)rtw_vrtl_carrier_sense ;
	registry_par->vcs_type = (u8)rtw_vcs_type;
	registry_par->rts_thresh=(u16)rtw_rts_thresh;
	registry_par->frag_thresh=(u16)rtw_frag_thresh;
	registry_par->preamble = (u8)rtw_preamble;
	registry_par->scan_mode = (u8)rtw_scan_mode;
	registry_par->adhoc_tx_pwr = (u8)rtw_adhoc_tx_pwr;
	registry_par->soft_ap=  (u8)rtw_soft_ap;
	registry_par->smart_ps =  (u8)rtw_smart_ps;
	registry_par->power_mgnt = (u8)rtw_power_mgnt;
	registry_par->ips_mode = (u8)rtw_ips_mode;
	registry_par->radio_enable = (u8)rtw_radio_enable;
	registry_par->radio_enable = (u8)rtw_radio_enable;
	registry_par->long_retry_lmt = (u8)rtw_long_retry_lmt;
	registry_par->short_retry_lmt = (u8)rtw_short_retry_lmt;
	registry_par->busy_thresh = (u16)rtw_busy_thresh;
	/* registry_par->qos_enable = (u8)rtw_qos_enable; */
	registry_par->ack_policy = (u8)rtw_ack_policy;
	registry_par->mp_mode = (u8)rtw_mp_mode;
	registry_par->software_encrypt = (u8)rtw_software_encrypt;
	registry_par->software_decrypt = (u8)rtw_software_decrypt;

	registry_par->acm_method = (u8)rtw_acm_method;

	 /* UAPSD */
	registry_par->wmm_enable = (u8)rtw_wmm_enable;
	registry_par->uapsd_enable = (u8)rtw_uapsd_enable;
	registry_par->uapsd_max_sp = (u8)rtw_uapsd_max_sp;
	registry_par->uapsd_acbk_en = (u8)rtw_uapsd_acbk_en;
	registry_par->uapsd_acbe_en = (u8)rtw_uapsd_acbe_en;
	registry_par->uapsd_acvi_en = (u8)rtw_uapsd_acvi_en;
	registry_par->uapsd_acvo_en = (u8)rtw_uapsd_acvo_en;

	registry_par->ht_enable = (u8)rtw_ht_enable;
	registry_par->cbw40_enable = (u8)rtw_cbw40_enable;
	registry_par->ampdu_enable = (u8)rtw_ampdu_enable;
	registry_par->rx_stbc = (u8)rtw_rx_stbc;
	registry_par->ampdu_amsdu = (u8)rtw_ampdu_amsdu;
	registry_par->lowrate_two_xmit = (u8)rtw_lowrate_two_xmit;
	registry_par->rf_config = (u8)rtw_rf_config;
	registry_par->low_power = (u8)rtw_low_power;

	registry_par->wifi_spec = (u8)rtw_wifi_spec;

	registry_par->channel_plan = (u8)rtw_channel_plan;

#ifdef CONFIG_BT_COEXIST
	registry_par->btcoex = (u8)rtw_btcoex_enable;
	registry_par->bt_iso = (u8)rtw_bt_iso;
	registry_par->bt_sco = (u8)rtw_bt_sco;
	registry_par->bt_ampdu = (u8)rtw_bt_ampdu;
#endif

	registry_par->bAcceptAddbaReq = (u8)rtw_AcceptAddbaReq;

	registry_par->antdiv_cfg = (u8)rtw_antdiv_cfg;
	registry_par->antdiv_type = (u8)rtw_antdiv_type;

#ifdef CONFIG_AUTOSUSPEND
	registry_par->usbss_enable = (u8)rtw_enusbss;/* 0:disable,1:enable */
#endif
	registry_par->hwpdn_mode = (u8)rtw_hwpdn_mode;/* 0:disable,1:enable,2:by EFUSE config */
	registry_par->hwpwrp_detect = (u8)rtw_hwpwrp_detect;/* 0:disable,1:enable */

	registry_par->qos_opt_enable = (u8)rtw_qos_opt_enable;
	registry_par->hw_wps_pbc = (u8)rtw_hw_wps_pbc;

	registry_par->max_roaming_times = (u8)rtw_max_roaming_times;
	registry_par->fw_iol = rtw_fw_iol;

#ifdef CONFIG_80211D
	registry_par->enable80211d = (u8)rtw_80211d;
#endif

	snprintf(registry_par->ifname, 16, "%s", ifname);

	registry_par->notch_filter = (u8)rtw_notch_filter;

	registry_par->regulatory_tid = (u8)rtw_regulatory_id;

	return status;
}

static int rtw_net_set_mac_address(struct net_device *pnetdev, void *p)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(pnetdev);
	struct sockaddr *addr = p;

	if (padapter->bup == false)
	{
		/* DBG_88E("r8711_net_set_mac_address(), MAC=%x:%x:%x:%x:%x:%x\n", addr->sa_data[0], addr->sa_data[1], addr->sa_data[2], addr->sa_data[3], */
		/* addr->sa_data[4], addr->sa_data[5]); */
		memcpy(padapter->eeprompriv.mac_addr, addr->sa_data, ETH_ALEN);
		/* memcpy(pnetdev->dev_addr, addr->sa_data, ETH_ALEN); */
		/* padapter->bset_hwaddr = true; */
	}

	return 0;
}

static struct net_device_stats *rtw_net_get_stats(struct net_device *pnetdev)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(pnetdev);
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct recv_priv *precvpriv = &padapter->recvpriv;

	padapter->stats.tx_packets = pxmitpriv->tx_pkts;/* pxmitpriv->tx_pkts++; */
	padapter->stats.rx_packets = precvpriv->rx_pkts;/* precvpriv->rx_pkts++; */
	padapter->stats.tx_dropped = pxmitpriv->tx_drop;
	padapter->stats.rx_dropped = precvpriv->rx_drop;
	padapter->stats.tx_bytes = pxmitpriv->tx_bytes;
	padapter->stats.rx_bytes = precvpriv->rx_bytes;

	return &padapter->stats;
}

#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
/*
 * AC to queue mapping
 *
 * AC_VO -> queue 0
 * AC_VI -> queue 1
 * AC_BE -> queue 2
 * AC_BK -> queue 3
 */
static const u16 rtw_1d_to_queue[8] = { 2, 3, 3, 2, 1, 1, 0, 0 };

/* Given a data frame determine the 802.1p/1d tag to use. */
static unsigned int rtw_classify8021d(struct sk_buff *skb)
{
	unsigned int dscp;

	/* skb->priority values from 256->263 are magic values to
	 * directly indicate a specific 802.1d priority.  This is used
	 * to allow 802.1d priority to be passed directly in from VLAN
	 * tags, etc.
	 */
	if (skb->priority >= 256 && skb->priority <= 263)
		return skb->priority - 256;

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		dscp = ip_hdr(skb)->tos & 0xfc;
		break;
	default:
		return 0;
	}

	return dscp >> 5;
}

static u16 rtw_select_queue(struct net_device *dev, struct sk_buff *skb,
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 2, 0))
			    struct net_device *sb_dev
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 18, 0))
			    struct net_device *sb_dev,
			    select_queue_fallback_t fallback
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
			    void *unused,
			    select_queue_fallback_t fallback
#elif LINUX_VERSION_CODE >= KERNEL_VERSION(3, 13, 0)
			    void *accel_priv
#endif
)
{
	struct adapter	*padapter = rtw_netdev_priv(dev);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	skb->priority = rtw_classify8021d(skb);

	if (pmlmepriv->acm_mask != 0)
	{
		skb->priority = qos_acm(pmlmepriv->acm_mask, skb->priority);
	}

	return rtw_1d_to_queue[skb->priority];
}

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3, 16, 0))
u16 rtw_recv_select_queue(struct sk_buff *skb)
#else
u16 rtw_recv_select_queue(struct sk_buff *skb,
			  void *accel_priv,
			  select_queue_fallback_t fallback)
#endif
{
	struct iphdr *piphdr;
	unsigned int dscp;
	__be16	eth_type;
	u32 priority;
	u8 *pdata = skb->data;

	memcpy(&eth_type, pdata+(ETH_ALEN<<1), 2);

	switch (be16_to_cpu(eth_type)) {
		case ETH_P_IP:
			piphdr = (struct iphdr *)(pdata+ETH_HLEN);

			dscp = piphdr->tos & 0xfc;

			priority = dscp >> 5;

			break;
		default:
			priority = 0;
	}

	return rtw_1d_to_queue[priority];
}

#endif

#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,29))
static const struct net_device_ops rtw_netdev_ops = {
	.ndo_open = netdev_open,
	.ndo_stop = netdev_close,
	.ndo_start_xmit = rtw_xmit_entry,
#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
	.ndo_select_queue	= rtw_select_queue,
#endif
	.ndo_set_mac_address = rtw_net_set_mac_address,
	.ndo_get_stats = rtw_net_get_stats,
	.ndo_do_ioctl = rtw_ioctl,
};
#endif

int rtw_init_netdev_name(struct net_device *pnetdev, const char *ifname)
{
	struct adapter *padapter = rtw_netdev_priv(pnetdev);

#ifdef CONFIG_EASY_REPLACEMENT
	struct net_device	*TargetNetdev = NULL;
	struct adapter			*TargetAdapter = NULL;
	struct net		*devnet = NULL;

	if (padapter->bDongle == 1)
	{
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
		TargetNetdev = dev_get_by_name("wlan0");
#else
	#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
		devnet = pnetdev->nd_net;
	#else
		devnet = dev_net(pnetdev);
	#endif
		TargetNetdev = dev_get_by_name(devnet, "wlan0");
#endif
		if (TargetNetdev) {
			DBG_88E("Force onboard module driver disappear !!!\n");
			TargetAdapter = rtw_netdev_priv(TargetNetdev);
			TargetAdapter->DriverState = DRIVER_DISAPPEAR;

			padapter->pid[0] = TargetAdapter->pid[0];
			padapter->pid[1] = TargetAdapter->pid[1];
			padapter->pid[2] = TargetAdapter->pid[2];

			dev_put(TargetNetdev);
			unregister_netdev(TargetNetdev);

			if (TargetAdapter->chip_type == padapter->chip_type)
				rtw_proc_remove_one(TargetNetdev);

			padapter->DriverState = DRIVER_REPLACE_DONGLE;
		}
	}
#endif

	if (dev_alloc_name(pnetdev, ifname) < 0)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("dev_alloc_name, fail!\n"));
	}

	netif_carrier_off(pnetdev);
	/* rtw_netif_stop_queue(pnetdev); */

	return 0;
}

struct net_device *rtw_init_netdev(struct adapter *old_padapter)
{
	struct adapter *padapter;
	struct net_device *pnetdev;

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+init_net_dev\n"));

	if (old_padapter != NULL)
		pnetdev = rtw_alloc_etherdev_with_old_priv(sizeof(struct adapter), (void *)old_padapter);
	else
		pnetdev = rtw_alloc_etherdev(sizeof(struct adapter));

	if (!pnetdev)
		return NULL;

	padapter = rtw_netdev_priv(pnetdev);
	padapter->pnetdev = pnetdev;

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)
	SET_MODULE_OWNER(pnetdev);
#endif

	/* pnetdev->init = NULL; */

#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,29))
	DBG_88E("register rtw_netdev_ops to netdev_ops\n");
	pnetdev->netdev_ops = &rtw_netdev_ops;
#else
	pnetdev->open = netdev_open;
	pnetdev->stop = netdev_close;
	pnetdev->hard_start_xmit = rtw_xmit_entry;
	pnetdev->set_mac_address = rtw_net_set_mac_address;
	pnetdev->get_stats = rtw_net_get_stats;
	pnetdev->do_ioctl = rtw_ioctl;
#endif

	pnetdev->watchdog_timeo = HZ*3; /* 3 second timeout */
#ifdef CONFIG_WIRELESS_EXT
	pnetdev->wireless_handlers = (struct iw_handler_def *)&rtw_handlers_def;
#endif

	/* step 2. */
	loadparam(padapter, pnetdev);

	return pnetdev;
}

u32 rtw_start_drv_threads(struct adapter *padapter)
{
	u32 _status = _SUCCESS;

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+rtw_start_drv_threads\n"));

	padapter->cmdThread = kthread_run(rtw_cmd_thread, padapter, "RTW_CMD_THREAD");
        if (IS_ERR(padapter->cmdThread))
		_status = _FAIL;
	else
		_rtw_down_sema(&padapter->cmdpriv.terminate_cmdthread_sema); /* wait for cmd_thread to run */

	rtw_hal_start_thread(padapter);
	return _status;

}

void rtw_unregister_netdevs(struct dvobj_priv *dvobj)
{
	int i;
	struct adapter *padapter = NULL;
	struct net_device *pnetdev = NULL;

	if (!dvobj || dvobj->iface_nums == 0)
		return;

	for (i=0;i<dvobj->iface_nums;i++) {
		padapter = dvobj->padapters[i];

		if (padapter == NULL)
			continue;

		pnetdev = padapter->pnetdev;
		if (!pnetdev)
			continue;

		if (padapter->DriverState != DRIVER_DISAPPEAR) {
			struct wireless_dev *wdev = padapter->rtw_wdev;

			wdev->current_bss = NULL;
			pnetdev->reg_state = NETREG_REGISTERED;
			unregister_netdev(pnetdev); /* will call netdev_close() */
			rtw_proc_remove_one(pnetdev);
		}

		if (padapter->rtw_wdev)
			rtw_wdev_unregister(padapter->rtw_wdev);
	}

}


void rtw_stop_drv_threads (struct adapter *padapter)
{
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+rtw_stop_drv_threads\n"));

	rtw_stop_cmd_thread(padapter);

	rtw_hal_stop_thread(padapter);
}

static u8 rtw_init_default_value(struct adapter *padapter)
{
	u8 ret  = _SUCCESS;
	struct registry_priv* pregistrypriv = &padapter->registrypriv;
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	struct mlme_priv *pmlmepriv= &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;

	/* xmit_priv */
	pxmitpriv->vcs_setting = pregistrypriv->vrtl_carrier_sense;
	pxmitpriv->vcs = pregistrypriv->vcs_type;
	pxmitpriv->vcs_type = pregistrypriv->vcs_type;
	/* pxmitpriv->rts_thresh = pregistrypriv->rts_thresh; */
	pxmitpriv->frag_len = pregistrypriv->frag_thresh;



	/* recv_priv */


	/* mlme_priv */
	pmlmepriv->scan_interval = SCAN_INTERVAL;/*  30*2 sec = 60sec */
	pmlmepriv->scan_mode = SCAN_ACTIVE;

	/* qos_priv */
	/* pmlmepriv->qospriv.qos_option = pregistrypriv->wmm_enable; */

	/* ht_priv */
	pmlmepriv->htpriv.ampdu_enable = false;/* set to disabled */

	/* security_priv */
	/* rtw_get_encrypt_decrypt_from_registrypriv(padapter); */
	psecuritypriv->binstallGrpkey = _FAIL;
	psecuritypriv->sw_encrypt=pregistrypriv->software_encrypt;
	psecuritypriv->sw_decrypt=pregistrypriv->software_decrypt;

	psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_Open; /* open system */
	psecuritypriv->dot11PrivacyAlgrthm = _NO_PRIVACY_;

	psecuritypriv->dot11PrivacyKeyIndex = 0;

	psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
	psecuritypriv->dot118021XGrpKeyid = 1;

	psecuritypriv->ndisauthtype = Ndis802_11AuthModeOpen;
	psecuritypriv->ndisencryptstatus = Ndis802_11WEPDisabled;


	/* pwrctrl_priv */


	/* registry_priv */
	rtw_init_registrypriv_dev_network(padapter);
	rtw_update_registrypriv_dev_network(padapter);


	/* hal_priv */
	rtw_hal_def_value_init(padapter);

	/* misc. */
	padapter->bReadPortCancel = false;
	padapter->bWritePortCancel = false;
	padapter->bRxRSSIDisplay = 0;
	padapter->bNotifyChannelChange = 0;
#ifdef CONFIG_P2P
	padapter->bShowGetP2PState = 1;
#endif

	return ret;
}

u8 rtw_reset_drv_sw(struct adapter *padapter)
{
	u8	ret8=_SUCCESS;
	struct mlme_priv *pmlmepriv= &padapter->mlmepriv;
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);

	/* hal_priv */
	rtw_hal_def_value_init(padapter);
	padapter->bReadPortCancel = false;
	padapter->bWritePortCancel = false;
	padapter->bRxRSSIDisplay = 0;
	pmlmepriv->scan_interval = SCAN_INTERVAL;/*  30*2 sec = 60sec */

	padapter->xmitpriv.tx_pkts = 0;
	padapter->recvpriv.rx_pkts = 0;

	pmlmepriv->LinkDetectInfo.bBusyTraffic = false;

	_clr_fwstate_(pmlmepriv, _FW_UNDER_SURVEY |_FW_UNDER_LINKING);

#ifdef CONFIG_AUTOSUSPEND
	#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22) && LINUX_VERSION_CODE<=KERNEL_VERSION(2,6,34))
		adapter_to_dvobj(padapter)->pusbdev->autosuspend_disabled = 1;/* autosuspend disabled by the user */
	#endif
#endif

	rtw_hal_sreset_reset_value(padapter);
	pwrctrlpriv->pwr_state_check_cnts = 0;

	/* mlmeextpriv */
	padapter->mlmeextpriv.sitesurvey_res.state= SCAN_DISABLE;

	rtw_set_signal_stat_timer(&padapter->recvpriv);

	return ret8;
}

u8 rtw_init_drv_sw(struct adapter *padapter)
{

	u8	ret8=_SUCCESS;

;

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+rtw_init_drv_sw\n"));

	if ((rtw_init_cmd_priv(&padapter->cmdpriv)) == _FAIL)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("\n Can't init cmd_priv\n"));
		ret8=_FAIL;
		goto exit;
	}

	padapter->cmdpriv.padapter=padapter;

	if ((rtw_init_evt_priv(&padapter->evtpriv)) == _FAIL)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("\n Can't init evt_priv\n"));
		ret8=_FAIL;
		goto exit;
	}


	if (rtw_init_mlme_priv(padapter) == _FAIL)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("\n Can't init mlme_priv\n"));
		ret8=_FAIL;
		goto exit;
	}

#ifdef CONFIG_P2P
	rtw_init_wifidirect_timers(padapter);
	init_wifidirect_info(padapter, P2P_ROLE_DISABLE);
	reset_global_wifidirect_info(padapter);
	rtw_init_cfg80211_wifidirect_info(padapter);
#ifdef CONFIG_P2P
	if (rtw_init_wifi_display_info(padapter) == _FAIL)
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("\n Can't init init_wifi_display_info\n"));
#endif
#endif /* CONFIG_P2P */

	if (init_mlme_ext_priv(padapter) == _FAIL)
	{
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("\n Can't init mlme_ext_priv\n"));
		ret8=_FAIL;
		goto exit;
	}

	if (_rtw_init_xmit_priv(&padapter->xmitpriv, padapter) == _FAIL)
	{
		DBG_88E("Can't _rtw_init_xmit_priv\n");
		ret8=_FAIL;
		goto exit;
	}

	if (_rtw_init_recv_priv(&padapter->recvpriv, padapter) == _FAIL)
	{
		DBG_88E("Can't _rtw_init_recv_priv\n");
		ret8=_FAIL;
		goto exit;
	}
	/*  add for CONFIG_IEEE80211W, none 11w also can use */
	spin_lock_init(&padapter->security_key_mutex);

	if (_rtw_init_sta_priv(&padapter->stapriv) == _FAIL)
	{
		DBG_88E("Can't _rtw_init_sta_priv\n");
		ret8=_FAIL;
		goto exit;
	}

	padapter->stapriv.padapter = padapter;
	padapter->setband = GHZ24_50;
	padapter->fix_rate = 0xFF;
	rtw_init_bcmc_stainfo(padapter);

	rtw_init_pwrctrl_priv(padapter);

	ret8 = rtw_init_default_value(padapter);

	rtw_hal_dm_init(padapter);
	rtw_hal_sw_led_init(padapter);

	rtw_hal_sreset_init(padapter);

#ifdef CONFIG_BR_EXT
	spin_lock_init(&padapter->br_ext_lock);
#endif	/*  CONFIG_BR_EXT */

exit:
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("-rtw_init_drv_sw\n"));
	return ret8;
}

void rtw_cancel_all_timer(struct adapter *padapter)
{
	struct wifidirect_info *pwdinfo = &padapter->wdinfo;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+rtw_cancel_all_timer\n"));

	_cancel_timer_ex(&padapter->mlmepriv.assoc_timer);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel association timer complete!\n"));

	_cancel_timer_ex(&padapter->mlmepriv.scan_to_timer);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel scan_to_timer!\n"));

	_cancel_timer_ex(&padapter->mlmepriv.dynamic_chk_timer);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel dynamic_chk_timer!\n"));

	/*  cancel sw led timer */
	rtw_hal_sw_led_deinit(padapter);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel DeInitSwLeds!\n"));

	_cancel_timer_ex(&adapter_to_pwrctl(padapter)->pwr_state_check_timer);

#ifdef CONFIG_P2P
	_cancel_timer_ex(&padapter->cfg80211_wdinfo.remain_on_ch_timer);
#endif /* CONFIG_P2P */

	_cancel_timer_ex(&padapter->mlmepriv.set_scan_deny_timer);
	rtw_clear_scan_deny(padapter);
	RT_TRACE(_module_os_intfs_c_,_drv_info_,("rtw_cancel_all_timer:cancel set_scan_deny_timer!\n"));

	_cancel_timer_ex(&padapter->recvpriv.signal_stat_timer);

#if defined(CONFIG_CHECK_BT_HANG) && defined(CONFIG_BT_COEXIST)
	if (padapter->HalFunc.hal_cancel_checkbthang_workqueue)
		padapter->HalFunc.hal_cancel_checkbthang_workqueue(padapter);
#endif
	_cancel_timer_ex(&pwdinfo->find_phase_timer);
	_cancel_timer_ex(&pwdinfo->restore_p2p_state_timer);
	_cancel_timer_ex(&pwdinfo->pre_tx_scan_timer);
	_cancel_timer_ex(&pwdinfo->reset_ch_sitesurvey);
	_cancel_timer_ex(&pmlmeext->survey_timer);
	_cancel_timer_ex(&pmlmeext->link_timer);
#ifdef CONFIG_IEEE80211W
	_cancel_timer_ex(&pmlmeext->sa_query_timer);
#endif
	/* cancel dm timer */
	rtw_hal_dm_deinit(padapter);

}

u8 rtw_free_drv_sw(struct adapter *padapter)
{
	struct net_device *pnetdev = (struct net_device*)padapter->pnetdev;

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("==>rtw_free_drv_sw"));

	/* we can call rtw_p2p_enable here, but: */
	/*  1. rtw_p2p_enable may have IO operation */
	/*  2. rtw_p2p_enable is bundled with wext interface */
	#ifdef CONFIG_P2P
	{
		struct wifidirect_info *pwdinfo = &padapter->wdinfo;
		if (!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
		{
			_cancel_timer_ex( &pwdinfo->find_phase_timer );
			_cancel_timer_ex( &pwdinfo->restore_p2p_state_timer );
			_cancel_timer_ex( &pwdinfo->pre_tx_scan_timer);
			rtw_p2p_set_state(pwdinfo, P2P_STATE_NONE);
		}
	}
	#endif
	/*  add for CONFIG_IEEE80211W, none 11w also can use */

	free_mlme_ext_priv(&padapter->mlmeextpriv);

	rtw_free_cmd_priv(&padapter->cmdpriv);

	rtw_free_evt_priv(&padapter->evtpriv);

	rtw_free_mlme_priv(&padapter->mlmepriv);
#if defined(CONFIG_CHECK_BT_HANG) && defined(CONFIG_BT_COEXIST)
	if (padapter->HalFunc.hal_free_checkbthang_workqueue)
		padapter->HalFunc.hal_free_checkbthang_workqueue(padapter);
#endif
	/* free_io_queue(padapter); */

	_rtw_free_xmit_priv(&padapter->xmitpriv);

	_rtw_free_sta_priv(&padapter->stapriv); /* will free bcmc_stainfo here */

	_rtw_free_recv_priv(&padapter->recvpriv);

	rtw_free_pwrctrl_priv(padapter);

	/* rtw_mfree((void *)padapter, sizeof (padapter)); */

#ifdef CONFIG_DRVEXT_MODULE
	free_drvext(&padapter->drvextpriv);
#endif

	rtw_hal_free_data(padapter);

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("<==rtw_free_drv_sw\n"));

	/* free the old_pnetdev */
	if (padapter->rereg_nd_name_priv.old_pnetdev) {
		free_netdev(padapter->rereg_nd_name_priv.old_pnetdev);
		padapter->rereg_nd_name_priv.old_pnetdev = NULL;
	}

	/*  clear pbuddy_adapter to avoid access wrong pointer. */
	if (padapter->pbuddy_adapter != NULL) {
		padapter->pbuddy_adapter->pbuddy_adapter = NULL;
	}

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("-rtw_free_drv_sw\n"));

	return _SUCCESS;

}

#ifdef CONFIG_BR_EXT
void netdev_br_init(struct net_device *netdev)
{
	struct adapter *adapter = (struct adapter *)rtw_netdev_priv(netdev);

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	rcu_read_lock();
#endif	/*  (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)) */

	/* if (check_fwstate(pmlmepriv, WIFI_STATION_STATE|WIFI_ADHOC_STATE) == true) */
	{
		/* struct net_bridge	*br = netdev->br_port->br;->dev->dev_addr; */
#if (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 35))
		if (netdev->br_port)
#else   /*  (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 35)) */
		if (rcu_dereference(adapter->pnetdev->rx_handler_data))
#endif  /*  (LINUX_VERSION_CODE <= KERNEL_VERSION(2, 6, 35)) */
		{
			struct net_device *br_netdev;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24))
			br_netdev = dev_get_by_name(CONFIG_BR_EXT_BRNAME);
#else	/*  (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)) */
			struct net *devnet = NULL;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26))
			devnet = netdev->nd_net;
#else	/*  (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)) */
			devnet = dev_net(netdev);
#endif	/*  (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)) */

			br_netdev = dev_get_by_name(devnet, CONFIG_BR_EXT_BRNAME);
#endif	/*  (LINUX_VERSION_CODE < KERNEL_VERSION(2,6,24)) */

			if (br_netdev) {
				memcpy(adapter->br_mac, br_netdev->dev_addr, ETH_ALEN);
				dev_put(br_netdev);
			} else
				DBG_88E("%s()-%d: dev_get_by_name(%s) failed!", __FUNCTION__, __LINE__, CONFIG_BR_EXT_BRNAME);
		}

		adapter->ethBrExtInfo.addPPPoETag = 1;
	}

#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35))
	rcu_read_unlock();
#endif	/*  (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 35)) */
}
#endif /* CONFIG_BR_EXT */

static int _rtw_drv_register_netdev(struct adapter *padapter, char *name)
{
	int ret = _SUCCESS;
	struct net_device *pnetdev = padapter->pnetdev;

	/* alloc netdev name */
	rtw_init_netdev_name(pnetdev, name);

	memcpy(pnetdev->dev_addr, padapter->eeprompriv.mac_addr, ETH_ALEN);

	/* Tell the network stack we exist */
	if (register_netdev(pnetdev) != 0) {
		DBG_88E(FUNC_NDEV_FMT "Failed!\n", FUNC_NDEV_ARG(pnetdev));
		ret = _FAIL;
		goto error_register_netdev;
	}

	DBG_88E("%s, MAC Address (if%d) = " MAC_FMT "\n", __FUNCTION__, (padapter->iface_id+1), MAC_ARG(pnetdev->dev_addr));

	return ret;

error_register_netdev:

	if (padapter->iface_id > IFACE_ID0)
	{
		rtw_free_drv_sw(padapter);

		rtw_free_netdev(pnetdev);
	}

	return ret;
}

int rtw_drv_register_netdev(struct adapter *if1)
{
	int i, status = _SUCCESS;
	struct dvobj_priv *dvobj = if1->dvobj;

	if (dvobj->iface_nums < IFACE_ID_MAX)
	{
		for (i=0; i<dvobj->iface_nums; i++)
		{
			struct adapter *padapter = dvobj->padapters[i];

			if (padapter)
			{
				char *name;

				if (padapter->iface_id == IFACE_ID0)
					name = if1->registrypriv.ifname;
				else
					name = "wlan%d";

				if ((status = _rtw_drv_register_netdev(padapter, name)) != _SUCCESS) {
					break;
				}
			}
		}
	}

	return status;
}

int _netdev_open(struct net_device *pnetdev)
{
	uint status;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(pnetdev);
	struct pwrctrl_priv *pwrctrlpriv = adapter_to_pwrctl(padapter);

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+871x_drv - dev_open\n"));
	DBG_88E("+871x_drv - drv_open, bup=%d\n", padapter->bup);

	if (pwrctrlpriv->ps_flag == true) {
		padapter->net_closed = false;
		goto netdev_open_normal_process;
	}

	if (padapter->bup == false)
	{
		padapter->bDriverStopped = false;
		padapter->bSurpriseRemoved = false;
		padapter->bCardDisableWOHSM = false;

		status = rtw_hal_init(padapter);
		if (status ==_FAIL)
		{
			RT_TRACE(_module_os_intfs_c_,_drv_err_,("rtl871x_hal_init(): Can't init h/w!\n"));
			goto netdev_open_error;
		}

		DBG_88E("MAC Address = "MAC_FMT"\n", MAC_ARG(pnetdev->dev_addr));

		status=rtw_start_drv_threads(padapter);
		if (status ==_FAIL)
		{
			DBG_88E("Initialize driver software resource Failed!\n");
			goto netdev_open_error;
		}

#ifdef CONFIG_DRVEXT_MODULE
		init_drvext(padapter);
#endif

		if (padapter->intf_start)
		{
			padapter->intf_start(padapter);
		}

		rtw_proc_init_one(pnetdev);

		rtw_cfg80211_init_wiphy(padapter);

		rtw_led_control(padapter, LED_CTL_NO_LINK);

		padapter->bup = true;

		pwrctrlpriv->bips_processing = false;
	}
	padapter->net_closed = false;

	_set_timer(&padapter->mlmepriv.dynamic_chk_timer, 2000);

	rtw_set_pwr_state_check_timer(pwrctrlpriv);

	/* netif_carrier_on(pnetdev);call this func when rtw_joinbss_event_callback return success */
	if (!rtw_netif_queue_stopped(pnetdev))
		rtw_netif_start_queue(pnetdev);
	else
		rtw_netif_wake_queue(pnetdev);

#ifdef CONFIG_BR_EXT
	netdev_br_init(pnetdev);
#endif	/*  CONFIG_BR_EXT */

netdev_open_normal_process:

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("-871x_drv - dev_open\n"));
	DBG_88E("-871x_drv - drv_open, bup=%d\n", padapter->bup);

	return 0;

netdev_open_error:

	padapter->bup = false;

	netif_carrier_off(pnetdev);
	rtw_netif_stop_queue(pnetdev);

	RT_TRACE(_module_os_intfs_c_,_drv_err_,("-871x_drv - dev_open, fail!\n"));
	DBG_88E("-871x_drv - drv_open fail, bup=%d\n", padapter->bup);

	return (-1);

}

int netdev_open(struct net_device *pnetdev)
{
	int ret;
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(pnetdev);

	_enter_critical_mutex(&adapter_to_dvobj(padapter)->hw_init_mutex, NULL);
	ret = _netdev_open(pnetdev);
	_exit_critical_mutex(&adapter_to_dvobj(padapter)->hw_init_mutex, NULL);

	return ret;
}

static int  ips_netdrv_open(struct adapter *padapter)
{
	int status = _SUCCESS;
	padapter->net_closed = false;
	DBG_88E("===> %s.........\n",__FUNCTION__);


	padapter->bDriverStopped = false;
	padapter->bCardDisableWOHSM = false;
	/* padapter->bup = true; */

	status = rtw_hal_init(padapter);
	if (status ==_FAIL) {
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("ips_netdrv_open(): Can't init h/w!\n"));
		goto netdev_open_error;
	}

	if (padapter->intf_start)
		padapter->intf_start(padapter);

	rtw_set_pwr_state_check_timer(adapter_to_pwrctl(padapter));
	_set_timer(&padapter->mlmepriv.dynamic_chk_timer,5000);

	return _SUCCESS;

netdev_open_error:
	/* padapter->bup = false; */
	DBG_88E("-ips_netdrv_open - drv_open failure, bup=%d\n", padapter->bup);

	return _FAIL;
}


int rtw_ips_pwr_up(struct adapter *padapter)
{
	int result;
	u32 start_time = jiffies;
	DBG_88E("===>  rtw_ips_pwr_up..............\n");
	rtw_reset_drv_sw(padapter);

	result = ips_netdrv_open(padapter);

	rtw_led_control(padapter, LED_CTL_NO_LINK);

	DBG_88E("<===  rtw_ips_pwr_up.............. in %dms\n", rtw_get_passing_time_ms(start_time));
	return result;

}

void rtw_ips_pwr_down(struct adapter *padapter)
{
	u32 start_time = jiffies;
	DBG_88E("===> rtw_ips_pwr_down...................\n");

	padapter->bCardDisableWOHSM = true;
	padapter->net_closed = true;

	rtw_led_control(padapter, LED_CTL_POWER_OFF);

	rtw_ips_dev_unload(padapter);
	padapter->bCardDisableWOHSM = false;
	DBG_88E("<=== rtw_ips_pwr_down..................... in %dms\n", rtw_get_passing_time_ms(start_time));
}

void rtw_ips_dev_unload(struct adapter *padapter)
{
	struct net_device *pnetdev= (struct net_device*)padapter->pnetdev;
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	DBG_88E("====> %s...\n",__FUNCTION__);

	rtw_hal_set_hwreg(padapter, HW_VAR_FIFO_CLEARN_UP, NULL);

	if (padapter->intf_stop)
	{
		padapter->intf_stop(padapter);
	}

	/* s5. */
	if (padapter->bSurpriseRemoved == false)
	{
		rtw_hal_deinit(padapter);
	}

}

int pm_netdev_open(struct net_device *pnetdev,u8 bnormal)
{
	int status;


	if (true == bnormal)
		status = netdev_open(pnetdev);
	else
		status =  (_SUCCESS == ips_netdrv_open((struct adapter *)rtw_netdev_priv(pnetdev)))?(0):(-1);
	return status;
}

static int netdev_close(struct net_device *pnetdev)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(pnetdev);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("+871x_drv - drv_close\n"));

	if (adapter_to_pwrctl(padapter)->bInternalAutoSuspend == true)
	{
		/* rtw_pwr_wakeup(padapter); */
		if (adapter_to_pwrctl(padapter)->rf_pwrstate == rf_off)
			adapter_to_pwrctl(padapter)->ps_flag = true;
	}
	padapter->net_closed = true;

	if (adapter_to_pwrctl(padapter)->rf_pwrstate == rf_on) {
		DBG_88E("(2)871x_drv - drv_close, bup=%d, hw_init_completed=%d\n", padapter->bup, padapter->hw_init_completed);

		/* s1. */
		if (pnetdev)
		{
			if (!rtw_netif_queue_stopped(pnetdev))
				rtw_netif_stop_queue(pnetdev);
		}

#ifndef CONFIG_ANDROID
		/* s2. */
		LeaveAllPowerSaveMode(padapter);
		rtw_disassoc_cmd(padapter, 500, false);
		/* s2-2.  indicate disconnect to os */
		rtw_indicate_disconnect(padapter);
		/* s2-3. */
		rtw_free_assoc_resources(padapter, 1);
		/* s2-4. */
		rtw_free_network_queue(padapter,true);
#endif
		/*  Close LED */
		rtw_led_control(padapter, LED_CTL_POWER_OFF);
	}

#ifdef CONFIG_BR_EXT
	/* if (OPMODE & (WIFI_STATION_STATE | WIFI_ADHOC_STATE)) */
	{
		/* void nat25_db_cleanup(struct adapter *priv); */
		nat25_db_cleanup(padapter);
	}
#endif	/*  CONFIG_BR_EXT */

#ifdef CONFIG_P2P
	rtw_p2p_enable(padapter, P2P_ROLE_DISABLE);
#endif /* CONFIG_P2P */

	kfree(dvobj->firmware.szFwBuffer);
	dvobj->firmware.szFwBuffer = NULL;
	rtw_scan_abort(padapter);
	wdev_to_priv(padapter->rtw_wdev)->bandroid_scan = false;
	padapter->rtw_wdev->iftype = NL80211_IFTYPE_MONITOR; /* set this at the end */

	RT_TRACE(_module_os_intfs_c_,_drv_info_,("-871x_drv - drv_close\n"));
	DBG_88E("-871x_drv - drv_close, bup=%d\n", padapter->bup);

	return 0;
}

void rtw_ndev_destructor(struct net_device *ndev)
{
	DBG_88E(FUNC_NDEV_FMT"\n", FUNC_NDEV_ARG(ndev));

	if (ndev->ieee80211_ptr)
		rtw_mfree((u8 *)ndev->ieee80211_ptr, sizeof(struct wireless_dev));
	free_netdev(ndev);
}

#ifdef CONFIG_ARP_KEEP_ALIVE
struct route_info {
    struct in_addr dst_addr;
    struct in_addr src_addr;
    struct in_addr gateway;
    unsigned int dev_index;
};

static void parse_routes(struct nlmsghdr *nl_hdr, struct route_info *rt_info)
{
    struct rtmsg *rt_msg;
    struct rtattr *rt_attr;
    int rt_len;

    rt_msg = (struct rtmsg *) NLMSG_DATA(nl_hdr);
    if ((rt_msg->rtm_family != AF_INET) || (rt_msg->rtm_table != RT_TABLE_MAIN))
        return;

    rt_attr = (struct rtattr *) RTM_RTA(rt_msg);
    rt_len = RTM_PAYLOAD(nl_hdr);

    for (; RTA_OK(rt_attr, rt_len); rt_attr = RTA_NEXT(rt_attr, rt_len))
	{
        switch (rt_attr->rta_type) {
        case RTA_OIF:
		rt_info->dev_index = *(int *) RTA_DATA(rt_attr);
            break;
        case RTA_GATEWAY:
            rt_info->gateway.s_addr = *(u_int *) RTA_DATA(rt_attr);
            break;
        case RTA_PREFSRC:
            rt_info->src_addr.s_addr = *(u_int *) RTA_DATA(rt_attr);
            break;
        case RTA_DST:
            rt_info->dst_addr.s_addr = *(u_int *) RTA_DATA(rt_attr);
            break;
        }
    }
}

static int route_dump(u32 *gw_addr ,int* gw_index)
{
	int err = 0;
	struct socket *sock;
	struct {
		struct nlmsghdr nlh;
		struct rtgenmsg g;
	} req;
	struct msghdr msg;
	struct iovec iov;
	struct sockaddr_nl nladdr;
	mm_segment_t oldfs;
	char *pg;
	int size = 0;

	err = sock_create(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE, &sock);
	if (err)
	{
		printk( ": Could not create a datagram socket, error = %d\n", -ENXIO);
		return err;
	}

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	req.nlh.nlmsg_len = sizeof(req);
	req.nlh.nlmsg_type = RTM_GETROUTE;
	req.nlh.nlmsg_flags = NLM_F_ROOT | NLM_F_MATCH | NLM_F_REQUEST;
	req.nlh.nlmsg_pid = 0;
	req.g.rtgen_family = AF_INET;

	iov.iov_base = &req;
	iov.iov_len = sizeof(req);

	msg.msg_name = &nladdr;
	msg.msg_namelen = sizeof(nladdr);
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	msg.msg_flags = MSG_DONTWAIT;

	oldfs = get_fs(); set_fs(KERNEL_DS);
	err = sock_sendmsg(sock, &msg, sizeof(req));
	set_fs(oldfs);

	if (size < 0)
		goto out_sock;

	pg = (char *) __get_free_page(GFP_KERNEL);
	if (pg == NULL) {
		err = -ENOMEM;
		goto out_sock;
	}

#if defined(CONFIG_IPV6) || defined (CONFIG_IPV6_MODULE)
restart:
#endif

	for (;;)
	{
		struct nlmsghdr *h;

		iov.iov_base = pg;
		iov.iov_len = PAGE_SIZE;

		oldfs = get_fs(); set_fs(KERNEL_DS);
		err = sock_recvmsg(sock, &msg, PAGE_SIZE, MSG_DONTWAIT);
		set_fs(oldfs);

		if (err < 0)
			goto out_sock_pg;

		if (msg.msg_flags & MSG_TRUNC) {
			err = -ENOBUFS;
			goto out_sock_pg;
		}

		h = (struct nlmsghdr*) pg;

		while (NLMSG_OK(h, err))
		{
			struct route_info rt_info;
			if (h->nlmsg_type == NLMSG_DONE) {
				err = 0;
				goto done;
			}

			if (h->nlmsg_type == NLMSG_ERROR) {
				struct nlmsgerr *errm = (struct nlmsgerr*) NLMSG_DATA(h);
				err = errm->error;
				printk( "NLMSG error: %d\n", errm->error);
				goto done;
			}

			if (h->nlmsg_type == RTM_GETROUTE)
			{
				printk( "RTM_GETROUTE: NLMSG: %d\n", h->nlmsg_type);
			}
			if (h->nlmsg_type != RTM_NEWROUTE) {
				printk( "NLMSG: %d\n", h->nlmsg_type);
				err = -EINVAL;
				goto done;
			}

			memset(&rt_info, 0, sizeof(struct route_info));
			parse_routes(h, &rt_info);
			if (!rt_info.dst_addr.s_addr && rt_info.gateway.s_addr && rt_info.dev_index)
			{
				*gw_addr = rt_info.gateway.s_addr;
				*gw_index = rt_info.dev_index;

			}
			h = NLMSG_NEXT(h, err);
		}

		if (err)
		{
			printk( "!!!Remnant of size %d %d %d\n", err, h->nlmsg_len, h->nlmsg_type);
			err = -EINVAL;
			break;
		}
	}

done:
#if defined(CONFIG_IPV6) || defined (CONFIG_IPV6_MODULE)
	if (!err && req.g.rtgen_family == AF_INET) {
		req.g.rtgen_family = AF_INET6;

		iov.iov_base = &req;
		iov.iov_len = sizeof(req);

		msg.msg_name = &nladdr;
		msg.msg_namelen = sizeof(nladdr);
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		msg.msg_control = NULL;
		msg.msg_controllen = 0;
		msg.msg_flags=MSG_DONTWAIT;

		oldfs = get_fs(); set_fs(KERNEL_DS);
		err = sock_sendmsg(sock, &msg, sizeof(req));
		set_fs(oldfs);

		if (err > 0)
			goto restart;
	}
#endif

out_sock_pg:
	free_page((unsigned long) pg);

out_sock:
	sock_release(sock);
	return err;
}

static int arp_query(unsigned char *haddr, u32 paddr,
             struct net_device *dev)
{
	struct neighbour *neighbor_entry;
	int	ret = 0;

	neighbor_entry = neigh_lookup(&arp_tbl, &paddr, dev);

	if (neighbor_entry != NULL) {
		neighbor_entry->used = jiffies;
		if (neighbor_entry->nud_state & NUD_VALID) {
			memcpy(haddr, neighbor_entry->ha, dev->addr_len);
			ret = 1;
		}
		neigh_release(neighbor_entry);
	}
	return ret;
}

static int get_defaultgw(u32 *ip_addr ,char mac[])
{
	int gw_index = 0; /*  oif device index */
	struct net_device *gw_dev = NULL; /* oif device */

	route_dump(ip_addr, &gw_index);

	if ( !(*ip_addr) || !gw_index )
	{
		/* DBG_88E("No default GW\n"); */
		return -1;
	}

	gw_dev = dev_get_by_index(&init_net, gw_index);

	if (gw_dev == NULL)
	{
		/* DBG_88E("get Oif Device Fail\n"); */
		return -1;
	}

	if (!arp_query(mac, *ip_addr, gw_dev))
	{
		/* DBG_88E( "arp query failed\n"); */
		dev_put(gw_dev);
		return -1;

	}
	dev_put(gw_dev);

	return 0;
}

int	rtw_gw_addr_query(struct adapter *padapter)
{
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	u32 gw_addr = 0; /*  default gw address */
	unsigned char gw_mac[32] = {0}; /*  default gw mac */
	int i;
	int res;

	res = get_defaultgw(&gw_addr, gw_mac);
	if (!res)
	{
		pmlmepriv->gw_ip[0] = gw_addr&0xff;
		pmlmepriv->gw_ip[1] = (gw_addr&0xff00)>>8;
		pmlmepriv->gw_ip[2] = (gw_addr&0xff0000)>>16;
		pmlmepriv->gw_ip[3] = (gw_addr&0xff000000)>>24;
		memcpy(pmlmepriv->gw_mac_addr, gw_mac, 6);
		DBG_88E("%s Gateway Mac:\t" MAC_FMT "\n", __FUNCTION__, MAC_ARG(pmlmepriv->gw_mac_addr));
		DBG_88E("%s Gateway IP:\t" IP_FMT "\n", __FUNCTION__, IP_ARG(pmlmepriv->gw_ip));
	}
	else
	{
		/* DBG_88E("Get Gateway IP/MAC fail!\n"); */
	}

	return res;
}
#endif

static int rtw_suspend_free_assoc_resource(struct adapter *padapter)
{
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct net_device *pnetdev = padapter->pnetdev;
	struct wifidirect_info*	pwdinfo = &padapter->wdinfo;

	DBG_88E("==> "FUNC_ADPT_FMT" entry....\n", FUNC_ADPT_ARG(padapter));

	rtw_cancel_all_timer(padapter);
	if (pnetdev) {
		netif_carrier_off(pnetdev);
		rtw_netif_stop_queue(pnetdev);
	}

	if (check_fwstate(pmlmepriv, WIFI_STATION_STATE) && check_fwstate(pmlmepriv, _FW_LINKED) && rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
	{
		DBG_88E("%s %s(" MAC_FMT "), length:%d assoc_ssid.length:%d\n",__FUNCTION__,
				pmlmepriv->cur_network.network.Ssid.Ssid,
				MAC_ARG(pmlmepriv->cur_network.network.MacAddress),
				pmlmepriv->cur_network.network.Ssid.SsidLength,
				pmlmepriv->assoc_ssid.SsidLength);
		rtw_set_roaming(padapter, 1);
	}

	if (check_fwstate(pmlmepriv, WIFI_STATION_STATE) && check_fwstate(pmlmepriv, _FW_LINKED))
	{
		rtw_disassoc_cmd(padapter, 0, false);
	}
	#ifdef CONFIG_AP_MODE
	else if (check_fwstate(pmlmepriv, WIFI_AP_STATE))
	{
		rtw_sta_flush(padapter);
	}
	#endif
	if (check_fwstate(pmlmepriv, WIFI_STATION_STATE) ) {
		/* s2-2.  indicate disconnect to os */
		rtw_indicate_disconnect(padapter);
	}

	/* s2-3. */
	rtw_free_assoc_resources(padapter, 1);

	/* s2-4. */
#ifdef CONFIG_AUTOSUSPEND
	if (is_primary_adapter(padapter) && (!adapter_to_pwrctl(padapter)->bInternalAutoSuspend ))
#endif
	rtw_free_network_queue(padapter, true);

	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY))
		rtw_indicate_scan_done(padapter, 1);

	DBG_88E("==> "FUNC_ADPT_FMT" exit....\n", FUNC_ADPT_ARG(padapter));
	return 0;
}

int rtw_suspend_common(struct adapter *padapter)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	int ret = 0;
	;
	LeaveAllPowerSaveMode(padapter);

	rtw_suspend_free_assoc_resource(padapter);

	rtw_led_control(padapter, LED_CTL_POWER_OFF);

	rtw_dev_unload(padapter);

exit:

	;
	return ret;
}

int rtw_resume_common(struct adapter *padapter)
{
	int ret = 0;
	struct net_device *pnetdev= padapter->pnetdev;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct mlme_priv *mlmepriv = &padapter->mlmepriv;

	rtw_reset_drv_sw(padapter);
	pwrpriv->bkeepfwalive = false;

	DBG_88E("bkeepfwalive(%x)\n",pwrpriv->bkeepfwalive);
	if (pm_netdev_open(pnetdev,true) != 0) {
		DBG_88E("%s ==> pm_netdev_open failed\n",__FUNCTION__);
		ret = -1;
		return ret;
	}

	netif_device_attach(pnetdev);
	netif_carrier_on(pnetdev);

	if (check_fwstate(mlmepriv, WIFI_STATION_STATE)) {
		DBG_88E(FUNC_ADPT_FMT" fwstate:0x%08x - WIFI_STATION_STATE\n", FUNC_ADPT_ARG(padapter), get_fwstate(mlmepriv));
		rtw_roaming(padapter, NULL);
	} else if (check_fwstate(mlmepriv, WIFI_AP_STATE)) {
		DBG_88E(FUNC_ADPT_FMT" fwstate:0x%08x - WIFI_AP_STATE\n", FUNC_ADPT_ARG(padapter), get_fwstate(mlmepriv));
		rtw_ap_restore_network(padapter);
	} else if (check_fwstate(mlmepriv, WIFI_ADHOC_STATE)) {
		DBG_88E(FUNC_ADPT_FMT" fwstate:0x%08x - WIFI_ADHOC_STATE\n", FUNC_ADPT_ARG(padapter), get_fwstate(mlmepriv));
	} else {
		DBG_88E(FUNC_ADPT_FMT" fwstate:0x%08x - ???\n", FUNC_ADPT_ARG(padapter), get_fwstate(mlmepriv));
	}
	return ret;
}
