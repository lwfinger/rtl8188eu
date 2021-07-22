// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#define _RTL8188E_SRESET_C_

/* #include <rtl8188e_sreset.h> */
#include <rtl8188e_hal.h>

#ifdef DBG_CONFIG_ERROR_DETECT

void rtl8188e_sreset_xmit_status_check(_adapter *padapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;

	unsigned long current_time;
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	unsigned int diff_time;
	u32 txdma_status;

	txdma_status = rtw_read32(padapter, REG_TXDMA_STATUS);
	if (txdma_status != 0x00 && txdma_status != 0xeaeaeaea) {
		RTW_INFO("%s REG_TXDMA_STATUS:0x%08x\n", __func__, txdma_status);
		rtw_hal_sreset_reset(padapter);
	}
	current_time = jiffies;

	if (0 == pxmitpriv->free_xmitbuf_cnt || 0 == pxmitpriv->free_xmit_extbuf_cnt) {

		diff_time = rtw_get_passing_time_ms(psrtpriv->last_tx_time);

		if (diff_time > 2000) {
			if (psrtpriv->last_tx_complete_time == 0)
				psrtpriv->last_tx_complete_time = current_time;
			else {
				diff_time = rtw_get_passing_time_ms(psrtpriv->last_tx_complete_time);
				if (diff_time > 4000) {
					u32 ability = 0;

					/* padapter->Wifi_Error_Status = WIFI_TX_HANG; */
					ability = rtw_phydm_ability_get(padapter);
					RTW_INFO("%s tx hang %s\n", __func__,
						(ability & ODM_BB_ADAPTIVITY) ? "ODM_BB_ADAPTIVITY" : "");

					if (!(ability & ODM_BB_ADAPTIVITY))
						rtw_hal_sreset_reset(padapter);
				}
			}
		}
	}

	if (psrtpriv->dbg_trigger_point == SRESET_TGP_XMIT_STATUS) {
		psrtpriv->dbg_trigger_point = SRESET_TGP_NULL;
		rtw_hal_sreset_reset(padapter);
		return;
	}
}

void rtl8188e_sreset_linked_status_check(_adapter *padapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct sreset_priv *psrtpriv = &pHalData->srestpriv;

	u32 rx_dma_status = 0;
	u8 fw_status = 0;
	rx_dma_status = rtw_read32(padapter, REG_RXDMA_STATUS);
	if (rx_dma_status != 0x00) {
		RTW_INFO("%s REG_RXDMA_STATUS:0x%08x\n", __func__, rx_dma_status);
		rtw_write32(padapter, REG_RXDMA_STATUS, rx_dma_status);
	}
	fw_status = rtw_read8(padapter, REG_FMETHR);
	if (fw_status != 0x00) {
		if (fw_status == 1)
			RTW_INFO("%s REG_FW_STATUS (0x%02x), Read_Efuse_Fail !!\n", __func__, fw_status);
		else if (fw_status == 2)
			RTW_INFO("%s REG_FW_STATUS (0x%02x), Condition_No_Match !!\n", __func__, fw_status);
	}
	if (psrtpriv->dbg_trigger_point == SRESET_TGP_LINK_STATUS) {
		psrtpriv->dbg_trigger_point = SRESET_TGP_NULL;
		rtw_hal_sreset_reset(padapter);
		return;
	}
}
#endif
