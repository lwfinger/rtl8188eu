/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#ifndef __HAL_SDIO_H_
#define __HAL_SDIO_H_

#define ffaddr2deviceId(pdvobj, addr)	(pdvobj->Queue2Pipe[addr])

u8 rtw_hal_sdio_max_txoqt_free_space(_adapter *padapter);
u8 rtw_hal_sdio_query_tx_freepage(_adapter *padapter, u8 PageIdx, u8 RequiredPageNum);
void rtw_hal_sdio_update_tx_freepage(_adapter *padapter, u8 PageIdx, u8 RequiredPageNum);
void rtw_hal_set_sdio_tx_max_length(PADAPTER padapter, u8 numHQ, u8 numNQ, u8 numLQ, u8 numPubQ);
u32 rtw_hal_get_sdio_tx_max_length(PADAPTER padapter, u8 queue_idx);
bool sdio_power_on_check(PADAPTER padapter);

#ifdef CONFIG_FW_C2H_REG
void sd_c2h_hisr_hdl(_adapter *adapter);
#endif

#endif /* __HAL_SDIO_H_ */
