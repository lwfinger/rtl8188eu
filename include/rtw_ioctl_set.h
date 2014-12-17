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
#ifndef __RTW_IOCTL_SET_H_
#define __RTW_IOCTL_SET_H_

#include <drv_conf.h>
#include <drv_types.h>


typedef u8 NDIS_802_11_PMKID_VALUE[16];

typedef struct _BSSIDInfo {
	NDIS_802_11_MAC_ADDRESS  BSSID;
	NDIS_802_11_PMKID_VALUE  PMKID;
} BSSIDInfo, *PBSSIDInfo;


u8 rtw_set_802_11_add_key(struct adapter * padapter, NDIS_802_11_KEY * key);
u8 rtw_set_802_11_authentication_mode(struct adapter *pdapter, NDIS_802_11_AUTHENTICATION_MODE authmode);
u8 rtw_set_802_11_bssid(struct adapter* padapter, u8 *bssid);
u8 rtw_set_802_11_add_wep(struct adapter * padapter, NDIS_802_11_WEP * wep);
u8 rtw_set_802_11_disassociate(struct adapter * padapter);
u8 rtw_set_802_11_bssid_list_scan(struct adapter* padapter, NDIS_802_11_SSID *pssid, int ssid_max_num);
u8 rtw_set_802_11_infrastructure_mode(struct adapter * padapter, NDIS_802_11_NETWORK_INFRASTRUCTURE networktype);
u8 rtw_set_802_11_remove_wep(struct adapter * padapter, u32 keyindex);
u8 rtw_set_802_11_ssid(struct adapter * padapter, NDIS_802_11_SSID * ssid);
u8 rtw_set_802_11_connect(struct adapter* padapter, u8 *bssid, NDIS_802_11_SSID *ssid);
u8 rtw_set_802_11_remove_key(struct adapter * padapter, NDIS_802_11_REMOVE_KEY * key);

u8 rtw_validate_bssid(u8 *bssid);
u8 rtw_validate_ssid(NDIS_802_11_SSID *ssid);

u16 rtw_get_cur_max_rate(struct adapter *adapter);
int rtw_set_scan_mode(struct adapter *adapter, RT_SCAN_TYPE scan_mode);
int rtw_set_channel_plan(struct adapter *adapter, u8 channel_plan);
int rtw_set_country(struct adapter *adapter, const char *country_code);

#endif

