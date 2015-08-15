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
#define _HCI_OPS_OS_C_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <osdep_intf.h>
#include <usb_ops.h>
#include <circ_buf.h>
#include <recv_osdep.h>
#include <rtl8188e_hal.h>

static int usbctrl_vendorreq(struct intf_hdl *pintfhdl, u8 request, u16 value, u16 index, void *pdata, u16 len, u8 requesttype)
{
	struct adapter	*padapter = pintfhdl->padapter;
	struct dvobj_priv  *pdvobjpriv = adapter_to_dvobj(padapter);
	struct usb_device *udev=pdvobjpriv->pusbdev;

	unsigned int pipe;
	int status = 0;
	u8 reqtype;
	u8 *pIo_buf;
	int vendorreq_times = 0;

	if ((padapter->bSurpriseRemoved) ||(dvobj_to_pwrctl(pdvobjpriv)->pnp_bstop_trx)) {
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usbctrl_vendorreq:(padapter->bSurpriseRemoved ||pwrctl->pnp_bstop_trx)!!!\n"));
		status = -EPERM;
		goto exit;
	}

	if (len>MAX_VENDOR_REQ_CMD_SIZE) {
		DBG_8192C( "[%s] Buffer len error ,vendor request failed\n", __FUNCTION__ );
		status = -EINVAL;
		goto exit;
	}

	_enter_critical_mutex(&pdvobjpriv->usb_vendor_req_mutex, NULL);

	/*  Acquire IO memory for vendorreq */
	pIo_buf = pdvobjpriv->usb_vendor_req_buf;

	if ( pIo_buf== NULL) {
		DBG_8192C( "[%s] pIo_buf == NULL\n", __FUNCTION__ );
		status = -ENOMEM;
		goto release_mutex;
	}

	while (++vendorreq_times<= MAX_USBCTRL_VENDORREQ_TIMES)
	{
		memset(pIo_buf, 0, len);

		if (requesttype == 0x01)
		{
			pipe = usb_rcvctrlpipe(udev, 0);/* read_in */
			reqtype =  REALTEK_USB_VENQT_READ;
		}
		else
		{
			pipe = usb_sndctrlpipe(udev, 0);/* write_out */
			reqtype =  REALTEK_USB_VENQT_WRITE;
			memcpy( pIo_buf, pdata, len);
		}

		status = rtw_usb_control_msg(udev, pipe, request, reqtype, value, index, pIo_buf, len, RTW_USB_CONTROL_MSG_TIMEOUT);

		if ( status == len)   /*  Success this control transfer. */
		{
			rtw_reset_continual_io_error(pdvobjpriv);
			if ( requesttype == 0x01 )
			{   /*  For Control read transfer, we have to copy the read data from pIo_buf to pdata. */
				memcpy( pdata, pIo_buf,  len );
			}
		}
		else { /*  error cases */
			DBG_8192C("reg 0x%x, usb %s %u fail, status:%d value=0x%x, vendorreq_times:%d\n"
				, value,(requesttype == 0x01)?"read":"write" , len, status, *(u32*)pdata, vendorreq_times);

			if (status < 0) {
				if (status == (-ESHUTDOWN)	|| status == -ENODEV	)
				{
					padapter->bSurpriseRemoved = true;
				} else {
					HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
					pHalData->srestpriv.Wifi_Error_Status = USB_VEN_REQ_CMD_FAIL;
				}
			}
			else /*  status != len && status >= 0 */
			{
				if (status > 0) {
					if ( requesttype == 0x01 )
					{   /*  For Control read transfer, we have to copy the read data from pIo_buf to pdata. */
						memcpy( pdata, pIo_buf,  len );
					}
				}
			}

			if (rtw_inc_and_chk_continual_io_error(pdvobjpriv) == true ) {
				padapter->bSurpriseRemoved = true;
				break;
			}

		}

		/*  firmware download is checksumed, don't retry */
		if ( (value >= FW_8188E_START_ADDRESS && value <= FW_8188E_END_ADDRESS) || status == len )
			break;

	}

	/*  release IO memory used by vendorreq */

release_mutex:
	_exit_critical_mutex(&pdvobjpriv->usb_vendor_req_mutex, NULL);
exit:
	return status;

}

