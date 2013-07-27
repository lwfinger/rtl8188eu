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
#define _RTL8188E_SRESET_C_

#include <rtl8188e_sreset.h>
#include <rtl8188e_hal.h>

extern void rtw_cancel_all_timer(struct adapter *padapter);
static void _restore_security_setting(struct adapter *padapter)
{
	u8 EntryId = 0;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct sta_priv * pstapriv = &padapter->stapriv;
	struct sta_info *psta;
	struct security_priv* psecuritypriv=&(padapter->securitypriv);
	struct mlme_ext_info	*pmlmeinfo = &padapter->mlmeextpriv.mlmext_info;

	(pmlmeinfo->auth_algo == dot11AuthAlgrthm_8021X)
		? rtw_write8(padapter, REG_SECCFG, 0xcc)
		: rtw_write8(padapter, REG_SECCFG, 0xcf);

	if (	( padapter->securitypriv.dot11PrivacyAlgrthm == _WEP40_ ) ||
		( padapter->securitypriv.dot11PrivacyAlgrthm == _WEP104_ ))
	{

		for (EntryId=0; EntryId<4; EntryId++)
		{
			if (EntryId == psecuritypriv->dot11PrivacyKeyIndex)
				rtw_set_key(padapter,&padapter->securitypriv, EntryId, 1);
			else
				rtw_set_key(padapter,&padapter->securitypriv, EntryId, 0);
		}

	}
	else if ((padapter->securitypriv.dot11PrivacyAlgrthm == _TKIP_) ||
		(padapter->securitypriv.dot11PrivacyAlgrthm == _AES_))
	{
		psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));
		if (psta) {
			/* pairwise key */
			rtw_setstakey_cmd(padapter, (unsigned char *)psta, true);
			/* group key */
			rtw_set_key(padapter,&padapter->securitypriv,padapter->securitypriv.dot118021XGrpKeyid, 0);
		}
	}

}

static void _restore_network_status(struct adapter *padapter)
{
	struct hal_data_8188e	*pHalData = GET_HAL_DATA(padapter);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct wlan_bssid_ex	*pnetwork = (struct wlan_bssid_ex*)(&(pmlmeinfo->network));
	unsigned short	caps;
	u8	join_type;

	/*  */
	/*  reset related register of Beacon control */

	/* set MSR to nolink */
	Set_MSR(padapter, _HW_STATE_NOLINK_);
	/*  reject all data frame */
	rtw_write16(padapter, REG_RXFLTMAP2,0x00);
	/* reset TSF */
	rtw_write8(padapter, REG_DUAL_TSF_RST, (BIT(0)|BIT(1)));

	/*  disable update TSF */
	SetBcnCtrlReg(padapter, BIT(4), 0);

	/*  */
	rtw_joinbss_reset(padapter);
	set_channel_bwmode(padapter, pmlmeext->cur_channel, pmlmeext->cur_ch_offset, pmlmeext->cur_bwmode);

	if (padapter->registrypriv.wifi_spec) {
		/*  for WiFi test, follow WMM test plan spec */
		rtw_write32(padapter, REG_EDCA_VO_PARAM, 0x002F431C);
		rtw_write32(padapter, REG_EDCA_VI_PARAM, 0x005E541C);
		rtw_write32(padapter, REG_EDCA_BE_PARAM, 0x0000A525);
		rtw_write32(padapter, REG_EDCA_BK_PARAM, 0x0000A549);
		/*  for WiFi test, mixed mode with intel STA under bg mode throughput issue */
		if (padapter->mlmepriv.htpriv.ht_option == 0)
		rtw_write32(padapter, REG_EDCA_BE_PARAM, 0x00004320);

	} else {
		rtw_write32(padapter, REG_EDCA_VO_PARAM, 0x002F3217);
		rtw_write32(padapter, REG_EDCA_VI_PARAM, 0x005E4317);
		rtw_write32(padapter, REG_EDCA_BE_PARAM, 0x00105320);
		rtw_write32(padapter, REG_EDCA_BK_PARAM, 0x0000A444);
	}

	rtw_hal_set_hwreg(padapter, HW_VAR_BSSID, pmlmeinfo->network.MacAddress);
	join_type = 0;
	rtw_hal_set_hwreg(padapter, HW_VAR_MLME_JOIN, (u8 *)(&join_type));

	Set_MSR(padapter, (pmlmeinfo->state & 0x3));

	mlmeext_joinbss_event_callback(padapter, 1);
	/* restore Sequence No. */
	rtw_write8(padapter,0x4dc,padapter->xmitpriv.nqos_ssn);
}

void rtl8188e_silentreset_for_specific_platform(struct adapter *padapter)
{
}

void rtl8188e_sreset_xmit_status_check(struct adapter *padapter)
{
	struct hal_data_8188e	*pHalData = GET_HAL_DATA(padapter);
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;

	unsigned long current_time;
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	unsigned int diff_time;
	u32 txdma_status;

	if ( (txdma_status=rtw_read32(padapter, REG_TXDMA_STATUS)) !=0x00){
		DBG_88E("%s REG_TXDMA_STATUS:0x%08x\n", __func__, txdma_status);
		rtw_write32(padapter,REG_TXDMA_STATUS,txdma_status);
		rtl8188e_silentreset_for_specific_platform(padapter);
	}
	/* total xmit irp = 4 */
	current_time = rtw_get_current_time();
	if (0==pxmitpriv->free_xmitbuf_cnt)
	{
		diff_time = jiffies_to_msecs(current_time - psrtpriv->last_tx_time);

		if (diff_time > 2000){
			if (psrtpriv->last_tx_complete_time==0){
				psrtpriv->last_tx_complete_time = current_time;
			}
			else{
				diff_time = jiffies_to_msecs(current_time - psrtpriv->last_tx_complete_time);
				if (diff_time > 4000){
					DBG_88E("%s tx hang\n", __func__);
					rtl8188e_silentreset_for_specific_platform(padapter);
				}
			}
		}
	}
}

void rtl8188e_sreset_linked_status_check(struct adapter *padapter)
{
	u32 rx_dma_status = 0;
	u8 fw_status=0;
	rx_dma_status = rtw_read32(padapter,REG_RXDMA_STATUS);
	if (rx_dma_status!= 0x00){
		DBG_88E("%s REG_RXDMA_STATUS:0x%08x\n",__func__,rx_dma_status);
		rtw_write32(padapter,REG_RXDMA_STATUS,rx_dma_status);
	}
	fw_status = rtw_read8(padapter,REG_FMETHR);
	if (fw_status != 0x00)
	{
		if (fw_status == 1)
			DBG_88E("%s REG_FW_STATUS (0x%02x), Read_Efuse_Fail !! \n",__func__,fw_status);
		else if (fw_status == 2)
			DBG_88E("%s REG_FW_STATUS (0x%02x), Condition_No_Match !! \n",__func__,fw_status);
	}
}
