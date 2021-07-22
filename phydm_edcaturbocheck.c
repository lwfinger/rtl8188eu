// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


/* ************************************************************
 * include files
 * ************************************************************ */
#include "mp_precomp.h"
#include "phydm_precomp.h"

#if PHYDM_SUPPORT_EDCA

void
odm_edca_turbo_init(
	void		*p_dm_void)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	struct _ADAPTER	*adapter = p_dm_odm->adapter;
	p_dm_odm->dm_edca_table.is_current_turbo_edca = false;
	p_dm_odm->dm_edca_table.is_cur_rdl_state = false;
	adapter->recvpriv.is_any_non_be_pkts = false;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("Orginial VO PARAM: 0x%x\n", odm_read_4byte(p_dm_odm, ODM_EDCA_VO_PARAM)));
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("Orginial VI PARAM: 0x%x\n", odm_read_4byte(p_dm_odm, ODM_EDCA_VI_PARAM)));
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("Orginial BE PARAM: 0x%x\n", odm_read_4byte(p_dm_odm, ODM_EDCA_BE_PARAM)));
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("Orginial BK PARAM: 0x%x\n", odm_read_4byte(p_dm_odm, ODM_EDCA_BK_PARAM)));
}	/* ODM_InitEdcaTurbo */

void
odm_edca_turbo_check(
	void		*p_dm_void
)
{
	/*  */
	/* For AP/ADSL use struct rtl8192cd_priv* */
	/* For CE/NIC use struct _ADAPTER* */
	/*  */

	/*  */
	/* 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate */
	/* at the same time. In the stage2/3, we need to prive universal interface and merge all */
	/* HW dynamic mechanism. */
	/*  */
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("odm_edca_turbo_check========================>\n"));

	if (!(p_dm_odm->support_ability & ODM_MAC_EDCA_TURBO))
		return;

	switch	(p_dm_odm->support_platform) {
	case	ODM_WIN:
		break;
	case	ODM_CE:
		odm_edca_turbo_check_ce(p_dm_odm);
		break;
	}
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("<========================odm_edca_turbo_check\n"));

}	/* odm_CheckEdcaTurbo */