static u8 usb_read8(struct intf_hdl *pintfhdl, u32 addr)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u8 data;

	request = 0x05;
	requesttype = 0x01;/* read_in */
	index = 0;/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 1;

	usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);
	return data;
}

static u16 usb_read16(struct intf_hdl *pintfhdl, u32 addr)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	__le32 data;

	request = 0x05;
	requesttype = 0x01;/* read_in */
	index = 0;/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 2;

	usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);
	return le32_to_cpu(data) & 0xffff;
}

static u32 usb_read32(struct intf_hdl *pintfhdl, u32 addr)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	__le32 data;

	request = 0x05;
	requesttype = 0x01;/* read_in */
	index = 0;/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 4;

	usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	return le32_to_cpu(data);
}

static int usb_write8(struct intf_hdl *pintfhdl, u32 addr, u8 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u8 data;
	int ret;

	request = 0x05;
	requesttype = 0x00;/* write_out */
	index = 0;/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 1;

	data = val;

	ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	return ret;
}

static int usb_write16(struct intf_hdl *pintfhdl, u32 addr, u16 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	__le32 data;
	int ret;

	request = 0x05;
	requesttype = 0x00;/* write_out */
	index = 0;/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 2;

	data = cpu_to_le32(val & 0x0000ffff);

	ret = usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	return ret;
}

static int usb_write32(struct intf_hdl *pintfhdl, u32 addr, u32 val)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	__le32 data;
	int ret;

	request = 0x05;
	requesttype = 0x00;/* write_out */
	index = 0;/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = 4;
	data = cpu_to_le32(val);

	ret =usbctrl_vendorreq(pintfhdl, request, wvalue, index, &data, len, requesttype);

	return ret;
}

static int usb_writeN(struct intf_hdl *pintfhdl, u32 addr, u32 length, u8 *pdata)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;
	u16 len;
	u8 buf[VENDOR_CMD_MAX_DATA_LEN]={0};

	request = 0x05;
	requesttype = 0x00;/* write_out */
	index = 0;/* n/a */

	wvalue = (u16)(addr&0x0000ffff);
	len = length;
	memcpy(buf, pdata, len );

	return usbctrl_vendorreq(pintfhdl, request, wvalue, index, buf, len, requesttype);
}

