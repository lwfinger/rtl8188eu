/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#ifndef __RECV_OSDEP_H_
#define __RECV_OSDEP_H_


extern sint _rtw_init_recv_priv(struct recv_priv *precvpriv, _adapter *padapter);
extern void _rtw_free_recv_priv(struct recv_priv *precvpriv);


extern s32  rtw_recv_entry(union recv_frame *precv_frame);
extern int rtw_recv_indicatepkt(_adapter *adapter, union recv_frame *precv_frame);
extern void rtw_recv_returnpacket(_nic_hdl cnxt, _pkt *preturnedpkt);

extern int rtw_recv_monitor(_adapter *padapter, union recv_frame *precv_frame);

extern void rtw_hostapd_mlme_rx(_adapter *padapter, union recv_frame *precv_frame);

struct sta_info;
extern void rtw_handle_tkip_mic_err(_adapter *padapter, struct sta_info *sta, u8 bgroup);


int rtw_os_recv_resource_init(struct recv_priv *precvpriv, _adapter *padapter);
int rtw_os_recv_resource_alloc(_adapter *padapter, union recv_frame *precvframe);
void rtw_os_recv_resource_free(struct recv_priv *precvpriv);


int rtw_os_alloc_recvframe(_adapter *padapter, union recv_frame *precvframe, u8 *pdata, _pkt *pskb);
int rtw_os_recvframe_duplicate_skb(_adapter *padapter, union recv_frame *pcloneframe, _pkt *pskb);
void rtw_os_free_recvframe(union recv_frame *precvframe);


int rtw_os_recvbuf_resource_alloc(_adapter *padapter, struct recv_buf *precvbuf);
int rtw_os_recvbuf_resource_free(_adapter *padapter, struct recv_buf *precvbuf);

_pkt *rtw_os_alloc_msdu_pkt(union recv_frame *prframe, u16 nSubframe_Length, u8 *pdata);
void rtw_os_recv_indicate_pkt(_adapter *padapter, _pkt *pkt, struct rx_pkt_attrib *pattrib);

void rtw_os_read_port(_adapter *padapter, struct recv_buf *precvbuf);

void rtw_init_recv_timer(struct recv_reorder_ctrl *preorder_ctrl);

#ifdef CONFIG_RTW_NAPI
#include <linux/netdevice.h>	/* struct napi_struct */

int rtw_recv_napi_poll(struct napi_struct *, int budget);
#endif /* CONFIG_RTW_NAPI */

#endif /*  */
