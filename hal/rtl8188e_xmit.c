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
#define _RTL8188E_XMIT_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <rtl8188e_hal.h>

void dump_txrpt_ccx_88e(void *buf)
{
	struct txrpt_ccx_88e *txrpt_ccx = (struct txrpt_ccx_88e *)buf;

	DBG_88E("%s:\n"
		"tag1:%u, pkt_num:%u, txdma_underflow:%u, int_bt:%u, int_tri:%u, int_ccx:%u\n"
		"mac_id:%u, pkt_ok:%u, bmc:%u\n"
		"retry_cnt:%u, lifetime_over:%u, retry_over:%u\n"
		"ccx_qtime:%u\n"
		"final_data_rate:0x%02x\n"
		"qsel:%u, sw:0x%03x\n"
		, __func__
		, txrpt_ccx->tag1, txrpt_ccx->pkt_num, txrpt_ccx->txdma_underflow, txrpt_ccx->int_bt, txrpt_ccx->int_tri, txrpt_ccx->int_ccx
		, txrpt_ccx->mac_id, txrpt_ccx->pkt_ok, txrpt_ccx->bmc
		, txrpt_ccx->retry_cnt, txrpt_ccx->lifetime_over, txrpt_ccx->retry_over
		, txrpt_ccx_qtime_88e(txrpt_ccx)
		, txrpt_ccx->final_data_rate
		, txrpt_ccx->qsel, txrpt_ccx_sw_88e(txrpt_ccx)
	);
}

void handle_txrpt_ccx_88e(struct adapter *adapter, u8 *buf)
{
	struct txrpt_ccx_88e *txrpt_ccx = (struct txrpt_ccx_88e *)buf;

	#ifdef DBG_CCX
	dump_txrpt_ccx_88e(buf);
	#endif

	if (txrpt_ccx->int_ccx) {
		if (txrpt_ccx->pkt_ok)
			rtw_ack_tx_done(&adapter->xmitpriv, RTW_SCTX_DONE_SUCCESS);
		else
			rtw_ack_tx_done(&adapter->xmitpriv, RTW_SCTX_DONE_CCX_PKT_FAIL);
	}
}

void _dbg_dump_tx_info(struct adapter	*padapter,int frame_tag,struct tx_desc *ptxdesc)
{
	u8 bDumpTxPkt;
	u8 bDumpTxDesc = false;
	rtw_hal_get_def_var(padapter, HAL_DEF_DBG_DUMP_TXPKT, &(bDumpTxPkt));

	if (bDumpTxPkt ==1) {/* dump txdesc for data frame */
		DBG_88E("dump tx_desc for data frame\n");
		if ((frame_tag&0x0f) == DATA_FRAMETAG) {
			bDumpTxDesc = true;
		}
	}
	else if (bDumpTxPkt ==2) {/* dump txdesc for mgnt frame */
		DBG_88E("dump tx_desc for mgnt frame\n");
		if ((frame_tag&0x0f) == MGNT_FRAMETAG) {
			bDumpTxDesc = true;
		}
	}
	else if (bDumpTxPkt ==3) {/* dump early info */
	}

	if (bDumpTxDesc) {
		DBG_8192C("=====================================\n");
		DBG_8192C("txdw0(0x%08x)\n",ptxdesc->txdw0);
		DBG_8192C("txdw1(0x%08x)\n",ptxdesc->txdw1);
		DBG_8192C("txdw2(0x%08x)\n",ptxdesc->txdw2);
		DBG_8192C("txdw3(0x%08x)\n",ptxdesc->txdw3);
		DBG_8192C("txdw4(0x%08x)\n",ptxdesc->txdw4);
		DBG_8192C("txdw5(0x%08x)\n",ptxdesc->txdw5);
		DBG_8192C("txdw6(0x%08x)\n",ptxdesc->txdw6);
		DBG_8192C("txdw7(0x%08x)\n",ptxdesc->txdw7);
		DBG_8192C("=====================================\n");
	}
}