static void interrupt_handler_8188eu(struct adapter *padapter,u16 pkt_len,u8 *pbuf)
{
	HAL_DATA_TYPE	*pHalData=GET_HAL_DATA(padapter);
	struct reportpwrstate_parm pwr_rpt;

	if ( pkt_len != INTERRUPT_MSG_FORMAT_LEN ) {
		DBG_8192C("%s Invalid interrupt content length (%d)!\n", __FUNCTION__, pkt_len);
		return ;
	}

	/*  HISR */
	memcpy(&(pHalData->IntArray[0]), &(pbuf[USB_INTR_CONTENT_HISR_OFFSET]), 4);
	memcpy(&(pHalData->IntArray[1]), &(pbuf[USB_INTR_CONTENT_HISRE_OFFSET]), 4);

	if (  pHalData->IntArray[1]  & IMR_TXERR_88E )
		DBG_88E("===> %s Tx Error Flag Interrupt Status\n",__FUNCTION__);
	if (  pHalData->IntArray[1]  & IMR_RXERR_88E )
		DBG_88E("===> %s Rx Error Flag INT Status\n",__FUNCTION__);
	if (  pHalData->IntArray[1]  & IMR_TXFOVW_88E )
		DBG_88E("===> %s Transmit FIFO Overflow\n",__FUNCTION__);
	if (  pHalData->IntArray[1]  & IMR_RXFOVW_88E )
		DBG_88E("===> %s Receive FIFO Overflow\n",__FUNCTION__);

	/*  C2H Event */
	if (pbuf[0]!= 0) {
		memcpy(&(pHalData->C2hArray[0]), &(pbuf[USB_INTR_CONTENT_C2H_OFFSET]), 16);
		/* rtw_c2h_wk_cmd(padapter); to do.. */
	}

}

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
static void usb_read_interrupt_complete(struct urb *purb, struct pt_regs *regs)
{
	int	err;
	struct adapter		*padapter = (struct adapter	 *)purb->context;

	if (padapter->bSurpriseRemoved || padapter->bDriverStopped||padapter->bReadPortCancel)
	{
		DBG_8192C("%s() RX Warning! bDriverStopped(%d) OR bSurpriseRemoved(%d) bReadPortCancel(%d)\n",
		__FUNCTION__,padapter->bDriverStopped, padapter->bSurpriseRemoved,padapter->bReadPortCancel);

		return;
	}

	if (purb->status== 0)/* SUCCESS */
	{
		if (purb->actual_length > INTERRUPT_MSG_FORMAT_LEN)
		{
			DBG_8192C("usb_read_interrupt_complete: purb->actual_length > INTERRUPT_MSG_FORMAT_LEN(%d)\n",INTERRUPT_MSG_FORMAT_LEN);
		}

		interrupt_handler_8188eu(padapter, purb->actual_length,purb->transfer_buffer );

		err = usb_submit_urb(purb, GFP_ATOMIC);
		if ((err) && (err != (-EPERM)))
		{
			DBG_8192C("cannot submit interrupt in-token(err = 0x%08x),urb_status = %d\n",err, purb->status);
		}
	}
	else
	{
		DBG_8192C("###=> usb_read_interrupt_complete => urb status(%d)\n", purb->status);

		switch (purb->status) {
			case -EINVAL:
			case -EPIPE:
			case -ENODEV:
			case -ESHUTDOWN:
				/* padapter->bSurpriseRemoved=true; */
				RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete:bSurpriseRemoved=true\n"));
			case -ENOENT:
				padapter->bDriverStopped=true;
				RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete:bDriverStopped=true\n"));
				break;
			case -EPROTO:
				break;
			case -EINPROGRESS:
				DBG_8192C("ERROR: URB IS IN PROGRESS!/n");
				break;
			default:
				break;
		}
	}

}

static u32 usb_read_interrupt(struct intf_hdl *pintfhdl, u32 addr)
{
	int	err;
	unsigned int pipe;
	u32	ret = _SUCCESS;
	struct adapter			*adapter = pintfhdl->padapter;
	struct dvobj_priv	*pdvobj = adapter_to_dvobj(adapter);
	struct recv_priv	*precvpriv = &adapter->recvpriv;
	struct usb_device	*pusbd = pdvobj->pusbdev;

;

	/* translate DMA FIFO addr to pipehandle */
	pipe = ffaddr2pipehdl(pdvobj, addr);

	usb_fill_int_urb(precvpriv->int_in_urb, pusbd, pipe,
					precvpriv->int_in_buf,
					INTERRUPT_MSG_FORMAT_LEN,
					usb_read_interrupt_complete,
					adapter,
					1);

	err = usb_submit_urb(precvpriv->int_in_urb, GFP_ATOMIC);
	if ((err) && (err != (-EPERM)))
	{
		DBG_8192C("cannot submit interrupt in-token(err = 0x%08x),urb_status = %d\n",err, precvpriv->int_in_urb->status);
		ret = _FAIL;
	}

;

	return ret;
}
#endif

static s32 pre_recv_entry(union recv_frame *precvframe, struct recv_stat *prxstat, struct phy_stat *pphy_status)
{
	s32 ret=_SUCCESS;
	return ret;
}

