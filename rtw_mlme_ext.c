// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#define _RTW_MLME_EXT_C_

#include <drv_types.h>
#ifdef CONFIG_IOCTL_CFG80211
	#include <rtw_wifi_regd.h>
#endif /* CONFIG_IOCTL_CFG80211 */
#include <hal_data.h>
#include <linux/printk.h>

static struct mlme_handler mlme_sta_tbl[] = {
	{WIFI_ASSOCREQ,		"OnAssocReq",	&OnAssocReq},
	{WIFI_ASSOCRSP,		"OnAssocRsp",	&OnAssocRsp},
	{WIFI_REASSOCREQ,	"OnReAssocReq",	&OnAssocReq},
	{WIFI_REASSOCRSP,	"OnReAssocRsp",	&OnAssocRsp},
	{WIFI_PROBEREQ,		"OnProbeReq",	&OnProbeReq},
	{WIFI_PROBERSP,		"OnProbeRsp",		&OnProbeRsp},

	/*----------------------------------------------------------
					below 2 are reserved
	-----------------------------------------------------------*/
	{0,					"DoReserved",		&DoReserved},
	{0,					"DoReserved",		&DoReserved},
	{WIFI_BEACON,		"OnBeacon",		&OnBeacon},
	{WIFI_ATIM,			"OnATIM",		&OnAtim},
	{WIFI_DISASSOC,		"OnDisassoc",		&OnDisassoc},
	{WIFI_AUTH,			"OnAuth",		&OnAuthClient},
	{WIFI_DEAUTH,		"OnDeAuth",		&OnDeAuth},
	{WIFI_ACTION,		"OnAction",		&OnAction},
	{WIFI_ACTION_NOACK, "OnActionNoAck",	&OnAction},
};

#ifdef _CONFIG_NATIVEAP_MLME_
struct mlme_handler mlme_ap_tbl[] = {
	{WIFI_ASSOCREQ,		"OnAssocReq",	&OnAssocReq},
	{WIFI_ASSOCRSP,		"OnAssocRsp",	&OnAssocRsp},
	{WIFI_REASSOCREQ,	"OnReAssocReq",	&OnAssocReq},
	{WIFI_REASSOCRSP,	"OnReAssocRsp",	&OnAssocRsp},
	{WIFI_PROBEREQ,		"OnProbeReq",	&OnProbeReq},
	{WIFI_PROBERSP,		"OnProbeRsp",		&OnProbeRsp},

	/*----------------------------------------------------------
					below 2 are reserved
	-----------------------------------------------------------*/
	{0,					"DoReserved",		&DoReserved},
	{0,					"DoReserved",		&DoReserved},
	{WIFI_BEACON,		"OnBeacon",		&OnBeacon},
	{WIFI_ATIM,			"OnATIM",		&OnAtim},
	{WIFI_DISASSOC,		"OnDisassoc",		&OnDisassoc},
	{WIFI_AUTH,			"OnAuth",		&OnAuth},
	{WIFI_DEAUTH,		"OnDeAuth",		&OnDeAuth},
	{WIFI_ACTION,		"OnAction",		&OnAction},
	{WIFI_ACTION_NOACK, "OnActionNoAck",	&OnAction},
};
#endif

static struct action_handler OnAction_tbl[] = {
	{RTW_WLAN_CATEGORY_SPECTRUM_MGMT,	 "ACTION_SPECTRUM_MGMT", on_action_spct},
	{RTW_WLAN_CATEGORY_QOS, "ACTION_QOS", &OnAction_qos},
	{RTW_WLAN_CATEGORY_DLS, "ACTION_DLS", &OnAction_dls},
	{RTW_WLAN_CATEGORY_BACK, "ACTION_BACK", &OnAction_back},
	{RTW_WLAN_CATEGORY_PUBLIC, "ACTION_PUBLIC", on_action_public},
	{RTW_WLAN_CATEGORY_RADIO_MEASUREMENT, "ACTION_RADIO_MEASUREMENT", &DoReserved},
	{RTW_WLAN_CATEGORY_FT, "ACTION_FT",	&OnAction_ft},
	{RTW_WLAN_CATEGORY_HT,	"ACTION_HT",	&OnAction_ht},
#ifdef CONFIG_IEEE80211W
	{RTW_WLAN_CATEGORY_SA_QUERY, "ACTION_SA_QUERY", &OnAction_sa_query},
#else
	{RTW_WLAN_CATEGORY_SA_QUERY, "ACTION_SA_QUERY", &DoReserved},
#endif /* CONFIG_IEEE80211W */
#ifdef CONFIG_RTW_WNM
	{RTW_WLAN_CATEGORY_WNM, "ACTION_WNM", &on_action_wnm},
#endif
	{RTW_WLAN_CATEGORY_UNPROTECTED_WNM, "ACTION_UNPROTECTED_WNM", &DoReserved},
	{RTW_WLAN_CATEGORY_SELF_PROTECTED, "ACTION_SELF_PROTECTED", &DoReserved},
	{RTW_WLAN_CATEGORY_WMM, "ACTION_WMM", &OnAction_wmm},
	{RTW_WLAN_CATEGORY_VHT, "ACTION_VHT", &OnAction_vht},
	{RTW_WLAN_CATEGORY_P2P, "ACTION_P2P", &OnAction_p2p},
};


static u8	null_addr[ETH_ALEN] = {0, 0, 0, 0, 0, 0};

/**************************************************
OUI definitions for the vendor specific IE
***************************************************/
unsigned char	RTW_WPA_OUI[] = {0x00, 0x50, 0xf2, 0x01};
unsigned char WMM_OUI[] = {0x00, 0x50, 0xf2, 0x02};
unsigned char	WPS_OUI[] = {0x00, 0x50, 0xf2, 0x04};
unsigned char	P2P_OUI[] = {0x50, 0x6F, 0x9A, 0x09};
unsigned char	WFD_OUI[] = {0x50, 0x6F, 0x9A, 0x0A};

unsigned char	WMM_INFO_OUI[] = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};
unsigned char	WMM_PARA_OUI[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};

unsigned char WPA_TKIP_CIPHER[4] = {0x00, 0x50, 0xf2, 0x02};
unsigned char RSN_TKIP_CIPHER[4] = {0x00, 0x0f, 0xac, 0x02};

#ifdef LEGACY_CHANNEL_PLAN_REF
/********************************************************
ChannelPlan definitions
*********************************************************/
static RT_CHANNEL_PLAN legacy_channel_plan[] = {
	/* 0x00, RTW_CHPLAN_FCC */						{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165}, 32},
	/* 0x01, RTW_CHPLAN_IC */						{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 136, 140, 149, 153, 157, 161, 165}, 31},
	/* 0x02, RTW_CHPLAN_ETSI */						{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140}, 32},
	/* 0x03, RTW_CHPLAN_SPAIN */						{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}, 13},
	/* 0x04, RTW_CHPLAN_FRANCE */					{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}, 13},
	/* 0x05, RTW_CHPLAN_MKK */						{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}, 13},
	/* 0x06, RTW_CHPLAN_MKK1 */						{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}, 13},
	/* 0x07, RTW_CHPLAN_ISRAEL */					{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 36, 40, 44, 48, 52, 56, 60, 64}, 21},
	/* 0x08, RTW_CHPLAN_TELEC */						{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 36, 40, 44, 48, 52, 56, 60, 64}, 22},
	/* 0x09, RTW_CHPLAN_GLOBAL_DOAMIN */			{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14}, 14},
	/* 0x0A, RTW_CHPLAN_WORLD_WIDE_13 */			{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13}, 13},
	/* 0x0B, RTW_CHPLAN_TAIWAN */					{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 56, 60, 64, 100, 104, 108, 112, 116, 136, 140, 149, 153, 157, 161, 165}, 26},
	/* 0x0C, RTW_CHPLAN_CHINA */					{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 149, 153, 157, 161, 165}, 18},
	/* 0x0D, RTW_CHPLAN_SINGAPORE_INDIA_MEXICO */	{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165}, 24},
	/* 0x0E, RTW_CHPLAN_KOREA */					{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 149, 153, 157, 161, 165}, 31},
	/* 0x0F, RTW_CHPLAN_TURKEY */					{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 36, 40, 44, 48, 52, 56, 60, 64}, 19},
	/* 0x10, RTW_CHPLAN_JAPAN */						{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140}, 32},
	/* 0x11, RTW_CHPLAN_FCC_NO_DFS */				{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 36, 40, 44, 48, 149, 153, 157, 161, 165}, 20},
	/* 0x12, RTW_CHPLAN_JAPAN_NO_DFS */				{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 36, 40, 44, 48}, 17},
	/* 0x13, RTW_CHPLAN_WORLD_WIDE_5G */			{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165}, 37},
	/* 0x14, RTW_CHPLAN_TAIWAN_NO_DFS */			{{1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 56, 60, 64, 149, 153, 157, 161, 165}, 19},
};
#endif

static struct ch_list_t RTW_ChannelPlan2G[] = {
	/* 0, RTW_RD_2G_NULL */		CH_LIST_ENT(0),
	/* 1, RTW_RD_2G_WORLD */	CH_LIST_ENT(13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13),
	/* 2, RTW_RD_2G_ETSI1 */		CH_LIST_ENT(13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13),
	/* 3, RTW_RD_2G_FCC1 */		CH_LIST_ENT(11, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11),
	/* 4, RTW_RD_2G_MKK1 */		CH_LIST_ENT(14, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14),
	/* 5, RTW_RD_2G_ETSI2 */		CH_LIST_ENT(4, 10, 11, 12, 13),
	/* 6, RTW_RD_2G_GLOBAL */	CH_LIST_ENT(14, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14),
	/* 7, RTW_RD_2G_MKK2 */		CH_LIST_ENT(13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13),
	/* 8, RTW_RD_2G_FCC2 */		CH_LIST_ENT(13, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13),
};

#ifdef CONFIG_IEEE80211_BAND_5GHZ
static struct ch_list_t RTW_ChannelPlan5G[] = {
	/* 0, RTW_RD_5G_NULL */		CH_LIST_ENT(0),
	/* 1, RTW_RD_5G_ETSI1 */		CH_LIST_ENT(19, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140),
	/* 2, RTW_RD_5G_ETSI2 */		CH_LIST_ENT(24, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165),
	/* 3, RTW_RD_5G_ETSI3 */		CH_LIST_ENT(22, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 149, 153, 157, 161, 165),
	/* 4, RTW_RD_5G_FCC1 */		CH_LIST_ENT(24, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 149, 153, 157, 161, 165),
	/* 5, RTW_RD_5G_FCC2 */		CH_LIST_ENT(9, 36, 40, 44, 48, 149, 153, 157, 161, 165),
	/* 6, RTW_RD_5G_FCC3 */		CH_LIST_ENT(13, 36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165),
	/* 7, RTW_RD_5G_FCC4 */		CH_LIST_ENT(12, 36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161),
	/* 8, RTW_RD_5G_FCC5 */		CH_LIST_ENT(5, 149, 153, 157, 161, 165),
	/* 9, RTW_RD_5G_FCC6 */		CH_LIST_ENT(8, 36, 40, 44, 48, 52, 56, 60, 64),
	/* 10, RTW_RD_5G_FCC7 */	CH_LIST_ENT(21, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165),
	/* 11, RTW_RD_5G_KCC1 */	CH_LIST_ENT(19, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 149, 153, 157, 161),
	/* 12, RTW_RD_5G_MKK1 */	CH_LIST_ENT(19, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140),
	/* 13, RTW_RD_5G_MKK2 */	CH_LIST_ENT(8, 36, 40, 44, 48, 52, 56, 60, 64),
	/* 14, RTW_RD_5G_MKK3 */	CH_LIST_ENT(11, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140),
	/* 15, RTW_RD_5G_NCC1 */	CH_LIST_ENT(16, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165),
	/* 16, RTW_RD_5G_NCC2 */	CH_LIST_ENT(8, 56, 60, 64, 149, 153, 157, 161, 165),
	/* 17, RTW_RD_5G_NCC3 */	CH_LIST_ENT(5, 149, 153, 157, 161, 165),
	/* 18, RTW_RD_5G_ETSI4 */	CH_LIST_ENT(4, 36, 40, 44, 48),
	/* 19, RTW_RD_5G_ETSI5 */	CH_LIST_ENT(21, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165),
	/* 20, RTW_RD_5G_FCC8 */	CH_LIST_ENT(4, 149, 153, 157, 161),
	/* 21, RTW_RD_5G_ETSI6 */	CH_LIST_ENT(8, 36, 40, 44, 48, 52, 56, 60, 64),
	/* 22, RTW_RD_5G_ETSI7 */	CH_LIST_ENT(13, 36, 40, 44, 48, 52, 56, 60, 64, 149, 153, 157, 161, 165),
	/* 23, RTW_RD_5G_ETSI8 */	CH_LIST_ENT(9, 36, 40, 44, 48, 149, 153, 157, 161, 165),
	/* 24, RTW_RD_5G_ETSI9 */	CH_LIST_ENT(11, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140),
	/* 25, RTW_RD_5G_ETSI10 */	CH_LIST_ENT(5, 149, 153, 157, 161, 165),
	/* 26, RTW_RD_5G_ETSI11 */	CH_LIST_ENT(16, 36, 40, 44, 48, 52, 56, 60, 64, 132, 136, 140, 149, 153, 157, 161, 165),
	/* 27, RTW_RD_5G_NCC4 */	CH_LIST_ENT(17, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165),
	/* 28, RTW_RD_5G_ETSI12 */	CH_LIST_ENT(4, 149, 153, 157, 161),
	/* 29, RTW_RD_5G_FCC9 */	CH_LIST_ENT(21, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161, 165),
	/* 30, RTW_RD_5G_ETSI13 */	CH_LIST_ENT(16, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140),
	/* 31, RTW_RD_5G_FCC10 */	CH_LIST_ENT(20, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 132, 136, 140, 149, 153, 157, 161),
	/* 32, RTW_RD_5G_MKK4 */	CH_LIST_ENT(4, 36, 40, 44, 48),
	/* 33, RTW_RD_5G_ETSI14 */	CH_LIST_ENT(11, 36, 40, 44, 48, 52, 56, 60, 64, 132, 136, 140),
	/* 34, RTW_RD_5G_FCC11 */	CH_LIST_ENT(25, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140, 144, 149, 153, 157, 161, 165),

	/* === Below are driver defined for legacy channel plan compatible, NO static index assigned ==== */
	/* RTW_RD_5G_OLD_FCC1 */	CH_LIST_ENT(20, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 136, 140, 149, 153, 157, 161, 165),
	/* RTW_RD_5G_OLD_NCC1 */	CH_LIST_ENT(15, 56, 60, 64, 100, 104, 108, 112, 116, 136, 140, 149, 153, 157, 161, 165),
	/* RTW_RD_5G_OLD_KCC1 */	CH_LIST_ENT(20, 36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112, 116, 120, 124, 149, 153, 157, 161, 165),
};
#endif /* CONFIG_IEEE80211_BAND_5GHZ */

static RT_CHANNEL_PLAN_MAP	RTW_ChannelPlanMap[] = {
	/* ===== 0x00 ~ 0x1F, legacy channel plan ===== */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_KCC1,		TXPWR_LMT_FCC),		/* 0x00, RTW_CHPLAN_FCC */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_OLD_FCC1,	TXPWR_LMT_FCC),		/* 0x01, RTW_CHPLAN_IC */
	CHPLAN_ENT(RTW_RD_2G_ETSI1,		RTW_RD_5G_ETSI1,	TXPWR_LMT_ETSI),	/* 0x02, RTW_CHPLAN_ETSI */
	CHPLAN_ENT(RTW_RD_2G_ETSI1,		RTW_RD_5G_NULL,		TXPWR_LMT_ETSI),	/* 0x03, RTW_CHPLAN_SPAIN */
	CHPLAN_ENT(RTW_RD_2G_ETSI1,		RTW_RD_5G_NULL,		TXPWR_LMT_ETSI),	/* 0x04, RTW_CHPLAN_FRANCE */
	CHPLAN_ENT(RTW_RD_2G_MKK1,		RTW_RD_5G_NULL,		TXPWR_LMT_MKK),		/* 0x05, RTW_CHPLAN_MKK */
	CHPLAN_ENT(RTW_RD_2G_MKK1,		RTW_RD_5G_NULL,		TXPWR_LMT_MKK),		/* 0x06, RTW_CHPLAN_MKK1 */
	CHPLAN_ENT(RTW_RD_2G_ETSI1,		RTW_RD_5G_FCC6,		TXPWR_LMT_ETSI),	/* 0x07, RTW_CHPLAN_ISRAEL */
	CHPLAN_ENT(RTW_RD_2G_MKK1,		RTW_RD_5G_FCC6,		TXPWR_LMT_MKK),		/* 0x08, RTW_CHPLAN_TELEC */
	CHPLAN_ENT(RTW_RD_2G_MKK1,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x09, RTW_CHPLAN_GLOBAL_DOAMIN */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x0A, RTW_CHPLAN_WORLD_WIDE_13 */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_OLD_NCC1,	TXPWR_LMT_FCC),		/* 0x0B, RTW_CHPLAN_TAIWAN */
	CHPLAN_ENT(RTW_RD_2G_ETSI1,		RTW_RD_5G_FCC5,		TXPWR_LMT_ETSI),	/* 0x0C, RTW_CHPLAN_CHINA */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_FCC3,		TXPWR_LMT_WW),		/* 0x0D, RTW_CHPLAN_SINGAPORE_INDIA_MEXICO */ /* ETSI:Singapore, India. FCC:Mexico => WW */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_OLD_KCC1,	TXPWR_LMT_ETSI),	/* 0x0E, RTW_CHPLAN_KOREA */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_FCC6,		TXPWR_LMT_ETSI),	/* 0x0F, RTW_CHPLAN_TURKEY */
	CHPLAN_ENT(RTW_RD_2G_ETSI1,		RTW_RD_5G_ETSI1,	TXPWR_LMT_MKK),		/* 0x10, RTW_CHPLAN_JAPAN */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_FCC2,		TXPWR_LMT_FCC),		/* 0x11, RTW_CHPLAN_FCC_NO_DFS */
	CHPLAN_ENT(RTW_RD_2G_ETSI1,		RTW_RD_5G_FCC7,		TXPWR_LMT_MKK),		/* 0x12, RTW_CHPLAN_JAPAN_NO_DFS */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_FCC1,		TXPWR_LMT_WW),		/* 0x13, RTW_CHPLAN_WORLD_WIDE_5G */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_NCC2,		TXPWR_LMT_FCC),		/* 0x14, RTW_CHPLAN_TAIWAN_NO_DFS */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_FCC7,		TXPWR_LMT_ETSI),	/* 0x15, RTW_CHPLAN_ETSI_NO_DFS */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_NCC1,		TXPWR_LMT_ETSI),	/* 0x16, RTW_CHPLAN_KOREA_NO_DFS */
	CHPLAN_ENT(RTW_RD_2G_MKK1,		RTW_RD_5G_FCC7,		TXPWR_LMT_MKK),		/* 0x17, RTW_CHPLAN_JAPAN_NO_DFS */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_FCC5,		TXPWR_LMT_ETSI),	/* 0x18, RTW_CHPLAN_PAKISTAN_NO_DFS */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_FCC5,		TXPWR_LMT_FCC),		/* 0x19, RTW_CHPLAN_TAIWAN2_NO_DFS */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x1A, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x1B, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x1C, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x1D, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x1E, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_FCC1,		TXPWR_LMT_WW),		/* 0x1F, RTW_CHPLAN_WORLD_WIDE_ONLY_5G */

	/* ===== 0x20 ~ 0x7F, new channel plan ===== */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x20, RTW_CHPLAN_WORLD_NULL */
	CHPLAN_ENT(RTW_RD_2G_ETSI1,		RTW_RD_5G_NULL,		TXPWR_LMT_ETSI),	/* 0x21, RTW_CHPLAN_ETSI1_NULL */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_NULL,		TXPWR_LMT_FCC),		/* 0x22, RTW_CHPLAN_FCC1_NULL */
	CHPLAN_ENT(RTW_RD_2G_MKK1,		RTW_RD_5G_NULL,		TXPWR_LMT_MKK),		/* 0x23, RTW_CHPLAN_MKK1_NULL */
	CHPLAN_ENT(RTW_RD_2G_ETSI2,		RTW_RD_5G_NULL,		TXPWR_LMT_ETSI),	/* 0x24, RTW_CHPLAN_ETSI2_NULL */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_FCC1,		TXPWR_LMT_FCC),		/* 0x25, RTW_CHPLAN_FCC1_FCC1 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_ETSI1,	TXPWR_LMT_ETSI),	/* 0x26, RTW_CHPLAN_WORLD_ETSI1 */
	CHPLAN_ENT(RTW_RD_2G_MKK1,		RTW_RD_5G_MKK1,		TXPWR_LMT_MKK),		/* 0x27, RTW_CHPLAN_MKK1_MKK1 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_KCC1,		TXPWR_LMT_ETSI),	/* 0x28, RTW_CHPLAN_WORLD_KCC1 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_FCC2,		TXPWR_LMT_FCC),		/* 0x29, RTW_CHPLAN_WORLD_FCC2 */
	CHPLAN_ENT(RTW_RD_2G_FCC2,		RTW_RD_5G_NULL,		TXPWR_LMT_FCC),		/* 0x2A, RTW_CHPLAN_FCC2_NULL */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x2B, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x2C, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x2D, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x2E, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x2F, */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_FCC3,		TXPWR_LMT_FCC),		/* 0x30, RTW_CHPLAN_WORLD_FCC3 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_FCC4,		TXPWR_LMT_FCC),		/* 0x31, RTW_CHPLAN_WORLD_FCC4 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_FCC5,		TXPWR_LMT_FCC),		/* 0x32, RTW_CHPLAN_WORLD_FCC5 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_FCC6,		TXPWR_LMT_FCC),		/* 0x33, RTW_CHPLAN_WORLD_FCC6 */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_FCC7,		TXPWR_LMT_FCC),		/* 0x34, RTW_CHPLAN_FCC1_FCC7 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_ETSI2,	TXPWR_LMT_ETSI),	/* 0x35, RTW_CHPLAN_WORLD_ETSI2 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_ETSI3,	TXPWR_LMT_ETSI),	/* 0x36, RTW_CHPLAN_WORLD_ETSI3 */
	CHPLAN_ENT(RTW_RD_2G_MKK1,		RTW_RD_5G_MKK2,		TXPWR_LMT_MKK),		/* 0x37, RTW_CHPLAN_MKK1_MKK2 */
	CHPLAN_ENT(RTW_RD_2G_MKK1,		RTW_RD_5G_MKK3,		TXPWR_LMT_MKK),		/* 0x38, RTW_CHPLAN_MKK1_MKK3 */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_NCC1,		TXPWR_LMT_FCC),		/* 0x39, RTW_CHPLAN_FCC1_NCC1 */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x3A, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x3B, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x3C, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x3D, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x3E, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x3F, */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_NCC2,		TXPWR_LMT_FCC),		/* 0x40, RTW_CHPLAN_FCC1_NCC2 */
	CHPLAN_ENT(RTW_RD_2G_GLOBAL,	RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x41, RTW_CHPLAN_GLOBAL_NULL */
	CHPLAN_ENT(RTW_RD_2G_ETSI1,		RTW_RD_5G_ETSI4,	TXPWR_LMT_ETSI),	/* 0x42, RTW_CHPLAN_ETSI1_ETSI4 */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_FCC2,		TXPWR_LMT_FCC),		/* 0x43, RTW_CHPLAN_FCC1_FCC2 */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_NCC3,		TXPWR_LMT_FCC),		/* 0x44, RTW_CHPLAN_FCC1_NCC3 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_ETSI5,	TXPWR_LMT_ETSI),	/* 0x45, RTW_CHPLAN_WORLD_ETSI5 */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_FCC8,		TXPWR_LMT_FCC),		/* 0x46, RTW_CHPLAN_FCC1_FCC8 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_ETSI6,	TXPWR_LMT_ETSI),	/* 0x47, RTW_CHPLAN_WORLD_ETSI6 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_ETSI7,	TXPWR_LMT_ETSI),	/* 0x48, RTW_CHPLAN_WORLD_ETSI7 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_ETSI8,	TXPWR_LMT_ETSI),	/* 0x49, RTW_CHPLAN_WORLD_ETSI8 */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x4A, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x4B, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x4C, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x4D, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x4E, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x4F, */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_ETSI9,	TXPWR_LMT_ETSI),	/* 0x50, RTW_CHPLAN_WORLD_ETSI9 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_ETSI10,	TXPWR_LMT_ETSI),	/* 0x51, RTW_CHPLAN_WORLD_ETSI10 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_ETSI11,	TXPWR_LMT_ETSI),	/* 0x52, RTW_CHPLAN_WORLD_ETSI11 */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_NCC4,		TXPWR_LMT_FCC),		/* 0x53, RTW_CHPLAN_FCC1_NCC4 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_ETSI12,	TXPWR_LMT_ETSI),	/* 0x54, RTW_CHPLAN_WORLD_ETSI12 */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_FCC9,		TXPWR_LMT_FCC),		/* 0x55, RTW_CHPLAN_FCC1_FCC9 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_ETSI13,	TXPWR_LMT_ETSI),	/* 0x56, RTW_CHPLAN_WORLD_ETSI13 */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_FCC10,	TXPWR_LMT_FCC),		/* 0x57, RTW_CHPLAN_FCC1_FCC10 */
	CHPLAN_ENT(RTW_RD_2G_MKK2,		RTW_RD_5G_MKK4,		TXPWR_LMT_MKK),		/* 0x58, RTW_CHPLAN_MKK2_MKK4 */
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_ETSI14,	TXPWR_LMT_ETSI),	/* 0x59, RTW_CHPLAN_WORLD_ETSI14 */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x5A, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x5B, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x5C, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x5D, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x5E, */
	CHPLAN_ENT(RTW_RD_2G_NULL,		RTW_RD_5G_NULL,		TXPWR_LMT_WW),		/* 0x5F, */
	CHPLAN_ENT(RTW_RD_2G_FCC1,		RTW_RD_5G_FCC5,		TXPWR_LMT_FCC),		/* 0x60, RTW_CHPLAN_FCC1_FCC5 */
	CHPLAN_ENT(RTW_RD_2G_FCC2,		RTW_RD_5G_FCC7,		TXPWR_LMT_FCC),		/* 0x61, RTW_CHPLAN_FCC2_FCC7 */
	CHPLAN_ENT(RTW_RD_2G_FCC2,		RTW_RD_5G_FCC1,		TXPWR_LMT_FCC),		/* 0x62, RTW_CHPLAN_FCC2_FCC1 */
};

static RT_CHANNEL_PLAN_MAP RTW_CHANNEL_PLAN_MAP_REALTEK_DEFINE =
	CHPLAN_ENT(RTW_RD_2G_WORLD,		RTW_RD_5G_FCC1,		TXPWR_LMT_FCC);		/* 0x7F, Realtek Define */

bool rtw_chplan_is_empty(u8 id)
{
	RT_CHANNEL_PLAN_MAP *chplan_map;

	if (id == RTW_CHPLAN_REALTEK_DEFINE)
		chplan_map = &RTW_CHANNEL_PLAN_MAP_REALTEK_DEFINE;
	else
		chplan_map = &RTW_ChannelPlanMap[id];

	if (chplan_map->Index2G == RTW_RD_2G_NULL
		#ifdef CONFIG_IEEE80211_BAND_5GHZ
		&& chplan_map->Index5G == RTW_RD_5G_NULL
		#endif
	)
		return true;

	return false;
}

void rtw_rfctl_init(_adapter *adapter)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);

	memset(rfctl, 0, sizeof(*rfctl));

#ifdef CONFIG_DFS_MASTER
	rfctl->cac_start_time = rfctl->cac_end_time = RTW_CAC_STOPPED;

	/* TODO: dfs_master_timer */
#endif
}

#ifdef CONFIG_DFS_MASTER
/*
* called in rtw_dfs_master_enable()
* assume the request channel coverage is DFS range
* base on the current status and the request channel coverage to check if need to reset complete CAC time
*/
bool rtw_is_cac_reset_needed(_adapter *adapter, u8 ch, u8 bw, u8 offset)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	bool needed = false;
	u32 cur_hi, cur_lo, hi, lo;

	if (rfctl->radar_detected == 1) {
		needed = true;
		goto exit;
	}

	if (rfctl->radar_detect_ch == 0) {
		needed = true;
		goto exit;
	}

	if (rtw_chbw_to_freq_range(ch, bw, offset, &hi, &lo) == false) {
		RTW_ERR("request detection range ch:%u, bw:%u, offset:%u\n", ch, bw, offset);
		rtw_warn_on(1);
	}

	if (rtw_chbw_to_freq_range(rfctl->radar_detect_ch, rfctl->radar_detect_bw, rfctl->radar_detect_offset, &cur_hi, &cur_lo) == false) {
		RTW_ERR("cur detection range ch:%u, bw:%u, offset:%u\n", rfctl->radar_detect_ch, rfctl->radar_detect_bw, rfctl->radar_detect_offset);
		rtw_warn_on(1);
	}

	if (hi <= lo || cur_hi <= cur_lo) {
		RTW_ERR("hi:%u, lo:%u, cur_hi:%u, cur_lo:%u\n", hi, lo, cur_hi, cur_lo);
		rtw_warn_on(1);
	}

	if (rtw_is_range_a_in_b(hi, lo, cur_hi, cur_lo)) {
		/* request is in current detect range */
		goto exit;
	}

	/* check if request channel coverage has new range and the new range is in DFS range */
	if (!rtw_is_range_overlap(hi, lo, cur_hi, cur_lo)) {
		/* request has no overlap with current */
		needed = true;
	} else if (rtw_is_range_a_in_b(cur_hi, cur_lo, hi, lo)) {
		/* request is supper set of current */
		if ((hi != cur_hi && rtw_is_dfs_range(hi, cur_hi)) || (lo != cur_lo && rtw_is_dfs_range(cur_lo, lo)))
			needed = true;
	} else {
		/* request is not supper set of current, but has overlap */
		if ((lo < cur_lo && rtw_is_dfs_range(cur_lo, lo)) || (hi > cur_hi && rtw_is_dfs_range(hi, cur_hi)))
			needed = true;
	}

exit:
	return needed;
}

bool _rtw_rfctl_overlap_radar_detect_ch(struct rf_ctl_t *rfctl, u8 ch, u8 bw, u8 offset)
{
	bool ret = false;
	u32 hi = 0, lo = 0;
	u32 r_hi = 0, r_lo = 0;
	int i;

	if (rfctl->radar_detect_by_others)
		goto exit;

	if (rfctl->radar_detect_ch == 0)
		goto exit;

	if (rtw_chbw_to_freq_range(ch, bw, offset, &hi, &lo) == false) {
		rtw_warn_on(1);
		goto exit;
	}

	if (rtw_chbw_to_freq_range(rfctl->radar_detect_ch
			, rfctl->radar_detect_bw, rfctl->radar_detect_offset
			, &r_hi, &r_lo) == false) {
		rtw_warn_on(1);
		goto exit;
	}

	if (rtw_is_range_overlap(hi, lo, r_hi, r_lo))
		ret = true;

exit:
	return ret;
}

bool rtw_rfctl_overlap_radar_detect_ch(struct rf_ctl_t *rfctl)
{
	return _rtw_rfctl_overlap_radar_detect_ch(rfctl
				, rfctl_to_dvobj(rfctl)->oper_channel
				, rfctl_to_dvobj(rfctl)->oper_bwmode
				, rfctl_to_dvobj(rfctl)->oper_ch_offset);
}

bool rtw_rfctl_is_tx_blocked_by_ch_waiting(struct rf_ctl_t *rfctl)
{
	return rtw_rfctl_overlap_radar_detect_ch(rfctl) && IS_CH_WAITING(rfctl);
}

bool rtw_chset_is_ch_non_ocp(RT_CHANNEL_INFO *ch_set, u8 ch, u8 bw, u8 offset)
{
	bool ret = false;
	u32 hi = 0, lo = 0;
	int i;

	if (rtw_chbw_to_freq_range(ch, bw, offset, &hi, &lo) == false)
		goto exit;

	for (i = 0; ch_set[i].ChannelNum != 0; i++) {
		if (!rtw_ch2freq(ch_set[i].ChannelNum)) {
			rtw_warn_on(1);
			continue;
		}

		if (!CH_IS_NON_OCP(&ch_set[i]))
			continue;

		if (lo <= rtw_ch2freq(ch_set[i].ChannelNum)
			&& rtw_ch2freq(ch_set[i].ChannelNum) <= hi
		) {
			ret = true;
			break;
		}
	}

exit:
	return ret;
}

u32 rtw_chset_get_ch_non_ocp_ms(RT_CHANNEL_INFO *ch_set, u8 ch, u8 bw, u8 offset)
{
	int ms = 0;
	u32 current_time;
	u32 hi = 0, lo = 0;
	int i;

	if (rtw_chbw_to_freq_range(ch, bw, offset, &hi, &lo) == false)
		goto exit;

	current_time = jiffies;

	for (i = 0; ch_set[i].ChannelNum != 0; i++) {
		if (!rtw_ch2freq(ch_set[i].ChannelNum)) {
			rtw_warn_on(1);
			continue;
		}

		if (!CH_IS_NON_OCP(&ch_set[i]))
			continue;

		if (lo <= rtw_ch2freq(ch_set[i].ChannelNum)
			&& rtw_ch2freq(ch_set[i].ChannelNum) <= hi
		) {
			if (rtw_systime_to_ms(ch_set[i].non_ocp_end_time - current_time) > ms)
				ms = rtw_systime_to_ms(ch_set[i].non_ocp_end_time - current_time);
		}
	}

exit:
	return ms;
}

/**
 * rtw_chset_update_non_ocp - update non_ocp_end_time according to the given @ch, @bw, @offset into @ch_set
 * @ch_set: the given channel set
 * @ch: channel number on which radar is detected
 * @bw: bandwidth on which radar is detected
 * @offset: bandwidth offset on which radar is detected
 * @ms: ms to add from now to update non_ocp_end_time, ms < 0 means use NON_OCP_TIME_MS
 */
static void _rtw_chset_update_non_ocp(RT_CHANNEL_INFO *ch_set, u8 ch, u8 bw, u8 offset, int ms)
{
	u32 hi = 0, lo = 0;
	int i;

	if (rtw_chbw_to_freq_range(ch, bw, offset, &hi, &lo) == false)
		goto exit;

	for (i = 0; ch_set[i].ChannelNum != 0; i++) {
		if (!rtw_ch2freq(ch_set[i].ChannelNum)) {
			rtw_warn_on(1);
			continue;
		}

		if (lo <= rtw_ch2freq(ch_set[i].ChannelNum)
			&& rtw_ch2freq(ch_set[i].ChannelNum) <= hi
		) {
			if (ms >= 0)
				ch_set[i].non_ocp_end_time = jiffies + rtw_ms_to_systime(ms);
			else
				ch_set[i].non_ocp_end_time = jiffies + rtw_ms_to_systime(NON_OCP_TIME_MS);
		}
	}

exit:
	return;
}

inline void rtw_chset_update_non_ocp(RT_CHANNEL_INFO *ch_set, u8 ch, u8 bw, u8 offset)
{
	_rtw_chset_update_non_ocp(ch_set, ch, bw, offset, -1);
}

inline void rtw_chset_update_non_ocp_ms(RT_CHANNEL_INFO *ch_set, u8 ch, u8 bw, u8 offset, int ms)
{
	_rtw_chset_update_non_ocp(ch_set, ch, bw, offset, ms);
}

u32 rtw_get_ch_waiting_ms(_adapter *adapter, u8 ch, u8 bw, u8 offset, u32 *r_non_ocp_ms, u32 *r_cac_ms)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	struct mlme_ext_priv *mlmeext = &adapter->mlmeextpriv;
	u32 non_ocp_ms;
	u32 cac_ms;
	u8 in_rd_range = 0; /* if in current radar detection range*/

	if (rtw_chset_is_ch_non_ocp(mlmeext->channel_set, ch, bw, offset))
		non_ocp_ms = rtw_chset_get_ch_non_ocp_ms(mlmeext->channel_set, ch, bw, offset);
	else
		non_ocp_ms = 0;

	if (rfctl->dfs_master_enabled) {
		u32 cur_hi, cur_lo, hi, lo;

		if (rtw_chbw_to_freq_range(ch, bw, offset, &hi, &lo) == false) {
			RTW_ERR("input range ch:%u, bw:%u, offset:%u\n", ch, bw, offset);
			rtw_warn_on(1);
		}

		if (rtw_chbw_to_freq_range(rfctl->radar_detect_ch, rfctl->radar_detect_bw, rfctl->radar_detect_offset, &cur_hi, &cur_lo) == false) {
			RTW_ERR("cur detection range ch:%u, bw:%u, offset:%u\n", rfctl->radar_detect_ch, rfctl->radar_detect_bw, rfctl->radar_detect_offset);
			rtw_warn_on(1);
		}

		if (rtw_is_range_a_in_b(hi, lo, cur_hi, cur_lo))
			in_rd_range = 1;
	}

	if (!rtw_is_dfs_ch(ch, bw, offset))
		cac_ms = 0;
	else if (in_rd_range && !non_ocp_ms) {
		if (IS_CH_WAITING(rfctl))
			cac_ms = rtw_systime_to_ms(rfctl->cac_end_time - jiffies);
		else
			cac_ms = 0;
	} else if (rtw_is_long_cac_ch(ch, bw, offset, rtw_odm_get_dfs_domain(adapter)))
		cac_ms = CAC_TIME_CE_MS;
	else
		cac_ms = CAC_TIME_MS;

	if (r_non_ocp_ms)
		*r_non_ocp_ms = non_ocp_ms;
	if (r_cac_ms)
		*r_cac_ms = cac_ms;

	return non_ocp_ms + cac_ms;
}

void rtw_reset_cac(_adapter *adapter, u8 ch, u8 bw, u8 offset)
{
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	u32 non_ocp_ms;
	u32 cac_ms;

	rtw_get_ch_waiting_ms(adapter
		, ch
		, bw
		, offset
		, &non_ocp_ms
		, &cac_ms
	);

	rfctl->cac_start_time = jiffies + rtw_ms_to_systime(non_ocp_ms);
	rfctl->cac_end_time = rfctl->cac_start_time + rtw_ms_to_systime(cac_ms);

	/* skip special value */
	if (rfctl->cac_start_time == RTW_CAC_STOPPED) {
		rfctl->cac_start_time++;
		rfctl->cac_end_time++;
	}
	if (rfctl->cac_end_time == RTW_CAC_STOPPED)
		rfctl->cac_end_time++;
}
#endif /* CONFIG_DFS_MASTER */

/* choose channel with shortest waiting (non ocp + cac) time */
bool rtw_choose_shortest_waiting_ch(_adapter *adapter, u8 req_bw, u8 *dec_ch, u8 *dec_bw, u8 *dec_offset, u8 d_flags)
{
#ifndef DBG_CHOOSE_SHORTEST_WAITING_CH
#define DBG_CHOOSE_SHORTEST_WAITING_CH 0
#endif

	struct mlme_ext_priv *mlmeext = &adapter->mlmeextpriv;
	struct rf_ctl_t *rfctl = adapter_to_rfctl(adapter);
	struct registry_priv *regsty = adapter_to_regsty(adapter);
	u8 ch, bw, offset;
	u8 ch_c = 0, bw_c = 0, offset_c = 0;
	int i;
	u32 min_waiting_ms = 0;

	if (!dec_ch || !dec_bw || !dec_offset) {
		rtw_warn_on(1);
		return false;
	}

	/* full search and narrow bw judegement first to avoid potetial judegement timing issue */
	for (bw = CHANNEL_WIDTH_20; bw <= req_bw; bw++) {
		if (!hal_is_bw_support(adapter, bw))
			continue;

		for (i = 0; i < mlmeext->max_chan_nums; i++) {
			u32 non_ocp_ms = 0;
			u32 cac_ms = 0;
			u32 waiting_ms = 0;

			ch = mlmeext->channel_set[i].ChannelNum;

			if ((d_flags & RTW_CHF_2G) && ch <= 14)
				continue;

			if ((d_flags & RTW_CHF_5G) && ch > 14)
				continue;

			if (ch > 14) {
				if (bw > REGSTY_BW_5G(regsty))
					continue;
			} else {
				if (bw > REGSTY_BW_2G(regsty))
					continue;
			}

			if (!rtw_get_offset_by_chbw(ch, bw, &offset))
				continue;

			if (!rtw_chset_is_chbw_valid(mlmeext->channel_set, ch, bw, offset))
				continue;

			if ((d_flags & RTW_CHF_NON_OCP) && rtw_chset_is_ch_non_ocp(mlmeext->channel_set, ch, bw, offset))
				continue;

			if ((d_flags & RTW_CHF_DFS) && rtw_is_dfs_ch(ch, bw, offset))
				continue;

			if ((d_flags & RTW_CHF_LONG_CAC) && rtw_is_long_cac_ch(ch, bw, offset, rtw_odm_get_dfs_domain(adapter)))
				continue;

			if ((d_flags & RTW_CHF_NON_DFS) && !rtw_is_dfs_ch(ch, bw, offset))
				continue;

			if ((d_flags & RTW_CHF_NON_LONG_CAC) && !rtw_is_long_cac_ch(ch, bw, offset, rtw_odm_get_dfs_domain(adapter)))
				continue;

			#ifdef CONFIG_DFS_MASTER
			waiting_ms = rtw_get_ch_waiting_ms(adapter, ch, bw, offset, &non_ocp_ms, &cac_ms);
			#endif

			if (DBG_CHOOSE_SHORTEST_WAITING_CH)
				RTW_INFO(FUNC_ADPT_FMT":%u,%u,%u %u(non_ocp:%u, cac:%u)\n"
					, FUNC_ADPT_ARG(adapter), ch, bw, offset, waiting_ms, non_ocp_ms, cac_ms);

			if (ch_c == 0
				|| min_waiting_ms > waiting_ms
				|| (min_waiting_ms == waiting_ms && bw > bw_c) /* wider bw first */
			) {
				ch_c = ch;
				bw_c = bw;
				offset_c = offset;
				min_waiting_ms = waiting_ms;
			}
		}
	}

	if (ch_c != 0) {
		RTW_INFO(FUNC_ADPT_FMT": d_flags:0x%02x %u,%u,%u waiting_ms:%u\n"
			, FUNC_ADPT_ARG(adapter), d_flags, ch_c, bw_c, offset_c, min_waiting_ms);

		*dec_ch = ch_c;
		*dec_bw = bw_c;
		*dec_offset = offset_c;
		return true;
	}

	if (d_flags == 0)
		rtw_warn_on(1);

	return false;
}

void dump_country_chplan(void *sel, const struct country_chplan *ent)
{
	_RTW_PRINT_SEL(sel, "\"%c%c\", 0x%02X%s\n"
		, ent->alpha2[0], ent->alpha2[1], ent->chplan
		, COUNTRY_CHPLAN_EN_11AC(ent) ? " ac" : ""
	);
}

void dump_country_chplan_map(void *sel)
{
	const struct country_chplan *ent;
	u8 code[2];

#if RTW_DEF_MODULE_REGULATORY_CERT
	_RTW_PRINT_SEL(sel, "RTW_DEF_MODULE_REGULATORY_CERT:0x%x\n", RTW_DEF_MODULE_REGULATORY_CERT);
#endif
#ifdef CONFIG_CUSTOMIZED_COUNTRY_CHPLAN_MAP
	_RTW_PRINT_SEL(sel, "CONFIG_CUSTOMIZED_COUNTRY_CHPLAN_MAP\n");
#endif

	for (code[0] = 'A'; code[0] <= 'Z'; code[0]++) {
		for (code[1] = 'A'; code[1] <= 'Z'; code[1]++) {
			ent = rtw_get_chplan_from_country(code);
			if (!ent)
				continue;

			dump_country_chplan(sel, ent);
		}
	}
}

void dump_chplan_id_list(void *sel)
{
	int i;

	for (i = 0; i < RTW_CHPLAN_MAX; i++) {
		if (!rtw_is_channel_plan_valid(i))
			continue;

		_RTW_PRINT_SEL(sel, "0x%02X ", i);
	}

	RTW_PRINT_SEL(sel, "0x7F\n");
}

void dump_chplan_test(void *sel)
{
	int i, j;

	/* check invalid channel */
	for (i = 0; i < RTW_RD_2G_MAX; i++) {
		for (j = 0; j < CH_LIST_LEN(RTW_ChannelPlan2G[i]); j++) {
			if (rtw_ch2freq(CH_LIST_CH(RTW_ChannelPlan2G[i], j)) == 0)
				RTW_PRINT_SEL(sel, "invalid ch:%u at (%d,%d)\n", CH_LIST_CH(RTW_ChannelPlan2G[i], j), i, j);
		}
	}

#ifdef CONFIG_IEEE80211_BAND_5GHZ
	for (i = 0; i < RTW_RD_5G_MAX; i++) {
		for (j = 0; j < CH_LIST_LEN(RTW_ChannelPlan5G[i]); j++) {
			if (rtw_ch2freq(CH_LIST_CH(RTW_ChannelPlan5G[i], j)) == 0)
				RTW_PRINT_SEL(sel, "invalid ch:%u at (%d,%d)\n", CH_LIST_CH(RTW_ChannelPlan5G[i], j), i, j);
		}
	}
#endif
}

void dump_chset(void *sel, RT_CHANNEL_INFO *ch_set)
{
	u8	i;

	for (i = 0; ch_set[i].ChannelNum != 0; i++) {
		RTW_PRINT_SEL(sel, "ch:%3u, freq:%u, scan_type:%d"
			, ch_set[i].ChannelNum, rtw_ch2freq(ch_set[i].ChannelNum), ch_set[i].ScanType);

#ifdef CONFIG_FIND_BEST_CHANNEL
		_RTW_PRINT_SEL(sel, ", rx_count:%u", ch_set[i].rx_count);
#endif

#ifdef CONFIG_DFS_MASTER
		if (rtw_is_dfs_ch(ch_set[i].ChannelNum, CHANNEL_WIDTH_20, HAL_PRIME_CHNL_OFFSET_DONT_CARE)) {
			if (CH_IS_NON_OCP(&ch_set[i]))
				_RTW_PRINT_SEL(sel, ", non_ocp:%d"
					, rtw_systime_to_ms(ch_set[i].non_ocp_end_time - jiffies));
			else
				_RTW_PRINT_SEL(sel, ", non_ocp:N/A");
		}
#endif

		_RTW_PRINT_SEL(sel, "\n");
	}

	RTW_PRINT_SEL(sel, "total ch number:%d\n", i);
}

void dump_cur_chset(void *sel, _adapter *adapter)
{
	struct mlme_priv *mlme = &adapter->mlmepriv;
	struct mlme_ext_priv *mlmeext = &adapter->mlmeextpriv;
	struct registry_priv *regsty = adapter_to_regsty(adapter);
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(adapter);
	int i;

	if (mlme->country_ent)
		dump_country_chplan(sel, mlme->country_ent);
	else
		RTW_PRINT_SEL(sel, "chplan:0x%02X\n", mlme->ChannelPlan);

	RTW_PRINT_SEL(sel, "2G_PLS:%u, 5G_PLS:%u\n"
		, hal_data->Regulation2_4G, hal_data->Regulation5G);

#ifdef CONFIG_DFS_MASTER
	RTW_PRINT_SEL(sel, "dfs_domain:%u\n", rtw_odm_get_dfs_domain(adapter));
#endif

	for (i = 0; i < MAX_CHANNEL_NUM; i++)
		if (regsty->excl_chs[i] != 0)
			break;

	if (i < MAX_CHANNEL_NUM) {
		_RTW_PRINT_SEL(sel, "excl_chs:");
		for (i = 0; i < MAX_CHANNEL_NUM; i++) {
			if (regsty->excl_chs[i] == 0)
				break;
			_RTW_PRINT_SEL(sel, "%u ", regsty->excl_chs[i]);
		}
		RTW_PRINT_SEL(sel, "\n");
	}

	dump_chset(sel, mlmeext->channel_set);
}

/*
 * Search the @param ch in given @param ch_set
 * @ch_set: the given channel set
 * @ch: the given channel number
 *
 * return the index of channel_num in channel_set, -1 if not found
 */
int rtw_ch_set_search_ch(RT_CHANNEL_INFO *ch_set, const u32 ch)
{
	int i;
	for (i = 0; ch_set[i].ChannelNum != 0; i++) {
		if (ch == ch_set[i].ChannelNum)
			break;
	}

	if (i >= ch_set[i].ChannelNum)
		return -1;
	return i;
}

/*
 * Check if the @param ch, bw, offset is valid for the given @param ch_set
 * @ch_set: the given channel set
 * @ch: the given channel number
 * @bw: the given bandwidth
 * @offset: the given channel offset
 *
 * return valid (1) or not (0)
 */
u8 rtw_chset_is_chbw_valid(RT_CHANNEL_INFO *ch_set, u8 ch, u8 bw, u8 offset)
{
	u8 cch;
	u8 *op_chs;
	u8 op_ch_num;
	u8 valid = 0;
	int i;

	cch = rtw_get_center_ch(ch, bw, offset);

	if (!rtw_get_op_chs_by_cch_bw(cch, bw, &op_chs, &op_ch_num))
		goto exit;

	for (i = 0; i < op_ch_num; i++) {
		if (0)
			RTW_INFO("%u,%u,%u - cch:%u, bw:%u, op_ch:%u\n", ch, bw, offset, cch, bw, *(op_chs + i));
		if (rtw_ch_set_search_ch(ch_set, *(op_chs + i)) == -1)
			break;
	}

	if (op_ch_num != 0 && i == op_ch_num)
		valid = 1;

exit:
	return valid;
}

/*
 * Check the @param ch is fit with setband setting of @param adapter
 * @adapter: the given adapter
 * @ch: the given channel number
 *
 * return true when check valid, false not valid
 */
bool rtw_mlme_band_check(_adapter *adapter, const u32 ch)
{
	if (adapter->setband == WIFI_FREQUENCY_BAND_AUTO /* 2.4G and 5G */
		|| (adapter->setband == WIFI_FREQUENCY_BAND_2GHZ && ch < 35) /* 2.4G only */
		|| (adapter->setband == WIFI_FREQUENCY_BAND_5GHZ && ch > 35) /* 5G only */
	)
		return true;
	return false;
}
inline void RTW_SET_SCAN_BAND_SKIP(_adapter *padapter, int skip_band)
{
	int bs = ATOMIC_READ(&padapter->bandskip);

	bs |= skip_band;
	ATOMIC_SET(&padapter->bandskip, bs);
}

inline void RTW_CLR_SCAN_BAND_SKIP(_adapter *padapter, int skip_band)
{
	int bs = ATOMIC_READ(&padapter->bandskip);

	bs &= ~(skip_band);
	ATOMIC_SET(&padapter->bandskip, bs);
}
inline int RTW_GET_SCAN_BAND_SKIP(_adapter *padapter)
{
	return ATOMIC_READ(&padapter->bandskip);
}

#define RTW_IS_SCAN_BAND_SKIP(padapter, skip_band) (ATOMIC_READ(&padapter->bandskip) & (skip_band))

bool rtw_mlme_ignore_chan(_adapter *adapter, const u32 ch)
{
	if (RTW_IS_SCAN_BAND_SKIP(adapter, BAND_24G) && ch < 35) /* SKIP 2.4G Band channel */
		return true;
	if (RTW_IS_SCAN_BAND_SKIP(adapter, BAND_5G)  && ch > 35) /* SKIP 5G Band channel */
		return true;

	return false;
}


/****************************************************************************

Following are the initialization functions for WiFi MLME

*****************************************************************************/

int init_hw_mlme_ext(_adapter *padapter)
{
	struct	mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	/* set_opmode_cmd(padapter, infra_client_with_mlme); */ /* removed */

	set_channel_bwmode(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode);

	return _SUCCESS;
}

void init_mlme_default_rate_set(_adapter *padapter)
{
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	unsigned char	mixed_datarate[NumRates] = {_1M_RATE_, _2M_RATE_, _5M_RATE_, _11M_RATE_, _6M_RATE_, _9M_RATE_, _12M_RATE_, _18M_RATE_, _24M_RATE_, _36M_RATE_, _48M_RATE_, _54M_RATE_, 0xff};
	unsigned char	mixed_basicrate[NumRates] = {_1M_RATE_, _2M_RATE_, _5M_RATE_, _11M_RATE_, _6M_RATE_, _12M_RATE_, _24M_RATE_, 0xff,};
	unsigned char	supported_mcs_set[16] = {0xff, 0xff, 0xff, 0x00, 0x00, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0};

	memcpy(pmlmeext->datarate, mixed_datarate, NumRates);
	memcpy(pmlmeext->basicrate, mixed_basicrate, NumRates);

	memcpy(pmlmeext->default_supported_mcs_set, supported_mcs_set, sizeof(pmlmeext->default_supported_mcs_set));
}

static void init_mlme_ext_priv_value(_adapter *padapter)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	ATOMIC_SET(&pmlmeext->event_seq, 0);
	pmlmeext->mgnt_seq = 0;/* reset to zero when disconnect at client mode */
#ifdef CONFIG_IEEE80211W
	pmlmeext->sa_query_seq = 0;
	pmlmeext->mgnt_80211w_IPN = 0;
	pmlmeext->mgnt_80211w_IPN_rx = 0;
#endif /* CONFIG_IEEE80211W */
	pmlmeext->cur_channel = padapter->registrypriv.channel;
	pmlmeext->cur_bwmode = CHANNEL_WIDTH_20;
	pmlmeext->cur_ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;

	pmlmeext->retry = 0;

	pmlmeext->cur_wireless_mode = padapter->registrypriv.wireless_mode;

	init_mlme_default_rate_set(padapter);

	if (pmlmeext->cur_channel > 14)
		pmlmeext->tx_rate = IEEE80211_OFDM_RATE_6MB;
	else
		pmlmeext->tx_rate = IEEE80211_CCK_RATE_1MB;

	mlmeext_set_scan_state(pmlmeext, SCAN_DISABLE);
	pmlmeext->sitesurvey_res.channel_idx = 0;
	pmlmeext->sitesurvey_res.bss_cnt = 0;
	pmlmeext->sitesurvey_res.scan_ch_ms = SURVEY_TO;
	pmlmeext->sitesurvey_res.rx_ampdu_accept = RX_AMPDU_ACCEPT_INVALID;
	pmlmeext->sitesurvey_res.rx_ampdu_size = RX_AMPDU_SIZE_INVALID;
#ifdef CONFIG_SCAN_BACKOP
	mlmeext_assign_scan_backop_flags_sta(pmlmeext, /*SS_BACKOP_EN|*/SS_BACKOP_PS_ANNC | SS_BACKOP_TX_RESUME);
	mlmeext_assign_scan_backop_flags_ap(pmlmeext, SS_BACKOP_EN | SS_BACKOP_PS_ANNC | SS_BACKOP_TX_RESUME);
	pmlmeext->sitesurvey_res.scan_cnt = 0;
	pmlmeext->sitesurvey_res.scan_cnt_max = RTW_SCAN_NUM_OF_CH;
	pmlmeext->sitesurvey_res.backop_ms = RTW_BACK_OP_CH_MS;
#endif
#if defined(CONFIG_ANTENNA_DIVERSITY) || defined(DBG_SCAN_SW_ANTDIV_BL)
	pmlmeext->sitesurvey_res.is_sw_antdiv_bl_scan = 0;
#endif
	pmlmeext->scan_abort = false;

	pmlmeinfo->state = WIFI_FW_NULL_STATE;
	pmlmeinfo->reauth_count = 0;
	pmlmeinfo->reassoc_count = 0;
	pmlmeinfo->link_count = 0;
	pmlmeinfo->auth_seq = 0;
	pmlmeinfo->auth_algo = dot11AuthAlgrthm_Open;
	pmlmeinfo->key_index = 0;
	pmlmeinfo->iv = 0;

	pmlmeinfo->enc_algo = _NO_PRIVACY_;
	pmlmeinfo->authModeToggle = 0;

	memset(pmlmeinfo->chg_txt, 0, 128);

	pmlmeinfo->slotTime = SHORT_SLOT_TIME;
	pmlmeinfo->preamble_mode = PREAMBLE_AUTO;

	pmlmeinfo->dialogToken = 0;

	pmlmeext->action_public_rxseq = 0xffff;
	pmlmeext->action_public_dialog_token = 0xff;
}

static int has_channel(RT_CHANNEL_INFO *channel_set,
		       u8 chanset_size,
		       u8 chan)
{
	int i;

	for (i = 0; i < chanset_size; i++) {
		if (channel_set[i].ChannelNum == chan)
			return 1;
	}

	return 0;
}

static void init_channel_list(_adapter *padapter, RT_CHANNEL_INFO *channel_set,
			      u8 chanset_size,
			      struct p2p_channels *channel_list)
{
	struct registry_priv *regsty = adapter_to_regsty(padapter);

	struct p2p_oper_class_map op_class[] = {
		{ IEEE80211G,  81,   1,  13,  1, BW20 },
		{ IEEE80211G,  82,  14,  14,  1, BW20 },
		{ IEEE80211A, 115,  36,  48,  4, BW20 },
		{ IEEE80211A, 116,  36,  44,  8, BW40PLUS },
		{ IEEE80211A, 117,  40,  48,  8, BW40MINUS },
		{ IEEE80211A, 124, 149, 161,  4, BW20 },
		{ IEEE80211A, 125, 149, 169,  4, BW20 },
		{ IEEE80211A, 126, 149, 157,  8, BW40PLUS },
		{ IEEE80211A, 127, 153, 161,  8, BW40MINUS },
		{ -1, 0, 0, 0, 0, BW20 }
	};

	int cla, op;

	cla = 0;

	for (op = 0; op_class[op].op_class; op++) {
		u8 ch;
		struct p2p_oper_class_map *o = &op_class[op];
		struct p2p_reg_class *reg = NULL;

		for (ch = o->min_chan; ch <= o->max_chan; ch += o->inc) {
			if (!has_channel(channel_set, chanset_size, ch))
				continue;

			if ((0 == padapter->registrypriv.ht_enable) && (8 == o->inc))
				continue;

			if ((REGSTY_IS_BW_5G_SUPPORT(regsty, CHANNEL_WIDTH_40)) &&
			    ((BW40MINUS == o->bw) || (BW40PLUS == o->bw)))
				continue;

			if (reg == NULL) {
				reg = &channel_list->reg_class[cla];
				cla++;
				reg->reg_class = o->op_class;
				reg->channels = 0;
			}
			reg->channel[reg->channels] = ch;
			reg->channels++;
		}
	}
	channel_list->reg_classes = cla;

}

static bool rtw_regsty_is_excl_chs(struct registry_priv *regsty, u8 ch)
{
	int i;

	for (i = 0; i < MAX_CHANNEL_NUM; i++) {
		if (regsty->excl_chs[i] == 0)
			break;
		if (regsty->excl_chs[i] == ch)
			return true;
	}
	return false;
}

static u8 init_channel_set(_adapter *padapter, u8 ChannelPlan, RT_CHANNEL_INFO *channel_set)
{
	struct registry_priv *regsty = adapter_to_regsty(padapter);
	u8	index, chanset_size = 0;
	u8	b5GBand = false, b2_4GBand = false;
	u8	Index2G = 0, Index5G = 0;
	HAL_DATA_TYPE *hal_data = GET_HAL_DATA(padapter);
	int i;

	if (!rtw_is_channel_plan_valid(ChannelPlan)) {
		RTW_ERR("ChannelPlan ID 0x%02X error !!!!!\n", ChannelPlan);
		return chanset_size;
	}

	memset(channel_set, 0, sizeof(RT_CHANNEL_INFO) * MAX_CHANNEL_NUM);

	if (IsSupported24G(padapter->registrypriv.wireless_mode))
		b2_4GBand = true;

	if (is_supported_5g(padapter->registrypriv.wireless_mode))
		b5GBand = true;

	if (b2_4GBand) {
		if (RTW_CHPLAN_REALTEK_DEFINE == ChannelPlan)
			Index2G = RTW_CHANNEL_PLAN_MAP_REALTEK_DEFINE.Index2G;
		else
			Index2G = RTW_ChannelPlanMap[ChannelPlan].Index2G;

		for (index = 0; index < CH_LIST_LEN(RTW_ChannelPlan2G[Index2G]); index++) {
			if (rtw_regsty_is_excl_chs(regsty, CH_LIST_CH(RTW_ChannelPlan2G[Index2G], index)))
				continue;

			channel_set[chanset_size].ChannelNum = CH_LIST_CH(RTW_ChannelPlan2G[Index2G], index);

			if (RTW_CHPLAN_GLOBAL_DOAMIN == ChannelPlan
				|| RTW_CHPLAN_GLOBAL_NULL == ChannelPlan
			) {
				/* Channel 1~11 is active, and 12~14 is passive */
				if (channel_set[chanset_size].ChannelNum >= 1 && channel_set[chanset_size].ChannelNum <= 11)
					channel_set[chanset_size].ScanType = SCAN_ACTIVE;
				else if ((channel_set[chanset_size].ChannelNum  >= 12 && channel_set[chanset_size].ChannelNum  <= 14))
					channel_set[chanset_size].ScanType  = SCAN_PASSIVE;
			} else if (RTW_CHPLAN_WORLD_WIDE_13 == ChannelPlan
				|| RTW_CHPLAN_WORLD_WIDE_5G == ChannelPlan
				|| RTW_RD_2G_WORLD == Index2G
			) {
				/* channel 12~13, passive scan */
				if (channel_set[chanset_size].ChannelNum <= 11)
					channel_set[chanset_size].ScanType = SCAN_ACTIVE;
				else
					channel_set[chanset_size].ScanType = SCAN_PASSIVE;
			} else
				channel_set[chanset_size].ScanType = SCAN_ACTIVE;

			chanset_size++;
		}
	}

#ifdef CONFIG_IEEE80211_BAND_5GHZ
	if (b5GBand) {
		if (RTW_CHPLAN_REALTEK_DEFINE == ChannelPlan)
			Index5G = RTW_CHANNEL_PLAN_MAP_REALTEK_DEFINE.Index5G;
		else
			Index5G = RTW_ChannelPlanMap[ChannelPlan].Index5G;

		for (index = 0; index < CH_LIST_LEN(RTW_ChannelPlan5G[Index5G]); index++) {
			if (rtw_regsty_is_excl_chs(regsty, CH_LIST_CH(RTW_ChannelPlan5G[Index5G], index)))
				continue;
#ifdef CONFIG_DFS
			channel_set[chanset_size].ChannelNum = CH_LIST_CH(RTW_ChannelPlan5G[Index5G], index);
			if (channel_set[chanset_size].ChannelNum <= 48
				|| channel_set[chanset_size].ChannelNum >= 149
			) {
				if (RTW_CHPLAN_WORLD_WIDE_5G == ChannelPlan) /* passive scan for all 5G channels */
					channel_set[chanset_size].ScanType = SCAN_PASSIVE;
				else
					channel_set[chanset_size].ScanType = SCAN_ACTIVE;
			} else
				channel_set[chanset_size].ScanType = SCAN_PASSIVE;
			chanset_size++;
#else /* CONFIG_DFS */
			if (CH_LIST_CH(RTW_ChannelPlan5G[Index5G], index) <= 48
				|| CH_LIST_CH(RTW_ChannelPlan5G[Index5G], index) >= 149
			) {
				channel_set[chanset_size].ChannelNum = CH_LIST_CH(RTW_ChannelPlan5G[Index5G], index);
				if (RTW_CHPLAN_WORLD_WIDE_5G == ChannelPlan) /* passive scan for all 5G channels */
					channel_set[chanset_size].ScanType = SCAN_PASSIVE;
				else
					channel_set[chanset_size].ScanType = SCAN_ACTIVE;
				chanset_size++;
			}
#endif /* CONFIG_DFS */
		}
	}

	#ifdef CONFIG_DFS_MASTER
	for (i = 0; i < chanset_size; i++)
		channel_set[i].non_ocp_end_time = jiffies;
	#endif
#endif /* CONFIG_IEEE80211_BAND_5GHZ */

	if (RTW_CHPLAN_REALTEK_DEFINE == ChannelPlan) {
		hal_data->Regulation2_4G = RTW_CHANNEL_PLAN_MAP_REALTEK_DEFINE.regd;
		hal_data->Regulation5G = RTW_CHANNEL_PLAN_MAP_REALTEK_DEFINE.regd;
	} else {
		hal_data->Regulation2_4G = RTW_ChannelPlanMap[ChannelPlan].regd;
		hal_data->Regulation5G = RTW_ChannelPlanMap[ChannelPlan].regd;
	}

	RTW_INFO(FUNC_ADPT_FMT" ChannelPlan ID:0x%02x, ch num:%d\n"
		, FUNC_ADPT_ARG(padapter), ChannelPlan, chanset_size);

	return chanset_size;
}

int	init_mlme_ext_priv(_adapter *padapter)
{
	int	res = _SUCCESS;
	struct registry_priv *pregistrypriv = &padapter->registrypriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	/* We don't need to memset padapter->XXX to zero, because adapter is allocated by rtw_zvmalloc(). */
	/* memset((u8 *)pmlmeext, 0, sizeof(struct mlme_ext_priv)); */

	pmlmeext->padapter = padapter;

	/* fill_fwpriv(padapter, &(pmlmeext->fwpriv)); */

	init_mlme_ext_priv_value(padapter);
	pmlmeinfo->bAcceptAddbaReq = pregistrypriv->bAcceptAddbaReq;

	init_mlme_ext_timer(padapter);

#ifdef CONFIG_AP_MODE
	init_mlme_ap_info(padapter);
#endif

	pmlmeext->max_chan_nums = init_channel_set(padapter, pmlmepriv->ChannelPlan, pmlmeext->channel_set);
	init_channel_list(padapter, pmlmeext->channel_set, pmlmeext->max_chan_nums, &pmlmeext->channel_list);
	pmlmeext->last_scan_time = 0;
	pmlmeext->mlmeext_init = true;


#ifdef CONFIG_ACTIVE_KEEP_ALIVE_CHECK
	pmlmeext->active_keep_alive_check = true;
#else
	pmlmeext->active_keep_alive_check = false;
#endif

#ifdef DBG_FIXED_CHAN
	pmlmeext->fixed_chan = 0xFF;
#endif

	return res;

}

void free_mlme_ext_priv(struct mlme_ext_priv *pmlmeext)
{
	_adapter *padapter = pmlmeext->padapter;

	if (!padapter)
		return;

	if (rtw_is_drv_stopped(padapter)) {
		_cancel_timer_ex(&pmlmeext->survey_timer);
		_cancel_timer_ex(&pmlmeext->link_timer);
		/* _cancel_timer_ex(&pmlmeext->ADDBA_timer); */
	}
}

static u8 cmp_pkt_chnl_diff(_adapter *padapter, u8 *pframe, uint packet_len)
{
	/* if the channel is same, return 0. else return channel differential	 */
	uint len;
	u8 channel;
	u8 *p;
	p = rtw_get_ie(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, _DSSET_IE_, &len, packet_len - _BEACON_IE_OFFSET_);
	if (p) {
		channel = *(p + 2);
		if (padapter->mlmeextpriv.cur_channel >= channel)
			return padapter->mlmeextpriv.cur_channel - channel;
		else
			return channel - padapter->mlmeextpriv.cur_channel;
	} else
		return 0;
}

static void _mgt_dispatcher(_adapter *padapter, struct mlme_handler *ptable, union recv_frame *precv_frame)
{
	u8 bc_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	u8 *pframe = precv_frame->u.hdr.rx_data;

	if (ptable->func) {
		/* receive the frames that ra(a1) is my address or ra(a1) is bc address. */
		if (memcmp(GetAddr1Ptr(pframe), adapter_mac_addr(padapter), ETH_ALEN) &&
		    memcmp(GetAddr1Ptr(pframe), bc_addr, ETH_ALEN))
			return;

		ptable->func(padapter, precv_frame);
	}

}

void mgt_dispatcher(_adapter *padapter, union recv_frame *precv_frame)
{
	int index;
	struct mlme_handler *ptable;
#ifdef CONFIG_AP_MODE
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
#endif /* CONFIG_AP_MODE */
	u8 bc_addr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	u8 *pframe = precv_frame->u.hdr.rx_data;
	struct sta_info *psta = rtw_get_stainfo(&padapter->stapriv, get_addr2_ptr(pframe));
	struct dvobj_priv *psdpriv = padapter->dvobj;
	struct debug_priv *pdbgpriv = &psdpriv->drv_dbg;

	if (GetFrameType(pframe) != WIFI_MGT_TYPE) {
		return;
	}

	/* receive the frames that ra(a1) is my address or ra(a1) is bc address. */
	if (memcmp(GetAddr1Ptr(pframe), adapter_mac_addr(padapter), ETH_ALEN) &&
	    memcmp(GetAddr1Ptr(pframe), bc_addr, ETH_ALEN))
		return;

	ptable = mlme_sta_tbl;

	index = get_frame_sub_type(pframe) >> 4;

#ifdef CONFIG_TDLS
	if ((index << 4) == WIFI_ACTION) {
		/* category==public (4), action==TDLS_DISCOVERY_RESPONSE */
		if (*(pframe + 24) == RTW_WLAN_CATEGORY_PUBLIC && *(pframe + 25) == TDLS_DISCOVERY_RESPONSE) {
			RTW_INFO("[TDLS] Recv %s from "MAC_FMT"\n", rtw_tdls_action_txt(TDLS_DISCOVERY_RESPONSE), MAC_ARG(get_addr2_ptr(pframe)));
			On_TDLS_Dis_Rsp(padapter, precv_frame);
		}
	}
#endif /* CONFIG_TDLS */

	if (index >= (sizeof(mlme_sta_tbl) / sizeof(struct mlme_handler))) {
		return;
	}
	ptable += index;

	if (psta != NULL) {
		if (GetRetry(pframe)) {
			if (le16_to_cpu(precv_frame->u.hdr.attrib.seq_num) == psta->RxMgmtFrameSeqNum) {
				/* drop the duplicate management frame */
				pdbgpriv->dbg_rx_dup_mgt_frame_drop_count++;
				RTW_INFO("Drop duplicate management frame with seq_num = %d.\n", precv_frame->u.hdr.attrib.seq_num);
				return;
			}
		}
		psta->RxMgmtFrameSeqNum = le16_to_cpu(precv_frame->u.hdr.attrib.seq_num);
	}

#ifdef CONFIG_AP_MODE
	switch (get_frame_sub_type(pframe)) {
	case WIFI_AUTH:
		if (check_fwstate(pmlmepriv, WIFI_AP_STATE))
			ptable->func = &OnAuth;
		else
			ptable->func = &OnAuthClient;
		__attribute__ ((__fallthrough__));/* FALL THRU */
	case WIFI_ASSOCREQ:
	case WIFI_REASSOCREQ:
		_mgt_dispatcher(padapter, ptable, precv_frame);
#ifdef CONFIG_HOSTAPD_MLME
		if (check_fwstate(pmlmepriv, WIFI_AP_STATE))
			rtw_hostapd_mlme_rx(padapter, precv_frame);
#endif
		break;
	case WIFI_PROBEREQ:
		if (check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
#ifdef CONFIG_HOSTAPD_MLME
			rtw_hostapd_mlme_rx(padapter, precv_frame);
#else
			_mgt_dispatcher(padapter, ptable, precv_frame);
#endif
		} else
			_mgt_dispatcher(padapter, ptable, precv_frame);
		break;
	case WIFI_BEACON:
		_mgt_dispatcher(padapter, ptable, precv_frame);
		break;
	case WIFI_ACTION:
		/* if(check_fwstate(pmlmepriv, WIFI_AP_STATE)) */
		_mgt_dispatcher(padapter, ptable, precv_frame);
		break;
	default:
		_mgt_dispatcher(padapter, ptable, precv_frame);
		if (check_fwstate(pmlmepriv, WIFI_AP_STATE))
			rtw_hostapd_mlme_rx(padapter, precv_frame);
		break;
	}
#else

	_mgt_dispatcher(padapter, ptable, precv_frame);

#endif

}

#ifdef CONFIG_P2P
static u32 p2p_listen_state_process(_adapter *padapter, unsigned char *da)
{
	bool response = true;

#ifdef CONFIG_IOCTL_CFG80211
	if (padapter->wdinfo.driver_interface == DRIVER_CFG80211) {
		if (rtw_cfg80211_get_is_roch(padapter) == false
			|| rtw_get_oper_ch(padapter) != padapter->wdinfo.listen_channel
			|| adapter_wdev_data(padapter)->p2p_enabled == false
			|| padapter->mlmepriv.wps_probe_resp_ie == NULL
			|| padapter->mlmepriv.p2p_probe_resp_ie == NULL
		) {
#ifdef CONFIG_DEBUG_CFG80211
			RTW_INFO(ADPT_FMT" DON'T issue_probersp_p2p: p2p_enabled:%d, wps_probe_resp_ie:%p, p2p_probe_resp_ie:%p\n"
				, ADPT_ARG(padapter)
				, adapter_wdev_data(padapter)->p2p_enabled
				, padapter->mlmepriv.wps_probe_resp_ie
				, padapter->mlmepriv.p2p_probe_resp_ie);
			RTW_INFO(ADPT_FMT" DON'T issue_probersp_p2p: is_ro_ch:%d, op_ch:%d, p2p_listen_channel:%d\n"
				, ADPT_ARG(padapter)
				, rtw_cfg80211_get_is_roch(padapter)
				, rtw_get_oper_ch(padapter)
				, padapter->wdinfo.listen_channel);
#endif
			response = false;
		}
	} else
#endif /* CONFIG_IOCTL_CFG80211 */
		if (padapter->wdinfo.driver_interface == DRIVER_WEXT) {
			/*	do nothing if the device name is empty */
			if (!padapter->wdinfo.device_name_len)
				response	= false;
		}

	if (response)
		issue_probersp_p2p(padapter, da);

	return _SUCCESS;
}
#endif /* CONFIG_P2P */


/****************************************************************************

Following are the callback functions for each subtype of the management frames

*****************************************************************************/

unsigned int OnProbeReq(_adapter *padapter, union recv_frame *precv_frame)
{
	unsigned int	ielen;
	unsigned char	*p;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX	*cur = &(pmlmeinfo->network);
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint len = precv_frame->u.hdr.len;
	u8 is_valid_p2p_probereq = false;

#ifdef CONFIG_ATMEL_RC_PATCH
	u8 *target_ie = NULL, *wps_ie = NULL;
	u8 *start;
	uint search_len = 0, wps_ielen = 0, target_ielen = 0;
	struct sta_info	*psta;
	struct sta_priv *pstapriv = &padapter->stapriv;
#endif


#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	struct rx_pkt_attrib	*pattrib = &precv_frame->u.hdr.attrib;
	u8 wifi_test_chk_rate = 1;

#ifdef CONFIG_IOCTL_CFG80211
	if ((pwdinfo->driver_interface == DRIVER_CFG80211)
	    && !rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)
	    && (GET_CFG80211_REPORT_MGMT(adapter_wdev_data(padapter), IEEE80211_STYPE_PROBE_REQ))
	) {
		rtw_cfg80211_rx_probe_request(padapter, precv_frame);
		return _SUCCESS;
	}
#endif /* CONFIG_IOCTL_CFG80211 */

	if (!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE) &&
	    !rtw_p2p_chk_state(pwdinfo, P2P_STATE_IDLE) &&
	    !rtw_p2p_chk_role(pwdinfo, P2P_ROLE_CLIENT) &&
	    !rtw_p2p_chk_state(pwdinfo, P2P_STATE_FIND_PHASE_SEARCH) &&
	    !rtw_p2p_chk_state(pwdinfo, P2P_STATE_SCAN)
	   ) {
		/*	Commented by Albert 2011/03/17 */
		/*	mcs_rate = 0->CCK 1M rate */
		/*	mcs_rate = 1->CCK 2M rate */
		/*	mcs_rate = 2->CCK 5.5M rate */
		/*	mcs_rate = 3->CCK 11M rate */
		/*	In the P2P mode, the driver should not support the CCK rate */

		/*	Commented by Kurt 2012/10/16 */
		/*	IOT issue: Google Nexus7 use 1M rate to send p2p_probe_req after GO nego completed and Nexus7 is client */
		if (padapter->registrypriv.wifi_spec == 1) {
			if (pattrib->data_rate <= 3)
				wifi_test_chk_rate = 0;
		}

		if (wifi_test_chk_rate == 1) {
			is_valid_p2p_probereq = process_probe_req_p2p_ie(pwdinfo, pframe, len);
			if (is_valid_p2p_probereq) {
				if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_DEVICE)) {
					/* FIXME */
					if (padapter->wdinfo.driver_interface == DRIVER_WEXT)
						report_survey_event(padapter, precv_frame);

					p2p_listen_state_process(padapter,  get_sa(pframe));

					return _SUCCESS;
				}

				if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO))
					goto _continue;
			}
		}
	}

_continue:
#endif /* CONFIG_P2P */

	if (check_fwstate(pmlmepriv, WIFI_STATION_STATE))
		return _SUCCESS;

	if (check_fwstate(pmlmepriv, _FW_LINKED) == false &&
	    check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE | WIFI_AP_STATE) == false)
		return _SUCCESS;


	/* RTW_INFO("+OnProbeReq\n"); */


#ifdef CONFIG_ATMEL_RC_PATCH
	wps_ie = rtw_get_wps_ie(
			      pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_,
			      len - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_,
			      NULL, &wps_ielen);
	if (wps_ie)
		target_ie = rtw_get_wps_attr_content(wps_ie, wps_ielen, WPS_ATTR_MANUFACTURER, NULL, &target_ielen);
	if ((target_ie && (target_ielen == 4)) && (!memcmp((void *)target_ie, "Ozmo", 4))) {
		/* psta->flag_atmel_rc = 1; */
		unsigned char *sa_addr = get_sa(pframe);
		RTW_INFO("%s: Find Ozmo RC -- %02x:%02x:%02x:%02x:%02x:%02x  \n\n",
		       __func__, *sa_addr, *(sa_addr + 1), *(sa_addr + 2), *(sa_addr + 3), *(sa_addr + 4), *(sa_addr + 5));
		memcpy(pstapriv->atmel_rc_pattern, get_sa(pframe), ETH_ALEN);
	}
#endif


#ifdef CONFIG_AUTO_AP_MODE
	if (check_fwstate(pmlmepriv, _FW_LINKED) &&
	    pmlmepriv->cur_network.join_res) {
		unsigned long	irqL;
		struct sta_info	*psta;
		u8 *mac_addr, *peer_addr;
		struct sta_priv *pstapriv = &padapter->stapriv;
		u8 RC_OUI[4] = {0x00, 0xE0, 0x4C, 0x0A};
		/* EID[1] + EID_LEN[1] + RC_OUI[4] + MAC[6] + PairingID[2] + ChannelNum[2] */

		p = rtw_get_ie(pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_, _VENDOR_SPECIFIC_IE_, (int *)&ielen,
			       len - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_);

		if (!p || ielen != 14)
			goto _non_rc_device;

		if (memcmp(p + 2, RC_OUI, sizeof(RC_OUI)))
			goto _non_rc_device;

		if (memcmp(p + 6, get_sa(pframe), ETH_ALEN)) {
			RTW_INFO("%s, do rc pairing ("MAC_FMT"), but mac addr mismatch!("MAC_FMT")\n", __func__,
				 MAC_ARG(get_sa(pframe)), MAC_ARG(p + 6));

			goto _non_rc_device;
		}

		RTW_INFO("%s, got the pairing device("MAC_FMT")\n", __func__,  MAC_ARG(get_sa(pframe)));

		/* new a station */
		psta = rtw_get_stainfo(pstapriv, get_sa(pframe));
		if (psta == NULL) {
			/* allocate a new one */
			RTW_INFO("going to alloc stainfo for rc="MAC_FMT"\n",  MAC_ARG(get_sa(pframe)));
			psta = rtw_alloc_stainfo(pstapriv, get_sa(pframe));
			if (psta == NULL) {
				/* TODO: */
				RTW_INFO(" Exceed the upper limit of supported clients...\n");
				return _SUCCESS;
			}

			_enter_critical_bh(&pstapriv->asoc_list_lock, &irqL);
			if (list_empty(&psta->asoc_list)) {
				psta->expire_to = pstapriv->expire_to;
				list_add_tail(&psta->asoc_list, &pstapriv->asoc_list);
				pstapriv->asoc_list_cnt++;
			}
			_exit_critical_bh(&pstapriv->asoc_list_lock, &irqL);

			/* generate pairing ID */
			mac_addr = adapter_mac_addr(padapter);
			peer_addr = psta->hwaddr;
			psta->pid = (u16)(((mac_addr[4] << 8) + mac_addr[5]) + ((peer_addr[4] << 8) + peer_addr[5]));

			/* update peer stainfo */
			psta->isrc = true;

			/* get a unique AID */
			if (psta->aid > 0)
				RTW_INFO("old AID %d\n", psta->aid);
			else {
				for (psta->aid = 1; psta->aid <= NUM_STA; psta->aid++)
					if (pstapriv->sta_aid[psta->aid - 1] == NULL)
						break;

				if (psta->aid > pstapriv->max_num_sta) {
					psta->aid = 0;
					RTW_INFO("no room for more AIDs\n");
					return _SUCCESS;
				} else {
					pstapriv->sta_aid[psta->aid - 1] = psta;
					RTW_INFO("allocate new AID = (%d)\n", psta->aid);
				}
			}

			psta->qos_option = 1;
			psta->bw_mode = CHANNEL_WIDTH_20;
			psta->ieee8021x_blocked = false;
			psta->htpriv.ht_option = true;
			psta->htpriv.ampdu_enable = false;
			psta->htpriv.sgi_20m = false;
			psta->htpriv.sgi_40m = false;
			psta->htpriv.ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
			psta->htpriv.agg_enable_bitmap = 0x0;/* reset */
			psta->htpriv.candidate_tid_bitmap = 0x0;/* reset */

			rtw_hal_set_odm_var(padapter, HAL_ODM_STA_INFO, psta, true);

			memset((void *)&psta->sta_stats, 0, sizeof(struct stainfo_stats));

			_enter_critical_bh(&psta->lock, &irqL);
			psta->state |= _FW_LINKED;
			_exit_critical_bh(&psta->lock, &irqL);

			report_add_sta_event(padapter, psta->hwaddr);

		}

		issue_probersp(padapter, get_sa(pframe), false);

		return _SUCCESS;

	}

_non_rc_device:

	return _SUCCESS;

#endif /* CONFIG_AUTO_AP_MODE */


#ifdef CONFIG_CONCURRENT_MODE
	if (((pmlmeinfo->state & 0x03) == WIFI_FW_AP_STATE) &&
	    rtw_mi_buddy_check_fwstate(padapter, _FW_UNDER_LINKING | _FW_UNDER_SURVEY)) {
		/* don't process probe req */
		return _SUCCESS;
	}
#endif

	p = rtw_get_ie(pframe + WLAN_HDR_A3_LEN + _PROBEREQ_IE_OFFSET_, _SSID_IE_, (int *)&ielen,
		       len - WLAN_HDR_A3_LEN - _PROBEREQ_IE_OFFSET_);


	/* check (wildcard) SSID */
	if (p != NULL) {
		if (is_valid_p2p_probereq)
			goto _issue_probersp;

		if ((ielen != 0 && false == !memcmp((void *)(p + 2), (void *)cur->Ssid.Ssid, cur->Ssid.SsidLength))
		    || (ielen == 0 && pmlmeinfo->hidden_ssid_mode)
		   )
			return _SUCCESS;

_issue_probersp:
		if (((check_fwstate(pmlmepriv, _FW_LINKED) &&
		      pmlmepriv->cur_network.join_res)) || check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE)) {
			/* RTW_INFO("+issue_probersp during ap mode\n"); */
			issue_probersp(padapter, get_sa(pframe), is_valid_p2p_probereq);
		}

	}

	return _SUCCESS;

}

unsigned int OnProbeRsp(_adapter *padapter, union recv_frame *precv_frame)
{
	struct sta_info		*psta;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct sta_priv		*pstapriv = &padapter->stapriv;
	u8	*pframe = precv_frame->u.hdr.rx_data;
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &padapter->wdinfo;
#endif


#ifdef CONFIG_P2P
	if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_TX_PROVISION_DIS_REQ)) {
		if (pwdinfo->tx_prov_disc_info.benable) {
			if (!memcmp(pwdinfo->tx_prov_disc_info.peerIFAddr, get_addr2_ptr(pframe), ETH_ALEN)) {
				if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_CLIENT)) {
					pwdinfo->tx_prov_disc_info.benable = false;
					issue_p2p_provision_request(padapter,
						pwdinfo->tx_prov_disc_info.ssid.Ssid,
						pwdinfo->tx_prov_disc_info.ssid.SsidLength,
						pwdinfo->tx_prov_disc_info.peerDevAddr);
				} else if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_DEVICE) || rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO)) {
					pwdinfo->tx_prov_disc_info.benable = false;
					issue_p2p_provision_request(padapter,
								    NULL,
								    0,
						pwdinfo->tx_prov_disc_info.peerDevAddr);
				}
			}
		}
		return _SUCCESS;
	} else if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_GONEGO_ING)) {
		if (pwdinfo->nego_req_info.benable) {
			RTW_INFO("[%s] P2P State is GONEGO ING!\n", __func__);
			if (!memcmp(pwdinfo->nego_req_info.peerDevAddr, get_addr2_ptr(pframe), ETH_ALEN)) {
				pwdinfo->nego_req_info.benable = false;
				issue_p2p_GO_request(padapter, pwdinfo->nego_req_info.peerDevAddr);
			}
		}
	} else if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_TX_INVITE_REQ)) {
		if (pwdinfo->invitereq_info.benable) {
			RTW_INFO("[%s] P2P_STATE_TX_INVITE_REQ!\n", __func__);
			if (!memcmp(pwdinfo->invitereq_info.peer_macaddr, get_addr2_ptr(pframe), ETH_ALEN)) {
				pwdinfo->invitereq_info.benable = false;
				issue_p2p_invitation_request(padapter, pwdinfo->invitereq_info.peer_macaddr);
			}
		}
	}
#endif


	if (mlmeext_chk_scan_state(pmlmeext, SCAN_PROCESS)) {
		rtw_mi_report_survey_event(padapter, precv_frame);
		return _SUCCESS;
	}

	return _SUCCESS;
}

/* for 11n Logo 4.2.31/4.2.32 */
static void rtw_check_legacy_ap(_adapter *padapter, u8 *pframe, u32 len)
{

	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	if (!padapter->registrypriv.wifi_spec)
		return;
	
	if(!MLME_IS_AP(padapter))
		return;
	

	if (pmlmeext->bstart_bss) {
		int left;
		u16 capability;
		unsigned char *pos;
		struct rtw_ieee802_11_elems elems;
		struct HT_info_element *pht_info = NULL;
		u16 cur_op_mode; 

		/* checking IEs */
		left = len - sizeof(struct rtw_ieee80211_hdr_3addr) - _BEACON_IE_OFFSET_;
		pos = pframe + sizeof(struct rtw_ieee80211_hdr_3addr) + _BEACON_IE_OFFSET_;
		if (rtw_ieee802_11_parse_elems(pos, left, &elems, 1) == ParseFailed) {
			RTW_INFO("%s: parse fail for "MAC_FMT"\n", __func__, MAC_ARG(GetAddr3Ptr(pframe)));
			return;
		}

		cur_op_mode = pmlmepriv->ht_op_mode & HT_INFO_OPERATION_MODE_OP_MODE_MASK;

		/* for legacy ap */
		if (elems.ht_capabilities == NULL && elems.ht_capabilities_len == 0) {

			if (0)
				RTW_INFO("%s: "MAC_FMT" is legacy ap\n", __func__, MAC_ARG(GetAddr3Ptr(pframe)));

			ATOMIC_SET(&pmlmepriv->olbc, true);
			ATOMIC_SET(&pmlmepriv->olbc_ht, true);
		}
			
	}
}

unsigned int OnBeacon(_adapter *padapter, union recv_frame *precv_frame)
{
	struct sta_info	*psta;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct sta_priv	*pstapriv = &padapter->stapriv;
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint len = precv_frame->u.hdr.len;
	WLAN_BSSID_EX *pbss;
	int ret = _SUCCESS;
	u8 *p = NULL;
	u32 ielen = 0;
#ifdef CONFIG_TDLS
	struct sta_info *ptdls_sta;
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
#ifdef CONFIG_TDLS_CH_SW
	struct tdls_ch_switch *pchsw_info = &padapter->tdlsinfo.chsw_info;
#endif
#endif /* CONFIG_TDLS */

	if (validate_beacon_len(pframe, len) == false)
		return _SUCCESS;
#ifdef CONFIG_ATTEMPT_TO_FIX_AP_BEACON_ERROR
	p = rtw_get_ie(pframe + sizeof(struct rtw_ieee80211_hdr_3addr) + _BEACON_IE_OFFSET_, _EXT_SUPPORTEDRATES_IE_, &ielen,
		precv_frame->u.hdr.len - sizeof(struct rtw_ieee80211_hdr_3addr) - _BEACON_IE_OFFSET_);
	if ((p != NULL) && (ielen > 0)) {
		if ((*(p + 1 + ielen) == 0x2D) && (*(p + 2 + ielen) != 0x2D)) {
			/* Invalid value 0x2D is detected in Extended Supported Rates (ESR) IE. Try to fix the IE length to avoid failed Beacon parsing. */
			RTW_INFO("[WIFIDBG] Error in ESR IE is detected in Beacon of BSSID:"MAC_FMT". Fix the length of ESR IE to avoid failed Beacon parsing.\n", MAC_ARG(GetAddr3Ptr(pframe)));
			*(p + 1) = ielen - 1;
		}
	}
#endif

	if (mlmeext_chk_scan_state(pmlmeext, SCAN_PROCESS)) {
		rtw_mi_report_survey_event(padapter, precv_frame);
		return _SUCCESS;
	}


	rtw_check_legacy_ap(padapter, pframe, len);

	if (!memcmp(GetAddr3Ptr(pframe), get_my_bssid(&pmlmeinfo->network), ETH_ALEN)) {
		if (pmlmeinfo->state & WIFI_FW_AUTH_NULL) {
			/* we should update current network before auth, or some IE is wrong */
			pbss = (WLAN_BSSID_EX *)rtw_malloc(sizeof(WLAN_BSSID_EX));
			if (pbss) {
				if (collect_bss_info(padapter, precv_frame, pbss) == _SUCCESS) {
					struct beacon_keys recv_beacon;

					update_network(&(pmlmepriv->cur_network.network), pbss, padapter, true);
					rtw_get_bcn_info(&(pmlmepriv->cur_network));

					/* update bcn keys */
					if (rtw_get_bcn_keys(padapter, pframe, len, &recv_beacon)) {
						RTW_INFO("%s: beacon keys ready\n", __func__);
						memcpy(&pmlmepriv->cur_beacon_keys,
							&recv_beacon, sizeof(recv_beacon));
						pmlmepriv->new_beacon_cnts = 0;
					} else {
						RTW_ERR("%s: get beacon keys failed\n", __func__);
						memset(&pmlmepriv->cur_beacon_keys, 0, sizeof(recv_beacon));
						pmlmepriv->new_beacon_cnts = 0;
					}
				}
				rtw_mfree((u8 *)pbss, sizeof(WLAN_BSSID_EX));
			}

			/* check the vendor of the assoc AP */
			pmlmeinfo->assoc_AP_vendor = check_assoc_AP(pframe + sizeof(struct rtw_ieee80211_hdr_3addr), len - sizeof(struct rtw_ieee80211_hdr_3addr));

			/* update TSF Value */
			update_TSF(pmlmeext, pframe, len);

			/* reset for adaptive_early_32k */
			pmlmeext->adaptive_tsf_done = false;
			pmlmeext->DrvBcnEarly = 0xff;
			pmlmeext->DrvBcnTimeOut = 0xff;
			pmlmeext->bcn_cnt = 0;
			memset(pmlmeext->bcn_delay_cnt, 0, sizeof(pmlmeext->bcn_delay_cnt));
			memset(pmlmeext->bcn_delay_ratio, 0, sizeof(pmlmeext->bcn_delay_ratio));

#ifdef CONFIG_P2P_PS
			process_p2p_ps_ie(padapter, (pframe + WLAN_HDR_A3_LEN), (len - WLAN_HDR_A3_LEN));
#endif /* CONFIG_P2P_PS */

#if defined(CONFIG_P2P) && defined(CONFIG_CONCURRENT_MODE)
			if (padapter->registrypriv.wifi_spec) {
				if (process_p2p_cross_connect_ie(padapter, (pframe + WLAN_HDR_A3_LEN), (len - WLAN_HDR_A3_LEN)) == false) {
					if (rtw_mi_buddy_check_mlmeinfo_state(padapter, WIFI_FW_AP_STATE)) {
						RTW_PRINT("no issue auth, P2P cross-connect does not permit\n ");
						return _SUCCESS;
					}
				}
			}
#endif /* CONFIG_P2P CONFIG_P2P and CONFIG_CONCURRENT_MODE */

			/* start auth */
			start_clnt_auth(padapter);

			return _SUCCESS;
		}

		if (((pmlmeinfo->state & 0x03) == WIFI_FW_STATION_STATE) && (pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)) {
			psta = rtw_get_stainfo(pstapriv, get_addr2_ptr(pframe));
			if (psta != NULL) {
#ifdef CONFIG_PATCH_JOIN_WRONG_CHANNEL
				/* Merge from 8712 FW code */
				if (cmp_pkt_chnl_diff(padapter, pframe, len) != 0) {
					/* join wrong channel, deauth and reconnect           */
					issue_deauth(padapter, (&(pmlmeinfo->network))->MacAddress, WLAN_REASON_DEAUTH_LEAVING);

					report_del_sta_event(padapter, (&(pmlmeinfo->network))->MacAddress, WLAN_REASON_JOIN_WRONG_CHANNEL, true, false);
					pmlmeinfo->state &= (~WIFI_FW_ASSOC_SUCCESS);
					return _SUCCESS;
				}
#endif /* CONFIG_PATCH_JOIN_WRONG_CHANNEL */

				ret = rtw_check_bcn_info(padapter, pframe, len);
				if (!ret) {
					RTW_PRINT("ap has changed, disconnect now\n ");
					receive_disconnect(padapter, pmlmeinfo->network.MacAddress , 0, false);
					return _SUCCESS;
				}
				/* update WMM, ERP in the beacon */
				/* todo: the timer is used instead of the number of the beacon received */
				if ((sta_rx_pkts(psta) & 0xf) == 0) {
					/* RTW_INFO("update_bcn_info\n"); */
					update_beacon_info(padapter, pframe, len, psta);
				}

				pmlmepriv->cur_network_scanned->network.Rssi = precv_frame->u.hdr.attrib.phy_info.RecvSignalPower;

				adaptive_early_32k(pmlmeext, pframe, len);

#ifdef CONFIG_TDLS
#ifdef CONFIG_TDLS_CH_SW
				if (rtw_tdls_is_chsw_allowed(padapter)) {
					/* Send TDLS Channel Switch Request when receiving Beacon */
					if ((padapter->tdlsinfo.chsw_info.ch_sw_state & TDLS_CH_SW_INITIATOR_STATE) && (ATOMIC_READ(&pchsw_info->chsw_on))
					    && (pmlmeext->cur_channel == rtw_get_oper_ch(padapter))) {
						ptdls_sta = rtw_get_stainfo(&padapter->stapriv, padapter->tdlsinfo.chsw_info.addr);
						if (ptdls_sta != NULL) {
							if (ptdls_sta->tdls_sta_state | TDLS_LINKED_STATE)
								_set_timer(&ptdls_sta->stay_on_base_chnl_timer, TDLS_CH_SW_STAY_ON_BASE_CHNL_TIMEOUT);
						}
					}
				}
#endif
#endif /* CONFIG_TDLS */

#ifdef CONFIG_DFS
				process_csa_ie(padapter, pframe, len);	/* channel switch announcement */
#endif /* CONFIG_DFS */

#ifdef CONFIG_P2P_PS
				process_p2p_ps_ie(padapter, (pframe + WLAN_HDR_A3_LEN), (len - WLAN_HDR_A3_LEN));
#endif /* CONFIG_P2P_PS */

				if (pmlmeext->en_hw_update_tsf)
					rtw_enable_hw_update_tsf_cmd(padapter);
			}
		} else if ((pmlmeinfo->state & 0x03) == WIFI_FW_ADHOC_STATE) {
			unsigned long irqL;
			u8 rate_set[16];
			u8 rate_num = 0;

			psta = rtw_get_stainfo(pstapriv, get_addr2_ptr(pframe));
			if (psta != NULL) {
				/*
				* update WMM, ERP in the beacon
				* todo: the timer is used instead of the number of the beacon received
				*/
				if ((sta_rx_pkts(psta) & 0xf) == 0)
					update_beacon_info(padapter, pframe, len, psta);

				if (pmlmeext->en_hw_update_tsf)
					rtw_enable_hw_update_tsf_cmd(padapter);
			} else {
				rtw_ies_get_supported_rate(pframe + WLAN_HDR_A3_LEN + _BEACON_IE_OFFSET_, len - WLAN_HDR_A3_LEN - _BEACON_IE_OFFSET_, rate_set, &rate_num);
				if (rate_num == 0) {
					RTW_INFO(FUNC_ADPT_FMT" RX beacon with no supported rate\n", FUNC_ADPT_ARG(padapter));
					goto _END_ONBEACON_;
				}

				psta = rtw_alloc_stainfo(pstapriv, get_addr2_ptr(pframe));
				if (psta == NULL) {
					RTW_INFO(FUNC_ADPT_FMT" Exceed the upper limit of supported clients\n", FUNC_ADPT_ARG(padapter));
					goto _END_ONBEACON_;
				}

				psta->expire_to = pstapriv->adhoc_expire_to;

				memcpy(psta->bssrateset, rate_set, rate_num);
				psta->bssratelen = rate_num;

				/* update TSF Value */
				update_TSF(pmlmeext, pframe, len);

				/* report sta add event */
				report_add_sta_event(padapter, get_addr2_ptr(pframe));
			}
		}
	}

_END_ONBEACON_:

	return _SUCCESS;

}

unsigned int OnAuth(_adapter *padapter, union recv_frame *precv_frame)
{
#ifdef CONFIG_AP_MODE
	unsigned long irqL;
	unsigned int	auth_mode, seq, ie_len;
	unsigned char	*sa, *p;
	u16	algorithm;
	int	status;
	static struct sta_info stat;
	struct	sta_info	*pstat = NULL;
	struct	sta_priv *pstapriv = &padapter->stapriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint len = precv_frame->u.hdr.len;
	u8	offset = 0;


#ifdef CONFIG_CONCURRENT_MODE
	if (((pmlmeinfo->state & 0x03) == WIFI_FW_AP_STATE) &&
	    rtw_mi_buddy_check_fwstate(padapter, _FW_UNDER_LINKING | _FW_UNDER_SURVEY)) {
		/* don't process auth request; */
		return _SUCCESS;
	}
#endif /* CONFIG_CONCURRENT_MODE */

	if ((pmlmeinfo->state & 0x03) != WIFI_FW_AP_STATE)
		return _FAIL;

	RTW_INFO("+OnAuth\n");

	sa = get_addr2_ptr(pframe);

	auth_mode = psecuritypriv->dot11AuthAlgrthm;

	if (GetPrivacy(pframe)) {
		u8	*iv;
		struct rx_pkt_attrib	*prxattrib = &(precv_frame->u.hdr.attrib);

		prxattrib->hdrlen = WLAN_HDR_A3_LEN;
		prxattrib->encrypt = _WEP40_;

		iv = pframe + prxattrib->hdrlen;
		prxattrib->key_index = ((iv[3] >> 6) & 0x3);

		prxattrib->iv_len = 4;
		prxattrib->icv_len = 4;

		rtw_wep_decrypt(padapter, (u8 *)precv_frame);

		offset = 4;
	}

	algorithm = le16_to_cpu(*(__le16 *)((SIZE_PTR)pframe + WLAN_HDR_A3_LEN + offset));
	seq	= le16_to_cpu(*(__le16 *)((SIZE_PTR)pframe + WLAN_HDR_A3_LEN + offset + 2));

	RTW_INFO("auth alg=%x, seq=%X\n", algorithm, seq);

	if (auth_mode == 2 &&
	    psecuritypriv->dot11PrivacyAlgrthm != _WEP40_ &&
	    psecuritypriv->dot11PrivacyAlgrthm != _WEP104_)
		auth_mode = 0;

	if ((algorithm > 0 && auth_mode == 0) ||	/* rx a shared-key auth but shared not enabled */
	    (algorithm == 0 && auth_mode == 1)) {	/* rx a open-system auth but shared-key is enabled */
		RTW_INFO("auth rejected due to bad alg [alg=%d, auth_mib=%d] %02X%02X%02X%02X%02X%02X\n",
			algorithm, auth_mode, sa[0], sa[1], sa[2], sa[3], sa[4], sa[5]);

		status = _STATS_NO_SUPP_ALG_;

		goto auth_fail;
	}

#if CONFIG_RTW_MACADDR_ACL
	if (rtw_access_ctrl(padapter, sa) == false) {
		status = _STATS_UNABLE_HANDLE_STA_;
		goto auth_fail;
	}
#endif

	pstat = rtw_get_stainfo(pstapriv, sa);
	if (pstat == NULL) {

		/* allocate a new one */
		RTW_INFO("going to alloc stainfo for sa="MAC_FMT"\n",  MAC_ARG(sa));
		pstat = rtw_alloc_stainfo(pstapriv, sa);
		if (pstat == NULL) {
			RTW_INFO(" Exceed the upper limit of supported clients...\n");
			status = _STATS_UNABLE_HANDLE_STA_;
			goto auth_fail;
		}

		pstat->state = WIFI_FW_AUTH_NULL;
		pstat->auth_seq = 0;

		/* pstat->flags = 0; */
		/* pstat->capability = 0; */
	} else {
#ifdef CONFIG_IEEE80211W
		if (pstat->bpairwise_key_installed != true && !(pstat->state & WIFI_FW_ASSOC_SUCCESS))
#endif /* CONFIG_IEEE80211W */
		{

			_enter_critical_bh(&pstapriv->asoc_list_lock, &irqL);
			if (list_empty(&pstat->asoc_list) == false) {
				list_del_init(&pstat->asoc_list);
				pstapriv->asoc_list_cnt--;
				if (pstat->expire_to > 0)
					;/* TODO: STA re_auth within expire_to */
			}
			_exit_critical_bh(&pstapriv->asoc_list_lock, &irqL);

			if (seq == 1)
				; /* TODO: STA re_auth and auth timeout */

		}
	}

#ifdef CONFIG_IEEE80211W
	if (pstat->bpairwise_key_installed != true && !(pstat->state & WIFI_FW_ASSOC_SUCCESS))
#endif /* CONFIG_IEEE80211W */
	{
		_enter_critical_bh(&pstapriv->auth_list_lock, &irqL);
		if (list_empty(&pstat->auth_list)) {

			list_add_tail(&pstat->auth_list, &pstapriv->auth_list);
			pstapriv->auth_list_cnt++;
		}
		_exit_critical_bh(&pstapriv->auth_list_lock, &irqL);
	}

	if (pstat->auth_seq == 0)
		pstat->expire_to = pstapriv->auth_to;


	if ((pstat->auth_seq + 1) != seq) {
		RTW_INFO("(1)auth rejected because out of seq [rx_seq=%d, exp_seq=%d]!\n",
			 seq, pstat->auth_seq + 1);
		status = _STATS_OUT_OF_AUTH_SEQ_;
		goto auth_fail;
	}

	if (algorithm == 0 && (auth_mode == 0 || auth_mode == 2 || auth_mode == 3)) {
		if (seq == 1) {
#ifdef CONFIG_IEEE80211W
			if (pstat->bpairwise_key_installed != true && !(pstat->state & WIFI_FW_ASSOC_SUCCESS))
#endif /* CONFIG_IEEE80211W */
			{
				pstat->state &= ~WIFI_FW_AUTH_NULL;
				pstat->state |= WIFI_FW_AUTH_SUCCESS;
				pstat->expire_to = pstapriv->assoc_to;
			}
			pstat->authalg = algorithm;
		} else {
			RTW_INFO("(2)auth rejected because out of seq [rx_seq=%d, exp_seq=%d]!\n",
				 seq, pstat->auth_seq + 1);
			status = _STATS_OUT_OF_AUTH_SEQ_;
			goto auth_fail;
		}
	} else { /* shared system or auto authentication */
		if (seq == 1) {
			/* prepare for the challenging txt... */

			/* get_random_bytes((void *)pstat->chg_txt, 128); */ /* TODO: */
			memset((void *)pstat->chg_txt, 78, 128);
#ifdef CONFIG_IEEE80211W
			if (pstat->bpairwise_key_installed != true && !(pstat->state & WIFI_FW_ASSOC_SUCCESS))
#endif /* CONFIG_IEEE80211W */
			{
				pstat->state &= ~WIFI_FW_AUTH_NULL;
				pstat->state |= WIFI_FW_AUTH_STATE;
			}
			pstat->authalg = algorithm;
			pstat->auth_seq = 2;
		} else if (seq == 3) {
			/* checking for challenging txt... */
			RTW_INFO("checking for challenging txt...\n");

			p = rtw_get_ie(pframe + WLAN_HDR_A3_LEN + 4 + _AUTH_IE_OFFSET_ , _CHLGETXT_IE_, (int *)&ie_len,
				len - WLAN_HDR_A3_LEN - _AUTH_IE_OFFSET_ - 4);

			if ((p == NULL) || (ie_len <= 0)) {
				RTW_INFO("auth rejected because challenge failure!(1)\n");
				status = _STATS_CHALLENGE_FAIL_;
				goto auth_fail;
			}

			if (!memcmp((void *)(p + 2), pstat->chg_txt, 128)) {
#ifdef CONFIG_IEEE80211W
				if (pstat->bpairwise_key_installed != true && !(pstat->state & WIFI_FW_ASSOC_SUCCESS))
#endif /* CONFIG_IEEE80211W */
				{
					pstat->state &= (~WIFI_FW_AUTH_STATE);
					pstat->state |= WIFI_FW_AUTH_SUCCESS;
					/* challenging txt is correct... */
					pstat->expire_to =  pstapriv->assoc_to;
				}
			} else {
				RTW_INFO("auth rejected because challenge failure!\n");
				status = _STATS_CHALLENGE_FAIL_;
				goto auth_fail;
			}
		} else {
			RTW_INFO("(3)auth rejected because out of seq [rx_seq=%d, exp_seq=%d]!\n",
				 seq, pstat->auth_seq + 1);
			status = _STATS_OUT_OF_AUTH_SEQ_;
			goto auth_fail;
		}
	}


	/* Now, we are going to issue_auth... */
	pstat->auth_seq = seq + 1;

#ifdef CONFIG_NATIVEAP_MLME
	issue_auth(padapter, pstat, (unsigned short)(_STATS_SUCCESSFUL_));
#endif

	if ((pstat->state & WIFI_FW_AUTH_SUCCESS) || (pstat->state & WIFI_FW_ASSOC_SUCCESS))
		pstat->auth_seq = 0;


	return _SUCCESS;

auth_fail:

	if (pstat)
		rtw_free_stainfo(padapter , pstat);

	pstat = &stat;
	memset((char *)pstat, '\0', sizeof(stat));
	pstat->auth_seq = 2;
	memcpy(pstat->hwaddr, sa, 6);

#ifdef CONFIG_NATIVEAP_MLME
	issue_auth(padapter, pstat, (unsigned short)status);
#endif

#endif
	return _FAIL;

}

unsigned int OnAuthClient(_adapter *padapter, union recv_frame *precv_frame)
{
	unsigned int	seq, len, status, algthm, offset;
	unsigned char	*p;
	unsigned int	go2asoc = 0;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
#ifdef CONFIG_RTW_80211R
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	ft_priv	*pftpriv = &pmlmepriv->ftpriv;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta = NULL;
#endif
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint pkt_len = precv_frame->u.hdr.len;

	RTW_INFO("%s\n", __func__);

	/* check A1 matches or not */
	if (memcmp(adapter_mac_addr(padapter), get_da(pframe), ETH_ALEN))
		return _SUCCESS;

	if (!(pmlmeinfo->state & WIFI_FW_AUTH_STATE))
		return _SUCCESS;

	offset = (GetPrivacy(pframe)) ? 4 : 0;

	algthm	= le16_to_cpu(*(__le16 *)((SIZE_PTR)pframe + WLAN_HDR_A3_LEN + offset));
	seq	= le16_to_cpu(*(__le16 *)((SIZE_PTR)pframe + WLAN_HDR_A3_LEN + offset + 2));
	status	= le16_to_cpu(*(__le16 *)((SIZE_PTR)pframe + WLAN_HDR_A3_LEN + offset + 4));

	if (status != 0) {
		RTW_INFO("clnt auth fail, status: %d\n", status);
		if (status == 13) { /* && pmlmeinfo->auth_algo == dot11AuthAlgrthm_Auto) */
			if (pmlmeinfo->auth_algo == dot11AuthAlgrthm_Shared)
				pmlmeinfo->auth_algo = dot11AuthAlgrthm_Open;
			else
				pmlmeinfo->auth_algo = dot11AuthAlgrthm_Shared;
			/* pmlmeinfo->reauth_count = 0; */
		}

		set_link_timer(pmlmeext, 1);
		goto authclnt_fail;
	}

	if (seq == 2) {
		if (pmlmeinfo->auth_algo == dot11AuthAlgrthm_Shared) {
			/* legendary shared system */
			p = rtw_get_ie(pframe + WLAN_HDR_A3_LEN + _AUTH_IE_OFFSET_, _CHLGETXT_IE_, (int *)&len,
				pkt_len - WLAN_HDR_A3_LEN - _AUTH_IE_OFFSET_);

			if (p == NULL) {
				/* RTW_INFO("marc: no challenge text?\n"); */
				goto authclnt_fail;
			}

			memcpy((void *)(pmlmeinfo->chg_txt), (void *)(p + 2), len);
			pmlmeinfo->auth_seq = 3;
			issue_auth(padapter, NULL, 0);
			set_link_timer(pmlmeext, REAUTH_TO);

			return _SUCCESS;
		} else {
			/* open, or 802.11r FTAA system */
			go2asoc = 1;
		}
	} else if (seq == 4) {
		if (pmlmeinfo->auth_algo == dot11AuthAlgrthm_Shared)
			go2asoc = 1;
		else
			goto authclnt_fail;
	} else {
		/* this is also illegal */
		/* RTW_INFO("marc: clnt auth failed due to illegal seq=%x\n", seq); */
		goto authclnt_fail;
	}

	if (go2asoc) {
#ifdef CONFIG_RTW_80211R
		if ((rtw_to_roam(padapter) > 0) && rtw_chk_ft_flags(padapter, RTW_FT_SUPPORTED)) {
			u8 target_ap_addr[ETH_ALEN] = {0};

			if ((rtw_chk_ft_status(padapter, RTW_FT_AUTHENTICATED_STA)) ||
				(rtw_chk_ft_status(padapter, RTW_FT_ASSOCIATING_STA)) ||
				(rtw_chk_ft_status(padapter, RTW_FT_ASSOCIATED_STA))) {
				/*report_ft_reassoc_event already, and waiting for cfg80211_rtw_update_ft_ies*/
				return _SUCCESS;
			}

			rtw_buf_update(&pmlmepriv->auth_rsp, &pmlmepriv->auth_rsp_len, pframe, pkt_len);
			pftpriv->ft_event.ies = pmlmepriv->auth_rsp + sizeof(struct rtw_ieee80211_hdr_3addr) + 6;
			pftpriv->ft_event.ies_len = pmlmepriv->auth_rsp_len - sizeof(struct rtw_ieee80211_hdr_3addr) - 6;

			/*Not support RIC*/
			pftpriv->ft_event.ric_ies =  NULL;
			pftpriv->ft_event.ric_ies_len =  0;
			memcpy(target_ap_addr, pmlmepriv->assoc_bssid, ETH_ALEN);
			report_ft_reassoc_event(padapter, target_ap_addr);
			return _SUCCESS;
		}
#endif

		RTW_PRINT("auth success, start assoc\n");
		start_clnt_assoc(padapter);
		return _SUCCESS;
	}

authclnt_fail:

	/* pmlmeinfo->state &= ~(WIFI_FW_AUTH_STATE); */

	return _FAIL;

}

unsigned int OnAssocReq(_adapter *padapter, union recv_frame *precv_frame)
{
#ifdef CONFIG_AP_MODE
	unsigned long irqL;
	u16 capab_info, listen_interval;
	struct rtw_ieee802_11_elems elems;
	struct sta_info	*pstat;
	unsigned char		reassoc, *p, *pos, *wpa_ie;
	unsigned char WMM_IE[] = {0x00, 0x50, 0xf2, 0x02, 0x00, 0x01};
	int		i, ie_len, wpa_ie_len, left;
	u8 rate_set[16];
	u8 rate_num;
	unsigned short		status = _STATS_SUCCESSFUL_;
	unsigned short		frame_type, ie_offset = 0;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX	*cur = &(pmlmeinfo->network);
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint pkt_len = precv_frame->u.hdr.len;
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	u8 p2p_status_code = P2P_STATUS_SUCCESS;
	u8 *p2pie;
	u32 p2pielen = 0;
#endif /* CONFIG_P2P */

#ifdef CONFIG_CONCURRENT_MODE
	if (((pmlmeinfo->state & 0x03) == WIFI_FW_AP_STATE) &&
	    rtw_mi_buddy_check_fwstate(padapter, _FW_UNDER_LINKING | _FW_UNDER_SURVEY)) {
		/* don't process assoc request; */
		return _SUCCESS;
	}
#endif /* CONFIG_CONCURRENT_MODE */

	if ((pmlmeinfo->state & 0x03) != WIFI_FW_AP_STATE)
		return _FAIL;

	frame_type = get_frame_sub_type(pframe);
	if (frame_type == WIFI_ASSOCREQ) {
		reassoc = 0;
		ie_offset = _ASOCREQ_IE_OFFSET_;
	} else { /* WIFI_REASSOCREQ */
		reassoc = 1;
		ie_offset = _REASOCREQ_IE_OFFSET_;
	}


	if (pkt_len < IEEE80211_3ADDR_LEN + ie_offset) {
		RTW_INFO("handle_assoc(reassoc=%d) - too short payload (len=%lu)"
			 "\n", reassoc, (unsigned long)pkt_len);
		return _FAIL;
	}

	pstat = rtw_get_stainfo(pstapriv, get_addr2_ptr(pframe));
	if (pstat == (struct sta_info *)NULL) {
		status = _RSON_CLS2_;
		goto asoc_class2_error;
	}

	capab_info = RTW_GET_LE16(pframe + WLAN_HDR_A3_LEN);
	/* capab_info = le16_to_cpu(*(__le16 *)(pframe + WLAN_HDR_A3_LEN));	 */
	/* listen_interval = le16_to_cpu(*(__le16 *)(pframe + WLAN_HDR_A3_LEN+2)); */
	listen_interval = RTW_GET_LE16(pframe + WLAN_HDR_A3_LEN + 2);

	left = pkt_len - (IEEE80211_3ADDR_LEN + ie_offset);
	pos = pframe + (IEEE80211_3ADDR_LEN + ie_offset);


	RTW_INFO("%s\n", __func__);

	/* check if this stat has been successfully authenticated/assocated */
	if (!((pstat->state) & WIFI_FW_AUTH_SUCCESS)) {
		if (!((pstat->state) & WIFI_FW_ASSOC_SUCCESS)) {
			status = _RSON_CLS2_;
			goto asoc_class2_error;
		} else {
			pstat->state &= (~WIFI_FW_ASSOC_SUCCESS);
			pstat->state |= WIFI_FW_ASSOC_STATE;
		}
	} else {
		pstat->state &= (~WIFI_FW_AUTH_SUCCESS);
		pstat->state |= WIFI_FW_ASSOC_STATE;
	}

	pstat->capability = capab_info;

	/* now parse all ieee802_11 ie to point to elems */
	if (rtw_ieee802_11_parse_elems(pos, left, &elems, 1) == ParseFailed ||
	    !elems.ssid) {
		RTW_INFO("STA " MAC_FMT " sent invalid association request\n",
			 MAC_ARG(pstat->hwaddr));
		status = _STATS_FAILURE_;
		goto OnAssocReqFail;
	}


	/* now we should check all the fields... */
	/* checking SSID */
	p = rtw_get_ie(pframe + WLAN_HDR_A3_LEN + ie_offset, _SSID_IE_, &ie_len,
		       pkt_len - WLAN_HDR_A3_LEN - ie_offset);
	if (p == NULL)
		status = _STATS_FAILURE_;

	if (ie_len == 0) /* broadcast ssid, however it is not allowed in assocreq */
		status = _STATS_FAILURE_;
	else {
		/* check if ssid match */
		if (memcmp((void *)(p + 2), cur->Ssid.Ssid, cur->Ssid.SsidLength))
			status = _STATS_FAILURE_;

		if (ie_len != cur->Ssid.SsidLength)
			status = _STATS_FAILURE_;
	}

	if (_STATS_SUCCESSFUL_ != status)
		goto OnAssocReqFail;

	rtw_ies_get_supported_rate(pframe + WLAN_HDR_A3_LEN + ie_offset, pkt_len - WLAN_HDR_A3_LEN - ie_offset, rate_set, &rate_num);
	if (rate_num == 0) {
		RTW_INFO(FUNC_ADPT_FMT" RX assoc-req with no supported rate\n", FUNC_ADPT_ARG(padapter));
		status = _STATS_FAILURE_;
		goto OnAssocReqFail;
	}
	memcpy(pstat->bssrateset, rate_set, rate_num);
	pstat->bssratelen = rate_num;
	UpdateBrateTblForSoftAP(pstat->bssrateset, pstat->bssratelen);

	/* check RSN/WPA/WPS */
	pstat->dot8021xalg = 0;
	pstat->wpa_psk = 0;
	pstat->wpa_group_cipher = 0;
	pstat->wpa2_group_cipher = 0;
	pstat->wpa_pairwise_cipher = 0;
	pstat->wpa2_pairwise_cipher = 0;
	memset(pstat->wpa_ie, 0, sizeof(pstat->wpa_ie));
	if ((psecuritypriv->wpa_psk & BIT(1)) && elems.rsn_ie) {

		int group_cipher = 0, pairwise_cipher = 0;

		wpa_ie = elems.rsn_ie;
		wpa_ie_len = elems.rsn_ie_len;

		if (rtw_parse_wpa2_ie(wpa_ie - 2, wpa_ie_len + 2, &group_cipher, &pairwise_cipher, NULL) == _SUCCESS) {
			pstat->dot8021xalg = 1;/* psk,  todo:802.1x						 */
			pstat->wpa_psk |= BIT(1);

			pstat->wpa2_group_cipher = group_cipher & psecuritypriv->wpa2_group_cipher;
			pstat->wpa2_pairwise_cipher = pairwise_cipher & psecuritypriv->wpa2_pairwise_cipher;

			if (!pstat->wpa2_group_cipher)
				status = WLAN_STATUS_GROUP_CIPHER_NOT_VALID;

			if (!pstat->wpa2_pairwise_cipher)
				status = WLAN_STATUS_PAIRWISE_CIPHER_NOT_VALID;
		} else
			status = WLAN_STATUS_INVALID_IE;

	} else if ((psecuritypriv->wpa_psk & BIT(0)) && elems.wpa_ie) {

		int group_cipher = 0, pairwise_cipher = 0;

		wpa_ie = elems.wpa_ie;
		wpa_ie_len = elems.wpa_ie_len;

		if (rtw_parse_wpa_ie(wpa_ie - 2, wpa_ie_len + 2, &group_cipher, &pairwise_cipher, NULL) == _SUCCESS) {
			pstat->dot8021xalg = 1;/* psk,  todo:802.1x						 */
			pstat->wpa_psk |= BIT(0);

			pstat->wpa_group_cipher = group_cipher & psecuritypriv->wpa_group_cipher;
			pstat->wpa_pairwise_cipher = pairwise_cipher & psecuritypriv->wpa_pairwise_cipher;

			if (!pstat->wpa_group_cipher)
				status = WLAN_STATUS_GROUP_CIPHER_NOT_VALID;

			if (!pstat->wpa_pairwise_cipher)
				status = WLAN_STATUS_PAIRWISE_CIPHER_NOT_VALID;

		} else
			status = WLAN_STATUS_INVALID_IE;

	} else {
		wpa_ie = NULL;
		wpa_ie_len = 0;
	}

	if (_STATS_SUCCESSFUL_ != status)
		goto OnAssocReqFail;

	pstat->flags &= ~(WLAN_STA_WPS | WLAN_STA_MAYBE_WPS);
	/* if (hapd->conf->wps_state && wpa_ie == NULL) { */ /* todo: to check ap if supporting WPS */
	if (wpa_ie == NULL) {
		if (elems.wps_ie) {
			RTW_INFO("STA included WPS IE in "
				 "(Re)Association Request - assume WPS is "
				 "used\n");
			pstat->flags |= WLAN_STA_WPS;
			/* wpabuf_free(sta->wps_ie); */
			/* sta->wps_ie = wpabuf_alloc_copy(elems.wps_ie + 4, */
			/*				elems.wps_ie_len - 4); */
		} else {
			RTW_INFO("STA did not include WPA/RSN IE "
				 "in (Re)Association Request - possible WPS "
				 "use\n");
			pstat->flags |= WLAN_STA_MAYBE_WPS;
		}


		/* AP support WPA/RSN, and sta is going to do WPS, but AP is not ready */
		/* that the selected registrar of AP is _FLASE */
		if ((psecuritypriv->wpa_psk > 0)
		    && (pstat->flags & (WLAN_STA_WPS | WLAN_STA_MAYBE_WPS))) {
			if (pmlmepriv->wps_beacon_ie) {
				u8 selected_registrar = 0;

				rtw_get_wps_attr_content(pmlmepriv->wps_beacon_ie, pmlmepriv->wps_beacon_ie_len, WPS_ATTR_SELECTED_REGISTRAR , &selected_registrar, NULL);

				if (!selected_registrar) {
					RTW_INFO("selected_registrar is false , or AP is not ready to do WPS\n");

					status = _STATS_UNABLE_HANDLE_STA_;

					goto OnAssocReqFail;
				}
			}
		}

	} else {
		int copy_len;

		if (psecuritypriv->wpa_psk == 0) {
			RTW_INFO("STA " MAC_FMT ": WPA/RSN IE in association "
				"request, but AP don't support WPA/RSN\n", MAC_ARG(pstat->hwaddr));

			status = WLAN_STATUS_INVALID_IE;

			goto OnAssocReqFail;

		}

		if (elems.wps_ie) {
			RTW_INFO("STA included WPS IE in "
				 "(Re)Association Request - WPS is "
				 "used\n");
			pstat->flags |= WLAN_STA_WPS;
			copy_len = 0;
		} else
			copy_len = ((wpa_ie_len + 2) > sizeof(pstat->wpa_ie)) ? (sizeof(pstat->wpa_ie)) : (wpa_ie_len + 2);


		if (copy_len > 0)
			memcpy(pstat->wpa_ie, wpa_ie - 2, copy_len);

	}


	/* check if there is WMM IE & support WWM-PS */
	pstat->flags &= ~WLAN_STA_WME;
	pstat->qos_option = 0;
	pstat->qos_info = 0;
	pstat->has_legacy_ac = true;
	pstat->uapsd_vo = 0;
	pstat->uapsd_vi = 0;
	pstat->uapsd_be = 0;
	pstat->uapsd_bk = 0;
	if (pmlmepriv->qospriv.qos_option) {
		p = pframe + WLAN_HDR_A3_LEN + ie_offset;
		ie_len = 0;
		for (;;) {
			p = rtw_get_ie(p, _VENDOR_SPECIFIC_IE_, &ie_len, pkt_len - WLAN_HDR_A3_LEN - ie_offset);
			if (p != NULL) {
				if (!memcmp(p + 2, WMM_IE, 6)) {

					pstat->flags |= WLAN_STA_WME;

					pstat->qos_option = 1;
					pstat->qos_info = *(p + 8);

					pstat->max_sp_len = (pstat->qos_info >> 5) & 0x3;

					if ((pstat->qos_info & 0xf) != 0xf)
						pstat->has_legacy_ac = true;
					else
						pstat->has_legacy_ac = false;

					if (pstat->qos_info & 0xf) {
						if (pstat->qos_info & BIT(0))
							pstat->uapsd_vo = BIT(0) | BIT(1);
						else
							pstat->uapsd_vo = 0;

						if (pstat->qos_info & BIT(1))
							pstat->uapsd_vi = BIT(0) | BIT(1);
						else
							pstat->uapsd_vi = 0;

						if (pstat->qos_info & BIT(2))
							pstat->uapsd_bk = BIT(0) | BIT(1);
						else
							pstat->uapsd_bk = 0;

						if (pstat->qos_info & BIT(3))
							pstat->uapsd_be = BIT(0) | BIT(1);
						else
							pstat->uapsd_be = 0;

					}

					break;
				}
			} else
				break;
			p = p + ie_len + 2;
		}
	}


	if (pmlmepriv->htpriv.ht_option == false)
		goto bypass_ht_chk;

	/* save HT capabilities in the sta object */
	memset(&pstat->htpriv.ht_cap, 0, sizeof(struct rtw_ieee80211_ht_cap));
	if (elems.ht_capabilities && elems.ht_capabilities_len >= sizeof(struct rtw_ieee80211_ht_cap)) {
		pstat->flags |= WLAN_STA_HT;

		pstat->flags |= WLAN_STA_WME;

		memcpy(&pstat->htpriv.ht_cap, elems.ht_capabilities, sizeof(struct rtw_ieee80211_ht_cap));

	} else
		pstat->flags &= ~WLAN_STA_HT;
bypass_ht_chk:

	if ((pmlmepriv->htpriv.ht_option == false) && (pstat->flags & WLAN_STA_HT)) {
		rtw_warn_on(1);
		status = _STATS_FAILURE_;
		goto OnAssocReqFail;
	}

	if (((pstat->flags & WLAN_STA_HT) || (pstat->flags & WLAN_STA_VHT)) &&
	    ((pstat->wpa2_pairwise_cipher & WPA_CIPHER_TKIP) ||
	     (pstat->wpa_pairwise_cipher & WPA_CIPHER_TKIP))) {

		RTW_INFO("(V)HT: " MAC_FMT " tried to use TKIP with (V)HT association\n", MAC_ARG(pstat->hwaddr));

		pstat->flags &= ~WLAN_STA_HT;
		pstat->flags &= ~WLAN_STA_VHT;
		/*status = WLAN_STATUS_CIPHER_REJECTED_PER_POLICY;
		  * goto OnAssocReqFail;
		*/
	}

	/*
	 * if (hapd->iface->current_mode->mode == HOSTAPD_MODE_IEEE80211G) */ /* ? */
	pstat->flags |= WLAN_STA_NONERP;
	for (i = 0; i < pstat->bssratelen; i++) {
		if ((pstat->bssrateset[i] & 0x7f) > 22) {
			pstat->flags &= ~WLAN_STA_NONERP;
			break;
		}
	}

	if (pstat->capability & WLAN_CAPABILITY_SHORT_PREAMBLE)
		pstat->flags |= WLAN_STA_SHORT_PREAMBLE;
	else
		pstat->flags &= ~WLAN_STA_SHORT_PREAMBLE;



	if (status != _STATS_SUCCESSFUL_)
		goto OnAssocReqFail;

#ifdef CONFIG_P2P
	pstat->is_p2p_device = false;
	if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO)) {
		p2pie = rtw_get_p2p_ie(pframe + WLAN_HDR_A3_LEN + ie_offset , pkt_len - WLAN_HDR_A3_LEN - ie_offset , NULL, &p2pielen);
		if (p2pie) {
			pstat->is_p2p_device = true;
			p2p_status_code = (u8)process_assoc_req_p2p_ie(pwdinfo, pframe, pkt_len, pstat);
			if (p2p_status_code > 0) {
				pstat->p2p_status_code = p2p_status_code;
				status = _STATS_CAP_FAIL_;
				goto OnAssocReqFail;
			}
		}
#ifdef CONFIG_WFD
		rtw_process_wfd_ies(padapter, pframe + WLAN_HDR_A3_LEN + ie_offset, pkt_len - WLAN_HDR_A3_LEN - ie_offset, __func__);
#endif
	}
	pstat->p2p_status_code = p2p_status_code;
#endif /* CONFIG_P2P */

	/* TODO: identify_proprietary_vendor_ie(); */
	/* Realtek proprietary IE */
	/* identify if this is Broadcom sta */
	/* identify if this is ralink sta */
	/* Customer proprietary IE */



	/* get a unique AID */
	if (pstat->aid > 0)
		RTW_INFO("  old AID %d\n", pstat->aid);
	else {
		for (pstat->aid = 1; pstat->aid <= NUM_STA; pstat->aid++) {
			if (pstapriv->sta_aid[pstat->aid - 1] == NULL) {
				if (pstat->aid > pstapriv->max_num_sta) {
					pstat->aid = 0;

					RTW_INFO("  no room for more AIDs\n");

					status = WLAN_STATUS_AP_UNABLE_TO_HANDLE_NEW_STA;

					goto OnAssocReqFail;


				} else {
					pstapriv->sta_aid[pstat->aid - 1] = pstat;
					RTW_INFO("allocate new AID = (%d)\n", pstat->aid);
					break;
				}
			}
		}
	}


	pstat->state &= (~WIFI_FW_ASSOC_STATE);
	pstat->state |= WIFI_FW_ASSOC_SUCCESS;
	/* RTW_INFO("==================%s, %d,  (%x), bpairwise_key_installed=%d, MAC:"MAC_FMT"\n"
	, __func__, __LINE__, pstat->state, pstat->bpairwise_key_installed, MAC_ARG(pstat->hwaddr)); */
#ifdef CONFIG_IEEE80211W
	if (pstat->bpairwise_key_installed != true)
#endif /* CONFIG_IEEE80211W */
	{
		_enter_critical_bh(&pstapriv->auth_list_lock, &irqL);
		if (!list_empty(&pstat->auth_list)) {
			list_del_init(&pstat->auth_list);
			pstapriv->auth_list_cnt--;
		}
		_exit_critical_bh(&pstapriv->auth_list_lock, &irqL);

		_enter_critical_bh(&pstapriv->asoc_list_lock, &irqL);
		if (list_empty(&pstat->asoc_list)) {
			pstat->expire_to = pstapriv->expire_to;
			list_add_tail(&pstat->asoc_list, &pstapriv->asoc_list);
			pstapriv->asoc_list_cnt++;
		}
		_exit_critical_bh(&pstapriv->asoc_list_lock, &irqL);
	}

	/* now the station is qualified to join our BSS...	 */
	if (pstat && (pstat->state & WIFI_FW_ASSOC_SUCCESS) && (_STATS_SUCCESSFUL_ == status)) {
#ifdef CONFIG_NATIVEAP_MLME
#ifdef CONFIG_IEEE80211W
		if (pstat->bpairwise_key_installed != true)
#endif /* CONFIG_IEEE80211W */
		{
			/* .1 bss_cap_update & sta_info_update */
			bss_cap_update_on_sta_join(padapter, pstat);
			sta_info_update(padapter, pstat);
		}
#ifdef CONFIG_IEEE80211W
		if (pstat->bpairwise_key_installed)
			status = _STATS_REFUSED_TEMPORARILY_;
#endif /* CONFIG_IEEE80211W */
		/* .2 issue assoc rsp before notify station join event. */
		if (frame_type == WIFI_ASSOCREQ)
			issue_asocrsp(padapter, status, pstat, WIFI_ASSOCRSP);
		else
			issue_asocrsp(padapter, status, pstat, WIFI_REASSOCRSP);

#ifdef CONFIG_IOCTL_CFG80211
		_enter_critical_bh(&pstat->lock, &irqL);
		if (pstat->passoc_req) {
			rtw_mfree(pstat->passoc_req, pstat->assoc_req_len);
			pstat->passoc_req = NULL;
			pstat->assoc_req_len = 0;
		}

		pstat->passoc_req =  rtw_zmalloc(pkt_len);
		if (pstat->passoc_req) {
			memcpy(pstat->passoc_req, pframe, pkt_len);
			pstat->assoc_req_len = pkt_len;
		}
		_exit_critical_bh(&pstat->lock, &irqL);
#endif /* CONFIG_IOCTL_CFG80211 */
#ifdef CONFIG_IEEE80211W
		if (pstat->bpairwise_key_installed != true)
#endif /* CONFIG_IEEE80211W */
		{
			/* .3-(1) report sta add event */
			report_add_sta_event(padapter, pstat->hwaddr);
		}
#ifdef CONFIG_IEEE80211W
		if (pstat->bpairwise_key_installed && padapter->securitypriv.binstallBIPkey == true) {
			RTW_INFO(MAC_FMT"\n", MAC_ARG(pstat->hwaddr));
			issue_action_SA_Query(padapter, pstat->hwaddr, 0, 0, IEEE80211W_RIGHT_KEY);
		}
#endif /* CONFIG_IEEE80211W */
#endif /* CONFIG_NATIVEAP_MLME */
	}

	return _SUCCESS;

asoc_class2_error:

#ifdef CONFIG_NATIVEAP_MLME
	issue_deauth(padapter, (void *)get_addr2_ptr(pframe), status);
#endif

	return _FAIL;

OnAssocReqFail:


#ifdef CONFIG_NATIVEAP_MLME
	pstat->aid = 0;
	if (frame_type == WIFI_ASSOCREQ)
		issue_asocrsp(padapter, status, pstat, WIFI_ASSOCRSP);
	else
		issue_asocrsp(padapter, status, pstat, WIFI_REASSOCRSP);
#endif


#endif /* CONFIG_AP_MODE */

	return _FAIL;

}

unsigned int OnAssocRsp(_adapter *padapter, union recv_frame *precv_frame)
{
	uint i;
	int res;
	unsigned short	status;
	PNDIS_802_11_VARIABLE_IEs	pIE;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	/* WLAN_BSSID_EX 		*cur_network = &(pmlmeinfo->network); */
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint pkt_len = precv_frame->u.hdr.len;
	PNDIS_802_11_VARIABLE_IEs	pWapiIE = NULL;

	RTW_INFO("%s\n", __func__);

	/* check A1 matches or not */
	if (memcmp(adapter_mac_addr(padapter), get_da(pframe), ETH_ALEN))
		return _SUCCESS;

	if (!(pmlmeinfo->state & (WIFI_FW_AUTH_SUCCESS | WIFI_FW_ASSOC_STATE)))
		return _SUCCESS;

	if (pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)
		return _SUCCESS;

	_cancel_timer_ex(&pmlmeext->link_timer);

	/* status */
	status = le16_to_cpu(*(__le16 *)(pframe + WLAN_HDR_A3_LEN + 2));
	if (status > 0) {
		RTW_INFO("assoc reject, status code: %d\n", status);
		pmlmeinfo->state = WIFI_FW_NULL_STATE;
		res = -4;
		goto report_assoc_result;
	}

	/* get capabilities */
	pmlmeinfo->capability = le16_to_cpu(*(__le16 *)(pframe + WLAN_HDR_A3_LEN));

	/* set slot time */
	pmlmeinfo->slotTime = (pmlmeinfo->capability & BIT(10)) ? 9 : 20;

	/* AID */
	res = pmlmeinfo->aid = (int)(le16_to_cpu(*(__le16 *)(pframe + WLAN_HDR_A3_LEN + 4)) & 0x3fff);

	/* following are moved to join event callback function */
	/* to handle HT, WMM, rate adaptive, update MAC reg */
	/* for not to handle the synchronous IO in the tasklet */
	for (i = (6 + WLAN_HDR_A3_LEN); i < pkt_len;) {
		pIE = (PNDIS_802_11_VARIABLE_IEs)(pframe + i);

		switch (pIE->ElementID) {
		case _VENDOR_SPECIFIC_IE_:
			if (!memcmp(pIE->data, WMM_PARA_OUI, 6))	/* WMM */
				WMM_param_handler(padapter, pIE);
#if defined(CONFIG_P2P) && defined(CONFIG_WFD)
			else if (!memcmp(pIE->data, WFD_OUI, 4))		/* WFD */
				rtw_process_wfd_ie(padapter, (u8 *)pIE, pIE->Length, __func__);
#endif
			break;

#ifdef CONFIG_WAPI_SUPPORT
		case _WAPI_IE_:
			pWapiIE = pIE;
			break;
#endif

		case _HT_CAPABILITY_IE_:	/* HT caps */
			HT_caps_handler(padapter, pIE);
			break;

		case _HT_EXTRA_INFO_IE_:	/* HT info */
			HT_info_handler(padapter, pIE);
			break;

		case _ERPINFO_IE_:
			ERP_IE_handler(padapter, pIE);
			break;
#ifdef CONFIG_TDLS
		case _EXT_CAP_IE_:
			if (check_ap_tdls_prohibited(pIE->data, pIE->Length))
				padapter->tdlsinfo.ap_prohibited = true;
			if (check_ap_tdls_ch_switching_prohibited(pIE->data, pIE->Length))
				padapter->tdlsinfo.ch_switch_prohibited = true;
			break;
#endif /* CONFIG_TDLS */
		default:
			break;
		}

		i += (pIE->Length + 2);
	}

#ifdef CONFIG_WAPI_SUPPORT
	rtw_wapi_on_assoc_ok(padapter, pIE);
#endif

	pmlmeinfo->state &= (~WIFI_FW_ASSOC_STATE);
	pmlmeinfo->state |= WIFI_FW_ASSOC_SUCCESS;

	/* Update Basic Rate Table for spec, 2010-12-28 , by thomas */
	UpdateBrateTbl(padapter, pmlmeinfo->network.SupportedRates);

report_assoc_result:
	if (res > 0)
		rtw_buf_update(&pmlmepriv->assoc_rsp, &pmlmepriv->assoc_rsp_len, pframe, pkt_len);
	else
		rtw_buf_free(&pmlmepriv->assoc_rsp, &pmlmepriv->assoc_rsp_len);

	report_join_res(padapter, res);

	return _SUCCESS;
}

unsigned int OnDeAuth(_adapter *padapter, union recv_frame *precv_frame)
{
	unsigned short	reason;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8 *pframe = precv_frame->u.hdr.rx_data;
#ifdef CONFIG_P2P
	struct wifidirect_info *pwdinfo = &(padapter->wdinfo);
#endif /* CONFIG_P2P */

	/* check A3 */
	if (!(!memcmp(GetAddr3Ptr(pframe), get_my_bssid(&pmlmeinfo->network), ETH_ALEN)))
		return _SUCCESS;

	RTW_INFO(FUNC_ADPT_FMT" - Start to Disconnect\n", FUNC_ADPT_ARG(padapter));

#ifdef CONFIG_P2P
	if (pwdinfo->rx_invitereq_info.scan_op_ch_only) {
		_cancel_timer_ex(&pwdinfo->reset_ch_sitesurvey);
		_set_timer(&pwdinfo->reset_ch_sitesurvey, 10);
	}
#endif /* CONFIG_P2P */

	reason = le16_to_cpu(*(__le16 *)(pframe + WLAN_HDR_A3_LEN));

	rtw_lock_rx_suspend_timeout(8000);

#ifdef CONFIG_AP_MODE
	if (check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
		unsigned long irqL;
		struct sta_info *psta;
		struct sta_priv *pstapriv = &padapter->stapriv;

		/* _enter_critical_bh(&(pstapriv->sta_hash_lock), &irqL);		 */
		/* rtw_free_stainfo(padapter, psta); */
		/* _exit_critical_bh(&(pstapriv->sta_hash_lock), &irqL);		 */

		RTW_PRINT(FUNC_ADPT_FMT" reason=%u, ta=%pM\n"
			, FUNC_ADPT_ARG(padapter), reason, get_addr2_ptr(pframe));

		psta = rtw_get_stainfo(pstapriv, get_addr2_ptr(pframe));
		if (psta) {
			u8 updated = false;

			_enter_critical_bh(&pstapriv->asoc_list_lock, &irqL);
			if (list_empty(&psta->asoc_list) == false) {
				list_del_init(&psta->asoc_list);
				pstapriv->asoc_list_cnt--;
				updated = ap_free_sta(padapter, psta, false, reason, true);

			}
			_exit_critical_bh(&pstapriv->asoc_list_lock, &irqL);

			associated_clients_update(padapter, updated, STA_INFO_UPDATE_ALL);
		}


		return _SUCCESS;
	} else
#endif
	{
		int	ignore_received_deauth = 0;

		/*	Commented by Albert 20130604 */
		/*	Before sending the auth frame to start the STA/GC mode connection with AP/GO,  */
		/*	we will send the deauth first. */
		/*	However, the Win8.1 with BRCM Wi-Fi will send the deauth with reason code 6 to us after receieving our deauth. */
		/*	Added the following code to avoid this case. */
		if ((pmlmeinfo->state & WIFI_FW_AUTH_STATE) ||
		    (pmlmeinfo->state & WIFI_FW_ASSOC_STATE)) {
			if (reason == WLAN_REASON_CLASS2_FRAME_FROM_NONAUTH_STA)
				ignore_received_deauth = 1;
			else if (WLAN_REASON_PREV_AUTH_NOT_VALID == reason) {
				/* TODO: 802.11r */
				ignore_received_deauth = 1;
			}
		}

		RTW_PRINT(FUNC_ADPT_FMT" reason=%u, ta=%pM, ignore=%d\n"
			, FUNC_ADPT_ARG(padapter), reason, get_addr2_ptr(pframe), ignore_received_deauth);

		if (0 == ignore_received_deauth)
			receive_disconnect(padapter, get_addr2_ptr(pframe), reason, false);
	}
	pmlmepriv->LinkDetectInfo.bBusyTraffic = false;
	return _SUCCESS;

}

unsigned int OnDisassoc(_adapter *padapter, union recv_frame *precv_frame)
{
	unsigned short	reason;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8 *pframe = precv_frame->u.hdr.rx_data;
#ifdef CONFIG_P2P
	struct wifidirect_info *pwdinfo = &(padapter->wdinfo);
#endif /* CONFIG_P2P */

	/* check A3 */
	if (!(!memcmp(GetAddr3Ptr(pframe), get_my_bssid(&pmlmeinfo->network), ETH_ALEN)))
		return _SUCCESS;

	RTW_INFO(FUNC_ADPT_FMT" - Start to Disconnect\n", FUNC_ADPT_ARG(padapter));

#ifdef CONFIG_P2P
	if (pwdinfo->rx_invitereq_info.scan_op_ch_only) {
		_cancel_timer_ex(&pwdinfo->reset_ch_sitesurvey);
		_set_timer(&pwdinfo->reset_ch_sitesurvey, 10);
	}
#endif /* CONFIG_P2P */

	reason = le16_to_cpu(*(__le16 *)(pframe + WLAN_HDR_A3_LEN));

	rtw_lock_rx_suspend_timeout(8000);

#ifdef CONFIG_AP_MODE
	if (check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
		unsigned long irqL;
		struct sta_info *psta;
		struct sta_priv *pstapriv = &padapter->stapriv;

		/* _enter_critical_bh(&(pstapriv->sta_hash_lock), &irqL);	 */
		/* rtw_free_stainfo(padapter, psta); */
		/* _exit_critical_bh(&(pstapriv->sta_hash_lock), &irqL);		 */

		RTW_PRINT(FUNC_ADPT_FMT" reason=%u, ta=%pM\n"
			, FUNC_ADPT_ARG(padapter), reason, get_addr2_ptr(pframe));

		psta = rtw_get_stainfo(pstapriv, get_addr2_ptr(pframe));
		if (psta) {
			u8 updated = false;

			_enter_critical_bh(&pstapriv->asoc_list_lock, &irqL);
			if (list_empty(&psta->asoc_list) == false) {
				list_del_init(&psta->asoc_list);
				pstapriv->asoc_list_cnt--;
				updated = ap_free_sta(padapter, psta, false, reason, true);

			}
			_exit_critical_bh(&pstapriv->asoc_list_lock, &irqL);

			associated_clients_update(padapter, updated, STA_INFO_UPDATE_ALL);
		}

		return _SUCCESS;
	} else
#endif
	{
		RTW_PRINT(FUNC_ADPT_FMT" reason=%u, ta=%pM\n"
			, FUNC_ADPT_ARG(padapter), reason, get_addr2_ptr(pframe));

		receive_disconnect(padapter, get_addr2_ptr(pframe), reason, false);
	}
	pmlmepriv->LinkDetectInfo.bBusyTraffic = false;
	return _SUCCESS;

}

unsigned int OnAtim(_adapter *padapter, union recv_frame *precv_frame)
{
	RTW_INFO("%s\n", __func__);
	return _SUCCESS;
}

static unsigned int on_action_spct_ch_switch(_adapter *padapter, struct sta_info *psta, u8 *ies, uint ies_len)
{
	unsigned int ret = _FAIL;
	struct mlme_ext_priv *mlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(mlmeext->mlmext_info);

	if (!(pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)) {
		ret = _SUCCESS;
		goto exit;
	}

	if ((pmlmeinfo->state & 0x03) == WIFI_FW_STATION_STATE) {

		int ch_switch_mode = -1, ch = -1, ch_switch_cnt = -1;
		int ch_offset = -1;
		u8 bwmode;
		struct ieee80211_info_element *ie;

		RTW_INFO(FUNC_NDEV_FMT" from "MAC_FMT"\n",
			FUNC_NDEV_ARG(padapter->pnetdev), MAC_ARG(psta->hwaddr));

		for_each_ie(ie, ies, ies_len) {
			if (ie->id == WLAN_EID_CHANNEL_SWITCH) {
				ch_switch_mode = ie->data[0];
				ch = ie->data[1];
				ch_switch_cnt = ie->data[2];
				RTW_INFO("ch_switch_mode:%d, ch:%d, ch_switch_cnt:%d\n",
					 ch_switch_mode, ch, ch_switch_cnt);
			} else if (ie->id == WLAN_EID_SECONDARY_CHANNEL_OFFSET) {
				ch_offset = secondary_ch_offset_to_hal_ch_offset(ie->data[0]);
				RTW_INFO("ch_offset:%d\n", ch_offset);
			}
		}

		if (ch == -1)
			return _SUCCESS;

		if (ch_offset == -1)
			bwmode = mlmeext->cur_bwmode;
		else
			bwmode = (ch_offset == HAL_PRIME_CHNL_OFFSET_DONT_CARE) ?
				 CHANNEL_WIDTH_20 : CHANNEL_WIDTH_40;

		ch_offset = (ch_offset == -1) ? mlmeext->cur_ch_offset : ch_offset;

		/* todo:
		 * 1. the decision of channel switching
		 * 2. things after channel switching
		 */

		ret = rtw_set_ch_cmd(padapter, ch, bwmode, ch_offset, true);
	}

exit:
	return ret;
}

unsigned int on_action_spct(_adapter *padapter, union recv_frame *precv_frame)
{
	unsigned int ret = _FAIL;
	struct sta_info *psta = NULL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint frame_len = precv_frame->u.hdr.len;
	u8 *frame_body = (u8 *)(pframe + sizeof(struct rtw_ieee80211_hdr_3addr));
	u8 category;
	u8 action;

	RTW_INFO(FUNC_NDEV_FMT"\n", FUNC_NDEV_ARG(padapter->pnetdev));

	psta = rtw_get_stainfo(pstapriv, get_addr2_ptr(pframe));

	if (!psta)
		goto exit;

	category = frame_body[0];
	if (category != RTW_WLAN_CATEGORY_SPECTRUM_MGMT)
		goto exit;

	action = frame_body[1];
	switch (action) {
	case RTW_WLAN_ACTION_SPCT_MSR_REQ:
	case RTW_WLAN_ACTION_SPCT_MSR_RPRT:
	case RTW_WLAN_ACTION_SPCT_TPC_REQ:
	case RTW_WLAN_ACTION_SPCT_TPC_RPRT:
		break;
	case RTW_WLAN_ACTION_SPCT_CHL_SWITCH:
#ifdef CONFIG_SPCT_CH_SWITCH
		ret = on_action_spct_ch_switch(padapter, psta, &frame_body[2],
				       frame_len - (frame_body - pframe) - 2);
#endif
		break;
	default:
		break;
	}

exit:
	return ret;
}

unsigned int OnAction_qos(_adapter *padapter, union recv_frame *precv_frame)
{
	return _SUCCESS;
}

unsigned int OnAction_dls(_adapter *padapter, union recv_frame *precv_frame)
{
	return _SUCCESS;
}

#ifdef CONFIG_RTW_WNM
unsigned int on_action_wnm(_adapter *adapter, union recv_frame *rframe)
{
	unsigned int ret = _FAIL;
	struct sta_info *sta = NULL;
	struct sta_priv *stapriv = &adapter->stapriv;
	u8 *frame = rframe->u.hdr.rx_data;
	uint frame_len = rframe->u.hdr.len;
	u8 *frame_body = (u8 *)(frame + sizeof(struct rtw_ieee80211_hdr_3addr));
	u8 category;
	u8 action;
	int cnt = 0;
	char msg[16];

	sta = rtw_get_stainfo(stapriv, get_addr2_ptr(frame));
	if (!sta)
		goto exit;

	category = frame_body[0];
	if (category != RTW_WLAN_CATEGORY_WNM)
		goto exit;

	action = frame_body[1];

	switch (action) {
	default:
		#ifdef CONFIG_IOCTL_CFG80211
		cnt += sprintf((msg + cnt), "ACT_WNM %u", action);
		rtw_cfg80211_rx_action(adapter, rframe, msg);
		#endif
		ret = _SUCCESS;
		break;
	}

exit:
	return ret;
}
#endif /* CONFIG_RTW_WNM */

/**
 * rtw_rx_ampdu_size - Get the target RX AMPDU buffer size for the specific @adapter
 * @adapter: the adapter to get target RX AMPDU buffer size
 *
 * Returns: the target RX AMPDU buffer size
 */
u8 rtw_rx_ampdu_size(_adapter *adapter)
{
	u8 size;
	HT_CAP_AMPDU_FACTOR max_rx_ampdu_factor;

	if (adapter->fix_rx_ampdu_size != RX_AMPDU_SIZE_INVALID) {
		size = adapter->fix_rx_ampdu_size;
		goto exit;
	}

#ifdef CONFIG_BT_COEXIST
	if (rtw_btcoex_IsBTCoexCtrlAMPDUSize(adapter)) {
		size = rtw_btcoex_GetAMPDUSize(adapter);
		goto exit;
	}
#endif

	/* for scan */
	if (!mlmeext_chk_scan_state(&adapter->mlmeextpriv, SCAN_DISABLE)
	    && !mlmeext_chk_scan_state(&adapter->mlmeextpriv, SCAN_COMPLETE)
	    && adapter->mlmeextpriv.sitesurvey_res.rx_ampdu_size != RX_AMPDU_SIZE_INVALID
	   ) {
		size = adapter->mlmeextpriv.sitesurvey_res.rx_ampdu_size;
		goto exit;
	}

	/* default value based on max_rx_ampdu_factor */
	if (adapter->driver_rx_ampdu_factor != 0xFF)
		max_rx_ampdu_factor = (HT_CAP_AMPDU_FACTOR)adapter->driver_rx_ampdu_factor;
	else
		rtw_hal_get_def_var(adapter, HW_VAR_MAX_RX_AMPDU_FACTOR, &max_rx_ampdu_factor);
	
	/* In Maximum A-MPDU Length Exponent subfield of A-MPDU Parameters field of HT Capabilities element,
		the unit of max_rx_ampdu_factor are octets. 8K, 16K, 32K, 64K is right.
		But the buffer size subfield of Block Ack Parameter Set field in ADDBA action frame indicates
		the number of buffers available for this particular TID. Each buffer is equal to max. size of 
		MSDU or AMSDU. 
		The size variable means how many MSDUs or AMSDUs, it's not Kbytes.
	*/
	if (MAX_AMPDU_FACTOR_64K == max_rx_ampdu_factor)
		size = 64;
	else if (MAX_AMPDU_FACTOR_32K == max_rx_ampdu_factor)
		size = 32;
	else if (MAX_AMPDU_FACTOR_16K == max_rx_ampdu_factor)
		size = 16;
	else if (MAX_AMPDU_FACTOR_8K == max_rx_ampdu_factor)
		size = 8;
	else
		size = 64;

exit:

	if (size > 127)
		size = 127;

	return size;
}

/**
 * rtw_rx_ampdu_is_accept - Get the permission if RX AMPDU should be set up for the specific @adapter
 * @adapter: the adapter to get the permission if RX AMPDU should be set up
 *
 * Returns: accept or not
 */
bool rtw_rx_ampdu_is_accept(_adapter *adapter)
{
	bool accept;

	if (adapter->fix_rx_ampdu_accept != RX_AMPDU_ACCEPT_INVALID) {
		accept = adapter->fix_rx_ampdu_accept;
		goto exit;
	}

#ifdef CONFIG_BT_COEXIST
	if (rtw_btcoex_IsBTCoexRejectAMPDU(adapter)) {
		accept = false;
		goto exit;
	}
#endif

	/* for scan */
	if (!mlmeext_chk_scan_state(&adapter->mlmeextpriv, SCAN_DISABLE)
	    && !mlmeext_chk_scan_state(&adapter->mlmeextpriv, SCAN_COMPLETE)
	    && adapter->mlmeextpriv.sitesurvey_res.rx_ampdu_accept != RX_AMPDU_ACCEPT_INVALID
	   ) {
		accept = adapter->mlmeextpriv.sitesurvey_res.rx_ampdu_accept;
		goto exit;
	}

	/* default value for other cases */
	accept = adapter->mlmeextpriv.mlmext_info.bAcceptAddbaReq;

exit:
	return accept;
}

/**
 * rtw_rx_ampdu_set_size - Set the target RX AMPDU buffer size for the specific @adapter and specific @reason
 * @adapter: the adapter to set target RX AMPDU buffer size
 * @size: the target RX AMPDU buffer size to set
 * @reason: reason for the target RX AMPDU buffer size setting
 *
 * Returns: whether the target RX AMPDU buffer size is changed
 */
bool rtw_rx_ampdu_set_size(_adapter *adapter, u8 size, u8 reason)
{
	bool is_adj = false;
	struct mlme_ext_priv *mlmeext;
	struct mlme_ext_info *mlmeinfo;

	mlmeext = &adapter->mlmeextpriv;
	mlmeinfo = &mlmeext->mlmext_info;

	if (reason == RX_AMPDU_DRV_FIXED) {
		if (adapter->fix_rx_ampdu_size != size) {
			adapter->fix_rx_ampdu_size = size;
			is_adj = true;
			RTW_INFO(FUNC_ADPT_FMT" fix_rx_ampdu_size:%u\n", FUNC_ADPT_ARG(adapter), size);
		}
	} else if (reason == RX_AMPDU_DRV_SCAN) {
		struct ss_res *ss = &adapter->mlmeextpriv.sitesurvey_res;

		if (ss->rx_ampdu_size != size) {
			ss->rx_ampdu_size = size;
			is_adj = true;
			RTW_INFO(FUNC_ADPT_FMT" ss.rx_ampdu_size:%u\n", FUNC_ADPT_ARG(adapter), size);
		}
	}

	return is_adj;
}

/**
 * rtw_rx_ampdu_set_accept - Set the permission if RX AMPDU should be set up for the specific @adapter and specific @reason
 * @adapter: the adapter to set if RX AMPDU should be set up
 * @accept: if RX AMPDU should be set up
 * @reason: reason for the permission if RX AMPDU should be set up
 *
 * Returns: whether the permission if RX AMPDU should be set up is changed
 */
bool rtw_rx_ampdu_set_accept(_adapter *adapter, u8 accept, u8 reason)
{
	bool is_adj = false;
	struct mlme_ext_priv *mlmeext;
	struct mlme_ext_info *mlmeinfo;

	mlmeext = &adapter->mlmeextpriv;
	mlmeinfo = &mlmeext->mlmext_info;

	if (reason == RX_AMPDU_DRV_FIXED) {
		if (adapter->fix_rx_ampdu_accept != accept) {
			adapter->fix_rx_ampdu_accept = accept;
			is_adj = true;
			RTW_INFO(FUNC_ADPT_FMT" fix_rx_ampdu_accept:%u\n", FUNC_ADPT_ARG(adapter), accept);
		}
	} else if (reason == RX_AMPDU_DRV_SCAN) {
		if (adapter->mlmeextpriv.sitesurvey_res.rx_ampdu_accept != accept) {
			adapter->mlmeextpriv.sitesurvey_res.rx_ampdu_accept = accept;
			is_adj = true;
			RTW_INFO(FUNC_ADPT_FMT" ss.rx_ampdu_accept:%u\n", FUNC_ADPT_ARG(adapter), accept);
		}
	}

	return is_adj;
}

/**
 * rx_ampdu_apply_sta_tid - Apply RX AMPDU setting to the specific @sta and @tid
 * @adapter: the adapter to which @sta belongs
 * @sta: the sta to be checked
 * @tid: the tid to be checked
 * @accept: the target permission if RX AMPDU should be set up
 * @size: the target RX AMPDU buffer size
 *
 * Returns:
 * 0: no canceled
 * 1: canceled by no permission
 * 2: canceled by different buffer size
 * 3: canceled by potential mismatched status
 *
 * Blocking function, may sleep
 */
u8 rx_ampdu_apply_sta_tid(_adapter *adapter, struct sta_info *sta, u8 tid, u8 accept, u8 size)
{
	u8 ret = 0;
	struct recv_reorder_ctrl *reorder_ctl = &sta->recvreorder_ctrl[tid];

	if (reorder_ctl->enable == false) {
		if (reorder_ctl->ampdu_size != RX_AMPDU_SIZE_INVALID) {
			send_delba_sta_tid_wait_ack(adapter, 0, sta, tid, 1);
			ret = 3;
		}
		goto exit;
	}

	if (accept == false) {
		send_delba_sta_tid_wait_ack(adapter, 0, sta, tid, 0);
		ret = 1;
	} else if (reorder_ctl->ampdu_size != size) {
		send_delba_sta_tid_wait_ack(adapter, 0, sta, tid, 0);
		ret = 2;
	}

exit:
	return ret;
}

/**
 * rx_ampdu_apply_sta - Apply RX AMPDU setting to the specific @sta
 * @adapter: the adapter to which @sta belongs
 * @sta: the sta to be checked
 * @accept: the target permission if RX AMPDU should be set up
 * @size: the target RX AMPDU buffer size
 *
 * Returns: number of the RX AMPDU assciation canceled for applying current target setting
 *
 * Blocking function, may sleep
 */
u8 rx_ampdu_apply_sta(_adapter *adapter, struct sta_info *sta, u8 accept, u8 size)
{
	u8 change_cnt = 0;
	int i;

	for (i = 0; i < TID_NUM; i++) {
		if (rx_ampdu_apply_sta_tid(adapter, sta, i, accept, size) != 0)
			change_cnt++;
	}

	return change_cnt;
}

/**
 * rtw_rx_ampdu_apply - Apply the current target RX AMPDU setting for the specific @adapter
 * @adapter: the adapter to be applied
 *
 * Returns: number of the RX AMPDU assciation canceled for applying current target setting
 */
u16 rtw_rx_ampdu_apply(_adapter *adapter)
{
	u16 adj_cnt = 0;
	struct mlme_ext_priv *mlmeext;
	struct sta_info *sta;
	u8 accept = rtw_rx_ampdu_is_accept(adapter);
	u8 size = rtw_rx_ampdu_size(adapter);

	mlmeext = &adapter->mlmeextpriv;

	if (mlmeext_msr(mlmeext) == WIFI_FW_STATION_STATE) {
		sta = rtw_get_stainfo(&adapter->stapriv, get_bssid(&adapter->mlmepriv));
		if (sta)
			adj_cnt += rx_ampdu_apply_sta(adapter, sta, accept, size);

	} else if (mlmeext_msr(mlmeext) == WIFI_FW_AP_STATE) {
		unsigned long irqL;
		_list *phead, *plist;
		u8 peer_num = 0;
		char peers[NUM_STA];
		struct sta_priv *pstapriv = &adapter->stapriv;
		int i;

		_enter_critical_bh(&pstapriv->asoc_list_lock, &irqL);

		phead = &pstapriv->asoc_list;
		plist = get_next(phead);

		while ((rtw_end_of_queue_search(phead, plist)) == false) {
			int stainfo_offset;

			sta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
			plist = get_next(plist);

			stainfo_offset = rtw_stainfo_offset(pstapriv, sta);
			if (stainfo_offset_valid(stainfo_offset))
				peers[peer_num++] = stainfo_offset;
		}

		_exit_critical_bh(&pstapriv->asoc_list_lock, &irqL);

		for (i = 0; i < peer_num; i++) {
			sta = rtw_get_stainfo_by_offset(pstapriv, peers[i]);
			if (sta)
				adj_cnt += rx_ampdu_apply_sta(adapter, sta, accept, size);
		}
	}

	return adj_cnt;
}

unsigned int OnAction_back(_adapter *padapter, union recv_frame *precv_frame)
{
	u8 *addr;
	struct sta_info *psta = NULL;
	struct recv_reorder_ctrl *preorder_ctrl;
	unsigned char		*frame_body;
	unsigned char		category, action;
	unsigned short	tid, status, reason_code = 0;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8 *pframe = precv_frame->u.hdr.rx_data;
	struct sta_priv *pstapriv = &padapter->stapriv;


	RTW_INFO("%s\n", __func__);

	/* check RA matches or not	 */
	if (memcmp(adapter_mac_addr(padapter), GetAddr1Ptr(pframe), ETH_ALEN))
		return _SUCCESS;

	if ((pmlmeinfo->state & 0x03) != WIFI_FW_AP_STATE)
		if (!(pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS))
			return _SUCCESS;

	addr = get_addr2_ptr(pframe);
	psta = rtw_get_stainfo(pstapriv, addr);

	if (psta == NULL)
		return _SUCCESS;

	frame_body = (unsigned char *)(pframe + sizeof(struct rtw_ieee80211_hdr_3addr));

	category = frame_body[0];
	if (category == RTW_WLAN_CATEGORY_BACK) { /* representing Block Ack */
#ifdef CONFIG_TDLS
		if ((psta->tdls_sta_state & TDLS_LINKED_STATE) &&
		    (psta->htpriv.ht_option) &&
		    (psta->htpriv.ampdu_enable))
			RTW_INFO("Recv [%s] from direc link\n", __func__);
		else
#endif /* CONFIG_TDLS */
			if (!pmlmeinfo->HT_enable)
				return _SUCCESS;

		action = frame_body[1];
		RTW_INFO("%s, action=%d\n", __func__, action);
		switch (action) {
		case RTW_WLAN_ACTION_ADDBA_REQ: /* ADDBA request */

			memcpy(&(pmlmeinfo->ADDBA_req), &(frame_body[2]), sizeof(struct ADDBA_request));
			/* process_addba_req(padapter, (u8*)&(pmlmeinfo->ADDBA_req), GetAddr3Ptr(pframe)); */
			process_addba_req(padapter, (u8 *)&(pmlmeinfo->ADDBA_req), addr);

			break;

		case RTW_WLAN_ACTION_ADDBA_RESP: /* ADDBA response */

			/* status = frame_body[3] | (frame_body[4] << 8); */ /* endian issue */
			status = RTW_GET_LE16(&frame_body[3]);
			tid = ((frame_body[5] >> 2) & 0x7);
			if (status == 0) {
				/* successful					 */
				RTW_INFO("agg_enable for TID=%d\n", tid);
				psta->htpriv.agg_enable_bitmap |= 1 << tid;
				psta->htpriv.candidate_tid_bitmap &= ~BIT(tid);
				/* amsdu in ampdu */
				if (frame_body[5] & 1)
					psta->htpriv.tx_amsdu_enable = true;
			} else
				psta->htpriv.agg_enable_bitmap &= ~BIT(tid);

			if (psta->state & WIFI_STA_ALIVE_CHK_STATE) {
				RTW_INFO("%s alive check - rx ADDBA response\n", __func__);
				psta->htpriv.agg_enable_bitmap &= ~BIT(tid);
				psta->expire_to = pstapriv->expire_to;
				psta->state ^= WIFI_STA_ALIVE_CHK_STATE;
			}

			/* RTW_INFO("marc: ADDBA RSP: %x\n", pmlmeinfo->agg_enable_bitmap); */
			break;

		case RTW_WLAN_ACTION_DELBA: /* DELBA */
			if ((frame_body[3] & BIT(3)) == 0) {
				psta->htpriv.agg_enable_bitmap &= ~(1 << ((frame_body[3] >> 4) & 0xf));
				psta->htpriv.candidate_tid_bitmap &= ~(1 << ((frame_body[3] >> 4) & 0xf));

				/* reason_code = frame_body[4] | (frame_body[5] << 8); */
				reason_code = RTW_GET_LE16(&frame_body[4]);
			} else if ((frame_body[3] & BIT(3)) == BIT(3)) {
				tid = (frame_body[3] >> 4) & 0x0F;

				preorder_ctrl = &psta->recvreorder_ctrl[tid];
				preorder_ctrl->enable = false;
				preorder_ctrl->ampdu_size = RX_AMPDU_SIZE_INVALID;
			}

			RTW_INFO("%s(): DELBA: %x(%x)\n", __func__, pmlmeinfo->agg_enable_bitmap, reason_code);
			/* todo: how to notify the host while receiving DELETE BA */
			break;

		default:
			break;
		}
	}
	return _SUCCESS;
}

#ifdef CONFIG_P2P

static int get_reg_classes_full_count(struct p2p_channels channel_list)
{
	int cnt = 0;
	int i;

	for (i = 0; i < channel_list.reg_classes; i++)
		cnt += channel_list.reg_class[i].channels;

	return cnt;
}

static void get_channel_cnt_24g_5gl_5gh(struct mlme_ext_priv *pmlmeext, u8 *p24g_cnt, u8 *p5gl_cnt, u8 *p5gh_cnt)
{
	int	i = 0;

	*p24g_cnt = 0;
	*p5gl_cnt = 0;
	*p5gh_cnt = 0;

	for (i = 0; i < pmlmeext->max_chan_nums; i++) {
		if (pmlmeext->channel_set[i].ChannelNum <= 14)
			(*p24g_cnt)++;
		else if ((pmlmeext->channel_set[i].ChannelNum > 14) && (pmlmeext->channel_set[i].ChannelNum <= 48)) {
			/*	Just include the channel 36, 40, 44, 48 channels for 5G low */
			(*p5gl_cnt)++;
		} else if ((pmlmeext->channel_set[i].ChannelNum >= 149) && (pmlmeext->channel_set[i].ChannelNum <= 161)) {
			/*	Just include the channel 149, 153, 157, 161 channels for 5G high */
			(*p5gh_cnt)++;
		}
	}
}

void issue_p2p_GO_request(_adapter *padapter, u8 *raddr)
{

	unsigned char category = RTW_WLAN_CATEGORY_PUBLIC;
	u8			action = P2P_PUB_ACTION_ACTION;
	__be32			p2poui = cpu_to_be32(P2POUI);
	u8			oui_subtype = P2P_GO_NEGO_REQ;
	u8			wpsie[255] = { 0x00 }, p2pie[255] = { 0x00 };
	u8			wpsielen = 0, p2pielen = 0, i;
	u8			channel_cnt_24g = 0, channel_cnt_5gl = 0, channel_cnt_5gh = 0;
	u16			len_channellist_attr = 0;
#ifdef CONFIG_WFD
	u32					wfdielen = 0;
#endif

	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);


	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return;

	RTW_INFO("[%s] In\n", __func__);
	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	memcpy(pwlanhdr->addr1, raddr, ETH_ALEN);
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	memcpy(pwlanhdr->addr3, adapter_mac_addr(padapter), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 4, (unsigned char *) &(p2poui), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(oui_subtype), &(pattrib->pktlen));
	pwdinfo->negotiation_dialog_token = 1;	/*	Initialize the dialog value */
	pframe = rtw_set_fixed_ie(pframe, 1, &pwdinfo->negotiation_dialog_token, &(pattrib->pktlen));



	/*	WPS Section */
	wpsielen = 0;
	/*	WPS OUI */
	*(__be32 *)(wpsie) = cpu_to_be32(WPSOUI);
	wpsielen += 4;

	/*	WPS version */
	/*	Type: */
	*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_VER1);
	wpsielen += 2;

	/*	Length: */
	*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0001);
	wpsielen += 2;

	/*	Value: */
	wpsie[wpsielen++] = WPS_VERSION_1;	/*	Version 1.0 */

	/*	Device Password ID */
	/*	Type: */
	*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_DEVICE_PWID);
	wpsielen += 2;

	/*	Length: */
	*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0002);
	wpsielen += 2;

	/*	Value: */

	if (pwdinfo->ui_got_wps_info == P2P_GOT_WPSINFO_PEER_DISPLAY_PIN)
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_DPID_USER_SPEC);
	else if (pwdinfo->ui_got_wps_info == P2P_GOT_WPSINFO_SELF_DISPLAY_PIN)
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_DPID_REGISTRAR_SPEC);
	else if (pwdinfo->ui_got_wps_info == P2P_GOT_WPSINFO_PBC)
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_DPID_PBC);

	wpsielen += 2;

	pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, wpsielen, (unsigned char *) wpsie, &pattrib->pktlen);


	/*	P2P IE Section. */

	/*	P2P OUI */
	p2pielen = 0;
	p2pie[p2pielen++] = 0x50;
	p2pie[p2pielen++] = 0x6F;
	p2pie[p2pielen++] = 0x9A;
	p2pie[p2pielen++] = 0x09;	/*	WFA P2P v1.0 */

	/*	Commented by Albert 20110306 */
	/*	According to the P2P Specification, the group negoitation request frame should contain 9 P2P attributes */
	/*	1. P2P Capability */
	/*	2. Group Owner Intent */
	/*	3. Configuration Timeout */
	/*	4. Listen Channel */
	/*	5. Extended Listen Timing */
	/*	6. Intended P2P Interface Address */
	/*	7. Channel List */
	/*	8. P2P Device Info */
	/*	9. Operating Channel */


	/*	P2P Capability */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_CAPABILITY;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0002);
	p2pielen += 2;

	/*	Value: */
	/*	Device Capability Bitmap, 1 byte */
	p2pie[p2pielen++] = DMP_P2P_DEVCAP_SUPPORT;

	/*	Group Capability Bitmap, 1 byte */
	if (pwdinfo->persistent_supported)
		p2pie[p2pielen++] = P2P_GRPCAP_CROSS_CONN | P2P_GRPCAP_PERSISTENT_GROUP;
	else
		p2pie[p2pielen++] = P2P_GRPCAP_CROSS_CONN;


	/*	Group Owner Intent */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_GO_INTENT;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0001);
	p2pielen += 2;

	/*	Value: */
	/*	Todo the tie breaker bit. */
	p2pie[p2pielen++] = ((pwdinfo->intent << 1) &  0xFE);

	/*	Configuration Timeout */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_CONF_TIMEOUT;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0002);
	p2pielen += 2;

	/*	Value: */
	p2pie[p2pielen++] = 200;	/*	2 seconds needed to be the P2P GO */
	p2pie[p2pielen++] = 200;	/*	2 seconds needed to be the P2P Client */


	/*	Listen Channel */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_LISTEN_CH;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0005);
	p2pielen += 2;

	/*	Value: */
	/*	Country String */
	p2pie[p2pielen++] = 'X';
	p2pie[p2pielen++] = 'X';

	/*	The third byte should be set to 0x04. */
	/*	Described in the "Operating Channel Attribute" section. */
	p2pie[p2pielen++] = 0x04;

	/*	Operating Class */
	p2pie[p2pielen++] = 0x51;	/*	Copy from SD7 */

	/*	Channel Number */
	p2pie[p2pielen++] = pwdinfo->listen_channel;	/*	listening channel number */


	/*	Extended Listen Timing ATTR */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_EX_LISTEN_TIMING;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0004);
	p2pielen += 2;

	/*	Value: */
	/*	Availability Period */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0xFFFF);
	p2pielen += 2;

	/*	Availability Interval */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0xFFFF);
	p2pielen += 2;


	/*	Intended P2P Interface Address */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_INTENDED_IF_ADDR;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(ETH_ALEN);
	p2pielen += 2;

	/*	Value: */
	memcpy(p2pie + p2pielen, adapter_mac_addr(padapter), ETH_ALEN);
	p2pielen += ETH_ALEN;


	/*	Channel List */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_CH_LIST;

	/* Length: */
	/* Country String(3) */
	/* + ( Operating Class (1) + Number of Channels(1) ) * Operation Classes (?) */
	/* + number of channels in all classes */
	len_channellist_attr = 3
		       + (1 + 1) * (u16)(pmlmeext->channel_list.reg_classes)
		       + get_reg_classes_full_count(pmlmeext->channel_list);

#ifdef CONFIG_CONCURRENT_MODE
	if (rtw_mi_check_status(padapter, MI_LINKED) && padapter->registrypriv.full_ch_in_p2p_handshake == 0)
		*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(5 + 1);
	else
		*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(len_channellist_attr);
#else

	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(len_channellist_attr);

#endif
	p2pielen += 2;

	/*	Value: */
	/*	Country String */
	p2pie[p2pielen++] = 'X';
	p2pie[p2pielen++] = 'X';

	/*	The third byte should be set to 0x04. */
	/*	Described in the "Operating Channel Attribute" section. */
	p2pie[p2pielen++] = 0x04;

	/*	Channel Entry List */

#ifdef CONFIG_CONCURRENT_MODE
	if (rtw_mi_check_status(padapter, MI_LINKED) && padapter->registrypriv.full_ch_in_p2p_handshake == 0) {
		u8 union_ch = rtw_mi_get_union_chan(padapter);

		/*	Operating Class */
		if (union_ch > 14) {
			if (union_ch >= 149)
				p2pie[p2pielen++] = 0x7c;
			else
				p2pie[p2pielen++] = 0x73;
		} else
			p2pie[p2pielen++] = 0x51;


		/*	Number of Channels */
		/*	Just support 1 channel and this channel is AP's channel */
		p2pie[p2pielen++] = 1;

		/*	Channel List */
		p2pie[p2pielen++] = union_ch;
	} else {
		int i, j;
		for (j = 0; j < pmlmeext->channel_list.reg_classes; j++) {
			/*	Operating Class */
			p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].reg_class;

			/*	Number of Channels */
			p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channels;

			/*	Channel List */
			for (i = 0; i < pmlmeext->channel_list.reg_class[j].channels; i++)
				p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channel[i];
		}
	}
#else /* CONFIG_CONCURRENT_MODE */
	{
		int i, j;
		for (j = 0; j < pmlmeext->channel_list.reg_classes; j++) {
			/*	Operating Class */
			p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].reg_class;

			/*	Number of Channels */
			p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channels;

			/*	Channel List */
			for (i = 0; i < pmlmeext->channel_list.reg_class[j].channels; i++)
				p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channel[i];
		}
	}
#endif /* CONFIG_CONCURRENT_MODE */

	/*	Device Info */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_DEVICE_INFO;

	/*	Length: */
	/*	21->P2P Device Address (6bytes) + Config Methods (2bytes) + Primary Device Type (8bytes)  */
	/*	+ NumofSecondDevType (1byte) + WPS Device Name ID field (2bytes) + WPS Device Name Len field (2bytes) */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(21 + pwdinfo->device_name_len);
	p2pielen += 2;

	/*	Value: */
	/*	P2P Device Address */
	memcpy(p2pie + p2pielen, adapter_mac_addr(padapter), ETH_ALEN);
	p2pielen += ETH_ALEN;

	/*	Config Method */
	/*	This field should be big endian. Noted by P2P specification. */

	*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(pwdinfo->supported_wps_cm);

	p2pielen += 2;

	/*	Primary Device Type */
	/*	Category ID */
	*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(WPS_PDT_CID_MULIT_MEDIA);
	p2pielen += 2;

	/*	OUI */
	*(__be32 *)(p2pie + p2pielen) = cpu_to_be32(WPSOUI);
	p2pielen += 4;

	/*	Sub Category ID */
	*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(WPS_PDT_SCID_MEDIA_SERVER);
	p2pielen += 2;

	/*	Number of Secondary Device Types */
	p2pie[p2pielen++] = 0x00;	/*	No Secondary Device Type List */

	/*	Device Name */
	/*	Type: */
	*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(WPS_ATTR_DEVICE_NAME);
	p2pielen += 2;

	/*	Length: */
	*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(pwdinfo->device_name_len);
	p2pielen += 2;

	/*	Value: */
	memcpy(p2pie + p2pielen, pwdinfo->device_name , pwdinfo->device_name_len);
	p2pielen += pwdinfo->device_name_len;


	/*	Operating Channel */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_OPERATING_CH;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0005);
	p2pielen += 2;

	/*	Value: */
	/*	Country String */
	p2pie[p2pielen++] = 'X';
	p2pie[p2pielen++] = 'X';

	/*	The third byte should be set to 0x04. */
	/*	Described in the "Operating Channel Attribute" section. */
	p2pie[p2pielen++] = 0x04;

	/*	Operating Class */
	if (pwdinfo->operating_channel <= 14) {
		/*	Operating Class */
		p2pie[p2pielen++] = 0x51;
	} else if ((pwdinfo->operating_channel >= 36) && (pwdinfo->operating_channel <= 48)) {
		/*	Operating Class */
		p2pie[p2pielen++] = 0x73;
	} else {
		/*	Operating Class */
		p2pie[p2pielen++] = 0x7c;
	}

	/*	Channel Number */
	p2pie[p2pielen++] = pwdinfo->operating_channel;	/*	operating channel number */

	pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &pattrib->pktlen);

#ifdef CONFIG_WFD
	wfdielen = build_nego_req_wfd_ie(pwdinfo, pframe);
	pframe += wfdielen;
	pattrib->pktlen += wfdielen;
#endif

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

	return;
}

static void issue_p2p_GO_response(_adapter *padapter, u8 *raddr, u8 *frame_body, uint len, u8 result)
{
	unsigned char category = RTW_WLAN_CATEGORY_PUBLIC;
	u8			action = P2P_PUB_ACTION_ACTION;
	__be32			p2poui = cpu_to_be32(P2POUI);
	u8			oui_subtype = P2P_GO_NEGO_RESP;
	u8			wpsie[255] = { 0x00 }, p2pie[255] = { 0x00 };
	u8			p2pielen = 0, i;
	uint			wpsielen = 0;
	u16			wps_devicepassword_id = 0x0000;
	uint			wps_devicepassword_id_len = 0;
	u8			channel_cnt_24g = 0, channel_cnt_5gl = 0, channel_cnt_5gh;
	u16			len_channellist_attr = 0;
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	__be16 be_tmp;
#ifdef CONFIG_WFD
	u32					wfdielen = 0;
#endif

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return;

	RTW_INFO("[%s] In, result = %d\n", __func__,  result);
	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	memcpy(pwlanhdr->addr1, raddr, ETH_ALEN);
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	memcpy(pwlanhdr->addr3, adapter_mac_addr(padapter), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 4, (unsigned char *) &(p2poui), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(oui_subtype), &(pattrib->pktlen));
	pwdinfo->negotiation_dialog_token = frame_body[7];	/*	The Dialog Token of provisioning discovery request frame. */
	pframe = rtw_set_fixed_ie(pframe, 1, &(pwdinfo->negotiation_dialog_token), &(pattrib->pktlen));

	/*	Commented by Albert 20110328 */
	/*	Try to get the device password ID from the WPS IE of group negotiation request frame */
	/*	WiFi Direct test plan 5.1.15 */
	rtw_get_wps_ie(frame_body + _PUBLIC_ACTION_IE_OFFSET_, len - _PUBLIC_ACTION_IE_OFFSET_, wpsie, &wpsielen);
	rtw_get_wps_attr_content(wpsie, wpsielen, WPS_ATTR_DEVICE_PWID, (u8 *) &be_tmp, &wps_devicepassword_id_len);
	wps_devicepassword_id = be16_to_cpu(be_tmp);

	memset(wpsie, 0x00, 255);
	wpsielen = 0;

	/*	WPS Section */
	wpsielen = 0;
	/*	WPS OUI */
	*(__be32 *)(wpsie) = cpu_to_be32(WPSOUI);
	wpsielen += 4;

	/*	WPS version */
	/*	Type: */
	*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_VER1);
	wpsielen += 2;

	/*	Length: */
	*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0001);
	wpsielen += 2;

	/*	Value: */
	wpsie[wpsielen++] = WPS_VERSION_1;	/*	Version 1.0 */

	/*	Device Password ID */
	/*	Type: */
	*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_DEVICE_PWID);
	wpsielen += 2;

	/*	Length: */
	*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0002);
	wpsielen += 2;

	/*	Value: */
	if (wps_devicepassword_id == WPS_DPID_USER_SPEC)
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_DPID_REGISTRAR_SPEC);
	else if (wps_devicepassword_id == WPS_DPID_REGISTRAR_SPEC)
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_DPID_USER_SPEC);
	else
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_DPID_PBC);
	wpsielen += 2;

	/*	Commented by Kurt 20120113 */
	/*	If some device wants to do p2p handshake without sending prov_disc_req */
	/*	We have to get peer_req_cm from here. */
	if (!memcmp(pwdinfo->rx_prov_disc_info.strconfig_method_desc_of_prov_disc_req, "000", 3)) {
		if (wps_devicepassword_id == WPS_DPID_USER_SPEC)
			memcpy(pwdinfo->rx_prov_disc_info.strconfig_method_desc_of_prov_disc_req, "dis", 3);
		else if (wps_devicepassword_id == WPS_DPID_REGISTRAR_SPEC)
			memcpy(pwdinfo->rx_prov_disc_info.strconfig_method_desc_of_prov_disc_req, "pad", 3);
		else
			memcpy(pwdinfo->rx_prov_disc_info.strconfig_method_desc_of_prov_disc_req, "pbc", 3);
	}

	pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, wpsielen, (unsigned char *) wpsie, &pattrib->pktlen);

	/*	P2P IE Section. */

	/*	P2P OUI */
	p2pielen = 0;
	p2pie[p2pielen++] = 0x50;
	p2pie[p2pielen++] = 0x6F;
	p2pie[p2pielen++] = 0x9A;
	p2pie[p2pielen++] = 0x09;	/*	WFA P2P v1.0 */

	/*	Commented by Albert 20100908 */
	/*	According to the P2P Specification, the group negoitation response frame should contain 9 P2P attributes */
	/*	1. Status */
	/*	2. P2P Capability */
	/*	3. Group Owner Intent */
	/*	4. Configuration Timeout */
	/*	5. Operating Channel */
	/*	6. Intended P2P Interface Address */
	/*	7. Channel List */
	/*	8. Device Info */
	/*	9. Group ID	( Only GO ) */

	/*	ToDo: */

	/*	P2P Status */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_STATUS;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0001);
	p2pielen += 2;

	/*	Value: */
	p2pie[p2pielen++] = result;

	/*	P2P Capability */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_CAPABILITY;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0002);
	p2pielen += 2;

	/*	Value: */
	/*	Device Capability Bitmap, 1 byte */

	if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_CLIENT)) {
		/*	Commented by Albert 2011/03/08 */
		/*	According to the P2P specification */
		/*	if the sending device will be client, the P2P Capability should be reserved of group negotation response frame */
		p2pie[p2pielen++] = 0;
	} else {
		/*	Be group owner or meet the error case */
		p2pie[p2pielen++] = DMP_P2P_DEVCAP_SUPPORT;
	}

	/*	Group Capability Bitmap, 1 byte */
	if (pwdinfo->persistent_supported)
		p2pie[p2pielen++] = P2P_GRPCAP_CROSS_CONN | P2P_GRPCAP_PERSISTENT_GROUP;
	else
		p2pie[p2pielen++] = P2P_GRPCAP_CROSS_CONN;

	/*	Group Owner Intent */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_GO_INTENT;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0001);
	p2pielen += 2;

	/*	Value: */
	if (pwdinfo->peer_intent & 0x01) {
		/*	Peer's tie breaker bit is 1, our tie breaker bit should be 0 */
		p2pie[p2pielen++] = (pwdinfo->intent << 1);
	} else {
		/*	Peer's tie breaker bit is 0, our tie breaker bit should be 1 */
		p2pie[p2pielen++] = ((pwdinfo->intent << 1) | BIT(0));
	}

	/*	Configuration Timeout */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_CONF_TIMEOUT;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0002);
	p2pielen += 2;

	/*	Value: */
	p2pie[p2pielen++] = 200;	/*	2 seconds needed to be the P2P GO */
	p2pie[p2pielen++] = 200;	/*	2 seconds needed to be the P2P Client */

	/*	Operating Channel */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_OPERATING_CH;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0005);
	p2pielen += 2;

	/*	Value: */
	/*	Country String */
	p2pie[p2pielen++] = 'X';
	p2pie[p2pielen++] = 'X';

	/*	The third byte should be set to 0x04. */
	/*	Described in the "Operating Channel Attribute" section. */
	p2pie[p2pielen++] = 0x04;

	/*	Operating Class */
	if (pwdinfo->operating_channel <= 14) {
		/*	Operating Class */
		p2pie[p2pielen++] = 0x51;
	} else if ((pwdinfo->operating_channel >= 36) && (pwdinfo->operating_channel <= 48)) {
		/*	Operating Class */
		p2pie[p2pielen++] = 0x73;
	} else {
		/*	Operating Class */
		p2pie[p2pielen++] = 0x7c;
	}

	/*	Channel Number */
	p2pie[p2pielen++] = pwdinfo->operating_channel;	/*	operating channel number */

	/*	Intended P2P Interface Address	 */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_INTENDED_IF_ADDR;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(ETH_ALEN);
	p2pielen += 2;

	/*	Value: */
	memcpy(p2pie + p2pielen, adapter_mac_addr(padapter), ETH_ALEN);
	p2pielen += ETH_ALEN;

	/*	Channel List */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_CH_LIST;

	/* Country String(3) */
	/* + ( Operating Class (1) + Number of Channels(1) ) * Operation Classes (?) */
	/* + number of channels in all classes */
	len_channellist_attr = 3
		       + (1 + 1) * (u16)pmlmeext->channel_list.reg_classes
		       + get_reg_classes_full_count(pmlmeext->channel_list);

#ifdef CONFIG_CONCURRENT_MODE
	if (rtw_mi_check_status(padapter, MI_LINKED) && padapter->registrypriv.full_ch_in_p2p_handshake == 0)
		*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(5 + 1);
	else
		*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(len_channellist_attr);
#else
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(len_channellist_attr);

#endif
	p2pielen += 2;

	/*	Value: */
	/*	Country String */
	p2pie[p2pielen++] = 'X';
	p2pie[p2pielen++] = 'X';

	/*	The third byte should be set to 0x04. */
	/*	Described in the "Operating Channel Attribute" section. */
	p2pie[p2pielen++] = 0x04;

	/*	Channel Entry List */

#ifdef CONFIG_CONCURRENT_MODE
	if (rtw_mi_check_status(padapter, MI_LINKED) && padapter->registrypriv.full_ch_in_p2p_handshake == 0) {

		u8 union_chan = rtw_mi_get_union_chan(padapter);

		/*Operating Class*/
		if (union_chan > 14) {
			if (union_chan >= 149)
				p2pie[p2pielen++] = 0x7c;
			else
				p2pie[p2pielen++] = 0x73;

		} else
			p2pie[p2pielen++] = 0x51;

		/*	Number of Channels
			Just support 1 channel and this channel is AP's channel*/
		p2pie[p2pielen++] = 1;

		/*Channel List*/
		p2pie[p2pielen++] = union_chan;
	} else {
		int i, j;
		for (j = 0; j < pmlmeext->channel_list.reg_classes; j++) {
			/*	Operating Class */
			p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].reg_class;

			/*	Number of Channels */
			p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channels;

			/*	Channel List */
			for (i = 0; i < pmlmeext->channel_list.reg_class[j].channels; i++)
				p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channel[i];
		}
	}
#else /* CONFIG_CONCURRENT_MODE */
	{
		int i, j;
		for (j = 0; j < pmlmeext->channel_list.reg_classes; j++) {
			/*	Operating Class */
			p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].reg_class;

			/*	Number of Channels */
			p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channels;

			/*	Channel List */
			for (i = 0; i < pmlmeext->channel_list.reg_class[j].channels; i++)
				p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channel[i];
		}
	}
#endif /* CONFIG_CONCURRENT_MODE */


	/*	Device Info */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_DEVICE_INFO;

	/*	Length: */
	/*	21->P2P Device Address (6bytes) + Config Methods (2bytes) + Primary Device Type (8bytes)  */
	/*	+ NumofSecondDevType (1byte) + WPS Device Name ID field (2bytes) + WPS Device Name Len field (2bytes) */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(21 + pwdinfo->device_name_len);
	p2pielen += 2;

	/*	Value: */
	/*	P2P Device Address */
	memcpy(p2pie + p2pielen, adapter_mac_addr(padapter), ETH_ALEN);
	p2pielen += ETH_ALEN;

	/*	Config Method */
	/*	This field should be big endian. Noted by P2P specification. */

	*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(pwdinfo->supported_wps_cm);

	p2pielen += 2;

	/*	Primary Device Type */
	/*	Category ID */
	*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(WPS_PDT_CID_MULIT_MEDIA);
	p2pielen += 2;

	/*	OUI */
	*(__be32 *)(p2pie + p2pielen) = cpu_to_be32(WPSOUI);
	p2pielen += 4;

	/*	Sub Category ID */
	*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(WPS_PDT_SCID_MEDIA_SERVER);
	p2pielen += 2;

	/*	Number of Secondary Device Types */
	p2pie[p2pielen++] = 0x00;	/*	No Secondary Device Type List */

	/*	Device Name */
	/*	Type: */
	*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(WPS_ATTR_DEVICE_NAME);
	p2pielen += 2;

	/*	Length: */
	*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(pwdinfo->device_name_len);
	p2pielen += 2;

	/*	Value: */
	memcpy(p2pie + p2pielen, pwdinfo->device_name , pwdinfo->device_name_len);
	p2pielen += pwdinfo->device_name_len;

	if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO)) {
		/*	Group ID Attribute */
		/*	Type: */
		p2pie[p2pielen++] = P2P_ATTR_GROUP_ID;

		/*	Length: */
		*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(ETH_ALEN + pwdinfo->nego_ssidlen);
		p2pielen += 2;

		/*	Value: */
		/*	p2P Device Address */
		memcpy(p2pie + p2pielen , pwdinfo->device_addr, ETH_ALEN);
		p2pielen += ETH_ALEN;

		/*	SSID */
		memcpy(p2pie + p2pielen, pwdinfo->nego_ssid, pwdinfo->nego_ssidlen);
		p2pielen += pwdinfo->nego_ssidlen;

	}

	pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &pattrib->pktlen);

#ifdef CONFIG_WFD
	wfdielen = build_nego_resp_wfd_ie(pwdinfo, pframe);
	pframe += wfdielen;
	pattrib->pktlen += wfdielen;
#endif

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

	return;

}

static void issue_p2p_GO_confirm(_adapter *padapter, u8 *raddr, u8 result)
{

	unsigned char category = RTW_WLAN_CATEGORY_PUBLIC;
	u8			action = P2P_PUB_ACTION_ACTION;
	__be32 p2poui = cpu_to_be32(P2POUI);
	u8			oui_subtype = P2P_GO_NEGO_CONF;
	u8			wpsie[255] = { 0x00 }, p2pie[255] = { 0x00 };
	u8			wpsielen = 0, p2pielen = 0;

	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
#ifdef CONFIG_WFD
	u32					wfdielen = 0;
#endif

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return;

	RTW_INFO("[%s] In\n", __func__);
	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	memcpy(pwlanhdr->addr1, raddr, ETH_ALEN);
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	memcpy(pwlanhdr->addr3, adapter_mac_addr(padapter), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 4, (unsigned char *) &(p2poui), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(oui_subtype), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(pwdinfo->negotiation_dialog_token), &(pattrib->pktlen));



	/*	P2P IE Section. */

	/*	P2P OUI */
	p2pielen = 0;
	p2pie[p2pielen++] = 0x50;
	p2pie[p2pielen++] = 0x6F;
	p2pie[p2pielen++] = 0x9A;
	p2pie[p2pielen++] = 0x09;	/*	WFA P2P v1.0 */

	/*	Commented by Albert 20110306 */
	/*	According to the P2P Specification, the group negoitation request frame should contain 5 P2P attributes */
	/*	1. Status */
	/*	2. P2P Capability */
	/*	3. Operating Channel */
	/*	4. Channel List */
	/*	5. Group ID	( if this WiFi is GO ) */

	/*	P2P Status */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_STATUS;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0001);
	p2pielen += 2;

	/*	Value: */
	p2pie[p2pielen++] = result;

	/*	P2P Capability */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_CAPABILITY;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0002);
	p2pielen += 2;

	/*	Value: */
	/*	Device Capability Bitmap, 1 byte */
	p2pie[p2pielen++] = DMP_P2P_DEVCAP_SUPPORT;

	/*	Group Capability Bitmap, 1 byte */
	if (pwdinfo->persistent_supported)
		p2pie[p2pielen++] = P2P_GRPCAP_CROSS_CONN | P2P_GRPCAP_PERSISTENT_GROUP;
	else
		p2pie[p2pielen++] = P2P_GRPCAP_CROSS_CONN;


	/*	Operating Channel */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_OPERATING_CH;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0005);
	p2pielen += 2;

	/*	Value: */
	/*	Country String */
	p2pie[p2pielen++] = 'X';
	p2pie[p2pielen++] = 'X';

	/*	The third byte should be set to 0x04. */
	/*	Described in the "Operating Channel Attribute" section. */
	p2pie[p2pielen++] = 0x04;


	if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_CLIENT)) {
		if (pwdinfo->peer_operating_ch <= 14) {
			/*	Operating Class */
			p2pie[p2pielen++] = 0x51;
		} else if ((pwdinfo->peer_operating_ch >= 36) && (pwdinfo->peer_operating_ch <= 48)) {
			/*	Operating Class */
			p2pie[p2pielen++] = 0x73;
		} else {
			/*	Operating Class */
			p2pie[p2pielen++] = 0x7c;
		}

		p2pie[p2pielen++] = pwdinfo->peer_operating_ch;
	} else {
		if (pwdinfo->operating_channel <= 14) {
			/*	Operating Class */
			p2pie[p2pielen++] = 0x51;
		} else if ((pwdinfo->operating_channel >= 36) && (pwdinfo->operating_channel <= 48)) {
			/*	Operating Class */
			p2pie[p2pielen++] = 0x73;
		} else {
			/*	Operating Class */
			p2pie[p2pielen++] = 0x7c;
		}

		/*	Channel Number */
		p2pie[p2pielen++] = pwdinfo->operating_channel;		/*	Use the listen channel as the operating channel */
	}


	/*	Channel List */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_CH_LIST;

	*(u16 *)(p2pie + p2pielen) = 6;
	p2pielen += 2;

	/*	Country String */
	p2pie[p2pielen++] = 'X';
	p2pie[p2pielen++] = 'X';

	/*	The third byte should be set to 0x04. */
	/*	Described in the "Operating Channel Attribute" section. */
	p2pie[p2pielen++] = 0x04;

	/*	Value: */
	if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_CLIENT)) {
		if (pwdinfo->peer_operating_ch <= 14) {
			/*	Operating Class */
			p2pie[p2pielen++] = 0x51;
		} else if ((pwdinfo->peer_operating_ch >= 36) && (pwdinfo->peer_operating_ch <= 48)) {
			/*	Operating Class */
			p2pie[p2pielen++] = 0x73;
		} else {
			/*	Operating Class */
			p2pie[p2pielen++] = 0x7c;
		}
		p2pie[p2pielen++] = 1;
		p2pie[p2pielen++] = pwdinfo->peer_operating_ch;
	} else {
		if (pwdinfo->operating_channel <= 14) {
			/*	Operating Class */
			p2pie[p2pielen++] = 0x51;
		} else if ((pwdinfo->operating_channel >= 36) && (pwdinfo->operating_channel <= 48)) {
			/*	Operating Class */
			p2pie[p2pielen++] = 0x73;
		} else {
			/*	Operating Class */
			p2pie[p2pielen++] = 0x7c;
		}

		/*	Channel Number */
		p2pie[p2pielen++] = 1;
		p2pie[p2pielen++] = pwdinfo->operating_channel;		/*	Use the listen channel as the operating channel */
	}

	if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO)) {
		/*	Group ID Attribute */
		/*	Type: */
		p2pie[p2pielen++] = P2P_ATTR_GROUP_ID;

		/*	Length: */
		*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(ETH_ALEN + pwdinfo->nego_ssidlen);
		p2pielen += 2;

		/*	Value: */
		/*	p2P Device Address */
		memcpy(p2pie + p2pielen , pwdinfo->device_addr, ETH_ALEN);
		p2pielen += ETH_ALEN;

		/*	SSID */
		memcpy(p2pie + p2pielen, pwdinfo->nego_ssid, pwdinfo->nego_ssidlen);
		p2pielen += pwdinfo->nego_ssidlen;
	}

	pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &pattrib->pktlen);

#ifdef CONFIG_WFD
	wfdielen = build_nego_confirm_wfd_ie(pwdinfo, pframe);
	pframe += wfdielen;
	pattrib->pktlen += wfdielen;
#endif

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

	return;

}

void issue_p2p_invitation_request(_adapter *padapter, u8 *raddr)
{
	unsigned char category = RTW_WLAN_CATEGORY_PUBLIC;
	u8 action = P2P_PUB_ACTION_ACTION;
	__be32 p2poui = cpu_to_be32(P2POUI);
	u8 oui_subtype = P2P_INVIT_REQ;
	u8 p2pie[255] = { 0x00 };
	u8 p2pielen = 0, i;
	u8 dialogToken = 3;
	u8 channel_cnt_24g = 0, channel_cnt_5gl = 0, channel_cnt_5gh = 0;
	u16 len_channellist_attr = 0;
#ifdef CONFIG_WFD
	u32					wfdielen = 0;
#endif

	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	struct xmit_priv *pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);


	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	memcpy(pwlanhdr->addr1, raddr, ETH_ALEN);
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	memcpy(pwlanhdr->addr3, raddr,  ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 4, (unsigned char *) &(p2poui), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(oui_subtype), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(dialogToken), &(pattrib->pktlen));

	/*	P2P IE Section. */

	/*	P2P OUI */
	p2pielen = 0;
	p2pie[p2pielen++] = 0x50;
	p2pie[p2pielen++] = 0x6F;
	p2pie[p2pielen++] = 0x9A;
	p2pie[p2pielen++] = 0x09;	/*	WFA P2P v1.0 */

	/*	Commented by Albert 20101011 */
	/*	According to the P2P Specification, the P2P Invitation request frame should contain 7 P2P attributes */
	/*	1. Configuration Timeout */
	/*	2. Invitation Flags */
	/*	3. Operating Channel	( Only GO ) */
	/*	4. P2P Group BSSID	( Should be included if I am the GO ) */
	/*	5. Channel List */
	/*	6. P2P Group ID */
	/*	7. P2P Device Info */

	/*	Configuration Timeout */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_CONF_TIMEOUT;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0002);
	p2pielen += 2;

	/*	Value: */
	p2pie[p2pielen++] = 200;	/*	2 seconds needed to be the P2P GO */
	p2pie[p2pielen++] = 200;	/*	2 seconds needed to be the P2P Client */

	/*	Invitation Flags */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_INVITATION_FLAGS;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0001);
	p2pielen += 2;

	/*	Value: */
	p2pie[p2pielen++] = P2P_INVITATION_FLAGS_PERSISTENT;


	/*	Operating Channel */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_OPERATING_CH;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0005);
	p2pielen += 2;

	/*	Value: */
	/*	Country String */
	p2pie[p2pielen++] = 'X';
	p2pie[p2pielen++] = 'X';

	/*	The third byte should be set to 0x04. */
	/*	Described in the "Operating Channel Attribute" section. */
	p2pie[p2pielen++] = 0x04;

	/*	Operating Class */
	if (pwdinfo->invitereq_info.operating_ch <= 14)
		p2pie[p2pielen++] = 0x51;
	else if ((pwdinfo->invitereq_info.operating_ch >= 36) && (pwdinfo->invitereq_info.operating_ch <= 48))
		p2pie[p2pielen++] = 0x73;
	else
		p2pie[p2pielen++] = 0x7c;

	/*	Channel Number */
	p2pie[p2pielen++] = pwdinfo->invitereq_info.operating_ch;	/*	operating channel number */

	if (!memcmp(adapter_mac_addr(padapter), pwdinfo->invitereq_info.go_bssid, ETH_ALEN)) {
		/*	P2P Group BSSID */
		/*	Type: */
		p2pie[p2pielen++] = P2P_ATTR_GROUP_BSSID;

		/*	Length: */
		*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(ETH_ALEN);
		p2pielen += 2;

		/*	Value: */
		/*	P2P Device Address for GO */
		memcpy(p2pie + p2pielen, pwdinfo->invitereq_info.go_bssid, ETH_ALEN);
		p2pielen += ETH_ALEN;
	}

	/*	Channel List */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_CH_LIST;


	/*	Length: */
	/* Country String(3) */
	/* + ( Operating Class (1) + Number of Channels(1) ) * Operation Classes (?) */
	/* + number of channels in all classes */
	len_channellist_attr = 3
		       + (1 + 1) * (u16)pmlmeext->channel_list.reg_classes
		       + get_reg_classes_full_count(pmlmeext->channel_list);

#ifdef CONFIG_CONCURRENT_MODE
	if (rtw_mi_check_status(padapter, MI_LINKED) && padapter->registrypriv.full_ch_in_p2p_handshake == 0)
		*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(5 + 1);
	else
		*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(len_channellist_attr);
#else
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(len_channellist_attr);
#endif
	p2pielen += 2;

	/*	Value: */
	/*	Country String */
	p2pie[p2pielen++] = 'X';
	p2pie[p2pielen++] = 'X';

	/*	The third byte should be set to 0x04. */
	/*	Described in the "Operating Channel Attribute" section. */
	p2pie[p2pielen++] = 0x04;

	/*	Channel Entry List */
#ifdef CONFIG_CONCURRENT_MODE
	if (rtw_mi_check_status(padapter, MI_LINKED) && padapter->registrypriv.full_ch_in_p2p_handshake == 0) {
		u8 union_ch =  rtw_mi_get_union_chan(padapter);

		/*	Operating Class */
		if (union_ch > 14) {
			if (union_ch >= 149)
				p2pie[p2pielen++] = 0x7c;
			else
				p2pie[p2pielen++] = 0x73;
		} else
			p2pie[p2pielen++] = 0x51;


		/*	Number of Channels */
		/*	Just support 1 channel and this channel is AP's channel */
		p2pie[p2pielen++] = 1;

		/*	Channel List */
		p2pie[p2pielen++] = union_ch;
	} else {
		int i, j;
		for (j = 0; j < pmlmeext->channel_list.reg_classes; j++) {
			/*	Operating Class */
			p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].reg_class;

			/*	Number of Channels */
			p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channels;

			/*	Channel List */
			for (i = 0; i < pmlmeext->channel_list.reg_class[j].channels; i++)
				p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channel[i];
		}
	}
#else /* CONFIG_CONCURRENT_MODE */
	{
		int i, j;
		for (j = 0; j < pmlmeext->channel_list.reg_classes; j++) {
			/*	Operating Class */
			p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].reg_class;

			/*	Number of Channels */
			p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channels;

			/*	Channel List */
			for (i = 0; i < pmlmeext->channel_list.reg_class[j].channels; i++)
				p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channel[i];
		}
	}
#endif /* CONFIG_CONCURRENT_MODE */


	/*	P2P Group ID */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_GROUP_ID;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(6 + pwdinfo->invitereq_info.ssidlen);
	p2pielen += 2;

	/*	Value: */
	/*	P2P Device Address for GO */
	memcpy(p2pie + p2pielen, pwdinfo->invitereq_info.go_bssid, ETH_ALEN);
	p2pielen += ETH_ALEN;

	/*	SSID */
	memcpy(p2pie + p2pielen, pwdinfo->invitereq_info.go_ssid, pwdinfo->invitereq_info.ssidlen);
	p2pielen += pwdinfo->invitereq_info.ssidlen;


	/*	Device Info */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_DEVICE_INFO;

	/*	Length: */
	/*	21->P2P Device Address (6bytes) + Config Methods (2bytes) + Primary Device Type (8bytes)  */
	/*	+ NumofSecondDevType (1byte) + WPS Device Name ID field (2bytes) + WPS Device Name Len field (2bytes) */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(21 + pwdinfo->device_name_len);
	p2pielen += 2;

	/*	Value: */
	/*	P2P Device Address */
	memcpy(p2pie + p2pielen, adapter_mac_addr(padapter), ETH_ALEN);
	p2pielen += ETH_ALEN;

	/*	Config Method */
	/*	This field should be big endian. Noted by P2P specification. */
	*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(WPS_CONFIG_METHOD_DISPLAY);
	p2pielen += 2;

	/*	Primary Device Type */
	/*	Category ID */
	*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(WPS_PDT_CID_MULIT_MEDIA);
	p2pielen += 2;

	/*	OUI */
	*(__be32 *)(p2pie + p2pielen) = cpu_to_be32(WPSOUI);
	p2pielen += 4;

	/*	Sub Category ID */
	*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(WPS_PDT_SCID_MEDIA_SERVER);
	p2pielen += 2;

	/*	Number of Secondary Device Types */
	p2pie[p2pielen++] = 0x00;	/*	No Secondary Device Type List */

	/*	Device Name */
	/*	Type: */
	*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(WPS_ATTR_DEVICE_NAME);
	p2pielen += 2;

	/*	Length: */
	*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(pwdinfo->device_name_len);
	p2pielen += 2;

	/*	Value: */
	memcpy(p2pie + p2pielen, pwdinfo->device_name, pwdinfo->device_name_len);
	p2pielen += pwdinfo->device_name_len;

	pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &pattrib->pktlen);

#ifdef CONFIG_WFD
	wfdielen = build_invitation_req_wfd_ie(pwdinfo, pframe);
	pframe += wfdielen;
	pattrib->pktlen += wfdielen;
#endif

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

	return;
}

void issue_p2p_invitation_response(_adapter *padapter, u8 *raddr, u8 dialogToken, u8 status_code)
{

	unsigned char category = RTW_WLAN_CATEGORY_PUBLIC;
	u8			action = P2P_PUB_ACTION_ACTION;
	__be32 p2poui = cpu_to_be32(P2POUI);
	u8			oui_subtype = P2P_INVIT_RESP;
	u8			p2pie[255] = { 0x00 };
	u8			p2pielen = 0, i;
	u8			channel_cnt_24g = 0, channel_cnt_5gl = 0, channel_cnt_5gh = 0;
	u16			len_channellist_attr = 0;
#ifdef CONFIG_WFD
	u32					wfdielen = 0;
#endif

	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);


	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	memcpy(pwlanhdr->addr1, raddr, ETH_ALEN);
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	memcpy(pwlanhdr->addr3, raddr,  ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 4, (unsigned char *) &(p2poui), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(oui_subtype), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(dialogToken), &(pattrib->pktlen));

	/*	P2P IE Section. */

	/*	P2P OUI */
	p2pielen = 0;
	p2pie[p2pielen++] = 0x50;
	p2pie[p2pielen++] = 0x6F;
	p2pie[p2pielen++] = 0x9A;
	p2pie[p2pielen++] = 0x09;	/*	WFA P2P v1.0 */

	/*	Commented by Albert 20101005 */
	/*	According to the P2P Specification, the P2P Invitation response frame should contain 5 P2P attributes */
	/*	1. Status */
	/*	2. Configuration Timeout */
	/*	3. Operating Channel	( Only GO ) */
	/*	4. P2P Group BSSID	( Only GO ) */
	/*	5. Channel List */

	/*	P2P Status */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_STATUS;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0001);
	p2pielen += 2;

	/*	Value: */
	/*	When status code is P2P_STATUS_FAIL_INFO_UNAVAILABLE. */
	/*	Sent the event receiving the P2P Invitation Req frame to DMP UI. */
	/*	DMP had to compare the MAC address to find out the profile. */
	/*	So, the WiFi driver will send the P2P_STATUS_FAIL_INFO_UNAVAILABLE to NB. */
	/*	If the UI found the corresponding profile, the WiFi driver sends the P2P Invitation Req */
	/*	to NB to rebuild the persistent group. */
	p2pie[p2pielen++] = status_code;

	/*	Configuration Timeout */
	/*	Type: */
	p2pie[p2pielen++] = P2P_ATTR_CONF_TIMEOUT;

	/*	Length: */
	*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0002);
	p2pielen += 2;

	/*	Value: */
	p2pie[p2pielen++] = 200;	/*	2 seconds needed to be the P2P GO */
	p2pie[p2pielen++] = 200;	/*	2 seconds needed to be the P2P Client */

	if (status_code == P2P_STATUS_SUCCESS) {
		if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO)) {
			/*	The P2P Invitation request frame asks this Wi-Fi device to be the P2P GO */
			/*	In this case, the P2P Invitation response frame should carry the two more P2P attributes. */
			/*	First one is operating channel attribute. */
			/*	Second one is P2P Group BSSID attribute. */

			/*	Operating Channel */
			/*	Type: */
			p2pie[p2pielen++] = P2P_ATTR_OPERATING_CH;

			/*	Length: */
			*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0005);
			p2pielen += 2;

			/*	Value: */
			/*	Country String */
			p2pie[p2pielen++] = 'X';
			p2pie[p2pielen++] = 'X';

			/*	The third byte should be set to 0x04. */
			/*	Described in the "Operating Channel Attribute" section. */
			p2pie[p2pielen++] = 0x04;

			/*	Operating Class */
			p2pie[p2pielen++] = 0x51;	/*	Copy from SD7 */

			/*	Channel Number */
			p2pie[p2pielen++] = pwdinfo->operating_channel;	/*	operating channel number */


			/*	P2P Group BSSID */
			/*	Type: */
			p2pie[p2pielen++] = P2P_ATTR_GROUP_BSSID;

			/*	Length: */
			*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(ETH_ALEN);
			p2pielen += 2;

			/*	Value: */
			/*	P2P Device Address for GO */
			memcpy(p2pie + p2pielen, adapter_mac_addr(padapter), ETH_ALEN);
			p2pielen += ETH_ALEN;

		}

		/*	Channel List */
		/*	Type: */
		p2pie[p2pielen++] = P2P_ATTR_CH_LIST;

		/*	Length: */
		/* Country String(3) */
		/* + ( Operating Class (1) + Number of Channels(1) ) * Operation Classes (?) */
		/* + number of channels in all classes */
		len_channellist_attr = 3
			+ (1 + 1) * (u16)pmlmeext->channel_list.reg_classes
			+ get_reg_classes_full_count(pmlmeext->channel_list);

#ifdef CONFIG_CONCURRENT_MODE
		if (rtw_mi_check_status(padapter, MI_LINKED) && padapter->registrypriv.full_ch_in_p2p_handshake == 0)
			*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(5 + 1);
		else
			*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(len_channellist_attr);
#else
		*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(len_channellist_attr);
#endif
		p2pielen += 2;

		/*	Value: */
		/*	Country String */
		p2pie[p2pielen++] = 'X';
		p2pie[p2pielen++] = 'X';

		/*	The third byte should be set to 0x04. */
		/*	Described in the "Operating Channel Attribute" section. */
		p2pie[p2pielen++] = 0x04;

		/*	Channel Entry List */
#ifdef CONFIG_CONCURRENT_MODE
		if (rtw_mi_check_status(padapter, MI_LINKED) && padapter->registrypriv.full_ch_in_p2p_handshake == 0) {
			u8 union_ch = rtw_mi_get_union_chan(padapter);

			/*	Operating Class */
			if (union_ch > 14) {
				if (union_ch >= 149)
					p2pie[p2pielen++]  = 0x7c;
				else
					p2pie[p2pielen++] = 0x73;
			} else
				p2pie[p2pielen++] = 0x51;


			/*	Number of Channels */
			/*	Just support 1 channel and this channel is AP's channel */
			p2pie[p2pielen++] = 1;

			/*	Channel List */
			p2pie[p2pielen++] = union_ch;
		} else {
			int i, j;
			for (j = 0; j < pmlmeext->channel_list.reg_classes; j++) {
				/*	Operating Class */
				p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].reg_class;

				/*	Number of Channels */
				p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channels;

				/*	Channel List */
				for (i = 0; i < pmlmeext->channel_list.reg_class[j].channels; i++)
					p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channel[i];
			}
		}
#else /* CONFIG_CONCURRENT_MODE */
		{
			int i, j;
			for (j = 0; j < pmlmeext->channel_list.reg_classes; j++) {
				/*	Operating Class */
				p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].reg_class;

				/*	Number of Channels */
				p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channels;

				/*	Channel List */
				for (i = 0; i < pmlmeext->channel_list.reg_class[j].channels; i++)
					p2pie[p2pielen++] = pmlmeext->channel_list.reg_class[j].channel[i];
			}
		}
#endif /* CONFIG_CONCURRENT_MODE */
	}

	pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &pattrib->pktlen);

#ifdef CONFIG_WFD
	wfdielen = build_invitation_resp_wfd_ie(pwdinfo, pframe);
	pframe += wfdielen;
	pattrib->pktlen += wfdielen;
#endif

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

	return;

}

void issue_p2p_provision_request(_adapter *padapter, u8 *pssid, u8 ussidlen, u8 *pdev_raddr)
{
	unsigned char category = RTW_WLAN_CATEGORY_PUBLIC;
	u8			action = P2P_PUB_ACTION_ACTION;
	u8			dialogToken = 1;
	__be32			p2poui = cpu_to_be32(P2POUI);
	u8			oui_subtype = P2P_PROVISION_DISC_REQ;
	u8			wpsie[100] = { 0x00 };
	u8			wpsielen = 0;
	u32			p2pielen = 0;
#ifdef CONFIG_WFD
	u32					wfdielen = 0;
#endif

	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);


	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return;

	RTW_INFO("[%s] In\n", __func__);
	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	memcpy(pwlanhdr->addr1, pdev_raddr, ETH_ALEN);
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	memcpy(pwlanhdr->addr3, pdev_raddr, ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 4, (unsigned char *) &(p2poui), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(oui_subtype), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(dialogToken), &(pattrib->pktlen));

	p2pielen = build_prov_disc_request_p2p_ie(pwdinfo, pframe, pssid, ussidlen, pdev_raddr);

	pframe += p2pielen;
	pattrib->pktlen += p2pielen;

	wpsielen = 0;
	/*	WPS OUI */
	*(__be32 *)(wpsie) = cpu_to_be32(WPSOUI);
	wpsielen += 4;

	/*	WPS version */
	/*	Type: */
	*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_VER1);
	wpsielen += 2;

	/*	Length: */
	*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0001);
	wpsielen += 2;

	/*	Value: */
	wpsie[wpsielen++] = WPS_VERSION_1;	/*	Version 1.0 */

	/*	Config Method */
	/*	Type: */
	*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_CONF_METHOD);
	wpsielen += 2;

	/*	Length: */
	*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0002);
	wpsielen += 2;

	/*	Value: */
	*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(pwdinfo->tx_prov_disc_info.wps_config_method_request);
	wpsielen += 2;

	pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, wpsielen, (unsigned char *) wpsie, &pattrib->pktlen);


#ifdef CONFIG_WFD
	wfdielen = build_provdisc_req_wfd_ie(pwdinfo, pframe);
	pframe += wfdielen;
	pattrib->pktlen += wfdielen;
#endif

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

	return;

}


static u8 is_matched_in_profilelist(u8 *peermacaddr, struct profile_info *profileinfo)
{
	u8 i, match_result = 0;

	RTW_INFO("[%s] peermac = %.2X %.2X %.2X %.2X %.2X %.2X\n", __func__,
		peermacaddr[0], peermacaddr[1], peermacaddr[2], peermacaddr[3], peermacaddr[4], peermacaddr[5]);

	for (i = 0; i < P2P_MAX_PERSISTENT_GROUP_NUM; i++, profileinfo++) {
		RTW_INFO("[%s] profileinfo_mac = %.2X %.2X %.2X %.2X %.2X %.2X\n", __func__,
			profileinfo->peermac[0], profileinfo->peermac[1], profileinfo->peermac[2], profileinfo->peermac[3], profileinfo->peermac[4], profileinfo->peermac[5]);
		if (!memcmp(peermacaddr, profileinfo->peermac, ETH_ALEN)) {
			match_result = 1;
			RTW_INFO("[%s] Match!\n", __func__);
			break;
		}
	}

	return match_result ;
}

void issue_probersp_p2p(_adapter *padapter, unsigned char *da)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	unsigned char					*mac;
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	/* WLAN_BSSID_EX 		*cur_network = &(pmlmeinfo->network); */
	u16					beacon_interval = 100;
	u16					capInfo = 0;
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	u8					wpsie[255] = { 0x00 };
	u32					wpsielen = 0, p2pielen = 0;
#ifdef CONFIG_WFD
	u32					wfdielen = 0;
#endif
#ifdef CONFIG_INTEL_WIDI
	u8 zero_array_check[L2SDTA_SERVICE_VE_LEN] = { 0x00 };
#endif /* CONFIG_INTEL_WIDI */

	/* RTW_INFO("%s\n", __func__); */

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	if (IS_CCK_RATE(pattrib->rate)) {
		/* force OFDM 6M rate */
		pattrib->rate = MGN_6M;
		pattrib->raid = rtw_get_mgntframe_raid(padapter, WIRELESS_11G);
	}

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	mac = adapter_mac_addr(padapter);

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;
	memcpy(pwlanhdr->addr1, da, ETH_ALEN);
	memcpy(pwlanhdr->addr2, mac, ETH_ALEN);

	/*	Use the device address for BSSID field.	 */
	memcpy(pwlanhdr->addr3, mac, ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(fctrl, WIFI_PROBERSP);

	pattrib->hdrlen = sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = pattrib->hdrlen;
	pframe += pattrib->hdrlen;

	/* timestamp will be inserted by hardware */
	pframe += 8;
	pattrib->pktlen += 8;

	/* beacon interval: 2 bytes */
	memcpy(pframe, (unsigned char *) &beacon_interval, 2);
	pframe += 2;
	pattrib->pktlen += 2;

	/*	capability info: 2 bytes */
	/*	ESS and IBSS bits must be 0 (defined in the 3.1.2.1.1 of WiFi Direct Spec) */
	capInfo |= cap_ShortPremble;
	capInfo |= cap_ShortSlot;

	memcpy(pframe, (unsigned char *) &capInfo, 2);
	pframe += 2;
	pattrib->pktlen += 2;


	/* SSID */
	pframe = rtw_set_ie(pframe, _SSID_IE_, 7, pwdinfo->p2p_wildcard_ssid, &pattrib->pktlen);

	/* supported rates... */
	/*	Use the OFDM rate in the P2P probe response frame. ( 6(B), 9(B), 12, 18, 24, 36, 48, 54 ) */
	pframe = rtw_set_ie(pframe, _SUPPORTEDRATES_IE_, 8, pwdinfo->support_rate, &pattrib->pktlen);

	/* DS parameter set */
	pframe = rtw_set_ie(pframe, _DSSET_IE_, 1, (unsigned char *)&pwdinfo->listen_channel, &pattrib->pktlen);

#ifdef CONFIG_IOCTL_CFG80211
	if (adapter_wdev_data(padapter)->p2p_enabled && pwdinfo->driver_interface == DRIVER_CFG80211) {
		if (pmlmepriv->wps_probe_resp_ie != NULL && pmlmepriv->p2p_probe_resp_ie != NULL) {
			/* WPS IE */
			memcpy(pframe, pmlmepriv->wps_probe_resp_ie, pmlmepriv->wps_probe_resp_ie_len);
			pattrib->pktlen += pmlmepriv->wps_probe_resp_ie_len;
			pframe += pmlmepriv->wps_probe_resp_ie_len;

			/* P2P IE */
			memcpy(pframe, pmlmepriv->p2p_probe_resp_ie, pmlmepriv->p2p_probe_resp_ie_len);
			pattrib->pktlen += pmlmepriv->p2p_probe_resp_ie_len;
			pframe += pmlmepriv->p2p_probe_resp_ie_len;
		}
	} else
#endif /* CONFIG_IOCTL_CFG80211		 */
	{

		/*	Todo: WPS IE */
		/*	Noted by Albert 20100907 */
		/*	According to the WPS specification, all the WPS attribute is presented by Big Endian. */

		wpsielen = 0;
		/*	WPS OUI */
		*(__be32 *)(wpsie) = cpu_to_be32(WPSOUI);
		wpsielen += 4;

		/*	WPS version */
		/*	Type: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_VER1);
		wpsielen += 2;

		/*	Length: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0001);
		wpsielen += 2;

		/*	Value: */
		wpsie[wpsielen++] = WPS_VERSION_1;	/*	Version 1.0 */

#ifdef CONFIG_INTEL_WIDI
		/*	Commented by Kurt */
		/*	Appended WiDi info. only if we did issued_probereq_widi(), and then we saved ven. ext. in pmlmepriv->sa_ext. */
		if (!memcmp(pmlmepriv->sa_ext, zero_array_check, L2SDTA_SERVICE_VE_LEN) == false
		    || pmlmepriv->num_p2p_sdt != 0) {
			/* Sec dev type */
			*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_SEC_DEV_TYPE_LIST);
			wpsielen += 2;

			/*	Length: */
			*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0008);
			wpsielen += 2;

			/*	Value: */
			/*	Category ID */
			*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_PDT_CID_DISPLAYS);
			wpsielen += 2;

			/*	OUI */
			*(__be32 *)(wpsie + wpsielen) = cpu_to_be32(INTEL_DEV_TYPE_OUI);
			wpsielen += 4;

			*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_PDT_SCID_WIDI_CONSUMER_SINK);
			wpsielen += 2;

			if (!memcmp(pmlmepriv->sa_ext, zero_array_check, L2SDTA_SERVICE_VE_LEN) == false) {
				/*	Vendor Extension */
				memcpy(wpsie + wpsielen, pmlmepriv->sa_ext, L2SDTA_SERVICE_VE_LEN);
				wpsielen += L2SDTA_SERVICE_VE_LEN;
			}
		}
#endif /* CONFIG_INTEL_WIDI */

		/*	WiFi Simple Config State */
		/*	Type: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_SIMPLE_CONF_STATE);
		wpsielen += 2;

		/*	Length: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0001);
		wpsielen += 2;

		/*	Value: */
		wpsie[wpsielen++] = WPS_WSC_STATE_NOT_CONFIG;	/*	Not Configured. */

		/*	Response Type */
		/*	Type: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_RESP_TYPE);
		wpsielen += 2;

		/*	Length: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0001);
		wpsielen += 2;

		/*	Value: */
		wpsie[wpsielen++] = WPS_RESPONSE_TYPE_8021X;

		/*	UUID-E */
		/*	Type: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_UUID_E);
		wpsielen += 2;

		/*	Length: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0010);
		wpsielen += 2;

		/*	Value: */
		if (pwdinfo->external_uuid == 0) {
			memset(wpsie + wpsielen, 0x0, 16);
			memcpy(wpsie + wpsielen, mac, ETH_ALEN);
		} else
			memcpy(wpsie + wpsielen, pwdinfo->uuid, 0x10);
		wpsielen += 0x10;

		/*	Manufacturer */
		/*	Type: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_MANUFACTURER);
		wpsielen += 2;

		/*	Length: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0007);
		wpsielen += 2;

		/*	Value: */
		memcpy(wpsie + wpsielen, "Realtek", 7);
		wpsielen += 7;

		/*	Model Name */
		/*	Type: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_MODEL_NAME);
		wpsielen += 2;

		/*	Length: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0006);
		wpsielen += 2;

		/*	Value: */
		memcpy(wpsie + wpsielen, "8192CU", 6);
		wpsielen += 6;

		/*	Model Number */
		/*	Type: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_MODEL_NUMBER);
		wpsielen += 2;

		/*	Length: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0001);
		wpsielen += 2;

		/*	Value: */
		wpsie[wpsielen++] = 0x31;		/*	character 1 */

		/*	Serial Number */
		/*	Type: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_SERIAL_NUMBER);
		wpsielen += 2;

		/*	Length: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(ETH_ALEN);
		wpsielen += 2;

		/*	Value: */
		memcpy(wpsie + wpsielen, "123456" , ETH_ALEN);
		wpsielen += ETH_ALEN;

		/*	Primary Device Type */
		/*	Type: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_PRIMARY_DEV_TYPE);
		wpsielen += 2;

		/*	Length: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0008);
		wpsielen += 2;

		/*	Value: */
		/*	Category ID */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_PDT_CID_MULIT_MEDIA);
		wpsielen += 2;

		/*	OUI */
		*(__be32 *)(wpsie + wpsielen) = cpu_to_be32(WPSOUI);
		wpsielen += 4;

		/*	Sub Category ID */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_PDT_SCID_MEDIA_SERVER);
		wpsielen += 2;

		/*	Device Name */
		/*	Type: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_DEVICE_NAME);
		wpsielen += 2;

		/*	Length: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(pwdinfo->device_name_len);
		wpsielen += 2;

		/*	Value: */
		memcpy(wpsie + wpsielen, pwdinfo->device_name, pwdinfo->device_name_len);
		wpsielen += pwdinfo->device_name_len;

		/*	Config Method */
		/*	Type: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_CONF_METHOD);
		wpsielen += 2;

		/*	Length: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0002);
		wpsielen += 2;

		/*	Value: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(pwdinfo->supported_wps_cm);
		wpsielen += 2;


		pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, wpsielen, (unsigned char *) wpsie, &pattrib->pktlen);


		p2pielen = build_probe_resp_p2p_ie(pwdinfo, pframe);
		pframe += p2pielen;
		pattrib->pktlen += p2pielen;
	}

#ifdef CONFIG_WFD
	wfdielen = rtw_append_probe_resp_wfd_ie(padapter, pframe);
	pframe += wfdielen;
	pattrib->pktlen += wfdielen;
#endif

	pattrib->last_txcmdsz = pattrib->pktlen;


	dump_mgntframe(padapter, pmgntframe);

	return;
}

static int _issue_probereq_p2p(_adapter *padapter, u8 *da, int wait_ack)
{
	int ret = _FAIL;
	struct xmit_frame		*pmgntframe;
	struct pkt_attrib		*pattrib;
	unsigned char			*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	unsigned char			*mac;
	unsigned char			bssrate[NumRates];
	struct xmit_priv		*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	int	bssrate_len = 0;
	u8	bc_addr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	u8					wpsie[255] = { 0x00 }, p2pie[255] = { 0x00 };
	u16					wpsielen = 0, p2pielen = 0;
#ifdef CONFIG_WFD
	u32					wfdielen = 0;
#endif

	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);


	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	if (IS_CCK_RATE(pattrib->rate)) {
		/* force OFDM 6M rate */
		pattrib->rate = MGN_6M;
		pattrib->raid = rtw_get_mgntframe_raid(padapter, WIRELESS_11G);
	}

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	mac = adapter_mac_addr(padapter);

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	if (da) {
		memcpy(pwlanhdr->addr1, da, ETH_ALEN);
		memcpy(pwlanhdr->addr3, da, ETH_ALEN);
	} else {
		if ((pwdinfo->p2p_info.scan_op_ch_only) || (pwdinfo->rx_invitereq_info.scan_op_ch_only)) {
			/*	This two flags will be set when this is only the P2P client mode. */
			memcpy(pwlanhdr->addr1, pwdinfo->p2p_peer_interface_addr, ETH_ALEN);
			memcpy(pwlanhdr->addr3, pwdinfo->p2p_peer_interface_addr, ETH_ALEN);
		} else {
			/*	broadcast probe request frame */
			memcpy(pwlanhdr->addr1, bc_addr, ETH_ALEN);
			memcpy(pwlanhdr->addr3, bc_addr, ETH_ALEN);
		}
	}
	memcpy(pwlanhdr->addr2, mac, ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_PROBEREQ);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_TX_PROVISION_DIS_REQ))
		pframe = rtw_set_ie(pframe, _SSID_IE_, pwdinfo->tx_prov_disc_info.ssid.SsidLength, pwdinfo->tx_prov_disc_info.ssid.Ssid, &(pattrib->pktlen));
	else
		pframe = rtw_set_ie(pframe, _SSID_IE_, P2P_WILDCARD_SSID_LEN, pwdinfo->p2p_wildcard_ssid, &(pattrib->pktlen));
	/*	Use the OFDM rate in the P2P probe request frame. ( 6(B), 9(B), 12(B), 24(B), 36, 48, 54 ) */
	pframe = rtw_set_ie(pframe, _SUPPORTEDRATES_IE_, 8, pwdinfo->support_rate, &pattrib->pktlen);

#ifdef CONFIG_IOCTL_CFG80211
	if (adapter_wdev_data(padapter)->p2p_enabled && pwdinfo->driver_interface == DRIVER_CFG80211) {
		if (pmlmepriv->wps_probe_req_ie != NULL && pmlmepriv->p2p_probe_req_ie != NULL) {
			/* WPS IE */
			memcpy(pframe, pmlmepriv->wps_probe_req_ie, pmlmepriv->wps_probe_req_ie_len);
			pattrib->pktlen += pmlmepriv->wps_probe_req_ie_len;
			pframe += pmlmepriv->wps_probe_req_ie_len;

			/* P2P IE */
			memcpy(pframe, pmlmepriv->p2p_probe_req_ie, pmlmepriv->p2p_probe_req_ie_len);
			pattrib->pktlen += pmlmepriv->p2p_probe_req_ie_len;
			pframe += pmlmepriv->p2p_probe_req_ie_len;
		}
	} else
#endif /* CONFIG_IOCTL_CFG80211 */
	{

		/*	WPS IE */
		/*	Noted by Albert 20110221 */
		/*	According to the WPS specification, all the WPS attribute is presented by Big Endian. */

		wpsielen = 0;
		/*	WPS OUI */
		*(__be32 *)(wpsie) = cpu_to_be32(WPSOUI);
		wpsielen += 4;

		/*	WPS version */
		/*	Type: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_VER1);
		wpsielen += 2;

		/*	Length: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0001);
		wpsielen += 2;

		/*	Value: */
		wpsie[wpsielen++] = WPS_VERSION_1;	/*	Version 1.0 */

		if (pmlmepriv->wps_probe_req_ie == NULL) {
			/*	UUID-E */
			/*	Type: */
			*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_UUID_E);
			wpsielen += 2;

			/*	Length: */
			*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0010);
			wpsielen += 2;
			/*	Value: */
			if (pwdinfo->external_uuid == 0) {
				memset(wpsie + wpsielen, 0x0, 16);
				memcpy(wpsie + wpsielen, mac, ETH_ALEN);
			} else
				memcpy(wpsie + wpsielen, pwdinfo->uuid, 0x10);
			wpsielen += 0x10;

			/*	Config Method */
			/*	Type: */
			*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_CONF_METHOD);
			wpsielen += 2;

			/*	Length: */
			*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0002);
			wpsielen += 2;

			/*	Value: */
			*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(pwdinfo->supported_wps_cm);
			wpsielen += 2;
		}

		/*	Device Name */
		/*	Type: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_DEVICE_NAME);
		wpsielen += 2;

		/*	Length: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(pwdinfo->device_name_len);
		wpsielen += 2;

		/*	Value: */
		memcpy(wpsie + wpsielen, pwdinfo->device_name, pwdinfo->device_name_len);
		wpsielen += pwdinfo->device_name_len;

		/*	Primary Device Type */
		/*	Type: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_PRIMARY_DEV_TYPE);
		wpsielen += 2;

		/*	Length: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0008);
		wpsielen += 2;

		/*	Value: */
		/*	Category ID */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_PDT_CID_RTK_WIDI);
		wpsielen += 2;

		/*	OUI */
		*(__be32 *)(wpsie + wpsielen) = cpu_to_be32(WPSOUI);
		wpsielen += 4;

		/*	Sub Category ID */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_PDT_SCID_RTK_DMP);
		wpsielen += 2;

		/*	Device Password ID */
		/*	Type: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_ATTR_DEVICE_PWID);
		wpsielen += 2;

		/*	Length: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(0x0002);
		wpsielen += 2;

		/*	Value: */
		*(__be16 *)(wpsie + wpsielen) = cpu_to_be16(WPS_DPID_REGISTRAR_SPEC);	/*	Registrar-specified */
		wpsielen += 2;

		pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, wpsielen, (unsigned char *) wpsie, &pattrib->pktlen);

		/*	P2P OUI */
		p2pielen = 0;
		p2pie[p2pielen++] = 0x50;
		p2pie[p2pielen++] = 0x6F;
		p2pie[p2pielen++] = 0x9A;
		p2pie[p2pielen++] = 0x09;	/*	WFA P2P v1.0 */

		/*	Commented by Albert 20110221 */
		/*	According to the P2P Specification, the probe request frame should contain 5 P2P attributes */
		/*	1. P2P Capability */
		/*	2. P2P Device ID if this probe request wants to find the specific P2P device */
		/*	3. Listen Channel */
		/*	4. Extended Listen Timing */
		/*	5. Operating Channel if this WiFi is working as the group owner now */

		/*	P2P Capability */
		/*	Type: */
		p2pie[p2pielen++] = P2P_ATTR_CAPABILITY;

		/*	Length: */
		*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0002);
		p2pielen += 2;

		/*	Value: */
		/*	Device Capability Bitmap, 1 byte */
		p2pie[p2pielen++] = DMP_P2P_DEVCAP_SUPPORT;

		/*	Group Capability Bitmap, 1 byte */
		if (pwdinfo->persistent_supported)
			p2pie[p2pielen++] = P2P_GRPCAP_PERSISTENT_GROUP | DMP_P2P_GRPCAP_SUPPORT;
		else
			p2pie[p2pielen++] = DMP_P2P_GRPCAP_SUPPORT;

		/*	Listen Channel */
		/*	Type: */
		p2pie[p2pielen++] = P2P_ATTR_LISTEN_CH;

		/*	Length: */
		*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0005);
		p2pielen += 2;

		/*	Value: */
		/*	Country String */
		p2pie[p2pielen++] = 'X';
		p2pie[p2pielen++] = 'X';

		/*	The third byte should be set to 0x04. */
		/*	Described in the "Operating Channel Attribute" section. */
		p2pie[p2pielen++] = 0x04;

		/*	Operating Class */
		p2pie[p2pielen++] = 0x51;	/*	Copy from SD7 */

		/*	Channel Number */
		p2pie[p2pielen++] = pwdinfo->listen_channel;	/*	listen channel */


		/*	Extended Listen Timing */
		/*	Type: */
		p2pie[p2pielen++] = P2P_ATTR_EX_LISTEN_TIMING;

		/*	Length: */
		*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0004);
		p2pielen += 2;

		/*	Value: */
		/*	Availability Period */
		*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0xFFFF);
		p2pielen += 2;

		/*	Availability Interval */
		*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0xFFFF);
		p2pielen += 2;

		if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO)) {
			/*	Operating Channel (if this WiFi is working as the group owner now) */
			/*	Type: */
			p2pie[p2pielen++] = P2P_ATTR_OPERATING_CH;

			/*	Length: */
			*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0005);
			p2pielen += 2;

			/*	Value: */
			/*	Country String */
			p2pie[p2pielen++] = 'X';
			p2pie[p2pielen++] = 'X';

			/*	The third byte should be set to 0x04. */
			/*	Described in the "Operating Channel Attribute" section. */
			p2pie[p2pielen++] = 0x04;

			/*	Operating Class */
			p2pie[p2pielen++] = 0x51;	/*	Copy from SD7 */

			/*	Channel Number */
			p2pie[p2pielen++] = pwdinfo->operating_channel;	/*	operating channel number */

		}

		pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &pattrib->pktlen);

	}

#ifdef CONFIG_WFD
	wfdielen = rtw_append_probe_req_wfd_ie(padapter, pframe);
	pframe += wfdielen;
	pattrib->pktlen += wfdielen;
#endif

	pattrib->last_txcmdsz = pattrib->pktlen;


	if (wait_ack)
		ret = dump_mgntframe_and_wait_ack(padapter, pmgntframe);
	else {
		dump_mgntframe(padapter, pmgntframe);
		ret = _SUCCESS;
	}

exit:
	return ret;
}

inline void issue_probereq_p2p(_adapter *adapter, u8 *da)
{
	_issue_probereq_p2p(adapter, da, false);
}

/*
 * wait_ms == 0 means that there is no need to wait ack through C2H_CCX_TX_RPT
 * wait_ms > 0 means you want to wait ack through C2H_CCX_TX_RPT, and the value of wait_ms means the interval between each TX
 * try_cnt means the maximal TX count to try
 */
int issue_probereq_p2p_ex(_adapter *adapter, u8 *da, int try_cnt, int wait_ms)
{
	int ret;
	int i = 0;
	u32 start = jiffies;

	do {
		ret = _issue_probereq_p2p(adapter, da, wait_ms > 0 ? true : false);

		i++;

		if (RTW_CANNOT_RUN(adapter))
			break;

		if (i < try_cnt && wait_ms > 0 && ret == _FAIL)
			rtw_msleep_os(wait_ms);

	} while ((i < try_cnt) && ((ret == _FAIL) || (wait_ms == 0)));

	if (ret != _FAIL) {
		ret = _SUCCESS;
#ifndef DBG_XMIT_ACK
		goto exit;
#endif
	}

	if (try_cnt && wait_ms) {
		if (da)
			RTW_INFO(FUNC_ADPT_FMT" to "MAC_FMT", ch:%u%s, %d/%d in %u ms\n",
				FUNC_ADPT_ARG(adapter), MAC_ARG(da), rtw_get_oper_ch(adapter),
				ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
		else
			RTW_INFO(FUNC_ADPT_FMT", ch:%u%s, %d/%d in %u ms\n",
				FUNC_ADPT_ARG(adapter), rtw_get_oper_ch(adapter),
				ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
	}
exit:
	return ret;
}

#endif /* CONFIG_P2P */

static s32 rtw_action_public_decache(union recv_frame *rframe, u8 token_offset)
{
	_adapter *adapter = rframe->u.hdr.adapter;
	struct mlme_ext_priv *mlmeext = &(adapter->mlmeextpriv);
	u8 *frame = rframe->u.hdr.rx_data;
	u16 seq_ctrl = ((le16_to_cpu(rframe->u.hdr.attrib.seq_num) & 0xffff) << 4) |
			((rframe->u.hdr.attrib.frag_num) & 0xf);
	u8 token = *(rframe->u.hdr.rx_data + sizeof(struct rtw_ieee80211_hdr_3addr) + token_offset);

	if (GetRetry(frame)) {
		if ((seq_ctrl == mlmeext->action_public_rxseq)
		    && (token == mlmeext->action_public_dialog_token)
		   ) {
			RTW_INFO(FUNC_ADPT_FMT" seq_ctrl=0x%x, rxseq=0x%x, token:%d\n",
				FUNC_ADPT_ARG(adapter), seq_ctrl, mlmeext->action_public_rxseq, token);
			return _FAIL;
		}
	}

	/* TODO: per sta seq & token */
	mlmeext->action_public_rxseq = seq_ctrl;
	mlmeext->action_public_dialog_token = token;

	return _SUCCESS;
}

static unsigned int on_action_public_p2p(union recv_frame *precv_frame)
{
	_adapter *padapter = precv_frame->u.hdr.adapter;
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint len = precv_frame->u.hdr.len;
	u8 *frame_body;
#ifdef CONFIG_P2P
	u8 *p2p_ie;
	u32	p2p_ielen, wps_ielen;
	struct	wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	u8	result = P2P_STATUS_SUCCESS;
	u8	empty_addr[ETH_ALEN] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	u8 *merged_p2pie = NULL;
	u32 merged_p2p_ielen = 0;
#endif /* CONFIG_P2P */

	frame_body = (unsigned char *)(pframe + sizeof(struct rtw_ieee80211_hdr_3addr));

#ifdef CONFIG_P2P
	_cancel_timer_ex(&pwdinfo->reset_ch_sitesurvey);
#ifdef CONFIG_IOCTL_CFG80211
	if (adapter_wdev_data(padapter)->p2p_enabled && pwdinfo->driver_interface == DRIVER_CFG80211)
		rtw_cfg80211_rx_p2p_action_public(padapter, precv_frame);
	else
#endif /* CONFIG_IOCTL_CFG80211 */
	{
		/*	Do nothing if the driver doesn't enable the P2P function. */
		if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE) || rtw_p2p_chk_state(pwdinfo, P2P_STATE_IDLE))
			return _SUCCESS;

		len -= sizeof(struct rtw_ieee80211_hdr_3addr);

		switch (frame_body[6]) { /* OUI Subtype */
		case P2P_GO_NEGO_REQ: {
			RTW_INFO("[%s] Got GO Nego Req Frame\n", __func__);
			memset(&pwdinfo->groupid_info, 0x00, sizeof(struct group_id_info));

			if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_RX_PROVISION_DIS_REQ))
				rtw_p2p_set_state(pwdinfo, rtw_p2p_pre_state(pwdinfo));

			if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_GONEGO_FAIL)) {
				/*	Commented by Albert 20110526 */
				/*	In this case, this means the previous nego fail doesn't be reset yet. */
				_cancel_timer_ex(&pwdinfo->restore_p2p_state_timer);
				/*	Restore the previous p2p state */
				rtw_p2p_set_state(pwdinfo, rtw_p2p_pre_state(pwdinfo));
				RTW_INFO("[%s] Restore the previous p2p state to %d\n", __func__, rtw_p2p_state(pwdinfo));
			}
#ifdef CONFIG_CONCURRENT_MODE
			if (rtw_mi_buddy_check_fwstate(padapter, _FW_LINKED))
				_cancel_timer_ex(&pwdinfo->ap_p2p_switch_timer);
#endif /* CONFIG_CONCURRENT_MODE */

			/*	Commented by Kurt 20110902 */
			/* Add if statement to avoid receiving duplicate prov disc req. such that pre_p2p_state would be covered. */
			if (!rtw_p2p_chk_state(pwdinfo, P2P_STATE_GONEGO_ING))
				rtw_p2p_set_pre_state(pwdinfo, rtw_p2p_state(pwdinfo));

			/*	Commented by Kurt 20120113 */
			/*	Get peer_dev_addr here if peer doesn't issue prov_disc frame. */
			if (!memcmp(pwdinfo->rx_prov_disc_info.peerDevAddr, empty_addr, ETH_ALEN))
				memcpy(pwdinfo->rx_prov_disc_info.peerDevAddr, get_addr2_ptr(pframe), ETH_ALEN);

			result = process_p2p_group_negotation_req(pwdinfo, frame_body, len);
			issue_p2p_GO_response(padapter, get_addr2_ptr(pframe), frame_body, len, result);
#ifdef CONFIG_INTEL_WIDI
			if (padapter->mlmepriv.widi_state == INTEL_WIDI_STATE_LISTEN) {
				padapter->mlmepriv.widi_state = INTEL_WIDI_STATE_WFD_CONNECTION;
				_cancel_timer_ex(&(padapter->mlmepriv.listen_timer));
				intel_widi_wk_cmd(padapter, INTEL_WIDI_LISTEN_STOP_WK, NULL, 0);
			}
#endif /* CONFIG_INTEL_WIDI */

			/*	Commented by Albert 20110718 */
			/*	No matter negotiating or negotiation failure, the driver should set up the restore P2P state timer. */
#ifdef CONFIG_CONCURRENT_MODE
			/*	Commented by Albert 20120107 */
			_set_timer(&pwdinfo->restore_p2p_state_timer, 3000);
#else /* CONFIG_CONCURRENT_MODE */
			_set_timer(&pwdinfo->restore_p2p_state_timer, 5000);
#endif /* CONFIG_CONCURRENT_MODE */
			break;
		}
		case P2P_GO_NEGO_RESP: {
			RTW_INFO("[%s] Got GO Nego Resp Frame\n", __func__);

			if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_GONEGO_ING)) {
				/*	Commented by Albert 20110425 */
				/*	The restore timer is enabled when issuing the nego request frame of rtw_p2p_connect function. */
				_cancel_timer_ex(&pwdinfo->restore_p2p_state_timer);
				pwdinfo->nego_req_info.benable = false;
				result = process_p2p_group_negotation_resp(pwdinfo, frame_body, len);
				issue_p2p_GO_confirm(pwdinfo->padapter, get_addr2_ptr(pframe), result);
				if (P2P_STATUS_SUCCESS == result) {
					if (rtw_p2p_role(pwdinfo) == P2P_ROLE_CLIENT) {
						pwdinfo->p2p_info.operation_ch[0] = pwdinfo->peer_operating_ch;
#ifdef CONFIG_P2P_OP_CHK_SOCIAL_CH
						pwdinfo->p2p_info.operation_ch[1] = 1;	/* Check whether GO is operating in channel 1; */
						pwdinfo->p2p_info.operation_ch[2] = 6;	/* Check whether GO is operating in channel 6; */
						pwdinfo->p2p_info.operation_ch[3] = 11;	/* Check whether GO is operating in channel 11; */
#endif /* CONFIG_P2P_OP_CHK_SOCIAL_CH */
						pwdinfo->p2p_info.scan_op_ch_only = 1;
						_set_timer(&pwdinfo->reset_ch_sitesurvey2, P2P_RESET_SCAN_CH);
					}
				}

				/*	Reset the dialog token for group negotiation frames. */
				pwdinfo->negotiation_dialog_token = 1;

				if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_GONEGO_FAIL))
					_set_timer(&pwdinfo->restore_p2p_state_timer, 5000);
			} else
				RTW_INFO("[%s] Skipped GO Nego Resp Frame (p2p_state != P2P_STATE_GONEGO_ING)\n", __func__);

			break;
		}
		case P2P_GO_NEGO_CONF: {
			RTW_INFO("[%s] Got GO Nego Confirm Frame\n", __func__);
			result = process_p2p_group_negotation_confirm(pwdinfo, frame_body, len);
			if (P2P_STATUS_SUCCESS == result) {
				if (rtw_p2p_role(pwdinfo) == P2P_ROLE_CLIENT) {
					pwdinfo->p2p_info.operation_ch[0] = pwdinfo->peer_operating_ch;
#ifdef CONFIG_P2P_OP_CHK_SOCIAL_CH
					pwdinfo->p2p_info.operation_ch[1] = 1;	/* Check whether GO is operating in channel 1; */
					pwdinfo->p2p_info.operation_ch[2] = 6;	/* Check whether GO is operating in channel 6; */
					pwdinfo->p2p_info.operation_ch[3] = 11;	/* Check whether GO is operating in channel 11; */
#endif /* CONFIG_P2P_OP_CHK_SOCIAL_CH */
					pwdinfo->p2p_info.scan_op_ch_only = 1;
					_set_timer(&pwdinfo->reset_ch_sitesurvey2, P2P_RESET_SCAN_CH);
				}
			}
			break;
		}
		case P2P_INVIT_REQ: {
			/*	Added by Albert 2010/10/05 */
			/*	Received the P2P Invite Request frame. */

			RTW_INFO("[%s] Got invite request frame!\n", __func__);
			p2p_ie = rtw_get_p2p_ie(frame_body + _PUBLIC_ACTION_IE_OFFSET_, len - _PUBLIC_ACTION_IE_OFFSET_, NULL, &p2p_ielen);
			if (p2p_ie) {
				/*	Parse the necessary information from the P2P Invitation Request frame. */
				/*	For example: The MAC address of sending this P2P Invitation Request frame. */
				u32	attr_contentlen = 0;
				u8	status_code = P2P_STATUS_FAIL_INFO_UNAVAILABLE;
				struct group_id_info group_id;
				u8	invitation_flag = 0;
				int j = 0;

				merged_p2p_ielen = rtw_get_p2p_merged_ies_len(frame_body + _PUBLIC_ACTION_IE_OFFSET_, len - _PUBLIC_ACTION_IE_OFFSET_);

				merged_p2pie = rtw_zmalloc(merged_p2p_ielen + 2);	/* 2 is for EID and Length */
				if (merged_p2pie == NULL) {
					RTW_INFO("[%s] Malloc p2p ie fail\n", __func__);
					goto exit;
				}
				memset(merged_p2pie, 0x00, merged_p2p_ielen);

				merged_p2p_ielen = rtw_p2p_merge_ies(frame_body + _PUBLIC_ACTION_IE_OFFSET_, len - _PUBLIC_ACTION_IE_OFFSET_, merged_p2pie);

				rtw_get_p2p_attr_content(merged_p2pie, merged_p2p_ielen, P2P_ATTR_INVITATION_FLAGS, &invitation_flag, &attr_contentlen);
				if (attr_contentlen) {

					rtw_get_p2p_attr_content(merged_p2pie, merged_p2p_ielen, P2P_ATTR_GROUP_BSSID, pwdinfo->p2p_peer_interface_addr, &attr_contentlen);
					/*	Commented by Albert 20120510 */
					/*	Copy to the pwdinfo->p2p_peer_interface_addr. */
					/*	So that the WFD UI ( or Sigma ) can get the peer interface address by using the following command. */
					/*	#> iwpriv wlan0 p2p_get peer_ifa */
					/*	After having the peer interface address, the sigma can find the correct conf file for wpa_supplicant. */

					if (attr_contentlen) {
						RTW_INFO("[%s] GO's BSSID = %.2X %.2X %.2X %.2X %.2X %.2X\n", __func__,
							pwdinfo->p2p_peer_interface_addr[0], pwdinfo->p2p_peer_interface_addr[1],
							pwdinfo->p2p_peer_interface_addr[2], pwdinfo->p2p_peer_interface_addr[3],
							pwdinfo->p2p_peer_interface_addr[4], pwdinfo->p2p_peer_interface_addr[5]);
					}

					if (invitation_flag & P2P_INVITATION_FLAGS_PERSISTENT) {
						/*	Re-invoke the persistent group. */

						memset(&group_id, 0x00, sizeof(struct group_id_info));
						rtw_get_p2p_attr_content(merged_p2pie, merged_p2p_ielen, P2P_ATTR_GROUP_ID, (u8 *) &group_id, &attr_contentlen);
						if (attr_contentlen) {
							if (!memcmp(group_id.go_device_addr, adapter_mac_addr(padapter), ETH_ALEN)) {
								/*	The p2p device sending this p2p invitation request wants this Wi-Fi device to be the persistent GO. */
								rtw_p2p_set_state(pwdinfo, P2P_STATE_RECV_INVITE_REQ_GO);
								rtw_p2p_set_role(pwdinfo, P2P_ROLE_GO);
								status_code = P2P_STATUS_SUCCESS;
							} else {
								/*	The p2p device sending this p2p invitation request wants to be the persistent GO. */
								if (is_matched_in_profilelist(pwdinfo->p2p_peer_interface_addr, &pwdinfo->profileinfo[0])) {
									u8 operatingch_info[5] = { 0x00 };
									if (rtw_get_p2p_attr_content(merged_p2pie, merged_p2p_ielen, P2P_ATTR_OPERATING_CH, operatingch_info,
										&attr_contentlen)) {
										if (rtw_ch_set_search_ch(padapter->mlmeextpriv.channel_set, (u32)operatingch_info[4]) >= 0) {
											/*	The operating channel is acceptable for this device. */
											pwdinfo->rx_invitereq_info.operation_ch[0] = operatingch_info[4];
#ifdef CONFIG_P2P_OP_CHK_SOCIAL_CH
											pwdinfo->rx_invitereq_info.operation_ch[1] = 1;		/* Check whether GO is operating in channel 1; */
											pwdinfo->rx_invitereq_info.operation_ch[2] = 6;		/* Check whether GO is operating in channel 6; */
											pwdinfo->rx_invitereq_info.operation_ch[3] = 11;		/* Check whether GO is operating in channel 11; */
#endif /* CONFIG_P2P_OP_CHK_SOCIAL_CH */
											pwdinfo->rx_invitereq_info.scan_op_ch_only = 1;
											_set_timer(&pwdinfo->reset_ch_sitesurvey, P2P_RESET_SCAN_CH);
											rtw_p2p_set_state(pwdinfo, P2P_STATE_RECV_INVITE_REQ_MATCH);
											rtw_p2p_set_role(pwdinfo, P2P_ROLE_CLIENT);
											status_code = P2P_STATUS_SUCCESS;
										} else {
											/*	The operating channel isn't supported by this device. */
											rtw_p2p_set_state(pwdinfo, P2P_STATE_RECV_INVITE_REQ_DISMATCH);
											rtw_p2p_set_role(pwdinfo, P2P_ROLE_DEVICE);
											status_code = P2P_STATUS_FAIL_NO_COMMON_CH;
											_set_timer(&pwdinfo->restore_p2p_state_timer, 3000);
										}
									} else {
										/*	Commented by Albert 20121130 */
										/*	Intel will use the different P2P IE to store the operating channel information */
										/*	Workaround for Intel WiDi 3.5 */
										rtw_p2p_set_state(pwdinfo, P2P_STATE_RECV_INVITE_REQ_MATCH);
										rtw_p2p_set_role(pwdinfo, P2P_ROLE_CLIENT);
										status_code = P2P_STATUS_SUCCESS;
									}
								} else {
									rtw_p2p_set_state(pwdinfo, P2P_STATE_RECV_INVITE_REQ_DISMATCH);
#ifdef CONFIG_INTEL_WIDI
									memcpy(pwdinfo->p2p_peer_device_addr, group_id.go_device_addr , ETH_ALEN);
									rtw_p2p_set_role(pwdinfo, P2P_ROLE_CLIENT);
#endif /* CONFIG_INTEL_WIDI */

									status_code = P2P_STATUS_FAIL_UNKNOWN_P2PGROUP;
								}
							}
						} else {
							RTW_INFO("[%s] P2P Group ID Attribute NOT FOUND!\n", __func__);
							status_code = P2P_STATUS_FAIL_INFO_UNAVAILABLE;
						}
					} else {
						/*	Received the invitation to join a P2P group. */

						memset(&group_id, 0x00, sizeof(struct group_id_info));
						rtw_get_p2p_attr_content(merged_p2pie, merged_p2p_ielen, P2P_ATTR_GROUP_ID, (u8 *) &group_id, &attr_contentlen);
						if (attr_contentlen) {
							if (!memcmp(group_id.go_device_addr, adapter_mac_addr(padapter), ETH_ALEN)) {
								/*	In this case, the GO can't be myself. */
								rtw_p2p_set_state(pwdinfo, P2P_STATE_RECV_INVITE_REQ_DISMATCH);
								status_code = P2P_STATUS_FAIL_INFO_UNAVAILABLE;
							} else {
								/*	The p2p device sending this p2p invitation request wants to join an existing P2P group */
								/*	Commented by Albert 2012/06/28 */
								/*	In this case, this Wi-Fi device should use the iwpriv command to get the peer device address. */
								/*	The peer device address should be the destination address for the provisioning discovery request. */
								/*	Then, this Wi-Fi device should use the iwpriv command to get the peer interface address. */
								/*	The peer interface address should be the address for WPS mac address */
								memcpy(pwdinfo->p2p_peer_device_addr, group_id.go_device_addr , ETH_ALEN);
								rtw_p2p_set_role(pwdinfo, P2P_ROLE_CLIENT);
								rtw_p2p_set_state(pwdinfo, P2P_STATE_RECV_INVITE_REQ_JOIN);
								status_code = P2P_STATUS_SUCCESS;
							}
						} else {
							RTW_INFO("[%s] P2P Group ID Attribute NOT FOUND!\n", __func__);
							status_code = P2P_STATUS_FAIL_INFO_UNAVAILABLE;
						}
					}
				} else {
					RTW_INFO("[%s] P2P Invitation Flags Attribute NOT FOUND!\n", __func__);
					status_code = P2P_STATUS_FAIL_INFO_UNAVAILABLE;
				}

				RTW_INFO("[%s] status_code = %d\n", __func__, status_code);

				pwdinfo->inviteresp_info.token = frame_body[7];
				issue_p2p_invitation_response(padapter, get_addr2_ptr(pframe), pwdinfo->inviteresp_info.token, status_code);
				_set_timer(&pwdinfo->restore_p2p_state_timer, 3000);
			}
#ifdef CONFIG_INTEL_WIDI
			if (padapter->mlmepriv.widi_state == INTEL_WIDI_STATE_LISTEN) {
				padapter->mlmepriv.widi_state = INTEL_WIDI_STATE_WFD_CONNECTION;
				_cancel_timer_ex(&(padapter->mlmepriv.listen_timer));
				intel_widi_wk_cmd(padapter, INTEL_WIDI_LISTEN_STOP_WK, NULL, 0);
			}
#endif /* CONFIG_INTEL_WIDI */
			break;
		}
		case P2P_INVIT_RESP: {
			u8	attr_content = 0x00;
			u32	attr_contentlen = 0;

			RTW_INFO("[%s] Got invite response frame!\n", __func__);
			_cancel_timer_ex(&pwdinfo->restore_p2p_state_timer);
			p2p_ie = rtw_get_p2p_ie(frame_body + _PUBLIC_ACTION_IE_OFFSET_, len - _PUBLIC_ACTION_IE_OFFSET_, NULL, &p2p_ielen);
			if (p2p_ie) {
				rtw_get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_STATUS, &attr_content, &attr_contentlen);

				if (attr_contentlen == 1) {
					RTW_INFO("[%s] Status = %d\n", __func__, attr_content);
					pwdinfo->invitereq_info.benable = false;

					if (attr_content == P2P_STATUS_SUCCESS) {
						if (!memcmp(pwdinfo->invitereq_info.go_bssid, adapter_mac_addr(padapter), ETH_ALEN))
							rtw_p2p_set_role(pwdinfo, P2P_ROLE_GO);
						else
							rtw_p2p_set_role(pwdinfo, P2P_ROLE_CLIENT);

						rtw_p2p_set_state(pwdinfo, P2P_STATE_RX_INVITE_RESP_OK);
					} else {
						rtw_p2p_set_role(pwdinfo, P2P_ROLE_DEVICE);
						rtw_p2p_set_state(pwdinfo, P2P_STATE_RX_INVITE_RESP_FAIL);
					}
				} else {
					rtw_p2p_set_role(pwdinfo, P2P_ROLE_DEVICE);
					rtw_p2p_set_state(pwdinfo, P2P_STATE_RX_INVITE_RESP_FAIL);
				}
			} else {
				rtw_p2p_set_role(pwdinfo, P2P_ROLE_DEVICE);
				rtw_p2p_set_state(pwdinfo, P2P_STATE_RX_INVITE_RESP_FAIL);
			}

			if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_RX_INVITE_RESP_FAIL))
				_set_timer(&pwdinfo->restore_p2p_state_timer, 5000);
			break;
		}
		case P2P_DEVDISC_REQ:

			process_p2p_devdisc_req(pwdinfo, pframe, len);

			break;

		case P2P_DEVDISC_RESP:

			process_p2p_devdisc_resp(pwdinfo, pframe, len);

			break;

		case P2P_PROVISION_DISC_REQ:
			RTW_INFO("[%s] Got Provisioning Discovery Request Frame\n", __func__);
			process_p2p_provdisc_req(pwdinfo, pframe, len);
			memcpy(pwdinfo->rx_prov_disc_info.peerDevAddr, get_addr2_ptr(pframe), ETH_ALEN);

			/* 20110902 Kurt */
			/* Add the following statement to avoid receiving duplicate prov disc req. such that pre_p2p_state would be covered. */
			if (!rtw_p2p_chk_state(pwdinfo, P2P_STATE_RX_PROVISION_DIS_REQ))
				rtw_p2p_set_pre_state(pwdinfo, rtw_p2p_state(pwdinfo));

			rtw_p2p_set_state(pwdinfo, P2P_STATE_RX_PROVISION_DIS_REQ);
			_set_timer(&pwdinfo->restore_p2p_state_timer, P2P_PROVISION_TIMEOUT);
#ifdef CONFIG_INTEL_WIDI
			if (padapter->mlmepriv.widi_state == INTEL_WIDI_STATE_LISTEN) {
				padapter->mlmepriv.widi_state = INTEL_WIDI_STATE_WFD_CONNECTION;
				_cancel_timer_ex(&(padapter->mlmepriv.listen_timer));
				intel_widi_wk_cmd(padapter, INTEL_WIDI_LISTEN_STOP_WK, NULL, 0);
			}
#endif /* CONFIG_INTEL_WIDI */
			break;

		case P2P_PROVISION_DISC_RESP:
			/*	Commented by Albert 20110707 */
			/*	Should we check the pwdinfo->tx_prov_disc_info.bsent flag here?? */
			RTW_INFO("[%s] Got Provisioning Discovery Response Frame\n", __func__);
			/*	Commented by Albert 20110426 */
			/*	The restore timer is enabled when issuing the provisioing request frame in rtw_p2p_prov_disc function. */
			_cancel_timer_ex(&pwdinfo->restore_p2p_state_timer);
			rtw_p2p_set_state(pwdinfo, P2P_STATE_RX_PROVISION_DIS_RSP);
			process_p2p_provdisc_resp(pwdinfo, pframe);
			_set_timer(&pwdinfo->restore_p2p_state_timer, P2P_PROVISION_TIMEOUT);
			break;

		}
	}


exit:

	if (merged_p2pie)
		rtw_mfree(merged_p2pie, merged_p2p_ielen + 2);
#endif /* CONFIG_P2P */
	return _SUCCESS;
}

static unsigned int on_action_public_vendor(union recv_frame *precv_frame)
{
	unsigned int ret = _FAIL;
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint frame_len = precv_frame->u.hdr.len;
	u8 *frame_body = pframe + sizeof(struct rtw_ieee80211_hdr_3addr);

	if (!memcmp(frame_body + 2, P2P_OUI, 4)) {
		if (rtw_action_public_decache(precv_frame, 7) == _FAIL)
			goto exit;

		if (!hal_chk_wl_func(precv_frame->u.hdr.adapter, WL_FUNC_MIRACAST))
			rtw_rframe_del_wfd_ie(precv_frame, 8);

		ret = on_action_public_p2p(precv_frame);
	}

exit:
	return ret;
}

static unsigned int on_action_public_default(union recv_frame *precv_frame, u8 action)
{
	unsigned int ret = _FAIL;
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint frame_len = precv_frame->u.hdr.len;
	u8 *frame_body = pframe + sizeof(struct rtw_ieee80211_hdr_3addr);
	u8 token;
	_adapter *adapter = precv_frame->u.hdr.adapter;
	int cnt = 0;
	char msg[64];

	token = frame_body[2];

	if (rtw_action_public_decache(precv_frame, 2) == _FAIL)
		goto exit;

#ifdef CONFIG_IOCTL_CFG80211
	cnt += sprintf((msg + cnt), "%s(token:%u)", action_public_str(action), token);
	rtw_cfg80211_rx_action(adapter, precv_frame, msg);
#endif

	ret = _SUCCESS;

exit:
	return ret;
}

unsigned int on_action_public(_adapter *padapter, union recv_frame *precv_frame)
{
	unsigned int ret = _FAIL;
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint frame_len = precv_frame->u.hdr.len;
	u8 *frame_body = pframe + sizeof(struct rtw_ieee80211_hdr_3addr);
	u8 category, action;

	/* check RA matches or not */
	if (memcmp(adapter_mac_addr(padapter), GetAddr1Ptr(pframe), ETH_ALEN))
		goto exit;

	category = frame_body[0];
	if (category != RTW_WLAN_CATEGORY_PUBLIC)
		goto exit;

	action = frame_body[1];
	switch (action) {
	case ACT_PUBLIC_BSSCOEXIST:
#ifdef CONFIG_AP_MODE
		/*20/40 BSS Coexistence Management frame is a Public Action frame*/
		if (check_fwstate(&padapter->mlmepriv, WIFI_AP_STATE))
			rtw_process_public_act_bsscoex(padapter, pframe, frame_len);
#endif /*CONFIG_AP_MODE*/
		break;
	case ACT_PUBLIC_VENDOR:
		ret = on_action_public_vendor(precv_frame);
		break;
	default:
		ret = on_action_public_default(precv_frame, action);
		break;
	}

exit:
	return ret;
}

unsigned int OnAction_ft(_adapter *padapter, union recv_frame *precv_frame)
{
#ifdef CONFIG_RTW_80211R
	u32	ret = _FAIL;
	u32	frame_len = 0;
	u8	action_code = 0;
	u8	category = 0;
	u8	*pframe = NULL;
	u8	*pframe_body = NULL;
	u8	sta_addr[ETH_ALEN] = {0};
	u8	*pie = NULL;
	u32	ft_ie_len = 0;
	u32 status_code = 0;
	struct mlme_ext_priv *pmlmeext = NULL;
	struct mlme_ext_info *pmlmeinfo = NULL;
	struct mlme_priv *pmlmepriv = NULL;
	struct wlan_network *proam_target = NULL;
	ft_priv *pftpriv = NULL;
	unsigned long  irqL;

	pmlmeext = &padapter->mlmeextpriv;
	pmlmeinfo = &(pmlmeext->mlmext_info);
	pmlmepriv = &padapter->mlmepriv;
	pftpriv = &pmlmepriv->ftpriv;
	pframe = precv_frame->u.hdr.rx_data;
	frame_len = precv_frame->u.hdr.len;
	pframe_body = pframe + sizeof(struct rtw_ieee80211_hdr_3addr);
	category = pframe_body[0];

	if (category != RTW_WLAN_CATEGORY_FT)
		goto exit;

	action_code = pframe_body[1];
	switch (action_code) {
	case RTW_WLAN_ACTION_FT_RESPONSE:
		RTW_INFO("FT: %s RTW_WLAN_ACTION_FT_RESPONSE\n", __func__);
		if (memcmp(adapter_mac_addr(padapter), &pframe_body[2], ETH_ALEN)) {
			RTW_ERR("FT: Unmatched STA MAC Address "MAC_FMT"\n", MAC_ARG(&pframe_body[2]));
			goto exit;
		}

		status_code = le16_to_cpu(*(u16 *)((SIZE_PTR)pframe +  sizeof(struct rtw_ieee80211_hdr_3addr) + 14));
		if (status_code != 0) {
			RTW_ERR("FT: WLAN ACTION FT RESPONSE fail, status: %d\n", status_code);
			goto exit;
		}

		if (is_zero_mac_addr(&pframe_body[8]) || is_broadcast_mac_addr(&pframe_body[8])) {
			RTW_ERR("FT: Invalid Target MAC Address "MAC_FMT"\n", MAC_ARG(padapter->mlmepriv.roam_tgt_addr));
			goto exit;
		}

		pie = rtw_get_ie(pframe_body, _MDIE_, &ft_ie_len, frame_len);
		if (pie) {
			if (memcmp(&pftpriv->mdid, pie+2, 2)) {
				RTW_ERR("FT: Invalid MDID\n");
				goto exit;
			}
		}

		rtw_set_ft_status(padapter, RTW_FT_REQUESTED_STA);
		_cancel_timer_ex(&pmlmeext->ft_link_timer);

		/*Disconnect current AP*/
		receive_disconnect(padapter, pmlmepriv->cur_network.network.MacAddress, WLAN_REASON_ACTIVE_ROAM, false);

		pftpriv->ft_action_len = frame_len;
		memcpy(pftpriv->ft_action, pframe, rtw_min(frame_len, RTW_MAX_FTIE_SZ));
		ret = _SUCCESS;
		break;
	case RTW_WLAN_ACTION_FT_REQUEST:
	case RTW_WLAN_ACTION_FT_CONFIRM:
	case RTW_WLAN_ACTION_FT_ACK:
	default:
		RTW_ERR("FT: Unsupported FT Action!\n");
		break;
	}

exit:
	return ret;
#else
	return _SUCCESS;
#endif
}

unsigned int OnAction_ht(_adapter *padapter, union recv_frame *precv_frame)
{
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint frame_len = precv_frame->u.hdr.len;
	u8 *frame_body = pframe + sizeof(struct rtw_ieee80211_hdr_3addr);
	u8 category, action;

	/* check RA matches or not */
	if (memcmp(adapter_mac_addr(padapter), GetAddr1Ptr(pframe), ETH_ALEN))
		goto exit;

	category = frame_body[0];
	if (category != RTW_WLAN_CATEGORY_HT)
		goto exit;

	action = frame_body[1];
	switch (action) {
	case RTW_WLAN_ACTION_HT_SM_PS:
#ifdef CONFIG_AP_MODE
		if (check_fwstate(&padapter->mlmepriv, WIFI_AP_STATE))
			rtw_process_ht_action_smps(padapter, get_addr2_ptr(pframe), frame_body[2]);
#endif /*CONFIG_AP_MODE*/
		break;
	case RTW_WLAN_ACTION_HT_COMPRESS_BEAMFORMING:
#ifdef CONFIG_BEAMFORMING
		/*RTW_INFO("RTW_WLAN_ACTION_HT_COMPRESS_BEAMFORMING\n");*/
		rtw_beamforming_get_report_frame(padapter, precv_frame);
#endif /*CONFIG_BEAMFORMING*/
		break;
	default:
		break;
	}

exit:

	return _SUCCESS;
}

#ifdef CONFIG_IEEE80211W
unsigned int OnAction_sa_query(_adapter *padapter, union recv_frame *precv_frame)
{
	u8 *pframe = precv_frame->u.hdr.rx_data;
	struct rx_pkt_attrib *pattrib = &precv_frame->u.hdr.attrib;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct sta_info		*psta;
	struct sta_priv		*pstapriv = &padapter->stapriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	u16 tid;
	/* Baron */

	RTW_INFO("OnAction_sa_query\n");

	switch (pframe[WLAN_HDR_A3_LEN + 1]) {
	case 0: /* SA Query req */
		memcpy(&tid, &pframe[WLAN_HDR_A3_LEN + 2], sizeof(u16));
		RTW_INFO("OnAction_sa_query request,action=%d, tid=%04x, pframe=%02x-%02x\n"
			, pframe[WLAN_HDR_A3_LEN + 1], tid, pframe[WLAN_HDR_A3_LEN + 2], pframe[WLAN_HDR_A3_LEN + 3]);
		issue_action_SA_Query(padapter, get_addr2_ptr(pframe), 1, tid, IEEE80211W_RIGHT_KEY);
		break;

	case 1: /* SA Query rsp */
		psta = rtw_get_stainfo(pstapriv, get_addr2_ptr(pframe));
		if (psta != NULL)
			_cancel_timer_ex(&psta->dot11w_expire_timer);

		memcpy(&tid, &pframe[WLAN_HDR_A3_LEN + 2], sizeof(u16));
		RTW_INFO("OnAction_sa_query response,action=%d, tid=%04x, cancel timer\n", pframe[WLAN_HDR_A3_LEN + 1], tid);
		break;
	default:
		break;
	}
	if (0) {
		int pp;
		RTW_INFO("pattrib->pktlen = %d =>", pattrib->pkt_len);
		for (pp = 0; pp < pattrib->pkt_len; pp++)
			RTW_INFO(" %02x ", pframe[pp]);
		RTW_INFO("\n");
	}

	return _SUCCESS;
}
#endif /* CONFIG_IEEE80211W */

unsigned int OnAction_wmm(_adapter *padapter, union recv_frame *precv_frame)
{
	return _SUCCESS;
}

unsigned int OnAction_vht(_adapter *padapter, union recv_frame *precv_frame)
{
	return _SUCCESS;
}

unsigned int OnAction_p2p(_adapter *padapter, union recv_frame *precv_frame)
{
#ifdef CONFIG_P2P
	u8 *frame_body;
	u8 category, OUI_Subtype, dialogToken = 0;
	u8 *pframe = precv_frame->u.hdr.rx_data;
	uint len = precv_frame->u.hdr.len;
	struct	wifidirect_info	*pwdinfo = &(padapter->wdinfo);

	/* check RA matches or not */
	if (memcmp(adapter_mac_addr(padapter), GetAddr1Ptr(pframe), ETH_ALEN))
		return _SUCCESS;

	frame_body = (unsigned char *)(pframe + sizeof(struct rtw_ieee80211_hdr_3addr));

	category = frame_body[0];
	if (category != RTW_WLAN_CATEGORY_P2P)
		return _SUCCESS;

	if (be32_to_cpu(*((__be32 *)(frame_body + 1))) != P2POUI)
		return _SUCCESS;

#ifdef CONFIG_IOCTL_CFG80211
	if (adapter_wdev_data(padapter)->p2p_enabled
		&& pwdinfo->driver_interface == DRIVER_CFG80211
	) {
		rtw_cfg80211_rx_action_p2p(padapter, precv_frame);
		return _SUCCESS;
	} else
#endif /* CONFIG_IOCTL_CFG80211 */
	{
		len -= sizeof(struct rtw_ieee80211_hdr_3addr);
		OUI_Subtype = frame_body[5];
		dialogToken = frame_body[6];

		switch (OUI_Subtype) {
		case P2P_NOTICE_OF_ABSENCE:

			break;

		case P2P_PRESENCE_REQUEST:

			process_p2p_presence_req(pwdinfo, pframe, len);

			break;

		case P2P_PRESENCE_RESPONSE:

			break;

		case P2P_GO_DISC_REQUEST:

			break;

		default:
			break;

		}
	}
#endif /* CONFIG_P2P */

	return _SUCCESS;

}

unsigned int OnAction(_adapter *padapter, union recv_frame *precv_frame)
{
	int i;
	unsigned char	category;
	struct action_handler *ptable;
	unsigned char	*frame_body;
	u8 *pframe = precv_frame->u.hdr.rx_data;

	frame_body = (unsigned char *)(pframe + sizeof(struct rtw_ieee80211_hdr_3addr));

	category = frame_body[0];

	for (i = 0; i < sizeof(OnAction_tbl) / sizeof(struct action_handler); i++) {
		ptable = &OnAction_tbl[i];

		if (category == ptable->num)
			ptable->func(padapter, precv_frame);

	}

	return _SUCCESS;

}

unsigned int DoReserved(_adapter *padapter, union recv_frame *precv_frame)
{

	/* RTW_INFO("rcvd mgt frame(%x, %x)\n", (get_frame_sub_type(pframe) >> 4), *(unsigned int *)GetAddr1Ptr(pframe)); */
	return _SUCCESS;
}

static struct xmit_frame *_alloc_mgtxmitframe(struct xmit_priv *pxmitpriv, bool once)
{
	struct xmit_frame *pmgntframe;
	struct xmit_buf *pxmitbuf;

	if (once)
		pmgntframe = rtw_alloc_xmitframe_once(pxmitpriv);
	else
		pmgntframe = rtw_alloc_xmitframe_ext(pxmitpriv);

	if (pmgntframe == NULL) {
		RTW_INFO(FUNC_ADPT_FMT" alloc xmitframe fail, once:%d\n", FUNC_ADPT_ARG(pxmitpriv->adapter), once);
		goto exit;
	}

	pxmitbuf = rtw_alloc_xmitbuf_ext(pxmitpriv);
	if (pxmitbuf == NULL) {
		RTW_INFO(FUNC_ADPT_FMT" alloc xmitbuf fail\n", FUNC_ADPT_ARG(pxmitpriv->adapter));
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		pmgntframe = NULL;
		goto exit;
	}

	pmgntframe->frame_tag = MGNT_FRAMETAG;
	pmgntframe->pxmitbuf = pxmitbuf;
	pmgntframe->buf_addr = pxmitbuf->pbuf;
	pxmitbuf->priv_data = pmgntframe;

exit:
	return pmgntframe;

}

inline struct xmit_frame *alloc_mgtxmitframe(struct xmit_priv *pxmitpriv)
{
	return _alloc_mgtxmitframe(pxmitpriv, false);
}

inline struct xmit_frame *alloc_mgtxmitframe_once(struct xmit_priv *pxmitpriv)
{
	return _alloc_mgtxmitframe(pxmitpriv, true);
}


/****************************************************************************

Following are some TX fuctions for WiFi MLME

*****************************************************************************/

void update_mgnt_tx_rate(_adapter *padapter, u8 rate)
{
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);

	pmlmeext->tx_rate = rate;
	/* RTW_INFO("%s(): rate = %x\n",__func__, rate); */
}


void update_monitor_frame_attrib(_adapter *padapter, struct pkt_attrib *pattrib)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	u8	wireless_mode;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct xmit_priv		*pxmitpriv = &padapter->xmitpriv;
	struct sta_info		*psta = NULL;
	struct sta_priv		*pstapriv = &padapter->stapriv;
	struct sta_info *pbcmc_sta = NULL;

	psta = rtw_get_stainfo(pstapriv, pattrib->ra);
	pbcmc_sta = rtw_get_bcmc_stainfo(padapter);

	pattrib->hdrlen = 24;
	pattrib->nr_frags = 1;
	pattrib->priority = 7;

	if (pbcmc_sta)
		pattrib->mac_id = pbcmc_sta->mac_id;
	else {
		pattrib->mac_id = 0;
		RTW_INFO("mgmt use mac_id 0 will affect RA\n");
	}
	pattrib->qsel = QSLT_MGNT;

	pattrib->pktlen = 0;

	if (pmlmeext->tx_rate == IEEE80211_CCK_RATE_1MB)
		wireless_mode = WIRELESS_11B;
	else
		wireless_mode = WIRELESS_11G;

	pattrib->raid = rtw_get_mgntframe_raid(padapter, wireless_mode);
	pattrib->rate = MGN_MCS7;

	pattrib->encrypt = _NO_PRIVACY_;
	pattrib->bswenc = false;

	pattrib->qos_en = false;
	pattrib->ht_en = 1;
	pattrib->bwmode = CHANNEL_WIDTH_20;
	pattrib->ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
	pattrib->sgi = false;

	pattrib->seqnum = pmlmeext->mgnt_seq;

	pattrib->retry_ctrl = true;

	pattrib->mbssid = 0;
	pattrib->hw_ssn_sel = pxmitpriv->hw_ssn_seq_no;

}


void update_mgntframe_attrib(_adapter *padapter, struct pkt_attrib *pattrib)
{
	u8	wireless_mode;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct xmit_priv		*pxmitpriv = &padapter->xmitpriv;
	struct sta_info *pbcmc_sta = NULL;
	/* memset((u8 *)(pattrib), 0, sizeof(struct pkt_attrib)); */

	pbcmc_sta = rtw_get_bcmc_stainfo(padapter);

	pattrib->hdrlen = 24;
	pattrib->nr_frags = 1;
	pattrib->priority = 7;

	if (pbcmc_sta)
		pattrib->mac_id = pbcmc_sta->mac_id;
	else {
		pattrib->mac_id = 1; /* use STA's BCMC sta-info macid */

		if (MLME_IS_AP(padapter) || MLME_IS_GO(padapter))
			RTW_INFO("%s-"ADPT_FMT" get bcmc sta_info fail,use MACID=1\n", __func__, ADPT_ARG(padapter));
	}
	pattrib->qsel = QSLT_MGNT;

#ifdef CONFIG_MCC_MODE
	update_mcc_mgntframe_attrib(padapter, pattrib);
#endif

	pattrib->pktlen = 0;

	if (IS_CCK_RATE(pmlmeext->tx_rate))
		wireless_mode = WIRELESS_11B;
	else
		wireless_mode = WIRELESS_11G;
	pattrib->raid =  rtw_get_mgntframe_raid(padapter, wireless_mode);
	pattrib->rate = pmlmeext->tx_rate;

	pattrib->encrypt = _NO_PRIVACY_;
	pattrib->bswenc = false;

	pattrib->qos_en = false;
	pattrib->ht_en = false;
	pattrib->bwmode = CHANNEL_WIDTH_20;
	pattrib->ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
	pattrib->sgi = false;

	pattrib->seqnum = pmlmeext->mgnt_seq;

	pattrib->retry_ctrl = true;

	pattrib->mbssid = 0;
	pattrib->hw_ssn_sel = pxmitpriv->hw_ssn_seq_no;
}

void update_mgntframe_attrib_addr(_adapter *padapter, struct xmit_frame *pmgntframe)
{
	u8	*pframe;
	struct pkt_attrib	*pattrib = &pmgntframe->attrib;
#ifdef CONFIG_BEAMFORMING
	struct sta_info		*sta = NULL;
#endif /* CONFIG_BEAMFORMING */

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;

	memcpy(pattrib->ra, GetAddr1Ptr(pframe), ETH_ALEN);
	memcpy(pattrib->ta, get_addr2_ptr(pframe), ETH_ALEN);

#ifdef CONFIG_BEAMFORMING
	sta = pattrib->psta;
	if (!sta) {
		sta = rtw_get_stainfo(&padapter->stapriv, pattrib->ra);
		pattrib->psta = sta;
	}
	if (sta)
		update_attrib_txbf_info(padapter, pattrib, sta);
#endif /* CONFIG_BEAMFORMING */
}

void dump_mgntframe(_adapter *padapter, struct xmit_frame *pmgntframe)
{
	if (RTW_CANNOT_RUN(padapter)) {
		rtw_free_xmitbuf(&padapter->xmitpriv, pmgntframe->pxmitbuf);
		rtw_free_xmitframe(&padapter->xmitpriv, pmgntframe);
		return;
	}

	rtw_hal_mgnt_xmit(padapter, pmgntframe);
}

s32 dump_mgntframe_and_wait(_adapter *padapter, struct xmit_frame *pmgntframe, int timeout_ms)
{
	s32 ret = _FAIL;
	unsigned long irqL;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct xmit_buf *pxmitbuf = pmgntframe->pxmitbuf;
	struct submit_ctx sctx;

	if (RTW_CANNOT_RUN(padapter)) {
		rtw_free_xmitbuf(&padapter->xmitpriv, pmgntframe->pxmitbuf);
		rtw_free_xmitframe(&padapter->xmitpriv, pmgntframe);
		return ret;
	}

	rtw_sctx_init(&sctx, timeout_ms);
	pxmitbuf->sctx = &sctx;

	ret = rtw_hal_mgnt_xmit(padapter, pmgntframe);

	if (ret == _SUCCESS)
		ret = rtw_sctx_wait(&sctx, __func__);

	_enter_critical(&pxmitpriv->lock_sctx, &irqL);
	pxmitbuf->sctx = NULL;
	_exit_critical(&pxmitpriv->lock_sctx, &irqL);

	return ret;
}

s32 dump_mgntframe_and_wait_ack_timeout(_adapter *padapter, struct xmit_frame *pmgntframe, int timeout_ms)
{
#ifdef CONFIG_XMIT_ACK
	static u8 seq_no = 0;
	s32 ret = _FAIL;
	struct xmit_priv	*pxmitpriv = &(GET_PRIMARY_ADAPTER(padapter))->xmitpriv;

	if (RTW_CANNOT_RUN(padapter)) {
		rtw_free_xmitbuf(&padapter->xmitpriv, pmgntframe->pxmitbuf);
		rtw_free_xmitframe(&padapter->xmitpriv, pmgntframe);
		return -1;
	}

	_enter_critical_mutex(&pxmitpriv->ack_tx_mutex, NULL);
	pxmitpriv->ack_tx = true;
	pxmitpriv->seq_no = seq_no++;
	pmgntframe->ack_report = 1;
	rtw_sctx_init(&(pxmitpriv->ack_tx_ops), timeout_ms);
	if (rtw_hal_mgnt_xmit(padapter, pmgntframe) == _SUCCESS)
		ret = rtw_sctx_wait(&(pxmitpriv->ack_tx_ops), __func__);

	pxmitpriv->ack_tx = false;
	_exit_critical_mutex(&pxmitpriv->ack_tx_mutex, NULL);

	return ret;
#else /* !CONFIG_XMIT_ACK */
	dump_mgntframe(padapter, pmgntframe);
	rtw_msleep_os(50);
	return _SUCCESS;
#endif /* !CONFIG_XMIT_ACK */
}

s32 dump_mgntframe_and_wait_ack(_adapter *padapter, struct xmit_frame *pmgntframe)
{
	/* In this case, use 500 ms as the default wait_ack timeout */
	return dump_mgntframe_and_wait_ack_timeout(padapter, pmgntframe, 500);
}


static int update_hidden_ssid(u8 *ies, u32 ies_len, u8 hidden_ssid_mode)
{
	u8 *ssid_ie;
	sint ssid_len_ori;
	int len_diff = 0;

	ssid_ie = rtw_get_ie(ies,  WLAN_EID_SSID, &ssid_len_ori, ies_len);

	/* RTW_INFO("%s hidden_ssid_mode:%u, ssid_ie:%p, ssid_len_ori:%d\n", __func__, hidden_ssid_mode, ssid_ie, ssid_len_ori); */

	if (ssid_ie && ssid_len_ori > 0) {
		switch (hidden_ssid_mode) {
		case 1: {
			u8 *next_ie = ssid_ie + 2 + ssid_len_ori;
			u32 remain_len = 0;

			remain_len = ies_len - (next_ie - ies);

			ssid_ie[1] = 0;
			memcpy(ssid_ie + 2, next_ie, remain_len);
			len_diff -= ssid_len_ori;

			break;
		}
		case 2:
			memset(&ssid_ie[2], 0, ssid_len_ori);
			break;
		default:
			break;
		}
	}

	return len_diff;
}

#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
u32 rtw_build_vendor_ie(_adapter *padapter , unsigned char *pframe , u8 mgmt_frame_tyte)
{
	int vendor_ie_num = 0;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	u32 len = 0;

	for (vendor_ie_num = 0 ; vendor_ie_num < WLAN_MAX_VENDOR_IE_NUM ; vendor_ie_num++) {
		if (pmlmepriv->vendor_ielen[vendor_ie_num] > 0 && pmlmepriv->vendor_ie_mask[vendor_ie_num] & mgmt_frame_tyte) {
			memcpy(pframe , pmlmepriv->vendor_ie[vendor_ie_num] , pmlmepriv->vendor_ielen[vendor_ie_num]);
			pframe +=  pmlmepriv->vendor_ielen[vendor_ie_num];
			len += pmlmepriv->vendor_ielen[vendor_ie_num];
		}
	}

	return len;
}
#endif

void issue_beacon(_adapter *padapter, int timeout_ms)
{
	struct xmit_frame	*pmgntframe;
	struct pkt_attrib	*pattrib;
	unsigned char	*pframe;
	struct rtw_ieee80211_hdr *pwlanhdr;
	__le16 *fctrl;
	unsigned int	rate_len;
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);
#if defined(CONFIG_AP_MODE) && defined (CONFIG_NATIVEAP_MLME)
	unsigned long irqL;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
#endif /* #if defined (CONFIG_AP_MODE) && defined (CONFIG_NATIVEAP_MLME) */
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*cur_network = &(pmlmeinfo->network);
	u8	bc_addr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
#endif /* CONFIG_P2P */


	/* RTW_INFO("%s\n", __func__); */

#ifdef CONFIG_BCN_ICF
	pmgntframe = rtw_alloc_bcnxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
#else
	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
#endif
	{
		RTW_INFO("%s, alloc mgnt frame fail\n", __func__);
		return;
	}
#if defined(CONFIG_AP_MODE) && defined (CONFIG_NATIVEAP_MLME)
	_enter_critical_bh(&pmlmepriv->bcn_update_lock, &irqL);
#endif /* #if defined (CONFIG_AP_MODE) && defined (CONFIG_NATIVEAP_MLME) */

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);
	pattrib->qsel = QSLT_BEACON;

#if defined(CONFIG_CONCURRENT_MODE) && (!defined(CONFIG_SWTIMER_BASED_TXBCN))
	if (padapter->hw_port == HW_PORT1)
		pattrib->mbssid = 1;
#endif

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;


	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	memcpy(pwlanhdr->addr1, bc_addr, ETH_ALEN);
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	memcpy(pwlanhdr->addr3, get_my_bssid(cur_network), ETH_ALEN);

	SetSeqNum(pwlanhdr, 0/*pmlmeext->mgnt_seq*/);
	/* pmlmeext->mgnt_seq++; */
	set_frame_sub_type(pframe, WIFI_BEACON);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	if ((pmlmeinfo->state & 0x03) == WIFI_FW_AP_STATE) {
		/* RTW_INFO("ie len=%d\n", cur_network->IELength); */
#ifdef CONFIG_P2P
		/* for P2P : Primary Device Type & Device Name */
		u32 wpsielen = 0, insert_len = 0;
		u8 *wpsie = NULL;
		wpsie = rtw_get_wps_ie(cur_network->IEs + _FIXED_IE_LENGTH_, cur_network->IELength - _FIXED_IE_LENGTH_, NULL, &wpsielen);

		if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO) && wpsie && wpsielen > 0) {
			uint wps_offset, remainder_ielen;
			u8 *premainder_ie, *pframe_wscie;

			wps_offset = (uint)(wpsie - cur_network->IEs);

			premainder_ie = wpsie + wpsielen;

			remainder_ielen = cur_network->IELength - wps_offset - wpsielen;

#ifdef CONFIG_IOCTL_CFG80211
			if (adapter_wdev_data(padapter)->p2p_enabled && pwdinfo->driver_interface == DRIVER_CFG80211) {
				if (pmlmepriv->wps_beacon_ie && pmlmepriv->wps_beacon_ie_len > 0) {
					memcpy(pframe, cur_network->IEs, wps_offset);
					pframe += wps_offset;
					pattrib->pktlen += wps_offset;

					memcpy(pframe, pmlmepriv->wps_beacon_ie, pmlmepriv->wps_beacon_ie_len);
					pframe += pmlmepriv->wps_beacon_ie_len;
					pattrib->pktlen += pmlmepriv->wps_beacon_ie_len;

					/* copy remainder_ie to pframe */
					memcpy(pframe, premainder_ie, remainder_ielen);
					pframe += remainder_ielen;
					pattrib->pktlen += remainder_ielen;
				} else {
					memcpy(pframe, cur_network->IEs, cur_network->IELength);
					pframe += cur_network->IELength;
					pattrib->pktlen += cur_network->IELength;
				}
			} else
#endif /* CONFIG_IOCTL_CFG80211 */
			{
				pframe_wscie = pframe + wps_offset;
				memcpy(pframe, cur_network->IEs, wps_offset + wpsielen);
				pframe += (wps_offset + wpsielen);
				pattrib->pktlen += (wps_offset + wpsielen);

				/* now pframe is end of wsc ie, insert Primary Device Type & Device Name */
				/*	Primary Device Type */
				/*	Type: */
				*(__be16 *)(pframe + insert_len) = cpu_to_be16(WPS_ATTR_PRIMARY_DEV_TYPE);
				insert_len += 2;

				/*	Length: */
				*(__be16 *)(pframe + insert_len) = cpu_to_be16(0x0008);
				insert_len += 2;

				/*	Value: */
				/*	Category ID */
				*(__be16 *)(pframe + insert_len) = cpu_to_be16(WPS_PDT_CID_MULIT_MEDIA);
				insert_len += 2;

				/*	OUI */
				*(__be32 *)(pframe + insert_len) = cpu_to_be32(WPSOUI);
				insert_len += 4;

				/*	Sub Category ID */
				*(__be16 *)(pframe + insert_len) = cpu_to_be16(WPS_PDT_SCID_MEDIA_SERVER);
				insert_len += 2;


				/*	Device Name */
				/*	Type: */
				*(__be16 *)(pframe + insert_len) = cpu_to_be16(WPS_ATTR_DEVICE_NAME);
				insert_len += 2;

				/*	Length: */
				*(__be16 *)(pframe + insert_len) = cpu_to_be16(pwdinfo->device_name_len);
				insert_len += 2;

				/*	Value: */
				memcpy(pframe + insert_len, pwdinfo->device_name, pwdinfo->device_name_len);
				insert_len += pwdinfo->device_name_len;


				/* update wsc ie length */
				*(pframe_wscie + 1) = (wpsielen - 2) + insert_len;

				/* pframe move to end */
				pframe += insert_len;
				pattrib->pktlen += insert_len;

				/* copy remainder_ie to pframe */
				memcpy(pframe, premainder_ie, remainder_ielen);
				pframe += remainder_ielen;
				pattrib->pktlen += remainder_ielen;
			}
		} else
#endif /* CONFIG_P2P */
		{
			int len_diff;
			memcpy(pframe, cur_network->IEs, cur_network->IELength);
			len_diff = update_hidden_ssid(
					   pframe + _BEACON_IE_OFFSET_
				   , cur_network->IELength - _BEACON_IE_OFFSET_
					   , pmlmeinfo->hidden_ssid_mode
				   );
			pframe += (cur_network->IELength + len_diff);
			pattrib->pktlen += (cur_network->IELength + len_diff);
		}

		{
			u8 *wps_ie;
			uint wps_ielen;
			u8 sr = 0;
			wps_ie = rtw_get_wps_ie(pmgntframe->buf_addr + TXDESC_OFFSET + sizeof(struct rtw_ieee80211_hdr_3addr) + _BEACON_IE_OFFSET_,
				pattrib->pktlen - sizeof(struct rtw_ieee80211_hdr_3addr) - _BEACON_IE_OFFSET_, NULL, &wps_ielen);
			if (wps_ie && wps_ielen > 0)
				rtw_get_wps_attr_content(wps_ie,  wps_ielen, WPS_ATTR_SELECTED_REGISTRAR, (u8 *)(&sr), NULL);
			if (sr != 0)
				set_fwstate(pmlmepriv, WIFI_UNDER_WPS);
			else
				_clr_fwstate_(pmlmepriv, WIFI_UNDER_WPS);
		}

#ifdef CONFIG_P2P
		if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO)) {
			u32 len;
#ifdef CONFIG_IOCTL_CFG80211
			if (adapter_wdev_data(padapter)->p2p_enabled && pwdinfo->driver_interface == DRIVER_CFG80211) {
				len = pmlmepriv->p2p_beacon_ie_len;
				if (pmlmepriv->p2p_beacon_ie && len > 0)
					memcpy(pframe, pmlmepriv->p2p_beacon_ie, len);
			} else
#endif /* CONFIG_IOCTL_CFG80211 */
			{
				len = build_beacon_p2p_ie(pwdinfo, pframe);
			}

			pframe += len;
			pattrib->pktlen += len;

#ifdef CONFIG_MCC_MODE
			pframe = rtw_hal_mcc_append_go_p2p_ie(padapter, pframe, &pattrib->pktlen);
#endif /* CONFIG_MCC_MODE*/

#ifdef CONFIG_WFD
			len = rtw_append_beacon_wfd_ie(padapter, pframe);
			pframe += len;
			pattrib->pktlen += len;
#endif
		}
#endif /* CONFIG_P2P */

#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
		pattrib->pktlen += rtw_build_vendor_ie(padapter , pframe , WIFI_BEACON_VENDOR_IE_BIT);
#endif
		goto _issue_bcn;

	}

	/* below for ad-hoc mode */

	/* timestamp will be inserted by hardware */
	pframe += 8;
	pattrib->pktlen += 8;

	/* beacon interval: 2 bytes */

	memcpy(pframe, (unsigned char *)(rtw_get_beacon_interval_from_ie(cur_network->IEs)), 2);

	pframe += 2;
	pattrib->pktlen += 2;

	/* capability info: 2 bytes */

	memcpy(pframe, (unsigned char *)(rtw_get_capability_from_ie(cur_network->IEs)), 2);

	pframe += 2;
	pattrib->pktlen += 2;

	/* SSID */
	pframe = rtw_set_ie(pframe, _SSID_IE_, cur_network->Ssid.SsidLength, cur_network->Ssid.Ssid, &pattrib->pktlen);

	/* supported rates... */
	rate_len = rtw_get_rateset_len(cur_network->SupportedRates);
	pframe = rtw_set_ie(pframe, _SUPPORTEDRATES_IE_, ((rate_len > 8) ? 8 : rate_len), cur_network->SupportedRates, &pattrib->pktlen);

	/* DS parameter set */
	pframe = rtw_set_ie(pframe, _DSSET_IE_, 1, (unsigned char *)&(cur_network->Configuration.DSConfig), &pattrib->pktlen);

	/* if( (pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE) */
	{
		u8 erpinfo = 0;
		u32 ATIMWindow;
		/* IBSS Parameter Set... */
		/* ATIMWindow = cur->Configuration.ATIMWindow; */
		ATIMWindow = 0;
		pframe = rtw_set_ie(pframe, _IBSS_PARA_IE_, 2, (unsigned char *)(&ATIMWindow), &pattrib->pktlen);

		/* ERP IE */
		pframe = rtw_set_ie(pframe, _ERPINFO_IE_, 1, &erpinfo, &pattrib->pktlen);
	}


	/* EXTERNDED SUPPORTED RATE */
	if (rate_len > 8)
		pframe = rtw_set_ie(pframe, _EXT_SUPPORTEDRATES_IE_, (rate_len - 8), (cur_network->SupportedRates + 8), &pattrib->pktlen);


	/* todo:HT for adhoc */

_issue_bcn:

#if defined(CONFIG_AP_MODE) && defined (CONFIG_NATIVEAP_MLME)
	pmlmepriv->update_bcn = false;

	_exit_critical_bh(&pmlmepriv->bcn_update_lock, &irqL);
#endif /* #if defined (CONFIG_AP_MODE) && defined (CONFIG_NATIVEAP_MLME) */

	if ((pattrib->pktlen + TXDESC_SIZE) > 512) {
		RTW_INFO("beacon frame too large\n");
		return;
	}

	pattrib->last_txcmdsz = pattrib->pktlen;

	/* RTW_INFO("issue bcn_sz=%d\n", pattrib->last_txcmdsz); */
	if (timeout_ms > 0)
		dump_mgntframe_and_wait(padapter, pmgntframe, timeout_ms);
	else
		dump_mgntframe(padapter, pmgntframe);

}

void issue_probersp(_adapter *padapter, unsigned char *da, u8 is_valid_p2p_probereq)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	unsigned char					*mac, *bssid;
	struct xmit_priv	*pxmitpriv = &(padapter->xmitpriv);
#if defined(CONFIG_AP_MODE) && defined (CONFIG_NATIVEAP_MLME)
	u8 *pwps_ie;
	uint wps_ielen;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
#endif /* #if defined (CONFIG_AP_MODE) && defined (CONFIG_NATIVEAP_MLME) */
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*cur_network = &(pmlmeinfo->network);
	unsigned int	rate_len;
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
#ifdef CONFIG_WFD
	u32					wfdielen = 0;
#endif
#endif /* CONFIG_P2P */

	/* RTW_INFO("%s\n", __func__); */

	if (da == NULL)
		return;

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		return;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL) {
		RTW_INFO("%s, alloc mgnt frame fail\n", __func__);
		return;
	}


	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	mac = adapter_mac_addr(padapter);
	bssid = cur_network->MacAddress;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;
	memcpy(pwlanhdr->addr1, da, ETH_ALEN);
	memcpy(pwlanhdr->addr2, mac, ETH_ALEN);
	memcpy(pwlanhdr->addr3, bssid, ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(fctrl, WIFI_PROBERSP);

	pattrib->hdrlen = sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = pattrib->hdrlen;
	pframe += pattrib->hdrlen;


	if (cur_network->IELength > MAX_IE_SZ)
		return;

#if defined(CONFIG_AP_MODE) && defined (CONFIG_NATIVEAP_MLME)
	if ((pmlmeinfo->state & 0x03) == WIFI_FW_AP_STATE) {
		pwps_ie = rtw_get_wps_ie(cur_network->IEs + _FIXED_IE_LENGTH_, cur_network->IELength - _FIXED_IE_LENGTH_, NULL, &wps_ielen);

		/* inerset & update wps_probe_resp_ie */
		if ((pmlmepriv->wps_probe_resp_ie != NULL) && pwps_ie && (wps_ielen > 0)) {
			uint wps_offset, remainder_ielen;
			u8 *premainder_ie;

			wps_offset = (uint)(pwps_ie - cur_network->IEs);

			premainder_ie = pwps_ie + wps_ielen;

			remainder_ielen = cur_network->IELength - wps_offset - wps_ielen;

			memcpy(pframe, cur_network->IEs, wps_offset);
			pframe += wps_offset;
			pattrib->pktlen += wps_offset;

			wps_ielen = (uint)pmlmepriv->wps_probe_resp_ie[1];/* to get ie data len */
			if ((wps_offset + wps_ielen + 2) <= MAX_IE_SZ) {
				memcpy(pframe, pmlmepriv->wps_probe_resp_ie, wps_ielen + 2);
				pframe += wps_ielen + 2;
				pattrib->pktlen += wps_ielen + 2;
			}

			if ((wps_offset + wps_ielen + 2 + remainder_ielen) <= MAX_IE_SZ) {
				memcpy(pframe, premainder_ie, remainder_ielen);
				pframe += remainder_ielen;
				pattrib->pktlen += remainder_ielen;
			}
		} else {
			memcpy(pframe, cur_network->IEs, cur_network->IELength);
			pframe += cur_network->IELength;
			pattrib->pktlen += cur_network->IELength;
		}

		/* retrieve SSID IE from cur_network->Ssid */
		{
			u8 *ssid_ie;
			sint ssid_ielen;
			sint ssid_ielen_diff;
			u8 buf[MAX_IE_SZ];
			u8 *ies = pmgntframe->buf_addr + TXDESC_OFFSET + sizeof(struct rtw_ieee80211_hdr_3addr);

			ssid_ie = rtw_get_ie(ies + _FIXED_IE_LENGTH_, _SSID_IE_, &ssid_ielen,
				     (pframe - ies) - _FIXED_IE_LENGTH_);

			ssid_ielen_diff = cur_network->Ssid.SsidLength - ssid_ielen;

			if (ssid_ie &&  cur_network->Ssid.SsidLength) {
				uint remainder_ielen;
				u8 *remainder_ie;
				remainder_ie = ssid_ie + 2;
				remainder_ielen = (pframe - remainder_ie);

				if (remainder_ielen > MAX_IE_SZ) {
					RTW_WARN(FUNC_ADPT_FMT" remainder_ielen > MAX_IE_SZ\n", FUNC_ADPT_ARG(padapter));
					remainder_ielen = MAX_IE_SZ;
				}

				memcpy(buf, remainder_ie, remainder_ielen);
				memcpy(remainder_ie + ssid_ielen_diff, buf, remainder_ielen);
				*(ssid_ie + 1) = cur_network->Ssid.SsidLength;
				memcpy(ssid_ie + 2, cur_network->Ssid.Ssid, cur_network->Ssid.SsidLength);

				pframe += ssid_ielen_diff;
				pattrib->pktlen += ssid_ielen_diff;
			}
		}
#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
		pattrib->pktlen += rtw_build_vendor_ie(padapter , pframe , WIFI_PROBERESP_VENDOR_IE_BIT);
#endif
	} else
#endif
	{

		/* timestamp will be inserted by hardware */
		pframe += 8;
		pattrib->pktlen += 8;

		/* beacon interval: 2 bytes */

		memcpy(pframe, (unsigned char *)(rtw_get_beacon_interval_from_ie(cur_network->IEs)), 2);

		pframe += 2;
		pattrib->pktlen += 2;

		/* capability info: 2 bytes */

		memcpy(pframe, (unsigned char *)(rtw_get_capability_from_ie(cur_network->IEs)), 2);

		pframe += 2;
		pattrib->pktlen += 2;

		/* below for ad-hoc mode */

		/* SSID */
		pframe = rtw_set_ie(pframe, _SSID_IE_, cur_network->Ssid.SsidLength, cur_network->Ssid.Ssid, &pattrib->pktlen);

		/* supported rates... */
		rate_len = rtw_get_rateset_len(cur_network->SupportedRates);
		pframe = rtw_set_ie(pframe, _SUPPORTEDRATES_IE_, ((rate_len > 8) ? 8 : rate_len), cur_network->SupportedRates, &pattrib->pktlen);

		/* DS parameter set */
		pframe = rtw_set_ie(pframe, _DSSET_IE_, 1, (unsigned char *)&(cur_network->Configuration.DSConfig), &pattrib->pktlen);

		if ((pmlmeinfo->state & 0x03) == WIFI_FW_ADHOC_STATE) {
			u8 erpinfo = 0;
			u32 ATIMWindow;
			/* IBSS Parameter Set... */
			/* ATIMWindow = cur->Configuration.ATIMWindow; */
			ATIMWindow = 0;
			pframe = rtw_set_ie(pframe, _IBSS_PARA_IE_, 2, (unsigned char *)(&ATIMWindow), &pattrib->pktlen);

			/* ERP IE */
			pframe = rtw_set_ie(pframe, _ERPINFO_IE_, 1, &erpinfo, &pattrib->pktlen);
		}


		/* EXTERNDED SUPPORTED RATE */
		if (rate_len > 8)
			pframe = rtw_set_ie(pframe, _EXT_SUPPORTEDRATES_IE_, (rate_len - 8), (cur_network->SupportedRates + 8), &pattrib->pktlen);


		/* todo:HT for adhoc */

	}

#ifdef CONFIG_P2P
	if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO)
	    /* IOT issue, When wifi_spec is not set, send probe_resp with P2P IE even if probe_req has no P2P IE */
	    && (is_valid_p2p_probereq || !padapter->registrypriv.wifi_spec)) {
		u32 len;
#ifdef CONFIG_IOCTL_CFG80211
		if (adapter_wdev_data(padapter)->p2p_enabled && pwdinfo->driver_interface == DRIVER_CFG80211) {
			/* if pwdinfo->role == P2P_ROLE_DEVICE will call issue_probersp_p2p() */
			len = pmlmepriv->p2p_go_probe_resp_ie_len;
			if (pmlmepriv->p2p_go_probe_resp_ie && len > 0)
				memcpy(pframe, pmlmepriv->p2p_go_probe_resp_ie, len);
		} else
#endif /* CONFIG_IOCTL_CFG80211 */
		{
			len = build_probe_resp_p2p_ie(pwdinfo, pframe);
		}

		pframe += len;
		pattrib->pktlen += len;

#ifdef CONFIG_MCC_MODE
		pframe = rtw_hal_mcc_append_go_p2p_ie(padapter, pframe, &pattrib->pktlen);
#endif /* CONFIG_MCC_MODE*/

#ifdef CONFIG_WFD
		len = rtw_append_probe_resp_wfd_ie(padapter, pframe);
		pframe += len;
		pattrib->pktlen += len;
#endif
	}
#endif /* CONFIG_P2P */


#ifdef CONFIG_AUTO_AP_MODE
	{
		struct sta_info	*psta;
		struct sta_priv *pstapriv = &padapter->stapriv;

		RTW_INFO("(%s)\n", __func__);

		/* check rc station */
		psta = rtw_get_stainfo(pstapriv, da);
		if (psta && psta->isrc && psta->pid > 0) {
			u8 RC_OUI[4] = {0x00, 0xE0, 0x4C, 0x0A};
			u8 RC_INFO[14] = {0};
			/* EID[1] + EID_LEN[1] + RC_OUI[4] + MAC[6] + PairingID[2] + ChannelNum[2] */
			u16 cu_ch = (u16)cur_network->Configuration.DSConfig;

			RTW_INFO("%s, reply rc(pid=0x%x) device "MAC_FMT" in ch=%d\n", __func__,
				 psta->pid, MAC_ARG(psta->hwaddr), cu_ch);

			/* append vendor specific ie */
			memcpy(RC_INFO, RC_OUI, sizeof(RC_OUI));
			memcpy(&RC_INFO[4], mac, ETH_ALEN);
			memcpy(&RC_INFO[10], (u8 *)&psta->pid, 2);
			memcpy(&RC_INFO[12], (u8 *)&cu_ch, 2);

			pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, sizeof(RC_INFO), RC_INFO, &pattrib->pktlen);
		}
	}
#endif /* CONFIG_AUTO_AP_MODE */


	pattrib->last_txcmdsz = pattrib->pktlen;


	dump_mgntframe(padapter, pmgntframe);

	return;

}

static int _issue_probereq(_adapter *padapter, NDIS_802_11_SSID *pssid, u8 *da, u8 ch, bool append_wps, int wait_ack)
{
	int ret = _FAIL;
	struct xmit_frame		*pmgntframe;
	struct pkt_attrib		*pattrib;
	unsigned char			*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	unsigned char			*mac;
	unsigned char			bssrate[NumRates];
	struct xmit_priv		*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	int	bssrate_len = 0;
	u8	bc_addr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		goto exit;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);


	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	mac = adapter_mac_addr(padapter);

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	if (da) {
		/*	unicast probe request frame */
		memcpy(pwlanhdr->addr1, da, ETH_ALEN);
		memcpy(pwlanhdr->addr3, da, ETH_ALEN);
	} else {
		/*	broadcast probe request frame */
		memcpy(pwlanhdr->addr1, bc_addr, ETH_ALEN);
		memcpy(pwlanhdr->addr3, bc_addr, ETH_ALEN);
	}

	memcpy(pwlanhdr->addr2, mac, ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_PROBEREQ);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	if (pssid)
		pframe = rtw_set_ie(pframe, _SSID_IE_, pssid->SsidLength, pssid->Ssid, &(pattrib->pktlen));
	else
		pframe = rtw_set_ie(pframe, _SSID_IE_, 0, NULL, &(pattrib->pktlen));

	get_rate_set(padapter, bssrate, &bssrate_len);

	if (bssrate_len > 8) {
		pframe = rtw_set_ie(pframe, _SUPPORTEDRATES_IE_ , 8, bssrate, &(pattrib->pktlen));
		pframe = rtw_set_ie(pframe, _EXT_SUPPORTEDRATES_IE_ , (bssrate_len - 8), (bssrate + 8), &(pattrib->pktlen));
	} else
		pframe = rtw_set_ie(pframe, _SUPPORTEDRATES_IE_ , bssrate_len , bssrate, &(pattrib->pktlen));

	if (ch)
		pframe = rtw_set_ie(pframe, _DSSET_IE_, 1, &ch, &pattrib->pktlen);

	if (append_wps) {
		/* add wps_ie for wps2.0 */
		if (pmlmepriv->wps_probe_req_ie_len > 0 && pmlmepriv->wps_probe_req_ie) {
			memcpy(pframe, pmlmepriv->wps_probe_req_ie, pmlmepriv->wps_probe_req_ie_len);
			pframe += pmlmepriv->wps_probe_req_ie_len;
			pattrib->pktlen += pmlmepriv->wps_probe_req_ie_len;
			/* pmlmepriv->wps_probe_req_ie_len = 0 ; */ /* reset to zero */
		}
	}
#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
	pattrib->pktlen += rtw_build_vendor_ie(padapter , pframe , WIFI_PROBEREQ_VENDOR_IE_BIT);
#endif

	pattrib->last_txcmdsz = pattrib->pktlen;


	if (wait_ack)
		ret = dump_mgntframe_and_wait_ack(padapter, pmgntframe);
	else {
		dump_mgntframe(padapter, pmgntframe);
		ret = _SUCCESS;
	}

exit:
	return ret;
}

inline void issue_probereq(_adapter *padapter, NDIS_802_11_SSID *pssid, u8 *da)
{
	_issue_probereq(padapter, pssid, da, 0, 1, false);
}

/*
 * wait_ms == 0 means that there is no need to wait ack through C2H_CCX_TX_RPT
 * wait_ms > 0 means you want to wait ack through C2H_CCX_TX_RPT, and the value of wait_ms means the interval between each TX
 * try_cnt means the maximal TX count to try
 */
int issue_probereq_ex(_adapter *padapter, NDIS_802_11_SSID *pssid, u8 *da, u8 ch, bool append_wps,
		      int try_cnt, int wait_ms)
{
	int ret = _FAIL;
	int i = 0;
	u32 start = jiffies;

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		goto exit;

	do {
		ret = _issue_probereq(padapter, pssid, da, ch, append_wps, wait_ms > 0 ? true : false);

		i++;

		if (RTW_CANNOT_RUN(padapter))
			break;

		if (i < try_cnt && wait_ms > 0 && ret == _FAIL)
			rtw_msleep_os(wait_ms);

	} while ((i < try_cnt) && ((ret == _FAIL) || (wait_ms == 0)));

	if (ret != _FAIL) {
		ret = _SUCCESS;
#ifndef DBG_XMIT_ACK
		goto exit;
#endif
	}

	if (try_cnt && wait_ms) {
		if (da)
			RTW_INFO(FUNC_ADPT_FMT" to "MAC_FMT", ch:%u%s, %d/%d in %u ms\n",
				FUNC_ADPT_ARG(padapter), MAC_ARG(da), rtw_get_oper_ch(padapter),
				ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
		else
			RTW_INFO(FUNC_ADPT_FMT", ch:%u%s, %d/%d in %u ms\n",
				FUNC_ADPT_ARG(padapter), rtw_get_oper_ch(padapter),
				ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
	}
exit:
	return ret;
}

/* if psta == NULL, indiate we are station(client) now... */
void issue_auth(_adapter *padapter, struct sta_info *psta, unsigned short status)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	unsigned int					val32;
	unsigned short				val16;
	int use_shared_key = 0;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	__le16 le_val16;
#ifdef CONFIG_RTW_80211R
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	ft_priv			*pftpriv = &pmlmepriv->ftpriv;
	u8	is_ft_roaming = false;
	u8	is_ft_roaming_with_rsn_ie = true;
	u8 *pie = NULL;
	u32 ft_ie_len = 0;
#endif

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		return;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_AUTH);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);


	if (psta) { /* for AP mode */
#ifdef CONFIG_NATIVEAP_MLME
		memcpy(pwlanhdr->addr1, psta->hwaddr, ETH_ALEN);
		memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
		memcpy(pwlanhdr->addr3, adapter_mac_addr(padapter), ETH_ALEN);


		/* setting auth algo number */
		val16 = (u16)psta->authalg;

		if (val16)	{
			le_val16 = cpu_to_le16(val16);
			use_shared_key = 1;
		} else {
			le_val16 = 0;
		}

		pframe = rtw_set_fixed_ie(pframe, _AUTH_ALGM_NUM_, (unsigned char *)&le_val16, &(pattrib->pktlen));

		/* setting auth seq number */
		val16 = (u16)psta->auth_seq;
		le_val16 = cpu_to_le16(val16);
		pframe = rtw_set_fixed_ie(pframe, _AUTH_SEQ_NUM_, (unsigned char *)&le_val16, &(pattrib->pktlen));

		/* setting status code... */
		val16 = status;
		le_val16 = cpu_to_le16(val16);
		pframe = rtw_set_fixed_ie(pframe, _STATUS_CODE_, (unsigned char *)&le_val16, &(pattrib->pktlen));

		/* added challenging text... */
		if ((psta->auth_seq == 2) && (psta->state & WIFI_FW_AUTH_STATE) && (use_shared_key == 1))
			pframe = rtw_set_ie(pframe, _CHLGETXT_IE_, 128, psta->chg_txt, &(pattrib->pktlen));
#endif
	} else {
		memcpy(pwlanhdr->addr1, get_my_bssid(&pmlmeinfo->network), ETH_ALEN);
		memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
		memcpy(pwlanhdr->addr3, get_my_bssid(&pmlmeinfo->network), ETH_ALEN);

#ifdef CONFIG_RTW_80211R
		/*For Fast BSS Transition */
		if ((rtw_to_roam(padapter) > 0) && rtw_chk_ft_flags(padapter, RTW_FT_SUPPORTED)) {
			is_ft_roaming = true;
			val16 = 2;	/* 2: 802.11R FTAA */
			le_val16 = cpu_to_le16(val16);
		} else
#endif
		{
			/* setting auth algo number */
			val16 = (pmlmeinfo->auth_algo == dot11AuthAlgrthm_Shared) ? 1 : 0;	/* 0:OPEN System, 1:Shared key */
			if (val16) {
				le_val16 = cpu_to_le16(val16);
				use_shared_key = 1;
			}
		}

		/* RTW_INFO("%s auth_algo= %s auth_seq=%d\n",__func__,(pmlmeinfo->auth_algo==0)?"OPEN":"SHARED",pmlmeinfo->auth_seq); */

		/* setting IV for auth seq #3 */
		if ((pmlmeinfo->auth_seq == 3) && (pmlmeinfo->state & WIFI_FW_AUTH_STATE) && (use_shared_key == 1)) {
			__le32 le_val32;

			/* RTW_INFO("==> iv(%d),key_index(%d)\n",pmlmeinfo->iv,pmlmeinfo->key_index); */
			val32 = ((pmlmeinfo->iv++) | (pmlmeinfo->key_index << 30));
			le_val32 = cpu_to_le32(val32);
			pframe = rtw_set_fixed_ie(pframe, 4, (unsigned char *)&le_val32, &(pattrib->pktlen));

			pattrib->iv_len = 4;
		}

		pframe = rtw_set_fixed_ie(pframe, _AUTH_ALGM_NUM_, (unsigned char *)&val16, &(pattrib->pktlen));

		/* setting auth seq number */
		val16 = pmlmeinfo->auth_seq;
		le_val16 = cpu_to_le16(val16);
		pframe = rtw_set_fixed_ie(pframe, _AUTH_SEQ_NUM_, (unsigned char *)&le_val16, &(pattrib->pktlen));


		/* setting status code... */
		val16 = status;
		le_val16 = cpu_to_le16(val16);
		pframe = rtw_set_fixed_ie(pframe, _STATUS_CODE_, (unsigned char *)&le_val16, &(pattrib->pktlen));

#ifdef CONFIG_RTW_80211R
		if (is_ft_roaming) {
			pie = rtw_get_ie(pftpriv->updated_ft_ies, EID_WPA2, &ft_ie_len, pftpriv->updated_ft_ies_len);
			if (pie)
				pframe = rtw_set_ie(pframe, EID_WPA2, ft_ie_len, pie+2, &(pattrib->pktlen));
			else
				is_ft_roaming_with_rsn_ie = false;

			pie = rtw_get_ie(pftpriv->updated_ft_ies, _MDIE_, &ft_ie_len, pftpriv->updated_ft_ies_len);
			if (pie)
				pframe = rtw_set_ie(pframe, _MDIE_, ft_ie_len , pie+2, &(pattrib->pktlen));

			pie = rtw_get_ie(pftpriv->updated_ft_ies, _FTIE_, &ft_ie_len, pftpriv->updated_ft_ies_len);
			if (pie && is_ft_roaming_with_rsn_ie)
				pframe = rtw_set_ie(pframe, _FTIE_, ft_ie_len , pie+2, &(pattrib->pktlen));
		}
#endif

		/* then checking to see if sending challenging text... */
		if ((pmlmeinfo->auth_seq == 3) && (pmlmeinfo->state & WIFI_FW_AUTH_STATE) && (use_shared_key == 1)) {
			pframe = rtw_set_ie(pframe, _CHLGETXT_IE_, 128, pmlmeinfo->chg_txt, &(pattrib->pktlen));

			SetPrivacy(fctrl);

			pattrib->hdrlen = sizeof(struct rtw_ieee80211_hdr_3addr);

			pattrib->encrypt = _WEP40_;

			pattrib->icv_len = 4;

			pattrib->pktlen += pattrib->icv_len;

		}

	}

	pattrib->last_txcmdsz = pattrib->pktlen;

	rtw_wep_encrypt(padapter, (u8 *)pmgntframe);
	RTW_INFO("%s\n", __func__);
	dump_mgntframe(padapter, pmgntframe);

	return;
}


void issue_asocrsp(_adapter *padapter, unsigned short status, struct sta_info *pstat, int pkt_type)
{
#ifdef CONFIG_AP_MODE
	struct xmit_frame	*pmgntframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	struct pkt_attrib *pattrib;
	unsigned char	*pbuf, *pframe;
	unsigned short val;
	__le16 *fctrl, le_val;
	struct xmit_priv *pxmitpriv = &(padapter->xmitpriv);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX *pnetwork = &(pmlmeinfo->network);
	u8 *ie = pnetwork->IEs;
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
#ifdef CONFIG_WFD
	u32					wfdielen = 0;
#endif

#endif /* CONFIG_P2P */

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		return;

	RTW_INFO("%s\n", __func__);

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);


	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	memcpy((void *)GetAddr1Ptr(pwlanhdr), pstat->hwaddr, ETH_ALEN);
	memcpy((void *)get_addr2_ptr(pwlanhdr), adapter_mac_addr(padapter), ETH_ALEN);
	memcpy((void *)GetAddr3Ptr(pwlanhdr), get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);


	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	if ((pkt_type == WIFI_ASSOCRSP) || (pkt_type == WIFI_REASSOCRSP))
		set_frame_sub_type(pwlanhdr, pkt_type);
	else
		return;

	pattrib->hdrlen = sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen += pattrib->hdrlen;
	pframe += pattrib->hdrlen;

	/* capability */
	val = *(unsigned short *)rtw_get_capability_from_ie(ie);

	pframe = rtw_set_fixed_ie(pframe, _CAPABILITY_ , (unsigned char *)&val, &(pattrib->pktlen));

	le_val = cpu_to_le16(status);
	pframe = rtw_set_fixed_ie(pframe , _STATUS_CODE_ , (unsigned char *)&le_val, &(pattrib->pktlen));

	le_val = cpu_to_le16(pstat->aid | BIT(14) | BIT(15));
	pframe = rtw_set_fixed_ie(pframe, _ASOC_ID_ , (unsigned char *)&le_val, &(pattrib->pktlen));

	if (pstat->bssratelen <= 8)
		pframe = rtw_set_ie(pframe, _SUPPORTEDRATES_IE_, pstat->bssratelen, pstat->bssrateset, &(pattrib->pktlen));
	else {
		pframe = rtw_set_ie(pframe, _SUPPORTEDRATES_IE_, 8, pstat->bssrateset, &(pattrib->pktlen));
		pframe = rtw_set_ie(pframe, _EXT_SUPPORTEDRATES_IE_, (pstat->bssratelen - 8), pstat->bssrateset + 8, &(pattrib->pktlen));
	}

#ifdef CONFIG_IEEE80211W
	if (status == _STATS_REFUSED_TEMPORARILY_) {
		u8 timeout_itvl[5];
		u32 timeout_interval = 3000;
		/* Association Comeback time */
		timeout_itvl[0] = 0x03;
		timeout_interval = cpu_to_le32(timeout_interval);
		memcpy(timeout_itvl + 1, &timeout_interval, 4);
		pframe = rtw_set_ie(pframe, _TIMEOUT_ITVL_IE_, 5, timeout_itvl, &(pattrib->pktlen));
	}
#endif /* CONFIG_IEEE80211W */

	if ((pstat->flags & WLAN_STA_HT) && (pmlmepriv->htpriv.ht_option)) {
		uint ie_len = 0;

		/* FILL HT CAP INFO IE */
		/* p = hostapd_eid_ht_capabilities_info(hapd, p); */
		pbuf = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _HT_CAPABILITY_IE_, &ie_len, (pnetwork->IELength - _BEACON_IE_OFFSET_));
		if (pbuf && ie_len > 0) {
			memcpy(pframe, pbuf, ie_len + 2);
			pframe += (ie_len + 2);
			pattrib->pktlen += (ie_len + 2);
		}

		/* FILL HT ADD INFO IE */
		/* p = hostapd_eid_ht_operation(hapd, p); */
		pbuf = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _HT_ADD_INFO_IE_, &ie_len, (pnetwork->IELength - _BEACON_IE_OFFSET_));
		if (pbuf && ie_len > 0) {
			memcpy(pframe, pbuf, ie_len + 2);
			pframe += (ie_len + 2);
			pattrib->pktlen += (ie_len + 2);
		}

	}

	/*adding EXT_CAPAB_IE */
	if (pmlmepriv->ext_capab_ie_len > 0) {
		uint ie_len = 0;

		pbuf = rtw_get_ie(ie + _BEACON_IE_OFFSET_, _EXT_CAP_IE_, &ie_len, (pnetwork->IELength - _BEACON_IE_OFFSET_));
		if (pbuf && ie_len > 0) {
			memcpy(pframe, pbuf, ie_len + 2);
			pframe += (ie_len + 2);
			pattrib->pktlen += (ie_len + 2);
		}
	}

	/* FILL WMM IE */
	if ((pstat->flags & WLAN_STA_WME) && (pmlmepriv->qospriv.qos_option)) {
		uint ie_len = 0;
		unsigned char WMM_PARA_IE[] = {0x00, 0x50, 0xf2, 0x02, 0x01, 0x01};

		for (pbuf = ie + _BEACON_IE_OFFSET_; ; pbuf += (ie_len + 2)) {
			pbuf = rtw_get_ie(pbuf, _VENDOR_SPECIFIC_IE_, &ie_len, (pnetwork->IELength - _BEACON_IE_OFFSET_ - (ie_len + 2)));
			if (pbuf && !memcmp(pbuf + 2, WMM_PARA_IE, 6)) {
				memcpy(pframe, pbuf, ie_len + 2);
				pframe += (ie_len + 2);
				pattrib->pktlen += (ie_len + 2);

				break;
			}

			if ((pbuf == NULL) || (ie_len == 0))
				break;
		}

	}


	if (pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_REALTEK)
		pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, 6 , REALTEK_96B_IE, &(pattrib->pktlen));

	/* add WPS IE ie for wps 2.0 */
	if (pmlmepriv->wps_assoc_resp_ie && pmlmepriv->wps_assoc_resp_ie_len > 0) {
		memcpy(pframe, pmlmepriv->wps_assoc_resp_ie, pmlmepriv->wps_assoc_resp_ie_len);

		pframe += pmlmepriv->wps_assoc_resp_ie_len;
		pattrib->pktlen += pmlmepriv->wps_assoc_resp_ie_len;
	}

#ifdef CONFIG_P2P
	if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO) && (pstat->is_p2p_device)) {
		u32 len;

		if (padapter->wdinfo.driver_interface == DRIVER_CFG80211) {
			len = 0;
			if (pmlmepriv->p2p_assoc_resp_ie && pmlmepriv->p2p_assoc_resp_ie_len > 0) {
				len = pmlmepriv->p2p_assoc_resp_ie_len;
				memcpy(pframe, pmlmepriv->p2p_assoc_resp_ie, len);
			}
		} else
			len = build_assoc_resp_p2p_ie(pwdinfo, pframe, pstat->p2p_status_code);
		pframe += len;
		pattrib->pktlen += len;
	}

#ifdef CONFIG_WFD
	if (rtw_p2p_chk_role(pwdinfo, P2P_ROLE_GO)) {
		wfdielen = rtw_append_assoc_resp_wfd_ie(padapter, pframe);
		pframe += wfdielen;
		pattrib->pktlen += wfdielen;
	}
#endif

#endif /* CONFIG_P2P */
#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
	pattrib->pktlen += rtw_build_vendor_ie(padapter , pframe , WIFI_ASSOCRESP_VENDOR_IE_BIT);
#endif
	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

#endif
}

void _issue_assocreq(_adapter *padapter, u8 is_reassoc)
{
	int ret = _FAIL;
	struct xmit_frame				*pmgntframe;
	struct pkt_attrib				*pattrib;
	unsigned char					*pframe, *p;
	struct rtw_ieee80211_hdr			*pwlanhdr;
	__le16 *fctrl;
	__le16 val16;
	unsigned int					i, j, ie_len, index = 0;
	unsigned char					rf_type, bssrate[NumRates], sta_bssrate[NumRates];
	PNDIS_802_11_VARIABLE_IEs	pIE;
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	struct xmit_priv		*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	int	bssrate_len = 0, sta_bssrate_len = 0;
	u8	vs_ie_length = 0;
#ifdef CONFIG_P2P
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	u8					p2pie[255] = { 0x00 };
	u16					p2pielen = 0;
#ifdef CONFIG_WFD
	u32					wfdielen = 0;
#endif
#endif /* CONFIG_P2P */

#ifdef CONFIG_DFS
	u16	cap;

	/* Dot H */
	u8 pow_cap_ele[2] = { 0x00 };
	u8 sup_ch[30 * 2] = {0x00 }, sup_ch_idx = 0, idx_5g = 2;	/* For supported channel */
#endif /* CONFIG_DFS */
#ifdef CONFIG_RTW_80211R
	u8 *pie = NULL;
	u32 ft_ie_len = 0;
	ft_priv *pftpriv = &pmlmepriv->ftpriv;
#endif

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		goto exit;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);


	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;
	memcpy(pwlanhdr->addr1, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	if (is_reassoc)
		set_frame_sub_type(pframe, WIFI_REASSOCREQ);
	else
		set_frame_sub_type(pframe, WIFI_ASSOCREQ);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	/* caps */

#ifdef CONFIG_DFS
	memcpy(&cap, rtw_get_capability_from_ie(pmlmeinfo->network.IEs), 2);
	cap |= cap_SpecMgmt;
	memcpy(pframe, &cap, 2);
#else
	memcpy(pframe, rtw_get_capability_from_ie(pmlmeinfo->network.IEs), 2);
#endif /* CONFIG_DFS */

	pframe += 2;
	pattrib->pktlen += 2;

	/* listen interval */
	/* todo: listen interval for power saving */
	val16 = cpu_to_le16(3);
	memcpy(pframe , (unsigned char *)&val16, 2);
	pframe += 2;
	pattrib->pktlen += 2;

	/*Construct Current AP Field for Reassoc-Req only*/
	if (is_reassoc) {
		memcpy(pframe, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);
		pframe += ETH_ALEN;
		pattrib->pktlen += ETH_ALEN;
	}

	/* SSID */
	pframe = rtw_set_ie(pframe, _SSID_IE_,  pmlmeinfo->network.Ssid.SsidLength, pmlmeinfo->network.Ssid.Ssid, &(pattrib->pktlen));

#ifdef CONFIG_DFS
	/* Dot H */
	if (pmlmeext->cur_channel > 14) {
		pow_cap_ele[0] = 13;	/* Minimum transmit power capability */
		pow_cap_ele[1] = 21;	/* Maximum transmit power capability */
		pframe = rtw_set_ie(pframe, EID_PowerCap, 2, pow_cap_ele, &(pattrib->pktlen));

		/* supported channels */
		do {
			if (pmlmeext->channel_set[sup_ch_idx].ChannelNum <= 14) {
				sup_ch[0] = 1;	/* First channel number */
				sup_ch[1] = pmlmeext->channel_set[sup_ch_idx].ChannelNum;	/* Number of channel */
			} else {
				sup_ch[idx_5g++] = pmlmeext->channel_set[sup_ch_idx].ChannelNum;
				sup_ch[idx_5g++] = 1;
			}
			sup_ch_idx++;
		} while (pmlmeext->channel_set[sup_ch_idx].ChannelNum != 0);
		pframe = rtw_set_ie(pframe, EID_SupportedChannels, idx_5g, sup_ch, &(pattrib->pktlen));
	}
#endif /* CONFIG_DFS */

	/* supported rate & extended supported rate */

	get_rate_set(padapter, sta_bssrate, &sta_bssrate_len);
	/* RTW_INFO("sta_bssrate_len=%d\n", sta_bssrate_len); */

	if (pmlmeext->cur_channel == 14) /* for JAPAN, channel 14 can only uses B Mode(CCK) */
		sta_bssrate_len = 4;


	/* for (i = 0; i < sta_bssrate_len; i++) { */
	/*	RTW_INFO("sta_bssrate[%d]=%02X\n", i, sta_bssrate[i]); */
	/* } */

	for (i = 0; i < NDIS_802_11_LENGTH_RATES_EX; i++) {
		if (pmlmeinfo->network.SupportedRates[i] == 0)
			break;
		RTW_INFO("network.SupportedRates[%d]=%02X\n", i, pmlmeinfo->network.SupportedRates[i]);
	}


	for (i = 0; i < NDIS_802_11_LENGTH_RATES_EX; i++) {
		if (pmlmeinfo->network.SupportedRates[i] == 0)
			break;


		/* Check if the AP's supported rates are also supported by STA. */
		for (j = 0; j < sta_bssrate_len; j++) {
			/* Avoid the proprietary data rate (22Mbps) of Handlink WSG-4000 AP */
			if ((pmlmeinfo->network.SupportedRates[i] | IEEE80211_BASIC_RATE_MASK)
			    == (sta_bssrate[j] | IEEE80211_BASIC_RATE_MASK)) {
				/* RTW_INFO("match i = %d, j=%d\n", i, j); */
				break;
			} else {
				/* RTW_INFO("not match: %02X != %02X\n", (pmlmeinfo->network.SupportedRates[i]|IEEE80211_BASIC_RATE_MASK), (sta_bssrate[j]|IEEE80211_BASIC_RATE_MASK)); */
			}
		}

		if (j == sta_bssrate_len) {
			/* the rate is not supported by STA */
			RTW_INFO("%s(): the rate[%d]=%02X is not supported by STA!\n", __func__, i, pmlmeinfo->network.SupportedRates[i]);
		} else {
			/* the rate is supported by STA */
			bssrate[index++] = pmlmeinfo->network.SupportedRates[i];
		}
	}

	bssrate_len = index;
	RTW_INFO("bssrate_len = %d\n", bssrate_len);

	if ((bssrate_len == 0) && (pmlmeinfo->network.SupportedRates[0] != 0)) {
		rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		goto exit; /* don't connect to AP if no joint supported rate */
	}


	if (bssrate_len > 8) {
		pframe = rtw_set_ie(pframe, _SUPPORTEDRATES_IE_ , 8, bssrate, &(pattrib->pktlen));
		pframe = rtw_set_ie(pframe, _EXT_SUPPORTEDRATES_IE_ , (bssrate_len - 8), (bssrate + 8), &(pattrib->pktlen));
	} else if (bssrate_len > 0)
		pframe = rtw_set_ie(pframe, _SUPPORTEDRATES_IE_ , bssrate_len , bssrate, &(pattrib->pktlen));
	else
		RTW_INFO("%s: Connect to AP without 11b and 11g data rate!\n", __func__);

	/* vendor specific IE, such as WPA, WMM, WPS */
	for (i = sizeof(NDIS_802_11_FIXED_IEs); i < pmlmeinfo->network.IELength;) {
		pIE = (PNDIS_802_11_VARIABLE_IEs)(pmlmeinfo->network.IEs + i);

		switch (pIE->ElementID) {
		case _VENDOR_SPECIFIC_IE_:
			if ((!memcmp(pIE->data, RTW_WPA_OUI, 4)) ||
			    (!memcmp(pIE->data, WMM_OUI, 4)) ||
			    (!memcmp(pIE->data, WPS_OUI, 4))) {
				vs_ie_length = pIE->Length;
				if ((!padapter->registrypriv.wifi_spec) && (!memcmp(pIE->data, WPS_OUI, 4))) {
					/* Commented by Kurt 20110629 */
					/* In some older APs, WPS handshake */
					/* would be fail if we append vender extensions informations to AP */

					vs_ie_length = 14;
				}

				pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, vs_ie_length, pIE->data, &(pattrib->pktlen));
			}
			break;

		case EID_WPA2:
#ifdef CONFIG_RTW_80211R
			if ((is_reassoc) && (rtw_to_roam(padapter) > 0) && rtw_chk_ft_flags(padapter, RTW_FT_SUPPORTED)) {
				pie = rtw_get_ie(pftpriv->updated_ft_ies, EID_WPA2, &ft_ie_len, pftpriv->updated_ft_ies_len);
				if (pie)
					pframe = rtw_set_ie(pframe, EID_WPA2, ft_ie_len, pie+2, &(pattrib->pktlen));
			} else
#endif
				pframe = rtw_set_ie(pframe, EID_WPA2, pIE->Length, pIE->data, &(pattrib->pktlen));
			break;
		case EID_HTCapability:
			if (padapter->mlmepriv.htpriv.ht_option) {
				if (!(is_ap_in_tkip(padapter))) {
					memcpy(&(pmlmeinfo->HT_caps), pIE->data, sizeof(struct HT_caps_element));
					pframe = rtw_set_ie(pframe, EID_HTCapability, pIE->Length , (u8 *)(&(pmlmeinfo->HT_caps)), &(pattrib->pktlen));
				}
			}
			break;

		case EID_EXTCapability:
			if (padapter->mlmepriv.htpriv.ht_option)
				pframe = rtw_set_ie(pframe, EID_EXTCapability, pIE->Length, pIE->data, &(pattrib->pktlen));
			break;
		default:
			break;
		}

		i += (pIE->Length + 2);
	}

	if (pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_REALTEK)
		pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, 6 , REALTEK_96B_IE, &(pattrib->pktlen));


#ifdef CONFIG_WAPI_SUPPORT
	rtw_build_assoc_req_wapi_ie(padapter, pframe, pattrib);
#endif


#ifdef CONFIG_P2P

#ifdef CONFIG_IOCTL_CFG80211
	if (adapter_wdev_data(padapter)->p2p_enabled && pwdinfo->driver_interface == DRIVER_CFG80211) {
		if (pmlmepriv->p2p_assoc_req_ie && pmlmepriv->p2p_assoc_req_ie_len > 0) {
			memcpy(pframe, pmlmepriv->p2p_assoc_req_ie, pmlmepriv->p2p_assoc_req_ie_len);
			pframe += pmlmepriv->p2p_assoc_req_ie_len;
			pattrib->pktlen += pmlmepriv->p2p_assoc_req_ie_len;
		}
	} else
#endif /* CONFIG_IOCTL_CFG80211 */
	{
		if (!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE) && !rtw_p2p_chk_state(pwdinfo, P2P_STATE_IDLE)) {
			/*	Should add the P2P IE in the association request frame.	 */
			/*	P2P OUI */

			p2pielen = 0;
			p2pie[p2pielen++] = 0x50;
			p2pie[p2pielen++] = 0x6F;
			p2pie[p2pielen++] = 0x9A;
			p2pie[p2pielen++] = 0x09;	/*	WFA P2P v1.0 */

			/*	Commented by Albert 20101109 */
			/*	According to the P2P Specification, the association request frame should contain 3 P2P attributes */
			/*	1. P2P Capability */
			/*	2. Extended Listen Timing */
			/*	3. Device Info */
			/*	Commented by Albert 20110516 */
			/*	4. P2P Interface */

			/*	P2P Capability */
			/*	Type: */
			p2pie[p2pielen++] = P2P_ATTR_CAPABILITY;

			/*	Length: */
			*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0002);
			p2pielen += 2;

			/*	Value: */
			/*	Device Capability Bitmap, 1 byte */
			p2pie[p2pielen++] = DMP_P2P_DEVCAP_SUPPORT;

			/*	Group Capability Bitmap, 1 byte */
			if (pwdinfo->persistent_supported)
				p2pie[p2pielen++] = P2P_GRPCAP_PERSISTENT_GROUP | DMP_P2P_GRPCAP_SUPPORT;
			else
				p2pie[p2pielen++] = DMP_P2P_GRPCAP_SUPPORT;

			/*	Extended Listen Timing */
			/*	Type: */
			p2pie[p2pielen++] = P2P_ATTR_EX_LISTEN_TIMING;

			/*	Length: */
			*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x0004);
			p2pielen += 2;

			/*	Value: */
			/*	Availability Period */
			*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0xFFFF);
			p2pielen += 2;

			/*	Availability Interval */
			*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0xFFFF);
			p2pielen += 2;

			/*	Device Info */
			/*	Type: */
			p2pie[p2pielen++] = P2P_ATTR_DEVICE_INFO;

			/*	Length: */
			/*	21->P2P Device Address (6bytes) + Config Methods (2bytes) + Primary Device Type (8bytes)  */
			/*	+ NumofSecondDevType (1byte) + WPS Device Name ID field (2bytes) + WPS Device Name Len field (2bytes) */
			*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(21 + pwdinfo->device_name_len);
			p2pielen += 2;

			/*	Value: */
			/*	P2P Device Address */
			memcpy(p2pie + p2pielen, adapter_mac_addr(padapter), ETH_ALEN);
			p2pielen += ETH_ALEN;

			/*	Config Method */
			/*	This field should be big endian. Noted by P2P specification. */
			if ((pwdinfo->ui_got_wps_info == P2P_GOT_WPSINFO_PEER_DISPLAY_PIN) ||
			    (pwdinfo->ui_got_wps_info == P2P_GOT_WPSINFO_SELF_DISPLAY_PIN))
				*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(WPS_CONFIG_METHOD_DISPLAY);
			else
				*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(WPS_CONFIG_METHOD_PBC);

			p2pielen += 2;

			/*	Primary Device Type */
			/*	Category ID */
			*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(WPS_PDT_CID_MULIT_MEDIA);
			p2pielen += 2;

			/*	OUI */
			*(__be32 *)(p2pie + p2pielen) = cpu_to_be32(WPSOUI);
			p2pielen += 4;

			/*	Sub Category ID */
			*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(WPS_PDT_SCID_MEDIA_SERVER);
			p2pielen += 2;

			/*	Number of Secondary Device Types */
			p2pie[p2pielen++] = 0x00;	/*	No Secondary Device Type List */

			/*	Device Name */
			/*	Type: */
			*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(WPS_ATTR_DEVICE_NAME);
			p2pielen += 2;

			/*	Length: */
			*(__be16 *)(p2pie + p2pielen) = cpu_to_be16(pwdinfo->device_name_len);
			p2pielen += 2;

			/*	Value: */
			memcpy(p2pie + p2pielen, pwdinfo->device_name, pwdinfo->device_name_len);
			p2pielen += pwdinfo->device_name_len;

			/*	P2P Interface */
			/*	Type: */
			p2pie[p2pielen++] = P2P_ATTR_INTERFACE;

			/*	Length: */
			*(__le16 *)(p2pie + p2pielen) = cpu_to_le16(0x000D);
			p2pielen += 2;

			/*	Value: */
			memcpy(p2pie + p2pielen, pwdinfo->device_addr, ETH_ALEN);	/*	P2P Device Address */
			p2pielen += ETH_ALEN;

			p2pie[p2pielen++] = 1;	/*	P2P Interface Address Count */

			memcpy(p2pie + p2pielen, pwdinfo->device_addr, ETH_ALEN);	/*	P2P Interface Address List */
			p2pielen += ETH_ALEN;

			pframe = rtw_set_ie(pframe, _VENDOR_SPECIFIC_IE_, p2pielen, (unsigned char *) p2pie, &pattrib->pktlen);
		}
	}

#endif /* CONFIG_P2P */

#ifdef CONFIG_WFD
	wfdielen = rtw_append_assoc_req_wfd_ie(padapter, pframe);
	pframe += wfdielen;
	pattrib->pktlen += wfdielen;
#endif
#ifdef CONFIG_APPEND_VENDOR_IE_ENABLE
	pattrib->pktlen += rtw_build_vendor_ie(padapter , pframe , WIFI_ASSOCREQ_VENDOR_IE_BIT);
#endif
#ifdef CONFIG_RTW_80211R
	if (rtw_chk_ft_flags(padapter, RTW_FT_SUPPORTED)) {
		u8 mdieval[3] = {0};

		memcpy(mdieval, &(pftpriv->mdid), 2);
		mdieval[2] = pftpriv->ft_cap;
		pframe = rtw_set_ie(pframe, _MDIE_, 3, mdieval, &(pattrib->pktlen));
	}

	if (is_reassoc) {
		if ((rtw_to_roam(padapter) > 0) && rtw_chk_ft_flags(padapter, RTW_FT_SUPPORTED)) {
			u8 is_ft_roaming_with_rsn_ie = true;

			pie = rtw_get_ie(pftpriv->updated_ft_ies, EID_WPA2, &ft_ie_len, pftpriv->updated_ft_ies_len);
			if (!pie)
				is_ft_roaming_with_rsn_ie = false;

			pie = rtw_get_ie(pftpriv->updated_ft_ies, _FTIE_, &ft_ie_len, pftpriv->updated_ft_ies_len);
			if (pie && is_ft_roaming_with_rsn_ie)
				pframe = rtw_set_ie(pframe, _FTIE_, ft_ie_len , pie+2, &(pattrib->pktlen));
		}
	}
#endif

	pattrib->last_txcmdsz = pattrib->pktlen;
	dump_mgntframe(padapter, pmgntframe);

	ret = _SUCCESS;

exit:
	if (ret == _SUCCESS)
		rtw_buf_update(&pmlmepriv->assoc_req, &pmlmepriv->assoc_req_len, (u8 *)pwlanhdr, pattrib->pktlen);
	else
		rtw_buf_free(&pmlmepriv->assoc_req, &pmlmepriv->assoc_req_len);

	return;
}

void issue_assocreq(_adapter *padapter)
{
	_issue_assocreq(padapter, false);
}

void issue_reassocreq(_adapter *padapter)
{
	_issue_assocreq(padapter, true);
}

/* when wait_ack is ture, this function shoule be called at process context */
static int _issue_nulldata(_adapter *padapter, unsigned char *da, unsigned int power_mode, int wait_ack)
{
	int ret = _FAIL;
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	struct xmit_priv	*pxmitpriv;
	struct mlme_ext_priv	*pmlmeext;
	struct mlme_ext_info	*pmlmeinfo;

	/* RTW_INFO("%s:%d\n", __func__, power_mode); */

	if (!padapter)
		goto exit;

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		goto exit;

	pxmitpriv = &(padapter->xmitpriv);
	pmlmeext = &(padapter->mlmeextpriv);
	pmlmeinfo = &(pmlmeext->mlmext_info);

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);
	pattrib->retry_ctrl = false;

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	if ((pmlmeinfo->state & 0x03) == WIFI_FW_AP_STATE)
		SetFrDs(fctrl);
	else if ((pmlmeinfo->state & 0x03) == WIFI_FW_STATION_STATE)
		SetToDs(fctrl);

	if (power_mode)
		SetPwrMgt(fctrl);

	memcpy(pwlanhdr->addr1, da, ETH_ALEN);
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_DATA_NULL);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	pattrib->last_txcmdsz = pattrib->pktlen;

	if (wait_ack)
		ret = dump_mgntframe_and_wait_ack(padapter, pmgntframe);
	else {
		dump_mgntframe(padapter, pmgntframe);
		ret = _SUCCESS;
	}

exit:
	return ret;
}

/*
 * [IMPORTANT] Don't call this function in interrupt context
 *
 * When wait_ms > 0, this function should be called at process context
 * wait_ms == 0 means that there is no need to wait ack through C2H_CCX_TX_RPT
 * wait_ms > 0 means you want to wait ack through C2H_CCX_TX_RPT, and the value of wait_ms means the interval between each TX
 * try_cnt means the maximal TX count to try
 * da == NULL for station mode
 */
int issue_nulldata(_adapter *padapter, unsigned char *da, unsigned int power_mode, int try_cnt, int wait_ms)
{
	int ret = _FAIL;
	int i = 0;
	u32 start = jiffies;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct sta_info *psta;
	u8 macid_sleep_reg_access = true;

#ifdef CONFIG_MCC_MODE
	if (MCC_EN(padapter)) {
		/* driver doesn't access macid sleep reg under MCC */
		if (rtw_hal_check_mcc_status(padapter, MCC_STATUS_DOING_MCC)) {
			macid_sleep_reg_access = false;

			if (da == NULL) {
				RTW_INFO("Warning: Do not tx null data to AP under MCC mode\n");
				rtw_warn_on(1);
			}

		}
	}
#endif

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		goto exit;

	/* da == NULL, assum it's null data for sta to ap*/
	if (da == NULL)
		da = get_my_bssid(&(pmlmeinfo->network));

	psta = rtw_get_stainfo(&padapter->stapriv, da);
	if (psta) {
		if (macid_sleep_reg_access) {
			if (power_mode)
				rtw_hal_macid_sleep(padapter, psta->mac_id);
			else
				rtw_hal_macid_wakeup(padapter, psta->mac_id);
		}
	} else {
		RTW_INFO(FUNC_ADPT_FMT ": Can't find sta info for " MAC_FMT ", skip macid %s!!\n",
			FUNC_ADPT_ARG(padapter), MAC_ARG(da), power_mode ? "sleep" : "wakeup");
		rtw_warn_on(1);
	}

	do {
		ret = _issue_nulldata(padapter, da, power_mode, wait_ms > 0 ? true : false);

		i++;

		if (RTW_CANNOT_RUN(padapter))
			break;

		if (i < try_cnt && wait_ms > 0 && ret == _FAIL)
			rtw_msleep_os(wait_ms);

	} while ((i < try_cnt) && ((ret == _FAIL) || (wait_ms == 0)));

	if (ret != _FAIL) {
		ret = _SUCCESS;
#ifndef DBG_XMIT_ACK
		goto exit;
#endif
	}

	if (try_cnt && wait_ms) {
		if (da)
			RTW_INFO(FUNC_ADPT_FMT" to "MAC_FMT", ch:%u%s, %d/%d in %u ms\n",
				FUNC_ADPT_ARG(padapter), MAC_ARG(da), rtw_get_oper_ch(padapter),
				ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
		else
			RTW_INFO(FUNC_ADPT_FMT", ch:%u%s, %d/%d in %u ms\n",
				FUNC_ADPT_ARG(padapter), rtw_get_oper_ch(padapter),
				ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
	}
exit:
	return ret;
}

/*
 * [IMPORTANT] This function run in interrupt context
 *
 * The null data packet would be sent without power bit,
 * and not guarantee success.
 */
s32 issue_nulldata_in_interrupt(PADAPTER padapter, u8 *da, unsigned int power_mode)
{
	int ret;
	struct mlme_ext_priv *pmlmeext;
	struct mlme_ext_info *pmlmeinfo;


	pmlmeext = &padapter->mlmeextpriv;
	pmlmeinfo = &pmlmeext->mlmext_info;

	/* da == NULL, assum it's null data for sta to ap*/
	if (da == NULL)
		da = get_my_bssid(&(pmlmeinfo->network));

	ret = _issue_nulldata(padapter, da, power_mode, false);

	return ret;
}

/* when wait_ack is ture, this function shoule be called at process context */
static int _issue_qos_nulldata(_adapter *padapter, unsigned char *da, u16 tid, int wait_ack)
{
	int ret = _FAIL;
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl, *qc;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		goto exit;

	RTW_INFO("%s\n", __func__);

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	pattrib->hdrlen += 2;
	pattrib->qos_en = true;
	pattrib->eosp = 1;
	pattrib->ack_policy = 0;
	pattrib->mdata = 0;

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	if ((pmlmeinfo->state & 0x03) == WIFI_FW_AP_STATE)
		SetFrDs(fctrl);
	else if ((pmlmeinfo->state & 0x03) == WIFI_FW_STATION_STATE)
		SetToDs(fctrl);

	if (pattrib->mdata)
		SetMData(fctrl);

	qc = (__le16 *)(pframe + pattrib->hdrlen - 2);

	SetPriority(qc, tid);

	SetEOSP(qc, pattrib->eosp);

	SetAckpolicy(qc, pattrib->ack_policy);

	memcpy(pwlanhdr->addr1, da, ETH_ALEN);
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_QOS_DATA_NULL);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr_qos);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr_qos);

	pattrib->last_txcmdsz = pattrib->pktlen;

	if (wait_ack)
		ret = dump_mgntframe_and_wait_ack(padapter, pmgntframe);
	else {
		dump_mgntframe(padapter, pmgntframe);
		ret = _SUCCESS;
	}

exit:
	return ret;
}

/*
 * when wait_ms >0 , this function should be called at process context
 * wait_ms == 0 means that there is no need to wait ack through C2H_CCX_TX_RPT
 * wait_ms > 0 means you want to wait ack through C2H_CCX_TX_RPT, and the value of wait_ms means the interval between each TX
 * try_cnt means the maximal TX count to try
 * da == NULL for station mode
 */
int issue_qos_nulldata(_adapter *padapter, unsigned char *da, u16 tid, int try_cnt, int wait_ms)
{
	int ret = _FAIL;
	int i = 0;
	u32 start = jiffies;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		goto exit;

	/* da == NULL, assum it's null data for sta to ap*/
	if (da == NULL)
		da = get_my_bssid(&(pmlmeinfo->network));

	do {
		ret = _issue_qos_nulldata(padapter, da, tid, wait_ms > 0 ? true : false);

		i++;

		if (RTW_CANNOT_RUN(padapter))
			break;

		if (i < try_cnt && wait_ms > 0 && ret == _FAIL)
			rtw_msleep_os(wait_ms);

	} while ((i < try_cnt) && ((ret == _FAIL) || (wait_ms == 0)));

	if (ret != _FAIL) {
		ret = _SUCCESS;
#ifndef DBG_XMIT_ACK
		goto exit;
#endif
	}

	if (try_cnt && wait_ms) {
		if (da)
			RTW_INFO(FUNC_ADPT_FMT" to "MAC_FMT", ch:%u%s, %d/%d in %u ms\n",
				FUNC_ADPT_ARG(padapter), MAC_ARG(da), rtw_get_oper_ch(padapter),
				ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
		else
			RTW_INFO(FUNC_ADPT_FMT", ch:%u%s, %d/%d in %u ms\n",
				FUNC_ADPT_ARG(padapter), rtw_get_oper_ch(padapter),
				ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
	}
exit:
	return ret;
}

static int _issue_deauth(_adapter *padapter, unsigned char *da, unsigned short reason, u8 wait_ack, u8 key_type)
{
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl, le_reason;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	int ret = _FAIL;
#ifdef CONFIG_P2P
	struct wifidirect_info *pwdinfo = &(padapter->wdinfo);
#endif /* CONFIG_P2P	 */

	/* RTW_INFO("%s to "MAC_FMT"\n", __func__, MAC_ARG(da)); */

#ifdef CONFIG_P2P
	if (!(rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE)) && (pwdinfo->rx_invitereq_info.scan_op_ch_only)) {
		_cancel_timer_ex(&pwdinfo->reset_ch_sitesurvey);
		_set_timer(&pwdinfo->reset_ch_sitesurvey, 10);
	}
#endif /* CONFIG_P2P */

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		goto exit;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);
	pattrib->retry_ctrl = false;
	pattrib->key_type = key_type;
	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	memcpy(pwlanhdr->addr1, da, ETH_ALEN);
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_DEAUTH);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	le_reason = cpu_to_le16(reason);
	pframe = rtw_set_fixed_ie(pframe, _RSON_CODE_ , (unsigned char *)&le_reason, &(pattrib->pktlen));

	pattrib->last_txcmdsz = pattrib->pktlen;


	if (wait_ack)
		ret = dump_mgntframe_and_wait_ack(padapter, pmgntframe);
	else {
		dump_mgntframe(padapter, pmgntframe);
		ret = _SUCCESS;
	}

exit:
	return ret;
}

int issue_deauth(_adapter *padapter, unsigned char *da, unsigned short reason)
{
	RTW_INFO("%s to "MAC_FMT"\n", __func__, MAC_ARG(da));
	return _issue_deauth(padapter, da, reason, false, IEEE80211W_RIGHT_KEY);
}

#ifdef CONFIG_IEEE80211W
int issue_deauth_11w(_adapter *padapter, unsigned char *da, unsigned short reason, u8 key_type)
{
	RTW_INFO("%s to "MAC_FMT"\n", __func__, MAC_ARG(da));
	return _issue_deauth(padapter, da, reason, false, key_type);
}
#endif /* CONFIG_IEEE80211W */

/*
 * wait_ms == 0 means that there is no need to wait ack through C2H_CCX_TX_RPT
 * wait_ms > 0 means you want to wait ack through C2H_CCX_TX_RPT, and the value of wait_ms means the interval between each TX
 * try_cnt means the maximal TX count to try
 */
int issue_deauth_ex(_adapter *padapter, u8 *da, unsigned short reason, int try_cnt,
		    int wait_ms)
{
	int ret = _FAIL;
	int i = 0;
	u32 start = jiffies;

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		goto exit;

	do {
		ret = _issue_deauth(padapter, da, reason, wait_ms > 0 ? true : false, IEEE80211W_RIGHT_KEY);

		i++;

		if (RTW_CANNOT_RUN(padapter))
			break;

		if (i < try_cnt && wait_ms > 0 && ret == _FAIL)
			rtw_msleep_os(wait_ms);

	} while ((i < try_cnt) && ((ret == _FAIL) || (wait_ms == 0)));

	if (ret != _FAIL) {
		ret = _SUCCESS;
#ifndef DBG_XMIT_ACK
		goto exit;
#endif
	}

	if (try_cnt && wait_ms) {
		if (da)
			RTW_INFO(FUNC_ADPT_FMT" to "MAC_FMT", ch:%u%s, %d/%d in %u ms\n",
				FUNC_ADPT_ARG(padapter), MAC_ARG(da), rtw_get_oper_ch(padapter),
				ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
		else
			RTW_INFO(FUNC_ADPT_FMT", ch:%u%s, %d/%d in %u ms\n",
				FUNC_ADPT_ARG(padapter), rtw_get_oper_ch(padapter),
				ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
	}
exit:
	return ret;
}

void issue_action_spct_ch_switch(_adapter *padapter, u8 *ra, u8 new_ch, u8 ch_offset)
{
	unsigned long	irqL;
	_list		*plist, *phead;
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char				*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		return;

	RTW_INFO(FUNC_NDEV_FMT" ra="MAC_FMT", ch:%u, offset:%u\n",
		FUNC_NDEV_ARG(padapter->pnetdev), MAC_ARG(ra), new_ch, ch_offset);

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	memcpy(pwlanhdr->addr1, ra, ETH_ALEN); /* RA */
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN); /* TA */
	memcpy(pwlanhdr->addr3, ra, ETH_ALEN); /* DA = RA */

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	/* category, action */
	{
		u8 category, action;
		category = RTW_WLAN_CATEGORY_SPECTRUM_MGMT;
		action = RTW_WLAN_ACTION_SPCT_CHL_SWITCH;

		pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
		pframe = rtw_set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));
	}

	pframe = rtw_set_ie_ch_switch(pframe, &(pattrib->pktlen), 0, new_ch, 0);
	pframe = rtw_set_ie_secondary_ch_offset(pframe, &(pattrib->pktlen),
			hal_ch_offset_to_secondary_ch_offset(ch_offset));

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);

}

#ifdef CONFIG_IEEE80211W
void issue_action_SA_Query(_adapter *padapter, unsigned char *raddr, unsigned char action, unsigned short tid, u8 key_type)
{
	u8	category = RTW_WLAN_CATEGORY_SA_QUERY;
	u16	reason_code;
	struct xmit_frame		*pmgntframe;
	struct pkt_attrib		*pattrib;
	u8					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	struct xmit_priv		*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct sta_info		*psta;
	struct sta_priv		*pstapriv = &padapter->stapriv;
	struct registry_priv		*pregpriv = &padapter->registrypriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		return;

	RTW_INFO("%s, %04x\n", __func__, tid);

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL) {
		RTW_INFO("%s: alloc_mgtxmitframe fail\n", __func__);
		return;
	}

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);
	pattrib->key_type = key_type;
	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	if (raddr)
		memcpy(pwlanhdr->addr1, raddr, ETH_ALEN);
	else
		memcpy(pwlanhdr->addr1, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	pframe = rtw_set_fixed_ie(pframe, 1, &category, &pattrib->pktlen);
	pframe = rtw_set_fixed_ie(pframe, 1, &action, &pattrib->pktlen);

	switch (action) {
	case 0: /* SA Query req */
		pframe = rtw_set_fixed_ie(pframe, 2, (unsigned char *)&pmlmeext->sa_query_seq, &pattrib->pktlen);
		pmlmeext->sa_query_seq++;
		/* send sa query request to AP, AP should reply sa query response in 1 second */
		if (pattrib->key_type == IEEE80211W_RIGHT_KEY) {
			psta = rtw_get_stainfo(pstapriv, raddr);
			if (psta != NULL) {
				/* RTW_INFO("%s, %d, set dot11w_expire_timer\n", __func__, __LINE__); */
				_set_timer(&psta->dot11w_expire_timer, 1000);
			}
		}
		break;

	case 1: /* SA Query rsp */
		tid = cpu_to_le16(tid);
		/* RTW_INFO("rtw_set_fixed_ie, %04x\n", tid); */
		pframe = rtw_set_fixed_ie(pframe, 2, (unsigned char *)&tid, &pattrib->pktlen);
		break;
	default:
		break;
	}

	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);
}
#endif /* CONFIG_IEEE80211W */

/**
 * issue_action_ba - internal function to TX Block Ack action frame
 * @padapter: the adapter to TX
 * @raddr: receiver address
 * @action: Block Ack Action
 * @tid: tid
 * @size: the announced AMPDU buffer size. used by ADDBA_RESP
 * @status: status/reason code. used by ADDBA_RESP, DELBA
 * @initiator: if we are the initiator of AMPDU association. used by DELBA
 * @wait_ack: used xmit ack
 *
 * Returns:
 * _SUCCESS: No xmit ack is used or acked
 * _FAIL: not acked when using xmit ack
 */
static int issue_action_ba(_adapter *padapter, unsigned char *raddr, unsigned char action
		   , u8 tid, u8 size, u16 status, u8 initiator, int wait_ack)
{
	int ret = _FAIL;
	u8	category = RTW_WLAN_CATEGORY_BACK;
	u16	start_seq;
	u16	BA_para_set;
	u16	BA_timeout_value;
	u16	BA_starting_seqctrl;
	struct xmit_frame		*pmgntframe;
	struct pkt_attrib		*pattrib;
	u8					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl, le_val;
	struct xmit_priv		*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct sta_info		*psta;
	struct sta_priv		*pstapriv = &padapter->stapriv;
	struct registry_priv		*pregpriv = &padapter->registrypriv;

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		goto exit;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		goto exit;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	/* memcpy(pwlanhdr->addr1, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN); */
	memcpy(pwlanhdr->addr1, raddr, ETH_ALEN);
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));

	if (category == 3) {
		switch (action) {
		case RTW_WLAN_ACTION_ADDBA_REQ:
			do {
				pmlmeinfo->dialogToken++;
			} while (pmlmeinfo->dialogToken == 0);
			pframe = rtw_set_fixed_ie(pframe, 1, &(pmlmeinfo->dialogToken), &(pattrib->pktlen));

			BA_para_set = (0x1002 | ((tid & 0xf) << 2)); /* immediate ack & 64 buffer size */

#ifdef CONFIG_TX_AMSDU
			if (padapter->tx_amsdu >= 1) /* TX AMSDU  enabled */
				BA_para_set |= BIT(0);
			else /* TX AMSDU disabled */
				BA_para_set &= ~BIT(0);
#endif
			le_val = cpu_to_le16(BA_para_set);
			pframe = rtw_set_fixed_ie(pframe, 2, (unsigned char *)(&(le_val)), &(pattrib->pktlen));

			/* BA_timeout_value = 0xffff; */ /* max: 65535 TUs(~ 65 ms) */
			BA_timeout_value = 5000;/* ~ 5ms */
			le_val = cpu_to_le16(BA_timeout_value);
			pframe = rtw_set_fixed_ie(pframe, 2, (unsigned char *)(&(le_val)), &(pattrib->pktlen));

			/* if ((psta = rtw_get_stainfo(pstapriv, pmlmeinfo->network.MacAddress)) != NULL) */
			psta = rtw_get_stainfo(pstapriv, raddr);
			if (psta != NULL) {
				start_seq = (psta->sta_xmitpriv.txseq_tid[tid & 0x07] & 0xfff) + 1;

				RTW_INFO("BA_starting_seqctrl = %d for TID=%d\n", start_seq, tid & 0x07);

				psta->BA_starting_seqctrl[tid & 0x07] = start_seq;

				BA_starting_seqctrl = start_seq << 4;
			} else {
				BA_starting_seqctrl = 0;
			}
			le_val = cpu_to_le16(BA_starting_seqctrl);
			pframe = rtw_set_fixed_ie(pframe, 2, (unsigned char *)(&(le_val)), &(pattrib->pktlen));
			break;

		case RTW_WLAN_ACTION_ADDBA_RESP:
			pframe = rtw_set_fixed_ie(pframe, 1, &(pmlmeinfo->ADDBA_req.dialog_token), &(pattrib->pktlen));
			le_val = cpu_to_le16(status);
			pframe = rtw_set_fixed_ie(pframe, 2, (unsigned char *)(&le_val), &(pattrib->pktlen));

			BA_para_set = le16_to_cpu(pmlmeinfo->ADDBA_req.BA_para_set);

			BA_para_set &= ~IEEE80211_ADDBA_PARAM_TID_MASK;
			BA_para_set |= (tid << 2) & IEEE80211_ADDBA_PARAM_TID_MASK;

			BA_para_set &= ~RTW_IEEE80211_ADDBA_PARAM_BUF_SIZE_MASK;
			BA_para_set |= (size << 6) & RTW_IEEE80211_ADDBA_PARAM_BUF_SIZE_MASK;

			if (!padapter->registrypriv.wifi_spec) {
				if (pregpriv->ampdu_amsdu == 0) /* disabled */
					BA_para_set &= ~BIT(0);
				else if (pregpriv->ampdu_amsdu == 1) /* enabled */
					BA_para_set |= BIT(0);
			}

			le_val = cpu_to_le16(BA_para_set);

			pframe = rtw_set_fixed_ie(pframe, 2, (unsigned char *)(&(le_val)), &(pattrib->pktlen));
			pframe = rtw_set_fixed_ie(pframe, 2, (unsigned char *)(&(pmlmeinfo->ADDBA_req.BA_timeout_value)), &(pattrib->pktlen));
			break;

		case RTW_WLAN_ACTION_DELBA:
			BA_para_set = 0;
			BA_para_set |= (tid << 12) & IEEE80211_DELBA_PARAM_TID_MASK;
			BA_para_set |= (initiator << 11) & IEEE80211_DELBA_PARAM_INITIATOR_MASK;

			le_val = cpu_to_le16(BA_para_set);
			pframe = rtw_set_fixed_ie(pframe, 2, (unsigned char *)(&(le_val)), &(pattrib->pktlen));
			le_val = cpu_to_le16(status);
			pframe = rtw_set_fixed_ie(pframe, 2, (unsigned char *)(&(le_val)), &(pattrib->pktlen));
			break;
		default:
			break;
		}
	}

	pattrib->last_txcmdsz = pattrib->pktlen;

	if (wait_ack)
		ret = dump_mgntframe_and_wait_ack(padapter, pmgntframe);
	else {
		dump_mgntframe(padapter, pmgntframe);
		ret = _SUCCESS;
	}

exit:
	return ret;
}

/**
 * issue_addba_req - TX ADDBA_REQ
 * @adapter: the adapter to TX
 * @ra: receiver address
 * @tid: tid
 */
inline void issue_addba_req(_adapter *adapter, unsigned char *ra, u8 tid)
{
	issue_action_ba(adapter, ra, RTW_WLAN_ACTION_ADDBA_REQ
			, tid
			, 0 /* unused */
			, 0 /* unused */
			, 0 /* unused */
			, false
		       );
	RTW_INFO(FUNC_ADPT_FMT" ra="MAC_FMT" tid=%u\n"
		 , FUNC_ADPT_ARG(adapter), MAC_ARG(ra), tid);

}

/**
 * issue_addba_rsp - TX ADDBA_RESP
 * @adapter: the adapter to TX
 * @ra: receiver address
 * @tid: tid
 * @status: status code
 * @size: the announced AMPDU buffer size
 */
inline void issue_addba_rsp(_adapter *adapter, unsigned char *ra, u8 tid, u16 status, u8 size)
{
	issue_action_ba(adapter, ra, RTW_WLAN_ACTION_ADDBA_RESP
			, tid
			, size
			, status
			, 0 /* unused */
			, false
		       );
	RTW_INFO(FUNC_ADPT_FMT" ra="MAC_FMT" status=%u, tid=%u, size=%u\n"
		 , FUNC_ADPT_ARG(adapter), MAC_ARG(ra), status, tid, size);
}

/**
 * issue_addba_rsp_wait_ack - TX ADDBA_RESP and wait ack
 * @adapter: the adapter to TX
 * @ra: receiver address
 * @tid: tid
 * @status: status code
 * @size: the announced AMPDU buffer size
 * @try_cnt: the maximal TX count to try
 * @wait_ms: == 0 means that there is no need to wait ack through C2H_CCX_TX_RPT
 *           > 0 means you want to wait ack through C2H_CCX_TX_RPT, and the value of wait_ms means the interval between each TX
 */
inline u8 issue_addba_rsp_wait_ack(_adapter *adapter, unsigned char *ra, u8 tid, u16 status, u8 size, int try_cnt, int wait_ms)
{
	int ret = _FAIL;
	int i = 0;
	u32 start = jiffies;

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(adapter)))
		goto exit;

	do {
		ret = issue_action_ba(adapter, ra, RTW_WLAN_ACTION_ADDBA_RESP
				      , tid
				      , size
				      , status
				      , 0 /* unused */
				      , true
				     );

		i++;

		if (RTW_CANNOT_RUN(adapter))
			break;

		if (i < try_cnt && wait_ms > 0 && ret == _FAIL)
			rtw_msleep_os(wait_ms);

	} while ((i < try_cnt) && ((ret == _FAIL) || (wait_ms == 0)));

	if (ret != _FAIL) {
		ret = _SUCCESS;
#ifndef DBG_XMIT_ACK
		/* goto exit; */
#endif
	}

	if (try_cnt && wait_ms) {
		RTW_INFO(FUNC_ADPT_FMT" ra="MAC_FMT" tid=%u%s, %d/%d in %u ms\n"
			, FUNC_ADPT_ARG(adapter), MAC_ARG(ra), tid
			, ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
	}

exit:
	return ret;
}

/**
 * issue_del_ba - TX DELBA
 * @adapter: the adapter to TX
 * @ra: receiver address
 * @tid: tid
 * @reason: reason code
 * @initiator: if we are the initiator of AMPDU association. used by DELBA
 */
inline void issue_del_ba(_adapter *adapter, unsigned char *ra, u8 tid, u16 reason, u8 initiator)
{
	issue_action_ba(adapter, ra, RTW_WLAN_ACTION_DELBA
			, tid
			, 0 /* unused */
			, reason
			, initiator
			, false
		       );
	RTW_INFO(FUNC_ADPT_FMT" ra="MAC_FMT" reason=%u, tid=%u, initiator=%u\n"
		 , FUNC_ADPT_ARG(adapter), MAC_ARG(ra), reason, tid, initiator);
}

/**
 * issue_del_ba_ex - TX DELBA with xmit ack options
 * @adapter: the adapter to TX
 * @ra: receiver address
 * @tid: tid
 * @reason: reason code
 * @initiator: if we are the initiator of AMPDU association. used by DELBA
 * @try_cnt: the maximal TX count to try
 * @wait_ms: == 0 means that there is no need to wait ack through C2H_CCX_TX_RPT
 *           > 0 means you want to wait ack through C2H_CCX_TX_RPT, and the value of wait_ms means the interval between each TX
 */
int issue_del_ba_ex(_adapter *adapter, unsigned char *ra, u8 tid, u16 reason, u8 initiator
		    , int try_cnt, int wait_ms)
{
	int ret = _FAIL;
	int i = 0;
	u32 start = jiffies;

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(adapter)))
		goto exit;

	do {
		ret = issue_action_ba(adapter, ra, RTW_WLAN_ACTION_DELBA
				      , tid
				      , 0 /* unused */
				      , reason
				      , initiator
				      , wait_ms > 0 ? true : false
				     );

		i++;

		if (RTW_CANNOT_RUN(adapter))
			break;

		if (i < try_cnt && wait_ms > 0 && ret == _FAIL)
			rtw_msleep_os(wait_ms);

	} while ((i < try_cnt) && ((ret == _FAIL) || (wait_ms == 0)));

	if (ret != _FAIL) {
		ret = _SUCCESS;
#ifndef DBG_XMIT_ACK
		/* goto exit; */
#endif
	}

	if (try_cnt && wait_ms) {
		RTW_INFO(FUNC_ADPT_FMT" ra="MAC_FMT" reason=%u, tid=%u, initiator=%u%s, %d/%d in %u ms\n"
			, FUNC_ADPT_ARG(adapter), MAC_ARG(ra), reason, tid, initiator
			, ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
	}
exit:
	return ret;
}

static void issue_action_BSSCoexistPacket(_adapter *padapter)
{
	unsigned long	irqL;
	_list		*plist, *phead;
	unsigned char category, action;
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char				*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	struct	wlan_network	*pnetwork = NULL;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	_queue		*queue	= &(pmlmepriv->scanned_queue);
	u8 InfoContent[16] = {0};
	u8 ICS[8][15];

	if ((pmlmepriv->num_FortyMHzIntolerant == 0) || (pmlmepriv->num_sta_no_ht == 0))
		return;

	if (pmlmeinfo->bwmode_updated)
		return;

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		return;

	RTW_INFO("%s\n", __func__);


	category = RTW_WLAN_CATEGORY_PUBLIC;
	action = ACT_PUBLIC_BSSCOEXIST;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	memcpy(pwlanhdr->addr1, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));


	/*  */
	if (pmlmepriv->num_FortyMHzIntolerant > 0) {
		u8 iedata = 0;

		iedata |= BIT(2);/* 20 MHz BSS Width Request */

		pframe = rtw_set_ie(pframe, EID_BSSCoexistence,  1, &iedata, &(pattrib->pktlen));

	}


	/*  */
	memset(ICS, 0, sizeof(ICS));
	if (pmlmepriv->num_sta_no_ht > 0) {
		int i;

		_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);

		phead = get_list_head(queue);
		plist = get_next(phead);

		while (1) {
			int len;
			u8 *p;
			WLAN_BSSID_EX *pbss_network;

			if (rtw_end_of_queue_search(phead, plist))
				break;

			pnetwork = LIST_CONTAINOR(plist, struct wlan_network, list);

			plist = get_next(plist);

			pbss_network = (WLAN_BSSID_EX *)&pnetwork->network;

			p = rtw_get_ie(pbss_network->IEs + _FIXED_IE_LENGTH_, _HT_CAPABILITY_IE_, &len, pbss_network->IELength - _FIXED_IE_LENGTH_);
			if ((p == NULL) || (len == 0)) { /* non-HT */
				if ((pbss_network->Configuration.DSConfig <= 0) || (pbss_network->Configuration.DSConfig > 14))
					continue;

				ICS[0][pbss_network->Configuration.DSConfig] = 1;

				if (ICS[0][0] == 0)
					ICS[0][0] = 1;
			}

		}

		_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);


		for (i = 0; i < 8; i++) {
			if (ICS[i][0] == 1) {
				int j, k = 0;

				InfoContent[k] = i;
				/* SET_BSS_INTOLERANT_ELE_REG_CLASS(InfoContent,i); */
				k++;

				for (j = 1; j <= 14; j++) {
					if (ICS[i][j] == 1) {
						if (k < 16) {
							InfoContent[k] = j; /* channel number */
							/* SET_BSS_INTOLERANT_ELE_CHANNEL(InfoContent+k, j); */
							k++;
						}
					}
				}

				pframe = rtw_set_ie(pframe, EID_BSSIntolerantChlReport, k, InfoContent, &(pattrib->pktlen));

			}

		}


	}


	pattrib->last_txcmdsz = pattrib->pktlen;

	dump_mgntframe(padapter, pmgntframe);
}

/* Spatial Multiplexing Powersave (SMPS) action frame */
static int _issue_action_SM_PS(_adapter *padapter ,  unsigned char *raddr , u8 NewMimoPsMode ,  u8 wait_ack)
{

	int ret = _FAIL;
	unsigned char category = RTW_WLAN_CATEGORY_HT;
	u8 action = RTW_WLAN_ACTION_HT_SM_PS;
	u8 sm_power_control = 0;
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	__le16 *fctrl;
	struct xmit_priv			*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);


	if (NewMimoPsMode == WLAN_HT_CAP_SM_PS_DISABLED) {
		sm_power_control = sm_power_control  & ~(BIT(0)); /* SM Power Save Enable = 0 SM Power Save Disable */
	} else if (NewMimoPsMode == WLAN_HT_CAP_SM_PS_STATIC) {
		sm_power_control = sm_power_control | BIT(0);    /* SM Power Save Enable = 1 SM Power Save Enable  */
		sm_power_control = sm_power_control & ~(BIT(1)); /* SM Mode = 0 Static Mode */
	} else if (NewMimoPsMode == WLAN_HT_CAP_SM_PS_DYNAMIC) {
		sm_power_control = sm_power_control | BIT(0); /* SM Power Save Enable = 1 SM Power Save Enable  */
		sm_power_control = sm_power_control | BIT(1); /* SM Mode = 1 Dynamic Mode */
	} else
		return ret;

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		return ret;

	RTW_INFO("%s, sm_power_control=%u, NewMimoPsMode=%u\n", __func__ , sm_power_control , NewMimoPsMode);

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return ret;

	/* update attribute */
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);

	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	memcpy(pwlanhdr->addr1, raddr, ETH_ALEN); /* RA */
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN); /* TA */
	memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN); /* DA = RA */

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	/* category, action */
	pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));

	pframe = rtw_set_fixed_ie(pframe, 1, &(sm_power_control), &(pattrib->pktlen));

	pattrib->last_txcmdsz = pattrib->pktlen;

	if (wait_ack)
		ret = dump_mgntframe_and_wait_ack(padapter, pmgntframe);
	else {
		dump_mgntframe(padapter, pmgntframe);
		ret = _SUCCESS;
	}

	if (ret != _SUCCESS)
		RTW_INFO("%s, ack to\n", __func__);

	return ret;
}

/*
 * wait_ms == 0 means that there is no need to wait ack through C2H_CCX_TX_RPT
 * wait_ms > 0 means you want to wait ack through C2H_CCX_TX_RPT, and the value of wait_ms means the interval between each TX
 * try_cnt means the maximal TX count to try
 */
int issue_action_SM_PS_wait_ack(_adapter *padapter, unsigned char *raddr, u8 NewMimoPsMode, int try_cnt, int wait_ms)
{
	int ret = _FAIL;
	int i = 0;
	u32 start = jiffies;

	if (rtw_rfctl_is_tx_blocked_by_ch_waiting(adapter_to_rfctl(padapter)))
		goto exit;

	do {
		ret = _issue_action_SM_PS(padapter, raddr, NewMimoPsMode , wait_ms > 0 ? true : false);

		i++;

		if (RTW_CANNOT_RUN(padapter))
			break;

		if (i < try_cnt && wait_ms > 0 && ret == _FAIL)
			rtw_msleep_os(wait_ms);

	} while ((i < try_cnt) && ((ret == _FAIL) || (wait_ms == 0)));

	if (ret != _FAIL) {
		ret = _SUCCESS;
#ifndef DBG_XMIT_ACK
		goto exit;
#endif
	}

	if (try_cnt && wait_ms) {
		if (raddr)
			RTW_INFO(FUNC_ADPT_FMT" to "MAC_FMT", %s , %d/%d in %u ms\n",
				 FUNC_ADPT_ARG(padapter), MAC_ARG(raddr),
				ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
		else
			RTW_INFO(FUNC_ADPT_FMT", %s , %d/%d in %u ms\n",
				 FUNC_ADPT_ARG(padapter),
				ret == _SUCCESS ? ", acked" : "", i, try_cnt, rtw_get_passing_time_ms(start));
	}
exit:

	return ret;
}

int issue_action_SM_PS(_adapter *padapter ,  unsigned char *raddr , u8 NewMimoPsMode)
{
	RTW_INFO("%s to "MAC_FMT"\n", __func__, MAC_ARG(raddr));
	return _issue_action_SM_PS(padapter, raddr, NewMimoPsMode , false);
}

/**
 * _send_delba_sta_tid - Cancel the AMPDU association for the specific @sta, @tid
 * @adapter: the adapter to which @sta belongs
 * @initiator: if we are the initiator of AMPDU association
 * @sta: the sta to be checked
 * @tid: the tid to be checked
 * @force: cancel and send DELBA even when no AMPDU association is setup
 * @wait_ack: send delba with xmit ack (valid when initiator == 0)
 *
 * Returns:
 * _FAIL if sta is NULL
 * when initiator is 1, always _SUCCESS
 * when initiator is 0, _SUCCESS if DELBA is acked
 */
static unsigned int _send_delba_sta_tid(_adapter *adapter, u8 initiator, struct sta_info *sta, u8 tid
					, u8 force, int wait_ack)
{
	int ret = _SUCCESS;

	if (sta == NULL) {
		ret = _FAIL;
		goto exit;
	}

	if (initiator == 0) {
		/* recipient */
		if (force || sta->recvreorder_ctrl[tid].enable) {
			u8 ampdu_size_bak = sta->recvreorder_ctrl[tid].ampdu_size;

			sta->recvreorder_ctrl[tid].enable = false;
			sta->recvreorder_ctrl[tid].ampdu_size = RX_AMPDU_SIZE_INVALID;

			if (rtw_del_rx_ampdu_test_trigger_no_tx_fail())
				ret = _FAIL;
			else if (wait_ack)
				ret = issue_del_ba_ex(adapter, sta->hwaddr, tid, 37, initiator, 3, 1);
			else
				issue_del_ba(adapter, sta->hwaddr, tid, 37, initiator);

			if (ret == _FAIL && sta->recvreorder_ctrl[tid].enable == false)
				sta->recvreorder_ctrl[tid].ampdu_size = ampdu_size_bak;
		}
	} else if (initiator == 1) {
		/* originator */
		if (force || sta->htpriv.agg_enable_bitmap & BIT(tid)) {
			sta->htpriv.agg_enable_bitmap &= ~BIT(tid);
			sta->htpriv.candidate_tid_bitmap &= ~BIT(tid);
			issue_del_ba(adapter, sta->hwaddr, tid, 37, initiator);
		}
	}

exit:
	return ret;
}

inline unsigned int send_delba_sta_tid(_adapter *adapter, u8 initiator, struct sta_info *sta, u8 tid
				       , u8 force)
{
	return _send_delba_sta_tid(adapter, initiator, sta, tid, force, 0);
}

inline unsigned int send_delba_sta_tid_wait_ack(_adapter *adapter, u8 initiator, struct sta_info *sta, u8 tid
		, u8 force)
{
	return _send_delba_sta_tid(adapter, initiator, sta, tid, force, 1);
}

unsigned int send_delba(_adapter *padapter, u8 initiator, u8 *addr)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta = NULL;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u16 tid;

	if ((pmlmeinfo->state & 0x03) != WIFI_FW_AP_STATE)
		if (!(pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS))
			return _SUCCESS;

	psta = rtw_get_stainfo(pstapriv, addr);
	if (psta == NULL)
		return _SUCCESS;
	for (tid = 0; tid < TID_NUM; tid++)
		send_delba_sta_tid(padapter, initiator, psta, tid, 0);
	return _SUCCESS;
}

unsigned int send_beacon(_adapter *padapter)
{
	u8	bxmitok = false;
	int	issue = 0;
	int poll = 0;
#if defined(CONFIG_PCI_HCI) && defined(RTL8814AE_SW_BCN)
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
#endif

#ifdef CONFIG_PCI_HCI
	/* RTW_INFO("%s\n", __func__); */

	rtw_hal_set_hwreg(padapter, HW_VAR_BCN_VALID, NULL);

	/* 8192EE Port select for Beacon DL */
	rtw_hal_set_hwreg(padapter, HW_VAR_DL_BCN_SEL, NULL);

	issue_beacon(padapter, 0);

#ifdef RTL8814AE_SW_BCN
	if (pHalData->bCorrectBCN != 0)
		RTW_INFO("%s, line%d, Warnning, pHalData->bCorrectBCN != 0\n", __func__, __LINE__);
	pHalData->bCorrectBCN = 1;
#endif

	return _SUCCESS;
#endif

	u32 start = jiffies;

	rtw_hal_set_hwreg(padapter, HW_VAR_BCN_VALID, NULL);
	rtw_hal_set_hwreg(padapter, HW_VAR_DL_BCN_SEL, NULL);
	do {
		issue_beacon(padapter, 100);
		issue++;
		do {
			rtw_yield_os();
			rtw_hal_get_hwreg(padapter, HW_VAR_BCN_VALID, (u8 *)(&bxmitok));
			poll++;
		} while ((poll % 10) != 0 && false == bxmitok && !RTW_CANNOT_RUN(padapter));

	} while (false == bxmitok && issue < 100 && !RTW_CANNOT_RUN(padapter));

	if (RTW_CANNOT_RUN(padapter))
		return _FAIL;


	if (false == bxmitok) {
		RTW_INFO("%s fail! %u ms\n", __func__, rtw_get_passing_time_ms(start));
		return _FAIL;
	} else {
		u32 passing_time = rtw_get_passing_time_ms(start);

		if (passing_time > 100 || issue > 3)
			RTW_INFO("%s success, issue:%d, poll:%d, %u ms\n", __func__, issue, poll, rtw_get_passing_time_ms(start));
		else if (0)
			RTW_INFO("%s success, issue:%d, poll:%d, %u ms\n", __func__, issue, poll, rtw_get_passing_time_ms(start));

		rtw_hal_fw_correct_bcn(padapter);

		return _SUCCESS;
	}
}

/****************************************************************************

Following are some utitity fuctions for WiFi MLME

*****************************************************************************/

bool IsLegal5GChannel(
	PADAPTER			Adapter,
	u8			channel)
{

	int i = 0;
	u8 Channel_5G[45] = {36, 38, 40, 42, 44, 46, 48, 50, 52, 54, 56, 58,
		60, 62, 64, 100, 102, 104, 106, 108, 110, 112, 114, 116, 118, 120, 122,
		124, 126, 128, 130, 132, 134, 136, 138, 140, 149, 151, 153, 155, 157, 159,
			     161, 163, 165
			    };
	for (i = 0; i < sizeof(Channel_5G); i++)
		if (channel == Channel_5G[i])
			return true;
	return false;
}

/* collect bss info from Beacon and Probe request/response frames. */
u8 collect_bss_info(_adapter *padapter, union recv_frame *precv_frame, WLAN_BSSID_EX *bssid)
{
	int	i;
	u32	len;
	u8	*p;
	u16	val16, subtype;
	u8	*pframe = precv_frame->u.hdr.rx_data;
	u32	packet_len = precv_frame->u.hdr.len;
	u8 ie_offset;
	struct registry_priv	*pregistrypriv = &padapter->registrypriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	len = packet_len - sizeof(struct rtw_ieee80211_hdr_3addr);

	if (len > MAX_IE_SZ) {
		/* RTW_INFO("IE too long for survey event\n"); */
		return _FAIL;
	}

	memset(bssid, 0, sizeof(WLAN_BSSID_EX));

	subtype = get_frame_sub_type(pframe);

	if (subtype == WIFI_BEACON) {
		bssid->Reserved[0] = 1;
		ie_offset = _BEACON_IE_OFFSET_;
	} else {
		/* FIXME : more type */
		if (subtype == WIFI_PROBERSP) {
			ie_offset = _PROBERSP_IE_OFFSET_;
			bssid->Reserved[0] = 3;
		} else if (subtype == WIFI_PROBEREQ) {
			ie_offset = _PROBEREQ_IE_OFFSET_;
			bssid->Reserved[0] = 2;
		} else {
			bssid->Reserved[0] = 0;
			ie_offset = _FIXED_IE_LENGTH_;
		}
	}

	bssid->Length = sizeof(WLAN_BSSID_EX) - MAX_IE_SZ + len;

	/* below is to copy the information element */
	bssid->IELength = len;
	memcpy(bssid->IEs, (pframe + sizeof(struct rtw_ieee80211_hdr_3addr)), bssid->IELength);

	/* get the signal strength */
	/* bssid->Rssi = precv_frame->u.hdr.attrib.SignalStrength; */ /* 0-100 index. */
	bssid->Rssi = precv_frame->u.hdr.attrib.phy_info.RecvSignalPower; /* in dBM.raw data	 */
	bssid->PhyInfo.SignalQuality = precv_frame->u.hdr.attrib.phy_info.SignalQuality;/* in percentage */
	bssid->PhyInfo.SignalStrength = precv_frame->u.hdr.attrib.phy_info.SignalStrength;/* in percentage */
#ifdef CONFIG_ANTENNA_DIVERSITY
	rtw_hal_get_odm_var(padapter, HAL_ODM_ANTDIV_SELECT, &(bssid->PhyInfo.Optimum_antenna), NULL);
#endif

	/* checking SSID */
	p = rtw_get_ie(bssid->IEs + ie_offset, _SSID_IE_, &len, bssid->IELength - ie_offset);
	if (p == NULL) {
		RTW_INFO("marc: cannot find SSID for survey event\n");
		return _FAIL;
	}

	if (*(p + 1)) {
		if (len > NDIS_802_11_LENGTH_SSID) {
			RTW_INFO("%s()-%d: IE too long (%d) for survey event\n", __func__, __LINE__, len);
			return _FAIL;
		}
		memcpy(bssid->Ssid.Ssid, (p + 2), *(p + 1));
		bssid->Ssid.SsidLength = *(p + 1);
	} else
		bssid->Ssid.SsidLength = 0;

	memset(bssid->SupportedRates, 0, NDIS_802_11_LENGTH_RATES_EX);

	/* checking rate info... */
	i = 0;
	p = rtw_get_ie(bssid->IEs + ie_offset, _SUPPORTEDRATES_IE_, &len, bssid->IELength - ie_offset);
	if (p != NULL) {
		if (len > NDIS_802_11_LENGTH_RATES_EX) {
			RTW_INFO("%s()-%d: IE too long (%d) for survey event\n", __func__, __LINE__, len);
			return _FAIL;
		}
		memcpy(bssid->SupportedRates, (p + 2), len);
		i = len;
	}

	p = rtw_get_ie(bssid->IEs + ie_offset, _EXT_SUPPORTEDRATES_IE_, &len, bssid->IELength - ie_offset);
	if (p != NULL) {
		if (len > (NDIS_802_11_LENGTH_RATES_EX - i)) {
			RTW_INFO("%s()-%d: IE too long (%d) for survey event\n", __func__, __LINE__, len);
			return _FAIL;
		}
		memcpy(bssid->SupportedRates + i, (p + 2), len);
	}

	/* todo: */
	bssid->NetworkTypeInUse = Ndis802_11OFDM24;

#ifdef CONFIG_P2P
	if (subtype == WIFI_PROBEREQ) {
		u8 *p2p_ie;
		u32	p2p_ielen;
		/* Set Listion Channel */
		p2p_ie = rtw_get_p2p_ie(bssid->IEs, bssid->IELength, NULL, &p2p_ielen);
		if (p2p_ie) {
			u32	attr_contentlen = 0;
			u8 listen_ch[5] = { 0x00 };

			rtw_get_p2p_attr_content(p2p_ie, p2p_ielen, P2P_ATTR_LISTEN_CH, listen_ch, &attr_contentlen);
			bssid->Configuration.DSConfig = listen_ch[4];
		} else {
			/* use current channel */
			bssid->Configuration.DSConfig = padapter->mlmeextpriv.cur_channel;
			RTW_INFO("%s()-%d: Cannot get p2p_ie. set DSconfig to op_ch(%d)\n", __func__, __LINE__, bssid->Configuration.DSConfig);
		}

		/* FIXME */
		bssid->InfrastructureMode = Ndis802_11Infrastructure;
		memcpy(bssid->MacAddress, get_addr2_ptr(pframe), ETH_ALEN);
		bssid->Privacy = 1;
		return _SUCCESS;
	}
#endif /* CONFIG_P2P */

	if (bssid->IELength < 12)
		return _FAIL;

	/* Checking for DSConfig */
	p = rtw_get_ie(bssid->IEs + ie_offset, _DSSET_IE_, &len, bssid->IELength - ie_offset);

	bssid->Configuration.DSConfig = 0;
	bssid->Configuration.Length = 0;

	if (p)
		bssid->Configuration.DSConfig = *(p + 2);
	else {
		/* In 5G, some ap do not have DSSET IE */
		/* checking HT info for channel */
		p = rtw_get_ie(bssid->IEs + ie_offset, _HT_ADD_INFO_IE_, &len, bssid->IELength - ie_offset);
		if (p) {
			struct HT_info_element *HT_info = (struct HT_info_element *)(p + 2);
			bssid->Configuration.DSConfig = HT_info->primary_channel;
		} else {
			/* use current channel */
			bssid->Configuration.DSConfig = rtw_get_oper_ch(padapter);
		}
	}

	memcpy(&bssid->Configuration.BeaconPeriod, rtw_get_beacon_interval_from_ie(bssid->IEs), 2);

	val16 = rtw_get_capability((WLAN_BSSID_EX *)bssid);

	if (val16 & BIT(0)) {
		bssid->InfrastructureMode = Ndis802_11Infrastructure;
		memcpy(bssid->MacAddress, get_addr2_ptr(pframe), ETH_ALEN);
	} else {
		bssid->InfrastructureMode = Ndis802_11IBSS;
		memcpy(bssid->MacAddress, GetAddr3Ptr(pframe), ETH_ALEN);
	}

	if (val16 & BIT(4))
		bssid->Privacy = 1;
	else
		bssid->Privacy = 0;

	bssid->Configuration.ATIMWindow = 0;

	/* 20/40 BSS Coexistence check */
	if ((pregistrypriv->wifi_spec == 1) && (false == pmlmeinfo->bwmode_updated)) {
		struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
		p = rtw_get_ie(bssid->IEs + ie_offset, _HT_CAPABILITY_IE_, &len, bssid->IELength - ie_offset);
		if (p && len > 0) {
			struct HT_caps_element	*pHT_caps;
			pHT_caps = (struct HT_caps_element *)(p + 2);

			if (pHT_caps->u.HT_cap_element.HT_caps_info & cpu_to_le16(BIT(14)))
				pmlmepriv->num_FortyMHzIntolerant++;
		} else
			pmlmepriv->num_sta_no_ht++;
	}

#ifdef CONFIG_INTEL_WIDI
	/* process_intel_widi_query_or_tigger(padapter, bssid); */
	if (process_intel_widi_query_or_tigger(padapter, bssid))
		return _FAIL;
#endif /* CONFIG_INTEL_WIDI */

#if defined(DBG_RX_SIGNAL_DISPLAY_SSID_MONITORED) & 1
	if (strcmp(bssid->Ssid.Ssid, DBG_RX_SIGNAL_DISPLAY_SSID_MONITORED) == 0) {
		RTW_INFO("Receiving %s("MAC_FMT", DSConfig:%u) from ch%u with ss:%3u, sq:%3u, RawRSSI:%3ld\n"
			, bssid->Ssid.Ssid, MAC_ARG(bssid->MacAddress), bssid->Configuration.DSConfig
			 , rtw_get_oper_ch(padapter)
			, bssid->PhyInfo.SignalStrength, bssid->PhyInfo.SignalQuality, bssid->Rssi
			);
	}
#endif

	/* mark bss info receving from nearby channel as SignalQuality 101 */
	if (bssid->Configuration.DSConfig != rtw_get_oper_ch(padapter))
		bssid->PhyInfo.SignalQuality = 101;

	return _SUCCESS;
}

void start_create_ibss(_adapter *padapter)
{
	unsigned short	caps;
	u8	val8;
	u8	join_type;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*pnetwork = (WLAN_BSSID_EX *)(&(pmlmeinfo->network));
	u8 doiqk = false;
	pmlmeext->cur_channel = (u8)pnetwork->Configuration.DSConfig;
	pmlmeinfo->bcn_interval = get_beacon_interval(pnetwork);

	/* update wireless mode */
	update_wireless_mode(padapter);

	/* udpate capability */
	caps = rtw_get_capability((WLAN_BSSID_EX *)pnetwork);
	update_capinfo(padapter, caps);
	if (caps & cap_IBSS) { /* adhoc master */
		/* set_opmode_cmd(padapter, adhoc); */ /* removed */

		val8 = 0xcf;
		rtw_hal_set_hwreg(padapter, HW_VAR_SEC_CFG, (u8 *)(&val8));

		doiqk = true;
		rtw_hal_set_hwreg(padapter , HW_VAR_DO_IQK , &doiqk);

		/* switch channel */
		set_channel_bwmode(padapter, pmlmeext->cur_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, CHANNEL_WIDTH_20);

		doiqk = false;
		rtw_hal_set_hwreg(padapter , HW_VAR_DO_IQK , &doiqk);

		beacon_timing_control(padapter);

		/* set msr to WIFI_FW_ADHOC_STATE */
		pmlmeinfo->state = WIFI_FW_ADHOC_STATE;
		Set_MSR(padapter, (pmlmeinfo->state & 0x3));

		/* issue beacon */
		if (send_beacon(padapter) == _FAIL) {

			report_join_res(padapter, -1);
			pmlmeinfo->state = WIFI_FW_NULL_STATE;
		} else {
			rtw_hal_set_hwreg(padapter, HW_VAR_BSSID, padapter->registrypriv.dev_network.MacAddress);
			join_type = 0;
			rtw_hal_set_hwreg(padapter, HW_VAR_MLME_JOIN, (u8 *)(&join_type));

			report_join_res(padapter, 1);
			pmlmeinfo->state |= WIFI_FW_ASSOC_SUCCESS;
			rtw_indicate_connect(padapter);
		}
	} else {
		RTW_INFO("start_create_ibss, invalid cap:%x\n", caps);
		return;
	}
	/* update bc/mc sta_info */
	update_bmc_sta(padapter);

}

void start_clnt_join(_adapter *padapter)
{
	unsigned short	caps;
	u8	val8;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*pnetwork = (WLAN_BSSID_EX *)(&(pmlmeinfo->network));
	int beacon_timeout;
	u8 ASIX_ID[] = {0x00, 0x0E, 0xC6};

	/* update wireless mode */
	update_wireless_mode(padapter);

	/* udpate capability */
	caps = rtw_get_capability((WLAN_BSSID_EX *)pnetwork);
	update_capinfo(padapter, caps);

	/* check if sta is ASIX peer and fix IOT issue if it is. */
	if (!memcmp(get_my_bssid(&pmlmeinfo->network) , ASIX_ID , 3)) {
		u8 iot_flag = true;
		rtw_hal_set_hwreg(padapter, HW_VAR_ASIX_IOT, (u8 *)(&iot_flag));
	}

	if (caps & cap_ESS) {
		Set_MSR(padapter, WIFI_FW_STATION_STATE);

		val8 = (pmlmeinfo->auth_algo == dot11AuthAlgrthm_8021X) ? 0xcc : 0xcf;

#ifdef CONFIG_WAPI_SUPPORT
		if (padapter->wapiInfo.bWapiEnable && pmlmeinfo->auth_algo == dot11AuthAlgrthm_WAPI) {
			/* Disable TxUseDefaultKey, RxUseDefaultKey, RxBroadcastUseDefaultKey. */
			val8 = 0x4c;
		}
#endif
		rtw_hal_set_hwreg(padapter, HW_VAR_SEC_CFG, (u8 *)(&val8));

#ifdef CONFIG_DEAUTH_BEFORE_CONNECT
		/* Because of AP's not receiving deauth before */
		/* AP may: 1)not response auth or 2)deauth us after link is complete */
		/* issue deauth before issuing auth to deal with the situation */

		/*	Commented by Albert 2012/07/21 */
		/*	For the Win8 P2P connection, it will be hard to have a successful connection if this Wi-Fi doesn't connect to it. */
		{
#ifdef CONFIG_P2P
			_queue *queue = &(padapter->mlmepriv.scanned_queue);
			_list	*head = get_list_head(queue);
			_list *pos = get_next(head);
			struct wlan_network *scanned = NULL;
			u8 ie_offset = 0;
			unsigned long irqL;
			bool has_p2p_ie = false;

			_enter_critical_bh(&(padapter->mlmepriv.scanned_queue.lock), &irqL);

			for (pos = get_next(head); !rtw_end_of_queue_search(head, pos); pos = get_next(pos)) {

				scanned = LIST_CONTAINOR(pos, struct wlan_network, list);

				if (!memcmp(&(scanned->network.Ssid), &(pnetwork->Ssid), sizeof(NDIS_802_11_SSID))
				    && !memcmp(scanned->network.MacAddress, pnetwork->MacAddress, sizeof(NDIS_802_11_MAC_ADDRESS))
				   ) {
					ie_offset = (scanned->network.Reserved[0] == 2 ? 0 : 12);
					if (rtw_get_p2p_ie(scanned->network.IEs + ie_offset, scanned->network.IELength - ie_offset, NULL, NULL))
						has_p2p_ie = true;
					break;
				}
			}

			_exit_critical_bh(&(padapter->mlmepriv.scanned_queue.lock), &irqL);

			if (scanned == NULL || rtw_end_of_queue_search(head, pos) || has_p2p_ie == false)
#endif /* CONFIG_P2P */
				/* To avoid connecting to AP fail during resume process, change retry count from 5 to 1 */
				issue_deauth_ex(padapter, pnetwork->MacAddress, WLAN_REASON_DEAUTH_LEAVING, 1, 100);
		}
#endif /* CONFIG_DEAUTH_BEFORE_CONNECT */

		/* here wait for receiving the beacon to start auth */
		/* and enable a timer */
		beacon_timeout = decide_wait_for_beacon_timeout(pmlmeinfo->bcn_interval);
		set_link_timer(pmlmeext, beacon_timeout);
		_set_timer(&padapter->mlmepriv.assoc_timer,
			(REAUTH_TO * REAUTH_LIMIT) + (REASSOC_TO * REASSOC_LIMIT) + beacon_timeout);

#ifdef CONFIG_RTW_80211R
		if ((rtw_to_roam(padapter) > 0) && rtw_chk_ft_flags(padapter, RTW_FT_SUPPORTED)) {
			if (rtw_chk_ft_flags(padapter, RTW_FT_OVER_DS_SUPPORTED)) {
				struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
				ft_priv *pftpriv = &pmlmepriv->ftpriv;

				pmlmeinfo->state = WIFI_FW_AUTH_SUCCESS | WIFI_FW_STATION_STATE;
				pftpriv->ft_event.ies =  pftpriv->ft_action + sizeof(struct rtw_ieee80211_hdr_3addr) + 16;
				pftpriv->ft_event.ies_len = pftpriv->ft_action_len - sizeof(struct rtw_ieee80211_hdr_3addr);

				/*Not support RIC*/
				pftpriv->ft_event.ric_ies =  NULL;
				pftpriv->ft_event.ric_ies_len = 0;
				report_ft_event(padapter);
			} else {
				pmlmeinfo->state = WIFI_FW_AUTH_NULL | WIFI_FW_STATION_STATE;
				start_clnt_auth(padapter);
			}
		} else
#endif
			pmlmeinfo->state = WIFI_FW_AUTH_NULL | WIFI_FW_STATION_STATE;
	} else if (caps & cap_IBSS) { /* adhoc client */
		Set_MSR(padapter, WIFI_FW_ADHOC_STATE);

		val8 = 0xcf;
		rtw_hal_set_hwreg(padapter, HW_VAR_SEC_CFG, (u8 *)(&val8));

		beacon_timing_control(padapter);

		pmlmeinfo->state = WIFI_FW_ADHOC_STATE;

		report_join_res(padapter, 1);
	} else {
		/* RTW_INFO("marc: invalid cap:%x\n", caps); */
		return;
	}

}

void start_clnt_auth(_adapter *padapter)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	_cancel_timer_ex(&pmlmeext->link_timer);

	pmlmeinfo->state &= (~WIFI_FW_AUTH_NULL);
	pmlmeinfo->state |= WIFI_FW_AUTH_STATE;

	pmlmeinfo->auth_seq = 1;
	pmlmeinfo->reauth_count = 0;
	pmlmeinfo->reassoc_count = 0;
	pmlmeinfo->link_count = 0;
	pmlmeext->retry = 0;

#ifdef CONFIG_RTW_80211R
	if ((rtw_to_roam(padapter) > 0) && rtw_chk_ft_flags(padapter, RTW_FT_SUPPORTED)) {
		rtw_set_ft_status(padapter, RTW_FT_AUTHENTICATING_STA);
		RTW_PRINT("start ft auth\n");
	} else
#endif
		RTW_PRINT("start auth\n");
	issue_auth(padapter, NULL, 0);

	set_link_timer(pmlmeext, REAUTH_TO);

}


void start_clnt_assoc(_adapter *padapter)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	_cancel_timer_ex(&pmlmeext->link_timer);

	pmlmeinfo->state &= (~(WIFI_FW_AUTH_NULL | WIFI_FW_AUTH_STATE));
	pmlmeinfo->state |= (WIFI_FW_AUTH_SUCCESS | WIFI_FW_ASSOC_STATE);

#ifdef CONFIG_RTW_80211R
	if ((rtw_to_roam(padapter) > 0) && rtw_chk_ft_flags(padapter, RTW_FT_SUPPORTED))
		issue_reassocreq(padapter);
	else
#endif
		issue_assocreq(padapter);

	set_link_timer(pmlmeext, REASSOC_TO);
}

unsigned int receive_disconnect(_adapter *padapter, unsigned char *MacAddr, unsigned short reason, u8 locally_generated)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (!(!memcmp(MacAddr, get_my_bssid(&pmlmeinfo->network), ETH_ALEN)))
		return _SUCCESS;

	RTW_INFO("%s\n", __func__);

	if ((pmlmeinfo->state & 0x03) == WIFI_FW_STATION_STATE) {
		if (pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS) {
			if (report_del_sta_event(padapter, MacAddr, reason, true, locally_generated) != _FAIL)
				pmlmeinfo->state = WIFI_FW_NULL_STATE;
		} else if (pmlmeinfo->state & WIFI_FW_LINKING_STATE) {
			if (report_join_res(padapter, -2) != _FAIL)
				pmlmeinfo->state = WIFI_FW_NULL_STATE;
		} else
			RTW_INFO(FUNC_ADPT_FMT" - End to Disconnect\n", FUNC_ADPT_ARG(padapter));
#ifdef CONFIG_RTW_80211R
		if ((rtw_to_roam(padapter) > 0) && !rtw_chk_ft_status(padapter, RTW_FT_REQUESTED_STA))
			rtw_reset_ft_status(padapter);
#endif
	}

	return _SUCCESS;
}

#ifdef CONFIG_80211D
static void process_80211d(PADAPTER padapter, WLAN_BSSID_EX *bssid)
{
	struct registry_priv *pregistrypriv;
	struct mlme_ext_priv *pmlmeext;
	RT_CHANNEL_INFO *chplan_new;
	u8 channel;
	u8 i;


	pregistrypriv = &padapter->registrypriv;
	pmlmeext = &padapter->mlmeextpriv;

	/* Adjust channel plan by AP Country IE */
	if (pregistrypriv->enable80211d
	    && (!pmlmeext->update_channel_plan_by_ap_done)) {
		u8 *ie, *p;
		u32 len;
		RT_CHANNEL_PLAN chplan_ap;
		RT_CHANNEL_INFO chplan_sta[MAX_CHANNEL_NUM];
		u8 country[4];
		u8 fcn; /* first channel number */
		u8 noc; /* number of channel */
		u8 j, k;

		ie = rtw_get_ie(bssid->IEs + _FIXED_IE_LENGTH_, _COUNTRY_IE_, &len, bssid->IELength - _FIXED_IE_LENGTH_);
		if (!ie)
			return;
		if (len < 6)
			return;

		ie += 2;
		p = ie;
		ie += len;

		memset(country, 0, 4);
		memcpy(country, p, 3);
		p += 3;
		RTW_INFO("%s: 802.11d country=%s\n", __func__, country);

		i = 0;
		while ((ie - p) >= 3) {
			fcn = *(p++);
			noc = *(p++);
			p++;

			for (j = 0; j < noc; j++) {
				if (fcn <= 14)
					channel = fcn + j; /* 2.4 GHz */
				else
					channel = fcn + j * 4; /* 5 GHz */

				chplan_ap.Channel[i++] = channel;
			}
		}
		chplan_ap.Len = i;

#ifdef CONFIG_RTW_DEBUG
		i = 0;
		RTW_INFO("%s: AP[%s] channel plan {", __func__, bssid->Ssid.Ssid);
		while ((i < chplan_ap.Len) && (chplan_ap.Channel[i] != 0)) {
			_RTW_INFO("%02d,", chplan_ap.Channel[i]);
			i++;
		}
		_RTW_INFO("}\n");
#endif

		memcpy(chplan_sta, pmlmeext->channel_set, sizeof(chplan_sta));
#ifdef CONFIG_RTW_DEBUG
		i = 0;
		RTW_INFO("%s: STA channel plan {", __func__);
		while ((i < MAX_CHANNEL_NUM) && (chplan_sta[i].ChannelNum != 0)) {
			_RTW_INFO("%02d(%c),", chplan_sta[i].ChannelNum, chplan_sta[i].ScanType == SCAN_PASSIVE ? 'p' : 'a');
			i++;
		}
		_RTW_INFO("}\n");
#endif

		memset(pmlmeext->channel_set, 0, sizeof(pmlmeext->channel_set));
		chplan_new = pmlmeext->channel_set;

		i = j = k = 0;
		if (pregistrypriv->wireless_mode & WIRELESS_11G) {
			do {
				if ((i == MAX_CHANNEL_NUM)
				    || (chplan_sta[i].ChannelNum == 0)
				    || (chplan_sta[i].ChannelNum > 14))
					break;

				if ((j == chplan_ap.Len) || (chplan_ap.Channel[j] > 14))
					break;

				if (chplan_sta[i].ChannelNum == chplan_ap.Channel[j]) {
					chplan_new[k].ChannelNum = chplan_ap.Channel[j];
					chplan_new[k].ScanType = SCAN_ACTIVE;
					i++;
					j++;
					k++;
				} else if (chplan_sta[i].ChannelNum < chplan_ap.Channel[j]) {
					chplan_new[k].ChannelNum = chplan_sta[i].ChannelNum;
					chplan_new[k].ScanType = SCAN_PASSIVE;
					i++;
					k++;
				} else if (chplan_sta[i].ChannelNum > chplan_ap.Channel[j]) {
					chplan_new[k].ChannelNum = chplan_ap.Channel[j];
					chplan_new[k].ScanType = SCAN_ACTIVE;
					j++;
					k++;
				}
			} while (1);

			/* change AP not support channel to Passive scan */
			while ((i < MAX_CHANNEL_NUM)
			       && (chplan_sta[i].ChannelNum != 0)
			       && (chplan_sta[i].ChannelNum <= 14)) {
				chplan_new[k].ChannelNum = chplan_sta[i].ChannelNum;
				chplan_new[k].ScanType = SCAN_PASSIVE;
				i++;
				k++;
			}

			/* add channel AP supported */
			while ((j < chplan_ap.Len) && (chplan_ap.Channel[j] <= 14)) {
				chplan_new[k].ChannelNum = chplan_ap.Channel[j];
				chplan_new[k].ScanType = SCAN_ACTIVE;
				j++;
				k++;
			}
		} else {
			/* keep original STA 2.4G channel plan */
			while ((i < MAX_CHANNEL_NUM)
			       && (chplan_sta[i].ChannelNum != 0)
			       && (chplan_sta[i].ChannelNum <= 14)) {
				chplan_new[k].ChannelNum = chplan_sta[i].ChannelNum;
				chplan_new[k].ScanType = chplan_sta[i].ScanType;
				i++;
				k++;
			}

			/* skip AP 2.4G channel plan */
			while ((j < chplan_ap.Len) && (chplan_ap.Channel[j] <= 14))
				j++;
		}

		if (pregistrypriv->wireless_mode & WIRELESS_11A) {
			do {
				if ((i >= MAX_CHANNEL_NUM)
				    || (chplan_sta[i].ChannelNum == 0))
					break;

				if ((j == chplan_ap.Len) || (chplan_ap.Channel[j] == 0))
					break;

				if (chplan_sta[i].ChannelNum == chplan_ap.Channel[j]) {
					chplan_new[k].ChannelNum = chplan_ap.Channel[j];
					chplan_new[k].ScanType = SCAN_ACTIVE;
					i++;
					j++;
					k++;
				} else if (chplan_sta[i].ChannelNum < chplan_ap.Channel[j]) {
					chplan_new[k].ChannelNum = chplan_sta[i].ChannelNum;
					chplan_new[k].ScanType = SCAN_PASSIVE;
					i++;
					k++;
				} else if (chplan_sta[i].ChannelNum > chplan_ap.Channel[j]) {
					chplan_new[k].ChannelNum = chplan_ap.Channel[j];
					chplan_new[k].ScanType = SCAN_ACTIVE;
					j++;
					k++;
				}
			} while (1);

			/* change AP not support channel to Passive scan */
			while ((i < MAX_CHANNEL_NUM) && (chplan_sta[i].ChannelNum != 0)) {
				chplan_new[k].ChannelNum = chplan_sta[i].ChannelNum;
				chplan_new[k].ScanType = SCAN_PASSIVE;
				i++;
				k++;
			}

			/* add channel AP supported */
			while ((j < chplan_ap.Len) && (chplan_ap.Channel[j] != 0)) {
				chplan_new[k].ChannelNum = chplan_ap.Channel[j];
				chplan_new[k].ScanType = SCAN_ACTIVE;
				j++;
				k++;
			}
		} else {
			/* keep original STA 5G channel plan */
			while ((i < MAX_CHANNEL_NUM) && (chplan_sta[i].ChannelNum != 0)) {
				chplan_new[k].ChannelNum = chplan_sta[i].ChannelNum;
				chplan_new[k].ScanType = chplan_sta[i].ScanType;
				i++;
				k++;
			}
		}

		pmlmeext->update_channel_plan_by_ap_done = 1;

#ifdef CONFIG_RTW_DEBUG
		k = 0;
		RTW_INFO("%s: new STA channel plan {", __func__);
		while ((k < MAX_CHANNEL_NUM) && (chplan_new[k].ChannelNum != 0)) {
			_RTW_INFO("%02d(%c),", chplan_new[k].ChannelNum, chplan_new[k].ScanType == SCAN_PASSIVE ? 'p' : 'c');
			k++;
		}
		_RTW_INFO("}\n");
#endif
	}

	/* If channel is used by AP, set channel scan type to active */
	channel = bssid->Configuration.DSConfig;
	chplan_new = pmlmeext->channel_set;
	i = 0;
	while ((i < MAX_CHANNEL_NUM) && (chplan_new[i].ChannelNum != 0)) {
		if (chplan_new[i].ChannelNum == channel) {
			if (chplan_new[i].ScanType == SCAN_PASSIVE) {
				/* 5G Bnad 2, 3 (DFS) doesn't change to active scan */
				if (channel >= 52 && channel <= 144)
					break;

				chplan_new[i].ScanType = SCAN_ACTIVE;
				RTW_INFO("%s: change channel %d scan type from passive to active\n",
					 __func__, channel);
			}
			break;
		}
		i++;
	}
}
#endif

/****************************************************************************

Following are the functions to report events

*****************************************************************************/

void report_survey_event(_adapter *padapter, union recv_frame *precv_frame)
{
	struct cmd_obj *pcmd_obj;
	u8	*pevtcmd;
	u32 cmdsz;
	struct survey_event	*psurvey_evt;
	struct C2HEvent_Header *pc2h_evt_hdr;
	struct mlme_ext_priv *pmlmeext;
	struct cmd_priv *pcmdpriv;
	/* u8 *pframe = precv_frame->u.hdr.rx_data; */
	/* uint len = precv_frame->u.hdr.len; */

	if (!padapter)
		return;

	pmlmeext = &padapter->mlmeextpriv;
	pcmdpriv = &padapter->cmdpriv;


	pcmd_obj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmd_obj == NULL)
		return;

	cmdsz = (sizeof(struct survey_event) + sizeof(struct C2HEvent_Header));
	pevtcmd = (u8 *)rtw_zmalloc(cmdsz);
	if (pevtcmd == NULL) {
		rtw_mfree((u8 *)pcmd_obj, sizeof(struct cmd_obj));
		return;
	}

	INIT_LIST_HEAD(&pcmd_obj->list);

	pcmd_obj->cmdcode = GEN_CMD_CODE(_Set_MLME_EVT);
	pcmd_obj->cmdsz = cmdsz;
	pcmd_obj->parmbuf = pevtcmd;

	pcmd_obj->rsp = NULL;
	pcmd_obj->rspsz  = 0;

	pc2h_evt_hdr = (struct C2HEvent_Header *)(pevtcmd);
	pc2h_evt_hdr->len = sizeof(struct survey_event);
	pc2h_evt_hdr->ID = GEN_EVT_CODE(_Survey);
	pc2h_evt_hdr->seq = ATOMIC_INC_RETURN(&pmlmeext->event_seq);

	psurvey_evt = (struct survey_event *)(pevtcmd + sizeof(struct C2HEvent_Header));

	if (collect_bss_info(padapter, precv_frame, (WLAN_BSSID_EX *)&psurvey_evt->bss) == _FAIL) {
		rtw_mfree((u8 *)pcmd_obj, sizeof(struct cmd_obj));
		rtw_mfree((u8 *)pevtcmd, cmdsz);
		return;
	}

#ifdef CONFIG_80211D
	process_80211d(padapter, &psurvey_evt->bss);
#endif

	rtw_enqueue_cmd(pcmdpriv, pcmd_obj);

	pmlmeext->sitesurvey_res.bss_cnt++;

	return;

}

void report_surveydone_event(_adapter *padapter)
{
	struct cmd_obj *pcmd_obj;
	u8	*pevtcmd;
	u32 cmdsz;
	struct surveydone_event *psurveydone_evt;
	struct C2HEvent_Header	*pc2h_evt_hdr;
	struct mlme_ext_priv		*pmlmeext = &padapter->mlmeextpriv;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	pcmd_obj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmd_obj == NULL)
		return;

	cmdsz = (sizeof(struct surveydone_event) + sizeof(struct C2HEvent_Header));
	pevtcmd = (u8 *)rtw_zmalloc(cmdsz);
	if (pevtcmd == NULL) {
		rtw_mfree((u8 *)pcmd_obj, sizeof(struct cmd_obj));
		return;
	}

	INIT_LIST_HEAD(&pcmd_obj->list);

	pcmd_obj->cmdcode = GEN_CMD_CODE(_Set_MLME_EVT);
	pcmd_obj->cmdsz = cmdsz;
	pcmd_obj->parmbuf = pevtcmd;

	pcmd_obj->rsp = NULL;
	pcmd_obj->rspsz  = 0;

	pc2h_evt_hdr = (struct C2HEvent_Header *)(pevtcmd);
	pc2h_evt_hdr->len = sizeof(struct surveydone_event);
	pc2h_evt_hdr->ID = GEN_EVT_CODE(_SurveyDone);
	pc2h_evt_hdr->seq = ATOMIC_INC_RETURN(&pmlmeext->event_seq);

	psurveydone_evt = (struct surveydone_event *)(pevtcmd + sizeof(struct C2HEvent_Header));
	psurveydone_evt->bss_cnt = pmlmeext->sitesurvey_res.bss_cnt;

	RTW_INFO("survey done event(%x) band:%d for "ADPT_FMT"\n", psurveydone_evt->bss_cnt, padapter->setband, ADPT_ARG(padapter));

	rtw_enqueue_cmd(pcmdpriv, pcmd_obj);

	return;

}

u32 report_join_res(_adapter *padapter, int res)
{
	struct cmd_obj *pcmd_obj;
	u8	*pevtcmd;
	u32 cmdsz;
	struct joinbss_event		*pjoinbss_evt;
	struct C2HEvent_Header	*pc2h_evt_hdr;
	struct mlme_ext_priv		*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	u32 ret = _FAIL;

	pcmd_obj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmd_obj == NULL)
		goto exit;

	cmdsz = (sizeof(struct joinbss_event) + sizeof(struct C2HEvent_Header));
	pevtcmd = (u8 *)rtw_zmalloc(cmdsz);
	if (pevtcmd == NULL) {
		rtw_mfree((u8 *)pcmd_obj, sizeof(struct cmd_obj));
		goto exit;
	}

	INIT_LIST_HEAD(&pcmd_obj->list);

	pcmd_obj->cmdcode = GEN_CMD_CODE(_Set_MLME_EVT);
	pcmd_obj->cmdsz = cmdsz;
	pcmd_obj->parmbuf = pevtcmd;

	pcmd_obj->rsp = NULL;
	pcmd_obj->rspsz  = 0;

	pc2h_evt_hdr = (struct C2HEvent_Header *)(pevtcmd);
	pc2h_evt_hdr->len = sizeof(struct joinbss_event);
	pc2h_evt_hdr->ID = GEN_EVT_CODE(_JoinBss);
	pc2h_evt_hdr->seq = ATOMIC_INC_RETURN(&pmlmeext->event_seq);

	pjoinbss_evt = (struct joinbss_event *)(pevtcmd + sizeof(struct C2HEvent_Header));
	memcpy((unsigned char *)(&(pjoinbss_evt->network.network)), &(pmlmeinfo->network), sizeof(WLAN_BSSID_EX));
	pjoinbss_evt->network.join_res	= pjoinbss_evt->network.aid = res;

	RTW_INFO("report_join_res(%d)\n", res);


	rtw_joinbss_event_prehandle(padapter, (u8 *)&pjoinbss_evt->network);


	ret = rtw_enqueue_cmd(pcmdpriv, pcmd_obj);

exit:
	return ret;
}

void report_wmm_edca_update(_adapter *padapter)
{
	struct cmd_obj *pcmd_obj;
	u8	*pevtcmd;
	u32 cmdsz;
	struct wmm_event		*pwmm_event;
	struct C2HEvent_Header	*pc2h_evt_hdr;
	struct mlme_ext_priv		*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	pcmd_obj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmd_obj == NULL)
		return;

	cmdsz = (sizeof(struct wmm_event) + sizeof(struct C2HEvent_Header));
	pevtcmd = (u8 *)rtw_zmalloc(cmdsz);
	if (pevtcmd == NULL) {
		rtw_mfree((u8 *)pcmd_obj, sizeof(struct cmd_obj));
		return;
	}

	INIT_LIST_HEAD(&pcmd_obj->list);

	pcmd_obj->cmdcode = GEN_CMD_CODE(_Set_MLME_EVT);
	pcmd_obj->cmdsz = cmdsz;
	pcmd_obj->parmbuf = pevtcmd;

	pcmd_obj->rsp = NULL;
	pcmd_obj->rspsz  = 0;

	pc2h_evt_hdr = (struct C2HEvent_Header *)(pevtcmd);
	pc2h_evt_hdr->len = sizeof(struct wmm_event);
	pc2h_evt_hdr->ID = GEN_EVT_CODE(_WMM);
	pc2h_evt_hdr->seq = ATOMIC_INC_RETURN(&pmlmeext->event_seq);

	pwmm_event = (struct wmm_event *)(pevtcmd + sizeof(struct C2HEvent_Header));
	pwmm_event->wmm = 0;

	rtw_enqueue_cmd(pcmdpriv, pcmd_obj);

	return;

}

u32 report_del_sta_event(_adapter *padapter, unsigned char *MacAddr, unsigned short reason, bool enqueue, u8 locally_generated)
{
	struct cmd_obj *pcmd_obj;
	u8	*pevtcmd;
	u32 cmdsz;
	struct sta_info *psta;
	int	mac_id = -1;
	struct stadel_event			*pdel_sta_evt;
	struct C2HEvent_Header	*pc2h_evt_hdr;
	struct mlme_ext_priv		*pmlmeext = &padapter->mlmeextpriv;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	u8 res = _SUCCESS;

	/* prepare cmd parameter */
	cmdsz = (sizeof(struct stadel_event) + sizeof(struct C2HEvent_Header));
	pevtcmd = (u8 *)rtw_zmalloc(cmdsz);
	if (pevtcmd == NULL) {
		res = _FAIL;
		goto exit;
	}

	pc2h_evt_hdr = (struct C2HEvent_Header *)(pevtcmd);
	pc2h_evt_hdr->len = sizeof(struct stadel_event);
	pc2h_evt_hdr->ID = GEN_EVT_CODE(_DelSTA);
	pc2h_evt_hdr->seq = ATOMIC_INC_RETURN(&pmlmeext->event_seq);

	pdel_sta_evt = (struct stadel_event *)(pevtcmd + sizeof(struct C2HEvent_Header));
	memcpy((unsigned char *)(&(pdel_sta_evt->macaddr)), MacAddr, ETH_ALEN);
	memcpy((unsigned char *)(pdel_sta_evt->rsvd), (unsigned char *)(&reason), 2);
	psta = rtw_get_stainfo(&padapter->stapriv, MacAddr);
	if (psta)
		mac_id = (int)psta->mac_id;
	else
		mac_id = (-1);
	pdel_sta_evt->mac_id = mac_id;
	pdel_sta_evt->locally_generated = locally_generated;

	if (!enqueue) {
		/* do directly */
		rtw_stadel_event_callback(padapter, (u8 *)pdel_sta_evt);
		rtw_mfree(pevtcmd, cmdsz);
	} else {
		pcmd_obj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
		if (pcmd_obj == NULL) {
			rtw_mfree(pevtcmd, cmdsz);
			res = _FAIL;
			goto exit;
		}

		INIT_LIST_HEAD(&pcmd_obj->list);
		pcmd_obj->cmdcode = GEN_CMD_CODE(_Set_MLME_EVT);
		pcmd_obj->cmdsz = cmdsz;
		pcmd_obj->parmbuf = pevtcmd;

		pcmd_obj->rsp = NULL;
		pcmd_obj->rspsz  = 0;

		res = rtw_enqueue_cmd(pcmdpriv, pcmd_obj);
	}

exit:

	RTW_INFO(FUNC_ADPT_FMT" "MAC_FMT" mac_id=%d, enqueue:%d, res:%u\n"
		, FUNC_ADPT_ARG(padapter), MAC_ARG(MacAddr), mac_id, enqueue, res);

	return res;
}

void report_add_sta_event(_adapter *padapter, unsigned char *MacAddr)
{
	struct cmd_obj *pcmd_obj;
	u8	*pevtcmd;
	u32 cmdsz;
	struct stassoc_event		*padd_sta_evt;
	struct C2HEvent_Header	*pc2h_evt_hdr;
	struct mlme_ext_priv		*pmlmeext = &padapter->mlmeextpriv;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	pcmd_obj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmd_obj == NULL)
		return;

	cmdsz = (sizeof(struct stassoc_event) + sizeof(struct C2HEvent_Header));
	pevtcmd = (u8 *)rtw_zmalloc(cmdsz);
	if (pevtcmd == NULL) {
		rtw_mfree((u8 *)pcmd_obj, sizeof(struct cmd_obj));
		return;
	}

	INIT_LIST_HEAD(&pcmd_obj->list);

	pcmd_obj->cmdcode = GEN_CMD_CODE(_Set_MLME_EVT);
	pcmd_obj->cmdsz = cmdsz;
	pcmd_obj->parmbuf = pevtcmd;

	pcmd_obj->rsp = NULL;
	pcmd_obj->rspsz  = 0;

	pc2h_evt_hdr = (struct C2HEvent_Header *)(pevtcmd);
	pc2h_evt_hdr->len = sizeof(struct stassoc_event);
	pc2h_evt_hdr->ID = GEN_EVT_CODE(_AddSTA);
	pc2h_evt_hdr->seq = ATOMIC_INC_RETURN(&pmlmeext->event_seq);

	padd_sta_evt = (struct stassoc_event *)(pevtcmd + sizeof(struct C2HEvent_Header));
	memcpy((unsigned char *)(&(padd_sta_evt->macaddr)), MacAddr, ETH_ALEN);

	RTW_INFO("report_add_sta_event: add STA\n");

	rtw_enqueue_cmd(pcmdpriv, pcmd_obj);

	return;
}


bool rtw_port_switch_chk(_adapter *adapter)
{
	bool switch_needed = false;
#ifdef CONFIG_CONCURRENT_MODE
#ifdef CONFIG_RUNTIME_PORT_SWITCH
	struct dvobj_priv *dvobj = adapter_to_dvobj(adapter);
	struct pwrctrl_priv *pwrctl = dvobj_to_pwrctl(dvobj);
	_adapter *if_port0 = NULL;
	_adapter *if_port1 = NULL;
	struct mlme_ext_info *if_port0_mlmeinfo = NULL;
	struct mlme_ext_info *if_port1_mlmeinfo = NULL;
	int i;

	for (i = 0; i < dvobj->iface_nums; i++) {
		if (get_hw_port(dvobj->padapters[i]) == HW_PORT0) {
			if_port0 = dvobj->padapters[i];
			if_port0_mlmeinfo = &(if_port0->mlmeextpriv.mlmext_info);
		} else if (get_hw_port(dvobj->padapters[i]) == HW_PORT1) {
			if_port1 = dvobj->padapters[i];
			if_port1_mlmeinfo = &(if_port1->mlmeextpriv.mlmext_info);
		}
	}

	if (if_port0 == NULL) {
		rtw_warn_on(1);
		goto exit;
	}

	if (if_port1 == NULL) {
		rtw_warn_on(1);
		goto exit;
	}

#ifdef DBG_RUNTIME_PORT_SWITCH
	RTW_INFO(FUNC_ADPT_FMT" wowlan_mode:%u\n"
		 ADPT_FMT", port0, mlmeinfo->state:0x%08x, p2p_state:%d, %d\n"
		 ADPT_FMT", port1, mlmeinfo->state:0x%08x, p2p_state:%d, %d\n",
		 FUNC_ADPT_ARG(adapter), pwrctl->wowlan_mode,
		ADPT_ARG(if_port0), if_port0_mlmeinfo->state, rtw_p2p_state(&if_port0->wdinfo), rtw_p2p_chk_state(&if_port0->wdinfo, P2P_STATE_NONE),
		ADPT_ARG(if_port1), if_port1_mlmeinfo->state, rtw_p2p_state(&if_port1->wdinfo), rtw_p2p_chk_state(&if_port1->wdinfo, P2P_STATE_NONE));
#endif /* DBG_RUNTIME_PORT_SWITCH */

#ifdef CONFIG_WOWLAN
	/* WOWLAN interface(primary, for now) should be port0 */
	if (pwrctl->wowlan_mode) {
		if (!is_primary_adapter(if_port0)) {
			RTW_INFO("%s "ADPT_FMT" enable WOWLAN\n", __func__, ADPT_ARG(if_port1));
			switch_needed = true;
		}
		goto exit;
	}
#endif /* CONFIG_WOWLAN */

	/* AP should use port0 for ctl frame's ack */
	if ((if_port1_mlmeinfo->state & 0x03) == WIFI_FW_AP_STATE) {
		RTW_INFO("%s "ADPT_FMT" is AP/GO\n", __func__, ADPT_ARG(if_port1));
		switch_needed = true;
		goto exit;
	}

	/* GC should use port0 for p2p ps */
	if (((if_port1_mlmeinfo->state & 0x03) == WIFI_FW_STATION_STATE)
	    && (if_port1_mlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)
#ifdef CONFIG_P2P
	    && !rtw_p2p_chk_state(&if_port1->wdinfo, P2P_STATE_NONE)
#endif
	    && !check_fwstate(&if_port1->mlmepriv, WIFI_UNDER_WPS)
	   ) {
		RTW_INFO("%s "ADPT_FMT" is GC\n", __func__, ADPT_ARG(if_port1));
		switch_needed = true;
		goto exit;
	}

	/* port1 linked, but port0 not linked */
	if ((if_port1_mlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)
	    && !(if_port0_mlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)
	    && ((if_port0_mlmeinfo->state & 0x03) != WIFI_FW_AP_STATE)
	   ) {
		RTW_INFO("%s "ADPT_FMT" is SINGLE_LINK\n", __func__, ADPT_ARG(if_port1));
		switch_needed = true;
		goto exit;
	}

exit:
#ifdef DBG_RUNTIME_PORT_SWITCH
	RTW_INFO(FUNC_ADPT_FMT" ret:%d\n", FUNC_ADPT_ARG(adapter), switch_needed);
#endif /* DBG_RUNTIME_PORT_SWITCH */
#endif /* CONFIG_RUNTIME_PORT_SWITCH */
#endif /* CONFIG_CONCURRENT_MODE */
	return switch_needed;
}

/****************************************************************************

Following are the event callback functions

*****************************************************************************/

/* for sta/adhoc mode */
void update_sta_info(_adapter *padapter, struct sta_info *psta)
{
	unsigned long	irqL;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	/* ERP */
	VCS_update(padapter, psta);

	/* HT */
	if (pmlmepriv->htpriv.ht_option) {
		psta->htpriv.ht_option = true;

		psta->htpriv.ampdu_enable = pmlmepriv->htpriv.ampdu_enable;

		psta->htpriv.rx_ampdu_min_spacing = (pmlmeinfo->HT_caps.u.HT_cap_element.AMPDU_para & IEEE80211_HT_CAP_AMPDU_DENSITY) >> 2;

		if (support_short_GI(padapter, &(pmlmeinfo->HT_caps), CHANNEL_WIDTH_20))
			psta->htpriv.sgi_20m = true;

		if (support_short_GI(padapter, &(pmlmeinfo->HT_caps), CHANNEL_WIDTH_40))
			psta->htpriv.sgi_40m = true;

		psta->qos_option = true;

		psta->htpriv.ldpc_cap = pmlmepriv->htpriv.ldpc_cap;
		psta->htpriv.stbc_cap = pmlmepriv->htpriv.stbc_cap;
		psta->htpriv.beamform_cap = pmlmepriv->htpriv.beamform_cap;

		memcpy(&psta->htpriv.ht_cap, &pmlmeinfo->HT_caps, sizeof(struct rtw_ieee80211_ht_cap));
	} else
	{
		psta->htpriv.ht_option = false;
		psta->htpriv.ampdu_enable = false;
		psta->htpriv.tx_amsdu_enable = false;
		psta->htpriv.sgi_20m = false;
		psta->htpriv.sgi_40m = false;
		psta->qos_option = false;

	}

	psta->htpriv.ch_offset = pmlmeext->cur_ch_offset;

	psta->htpriv.agg_enable_bitmap = 0x0;/* reset */
	psta->htpriv.candidate_tid_bitmap = 0x0;/* reset */

	psta->bw_mode = pmlmeext->cur_bwmode;

	/* QoS */
	if (pmlmepriv->qospriv.qos_option)
		psta->qos_option = true;

	update_ldpc_stbc_cap(psta);

	_enter_critical_ex(&psta->lock, &irqL);
	psta->state = _FW_LINKED;
	spin_unlock_irqrestore(&psta->lock, irqL);

}

static void rtw_mlmeext_disconnect(_adapter *padapter)
{
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*pnetwork = (WLAN_BSSID_EX *)(&(pmlmeinfo->network));
	u8 state_backup = (pmlmeinfo->state & 0x03);
	u8 ASIX_ID[] = {0x00, 0x0E, 0xC6};

	/* set_opmode_cmd(padapter, infra_client_with_mlme); */

	rtw_hal_set_hwreg(padapter, HW_VAR_MLME_DISCONNECT, NULL);
	rtw_hal_set_hwreg(padapter, HW_VAR_BSSID, null_addr);

	/* set MSR to no link state->infra. mode */
	Set_MSR(padapter, _HW_STATE_STATION_);

	/* check if sta is ASIX peer and fix IOT issue if it is. */
	if (!memcmp(get_my_bssid(&pmlmeinfo->network) , ASIX_ID , 3)) {
		u8 iot_flag = false;
		rtw_hal_set_hwreg(padapter, HW_VAR_ASIX_IOT, (u8 *)(&iot_flag));
	}
	pmlmeinfo->state = WIFI_FW_NULL_STATE;

#ifdef CONFIG_MCC_MODE
	/* mcc disconnect setting before download LPS rsvd page */
	rtw_hal_set_mcc_setting_disconnect(padapter);
#endif /* CONFIG_MCC_MODE */

	if (state_backup == WIFI_FW_STATION_STATE) {
		if (rtw_port_switch_chk(padapter)) {
			rtw_hal_set_hwreg(padapter, HW_VAR_PORT_SWITCH, NULL);
#ifdef CONFIG_LPS
			{
				_adapter *port0_iface = dvobj_get_port0_adapter(adapter_to_dvobj(padapter));
				if (port0_iface)
					rtw_lps_ctrl_wk_cmd(port0_iface, LPS_CTRL_CONNECT, 0);
			}
#endif
		}
	}

	/* switch to the 20M Hz mode after disconnect */
	pmlmeext->cur_bwmode = CHANNEL_WIDTH_20;
	pmlmeext->cur_ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;

#ifdef CONFIG_FCS_MODE
	if (EN_FCS(padapter))
		rtw_hal_set_hwreg(padapter, HW_VAR_STOP_FCS_MODE, NULL);
#endif

#ifdef CONFIG_DFS_MASTER
	if (check_fwstate(pmlmepriv, WIFI_AP_STATE))
		rtw_dfs_master_status_apply(padapter, MLME_AP_STOPPED);
	else if (check_fwstate(pmlmepriv, WIFI_STATION_STATE))
		rtw_dfs_master_status_apply(padapter, MLME_STA_DISCONNECTED);
#endif

	{
		u8 ch, bw, offset;

		if (rtw_mi_get_ch_setting_union_no_self(padapter, &ch, &bw, &offset) != 0) {
			set_channel_bwmode(padapter, ch, offset, bw);
			rtw_mi_update_union_chan_inf(padapter, ch, offset, bw);
		}
	}

	flush_all_cam_entry(padapter);

	_cancel_timer_ex(&pmlmeext->link_timer);

	/* pmlmepriv->LinkDetectInfo.TrafficBusyState = false; */
	pmlmepriv->LinkDetectInfo.TrafficTransitionCount = 0;
	pmlmepriv->LinkDetectInfo.LowPowerTransitionCount = 0;

#ifdef CONFIG_TDLS
	padapter->tdlsinfo.ap_prohibited = false;

	/* For TDLS channel switch, currently we only allow it to work in wifi logo test mode */
	if (padapter->registrypriv.wifi_spec == 1)
		padapter->tdlsinfo.ch_switch_prohibited = false;
#endif /* CONFIG_TDLS */

}

void mlmeext_joinbss_event_callback(_adapter *padapter, int join_res)
{
	struct sta_info		*psta, *psta_bmc;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*cur_network = &(pmlmeinfo->network);
	struct sta_priv		*pstapriv = &padapter->stapriv;
	u8	join_type;
#ifdef CONFIG_ARP_KEEP_ALIVE
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
#endif
	struct security_priv *psecuritypriv = &padapter->securitypriv;

	if (join_res < 0) {
		join_type = 1;
		rtw_hal_set_hwreg(padapter, HW_VAR_MLME_JOIN, (u8 *)(&join_type));
		rtw_hal_set_hwreg(padapter, HW_VAR_BSSID, null_addr);

		goto exit_mlmeext_joinbss_event_callback;
	}
#ifdef CONFIG_ARP_KEEP_ALIVE
	pmlmepriv->bGetGateway = 1;
	pmlmepriv->GetGatewayTryCnt = 0;
#endif

	if ((pmlmeinfo->state & 0x03) == WIFI_FW_ADHOC_STATE) {
		/* update bc/mc sta_info */
		update_bmc_sta(padapter);
	}


	/* turn on dynamic functions */
	/* Switch_DM_Func(padapter, DYNAMIC_ALL_FUNC_ENABLE, true); */

	/* update IOT-releated issue */
	update_IOT_info(padapter);

	rtw_hal_set_hwreg(padapter, HW_VAR_BASIC_RATE, cur_network->SupportedRates);

	/* BCN interval */
	rtw_hal_set_hwreg(padapter, HW_VAR_BEACON_INTERVAL, (u8 *)(&pmlmeinfo->bcn_interval));

	/* udpate capability */
	update_capinfo(padapter, pmlmeinfo->capability);

	/* WMM, Update EDCA param */
	WMMOnAssocRsp(padapter);

	/* HT */
	HTOnAssocRsp(padapter);

	psta = rtw_get_stainfo(pstapriv, cur_network->MacAddress);
	if (psta) { /* only for infra. mode */
		/* RTW_INFO("set_sta_rate\n"); */

		psta->wireless_mode = pmlmeext->cur_wireless_mode;

#ifdef CONFIG_FW_MULTI_PORT_SUPPORT
		rtw_hal_set_default_port_id_cmd(padapter, psta->mac_id);
#endif

		/* set per sta rate after updating HT cap. */
		set_sta_rate(padapter, psta);

		rtw_sta_media_status_rpt(padapter, psta, 1);

		/* wakeup macid after join bss successfully to ensure
			the subsequent data frames can be sent out normally */
		rtw_hal_macid_wakeup(padapter, psta->mac_id);
	}

#ifndef CONFIG_IOCTL_CFG80211
	if (is_wep_enc(psecuritypriv->dot11PrivacyAlgrthm))
		rtw_sec_restore_wep_key(padapter);
#endif /* CONFIG_IOCTL_CFG80211 */

	if (rtw_port_switch_chk(padapter))
		rtw_hal_set_hwreg(padapter, HW_VAR_PORT_SWITCH, NULL);

	join_type = 2;
	rtw_hal_set_hwreg(padapter, HW_VAR_MLME_JOIN, (u8 *)(&join_type));

	if ((pmlmeinfo->state & 0x03) == WIFI_FW_STATION_STATE) {
		/* correcting TSF */
		correct_TSF(padapter, pmlmeext);

		/* set_link_timer(pmlmeext, DISCONNECT_TO); */
	}

#ifdef CONFIG_LPS
	#ifndef CONFIG_FW_MULTI_PORT_SUPPORT
	if (get_hw_port(padapter) == HW_PORT0)
	#endif
		rtw_lps_ctrl_wk_cmd(padapter, LPS_CTRL_CONNECT, 0);
#endif

#ifdef CONFIG_BEAMFORMING
	if (psta)
		beamforming_wk_cmd(padapter, BEAMFORMING_CTRL_ENTER, (u8 *)psta, sizeof(struct sta_info), 0);
#endif/*CONFIG_BEAMFORMING*/

exit_mlmeext_joinbss_event_callback:

	rtw_join_done_chk_ch(padapter, join_res);

	RTW_INFO("=>%s - End to Connection without 4-way\n", __func__);
}

/* currently only adhoc mode will go here */
void mlmeext_sta_add_event_callback(_adapter *padapter, struct sta_info *psta)
{
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8	join_type;

	RTW_INFO("%s\n", __func__);

	if ((pmlmeinfo->state & 0x03) == WIFI_FW_ADHOC_STATE) {
		if (pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS) { /* adhoc master or sta_count>1 */
			/* nothing to do */
		} else { /* adhoc client */
			/* update TSF Value */
			/* update_TSF(pmlmeext, pframe, len);			 */

			/* correcting TSF */
			correct_TSF(padapter, pmlmeext);

			/* start beacon */
			if (send_beacon(padapter) == _FAIL)
				rtw_warn_on(1);

			pmlmeinfo->state |= WIFI_FW_ASSOC_SUCCESS;
		}

		join_type = 2;
		rtw_hal_set_hwreg(padapter, HW_VAR_MLME_JOIN, (u8 *)(&join_type));
	}

	/* update adhoc sta_info */
	update_sta_info(padapter, psta);

	rtw_hal_update_sta_rate_mask(padapter, psta);

	/* ToDo: HT for Ad-hoc */
	psta->wireless_mode = rtw_check_network_type(psta->bssrateset, psta->bssratelen, pmlmeext->cur_channel);
	psta->raid = rtw_hal_networktype_to_raid(padapter, psta);

	/* rate radaptive */
	Update_RA_Entry(padapter, psta);
}

void mlmeext_sta_del_event_callback(_adapter *padapter)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (is_client_associated_to_ap(padapter) || is_IBSS_empty(padapter))
		rtw_mlmeext_disconnect(padapter);

}

/****************************************************************************

Following are the functions for the timer handlers

*****************************************************************************/
void _linked_info_dump(_adapter *padapter)
{
	int i;
	struct mlme_ext_priv    *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info    *pmlmeinfo = &(pmlmeext->mlmext_info);
	HAL_DATA_TYPE *HalData = GET_HAL_DATA(padapter);
	int undecorated_smoothed_pwdb = 0;

	if (padapter->bLinkInfoDump) {

		RTW_INFO("\n============["ADPT_FMT"] linked status check ===================\n", ADPT_ARG(padapter));

		if ((pmlmeinfo->state & 0x03) == WIFI_FW_STATION_STATE) {
			rtw_hal_get_def_var(padapter, HAL_DEF_UNDERCORATEDSMOOTHEDPWDB, &undecorated_smoothed_pwdb);

			RTW_INFO("AP[" MAC_FMT "] - undecorated_smoothed_pwdb:%d\n",
				MAC_ARG(padapter->mlmepriv.cur_network.network.MacAddress), undecorated_smoothed_pwdb);
		} else if ((pmlmeinfo->state & 0x03) == _HW_STATE_AP_) {
			unsigned long irqL;
			_list	*phead, *plist;

			struct sta_info *psta = NULL;
			struct sta_priv *pstapriv = &padapter->stapriv;

			_enter_critical_bh(&pstapriv->asoc_list_lock, &irqL);
			phead = &pstapriv->asoc_list;
			plist = get_next(phead);
			while ((rtw_end_of_queue_search(phead, plist)) == false) {
				psta = LIST_CONTAINOR(plist, struct sta_info, asoc_list);
				plist = get_next(plist);

				RTW_INFO("STA[" MAC_FMT "]:undecorated_smoothed_pwdb:%d\n",
					MAC_ARG(psta->hwaddr), psta->rssi_stat.undecorated_smoothed_pwdb);
			}
			_exit_critical_bh(&pstapriv->asoc_list_lock, &irqL);

		}

		/*============  tx info ============	*/
		rtw_hal_get_def_var(padapter, HW_DEF_RA_INFO_DUMP, RTW_DBGDUMP);

		rtw_hal_set_odm_var(padapter, HAL_ODM_RX_INFO_DUMP, RTW_DBGDUMP, false);

	}


}
/********************************************************************

When station does not receive any packet in MAX_CONTINUAL_NORXPACKET_COUNT*2 seconds,
recipient station will teardown the block ack by issuing DELBA frame.

*********************************************************************/
static void rtw_delba_check(_adapter *padapter, struct sta_info *psta, u8 from_timer)
{
	int	i = 0;
	int ret = _SUCCESS;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	/*
		IOT issue,occur Broadcom ap(Buffalo WZR-D1800H,Netgear R6300).
		AP is originator.AP does not transmit unicast packets when STA response its BAR.
		This case probably occur ap issue BAR after AP builds BA.

		Follow 802.11 spec, STA shall maintain an inactivity timer for every negotiated Block Ack setup.
		The inactivity timer is not reset when MPDUs corresponding to other TIDs are received.
	*/
	if (pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_BROADCOM) {
		for (i = 0; i < TID_NUM ; i++) {
			if ((psta->recvreorder_ctrl[i].enable) && 
                        (sta_rx_data_qos_pkts(psta, i) == sta_last_rx_data_qos_pkts(psta, i)) ) {			
					if (rtw_inc_and_chk_continual_no_rx_packet(psta, i)) {					
						/* send a DELBA frame to the peer STA with the Reason Code field set to TIMEOUT */
						if (!from_timer)
							ret = issue_del_ba_ex(padapter, psta->hwaddr, i, 39, 0, 3, 1);
						else
							issue_del_ba(padapter,  psta->hwaddr, i, 39, 0);
						psta->recvreorder_ctrl[i].enable = false;
						if (ret != _FAIL)
							psta->recvreorder_ctrl[i].ampdu_size = RX_AMPDU_SIZE_INVALID;
						rtw_reset_continual_no_rx_packet(psta, i);
					}				
			} else {
				/* The inactivity timer is reset when MPDUs to the TID is received. */
				rtw_reset_continual_no_rx_packet(psta, i);
			}
		}
	}
}

static u8 chk_ap_is_alive(_adapter *padapter, struct sta_info *psta)
{
	u8 ret = false;
	int i = 0;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

#ifdef DBG_EXPIRATION_CHK
	RTW_INFO(FUNC_ADPT_FMT" rx:"STA_PKTS_FMT", beacon:%llu, probersp_to_self:%llu"
		/*", probersp_bm:%llu, probersp_uo:%llu, probereq:%llu, BI:%u"*/
		 ", retry:%u\n"
		 , FUNC_ADPT_ARG(padapter)
		 , STA_RX_PKTS_DIFF_ARG(psta)
		, psta->sta_stats.rx_beacon_pkts - psta->sta_stats.last_rx_beacon_pkts
		, psta->sta_stats.rx_probersp_pkts - psta->sta_stats.last_rx_probersp_pkts
		/*, psta->sta_stats.rx_probersp_bm_pkts - psta->sta_stats.last_rx_probersp_bm_pkts
		, psta->sta_stats.rx_probersp_uo_pkts - psta->sta_stats.last_rx_probersp_uo_pkts
		, psta->sta_stats.rx_probereq_pkts - psta->sta_stats.last_rx_probereq_pkts
		 , pmlmeinfo->bcn_interval*/
		 , pmlmeext->retry
		);

	RTW_INFO(FUNC_ADPT_FMT" tx_pkts:%llu, link_count:%u\n", FUNC_ADPT_ARG(padapter)
		 , padapter->xmitpriv.tx_pkts
		 , pmlmeinfo->link_count
		);
#endif

	if ((sta_rx_data_pkts(psta) == sta_last_rx_data_pkts(psta))
	    && sta_rx_beacon_pkts(psta) == sta_last_rx_beacon_pkts(psta)
	    && sta_rx_probersp_pkts(psta) == sta_last_rx_probersp_pkts(psta)
	   )
		ret = false;
	else
		ret = true;

	sta_update_last_rx_pkts(psta);

	/*
		record last rx data packets for every tid.
	*/
	for (i = 0; i < TID_NUM; i++)
		psta->sta_stats.last_rx_data_qos_pkts[i] = psta->sta_stats.rx_data_qos_pkts[i];

	return ret;
}

static u8 chk_adhoc_peer_is_alive(struct sta_info *psta)
{
	u8 ret = true;

#ifdef DBG_EXPIRATION_CHK
	RTW_INFO("sta:"MAC_FMT", rssi:%d, rx:"STA_PKTS_FMT", beacon:%llu, probersp_to_self:%llu"
		/*", probersp_bm:%llu, probersp_uo:%llu, probereq:%llu, BI:%u"*/
		 ", expire_to:%u\n"
		 , MAC_ARG(psta->hwaddr)
		 , psta->rssi_stat.undecorated_smoothed_pwdb
		 , STA_RX_PKTS_DIFF_ARG(psta)
		, psta->sta_stats.rx_beacon_pkts - psta->sta_stats.last_rx_beacon_pkts
		, psta->sta_stats.rx_probersp_pkts - psta->sta_stats.last_rx_probersp_pkts
		/*, psta->sta_stats.rx_probersp_bm_pkts - psta->sta_stats.last_rx_probersp_bm_pkts
		, psta->sta_stats.rx_probersp_uo_pkts - psta->sta_stats.last_rx_probersp_uo_pkts
		, psta->sta_stats.rx_probereq_pkts - psta->sta_stats.last_rx_probereq_pkts
		 , pmlmeinfo->bcn_interval*/
		 , psta->expire_to
		);
#endif

	if (sta_rx_data_pkts(psta) == sta_last_rx_data_pkts(psta)
	    && sta_rx_beacon_pkts(psta) == sta_last_rx_beacon_pkts(psta)
	    && sta_rx_probersp_pkts(psta) == sta_last_rx_probersp_pkts(psta))
		ret = false;

	sta_update_last_rx_pkts(psta);

	return ret;
}

#ifdef CONFIG_TDLS
u8 chk_tdls_peer_sta_is_alive(_adapter *padapter, struct sta_info *psta)
{
	if ((psta->sta_stats.rx_data_pkts == psta->sta_stats.last_rx_data_pkts)
	    && (psta->sta_stats.rx_tdls_disc_rsp_pkts == psta->sta_stats.last_rx_tdls_disc_rsp_pkts))
		return false;

	return true;
}

void linked_status_chk_tdls(_adapter *padapter)
{
	struct candidate_pool {
		struct sta_info *psta;
		u8 addr[ETH_ALEN];
	};
	struct sta_priv *pstapriv = &padapter->stapriv;
	unsigned long irqL;
	u8 ack_chk;
	struct sta_info *psta;
	int i, num_teardown = 0, num_checkalive = 0;
	_list	*plist, *phead;
	struct tdls_txmgmt txmgmt;
	struct candidate_pool checkalive[MAX_ALLOWED_TDLS_STA_NUM];
	struct candidate_pool teardown[MAX_ALLOWED_TDLS_STA_NUM];
	u8 tdls_sta_max = false;

#define ALIVE_MIN 2
#define ALIVE_MAX 5

	memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
	memset(checkalive, 0x00, sizeof(checkalive));
	memset(teardown, 0x00, sizeof(teardown));

	if ((padapter->tdlsinfo.link_established)) {
		_enter_critical_bh(&pstapriv->sta_hash_lock, &irqL);
		for (i = 0; i < NUM_STA; i++) {
			phead = &(pstapriv->sta_hash[i]);
			plist = get_next(phead);

			while ((rtw_end_of_queue_search(phead, plist)) == false) {
				psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
				plist = get_next(plist);

				if (psta->tdls_sta_state & TDLS_LINKED_STATE) {
					psta->alive_count++;
					if (psta->alive_count >= ALIVE_MIN) {
						if (chk_tdls_peer_sta_is_alive(padapter, psta) == false) {
							if (psta->alive_count < ALIVE_MAX) {
								memcpy(checkalive[num_checkalive].addr, psta->hwaddr, ETH_ALEN);
								checkalive[num_checkalive].psta = psta;
								num_checkalive++;
							} else {
								memcpy(teardown[num_teardown].addr, psta->hwaddr, ETH_ALEN);
								teardown[num_teardown].psta = psta;
								num_teardown++;
							}
						} else
							psta->alive_count = 0;
					}
					psta->sta_stats.last_rx_data_pkts = psta->sta_stats.rx_data_pkts;
					psta->sta_stats.last_rx_tdls_disc_rsp_pkts = psta->sta_stats.rx_tdls_disc_rsp_pkts;

					if ((num_checkalive >= MAX_ALLOWED_TDLS_STA_NUM) || (num_teardown >= MAX_ALLOWED_TDLS_STA_NUM)) {
						tdls_sta_max = true;
						break;
					}
				}
			}

			if (tdls_sta_max)
				break;
		}
		_exit_critical_bh(&pstapriv->sta_hash_lock, &irqL);

		if (num_checkalive > 0) {
			for (i = 0; i < num_checkalive; i++) {
				memcpy(txmgmt.peer, checkalive[i].addr, ETH_ALEN);
				issue_tdls_dis_req(padapter, &txmgmt);
				issue_tdls_dis_req(padapter, &txmgmt);
				issue_tdls_dis_req(padapter, &txmgmt);
			}
		}

		if (num_teardown > 0) {
			for (i = 0; i < num_teardown; i++) {
				RTW_INFO("[%s %d] Send teardown to "MAC_FMT"\n", __func__, __LINE__, MAC_ARG(teardown[i].addr));
				txmgmt.status_code = _RSON_TDLS_TEAR_TOOFAR_;
				memcpy(txmgmt.peer, teardown[i].addr, ETH_ALEN);
				issue_tdls_teardown(padapter, &txmgmt, false);
			}
		}
	}

}
#endif /* CONFIG_TDLS */

/* from_timer == 1 means driver is in LPS */
void linked_status_chk(_adapter *padapter, u8 from_timer)
{
	u32	i;
	struct sta_info		*psta;
	struct xmit_priv		*pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct sta_priv		*pstapriv = &padapter->stapriv;
#if defined(CONFIG_ARP_KEEP_ALIVE) || defined(CONFIG_LAYER2_ROAMING)
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
#endif
#ifdef CONFIG_LAYER2_ROAMING
	struct recv_priv	*precvpriv = &padapter->recvpriv;
#endif

	if (padapter->registrypriv.mp_mode)
		return;

	if (is_client_associated_to_ap(padapter)) {
		/* linked infrastructure client mode */

		int tx_chk = _SUCCESS, rx_chk = _SUCCESS;
		int rx_chk_limit;
		int link_count_limit;

#ifdef CONFIG_LAYER2_ROAMING
		if (rtw_chk_roam_flags(padapter, RTW_ROAM_ACTIVE)) {
			RTW_INFO("signal_strength_data.avg_val = %d\n", precvpriv->signal_strength_data.avg_val);
			if (precvpriv->signal_strength_data.avg_val < pmlmepriv->roam_rssi_threshold) {
				pmlmepriv->need_to_roam = true;
				rtw_drv_scan_by_self(padapter, RTW_AUTO_SCAN_REASON_ROAM);
			} else {
				pmlmepriv->need_to_roam = false;
			}
		}
#endif
#ifdef CONFIG_MCC_MODE
		/*
		 * due to tx ps null date to ao, so ap doest not tx pkt to driver
		 * we may check chk_ap_is_alive fail, and may issue_probereq to wrong channel under sitesurvey
		 * don't keep alive check under MCC
		 */
		if (rtw_hal_mcc_link_status_chk(padapter, __func__) == false)
			return;
#endif

#if defined(DBG_ROAMING_TEST)
		rx_chk_limit = 1;
#elif defined(CONFIG_ACTIVE_KEEP_ALIVE_CHECK) && !defined(CONFIG_LPS_LCLK_WD_TIMER)
		rx_chk_limit = 4;
#else
		rx_chk_limit = 8;
#endif
#ifdef CONFIG_ARP_KEEP_ALIVE
		if (!from_timer && pmlmepriv->bGetGateway == 1 && pmlmepriv->GetGatewayTryCnt < 3) {
			RTW_INFO("do rtw_gw_addr_query() : %d\n", pmlmepriv->GetGatewayTryCnt);
			pmlmepriv->GetGatewayTryCnt++;
			if (rtw_gw_addr_query(padapter) == 0)
				pmlmepriv->bGetGateway = 0;
			else {
				memset(pmlmepriv->gw_ip, 0, 4);
				memset(pmlmepriv->gw_mac_addr, 0, 6);
			}
		}
#endif
#ifdef CONFIG_P2P
		if (!rtw_p2p_chk_state(&padapter->wdinfo, P2P_STATE_NONE)) {
			if (!from_timer)
				link_count_limit = 3; /* 8 sec */
			else
				link_count_limit = 15; /* 32 sec */
		} else
#endif /* CONFIG_P2P */
		{
			if (!from_timer)
				link_count_limit = 7; /* 16 sec */
			else
				link_count_limit = 29; /* 60 sec */
		}

#ifdef CONFIG_TDLS
#ifdef CONFIG_TDLS_CH_SW
		if (ATOMIC_READ(&padapter->tdlsinfo.chsw_info.chsw_on))
			return;
#endif /* CONFIG_TDLS_CH_SW */

#ifdef CONFIG_TDLS_AUTOCHECKALIVE
		linked_status_chk_tdls(padapter);
#endif /* CONFIG_TDLS_AUTOCHECKALIVE */
#endif /* CONFIG_TDLS */

		psta = rtw_get_stainfo(pstapriv, pmlmeinfo->network.MacAddress);
		if (psta != NULL) {
			bool is_p2p_enable = false;
#ifdef CONFIG_P2P
			is_p2p_enable = !rtw_p2p_chk_state(&padapter->wdinfo, P2P_STATE_NONE);
#endif

#ifdef CONFIG_ISSUE_DELBA_WHEN_NO_TRAFFIC 
			/*issue delba when ap does not tx data packet that is Broadcom ap */
			rtw_delba_check(padapter, psta, from_timer);
#endif
			if (chk_ap_is_alive(padapter, psta) == false)
				rx_chk = _FAIL;

			if (pxmitpriv->last_tx_pkts == pxmitpriv->tx_pkts)
				tx_chk = _FAIL;

#if defined(CONFIG_ACTIVE_KEEP_ALIVE_CHECK) && !defined(CONFIG_LPS_LCLK_WD_TIMER)
			if (pmlmeext->active_keep_alive_check && (rx_chk == _FAIL || tx_chk == _FAIL)
				#ifdef CONFIG_MCC_MODE
				/* Driver don't know operation channel under MCC*/
				/* So driver don't  do KEEP_ALIVE_CHECK */
				&& (!rtw_hal_check_mcc_status(padapter, MCC_STATUS_NEED_MCC))
				#endif
			) {
				u8 backup_ch = 0, backup_bw, backup_offset;
				u8 union_ch = 0, union_bw, union_offset;

				if (!rtw_mi_get_ch_setting_union(padapter, &union_ch, &union_bw, &union_offset)
					|| pmlmeext->cur_channel != union_ch)
						goto bypass_active_keep_alive;

				/* switch to correct channel of current network  before issue keep-alive frames */
				if (rtw_get_oper_ch(padapter) != pmlmeext->cur_channel) {
					backup_ch = rtw_get_oper_ch(padapter);
					backup_bw = rtw_get_oper_bw(padapter);
					backup_offset = rtw_get_oper_choffset(padapter);
					set_channel_bwmode(padapter, union_ch, union_offset, union_bw);
				}

				if (rx_chk != _SUCCESS)
					issue_probereq_ex(padapter, &pmlmeinfo->network.Ssid, psta->hwaddr, 0, 0, 3, 1);

				if ((tx_chk != _SUCCESS && pmlmeinfo->link_count++ == link_count_limit) || rx_chk != _SUCCESS) {
					tx_chk = issue_nulldata(padapter, psta->hwaddr, 0, 3, 1);
					/* if tx acked and p2p disabled, set rx_chk _SUCCESS to reset retry count */
					if (tx_chk == _SUCCESS && !is_p2p_enable)
						rx_chk = _SUCCESS;
				}

				/* back to the original operation channel */
				if (backup_ch > 0)
					set_channel_bwmode(padapter, backup_ch, backup_offset, backup_bw);

bypass_active_keep_alive:
				;
			} else
#endif /* CONFIG_ACTIVE_KEEP_ALIVE_CHECK */
			{
				if (rx_chk != _SUCCESS) {
					if (pmlmeext->retry == 0) {
#ifdef DBG_EXPIRATION_CHK
						RTW_INFO("issue_probereq to trigger probersp, retry=%d\n", pmlmeext->retry);
#endif
						issue_probereq_ex(padapter, &pmlmeinfo->network.Ssid, pmlmeinfo->network.MacAddress, 0, 0, 0, 0);
						issue_probereq_ex(padapter, &pmlmeinfo->network.Ssid, pmlmeinfo->network.MacAddress, 0, 0, 0, 0);
						issue_probereq_ex(padapter, &pmlmeinfo->network.Ssid, pmlmeinfo->network.MacAddress, 0, 0, 0, 0);
					}
				}

				if (tx_chk != _SUCCESS && pmlmeinfo->link_count++ == link_count_limit
#ifdef CONFIG_MCC_MODE
				    /* FW tx nulldata under MCC mode, we just check  ap is alive */
				    && (!rtw_hal_check_mcc_status(padapter, MCC_STATUS_NEED_MCC))
#endif /* CONFIG_MCC_MODE */
				   ) {
#ifdef DBG_EXPIRATION_CHK
					RTW_INFO("%s issue_nulldata(%d)\n", __func__, from_timer ? 1 : 0);
#endif
					tx_chk = issue_nulldata_in_interrupt(padapter, NULL, from_timer ? 1 : 0);
				}
			}

			if (rx_chk == _FAIL) {
				pmlmeext->retry++;
				if (pmlmeext->retry > rx_chk_limit) {
					RTW_PRINT(FUNC_ADPT_FMT" disconnect or roaming\n",
						  FUNC_ADPT_ARG(padapter));
					receive_disconnect(padapter, pmlmeinfo->network.MacAddress
						, WLAN_REASON_EXPIRATION_CHK, false);
					return;
				}
			} else
				pmlmeext->retry = 0;

			if (tx_chk == _FAIL)
				pmlmeinfo->link_count %= (link_count_limit + 1);
			else {
				pxmitpriv->last_tx_pkts = pxmitpriv->tx_pkts;
				pmlmeinfo->link_count = 0;
			}

		} /* end of if ((psta = rtw_get_stainfo(pstapriv, passoc_res->network.MacAddress)) != NULL) */

	} else if (is_client_associated_to_ibss(padapter)) {
		unsigned long irqL;
		_list *phead, *plist, dlist;

		INIT_LIST_HEAD(&dlist);

		_enter_critical_bh(&pstapriv->sta_hash_lock, &irqL);

		for (i = 0; i < NUM_STA; i++) {

			phead = &(pstapriv->sta_hash[i]);
			plist = get_next(phead);
			while ((rtw_end_of_queue_search(phead, plist)) == false) {
				psta = LIST_CONTAINOR(plist, struct sta_info, hash_list);
				plist = get_next(plist);

				if (is_broadcast_mac_addr(psta->hwaddr))
					continue;

				if (chk_adhoc_peer_is_alive(psta) || !psta->expire_to)
					psta->expire_to = pstapriv->adhoc_expire_to;
				else
					psta->expire_to--;

				if (psta->expire_to <= 0) {
					list_del_init(&psta->list);
					list_add_tail(&psta->list, &dlist);
				}
			}
		}

		_exit_critical_bh(&pstapriv->sta_hash_lock, &irqL);

		plist = get_next(&dlist);
		while (rtw_end_of_queue_search(&dlist, plist) == false) {
			psta = LIST_CONTAINOR(plist, struct sta_info, list);
			plist = get_next(plist);
			list_del_init(&psta->list);
			RTW_INFO(FUNC_ADPT_FMT" ibss expire "MAC_FMT"\n"
				, FUNC_ADPT_ARG(padapter), MAC_ARG(psta->hwaddr));
			report_del_sta_event(padapter, psta->hwaddr, WLAN_REASON_EXPIRATION_CHK, from_timer ? true : false, false);
		}
	}

}

void survey_timer_hdl(_adapter *padapter)
{
	struct cmd_obj *cmd;
	struct sitesurvey_parm *psurveyPara;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
#ifdef CONFIG_P2P
	struct wifidirect_info *pwdinfo = &(padapter->wdinfo);
#endif

	if (mlmeext_scan_state(pmlmeext) > SCAN_DISABLE) {
		cmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
		if (cmd == NULL) {
			rtw_warn_on(1);
			goto exit;
		}

		psurveyPara = (struct sitesurvey_parm *)rtw_zmalloc(sizeof(struct sitesurvey_parm));
		if (psurveyPara == NULL) {
			rtw_warn_on(1);
			rtw_mfree((unsigned char *)cmd, sizeof(struct cmd_obj));
			goto exit;
		}

		init_h2fwcmd_w_parm_no_rsp(cmd, psurveyPara, GEN_CMD_CODE(_SiteSurvey));
		rtw_enqueue_cmd(pcmdpriv, cmd);
	}

exit:
	return;
}

void link_timer_hdl(_adapter *padapter)
{
	/* static unsigned int		rx_pkt = 0; */
	/* static u64				tx_cnt = 0; */
	/* struct xmit_priv		*pxmitpriv = &(padapter->xmitpriv); */
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	/* struct sta_priv		*pstapriv = &padapter->stapriv; */
#ifdef CONFIG_RTW_80211R
	struct	sta_priv		*pstapriv = &padapter->stapriv;
	struct	sta_info		*psta = NULL;
	WLAN_BSSID_EX		*pnetwork = (WLAN_BSSID_EX *)(&(pmlmeinfo->network));
#endif

	if (pmlmeinfo->state & WIFI_FW_AUTH_NULL) {
		RTW_INFO("link_timer_hdl:no beacon while connecting\n");
		pmlmeinfo->state = WIFI_FW_NULL_STATE;
		report_join_res(padapter, -3);
	} else if (pmlmeinfo->state & WIFI_FW_AUTH_STATE) {
		/* re-auth timer */
		if (++pmlmeinfo->reauth_count > REAUTH_LIMIT) {
			/* if (pmlmeinfo->auth_algo != dot11AuthAlgrthm_Auto) */
			/* { */
			pmlmeinfo->state = 0;
			report_join_res(padapter, -1);
			return;
			/* } */
			/* else */
			/* { */
			/*	pmlmeinfo->auth_algo = dot11AuthAlgrthm_Shared; */
			/*	pmlmeinfo->reauth_count = 0; */
			/* } */
		}

		RTW_INFO("link_timer_hdl: auth timeout and try again\n");
		pmlmeinfo->auth_seq = 1;
		issue_auth(padapter, NULL, 0);
		set_link_timer(pmlmeext, REAUTH_TO);
	} else if (pmlmeinfo->state & WIFI_FW_ASSOC_STATE) {
		/* re-assoc timer */
		if (++pmlmeinfo->reassoc_count > REASSOC_LIMIT) {
			pmlmeinfo->state = WIFI_FW_NULL_STATE;
#ifdef CONFIG_RTW_80211R
			if ((rtw_to_roam(padapter) > 0) && rtw_chk_ft_flags(padapter, RTW_FT_SUPPORTED)) {
				psta = rtw_get_stainfo(pstapriv, pmlmeinfo->network.MacAddress);
				if (psta)
					rtw_free_stainfo(padapter,  psta);
			}
#endif
			report_join_res(padapter, -2);
			return;
		}

#ifdef CONFIG_RTW_80211R
		if ((rtw_to_roam(padapter) > 0) && rtw_chk_ft_flags(padapter, RTW_FT_SUPPORTED)) {
			RTW_INFO("link_timer_hdl: reassoc timeout and try again\n");
			issue_reassocreq(padapter);
		} else
#endif
		{
			RTW_INFO("link_timer_hdl: assoc timeout and try again\n");
			issue_assocreq(padapter);
		}

		set_link_timer(pmlmeext, REASSOC_TO);
	}

	return;
}

void addba_timer_hdl(struct sta_info *psta)
{
	struct ht_priv	*phtpriv;

	if (!psta)
		return;

	phtpriv = &psta->htpriv;

	if ((phtpriv->ht_option) && (phtpriv->ampdu_enable == true)) {
		if (phtpriv->candidate_tid_bitmap)
			phtpriv->candidate_tid_bitmap = 0x0;

	}
}

#ifdef CONFIG_IEEE80211W
void report_sta_timeout_event(_adapter *padapter, u8 *MacAddr, unsigned short reason)
{
	struct cmd_obj *pcmd_obj;
	u8	*pevtcmd;
	u32 cmdsz;
	struct sta_info *psta;
	int	mac_id;
	struct stadel_event			*pdel_sta_evt;
	struct C2HEvent_Header	*pc2h_evt_hdr;
	struct mlme_ext_priv		*pmlmeext = &padapter->mlmeextpriv;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	pcmd_obj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmd_obj == NULL)
		return;

	cmdsz = (sizeof(struct stadel_event) + sizeof(struct C2HEvent_Header));
	pevtcmd = (u8 *)rtw_zmalloc(cmdsz);
	if (pevtcmd == NULL) {
		rtw_mfree((u8 *)pcmd_obj, sizeof(struct cmd_obj));
		return;
	}

	INIT_LIST_HEAD(&pcmd_obj->list);

	pcmd_obj->cmdcode = GEN_CMD_CODE(_Set_MLME_EVT);
	pcmd_obj->cmdsz = cmdsz;
	pcmd_obj->parmbuf = pevtcmd;

	pcmd_obj->rsp = NULL;
	pcmd_obj->rspsz  = 0;

	pc2h_evt_hdr = (struct C2HEvent_Header *)(pevtcmd);
	pc2h_evt_hdr->len = sizeof(struct stadel_event);
	pc2h_evt_hdr->ID = GEN_EVT_CODE(_TimeoutSTA);
	pc2h_evt_hdr->seq = ATOMIC_INC_RETURN(&pmlmeext->event_seq);

	pdel_sta_evt = (struct stadel_event *)(pevtcmd + sizeof(struct C2HEvent_Header));
	memcpy((unsigned char *)(&(pdel_sta_evt->macaddr)), MacAddr, ETH_ALEN);
	memcpy((unsigned char *)(pdel_sta_evt->rsvd), (unsigned char *)(&reason), 2);


	psta = rtw_get_stainfo(&padapter->stapriv, MacAddr);
	if (psta)
		mac_id = (int)psta->mac_id;
	else
		mac_id = (-1);

	pdel_sta_evt->mac_id = mac_id;

	RTW_INFO("report_del_sta_event: delete STA, mac_id=%d\n", mac_id);

	rtw_enqueue_cmd(pcmdpriv, pcmd_obj);

	return;
}

void clnt_sa_query_timeout(_adapter *padapter)
{

	rtw_disassoc_cmd(padapter, 0, true);
	rtw_indicate_disconnect(padapter, 0, false);
	rtw_free_assoc_resources(padapter, 1);

	RTW_INFO("SA query timeout client disconnect\n");
}

void sa_query_timer_hdl(struct sta_info *psta)
{
	_adapter *padapter = psta->padapter;
	unsigned long irqL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	if (check_fwstate(pmlmepriv, WIFI_STATION_STATE) &&
	    check_fwstate(pmlmepriv, _FW_LINKED))
		clnt_sa_query_timeout(padapter);
	else if (check_fwstate(pmlmepriv, WIFI_AP_STATE))
		report_sta_timeout_event(padapter, psta->hwaddr, WLAN_REASON_PREV_AUTH_NOT_VALID);
}

#endif /* CONFIG_IEEE80211W */

#ifdef CONFIG_RTW_80211R
void start_clnt_ft_action(_adapter *padapter, u8 *pTargetAddr)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	rtw_set_ft_status(padapter, RTW_FT_REQUESTING_STA);
	issue_action_ft_request(padapter, pTargetAddr);
	_set_timer(&pmlmeext->ft_link_timer, REASSOC_TO);
}

void ft_link_timer_hdl(_adapter *padapter)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct	mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	ft_priv		*pftpriv = &pmlmepriv->ftpriv;

	if (rtw_chk_ft_status(padapter, RTW_FT_REQUESTING_STA)) {
		if (pftpriv->ft_req_retry_cnt < FT_ACTION_REQ_LIMIT) {
			pftpriv->ft_req_retry_cnt++;
			issue_action_ft_request(padapter, (u8 *)pmlmepriv->roam_network->network.MacAddress);
			_set_timer(&pmlmeext->ft_link_timer, REASSOC_TO);
		} else {
			pftpriv->ft_req_retry_cnt = 0;

			if (pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS)
				rtw_set_ft_status(padapter, RTW_FT_ASSOCIATED_STA);
			else
				rtw_reset_ft_status(padapter);
		}
	}
}

void ft_roam_timer_hdl(_adapter *padapter)
{
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);

	receive_disconnect(padapter, pmlmepriv->cur_network.network.MacAddress
							, WLAN_REASON_ACTIVE_ROAM, false);
}

void issue_action_ft_request(_adapter *padapter, u8 *pTargetAddr)
{
	struct mlme_priv	*pmlmepriv = &(padapter->mlmepriv);
	struct xmit_priv *pxmitpriv = &(padapter->xmitpriv);
	struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	struct xmit_frame *pmgntframe = NULL;
	struct rtw_ieee80211_hdr *pwlanhdr = NULL;
	struct pkt_attrib *pattrib = NULL;
	ft_priv *pftpriv = NULL;
	u8 *pframe = NULL;
	u8 category = RTW_WLAN_CATEGORY_FT;
	u8 action = RTW_WLAN_ACTION_FT_REQUEST;
	u8 is_ft_roaming_with_rsn_ie = true;
	u8 *pie = NULL;
	__le16 *fctrl;
	u32 ft_ie_len = 0;

	pmgntframe = alloc_mgtxmitframe(pxmitpriv);
	if (pmgntframe == NULL)
		return;

	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);
	memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	memcpy(pwlanhdr->addr1, get_my_bssid(&pmlmeinfo->network), ETH_ALEN);
	memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	memcpy(pwlanhdr->addr3, get_my_bssid(&pmlmeinfo->network), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	set_frame_sub_type(pframe, WIFI_ACTION);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	pframe = rtw_set_fixed_ie(pframe, 1, &(category), &(pattrib->pktlen));
	pframe = rtw_set_fixed_ie(pframe, 1, &(action), &(pattrib->pktlen));

	memcpy(pframe, adapter_mac_addr(padapter), ETH_ALEN);
	pframe += ETH_ALEN;
	pattrib->pktlen += ETH_ALEN;

	memcpy(pframe, pTargetAddr, ETH_ALEN);
	pframe += ETH_ALEN;
	pattrib->pktlen += ETH_ALEN;

	pftpriv = &pmlmepriv->ftpriv;
	pie = rtw_get_ie(pftpriv->updated_ft_ies, EID_WPA2, &ft_ie_len, pftpriv->updated_ft_ies_len);
	if (pie)
		pframe = rtw_set_ie(pframe, EID_WPA2, ft_ie_len, pie+2, &(pattrib->pktlen));
	else
		is_ft_roaming_with_rsn_ie = false;

	pie = rtw_get_ie(pftpriv->updated_ft_ies, _MDIE_, &ft_ie_len, pftpriv->updated_ft_ies_len);
	if (pie)
		pframe = rtw_set_ie(pframe, _MDIE_, ft_ie_len , pie+2, &(pattrib->pktlen));

	pie = rtw_get_ie(pftpriv->updated_ft_ies, _FTIE_, &ft_ie_len, pftpriv->updated_ft_ies_len);
	if (pie && is_ft_roaming_with_rsn_ie)
		pframe = rtw_set_ie(pframe, _FTIE_, ft_ie_len , pie+2, &(pattrib->pktlen));

	pattrib->last_txcmdsz = pattrib->pktlen;
	dump_mgntframe(padapter, pmgntframe);
}

void report_ft_event(_adapter *padapter)
{
	struct mlme_priv		*pmlmepriv = &(padapter->mlmepriv);
	ft_priv	*pftpriv = &pmlmepriv->ftpriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*pnetwork = (WLAN_BSSID_EX *)(&(pmlmeinfo->network));
	struct cfg80211_ft_event_params ft_evt_parms;
	unsigned long irqL;

	memset(&ft_evt_parms, 0, sizeof(ft_evt_parms));
	rtw_update_ft_stainfo(padapter, pnetwork);

	if (!pnetwork)
		goto err_2;

	ft_evt_parms.ies_len = pftpriv->ft_event.ies_len;
	ft_evt_parms.ies =  rtw_zmalloc(ft_evt_parms.ies_len);
	if (ft_evt_parms.ies)
		memcpy((void *)ft_evt_parms.ies, pftpriv->ft_event.ies, ft_evt_parms.ies_len);
	 else
		goto err_2;

	ft_evt_parms.target_ap = rtw_zmalloc(ETH_ALEN);
	if (ft_evt_parms.target_ap)
		memcpy((void *)ft_evt_parms.target_ap, pnetwork->MacAddress, ETH_ALEN);
	else
		goto err_1;

	ft_evt_parms.ric_ies = pftpriv->ft_event.ric_ies;
	ft_evt_parms.ric_ies_len = pftpriv->ft_event.ric_ies_len;

	_enter_critical_bh(&pmlmepriv->lock, &irqL);
	rtw_set_ft_status(padapter, RTW_FT_AUTHENTICATED_STA);
	_exit_critical_bh(&pmlmepriv->lock, &irqL);

	rtw_cfg80211_ft_event(padapter, &ft_evt_parms);
	RTW_INFO("FT: report_ft_event\n");
	rtw_mfree((u8 *)pftpriv->ft_event.target_ap, ETH_ALEN);
err_1:
	rtw_mfree((u8 *)ft_evt_parms.ies, ft_evt_parms.ies_len);
err_2:
	return;
}

void report_ft_reassoc_event(_adapter *padapter, u8 *pMacAddr)
{
	struct mlme_ext_priv		*pmlmeext = &padapter->mlmeextpriv;
	struct cmd_priv			*pcmdpriv = &padapter->cmdpriv;
	struct cmd_obj			*pcmd_obj = NULL;
	struct stassoc_event		*passoc_sta_evt = NULL;
	struct C2HEvent_Header	*pc2h_evt_hdr = NULL;
	u8	*pevtcmd = NULL;
	u32	cmdsz = 0;

	pcmd_obj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmd_obj == NULL)
		return;

	cmdsz = (sizeof(struct stassoc_event) + sizeof(struct C2HEvent_Header));
	pevtcmd = (u8 *)rtw_zmalloc(cmdsz);
	if (pevtcmd == NULL) {
		rtw_mfree((u8 *)pcmd_obj, sizeof(struct cmd_obj));
		return;
	}

	INIT_LIST_HEAD(&pcmd_obj->list);
	pcmd_obj->cmdcode = GEN_CMD_CODE(_Set_MLME_EVT);
	pcmd_obj->cmdsz = cmdsz;
	pcmd_obj->parmbuf = pevtcmd;
	pcmd_obj->rsp = NULL;
	pcmd_obj->rspsz  = 0;

	pc2h_evt_hdr = (struct C2HEvent_Header *)(pevtcmd);
	pc2h_evt_hdr->len = sizeof(struct stassoc_event);
	pc2h_evt_hdr->ID = GEN_EVT_CODE(_FT_REASSOC);
	pc2h_evt_hdr->seq = ATOMIC_INC_RETURN(&pmlmeext->event_seq);

	passoc_sta_evt = (struct stassoc_event *)(pevtcmd + sizeof(struct C2HEvent_Header));
	memcpy((unsigned char *)(&(passoc_sta_evt->macaddr)), pMacAddr, ETH_ALEN);
	rtw_enqueue_cmd(pcmdpriv, pcmd_obj);
}
#endif

u8 NULL_hdl(_adapter *padapter, u8 *pbuf)
{
	return H2C_SUCCESS;
}

#ifdef CONFIG_AUTO_AP_MODE
void rtw_start_auto_ap(_adapter *adapter)
{
	RTW_INFO("%s\n", __func__);

	rtw_set_802_11_infrastructure_mode(adapter, Ndis802_11APMode);

	rtw_setopmode_cmd(adapter, Ndis802_11APMode, true);
}

static int rtw_auto_ap_start_beacon(_adapter *adapter)
{
	int ret = 0;
	u8 *pbuf = NULL;
	uint len;
	u8	supportRate[16];
	int	sz = 0, rateLen;
	u8	*ie;
	u8	wireless_mode, oper_channel;
	u8 ssid[3] = {0}; /* hidden ssid */
	u32 ssid_len = sizeof(ssid);
	struct mlme_priv *pmlmepriv = &(adapter->mlmepriv);


	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) != true)
		return -EINVAL;


	len = 128;
	pbuf = rtw_zmalloc(len);
	if (!pbuf)
		return -ENOMEM;


	/* generate beacon */
	ie = pbuf;

	/* timestamp will be inserted by hardware */
	sz += 8;
	ie += sz;

	/* beacon interval : 2bytes */
	*(__le16 *)ie = cpu_to_le16((u16)100); /* BCN_INTERVAL=100; */
	sz += 2;
	ie += 2;

	/* capability info */
	*(u16 *)ie = 0;
	*(u16 *)ie |= cpu_to_le16(cap_ESS);
	*(u16 *)ie |= cpu_to_le16(cap_ShortPremble);
	/* *(u16*)ie |= cpu_to_le16(cap_Privacy); */
	sz += 2;
	ie += 2;

	/* SSID */
	ie = rtw_set_ie(ie, _SSID_IE_, ssid_len, ssid, &sz);

	/* supported rates */
	wireless_mode = WIRELESS_11BG_24N;
	rtw_set_supported_rate(supportRate, wireless_mode) ;
	rateLen = rtw_get_rateset_len(supportRate);
	if (rateLen > 8)
		ie = rtw_set_ie(ie, _SUPPORTEDRATES_IE_, 8, supportRate, &sz);
	else
		ie = rtw_set_ie(ie, _SUPPORTEDRATES_IE_, rateLen, supportRate, &sz);


	/* DS parameter set */
	if (rtw_mi_check_status(adapter, MI_LINKED))
		oper_channel = rtw_mi_get_union_chan(adapter);
	else
		oper_channel = adapter_to_dvobj(adapter)->oper_channel;

	ie = rtw_set_ie(ie, _DSSET_IE_, 1, &oper_channel, &sz);

	/* ext supported rates */
	if (rateLen > 8)
		ie = rtw_set_ie(ie, _EXT_SUPPORTEDRATES_IE_, (rateLen - 8), (supportRate + 8), &sz);

	RTW_INFO("%s, start auto ap beacon sz=%d\n", __func__, sz);

	/* lunch ap mode & start to issue beacon */
	if (rtw_check_beacon_data(adapter, pbuf,  sz) == _SUCCESS) {

	} else
		ret = -EINVAL;


	rtw_mfree(pbuf, len);

	return ret;

}
#endif/* CONFIG_AUTO_AP_MODE */

u8 setopmode_hdl(_adapter *padapter, u8 *pbuf)
{
	u8	type;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct setopmode_parm *psetop = (struct setopmode_parm *)pbuf;

	if (psetop->mode == Ndis802_11APMode) {
		pmlmeinfo->state = WIFI_FW_AP_STATE;
		type = _HW_STATE_AP_;
#ifdef CONFIG_NATIVEAP_MLME
		/* start_ap_mode(padapter); */
#endif
	} else if (psetop->mode == Ndis802_11Infrastructure) {
		pmlmeinfo->state &= ~(BIT(0) | BIT(1)); /* clear state */
		pmlmeinfo->state |= WIFI_FW_STATION_STATE;/* set to 	STATION_STATE */
		type = _HW_STATE_STATION_;
	} else if (psetop->mode == Ndis802_11IBSS)
		type = _HW_STATE_ADHOC_;
	else if (psetop->mode == Ndis802_11Monitor)
		type = _HW_STATE_MONITOR_;
	else
		type = _HW_STATE_NOLINK_;

#ifdef CONFIG_AP_PORT_SWAP
	rtw_hal_set_hwreg(padapter, HW_VAR_PORT_SWITCH, (u8 *)(&type));
#endif

	rtw_hal_set_hwreg(padapter, HW_VAR_SET_OPMODE, (u8 *)(&type));

#ifdef CONFIG_AUTO_AP_MODE
	if (psetop->mode == Ndis802_11APMode)
		rtw_auto_ap_start_beacon(padapter);
#endif

	if (rtw_port_switch_chk(padapter)) {
		rtw_hal_set_hwreg(padapter, HW_VAR_PORT_SWITCH, NULL);

		if (psetop->mode == Ndis802_11APMode)
			adapter_to_pwrctl(padapter)->fw_psmode_iface_id = 0xff; /* ap mode won't dowload rsvd pages */
		else if (psetop->mode == Ndis802_11Infrastructure) {
#ifdef CONFIG_LPS
			_adapter *port0_iface = dvobj_get_port0_adapter(adapter_to_dvobj(padapter));
			if (port0_iface)
				rtw_lps_ctrl_wk_cmd(port0_iface, LPS_CTRL_CONNECT, 0);
#endif
		}
	}

#ifdef CONFIG_BT_COEXIST
	if (psetop->mode == Ndis802_11APMode ||
		psetop->mode == Ndis802_11Monitor) {
		/* Do this after port switch to */
		/* prevent from downloading rsvd page to wrong port */
		rtw_btcoex_MediaStatusNotify(padapter, 1); /* connect */
	}
#endif /* CONFIG_BT_COEXIST */

	return H2C_SUCCESS;

}

u8 createbss_hdl(_adapter *padapter, u8 *pbuf)
{
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX	*pnetwork = (WLAN_BSSID_EX *)(&(pmlmeinfo->network));
	WLAN_BSSID_EX	*pdev_network = &padapter->registrypriv.dev_network;
	struct createbss_parm *parm = (struct createbss_parm *)pbuf;
	u8 ret = H2C_SUCCESS;
	/* u8	initialgain; */

#ifdef CONFIG_AP_MODE
	if (pmlmeinfo->state == WIFI_FW_AP_STATE) {
		start_bss_network(padapter, parm);
		goto exit;
	}
#endif

	/* below is for ad-hoc master */
	if (parm->adhoc) {
		int tmp_len;

		rtw_warn_on(pdev_network->InfrastructureMode != Ndis802_11IBSS);
		rtw_joinbss_reset(padapter);

		pmlmeext->cur_bwmode = CHANNEL_WIDTH_20;
		pmlmeext->cur_ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
		pmlmeinfo->ERP_enable = 0;
		pmlmeinfo->WMM_enable = 0;
		pmlmeinfo->HT_enable = 0;
		pmlmeinfo->HT_caps_enable = 0;
		pmlmeinfo->HT_info_enable = 0;
		pmlmeinfo->agg_enable_bitmap = 0;
		pmlmeinfo->candidate_tid_bitmap = 0;

		/* config the initial gain under linking, need to write the BB registers */
		/* initialgain = 0x1E; */
		/*rtw_hal_set_odm_var(padapter, HAL_ODM_INITIAL_GAIN, &initialgain, false);*/

		/* disable dynamic functions, such as high power, DIG */
		rtw_phydm_ability_backup(padapter);
		rtw_phydm_func_disable_all(padapter);

		/* cancel link timer */
		_cancel_timer_ex(&pmlmeext->link_timer);

		/* clear CAM */
		flush_all_cam_entry(padapter);

		pdev_network->Length = get_WLAN_BSSID_EX_sz(pdev_network);
		tmp_len = pdev_network->IELength;
		if (tmp_len >= MAX_IE_SZ || tmp_len >= sizeof(pnetwork)){
			pr_info("********** tmp_len too large, value = 0x%x\n", tmp_len);
			ret = H2C_PARAMETERS_ERROR;
			goto ibss_post_hdl;
		}

		memcpy(pnetwork, pdev_network, tmp_len);
		pnetwork->IELength = pdev_network->IELength;

		memcpy(pnetwork->IEs, pdev_network->IEs, pnetwork->IELength);
		start_create_ibss(padapter);
	} else {
		pr_info("NOT an ad-hoc master\n");
		return  H2C_PARAMETERS_ERROR;
	}

ibss_post_hdl:
	rtw_create_ibss_post_hdl(padapter, ret);

exit:
	return ret;
}

u8 join_cmd_hdl(_adapter *padapter, u8 *pbuf)
{
	u8	join_type;
	PNDIS_802_11_VARIABLE_IEs	pIE;
	struct registry_priv	*pregpriv = &padapter->registrypriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*pnetwork = (WLAN_BSSID_EX *)(&(pmlmeinfo->network));
#ifdef CONFIG_ANTENNA_DIVERSITY
	struct joinbss_parm	*pparm = (struct joinbss_parm *)pbuf;
#endif /* CONFIG_ANTENNA_DIVERSITY */
	u32 i;
	/* u8	initialgain; */
	/* u32	acparm; */
	u8 u_ch, u_bw, u_offset;
	u8 doiqk = false;

	/* check already connecting to AP or not */
	if (pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS) {
		if (pmlmeinfo->state & WIFI_FW_STATION_STATE)
			issue_deauth_ex(padapter, pnetwork->MacAddress, WLAN_REASON_DEAUTH_LEAVING, 1, 100);
		pmlmeinfo->state = WIFI_FW_NULL_STATE;

		/* clear CAM */
		flush_all_cam_entry(padapter);

		_cancel_timer_ex(&pmlmeext->link_timer);

		/* set MSR to nolink->infra. mode		 */
		/* Set_MSR(padapter, _HW_STATE_NOLINK_); */
		Set_MSR(padapter, _HW_STATE_STATION_);


		rtw_hal_set_hwreg(padapter, HW_VAR_MLME_DISCONNECT, NULL);
	}

#ifdef CONFIG_ANTENNA_DIVERSITY
	rtw_antenna_select_cmd(padapter, pparm->network.PhyInfo.Optimum_antenna, false);
#endif

#ifdef CONFIG_WAPI_SUPPORT
	rtw_wapi_clear_all_cam_entry(padapter);
#endif

	rtw_joinbss_reset(padapter);

	pmlmeinfo->ERP_enable = 0;
	pmlmeinfo->WMM_enable = 0;
	pmlmeinfo->HT_enable = 0;
	pmlmeinfo->HT_caps_enable = 0;
	pmlmeinfo->HT_info_enable = 0;
	pmlmeinfo->agg_enable_bitmap = 0;
	pmlmeinfo->candidate_tid_bitmap = 0;
	pmlmeinfo->bwmode_updated = false;
	/* pmlmeinfo->assoc_AP_vendor = HT_IOT_PEER_MAX; */
	pmlmeinfo->VHT_enable = 0;

	memcpy(pnetwork, pbuf, FIELD_OFFSET(WLAN_BSSID_EX, IELength));
	pnetwork->IELength = ((WLAN_BSSID_EX *)pbuf)->IELength;

	if (pnetwork->IELength > MAX_IE_SZ) /* Check pbuf->IELength */
		return H2C_PARAMETERS_ERROR;

	if (pnetwork->IELength < 2) {
		report_join_res(padapter, (-4));
		return H2C_SUCCESS;
	}
	memcpy(pnetwork->IEs, ((WLAN_BSSID_EX *)pbuf)->IEs, pnetwork->IELength);

	pmlmeinfo->bcn_interval = get_beacon_interval(pnetwork);

	/* Check AP vendor to move rtw_joinbss_cmd() */
	/* pmlmeinfo->assoc_AP_vendor = check_assoc_AP(pnetwork->IEs, pnetwork->IELength); */

	/* sizeof(NDIS_802_11_FIXED_IEs)	 */
	for (i = _FIXED_IE_LENGTH_ ; i < pnetwork->IELength - 2 ;) {
		pIE = (PNDIS_802_11_VARIABLE_IEs)(pnetwork->IEs + i);

		switch (pIE->ElementID) {
		case _VENDOR_SPECIFIC_IE_: /* Get WMM IE. */
			if (!memcmp(pIE->data, WMM_OUI, 4))
				WMM_param_handler(padapter, pIE);
			break;
		case _HT_CAPABILITY_IE_:	/* Get HT Cap IE. */
			pmlmeinfo->HT_caps_enable = 1;
			break;

		case _HT_EXTRA_INFO_IE_:	/* Get HT Info IE. */
			pmlmeinfo->HT_info_enable = 1;
			break;
		default:
			break;
		}

		i += (pIE->Length + 2);
	}

	rtw_bss_get_chbw(pnetwork
		, &pmlmeext->cur_channel, &pmlmeext->cur_bwmode, &pmlmeext->cur_ch_offset);

	rtw_adjust_chbw(padapter, pmlmeext->cur_channel, &pmlmeext->cur_bwmode, &pmlmeext->cur_ch_offset);

	/* check channel, bandwidth, offset and switch */
	if (rtw_chk_start_clnt_join(padapter, &u_ch, &u_bw, &u_offset) == _FAIL) {
		report_join_res(padapter, (-4));
		return H2C_SUCCESS;
	}

	rtw_hal_set_hwreg(padapter, HW_VAR_BSSID, pmlmeinfo->network.MacAddress);
	join_type = 0;
	rtw_hal_set_hwreg(padapter, HW_VAR_MLME_JOIN, (u8 *)(&join_type));
	doiqk = true;
	rtw_hal_set_hwreg(padapter , HW_VAR_DO_IQK , &doiqk);

	set_channel_bwmode(padapter, u_ch, u_offset, u_bw);
	rtw_mi_update_union_chan_inf(padapter, u_ch, u_offset, u_bw);

	doiqk = false;
	rtw_hal_set_hwreg(padapter , HW_VAR_DO_IQK , &doiqk);

	/* cancel link timer */
	_cancel_timer_ex(&pmlmeext->link_timer);

	start_clnt_join(padapter);

	return H2C_SUCCESS;
}

u8 disconnect_hdl(_adapter *padapter, unsigned char *pbuf)
{
	struct disconnect_parm *param = (struct disconnect_parm *)pbuf;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	WLAN_BSSID_EX		*pnetwork = (WLAN_BSSID_EX *)(&(pmlmeinfo->network));
	u8 val8;

	if (is_client_associated_to_ap(padapter)) {
#ifdef CONFIG_DFS
		if (padapter->mlmepriv.handle_dfs == false)
#endif /* CONFIG_DFS */
#ifdef CONFIG_PLATFORM_ROCKCHIPS
			/* To avoid connecting to AP fail during resume process, change retry count from 5 to 1 */
			issue_deauth_ex(padapter, pnetwork->MacAddress, WLAN_REASON_DEAUTH_LEAVING, 1, 100);
#else
			issue_deauth_ex(padapter, pnetwork->MacAddress, WLAN_REASON_DEAUTH_LEAVING, param->deauth_timeout_ms / 100, 100);
#endif /* CONFIG_PLATFORM_ROCKCHIPS */
	}

#ifdef CONFIG_DFS
	if (padapter->mlmepriv.handle_dfs)
		padapter->mlmepriv.handle_dfs = false;
#endif /* CONFIG_DFS */

	if (((pmlmeinfo->state & 0x03) == WIFI_FW_ADHOC_STATE) || ((pmlmeinfo->state & 0x03) == WIFI_FW_AP_STATE)) {
		/* Stop BCN */
		val8 = 0;
		rtw_hal_set_hwreg(padapter, HW_VAR_BCN_FUNC, (u8 *)(&val8));
	}

	rtw_mlmeext_disconnect(padapter);

	rtw_free_uc_swdec_pending_queue(padapter);

	return	H2C_SUCCESS;
}

static const char *const _scan_state_str[] = {
	"SCAN_DISABLE",
	"SCAN_START",
	"SCAN_PS_ANNC_WAIT",
	"SCAN_ENTER",
	"SCAN_PROCESS",
	"SCAN_BACKING_OP",
	"SCAN_BACK_OP",
	"SCAN_LEAVING_OP",
	"SCAN_LEAVE_OP",
	"SCAN_SW_ANTDIV_BL",
	"SCAN_TO_P2P_LISTEN",
	"SCAN_P2P_LISTEN",
	"SCAN_COMPLETE",
	"SCAN_STATE_MAX",
};

const char *scan_state_str(u8 state)
{
	state = (state >= SCAN_STATE_MAX) ? SCAN_STATE_MAX : state;
	return _scan_state_str[state];
}

static bool scan_abort_hdl(_adapter *adapter)
{
	struct mlme_ext_priv *pmlmeext = &adapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	struct ss_res *ss = &pmlmeext->sitesurvey_res;
#ifdef CONFIG_P2P
	struct wifidirect_info *pwdinfo = &adapter->wdinfo;
#endif
	bool ret = false;

	if (pmlmeext->scan_abort) {
#ifdef CONFIG_P2P
		if (!rtw_p2p_chk_state(&adapter->wdinfo, P2P_STATE_NONE)) {
			rtw_p2p_findphase_ex_set(pwdinfo, P2P_FINDPHASE_EX_MAX);
			ss->channel_idx = 3;
			RTW_INFO("%s idx:%d, cnt:%u\n", __func__
				 , ss->channel_idx
				 , pwdinfo->find_phase_state_exchange_cnt
				);
		} else
#endif
		{
			ss->channel_idx = ss->ch_num;
			RTW_INFO("%s idx:%d\n", __func__
				 , ss->channel_idx
				);
		}
		pmlmeext->scan_abort = false;
		ret = true;
	}

	return ret;
}

static u8 rtw_scan_sparse(_adapter *adapter, struct rtw_ieee80211_channel *ch, u8 ch_num)
{
	/* interval larger than this is treated as backgroud scan */
#ifndef RTW_SCAN_SPARSE_BG_INTERVAL_MS
#define RTW_SCAN_SPARSE_BG_INTERVAL_MS 12000
#endif

#ifndef RTW_SCAN_SPARSE_CH_NUM_MIRACAST
#define RTW_SCAN_SPARSE_CH_NUM_MIRACAST 1
#endif
#ifndef RTW_SCAN_SPARSE_CH_NUM_BG
#define RTW_SCAN_SPARSE_CH_NUM_BG 4
#endif
#ifdef CONFIG_LAYER2_ROAMING
#ifndef RTW_SCAN_SPARSE_CH_NUM_ROAMING_ACTIVE
#define RTW_SCAN_SPARSE_CH_NUM_ROAMING_ACTIVE 1
#endif
#endif

#define SCAN_SPARSE_CH_NUM_INVALID 255

	static u8 token = 255;
	u32 interval;
	bool busy_traffic = false;
	bool miracast_enabled = false;
	bool bg_scan = false;
	u8 max_allow_ch = SCAN_SPARSE_CH_NUM_INVALID;
	u8 scan_division_num;
	u8 ret_num = ch_num;
	struct registry_priv *regsty = dvobj_to_regsty(adapter_to_dvobj(adapter));
	struct mlme_ext_priv *mlmeext = &adapter->mlmeextpriv;

	if (regsty->wifi_spec)
		goto exit;

	/* assume ch_num > 6 is normal scan */
	if (ch_num <= 6)
		goto exit;

	if (mlmeext->last_scan_time == 0)
		mlmeext->last_scan_time = jiffies;

	interval = rtw_get_passing_time_ms(mlmeext->last_scan_time);


	if (rtw_mi_busy_traffic_check(adapter, false))
		busy_traffic = true;

	if (rtw_mi_check_miracast_enabled(adapter))
		miracast_enabled = true;

	if (interval > RTW_SCAN_SPARSE_BG_INTERVAL_MS)
		bg_scan = true;

	/* max_allow_ch by conditions*/

#if RTW_SCAN_SPARSE_MIRACAST
	if (miracast_enabled && busy_traffic == true)
		max_allow_ch = rtw_min(max_allow_ch, RTW_SCAN_SPARSE_CH_NUM_MIRACAST);
#endif

#if RTW_SCAN_SPARSE_BG
	if (bg_scan)
		max_allow_ch = rtw_min(max_allow_ch, RTW_SCAN_SPARSE_CH_NUM_BG);
#endif

#if  defined(CONFIG_LAYER2_ROAMING) && defined(RTW_SCAN_SPARSE_ROAMING_ACTIVE)
	if (rtw_chk_roam_flags(adapter, RTW_ROAM_ACTIVE)) {
		if (busy_traffic && adapter->mlmepriv.need_to_roam == true)
			max_allow_ch = rtw_min(max_allow_ch, RTW_SCAN_SPARSE_CH_NUM_ROAMING_ACTIVE);
	}
#endif


	if (max_allow_ch != SCAN_SPARSE_CH_NUM_INVALID) {
		int i;
		int k = 0;

		scan_division_num = (ch_num / max_allow_ch) + ((ch_num % max_allow_ch) ? 1 : 0);
		token = (token + 1) % scan_division_num;

		if (0)
			RTW_INFO("scan_division_num:%u, token:%u\n", scan_division_num, token);

		for (i = 0; i < ch_num; i++) {
			if (ch[i].hw_value && (i % scan_division_num) == token
			   ) {
				if (i != k)
					memcpy(&ch[k], &ch[i], sizeof(struct rtw_ieee80211_channel));
				k++;
			}
		}

		memset(&ch[k], 0, sizeof(struct rtw_ieee80211_channel));

		ret_num = k;
		mlmeext->last_scan_time = jiffies;
	}

exit:
	return ret_num;
}

static int rtw_scan_ch_decision(_adapter *padapter, struct rtw_ieee80211_channel *out,
		u32 out_num, struct rtw_ieee80211_channel *in, u32 in_num)
{
	int i, j;
	int scan_ch_num = 0;
	int set_idx;
	u8 chan;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;

	/* clear first */
	memset(out, 0, sizeof(struct rtw_ieee80211_channel) * out_num);

	/* acquire channels from in */
	j = 0;
	for (i = 0; i < in_num; i++) {

		if (0)
			RTW_INFO(FUNC_ADPT_FMT" "CHAN_FMT"\n", FUNC_ADPT_ARG(padapter), CHAN_ARG(&in[i]));

		if (!in[i].hw_value || (in[i].flags & RTW_IEEE80211_CHAN_DISABLED))
			continue;
		if (rtw_mlme_band_check(padapter, in[i].hw_value) == false)
			continue;

		set_idx = rtw_ch_set_search_ch(pmlmeext->channel_set, in[i].hw_value);
		if (set_idx >= 0) {
			if (j >= out_num) {
				RTW_PRINT(FUNC_ADPT_FMT" out_num:%u not enough\n",
					  FUNC_ADPT_ARG(padapter), out_num);
				break;
			}

			memcpy(&out[j], &in[i], sizeof(struct rtw_ieee80211_channel));

			if (pmlmeext->channel_set[set_idx].ScanType == SCAN_PASSIVE)
				out[j].flags |= RTW_IEEE80211_CHAN_PASSIVE_SCAN;

			j++;
		}
		if (j >= out_num)
			break;
	}

	/* if out is empty, use channel_set as default */
	if (j == 0) {
		for (i = 0; i < pmlmeext->max_chan_nums; i++) {
			chan = pmlmeext->channel_set[i].ChannelNum;
			if (rtw_mlme_band_check(padapter, chan)) {
				if (rtw_mlme_ignore_chan(padapter, chan))
					continue;

				if (0)
					RTW_INFO(FUNC_ADPT_FMT" ch:%u\n", FUNC_ADPT_ARG(padapter), chan);

				if (j >= out_num) {
					RTW_PRINT(FUNC_ADPT_FMT" out_num:%u not enough\n",
						FUNC_ADPT_ARG(padapter), out_num);
					break;
				}

				out[j].hw_value = chan;

				if (pmlmeext->channel_set[i].ScanType == SCAN_PASSIVE)
					out[j].flags |= RTW_IEEE80211_CHAN_PASSIVE_SCAN;

				j++;
			}
		}
	}

	/* scan_sparse */
	j = rtw_scan_sparse(padapter, out, j);

	return j;
}

static void sitesurvey_res_reset(_adapter *adapter, struct sitesurvey_parm *parm)
{
	struct ss_res *ss = &adapter->mlmeextpriv.sitesurvey_res;
	int i;

	ss->bss_cnt = 0;
	ss->channel_idx = 0;
	ss->igi_scan = 0;
	ss->igi_before_scan = 0;
#ifdef CONFIG_SCAN_BACKOP
	ss->scan_cnt = 0;
#endif
#if defined(CONFIG_ANTENNA_DIVERSITY) || defined(DBG_SCAN_SW_ANTDIV_BL)
	ss->is_sw_antdiv_bl_scan = 0;
#endif

	for (i = 0; i < RTW_SSID_SCAN_AMOUNT; i++) {
		if (parm->ssid[i].SsidLength) {
			memcpy(ss->ssid[i].Ssid, parm->ssid[i].Ssid, IW_ESSID_MAX_SIZE);
			ss->ssid[i].SsidLength = parm->ssid[i].SsidLength;
		} else
			ss->ssid[i].SsidLength = 0;
	}

	ss->ch_num = rtw_scan_ch_decision(adapter
					  , ss->ch, RTW_CHANNEL_SCAN_AMOUNT
					  , parm->ch, parm->ch_num
					 );

	ss->scan_mode = parm->scan_mode;
}

static u8 sitesurvey_pick_ch_behavior(_adapter *padapter, u8 *ch, RT_SCAN_TYPE *type)
{
	u8 next_state;
	u8 scan_ch = 0;
	RT_SCAN_TYPE scan_type = SCAN_PASSIVE;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &pmlmeext->mlmext_info;
	struct ss_res *ss = &pmlmeext->sitesurvey_res;

#ifdef CONFIG_P2P
	struct wifidirect_info *pwdinfo = &padapter->wdinfo;
#endif

	/* handle scan abort request */
	scan_abort_hdl(padapter);

#ifdef CONFIG_P2P
	if (pwdinfo->rx_invitereq_info.scan_op_ch_only || pwdinfo->p2p_info.scan_op_ch_only) {
		if (pwdinfo->rx_invitereq_info.scan_op_ch_only)
			scan_ch = pwdinfo->rx_invitereq_info.operation_ch[ss->channel_idx];
		else
			scan_ch = pwdinfo->p2p_info.operation_ch[ss->channel_idx];
		scan_type = SCAN_ACTIVE;
	} else if (rtw_p2p_findphase_ex_is_social(pwdinfo)) {
		/*
		* Commented by Albert 2011/06/03
		* The driver is in the find phase, it should go through the social channel.
		*/
		int ch_set_idx;

		scan_ch = pwdinfo->social_chan[ss->channel_idx];
		ch_set_idx = rtw_ch_set_search_ch(pmlmeext->channel_set, scan_ch);
		if (ch_set_idx >= 0)
			scan_type = pmlmeext->channel_set[ch_set_idx].ScanType;
		else
			scan_type = SCAN_ACTIVE;
	} else
#endif /* CONFIG_P2P */
	{
		struct rtw_ieee80211_channel *ch;

		if (ss->channel_idx < ss->ch_num) {
			ch = &ss->ch[ss->channel_idx];
			scan_ch = ch->hw_value;
			scan_type = (ch->flags & RTW_IEEE80211_CHAN_PASSIVE_SCAN) ? SCAN_PASSIVE : SCAN_ACTIVE;
		}
	}

	if (scan_ch != 0) {
		next_state = SCAN_PROCESS;
#ifdef CONFIG_SCAN_BACKOP
		{
			struct mi_state mstate;
			u8 backop_flags = 0;

			rtw_mi_status(padapter, &mstate);

			if ((MSTATE_STA_LD_NUM(&mstate) && mlmeext_chk_scan_backop_flags_sta(pmlmeext, SS_BACKOP_EN))
				|| (MSTATE_STA_NUM(&mstate) && mlmeext_chk_scan_backop_flags_sta(pmlmeext, SS_BACKOP_EN_NL)))
				backop_flags |= mlmeext_scan_backop_flags_sta(pmlmeext);

			if ((MSTATE_AP_LD_NUM(&mstate) && mlmeext_chk_scan_backop_flags_ap(pmlmeext, SS_BACKOP_EN))
				|| (MSTATE_AP_NUM(&mstate) && mlmeext_chk_scan_backop_flags_ap(pmlmeext, SS_BACKOP_EN_NL)))
				backop_flags |= mlmeext_scan_backop_flags_ap(pmlmeext);

			if (backop_flags) {
				if (ss->scan_cnt < ss->scan_cnt_max)
					ss->scan_cnt++;
				else {
					mlmeext_assign_scan_backop_flags(pmlmeext, backop_flags);
					next_state = SCAN_BACKING_OP;
				}
			}
		}
#endif /* CONFIG_SCAN_BACKOP */
	} else if (rtw_p2p_findphase_ex_is_needed(pwdinfo)) {
		/* go p2p listen */
		next_state = SCAN_TO_P2P_LISTEN;

#ifdef CONFIG_ANTENNA_DIVERSITY
	} else if (rtw_hal_antdiv_before_linked(padapter)) {
		/* go sw antdiv before link */
		next_state = SCAN_SW_ANTDIV_BL;
#endif
	} else {
		next_state = SCAN_COMPLETE;

#if defined(DBG_SCAN_SW_ANTDIV_BL)
		{
			/* for SCAN_SW_ANTDIV_BL state testing */
			struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
			int i;
			bool is_linked = false;

			for (i = 0; i < dvobj->iface_nums; i++) {
				if (rtw_linked_check(dvobj->padapters[i]))
					is_linked = true;
			}

			if (!is_linked) {
				static bool fake_sw_antdiv_bl_state = 0;

				if (fake_sw_antdiv_bl_state == 0) {
					next_state = SCAN_SW_ANTDIV_BL;
					fake_sw_antdiv_bl_state = 1;
				} else
					fake_sw_antdiv_bl_state = 0;
			}
		}
#endif /* defined(DBG_SCAN_SW_ANTDIV_BL) */
	}

#ifdef CONFIG_SCAN_BACKOP
	if (next_state != SCAN_PROCESS)
		ss->scan_cnt = 0;
#endif


#ifdef DBG_FIXED_CHAN
	if (pmlmeext->fixed_chan != 0xff && next_state == SCAN_PROCESS)
		scan_ch = pmlmeext->fixed_chan;
#endif

	if (ch)
		*ch = scan_ch;
	if (type)
		*type = scan_type;

	return next_state;
}

void site_survey(_adapter *padapter, u8 survey_channel, RT_SCAN_TYPE ScanType)
{
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);

#ifdef CONFIG_P2P
	struct wifidirect_info *pwdinfo = &(padapter->wdinfo);
#endif

	if (survey_channel != 0) {
		set_channel_bwmode(padapter, survey_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, CHANNEL_WIDTH_20);

#ifdef CONFIG_AUTO_CHNL_SEL_NHM
		if (ACS_ENABLE == GET_ACS_STATE(padapter)) {
			ACS_OP acs_op = ACS_RESET;

			rtw_hal_set_odm_var(padapter, HAL_ODM_AUTO_CHNL_SEL, &acs_op, true);
			rtw_set_acs_channel(padapter, survey_channel);
#ifdef DBG_AUTO_CHNL_SEL_NHM
			RTW_INFO("[ACS-"ADPT_FMT"]-set ch:%u\n",
				ADPT_ARG(padapter), rtw_get_acs_channel(padapter));
#endif
		}
#endif

		if (ScanType == SCAN_ACTIVE) {
#ifdef CONFIG_P2P
			#ifdef CONFIG_IOCTL_CFG80211
			if (rtw_cfg80211_is_p2p_scan(padapter))
			#else
			if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_SCAN)
				|| rtw_p2p_chk_state(pwdinfo, P2P_STATE_FIND_PHASE_SEARCH))
			#endif
			{
				issue_probereq_p2p(padapter, NULL);
				issue_probereq_p2p(padapter, NULL);
				issue_probereq_p2p(padapter, NULL);
			} else
#endif /* CONFIG_P2P */
			{
				int i;

				for (i = 0; i < RTW_SSID_SCAN_AMOUNT; i++) {
					if (pmlmeext->sitesurvey_res.ssid[i].SsidLength) {
						/* IOT issue, When wifi_spec is not set, send one probe req without WPS IE. */
						if (padapter->registrypriv.wifi_spec)
							issue_probereq(padapter, &(pmlmeext->sitesurvey_res.ssid[i]), NULL);
						else
							issue_probereq_ex(padapter, &(pmlmeext->sitesurvey_res.ssid[i]), NULL, 0, 0, 0, 0);
						issue_probereq(padapter, &(pmlmeext->sitesurvey_res.ssid[i]), NULL);
					}
				}

				if (pmlmeext->sitesurvey_res.scan_mode == SCAN_ACTIVE) {
					/* IOT issue, When wifi_spec is not set, send one probe req without WPS IE. */
					if (padapter->registrypriv.wifi_spec)
						issue_probereq(padapter, NULL, NULL);
					else
						issue_probereq_ex(padapter, NULL, NULL, 0, 0, 0, 0);
					issue_probereq(padapter, NULL, NULL);
				}
			}
		}
	} else {
		/* channel number is 0 or this channel is not valid. */
		rtw_warn_on(1);
	}

	return;
}

static void survey_done_set_ch_bw(_adapter *padapter)
{
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	u8 cur_channel = 0;
	u8 cur_bwmode;
	u8 cur_ch_offset;

#ifdef CONFIG_MCC_MODE
	if (!rtw_hal_mcc_change_scan_flag(padapter, &cur_channel, &cur_bwmode, &cur_ch_offset)) {
		if (0)
			RTW_INFO(FUNC_ADPT_FMT" back to AP channel - ch:%u, bw:%u, offset:%u\n",
				FUNC_ADPT_ARG(padapter), cur_channel, cur_bwmode, cur_ch_offset);
		goto exit;
	}
#endif

	if (rtw_mi_get_ch_setting_union(padapter, &cur_channel, &cur_bwmode, &cur_ch_offset) != 0) {
		if (0)
			RTW_INFO(FUNC_ADPT_FMT" back to linked/linking union - ch:%u, bw:%u, offset:%u\n",
				FUNC_ADPT_ARG(padapter), cur_channel, cur_bwmode, cur_ch_offset);
	} else {
#ifdef CONFIG_P2P
		struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);
		_adapter *iface;
		int i;

		for (i = 0; i < dvobj->iface_nums; i++) {
			iface = dvobj->padapters[i];
			if (!iface)
				continue;

#ifdef CONFIG_IOCTL_CFG80211
			if (iface->wdinfo.driver_interface == DRIVER_CFG80211 && !adapter_wdev_data(iface)->p2p_enabled)
				continue;
#endif

			if (rtw_p2p_chk_state(&iface->wdinfo, P2P_STATE_LISTEN)) {
				cur_channel = iface->wdinfo.listen_channel;
				cur_bwmode = CHANNEL_WIDTH_20;
				cur_ch_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
				if (0)
					RTW_INFO(FUNC_ADPT_FMT" back to "ADPT_FMT"'s listen ch - ch:%u, bw:%u, offset:%u\n",
						FUNC_ADPT_ARG(padapter), ADPT_ARG(iface), cur_channel, cur_bwmode, cur_ch_offset);
				break;
			}
		}
#endif /* CONFIG_P2P */

		if (cur_channel == 0) {
			cur_channel = pmlmeext->cur_channel;
			cur_bwmode = pmlmeext->cur_bwmode;
			cur_ch_offset = pmlmeext->cur_ch_offset;
			if (0)
				RTW_INFO(FUNC_ADPT_FMT" back to ch:%u, bw:%u, offset:%u\n",
					FUNC_ADPT_ARG(padapter), cur_channel, cur_bwmode, cur_ch_offset);
		}
	}
exit:
	set_channel_bwmode(padapter, cur_channel, cur_ch_offset, cur_bwmode);
}

/**
 * sitesurvey_ps_annc - check and doing ps announcement for all the adapters of given @dvobj
 * @padapter
 * @ps: power saving or not
 *
 * Returns: 0: no ps announcement is doing. 1: ps announcement is doing
 */

static u8 sitesurvey_ps_annc(_adapter *padapter, bool ps)
{
	u8 ps_anc = 0;

	if (rtw_mi_issue_nulldata(padapter, NULL, ps, 3, 500))
		ps_anc = 1;
	return ps_anc;
}

static void sitesurvey_set_igi(_adapter *adapter)
{
	struct mlme_ext_priv *mlmeext = &adapter->mlmeextpriv;
	struct ss_res *ss = &mlmeext->sitesurvey_res;
	u8 igi;
#ifdef CONFIG_P2P
	struct wifidirect_info *pwdinfo = &adapter->wdinfo;
#endif

	switch (mlmeext_scan_state(mlmeext)) {
	case SCAN_ENTER:
		#ifdef CONFIG_P2P
		#ifdef CONFIG_IOCTL_CFG80211
		if (adapter_wdev_data(adapter)->p2p_enabled && pwdinfo->driver_interface == DRIVER_CFG80211)
			igi = 0x30;
		else
		#endif /* CONFIG_IOCTL_CFG80211 */
		if (!rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
			igi = 0x28;
		else
		#endif /* CONFIG_P2P */
			igi = 0x1e;

		/* record IGI status */
		ss->igi_scan = igi;
		rtw_hal_get_odm_var(adapter, HAL_ODM_INITIAL_GAIN, &ss->igi_before_scan, NULL);

		/* disable DIG and set IGI for scan */
		rtw_hal_set_odm_var(adapter, HAL_ODM_INITIAL_GAIN, &igi, false);
		break;
	case SCAN_COMPLETE:
	case SCAN_TO_P2P_LISTEN:
		/* enable DIG and restore IGI */
		igi = 0xff;
		rtw_hal_set_odm_var(adapter, HAL_ODM_INITIAL_GAIN, &igi, false);
		break;
#ifdef CONFIG_SCAN_BACKOP
	case SCAN_BACKING_OP:
		/* write IGI for op channel when DIG is not enabled */
		odm_write_dig(GET_ODM(adapter), ss->igi_before_scan);
		break;
	case SCAN_LEAVE_OP:
		/* write IGI for scan when DIG is not enabled */
		odm_write_dig(GET_ODM(adapter), ss->igi_scan);
		break;
#endif /* CONFIG_SCAN_BACKOP */
	default:
		rtw_warn_on(1);
		break;
	}
}

static void sitesurvey_set_msr(_adapter *adapter, bool enter)
{
	u8 network_type;
	struct mlme_ext_priv *pmlmeext = &adapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);

	if (enter) {
#ifdef CONFIG_MI_WITH_MBSSID_CAM
		rtw_hal_get_hwreg(adapter, HW_VAR_MEDIA_STATUS, (u8 *)(&pmlmeinfo->hw_media_state));
#endif
		/* set MSR to no link state */
		network_type = _HW_STATE_NOLINK_;
	} else {
#ifdef CONFIG_MI_WITH_MBSSID_CAM
		network_type = pmlmeinfo->hw_media_state;
#else
		network_type = pmlmeinfo->state & 0x3;
#endif
	}
	Set_MSR(adapter, network_type);
}
u8 sitesurvey_cmd_hdl(_adapter *padapter, u8 *pbuf)
{
	struct sitesurvey_parm	*pparm = (struct sitesurvey_parm *)pbuf;
	struct dvobj_priv *dvobj = padapter->dvobj;
	struct debug_priv *pdbgpriv = &dvobj->drv_dbg;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	struct ss_res *ss = &pmlmeext->sitesurvey_res;
	u8 val8;

#ifdef CONFIG_P2P
	struct wifidirect_info *pwdinfo = &padapter->wdinfo;
#endif

#ifdef DBG_CHECK_FW_PS_STATE
	if (rtw_fw_ps_state(padapter) == _FAIL) {
		RTW_INFO("scan without leave 32k\n");
		pdbgpriv->dbg_scan_pwr_state_cnt++;
	}
#endif /* DBG_CHECK_FW_PS_STATE */

	/* increase channel idx */
	if (mlmeext_chk_scan_state(pmlmeext, SCAN_PROCESS))
		ss->channel_idx++;

	/* update scan state to next state (assigned by previous cmd hdl) */
	if (mlmeext_scan_state(pmlmeext) != mlmeext_scan_next_state(pmlmeext))
		mlmeext_set_scan_state(pmlmeext, mlmeext_scan_next_state(pmlmeext));

operation_by_state:
	switch (mlmeext_scan_state(pmlmeext)) {

	case SCAN_DISABLE:
		/*
		* SW parameter initialization
		*/

		sitesurvey_res_reset(padapter, pparm);
		mlmeext_set_scan_state(pmlmeext, SCAN_START);
		goto operation_by_state;

	case SCAN_START:
		/*
		* prepare to leave operating channel
		*/

#ifdef CONFIG_MCC_MODE
		rtw_hal_set_mcc_setting_scan_start(padapter);
#endif /* CONFIG_MCC_MODE */

		/* apply rx ampdu setting */
		if (ss->rx_ampdu_accept != RX_AMPDU_ACCEPT_INVALID
			|| ss->rx_ampdu_size != RX_AMPDU_SIZE_INVALID)
			rtw_rx_ampdu_apply(padapter);

		/* clear HW TX queue before scan */
		rtw_hal_set_hwreg(padapter, HW_VAR_CHECK_TXBUF, NULL);

		/* power save state announcement */
		if (sitesurvey_ps_annc(padapter, 1)) {
			mlmeext_set_scan_state(pmlmeext, SCAN_PS_ANNC_WAIT);
			mlmeext_set_scan_next_state(pmlmeext, SCAN_ENTER);
			set_survey_timer(pmlmeext, 50); /* delay 50ms to protect nulldata(1) */
		} else {
			mlmeext_set_scan_state(pmlmeext, SCAN_ENTER);
			goto operation_by_state;
		}

		break;

	case SCAN_ENTER:
		/*
		* HW register and DM setting for enter scan
		*/

		rtw_phydm_ability_backup(padapter);

		sitesurvey_set_igi(padapter);

		/* config dynamic functions for off channel */
		rtw_phydm_func_for_offchannel(padapter);
		/* set MSR to no link state */
		sitesurvey_set_msr(padapter, true);

		val8 = 1; /* under site survey */
		rtw_hal_set_hwreg(padapter, HW_VAR_MLME_SITESURVEY, (u8 *)(&val8));

		mlmeext_set_scan_state(pmlmeext, SCAN_PROCESS);
		goto operation_by_state;

	case SCAN_PROCESS: {
		u8 scan_ch;
		RT_SCAN_TYPE scan_type;
		u8 next_state;
		u32 scan_ms;

#ifdef CONFIG_AUTO_CHNL_SEL_NHM
		if ((ACS_ENABLE == GET_ACS_STATE(padapter)) && (0 != rtw_get_acs_channel(padapter))) {
			ACS_OP acs_op = ACS_SELECT;

			rtw_hal_set_odm_var(padapter, HAL_ODM_AUTO_CHNL_SEL, &acs_op, true);
		}
#endif

		next_state = sitesurvey_pick_ch_behavior(padapter, &scan_ch, &scan_type);
		if (next_state != SCAN_PROCESS) {
#ifdef CONFIG_AUTO_CHNL_SEL_NHM
			if (ACS_ENABLE == GET_ACS_STATE(padapter)) {
				rtw_set_acs_channel(padapter, 0);
#ifdef DBG_AUTO_CHNL_SEL_NHM
				RTW_INFO("[ACS-"ADPT_FMT"]-set ch:%u\n", ADPT_ARG(padapter), rtw_get_acs_channel(padapter));
#endif
			}
#endif

			mlmeext_set_scan_state(pmlmeext, next_state);
			goto operation_by_state;
		}

		/* still SCAN_PROCESS state */
		if (0)
#ifdef CONFIG_P2P
			RTW_INFO(FUNC_ADPT_FMT" %s ch:%u (cnt:%u,idx:%d) at %dms, %c%c%c\n"
				 , FUNC_ADPT_ARG(padapter)
				 , mlmeext_scan_state_str(pmlmeext)
				 , scan_ch
				, pwdinfo->find_phase_state_exchange_cnt, ss->channel_idx
				, rtw_get_passing_time_ms(padapter->mlmepriv.scan_start_time)
				, scan_type ? 'A' : 'P', ss->scan_mode ? 'A' : 'P'
				 , ss->ssid[0].SsidLength ? 'S' : ' '
				);
#else
			RTW_INFO(FUNC_ADPT_FMT" %s ch:%u (idx:%d) at %dms, %c%c%c\n"
				 , FUNC_ADPT_ARG(padapter)
				 , mlmeext_scan_state_str(pmlmeext)
				 , scan_ch
				 , ss->channel_idx
				, rtw_get_passing_time_ms(padapter->mlmepriv.scan_start_time)
				, scan_type ? 'A' : 'P', ss->scan_mode ? 'A' : 'P'
				 , ss->ssid[0].SsidLength ? 'S' : ' '
				);
#endif /* CONFIG_P2P */

#ifdef DBG_FIXED_CHAN
		if (pmlmeext->fixed_chan != 0xff)
			RTW_INFO(FUNC_ADPT_FMT" fixed_chan:%u\n", pmlmeext->fixed_chan);
#endif

		site_survey(padapter, scan_ch, scan_type);

#if defined(CONFIG_ATMEL_RC_PATCH)
		if (check_fwstate(pmlmepriv, _FW_LINKED))
			scan_ms = 20;
		else
			scan_ms = 40;
#else
		scan_ms = ss->scan_ch_ms;
#endif

#if defined(CONFIG_ANTENNA_DIVERSITY) || defined(DBG_SCAN_SW_ANTDIV_BL)
		if (ss->is_sw_antdiv_bl_scan)
			scan_ms = scan_ms / 2;
#endif

#if defined(CONFIG_SIGNAL_DISPLAY_DBM) && defined(CONFIG_BACKGROUND_NOISE_MONITOR)
		{
			struct noise_info info;

			info.bPauseDIG = false;
			info.IGIValue = 0;
			info.max_time = scan_ms / 2;
			info.chan = scan_ch;
			rtw_hal_set_odm_var(padapter, HAL_ODM_NOISE_MONITOR, &info, false);
		}
#endif

		set_survey_timer(pmlmeext, scan_ms);
		break;
	}

#ifdef CONFIG_SCAN_BACKOP
	case SCAN_BACKING_OP: {
		u8 back_ch, back_bw, back_ch_offset;
		u8 need_ch_setting_union = true;

#ifdef CONFIG_MCC_MODE
		need_ch_setting_union = rtw_hal_mcc_change_scan_flag(padapter,
				&back_ch, &back_bw, &back_ch_offset);
#endif /* CONFIG_MCC_MODE */

		if (need_ch_setting_union) {
			if (rtw_mi_get_ch_setting_union(padapter, &back_ch, &back_bw, &back_ch_offset) == 0)
				rtw_warn_on(1);
		}

		if (0)
			RTW_INFO(FUNC_ADPT_FMT" %s ch:%u, bw:%u, offset:%u at %dms\n"
				 , FUNC_ADPT_ARG(padapter)
				 , mlmeext_scan_state_str(pmlmeext)
				 , back_ch, back_bw, back_ch_offset
				, rtw_get_passing_time_ms(padapter->mlmepriv.scan_start_time)
				);

		set_channel_bwmode(padapter, back_ch, back_ch_offset, back_bw);

		sitesurvey_set_msr(padapter, false);

		val8 = 0; /* survey done */
		rtw_hal_set_hwreg(padapter, HW_VAR_MLME_SITESURVEY, (u8 *)(&val8));

		if (mlmeext_chk_scan_backop_flags(pmlmeext, SS_BACKOP_PS_ANNC)) {
			sitesurvey_set_igi(padapter);
			sitesurvey_ps_annc(padapter, 0);
		}

		mlmeext_set_scan_state(pmlmeext, SCAN_BACK_OP);
		ss->backop_time = jiffies;

		if (mlmeext_chk_scan_backop_flags(pmlmeext, SS_BACKOP_TX_RESUME))
			rtw_mi_os_xmit_schedule(padapter);


		goto operation_by_state;
	}

	case SCAN_BACK_OP:
		if (rtw_get_passing_time_ms(ss->backop_time) >= ss->backop_ms
		    || pmlmeext->scan_abort
		   ) {
			mlmeext_set_scan_state(pmlmeext, SCAN_LEAVING_OP);
			goto operation_by_state;
		}
		set_survey_timer(pmlmeext, 50);
		break;

	case SCAN_LEAVING_OP:
		/*
		* prepare to leave operating channel
		*/

		/* clear HW TX queue before scan */
		rtw_hal_set_hwreg(padapter, HW_VAR_CHECK_TXBUF, 0);

		if (mlmeext_chk_scan_backop_flags(pmlmeext, SS_BACKOP_PS_ANNC)
		    && sitesurvey_ps_annc(padapter, 1)
		   ) {
			mlmeext_set_scan_state(pmlmeext, SCAN_PS_ANNC_WAIT);
			mlmeext_set_scan_next_state(pmlmeext, SCAN_LEAVE_OP);
			set_survey_timer(pmlmeext, 50); /* delay 50ms to protect nulldata(1) */
		} else {
			mlmeext_set_scan_state(pmlmeext, SCAN_LEAVE_OP);
			goto operation_by_state;
		}

		break;

	case SCAN_LEAVE_OP:
		/*
		* HW register and DM setting for enter scan
		*/

		if (mlmeext_chk_scan_backop_flags(pmlmeext, SS_BACKOP_PS_ANNC))
			sitesurvey_set_igi(padapter);

		sitesurvey_set_msr(padapter, true);

		val8 = 1; /* under site survey */
		rtw_hal_set_hwreg(padapter, HW_VAR_MLME_SITESURVEY, (u8 *)(&val8));

		mlmeext_set_scan_state(pmlmeext, SCAN_PROCESS);
		goto operation_by_state;

#endif /* CONFIG_SCAN_BACKOP */

#if defined(CONFIG_ANTENNA_DIVERSITY) || defined(DBG_SCAN_SW_ANTDIV_BL)
	case SCAN_SW_ANTDIV_BL:
		/*
		* 20100721
		* For SW antenna diversity before link, it needs to switch to another antenna and scan again.
		* It compares the scan result and select better one to do connection.
		*/
		ss->bss_cnt = 0;
		ss->channel_idx = 0;
		ss->is_sw_antdiv_bl_scan = 1;

		mlmeext_set_scan_next_state(pmlmeext, SCAN_PROCESS);
		set_survey_timer(pmlmeext, ss->scan_ch_ms);
		break;
#endif

#ifdef CONFIG_P2P
	case SCAN_TO_P2P_LISTEN:
		/*
		* Set the P2P State to the listen state of find phase
		* and set the current channel to the listen channel
		*/
		set_channel_bwmode(padapter, pwdinfo->listen_channel, HAL_PRIME_CHNL_OFFSET_DONT_CARE, CHANNEL_WIDTH_20);
		rtw_p2p_set_state(pwdinfo, P2P_STATE_FIND_PHASE_LISTEN);

		/* turn on phy-dynamic functions */
		rtw_phydm_ability_restore(padapter);

		sitesurvey_set_igi(padapter);

		mlmeext_set_scan_state(pmlmeext, SCAN_P2P_LISTEN);
		_set_timer(&pwdinfo->find_phase_timer, (u32)((u32)pwdinfo->listen_dwell * 100));
		break;

	case SCAN_P2P_LISTEN:
		mlmeext_set_scan_state(pmlmeext, SCAN_PROCESS);
		ss->channel_idx = 0;
		goto operation_by_state;
#endif /* CONFIG_P2P */

	case SCAN_COMPLETE:
#ifdef CONFIG_P2P
		if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_SCAN)
		    || rtw_p2p_chk_state(pwdinfo, P2P_STATE_FIND_PHASE_SEARCH)
		   ) {
#ifdef CONFIG_CONCURRENT_MODE
			if (pwdinfo->driver_interface == DRIVER_WEXT) {
				if (rtw_mi_check_status(padapter, MI_LINKED))
					_set_timer(&pwdinfo->ap_p2p_switch_timer, 500);
			}
#endif

			rtw_p2p_set_state(pwdinfo, rtw_p2p_pre_state(pwdinfo));
		}
		rtw_p2p_findphase_ex_set(pwdinfo, P2P_FINDPHASE_EX_NONE);
#endif /* CONFIG_P2P */

		/* switch channel */
		survey_done_set_ch_bw(padapter);

		sitesurvey_set_msr(padapter, false);

		val8 = 0; /* survey done */
		rtw_hal_set_hwreg(padapter, HW_VAR_MLME_SITESURVEY, (u8 *)(&val8));

		/* turn on phy-dynamic functions */
		rtw_phydm_ability_restore(padapter);

		sitesurvey_set_igi(padapter);

#ifdef CONFIG_MCC_MODE
		/* start MCC fail, then tx null data */
		if (!rtw_hal_set_mcc_setting_scan_complete(padapter))
#endif /* CONFIG_MCC_MODE */
			sitesurvey_ps_annc(padapter, 0);

		/* apply rx ampdu setting */
		rtw_rx_ampdu_apply(padapter);

		mlmeext_set_scan_state(pmlmeext, SCAN_DISABLE);

		report_surveydone_event(padapter);

		issue_action_BSSCoexistPacket(padapter);
		issue_action_BSSCoexistPacket(padapter);
		issue_action_BSSCoexistPacket(padapter);
	}

	return H2C_SUCCESS;
}

u8 setauth_hdl(_adapter *padapter, unsigned char *pbuf)
{
	struct setauth_parm		*pparm = (struct setauth_parm *)pbuf;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (pparm->mode < 4)
		pmlmeinfo->auth_algo = pparm->mode;

	return	H2C_SUCCESS;
}

/*
SEC CAM Entry format (32 bytes)
DW0 - MAC_ADDR[15:0] | Valid[15] | MFB[14:8] | RSVD[7]  | GK[6] | MIC_KEY[5] | SEC_TYPE[4:2] | KID[1:0]
DW0 - MAC_ADDR[15:0] | Valid[15] |RSVD[14:9] | RPT_MODE[8] | SPP_MODE[7]  | GK[6] | MIC_KEY[5] | SEC_TYPE[4:2] | KID[1:0] (92E/8812A/8814A)
DW1 - MAC_ADDR[47:16]
DW2 - KEY[31:0]
DW3 - KEY[63:32]
DW4 - KEY[95:64]
DW5 - KEY[127:96]
DW6 - RSVD
DW7 - RSVD
*/

/*Set WEP key or Group Key*/
u8 setkey_hdl(_adapter *padapter, u8 *pbuf)
{
	u16	ctrl = 0;
	s16 cam_id = 0;
	struct setkey_parm		*pparm = (struct setkey_parm *)pbuf;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	unsigned char null_addr[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	u8 *addr;
	bool used = false;

	/* main tx key for wep. */
	if (pparm->set_tx)
		pmlmeinfo->key_index = pparm->keyid;

#ifdef CONFIG_CONCURRENT_MODE
	if (((pmlmeinfo->state & 0x03) == WIFI_FW_AP_STATE) || ((pmlmeinfo->state & 0x03) == WIFI_FW_ADHOC_STATE))
		cam_id = rtw_iface_bcmc_id_get(padapter);
	else
#endif
		cam_id = rtw_camid_alloc(padapter, NULL, pparm->keyid, &used);

	if (cam_id < 0)
		goto enable_mc;

#ifndef CONFIG_CONCURRENT_MODE
	if (cam_id >= 0 && cam_id <= 3)
		addr = null_addr;
	else
#endif
	{
		if (((pmlmeinfo->state & 0x03) == WIFI_FW_AP_STATE) || ((pmlmeinfo->state & 0x03) == WIFI_FW_ADHOC_STATE))
			/* for AP mode ,we will force sec cam entry_id so hw dont search cam when tx*/
			addr = adapter_mac_addr(padapter);
		else
			/* not default key, searched by A2 */
			addr = get_bssid(&padapter->mlmepriv);
	}

	/* cam entry searched is pairwise key */
	if (used && rtw_camid_is_gk(padapter, cam_id) == false) {
		s16 camid_clr;

		RTW_PRINT(FUNC_ADPT_FMT" group key with "MAC_FMT" id:%u the same key id as pairwise key\n"
			, FUNC_ADPT_ARG(padapter), MAC_ARG(addr), pparm->keyid);

		/* HW has problem to distinguish this group key with existing pairwise key, stop HW enc and dec for BMC */
		rtw_camctl_set_flags(padapter, SEC_STATUS_STA_PK_GK_CONFLICT_DIS_BMC_SEARCH);
		rtw_hal_set_hwreg(padapter, HW_VAR_SEC_CFG, NULL);

		/* clear group key */
		while ((camid_clr = rtw_camid_search(padapter, addr, -1, 1)) >= 0) {
			RTW_PRINT("clear group key for addr:"MAC_FMT", camid:%d\n", MAC_ARG(addr), camid_clr);
			clear_cam_entry(padapter, camid_clr);
			rtw_camid_free(padapter, camid_clr);
		}

		goto enable_mc;
	}

	ctrl = BIT(15) | BIT(6) | ((pparm->algorithm) << 2) | pparm->keyid;

	RTW_PRINT("set group key camid:%d, addr:"MAC_FMT", kid:%d, type:%s\n"
		, cam_id, MAC_ARG(addr), pparm->keyid, security_type_str(pparm->algorithm));

	write_cam(padapter, cam_id, ctrl, addr, pparm->key);

	/* if ((cam_id > 3) && (((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE) || ((pmlmeinfo->state&0x03) == WIFI_FW_ADHOC_STATE)))*/
#ifdef CONFIG_CONCURRENT_MODE
	if (((pmlmeinfo->state & 0x03) == WIFI_FW_AP_STATE) || ((pmlmeinfo->state & 0x03) == WIFI_FW_ADHOC_STATE)) {
		if (is_wep_enc(pparm->algorithm)) {
			padapter->securitypriv.dot11Def_camid[pparm->keyid] = cam_id;
			padapter->securitypriv.dot118021x_bmc_cam_id =
				padapter->securitypriv.dot11Def_camid[padapter->securitypriv.dot11PrivacyKeyIndex];
			RTW_PRINT("wep group key - force camid:%d\n", padapter->securitypriv.dot118021x_bmc_cam_id);
		} else {
			/*u8 org_cam_id = padapter->securitypriv.dot118021x_bmc_cam_id;*/

			/*force GK's cam id*/
			padapter->securitypriv.dot118021x_bmc_cam_id = cam_id;

			/* for GTK rekey
			if ((org_cam_id != INVALID_SEC_MAC_CAM_ID) &&
				(org_cam_id != cam_id)) {
				RTW_PRINT("clear group key for addr:"MAC_FMT", org_camid:%d new_camid:%d\n", MAC_ARG(addr), org_cam_id, cam_id);
				clear_cam_entry(padapter, org_cam_id);
				rtw_camid_free(padapter, org_cam_id);
			}*/
		}
	}
#endif


#ifndef CONFIG_CONCURRENT_MODE
	if (cam_id >= 0 && cam_id <= 3)
		rtw_hal_set_hwreg(padapter, HW_VAR_SEC_DK_CFG, (u8 *)true);
#endif

	/* 8814au should set both broadcast and unicast CAM entry for WEP key in STA mode */
	if (is_wep_enc(pparm->algorithm) && check_mlmeinfo_state(pmlmeext, WIFI_FW_STATION_STATE) &&
	    _rtw_camctl_chk_cap(padapter, SEC_CAP_CHK_BMC)) {
		struct set_stakey_parm	sta_pparm;

		sta_pparm.algorithm = pparm->algorithm;
		sta_pparm.keyid = pparm->keyid;
		memcpy(sta_pparm.key, pparm->key, 16);
		memcpy(sta_pparm.addr, get_bssid(&padapter->mlmepriv), ETH_ALEN);
		set_stakey_hdl(padapter, (u8 *)&sta_pparm);
	}

enable_mc:
	/* allow multicast packets to driver */
	rtw_hal_set_hwreg(padapter, HW_VAR_ON_RCR_AM, null_addr);

	return H2C_SUCCESS;
}

void rtw_ap_wep_pk_setting(_adapter *adapter, struct sta_info *psta)
{
	struct security_priv *psecuritypriv = &(adapter->securitypriv);
	struct set_stakey_parm	sta_pparm;
	sint keyid;

	if (!is_wep_enc(psecuritypriv->dot11PrivacyAlgrthm))
		return;

	for (keyid = 0; keyid < 4; keyid++) {
		if ((psecuritypriv->key_mask & BIT(keyid)) && (keyid == psecuritypriv->dot11PrivacyKeyIndex)) {
			sta_pparm.algorithm = psecuritypriv->dot11PrivacyAlgrthm;
			sta_pparm.keyid = keyid;
			memcpy(sta_pparm.key, &(psecuritypriv->dot11DefKey[keyid].skey[0]), 16);
			memcpy(sta_pparm.addr, psta->hwaddr, ETH_ALEN);

			RTW_PRINT(FUNC_ADPT_FMT"set WEP - PK with "MAC_FMT" keyid:%u\n"
				, FUNC_ADPT_ARG(adapter), MAC_ARG(psta->hwaddr), keyid);

			set_stakey_hdl(adapter, (u8 *)&sta_pparm);
		}
	}
}

u8 set_stakey_hdl(_adapter *padapter, u8 *pbuf)
{
	u16 ctrl = 0;
	s16 cam_id = 0;
	bool used;
	u8 kid = 0;
	u8 ret = H2C_SUCCESS;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct set_stakey_parm	*pparm = (struct set_stakey_parm *)pbuf;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta;

	if (pparm->algorithm == _NO_PRIVACY_)
		goto write_to_cam;

	psta = rtw_get_stainfo(pstapriv, pparm->addr);
	if (!psta) {
		RTW_PRINT("%s sta:"MAC_FMT" not found\n", __func__, MAC_ARG(pparm->addr));
		ret = H2C_REJECTED;
		goto exit;
	}

	pmlmeinfo->enc_algo = pparm->algorithm;
	if (is_wep_enc(pparm->algorithm))
		kid = pparm->keyid;
	cam_id = rtw_camid_alloc(padapter, psta, kid, &used);
	if (cam_id < 0)
		goto exit;

	/* cam entry searched is group key */
	if (used && rtw_camid_is_gk(padapter, cam_id) == true) {
		s16 camid_clr;

		RTW_PRINT(FUNC_ADPT_FMT" pairwise key with "MAC_FMT" id:%u the same key id as group key\n"
			, FUNC_ADPT_ARG(padapter), MAC_ARG(pparm->addr), pparm->keyid);

		/* HW has problem to distinguish this pairwise key with existing group key, stop HW enc and dec for BMC */
		rtw_camctl_set_flags(padapter, SEC_STATUS_STA_PK_GK_CONFLICT_DIS_BMC_SEARCH);
		rtw_hal_set_hwreg(padapter, HW_VAR_SEC_CFG, NULL);

		/* clear group key */
		while ((camid_clr = rtw_camid_search(padapter, pparm->addr, -1, 1)) >= 0) {
			RTW_PRINT("clear group key for addr:"MAC_FMT", camid:%d\n", MAC_ARG(pparm->addr), camid_clr);
			clear_cam_entry(padapter, camid_clr);
			rtw_camid_free(padapter, camid_clr);
		}
	}

write_to_cam:
	if (pparm->algorithm == _NO_PRIVACY_) {
		while ((cam_id = rtw_camid_search(padapter, pparm->addr, -1, -1)) >= 0) {
			RTW_PRINT("clear key for addr:"MAC_FMT", camid:%d\n", MAC_ARG(pparm->addr), cam_id);
			clear_cam_entry(padapter, cam_id);
			rtw_camid_free(padapter, cam_id);
		}
	} else {
		RTW_PRINT("set pairwise key camid:%d, addr:"MAC_FMT", kid:%d, type:%s\n",
			cam_id, MAC_ARG(pparm->addr), pparm->keyid, security_type_str(pparm->algorithm));
		ctrl = BIT(15) | ((pparm->algorithm) << 2) | pparm->keyid;
		write_cam(padapter, cam_id, ctrl, pparm->addr, pparm->key);
	}
	ret = H2C_SUCCESS_RSP;

exit:
	return ret;
}

u8 add_ba_hdl(_adapter *padapter, unsigned char *pbuf)
{
	struct addBaReq_parm	*pparm = (struct addBaReq_parm *)pbuf;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	struct sta_info *psta = rtw_get_stainfo(&padapter->stapriv, pparm->addr);

	if (!psta)
		return	H2C_SUCCESS;

	if (((pmlmeinfo->state & WIFI_FW_ASSOC_SUCCESS) && (pmlmeinfo->HT_enable)) ||
	    ((pmlmeinfo->state & 0x03) == WIFI_FW_AP_STATE)) {
		/* pmlmeinfo->ADDBA_retry_count = 0; */
		/* pmlmeinfo->candidate_tid_bitmap |= (0x1 << pparm->tid);		 */
		/* psta->htpriv.candidate_tid_bitmap |= BIT(pparm->tid); */
		issue_addba_req(padapter, pparm->addr, (u8)pparm->tid);
		/* _set_timer(&pmlmeext->ADDBA_timer, ADDBA_TO); */
		_set_timer(&psta->addba_retry_timer, ADDBA_TO);
	}
#ifdef CONFIG_TDLS
	else if ((psta->tdls_sta_state & TDLS_LINKED_STATE) &&
		 (psta->htpriv.ht_option) &&
		 (psta->htpriv.ampdu_enable)) {
		issue_addba_req(padapter, pparm->addr, (u8)pparm->tid);
		_set_timer(&psta->addba_retry_timer, ADDBA_TO);
	}
#endif /* CONFIG */
	else
		psta->htpriv.candidate_tid_bitmap &= ~BIT(pparm->tid);
	return	H2C_SUCCESS;
}


u8 add_ba_rsp_hdl(_adapter *padapter, unsigned char *pbuf)
{
	struct addBaRsp_parm *pparm = (struct addBaRsp_parm *)pbuf;
	u8 ret = true, i = 0, try_cnt = 3, wait_ms = 50;
	struct recv_reorder_ctrl *preorder_ctrl;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct sta_info *psta;

	psta = rtw_get_stainfo(pstapriv, pparm->addr);
	if (!psta)
		goto exit;

	preorder_ctrl = &psta->recvreorder_ctrl[pparm->tid];
	ret = issue_addba_rsp_wait_ack(padapter, pparm->addr, pparm->tid, pparm->status, pparm->size, 3, 50);

#ifdef CONFIG_UPDATE_INDICATE_SEQ_WHILE_PROCESS_ADDBA_REQ
	/* status = 0 means accept this addba req, so update indicate seq = start_seq under this compile flag */
	if (pparm->status == 0) {
		preorder_ctrl->indicate_seq = pparm->start_seq;
#ifdef DBG_RX_SEQ
		RTW_INFO("DBG_RX_SEQ %s:%d IndicateSeq: %d, start_seq: %d\n", __func__, __LINE__,
			 preorder_ctrl->indicate_seq, pparm->start_seq);
#endif
	}
#else
	preorder_ctrl->indicate_seq = 0xffff;
#endif

	/*
	  * status = 0 means accept this addba req
	  * status = 37 means reject this addba req
	  */
	if (pparm->status == 0) {
		preorder_ctrl->enable = true;
		preorder_ctrl->ampdu_size = pparm->size;
	} else if (pparm->status == 37)
		preorder_ctrl->enable = false;

exit:
	return H2C_SUCCESS;
}

u8 chk_bmc_sleepq_cmd(_adapter *padapter)
{
	struct cmd_obj *ph2c;
	struct cmd_priv *pcmdpriv = &(padapter->cmdpriv);
	u8 res = _SUCCESS;


	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}

	init_h2fwcmd_w_parm_no_parm_rsp(ph2c, GEN_CMD_CODE(_ChkBMCSleepq));

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);

exit:


	return res;
}

u8 set_tx_beacon_cmd(_adapter *padapter)
{
	struct cmd_obj	*ph2c;
	struct Tx_Beacon_param	*ptxBeacon_parm;
	struct cmd_priv	*pcmdpriv = &(padapter->cmdpriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	u8	res = _SUCCESS;
	int len_diff = 0;


	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}

	ptxBeacon_parm = (struct Tx_Beacon_param *)rtw_zmalloc(sizeof(struct Tx_Beacon_param));
	if (ptxBeacon_parm == NULL) {
		rtw_mfree((unsigned char *)ph2c, sizeof(struct	cmd_obj));
		res = _FAIL;
		goto exit;
	}

	memcpy(&(ptxBeacon_parm->network), &(pmlmeinfo->network), sizeof(WLAN_BSSID_EX));

	len_diff = update_hidden_ssid(
			   ptxBeacon_parm->network.IEs + _BEACON_IE_OFFSET_
		   , ptxBeacon_parm->network.IELength - _BEACON_IE_OFFSET_
			   , pmlmeinfo->hidden_ssid_mode
		   );
	ptxBeacon_parm->network.IELength += len_diff;

	init_h2fwcmd_w_parm_no_rsp(ph2c, ptxBeacon_parm, GEN_CMD_CODE(_TX_Beacon));

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);


exit:


	return res;
}


u8 mlme_evt_hdl(_adapter *padapter, unsigned char *pbuf)
{
	u8 evt_code, evt_seq;
	u16 evt_sz;
	uint	*peventbuf;
	void (*event_callback)(_adapter *dev, u8 *pbuf);
	struct evt_priv *pevt_priv = &(padapter->evtpriv);

	if (pbuf == NULL)
		goto _abort_event_;

	peventbuf = (uint *)pbuf;
	evt_sz = (u16)(*peventbuf & 0xffff);
	evt_seq = (u8)((*peventbuf >> 24) & 0x7f);
	evt_code = (u8)((*peventbuf >> 16) & 0xff);


#ifdef CHECK_EVENT_SEQ
	/* checking event sequence...		 */
	if (evt_seq != (ATOMIC_READ(&pevt_priv->event_seq) & 0x7f)) {

		pevt_priv->event_seq = (evt_seq + 1) & 0x7f;

		goto _abort_event_;
	}
#endif

	/* checking if event code is valid */
	if (evt_code >= MAX_C2HEVT) {
		goto _abort_event_;
	}

	/* checking if event size match the event parm size	 */
	if ((wlanevents[evt_code].parmsize != 0) &&
	    (wlanevents[evt_code].parmsize != evt_sz)) {

		goto _abort_event_;

	}

	ATOMIC_INC(&pevt_priv->event_seq);

	peventbuf += 2;

	if (peventbuf) {
		event_callback = wlanevents[evt_code].event_callback;
		event_callback(padapter, (u8 *)peventbuf);

		pevt_priv->evt_done_cnt++;
	}


_abort_event_:


	return H2C_SUCCESS;

}

u8 h2c_msg_hdl(_adapter *padapter, unsigned char *pbuf)
{
	if (!pbuf)
		return H2C_PARAMETERS_ERROR;

	return H2C_SUCCESS;
}

u8 chk_bmc_sleepq_hdl(_adapter *padapter, unsigned char *pbuf)
{
#ifdef CONFIG_AP_MODE
	unsigned long irqL;
	struct sta_info *psta_bmc;
	_list	*xmitframe_plist, *xmitframe_phead;
	struct xmit_frame *pxmitframe = NULL;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct sta_priv  *pstapriv = &padapter->stapriv;

	/* for BC/MC Frames */
	psta_bmc = rtw_get_bcmc_stainfo(padapter);
	if (!psta_bmc)
		return H2C_SUCCESS;

	if ((pstapriv->tim_bitmap & BIT(0)) && (psta_bmc->sleepq_len > 0)) {
#ifndef CONFIG_PCI_HCI
		rtw_msleep_os(10);/* 10ms, ATIM(HIQ) Windows */
#endif
		/* _enter_critical_bh(&psta_bmc->sleep_q.lock, &irqL); */
		_enter_critical_bh(&pxmitpriv->lock, &irqL);

		xmitframe_phead = get_list_head(&psta_bmc->sleep_q);
		xmitframe_plist = get_next(xmitframe_phead);

		while ((rtw_end_of_queue_search(xmitframe_phead, xmitframe_plist)) == false) {
			pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);

			xmitframe_plist = get_next(xmitframe_plist);

			list_del_init(&pxmitframe->list);

			psta_bmc->sleepq_len--;
			if (psta_bmc->sleepq_len > 0)
				pxmitframe->attrib.mdata = 1;
			else
				pxmitframe->attrib.mdata = 0;

			pxmitframe->attrib.triggered = 1;

			if (xmitframe_hiq_filter(pxmitframe))
				pxmitframe->attrib.qsel = QSLT_HIGH;/* HIQ */

			rtw_hal_xmitframe_enqueue(padapter, pxmitframe);
		}

		_exit_critical_bh(&pxmitpriv->lock, &irqL);

		if (rtw_get_intf_type(padapter) != RTW_PCIE) {
			/* check hi queue and bmc_sleepq */
			rtw_chk_hi_queue_cmd(padapter);
		}
	}
#endif

	return H2C_SUCCESS;
}

u8 tx_beacon_hdl(_adapter *padapter, unsigned char *pbuf)
{

#ifdef CONFIG_SWTIMER_BASED_TXBCN

	tx_beacon_handler(padapter->dvobj);

#else

	if (send_beacon(padapter) == _FAIL) {
		RTW_INFO("issue_beacon, fail!\n");
		return H2C_PARAMETERS_ERROR;
	}

	/* tx bc/mc frames after update TIM */
	chk_bmc_sleepq_hdl(padapter, NULL);
#endif

	return H2C_SUCCESS;
}

/*
* according to channel
* add/remove WLAN_BSSID_EX.IEs's ERP ie
* set WLAN_BSSID_EX.SupportedRates
* update WLAN_BSSID_EX.IEs's Supported Rate and Extended Supported Rate ie
*/
void change_band_update_ie(_adapter *padapter, WLAN_BSSID_EX *pnetwork, u8 ch)
{
	u8	network_type, rate_len, total_rate_len, remainder_rate_len;
	struct mlme_ext_priv *pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info *pmlmeinfo = &(pmlmeext->mlmext_info);
	u8	erpinfo = 0x4;

	if (ch >= 36) {
		network_type = WIRELESS_11A;
		total_rate_len = IEEE80211_NUM_OFDM_RATESLEN;
		rtw_remove_bcn_ie(padapter, pnetwork, _ERPINFO_IE_);
	} else {
		network_type = WIRELESS_11BG;
		total_rate_len = IEEE80211_CCK_RATE_LEN + IEEE80211_NUM_OFDM_RATESLEN;
		rtw_add_bcn_ie(padapter, pnetwork, _ERPINFO_IE_, &erpinfo, 1);
	}

	rtw_set_supported_rate(pnetwork->SupportedRates, network_type);

	UpdateBrateTbl(padapter, pnetwork->SupportedRates);

	if (total_rate_len > 8) {
		rate_len = 8;
		remainder_rate_len = total_rate_len - 8;
	} else {
		rate_len = total_rate_len;
		remainder_rate_len = 0;
	}

	rtw_add_bcn_ie(padapter, pnetwork, _SUPPORTEDRATES_IE_, pnetwork->SupportedRates, rate_len);

	if (remainder_rate_len)
		rtw_add_bcn_ie(padapter, pnetwork, _EXT_SUPPORTEDRATES_IE_, (pnetwork->SupportedRates + 8), remainder_rate_len);
	else
		rtw_remove_bcn_ie(padapter, pnetwork, _EXT_SUPPORTEDRATES_IE_);

	pnetwork->Length = get_WLAN_BSSID_EX_sz(pnetwork);
}

void rtw_join_done_chk_ch(_adapter *adapter, int join_res)
{
#define DUMP_ADAPTERS_STATUS 0

	struct dvobj_priv *dvobj;
	_adapter *iface;
	struct mlme_priv *mlme;
	struct mlme_ext_priv *mlmeext;
	u8 u_ch, u_offset, u_bw;
	int i;

	dvobj = adapter_to_dvobj(adapter);

	if (DUMP_ADAPTERS_STATUS) {
		RTW_INFO(FUNC_ADPT_FMT" enter\n", FUNC_ADPT_ARG(adapter));
		dump_adapters_status(RTW_DBGDUMP , dvobj);
	}

	if (join_res >= 0) {

#ifdef CONFIG_MCC_MODE
		/* MCC setting success, don't go to ch union process */
		if (rtw_hal_set_mcc_setting_join_done_chk_ch(adapter))
			return;
#endif /* CONFIG_MCC_MODE */

		if (rtw_mi_get_ch_setting_union(adapter, &u_ch, &u_bw, &u_offset) <= 0) {
			dump_adapters_status(RTW_DBGDUMP , dvobj);
			rtw_warn_on(1);
		}

		for (i = 0; i < dvobj->iface_nums; i++) {
			iface = dvobj->padapters[i];
			mlme = &iface->mlmepriv;
			mlmeext = &iface->mlmeextpriv;

			if (!iface || iface == adapter)
				continue;

			if (check_fwstate(mlme, WIFI_AP_STATE)
			    && check_fwstate(mlme, WIFI_ASOC_STATE)
			   ) {
				bool is_grouped = rtw_is_chbw_grouped(u_ch, u_bw, u_offset
					, mlmeext->cur_channel, mlmeext->cur_bwmode, mlmeext->cur_ch_offset);

				if (is_grouped == false) {
					/* handle AP which need to switch ch setting */

					/* restore original bw, adjust bw by registry setting on target ch */
					mlmeext->cur_bwmode = mlme->ori_bw;
					mlmeext->cur_channel = u_ch;
					rtw_adjust_chbw(iface
						, mlmeext->cur_channel, &mlmeext->cur_bwmode, &mlmeext->cur_ch_offset);

					rtw_sync_chbw(&mlmeext->cur_channel, &mlmeext->cur_bwmode, &mlmeext->cur_ch_offset
						, &u_ch, &u_bw, &u_offset);

					rtw_ap_update_bss_chbw(iface, &(mlmeext->mlmext_info.network)
						, mlmeext->cur_channel, mlmeext->cur_bwmode, mlmeext->cur_ch_offset);

					memcpy(&(mlme->cur_network.network), &(mlmeext->mlmext_info.network), sizeof(WLAN_BSSID_EX));

					rtw_start_bss_hdl_after_chbw_decided(iface);
				}

				update_beacon(iface, 0, NULL, true);
			}

			clr_fwstate(mlme, WIFI_OP_CH_SWITCHING);
		}

#ifdef CONFIG_DFS_MASTER
		rtw_dfs_master_status_apply(adapter, MLME_STA_CONNECTED);
#endif
	} else {
		for (i = 0; i < dvobj->iface_nums; i++) {
			iface = dvobj->padapters[i];
			mlme = &iface->mlmepriv;
			mlmeext = &iface->mlmeextpriv;

			if (!iface || iface == adapter)
				continue;

			if (check_fwstate(mlme, WIFI_AP_STATE)
			    && check_fwstate(mlme, WIFI_ASOC_STATE))
				update_beacon(iface, 0, NULL, true);

			clr_fwstate(mlme, WIFI_OP_CH_SWITCHING);
		}
#ifdef CONFIG_DFS_MASTER
		rtw_dfs_master_status_apply(adapter, MLME_STA_DISCONNECTED);
#endif
	}

	if (rtw_mi_get_ch_setting_union(adapter, &u_ch, &u_bw, &u_offset)) {
		set_channel_bwmode(adapter, u_ch, u_offset, u_bw);
		rtw_mi_update_union_chan_inf(adapter, u_ch, u_offset, u_bw);
	}

	if (DUMP_ADAPTERS_STATUS) {
		RTW_INFO(FUNC_ADPT_FMT" exit\n", FUNC_ADPT_ARG(adapter));
		dump_adapters_status(RTW_DBGDUMP , dvobj);
	}
}

int rtw_chk_start_clnt_join(_adapter *adapter, u8 *ch, u8 *bw, u8 *offset)
{
	bool chbw_allow = true;
	bool connect_allow = true;
	struct mlme_ext_priv	*pmlmeext = &adapter->mlmeextpriv;
	u8 cur_ch, cur_bw, cur_ch_offset;
	u8 u_ch, u_offset, u_bw;

	u_ch = cur_ch = pmlmeext->cur_channel;
	u_bw = cur_bw = pmlmeext->cur_bwmode;
	u_offset = cur_ch_offset = pmlmeext->cur_ch_offset;

	if (!ch || !bw || !offset) {
		connect_allow = false;
		rtw_warn_on(1);
		goto exit;
	}

	if (cur_ch == 0) {
		connect_allow = false;
		RTW_ERR(FUNC_ADPT_FMT" cur_ch:%u\n"
			, FUNC_ADPT_ARG(adapter), cur_ch);
		rtw_warn_on(1);
		goto exit;
	}
	RTW_INFO(FUNC_ADPT_FMT" req: %u,%u,%u\n", FUNC_ADPT_ARG(adapter), u_ch, u_bw, u_offset);

#ifdef CONFIG_CONCURRENT_MODE
	{
		struct dvobj_priv *dvobj;
		_adapter *iface;
		struct mlme_priv *mlme;
		struct mlme_ext_priv *mlmeext;
		struct mi_state mstate;
		int i;

		dvobj = adapter_to_dvobj(adapter);

		rtw_mi_status_no_self(adapter, &mstate);
		RTW_INFO(FUNC_ADPT_FMT" ld_sta_num:%u, ap_num:%u\n"
			 , FUNC_ADPT_ARG(adapter), MSTATE_STA_LD_NUM(&mstate), MSTATE_AP_NUM(&mstate));

		if (!MSTATE_STA_LD_NUM(&mstate) && !MSTATE_AP_NUM(&mstate)) {
			/* consider linking STA? */
			goto connect_allow_hdl;
		}

		if (rtw_mi_get_ch_setting_union_no_self(adapter, &u_ch, &u_bw, &u_offset) <= 0) {
			dump_adapters_status(RTW_DBGDUMP , dvobj);
			rtw_warn_on(1);
		}
		RTW_INFO(FUNC_ADPT_FMT" union no self: %u,%u,%u\n"
			 , FUNC_ADPT_ARG(adapter), u_ch, u_bw, u_offset);

		/* chbw_allow? */
		chbw_allow = rtw_is_chbw_grouped(pmlmeext->cur_channel, pmlmeext->cur_bwmode, pmlmeext->cur_ch_offset
						 , u_ch, u_bw, u_offset);

		RTW_INFO(FUNC_ADPT_FMT" chbw_allow:%d\n"
			 , FUNC_ADPT_ARG(adapter), chbw_allow);

#ifdef CONFIG_MCC_MODE
		/* check setting success, don't go to ch union process */
		if (rtw_hal_set_mcc_setting_chk_start_clnt_join(adapter, &u_ch, &u_bw, &u_offset, chbw_allow))
			goto exit;
#endif

		if (chbw_allow) {
			rtw_sync_chbw(&cur_ch, &cur_bw, &cur_ch_offset, &u_ch, &u_bw, &u_offset);
			rtw_warn_on(cur_ch != pmlmeext->cur_channel);
			rtw_warn_on(cur_bw != pmlmeext->cur_bwmode);
			rtw_warn_on(cur_ch_offset != pmlmeext->cur_ch_offset);
			goto connect_allow_hdl;
		}

#ifdef CONFIG_CFG80211_ONECHANNEL_UNDER_CONCURRENT
		/* chbw_allow is false, connect allow? */
		for (i = 0; i < dvobj->iface_nums; i++) {
			iface = dvobj->padapters[i];
			mlme = &iface->mlmepriv;
			mlmeext = &iface->mlmeextpriv;

			if (check_fwstate(mlme, WIFI_STATION_STATE)
			    && check_fwstate(mlme, WIFI_ASOC_STATE)
#if defined(CONFIG_P2P)
			    && rtw_p2p_chk_state(&(iface->wdinfo), P2P_STATE_NONE)
#endif
			   ) {
				connect_allow = false;
				break;
			}
		}
#endif /* CONFIG_CFG80211_ONECHANNEL_UNDER_CONCURRENT */

		if (MSTATE_STA_LD_NUM(&mstate) + MSTATE_AP_LD_NUM(&mstate) >= 2)
			connect_allow = false;

		RTW_INFO(FUNC_ADPT_FMT" connect_allow:%d\n"
			 , FUNC_ADPT_ARG(adapter), connect_allow);

		if (connect_allow == false)
			goto exit;

connect_allow_hdl:
		/* connect_allow */

#ifdef CONFIG_DFS_MASTER
		rtw_dfs_master_status_apply(adapter, MLME_STA_CONNECTING);
#endif

		if (chbw_allow == false) {
			u_ch = cur_ch;
			u_bw = cur_bw;
			u_offset = cur_ch_offset;

			for (i = 0; i < dvobj->iface_nums; i++) {
				iface = dvobj->padapters[i];
				mlme = &iface->mlmepriv;
				mlmeext = &iface->mlmeextpriv;

				if (!iface || iface == adapter)
					continue;

				if (check_fwstate(mlme, WIFI_AP_STATE)
				    && check_fwstate(mlme, WIFI_ASOC_STATE)
				   ) {
#ifdef CONFIG_SPCT_CH_SWITCH
					if (1)
						rtw_ap_inform_ch_switch(iface, pmlmeext->cur_channel , pmlmeext->cur_ch_offset);
					else
#endif
						rtw_sta_flush(iface, false);

					rtw_hal_set_hwreg(iface, HW_VAR_CHECK_TXBUF, 0);
					set_fwstate(mlme, WIFI_OP_CH_SWITCHING);
				} else if (check_fwstate(mlme, WIFI_STATION_STATE)
					&& check_fwstate(mlme, WIFI_ASOC_STATE)
					  ) {
					rtw_disassoc_cmd(iface, 500, false);
					rtw_indicate_disconnect(iface, 0, false);
					rtw_free_assoc_resources(iface, 1);
				}
			}
		}
	}
#endif /* CONFIG_CONCURRENT_MODE */

exit:

	if (connect_allow) {
		RTW_INFO(FUNC_ADPT_FMT" union: %u,%u,%u\n", FUNC_ADPT_ARG(adapter), u_ch, u_bw, u_offset);
		*ch = u_ch;
		*bw = u_bw;
		*offset = u_offset;
	}

	return connect_allow ? _SUCCESS : _FAIL;
}


u8 set_ch_hdl(_adapter *padapter, u8 *pbuf)
{
	struct set_ch_parm *set_ch_parm;
	struct mlme_priv		*pmlmepriv = &padapter->mlmepriv;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;

	if (!pbuf)
		return H2C_PARAMETERS_ERROR;

	set_ch_parm = (struct set_ch_parm *)pbuf;

	RTW_INFO(FUNC_NDEV_FMT" ch:%u, bw:%u, ch_offset:%u\n",
		 FUNC_NDEV_ARG(padapter->pnetdev),
		 set_ch_parm->ch, set_ch_parm->bw, set_ch_parm->ch_offset);

	pmlmeext->cur_channel = set_ch_parm->ch;
	pmlmeext->cur_ch_offset = set_ch_parm->ch_offset;
	pmlmeext->cur_bwmode = set_ch_parm->bw;

	set_channel_bwmode(padapter, set_ch_parm->ch, set_ch_parm->ch_offset, set_ch_parm->bw);

	return	H2C_SUCCESS;
}

u8 set_chplan_hdl(_adapter *padapter, unsigned char *pbuf)
{
	struct SetChannelPlan_param *setChannelPlan_param;
	struct mlme_priv *mlme = &padapter->mlmepriv;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;

	if (!pbuf)
		return H2C_PARAMETERS_ERROR;

	setChannelPlan_param = (struct SetChannelPlan_param *)pbuf;

	if (!rtw_is_channel_plan_valid(setChannelPlan_param->channel_plan))
		return H2C_PARAMETERS_ERROR;

	mlme->country_ent = setChannelPlan_param->country_ent;
	mlme->ChannelPlan = setChannelPlan_param->channel_plan;

	pmlmeext->max_chan_nums = init_channel_set(padapter, setChannelPlan_param->channel_plan, pmlmeext->channel_set);
	init_channel_list(padapter, pmlmeext->channel_set, pmlmeext->max_chan_nums, &pmlmeext->channel_list);

	rtw_hal_set_odm_var(padapter, HAL_ODM_REGULATION, NULL, true);

#ifdef CONFIG_IOCTL_CFG80211
	rtw_reg_notify_by_driver(padapter);
#endif /* CONFIG_IOCTL_CFG80211 */

	return	H2C_SUCCESS;
}

u8 led_blink_hdl(_adapter *padapter, unsigned char *pbuf)
{
	struct LedBlink_param *ledBlink_param;

	if (!pbuf)
		return H2C_PARAMETERS_ERROR;

	ledBlink_param = (struct LedBlink_param *)pbuf;

#ifdef CONFIG_LED_HANDLED_BY_CMD_THREAD
	BlinkHandler((PLED_DATA)ledBlink_param->pLed);
#endif

	return	H2C_SUCCESS;
}

u8 set_csa_hdl(_adapter *padapter, unsigned char *pbuf)
{
#ifdef CONFIG_DFS
	struct SetChannelSwitch_param *setChannelSwitch_param;
	u8 new_ch_no;
	u8 gval8 = 0x00, sval8 = 0xff;

	if (!pbuf)
		return H2C_PARAMETERS_ERROR;

	setChannelSwitch_param = (struct SetChannelSwitch_param *)pbuf;
	new_ch_no = setChannelSwitch_param->new_ch_no;

	rtw_hal_get_hwreg(padapter, HW_VAR_TXPAUSE, &gval8);

	rtw_hal_set_hwreg(padapter, HW_VAR_TXPAUSE, &sval8);

	RTW_INFO("DFS detected! Swiching channel to %d!\n", new_ch_no);
	set_channel_bwmode(padapter, new_ch_no, HAL_PRIME_CHNL_OFFSET_DONT_CARE, CHANNEL_WIDTH_20);

	rtw_hal_set_hwreg(padapter, HW_VAR_TXPAUSE, &gval8);

	rtw_disassoc_cmd(padapter, 0, false);
	rtw_indicate_disconnect(padapter, 0, false);
	rtw_free_assoc_resources(padapter, 1);
	rtw_free_network_queue(padapter, true);

	if (((new_ch_no >= 52) && (new_ch_no <= 64)) || ((new_ch_no >= 100) && (new_ch_no <= 140)))
		RTW_INFO("Switched to DFS band (ch %02x) again!!\n", new_ch_no);

	return	H2C_SUCCESS;
#else
	return	H2C_REJECTED;
#endif /* CONFIG_DFS */

}

u8 tdls_hdl(_adapter *padapter, unsigned char *pbuf)
{
#ifdef CONFIG_TDLS
	unsigned long irqL;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct tdls_info *ptdlsinfo = &padapter->tdlsinfo;
#ifdef CONFIG_TDLS_CH_SW
	struct tdls_ch_switch *pchsw_info = &ptdlsinfo->chsw_info;
#endif
	struct TDLSoption_param *TDLSoption;
	struct sta_info *ptdls_sta = NULL;
	struct mlme_ext_priv *pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info *pmlmeinfo = &pmlmeext->mlmext_info;
	u8 survey_channel, i, min, option;
	struct tdls_txmgmt txmgmt;
	u32 setchtime, resp_sleep = 0, wait_time;
	u8 zaddr[ETH_ALEN] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
	u8 ret;
	u8 doiqk;

	if (!pbuf)
		return H2C_PARAMETERS_ERROR;

	TDLSoption = (struct TDLSoption_param *)pbuf;
	option = TDLSoption->option;

	if (memcmp(TDLSoption->addr, zaddr, ETH_ALEN)) {
		ptdls_sta = rtw_get_stainfo(&(padapter->stapriv), TDLSoption->addr);
		if (ptdls_sta == NULL)
			return H2C_REJECTED;
	} else {
		if (!(option == TDLS_RS_RCR))
			return H2C_REJECTED;
	}

	/* _enter_critical_bh(&(ptdlsinfo->hdl_lock), &irqL); */
	/* RTW_INFO("[%s] option:%d\n", __func__, option); */

	switch (option) {
	case TDLS_ESTABLISHED: {
		/* As long as TDLS handshake success, we should set RCR_CBSSID_DATA bit to 0 */
		/* So we can receive all kinds of data frames. */
		u8 sta_band = 0;

		/* leave ALL PS when TDLS is established */
		rtw_pwr_wakeup(padapter);

		rtw_hal_set_hwreg(padapter, HW_VAR_TDLS_WRCR, 0);
		RTW_INFO("Created Direct Link with "MAC_FMT"\n", MAC_ARG(ptdls_sta->hwaddr));

		/* Set TDLS sta rate. */
		/* Update station supportRate */
		rtw_hal_update_sta_rate_mask(padapter, ptdls_sta);
		if (pmlmeext->cur_channel > 14) {
			if (ptdls_sta->ra_mask & 0xffff000)
				sta_band |= WIRELESS_11_5N ;

			if (ptdls_sta->ra_mask & 0xff0)
				sta_band |= WIRELESS_11A;
		} else {
			/* 5G band */
			if (ptdls_sta->ra_mask & 0xffff000)
				sta_band |= WIRELESS_11_24N;

			if (ptdls_sta->ra_mask & 0xff0)
				sta_band |= WIRELESS_11G;

			if (ptdls_sta->ra_mask & 0x0f)
				sta_band |= WIRELESS_11B;
		}
		ptdls_sta->wireless_mode = sta_band;
		ptdls_sta->raid = rtw_hal_networktype_to_raid(padapter, ptdls_sta);
		set_sta_rate(padapter, ptdls_sta);
		rtw_sta_media_status_rpt(padapter, ptdls_sta, 1);
		/* Sta mode */
		rtw_hal_set_odm_var(padapter, HAL_ODM_STA_INFO, ptdls_sta, true);
		break;
	}
	case TDLS_ISSUE_PTI:
		ptdls_sta->tdls_sta_state |= TDLS_WAIT_PTR_STATE;
		issue_tdls_peer_traffic_indication(padapter, ptdls_sta);
		_set_timer(&ptdls_sta->pti_timer, TDLS_PTI_TIME);
		break;
#ifdef CONFIG_TDLS_CH_SW
	case TDLS_CH_SW_RESP:
		memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
		txmgmt.status_code = 0;
		memcpy(txmgmt.peer, ptdls_sta->hwaddr, ETH_ALEN);

		issue_nulldata(padapter, NULL, 1, 3, 3);

		RTW_INFO("[TDLS ] issue tdls channel switch response\n");
		ret = issue_tdls_ch_switch_rsp(padapter, &txmgmt, true);

		/* If we receive TDLS_CH_SW_REQ at off channel which it's target is AP's channel */
		/* then we just switch to AP's channel*/
		if (padapter->mlmeextpriv.cur_channel == pchsw_info->off_ch_num) {
			rtw_tdls_cmd(padapter, ptdls_sta->hwaddr, TDLS_CH_SW_END_TO_BASE_CHNL);
			break;
		}

		if (ret == _SUCCESS)
			rtw_tdls_cmd(padapter, ptdls_sta->hwaddr, TDLS_CH_SW_TO_OFF_CHNL);
		else
			RTW_INFO("[TDLS] issue_tdls_ch_switch_rsp wait ack fail !!!!!!!!!!\n");

		break;
	case TDLS_CH_SW_PREPARE:
		pchsw_info->ch_sw_state |= TDLS_CH_SWITCH_PREPARE_STATE;

		/* to collect IQK info of off-chnl */
		doiqk = true;
		rtw_hal_set_hwreg(padapter, HW_VAR_DO_IQK, &doiqk);
		set_channel_bwmode(padapter, pchsw_info->off_ch_num, pchsw_info->ch_offset, (pchsw_info->ch_offset) ? CHANNEL_WIDTH_40 : CHANNEL_WIDTH_20);
		doiqk = false;
		rtw_hal_set_hwreg(padapter, HW_VAR_DO_IQK, &doiqk);

		/* switch back to base-chnl */
		set_channel_bwmode(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode);

		rtw_tdls_cmd(padapter, ptdls_sta->hwaddr, TDLS_CH_SW_START);

		pchsw_info->ch_sw_state &= ~(TDLS_CH_SWITCH_PREPARE_STATE);

		break;
	case TDLS_CH_SW_START:
		rtw_tdls_set_ch_sw_oper_control(padapter, true);
		break;
	case TDLS_CH_SW_TO_OFF_CHNL:
		issue_nulldata(padapter, NULL, 1, 3, 3);

		if (!(pchsw_info->ch_sw_state & TDLS_CH_SW_INITIATOR_STATE))
			_set_timer(&ptdls_sta->ch_sw_timer, (u32)(ptdls_sta->ch_switch_timeout) / 1000);

		if (rtw_tdls_do_ch_sw(padapter, ptdls_sta, TDLS_CH_SW_OFF_CHNL, pchsw_info->off_ch_num,
			pchsw_info->ch_offset, (pchsw_info->ch_offset) ? CHANNEL_WIDTH_40 : CHANNEL_WIDTH_20, ptdls_sta->ch_switch_time) == _SUCCESS) {
			pchsw_info->ch_sw_state &= ~(TDLS_PEER_AT_OFF_STATE);
			if (pchsw_info->ch_sw_state & TDLS_CH_SW_INITIATOR_STATE) {
				if (issue_nulldata_to_TDLS_peer_STA(ptdls_sta->padapter, ptdls_sta->hwaddr, 0, 1, 3) == _FAIL)
					rtw_tdls_cmd(padapter, ptdls_sta->hwaddr, TDLS_CH_SW_TO_BASE_CHNL);
			}
		} else {
			u8 bcancelled;

			if (!(pchsw_info->ch_sw_state & TDLS_CH_SW_INITIATOR_STATE))
				_cancel_timer(&ptdls_sta->ch_sw_timer, &bcancelled);
		}


		break;
	case TDLS_CH_SW_END:
	case TDLS_CH_SW_END_TO_BASE_CHNL:
		rtw_tdls_set_ch_sw_oper_control(padapter, false);
		_cancel_timer_ex(&ptdls_sta->ch_sw_timer);
		_cancel_timer_ex(&ptdls_sta->stay_on_base_chnl_timer);
		_cancel_timer_ex(&ptdls_sta->ch_sw_monitor_timer);
		if (option == TDLS_CH_SW_END_TO_BASE_CHNL)
			rtw_tdls_cmd(padapter, ptdls_sta->hwaddr, TDLS_CH_SW_TO_BASE_CHNL);

		break;
	case TDLS_CH_SW_TO_BASE_CHNL_UNSOLICITED:
	case TDLS_CH_SW_TO_BASE_CHNL:
		pchsw_info->ch_sw_state &= ~(TDLS_PEER_AT_OFF_STATE | TDLS_WAIT_CH_RSP_STATE);

		if (option == TDLS_CH_SW_TO_BASE_CHNL_UNSOLICITED) {
			if (ptdls_sta != NULL) {
				/* Send unsolicited channel switch rsp. to peer */
				memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
				txmgmt.status_code = 0;
				memcpy(txmgmt.peer, ptdls_sta->hwaddr, ETH_ALEN);
				issue_tdls_ch_switch_rsp(padapter, &txmgmt, false);
			}
		}

		if (rtw_tdls_do_ch_sw(padapter, ptdls_sta, TDLS_CH_SW_BASE_CHNL, pmlmeext->cur_channel,
			pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode, ptdls_sta->ch_switch_time) == _SUCCESS) {
			issue_nulldata(padapter, NULL, 0, 3, 3);
			/* set ch sw monitor timer for responder */
			if (!(pchsw_info->ch_sw_state & TDLS_CH_SW_INITIATOR_STATE))
				_set_timer(&ptdls_sta->ch_sw_monitor_timer, TDLS_CH_SW_MONITOR_TIMEOUT);
		}

		break;
#endif
	case TDLS_RS_RCR:
		rtw_hal_set_hwreg(padapter, HW_VAR_TDLS_RS_RCR, 0);
		RTW_INFO("[TDLS] write REG_RCR, set bit6 on\n");
		break;
	case TDLS_TEARDOWN_STA:
		memset(&txmgmt, 0x00, sizeof(struct tdls_txmgmt));
		txmgmt.status_code = 0;
		memcpy(txmgmt.peer, ptdls_sta->hwaddr, ETH_ALEN);

		issue_tdls_teardown(padapter, &txmgmt, true);
		break;
	case TDLS_TEARDOWN_STA_LOCALLY:
#ifdef CONFIG_TDLS_CH_SW
		if (!memcmp(TDLSoption->addr, pchsw_info->addr, ETH_ALEN)) {
			pchsw_info->ch_sw_state &= ~(TDLS_CH_SW_INITIATOR_STATE |
						     TDLS_CH_SWITCH_ON_STATE |
						     TDLS_PEER_AT_OFF_STATE);
			rtw_tdls_set_ch_sw_oper_control(padapter, false);
			memset(pchsw_info->addr, 0x00, ETH_ALEN);
		}
#endif
		rtw_sta_media_status_rpt(padapter, ptdls_sta, 0);
		free_tdls_sta(padapter, ptdls_sta);
		break;
	}

	/* _exit_critical_bh(&(ptdlsinfo->hdl_lock), &irqL); */

	return H2C_SUCCESS;
#else
	return H2C_REJECTED;
#endif /* CONFIG_TDLS */

}

u8 run_in_thread_hdl(_adapter *padapter, u8 *pbuf)
{
	struct RunInThread_param *p;


	if (NULL == pbuf)
		return H2C_PARAMETERS_ERROR;
	p = (struct RunInThread_param *)pbuf;

	if (p->func)
		p->func(p->context);

	return H2C_SUCCESS;
}

u8 rtw_getmacreg_hdl(_adapter *padapter, u8 *pbuf)
{

	struct readMAC_parm *preadmacparm = NULL;
	u8 sz = 0;
	u32	addr = 0;
	u32	value = 0;

	if (!pbuf)
		return H2C_PARAMETERS_ERROR;

	preadmacparm = (struct readMAC_parm *) pbuf;
	sz = preadmacparm->len;
	addr = preadmacparm->addr;
	value = 0;

	switch (sz) {
	case 1:
		value = rtw_read8(padapter, addr);
		break;
	case 2:
		value = rtw_read16(padapter, addr);
		break;
	case 4:
		value = rtw_read32(padapter, addr);
		break;
	default:
		RTW_INFO("%s: Unknown size\n", __func__);
		break;
	}
	RTW_INFO("%s: addr:0x%02x valeu:0x%02x\n", __func__, addr, value);

	return H2C_SUCCESS;
}
