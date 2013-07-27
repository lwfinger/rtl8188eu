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

#include <linux/module.h>
#include <linux/netdevice.h>

#include <rtw_android.h>
#include <osdep_service.h>
#include <rtw_debug.h>
#include <ioctl_cfg80211.h>
#include <rtw_ioctl_set.h>

static const char *android_wifi_cmd_str[ANDROID_WIFI_CMD_MAX] = {
	"START",
	"STOP",
	"SCAN-ACTIVE",
	"SCAN-PASSIVE",
	"RSSI",
	"LINKSPEED",
	"RXFILTER-START",
	"RXFILTER-STOP",
	"RXFILTER-ADD",
	"RXFILTER-REMOVE",
	"BTCOEXSCAN-START",
	"BTCOEXSCAN-STOP",
	"BTCOEXMODE",
	"SETSUSPENDOPT",
	"P2P_DEV_ADDR",
	"SETFWPATH",
	"SETBAND",
	"GETBAND",
	"COUNTRY",
	"P2P_SET_NOA",
	"P2P_GET_NOA",
	"P2P_SET_PS",
	"SET_AP_WPS_P2P_IE",
	"MACADDR",
	"BLOCK",
	"WFD-ENABLE",
	"WFD-DISABLE",
	"WFD-SET-TCPPORT",
	"WFD-SET-MAXTPUT",
	"WFD-SET-DEVTYPE",
};

struct android_wifi_priv_cmd {
	const char __user *buf;
	int used_len;
	int total_len;
};

/**
 * Local (static) functions and variables
 */

/* Initialize g_wifi_on to 1 so dhd_bus_start will be called for the first
 * time (only) in dhd_open, subsequential wifi on will be handled by
 * wl_android_wifi_on
 */
static int g_wifi_on = true;

int rtw_android_cmdstr_to_num(char *cmdstr)
{
	int cmd_num;
	for (cmd_num=0 ; cmd_num<ANDROID_WIFI_CMD_MAX; cmd_num++)
		if (0 == strnicmp(cmdstr , android_wifi_cmd_str[cmd_num], strlen(android_wifi_cmd_str[cmd_num])) )
			break;

	return cmd_num;
}

static int rtw_android_get_rssi(struct net_device *net, char *command, int total_len)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(net);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct	wlan_network	*pcur_network = &pmlmepriv->cur_network;
	int bytes_written = 0;

	if (check_fwstate(pmlmepriv, _FW_LINKED) == true) {
		bytes_written += snprintf(&command[bytes_written], total_len, "%s rssi %d",
			pcur_network->network.Ssid.Ssid, padapter->recvpriv.rssi);
	}

	return bytes_written;
}

static int rtw_android_get_link_speed(struct net_device *net, char *command, int total_len)
{
	struct adapter *padapter = (struct adapter *)rtw_netdev_priv(net);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct	wlan_network	*pcur_network = &pmlmepriv->cur_network;
	int bytes_written = 0;
	u16 link_speed = 0;

	link_speed = rtw_get_cur_max_rate(padapter)/10;
	bytes_written = snprintf(command, total_len, "LinkSpeed %d", link_speed);

	return bytes_written;
}

static int rtw_android_get_macaddr(struct net_device *net, char *command, int total_len)
{
	struct adapter *adapter = (struct adapter *)rtw_netdev_priv(net);
	int bytes_written = 0;

	bytes_written = snprintf(command, total_len, "Macaddr = %pM", net->dev_addr);
	return bytes_written;
}

static int rtw_android_set_country(struct net_device *net, char *command, int total_len)
{
	struct adapter *adapter = (struct adapter *)rtw_netdev_priv(net);
	char *country_code = command + strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_COUNTRY]) + 1;
	int ret;

	ret = rtw_set_country(adapter, country_code);

	return (ret==_SUCCESS)?0:-1;
}

static int rtw_android_get_p2p_dev_addr(struct net_device *net, char *command, int total_len)
{
	int ret;
	int bytes_written = 0;

	//We use the same address as our HW MAC address
	_rtw_memcpy(command, net->dev_addr, ETH_ALEN);

	bytes_written = ETH_ALEN;
	return bytes_written;
}

static int rtw_android_set_block(struct net_device *net, char *command, int total_len)
{
	int ret;
	struct adapter *adapter = (struct adapter *)rtw_netdev_priv(net);
	char *block_value = command + strlen(android_wifi_cmd_str[ANDROID_WIFI_CMD_BLOCK]) + 1;

	return 0;
}

static int get_int_from_command( char* pcmd )
{
	int i = 0;

	for ( i = 0; i < strlen( pcmd ); i++ )
	{
		if ( pcmd[ i ] == '=' )
		{
			//	Skip the '=' and space characters.
			i += 2;
			break;
		}
	}
	return ( rtw_atoi( pcmd + i ) );
}