static int recvbuf2recvframe(struct adapter *padapter, struct sk_buff *pskb)
{
	u8	*pbuf;
	u8	shift_sz = 0;
	u16	pkt_cnt;
	u32	pkt_offset, skb_len, alloc_sz;
	s32	transfer_len;
	struct recv_stat	*prxstat;
	struct phy_stat	*pphy_status = NULL;
	struct sk_buff *pkt_copy = NULL;
	union recv_frame	*precvframe = NULL;
	struct rx_pkt_attrib	*pattrib = NULL;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct recv_priv	*precvpriv = &padapter->recvpriv;
	struct  __queue *pfree_recv_queue = &precvpriv->free_recv_queue;

	transfer_len = (s32)pskb->len;
	pbuf = pskb->data;

	prxstat = (struct recv_stat *)pbuf;
	pkt_cnt = (le32_to_cpu(prxstat->rxdw2)>>16) & 0xff;

	do{
		RT_TRACE(_module_rtl871x_recv_c_, _drv_info_,
			 ("recvbuf2recvframe: rxdesc=offsset 0:0x%08x, 4:0x%08x, 8:0x%08x, C:0x%08x\n",
			  prxstat->rxdw0, prxstat->rxdw1, prxstat->rxdw2, prxstat->rxdw4));

		prxstat = (struct recv_stat *)pbuf;

		precvframe = rtw_alloc_recvframe(pfree_recv_queue);
		if (precvframe== NULL)
		{
			RT_TRACE(_module_rtl871x_recv_c_,_drv_err_,("recvbuf2recvframe: precvframe== NULL\n"));
			DBG_8192C("%s()-%d: rtw_alloc_recvframe() failed! RX Drop!\n", __FUNCTION__, __LINE__);
			goto _exit_recvbuf2recvframe;
		}

		_rtw_init_listhead(&precvframe->u.hdr.list);
		precvframe->u.hdr.precvbuf = NULL;	/* can't access the precvbuf for new arch. */
		precvframe->u.hdr.len=0;

		/* rtl8192c_query_rx_desc_status(precvframe, prxstat); */
		update_recvframe_attrib_88e(precvframe, prxstat);

		pattrib = &precvframe->u.hdr.attrib;

		if ((padapter->registrypriv.mp_mode == 0) &&((pattrib->crc_err) || (pattrib->icv_err)))
		{
			DBG_8192C("%s: RX Warning! crc_err=%d icv_err=%d, skip!\n", __FUNCTION__, pattrib->crc_err, pattrib->icv_err);

			rtw_free_recvframe(precvframe, pfree_recv_queue);
			goto _exit_recvbuf2recvframe;
		}

		if ( (pattrib->physt) && (pattrib->pkt_rpt_type == NORMAL_RX))
		{
			pphy_status = (struct phy_stat *)(pbuf + RXDESC_OFFSET);
		}

		pkt_offset = RXDESC_SIZE + pattrib->drvinfo_sz + pattrib->shift_sz + pattrib->pkt_len;

		if ((pattrib->pkt_len<=0) || (pkt_offset>transfer_len))
		{
			RT_TRACE(_module_rtl871x_recv_c_,_drv_info_,("recvbuf2recvframe: pkt_len<=0\n"));
			DBG_8192C("%s()-%d: RX Warning!,pkt_len<=0 or pkt_offset> transfoer_len\n", __FUNCTION__, __LINE__);
			rtw_free_recvframe(precvframe, pfree_recv_queue);
			goto _exit_recvbuf2recvframe;
		}

		/* 	Modified by Albert 20101213 */
		/* 	For 8 bytes IP header alignment. */
		if (pattrib->qos)	/* 	Qos data, wireless lan header length is 26 */
		{
			shift_sz = 6;
		}
		else
		{
			shift_sz = 0;
		}

		skb_len = pattrib->pkt_len;

		/*  for first fragment packet, driver need allocate 1536+drvinfo_sz+RXDESC_SIZE to defrag packet. */
		/*  modify alloc_sz for recvive crc error packet by thomas 2011-06-02 */
		if ((pattrib->mfrag == 1)&&(pattrib->frag_num == 0)) {
			if (skb_len <= 1650)
				alloc_sz = 1664;
			else
				alloc_sz = skb_len + 14;
		}
		else {
			alloc_sz = skb_len;
			/* 	6 is for IP header 8 bytes alignment in QoS packet case. */
			/* 	8 is for skb->data 4 bytes alignment. */
			alloc_sz += 14;
		}

		pkt_copy = rtw_skb_alloc(alloc_sz);

		if (pkt_copy)
		{
			pkt_copy->dev = padapter->pnetdev;
			precvframe->u.hdr.pkt = pkt_copy;
			precvframe->u.hdr.rx_head = pkt_copy->data;
			precvframe->u.hdr.rx_end = pkt_copy->data + alloc_sz;
			skb_reserve( pkt_copy, 8 - ((SIZE_PTR)( pkt_copy->data ) & 7 ));/* force pkt_copy->data at 8-byte alignment address */
			skb_reserve( pkt_copy, shift_sz );/* force ip_hdr at 8-byte alignment address according to shift_sz. */
			memcpy(pkt_copy->data, (pbuf + pattrib->drvinfo_sz + RXDESC_SIZE), skb_len);
			precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail = pkt_copy->data;
		}
		else
		{
			if ((pattrib->mfrag == 1)&&(pattrib->frag_num == 0))
			{
				DBG_8192C("recvbuf2recvframe: alloc_skb fail , drop frag frame\n");
				rtw_free_recvframe(precvframe, pfree_recv_queue);
				goto _exit_recvbuf2recvframe;
			}

			precvframe->u.hdr.pkt = rtw_skb_clone(pskb);
			if (precvframe->u.hdr.pkt)
			{
				precvframe->u.hdr.rx_head = precvframe->u.hdr.rx_data = precvframe->u.hdr.rx_tail
					= pbuf+ pattrib->drvinfo_sz + RXDESC_SIZE;
				precvframe->u.hdr.rx_end =  pbuf +pattrib->drvinfo_sz + RXDESC_SIZE+ alloc_sz;
			}
			else
			{
				DBG_8192C("recvbuf2recvframe: rtw_skb_clone fail\n");
				rtw_free_recvframe(precvframe, pfree_recv_queue);
				goto _exit_recvbuf2recvframe;
			}

		}

		recvframe_put(precvframe, skb_len);
		/* recvframe_pull(precvframe, drvinfo_sz + RXDESC_SIZE); */

		switch (pHalData->UsbRxAggMode) {
		case USB_RX_AGG_DMA:
		case USB_RX_AGG_MIX:
			pkt_offset = (u16)_RND128(pkt_offset);
			break;
			case USB_RX_AGG_USB:
			pkt_offset = (u16)_RND4(pkt_offset);
			break;
		case USB_RX_AGG_DISABLE:
		default:
			break;
		}

		if (pattrib->pkt_rpt_type == NORMAL_RX)/* Normal rx packet */
		{
			if (pattrib->physt)
				update_recvframe_phyinfo_88e(precvframe, (struct phy_stat*)pphy_status);
			if (rtw_recv_entry(precvframe) != _SUCCESS)
			{
				RT_TRACE(_module_rtl871x_recv_c_,_drv_err_,
					("recvbuf2recvframe: rtw_recv_entry(precvframe) != _SUCCESS\n"));
			}
		} else { /*  pkt_rpt_type == TX_REPORT1-CCX, TX_REPORT2-TX RTP,HIS_REPORT-USB HISR RTP */
			/* enqueue recvframe to txrtp queue */
			if (pattrib->pkt_rpt_type == TX_REPORT1) {
				/* DBG_8192C("rx CCX\n"); */
				/* CCX-TXRPT ack for xmit mgmt frames. */
				handle_txrpt_ccx_88e(padapter, precvframe->u.hdr.rx_data);
			}
			else if (pattrib->pkt_rpt_type == TX_REPORT2) {
				/* DBG_8192C("rx TX RPT\n"); */
				ODM_RA_TxRPT2Handle_8188E(
							&pHalData->odmpriv,
							precvframe->u.hdr.rx_data,
							pattrib->pkt_len,
							pattrib->MacIDValidEntry[0],
							pattrib->MacIDValidEntry[1]
							);
			} else if (pattrib->pkt_rpt_type == HIS_REPORT) {
				interrupt_handler_8188eu(padapter,pattrib->pkt_len,precvframe->u.hdr.rx_data);
			}
			rtw_free_recvframe(precvframe, pfree_recv_queue);
		}

		pkt_cnt--;
		transfer_len -= pkt_offset;
		pbuf += pkt_offset;
		precvframe = NULL;
		pkt_copy = NULL;

		if (transfer_len>0 && pkt_cnt== 0)
			pkt_cnt = (le32_to_cpu(prxstat->rxdw2)>>16) & 0xff;

	}while ((transfer_len>0) && (pkt_cnt>0));

_exit_recvbuf2recvframe:

	return _SUCCESS;
}

