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

#include <drv_types.h>


typedef u8 NDIS_802_11_PMKID_VALUE[16];

u8 rtw_set_802_11_add_key(_adapter * padapter, struct ndis_802_11_key * key);
u8 rtw_set_802_11_authentication_mode(_adapter *pdapter, enum ndis_802_11_auth_mode authmode);
u8 rtw_set_802_11_bssid(_adapter* padapter, u8 *bssid);
u8 rtw_set_802_11_add_wep(_adapter * padapter, struct ndis_802_11_wep * wep);
u8 rtw_set_802_11_disassociate(_adapter * padapter);
u8 rtw_set_802_11_bssid_list_scan(_adapter* padapter, struct ndis_802_11_ssid *pssid, int ssid_max_num);
u8 rtw_set_802_11_infrastructure_mode(_adapter * padapter, enum ndis_802_11_network_infra networktype);
u8 rtw_set_802_11_remove_wep(_adapter * padapter, u32 keyindex);
u8 rtw_set_802_11_ssid(_adapter * padapter, struct ndis_802_11_ssid * ssid);
u8 rtw_set_802_11_remove_key(_adapter * padapter, struct ndis_802_11_remove_key * key);


u8 rtw_validate_ssid(struct ndis_802_11_ssid *ssid);

u16 rtw_get_cur_max_rate(_adapter *adapter);
int rtw_set_scan_mode(_adapter *adapter, RT_SCAN_TYPE scan_mode);
int rtw_set_channel_plan(_adapter *adapter, u8 channel_plan);
int rtw_set_country(_adapter *adapter, const char *country_code);
int rtw_change_ifname(_adapter *padapter, const char *ifname);

#endif