int rtw_android_priv_cmd(struct net_device *net, struct ifreq *ifr, int cmd)
{
	int ret = 0;
	char *command = NULL;
	int cmd_num;
	int bytes_written = 0;
	struct android_wifi_priv_cmd priv_cmd;

	rtw_lock_suspend();

	if (!ifr->ifr_data) {
		ret = -EINVAL;
		goto exit;
	}
	if (copy_from_user(&priv_cmd, ifr->ifr_data, sizeof(struct android_wifi_priv_cmd))) {
		ret = -EFAULT;
		goto exit;
	}

	command = kmalloc(priv_cmd.total_len, GFP_KERNEL);
	if (!command)
	{
		DBG_88E("%s: failed to allocate memory\n", __func__);
		ret = -ENOMEM;
		goto exit;
	}

	if (!access_ok(VERIFY_READ, priv_cmd.buf, priv_cmd.total_len)){
		DBG_88E("%s: failed to access memory\n", __func__);
		ret = -EFAULT;
		goto exit;
	 }
	if (copy_from_user(command, (char __user *)priv_cmd.buf, priv_cmd.total_len)) {
		ret = -EFAULT;
		goto exit;
	}

	DBG_88E("%s: Android private cmd \"%s\" on %s\n"
		, __func__, command, ifr->ifr_name);

	cmd_num = rtw_android_cmdstr_to_num(command);

	switch (cmd_num) {
	case ANDROID_WIFI_CMD_START:
		//bytes_written = wl_android_wifi_on(net);
		goto response;
	case ANDROID_WIFI_CMD_SETFWPATH:
		goto response;
	}

	if (!g_wifi_on) {
		DBG_88E("%s: Ignore private cmd \"%s\" - iface %s is down\n"
			,__func__, command, ifr->ifr_name);
		ret = 0;
		goto exit;
	}

	switch (cmd_num) {
	case ANDROID_WIFI_CMD_STOP:
		break;
	case ANDROID_WIFI_CMD_SCAN_ACTIVE:
		break;
	case ANDROID_WIFI_CMD_SCAN_PASSIVE:
		break;
	case ANDROID_WIFI_CMD_RSSI:
		bytes_written = rtw_android_get_rssi(net, command, priv_cmd.total_len);
		break;
	case ANDROID_WIFI_CMD_LINKSPEED:
		bytes_written = rtw_android_get_link_speed(net, command, priv_cmd.total_len);
		break;
	case ANDROID_WIFI_CMD_MACADDR:
		bytes_written = rtw_android_get_macaddr(net, command, priv_cmd.total_len);
		break;
	case ANDROID_WIFI_CMD_BLOCK:
		bytes_written = rtw_android_set_block(net, command, priv_cmd.total_len);
		break;
	case ANDROID_WIFI_CMD_RXFILTER_START:
		break;
	case ANDROID_WIFI_CMD_RXFILTER_STOP:
		break;
	case ANDROID_WIFI_CMD_RXFILTER_ADD:
		break;
	case ANDROID_WIFI_CMD_RXFILTER_REMOVE:
		break;
	case ANDROID_WIFI_CMD_BTCOEXSCAN_START:
		/* TBD: BTCOEXSCAN-START */
		break;
	case ANDROID_WIFI_CMD_BTCOEXSCAN_STOP:
		/* TBD: BTCOEXSCAN-STOP */
		break;
	case ANDROID_WIFI_CMD_BTCOEXMODE:
		break;
	case ANDROID_WIFI_CMD_SETSUSPENDOPT:
		break;
	case ANDROID_WIFI_CMD_SETBAND:
		break;
	case ANDROID_WIFI_CMD_GETBAND:
		break;
	case ANDROID_WIFI_CMD_COUNTRY:
		bytes_written = rtw_android_set_country(net, command, priv_cmd.total_len);
		break;
	case ANDROID_WIFI_CMD_P2P_DEV_ADDR:
		bytes_written = rtw_android_get_p2p_dev_addr(net, command, priv_cmd.total_len);
		break;
	case ANDROID_WIFI_CMD_P2P_SET_NOA:
		break;
	case ANDROID_WIFI_CMD_P2P_GET_NOA:
		break;
	case ANDROID_WIFI_CMD_P2P_SET_PS:
		break;
	default:
		DBG_88E("Unknown PRIVATE command %s - ignored\n", command);
		snprintf(command, 3, "OK");
		bytes_written = strlen("OK");
	}

response:
	if (bytes_written >= 0) {
		if ((bytes_written == 0) && (priv_cmd.total_len > 0))
			command[0] = '\0';
		if (bytes_written >= priv_cmd.total_len) {
			DBG_88E("%s: bytes_written = %d\n", __func__, bytes_written);
			bytes_written = priv_cmd.total_len;
		} else {
			bytes_written++;
		}
		priv_cmd.used_len = bytes_written;
		if (copy_to_user((char __user *)priv_cmd.buf, command, bytes_written)) {
			DBG_88E("%s: failed to copy data to user buffer\n", __func__);
			ret = -EFAULT;
		}
	} else {
		ret = bytes_written;
	}
exit:
	rtw_unlock_suspend();
	if (command)
		kfree(command);
	return ret;
}