void rtl8188eu_recv_tasklet(void *priv)
{
	struct sk_buff *pskb;
	struct adapter		*padapter = (struct adapter*)priv;
	struct recv_priv	*precvpriv = &padapter->recvpriv;

	while (NULL != (pskb = skb_dequeue(&precvpriv->rx_skb_queue)))
	{
		if ((padapter->bDriverStopped == true)||(padapter->bSurpriseRemoved== true))
		{
			DBG_8192C("recv_tasklet => bDriverStopped or bSurpriseRemoved\n");
			rtw_skb_free(pskb);
			break;
		}

		recvbuf2recvframe(padapter, pskb);

		skb_reset_tail_pointer(pskb);

		pskb->len = 0;

		skb_queue_tail(&precvpriv->free_recv_skb_queue, pskb);
	}

}


static void usb_read_port_complete(struct urb *purb, struct pt_regs *regs)
{
	unsigned long irqL;
	uint isevt, *pbuf;
	struct recv_buf	*precvbuf = (struct recv_buf *)purb->context;
	struct adapter			*padapter =(struct adapter *)precvbuf->adapter;
	struct recv_priv	*precvpriv = &padapter->recvpriv;

	RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete!!!\n"));

	/* _enter_critical(&precvpriv->lock, &irqL); */
	/* precvbuf->irp_pending=false; */
	/* precvpriv->rx_pending_cnt --; */
	/* _exit_critical(&precvpriv->lock, &irqL); */

	precvpriv->rx_pending_cnt --;

	/* if (precvpriv->rx_pending_cnt== 0) */
	/*  */
	/* 	RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete: rx_pending_cnt== 0, set allrxreturnevt!\n")); */
	/* 	_rtw_up_sema(&precvpriv->allrxreturnevt); */
	/*  */

	if (padapter->bSurpriseRemoved || padapter->bDriverStopped||padapter->bReadPortCancel)
	{
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete:bDriverStopped(%d) OR bSurpriseRemoved(%d)\n", padapter->bDriverStopped, padapter->bSurpriseRemoved));

		precvbuf->reuse = true;
		DBG_8192C("%s() RX Warning! bDriverStopped(%d) OR bSurpriseRemoved(%d) bReadPortCancel(%d)\n",
		__FUNCTION__,padapter->bDriverStopped, padapter->bSurpriseRemoved,padapter->bReadPortCancel);
		goto exit;
	}

	if (purb->status== 0)/* SUCCESS */
	{
		if ((purb->actual_length > MAX_RECVBUF_SZ) || (purb->actual_length < RXDESC_SIZE))
		{
			RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete: (purb->actual_length > MAX_RECVBUF_SZ) || (purb->actual_length < RXDESC_SIZE)\n"));
			precvbuf->reuse = true;
			rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
			DBG_8192C("%s()-%d: RX Warning!\n", __FUNCTION__, __LINE__);
		}
		else
		{
			rtw_reset_continual_io_error(adapter_to_dvobj(padapter));

			precvbuf->transfer_len = purb->actual_length;
			skb_put(precvbuf->pskb, purb->actual_length);
			skb_queue_tail(&precvpriv->rx_skb_queue, precvbuf->pskb);

			if (skb_queue_len(&precvpriv->rx_skb_queue)<=1)
				tasklet_schedule(&precvpriv->recv_tasklet);

			precvbuf->pskb = NULL;
			precvbuf->reuse = false;
			rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
		}
	}
	else
	{
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete : purb->status(%d) != 0\n", purb->status));

		DBG_8192C("###=> usb_read_port_complete => urb status(%d)\n", purb->status);

		if (rtw_inc_and_chk_continual_io_error(adapter_to_dvobj(padapter)) == true ) {
			padapter->bSurpriseRemoved = true;
		}

		switch (purb->status) {
			case -EINVAL:
			case -EPIPE:
			case -ENODEV:
			case -ESHUTDOWN:
				/* padapter->bSurpriseRemoved=true; */
				RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete:bSurpriseRemoved=true\n"));
			case -ENOENT:
				padapter->bDriverStopped=true;
				RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port_complete:bDriverStopped=true\n"));
				break;
			case -EPROTO:
			case -EILSEQ:
			case -ETIME:
			case -ECOMM:
			case -EOVERFLOW:
				{
					HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
					pHalData->srestpriv.Wifi_Error_Status = USB_READ_PORT_FAIL;
				}
				precvbuf->reuse = true;
				rtw_read_port(padapter, precvpriv->ff_hwaddr, 0, (unsigned char *)precvbuf);
				break;
			case -EINPROGRESS:
				DBG_8192C("ERROR: URB IS IN PROGRESS!/n");
				break;
			default:
				break;
		}

	}

exit:

;

}