void
odm_edca_turbo_check_ce(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ADAPTER		*adapter = p_dm_odm->adapter;
	u32	EDCA_BE_UL = 0x5ea42b;/* Parameter suggested by Scott  */ /* edca_setting_UL[p_mgnt_info->iot_peer]; */
	u32	EDCA_BE_DL = 0x00a42b;/* Parameter suggested by Scott  */ /* edca_setting_DL[p_mgnt_info->iot_peer]; */
	u32	ic_type = p_dm_odm->support_ic_type;
	u32	iot_peer = 0;
	u8	wireless_mode = 0xFF;                 /* invalid value */
	u32	traffic_index;
	u32	edca_param;
	u64	cur_tx_bytes = 0;
	u64	cur_rx_bytes = 0;
	u8	bbtchange = true;
	u8	is_bias_on_rx = false;
	HAL_DATA_TYPE		*p_hal_data = GET_HAL_DATA(adapter);
	struct dvobj_priv		*pdvobjpriv = adapter_to_dvobj(adapter);
	struct xmit_priv		*pxmitpriv = &(adapter->xmitpriv);
	struct recv_priv		*precvpriv = &(adapter->recvpriv);
	struct registry_priv	*pregpriv = &adapter->registrypriv;
	struct mlme_ext_priv	*pmlmeext = &(adapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

	if (p_dm_odm->is_linked != true) {
		precvpriv->is_any_non_be_pkts = false;
		return;
	}

	if ((pregpriv->wifi_spec == 1)) { /* || (pmlmeinfo->HT_enable == 0)) */
		precvpriv->is_any_non_be_pkts = false;
		return;
	}

	if (p_dm_odm->p_wireless_mode != NULL)
		wireless_mode = *(p_dm_odm->p_wireless_mode);

	iot_peer = pmlmeinfo->assoc_AP_vendor;

	if (iot_peer >=  HT_IOT_PEER_MAX) {
		precvpriv->is_any_non_be_pkts = false;
		return;
	}

	if (p_dm_odm->support_ic_type & ODM_RTL8188E) {
		if ((iot_peer == HT_IOT_PEER_RALINK) || (iot_peer == HT_IOT_PEER_ATHEROS))
			is_bias_on_rx = true;
	}

	/* Check if the status needs to be changed. */
	if ((bbtchange) || (!precvpriv->is_any_non_be_pkts)) {
		cur_tx_bytes = pdvobjpriv->traffic_stat.cur_tx_bytes;
		cur_rx_bytes = pdvobjpriv->traffic_stat.cur_rx_bytes;

		/* traffic, TX or RX */
		if (is_bias_on_rx) {
			if (cur_tx_bytes > (cur_rx_bytes << 2)) {
				/* Uplink TP is present. */
				traffic_index = UP_LINK;
			} else {
				/* Balance TP is present. */
				traffic_index = DOWN_LINK;
			}
		} else {
			if (cur_rx_bytes > (cur_tx_bytes << 2)) {
				/* Downlink TP is present. */
				traffic_index = DOWN_LINK;
			} else {
				/* Balance TP is present. */
				traffic_index = UP_LINK;
			}
		}

		/* if ((p_dm_odm->dm_edca_table.prv_traffic_idx != traffic_index) || (!p_dm_odm->dm_edca_table.is_current_turbo_edca)) */
		{
			if (p_dm_odm->support_interface == ODM_ITRF_PCIE) {
				EDCA_BE_UL = 0x6ea42b;
				EDCA_BE_DL = 0x6ea42b;
			}

			/* 92D txop can't be set to 0x3e for cisco1250 */
			if ((iot_peer == HT_IOT_PEER_CISCO) && (wireless_mode == ODM_WM_N24G)) {
				EDCA_BE_DL = edca_setting_DL[iot_peer];
				EDCA_BE_UL = edca_setting_UL[iot_peer];
			}
			/* merge from 92s_92c_merge temp brunch v2445    20120215 */
			else if ((iot_peer == HT_IOT_PEER_CISCO) && ((wireless_mode == ODM_WM_G) || (wireless_mode == (ODM_WM_B | ODM_WM_G)) || (wireless_mode == ODM_WM_A) || (wireless_mode == ODM_WM_B)))
				EDCA_BE_DL = edca_setting_dl_g_mode[iot_peer];
			else if ((iot_peer == HT_IOT_PEER_AIRGO) && ((wireless_mode == ODM_WM_G) || (wireless_mode == ODM_WM_A)))
				EDCA_BE_DL = 0xa630;
			else if (iot_peer == HT_IOT_PEER_MARVELL) {
				EDCA_BE_DL = edca_setting_DL[iot_peer];
				EDCA_BE_UL = edca_setting_UL[iot_peer];
			} else if (iot_peer == HT_IOT_PEER_ATHEROS) {
				/* Set DL EDCA for Atheros peer to 0x3ea42b. Suggested by SD3 Wilson for ASUS TP issue. */
				EDCA_BE_DL = edca_setting_DL[iot_peer];
			}

			if ((ic_type == ODM_RTL8812) || (ic_type == ODM_RTL8821) || (ic_type == ODM_RTL8192E)) { /* add 8812AU/8812AE */
				EDCA_BE_UL = 0x5ea42b;
				EDCA_BE_DL = 0x5ea42b;

				ODM_RT_TRACE(p_dm_odm, ODM_COMP_EDCA_TURBO, ODM_DBG_LOUD, ("8812A: EDCA_BE_UL=0x%x EDCA_BE_DL =0x%x", EDCA_BE_UL, EDCA_BE_DL));
			}

			if (traffic_index == DOWN_LINK)
				edca_param = EDCA_BE_DL;
			else
				edca_param = EDCA_BE_UL;

			rtw_write32(adapter, REG_EDCA_BE_PARAM, edca_param);

			p_dm_odm->dm_edca_table.prv_traffic_idx = traffic_index;
		}

		p_dm_odm->dm_edca_table.is_current_turbo_edca = true;
	} else {
		/*  */
		/* Turn Off EDCA turbo here. */
		/* Restore original EDCA according to the declaration of AP. */
		/*  */
		if (p_dm_odm->dm_edca_table.is_current_turbo_edca) {
			rtw_write32(adapter, REG_EDCA_BE_PARAM, p_hal_data->ac_param_be);
			p_dm_odm->dm_edca_table.is_current_turbo_edca = false;
		}
	}
}

#endif /*PHYDM_SUPPORT_EDCA*/