static u32 usb_read_port(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *rmem)
{
	unsigned long irqL;
	int err;
	unsigned int pipe;
	SIZE_PTR tmpaddr=0;
	SIZE_PTR alignment=0;
	u32 ret = _SUCCESS;
	struct urb *purb = NULL;
	struct recv_buf	*precvbuf = (struct recv_buf *)rmem;
	struct adapter		*adapter = pintfhdl->padapter;
	struct dvobj_priv	*pdvobj = adapter_to_dvobj(adapter);
	struct recv_priv	*precvpriv = &adapter->recvpriv;
	struct usb_device	*pusbd = pdvobj->pusbdev;

	if (adapter->bDriverStopped || adapter->bSurpriseRemoved ||dvobj_to_pwrctl(pdvobj)->pnp_bstop_trx)
	{
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_read_port:( padapter->bDriverStopped ||padapter->bSurpriseRemoved ||pwrctl->pnp_bstop_trx)!!!\n"));
		return _FAIL;
	}

	if ((precvbuf->reuse == false) || (precvbuf->pskb == NULL)) {
		if (NULL != (precvbuf->pskb = skb_dequeue(&precvpriv->free_recv_skb_queue)))
			precvbuf->reuse = true;
	}

	rtl8188eu_init_recvbuf(adapter, precvbuf);

	/* re-assign for linux based on skb */
	if ((precvbuf->reuse == false) || (precvbuf->pskb == NULL)) {
		precvbuf->pskb = rtw_skb_alloc(MAX_RECVBUF_SZ + RECVBUFF_ALIGN_SZ);

		if (precvbuf->pskb == NULL) {
			RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("init_recvbuf(): alloc_skb fail!\n"));
			DBG_8192C("#### usb_read_port() alloc_skb fail!#####\n");
			return _FAIL;
		}

		tmpaddr = (SIZE_PTR)precvbuf->pskb->data;
		alignment = tmpaddr & (RECVBUFF_ALIGN_SZ-1);
		skb_reserve(precvbuf->pskb, (RECVBUFF_ALIGN_SZ - alignment));

		precvbuf->phead = precvbuf->pskb->head;
		precvbuf->pdata = precvbuf->pskb->data;
		precvbuf->ptail = skb_tail_pointer(precvbuf->pskb);
		precvbuf->pend = skb_end_pointer(precvbuf->pskb);
		precvbuf->pbuf = precvbuf->pskb->data;
	} else/* reuse skb */
	{
		precvbuf->phead = precvbuf->pskb->head;
		precvbuf->pdata = precvbuf->pskb->data;
		precvbuf->ptail = skb_tail_pointer(precvbuf->pskb);
		precvbuf->pend = skb_end_pointer(precvbuf->pskb);
		precvbuf->pbuf = precvbuf->pskb->data;

		precvbuf->reuse = false;
	}

	precvpriv->rx_pending_cnt++;

	purb = precvbuf->purb;

	/* translate DMA FIFO addr to pipehandle */
	pipe = ffaddr2pipehdl(pdvobj, addr);

	usb_fill_bulk_urb(purb, pusbd, pipe, precvbuf->pbuf,
			  MAX_RECVBUF_SZ, usb_read_port_complete,
			  precvbuf);/* context is precvbuf */

	err = usb_submit_urb(purb, GFP_ATOMIC);
	if ((err) && (err != (-EPERM))) {
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,
			 ("cannot submit rx in-token(err=0x%.8x), URB_STATUS =0x%.8x",
			 err, purb->status));
		DBG_8192C("cannot submit rx in-token(err = 0x%08x),urb_status = %d\n",
			  err, purb->status);
		ret = _FAIL;
	}
	return ret;
}

void rtl8188eu_xmit_tasklet(void *priv)
{
	int ret = false;
	struct adapter *padapter = (struct adapter*)priv;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;

	if (check_fwstate(&padapter->mlmepriv, _FW_UNDER_SURVEY) == true)
		return;

	while (1) {
		if (padapter->bDriverStopped ||
		    padapter->bSurpriseRemoved ||
		    padapter->bWritePortCancel) {
			DBG_8192C("xmit_tasklet => bDriverStopped or bSurpriseRemoved or bWritePortCancel\n");
			break;
		}

		ret = rtl8188eu_xmitframe_complete(padapter, pxmitpriv, NULL);

		if (!ret)
			break;
	}
}

void rtl8188eu_set_intf_ops(struct _io_ops	*pops)
{
	memset((u8 *)pops, 0, sizeof(struct _io_ops));

	pops->_read8 = &usb_read8;
	pops->_read16 = &usb_read16;
	pops->_read32 = &usb_read32;
	pops->_read_mem = &usb_read_mem;
	pops->_read_port = &usb_read_port;

	pops->_write8 = &usb_write8;
	pops->_write16 = &usb_write16;
	pops->_write32 = &usb_write32;
	pops->_writeN = &usb_writeN;

#ifdef CONFIG_USB_SUPPORT_ASYNC_VDN_REQ
	pops->_write8_async= &usb_async_write8;
	pops->_write16_async = &usb_async_write16;
	pops->_write32_async = &usb_async_write32;
#endif
	pops->_write_mem = &usb_write_mem;
	pops->_write_port = &usb_write_port;

	pops->_read_port_cancel = &usb_read_port_cancel;
	pops->_write_port_cancel = &usb_write_port_cancel;

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
	pops->_read_interrupt = &usb_read_interrupt;
#endif
}

void rtl8188eu_set_hw_type(struct adapter *padapter)
{
	padapter->chip_type = RTL8188E;
	padapter->HardwareType = HARDWARE_TYPE_RTL8188EU;
	DBG_88E("CHIP TYPE: RTL8188E\n");
}
