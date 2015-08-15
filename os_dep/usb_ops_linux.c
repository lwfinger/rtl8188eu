/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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
 *******************************************************************************/
#define _USB_OPS_LINUX_C_

#include <drv_types.h>
#include <usb_ops_linux.h>
#include <rtw_sreset.h>

#ifdef CONFIG_USB_SUPPORT_ASYNC_VDN_REQ
static void _usbctrl_vendorreq_async_callback(struct urb *urb, struct pt_regs *regs)
{
	if (urb) {
		if (urb->context) {
			rtw_mfree(urb->context);
		}
		usb_free_urb(urb);
	}
}

static int _usbctrl_vendorreq_async_write(struct usb_device *udev, u8 request,
	u16 value, u16 index, void *pdata, u16 len, u8 requesttype)
{
	int rc;
	unsigned int pipe;
	u8 reqtype;
	struct usb_ctrlrequest *dr;
	struct urb *urb;
	struct rtl819x_async_write_data {
		u8 data[VENDOR_CMD_MAX_DATA_LEN];
		struct usb_ctrlrequest dr;
	} *buf;


	if (requesttype == VENDOR_READ) {
		pipe = usb_rcvctrlpipe(udev, 0);/* read_in */
		reqtype =  REALTEK_USB_VENQT_READ;
	}
	else {
		pipe = usb_sndctrlpipe(udev, 0);/* write_out */
		reqtype =  REALTEK_USB_VENQT_WRITE;
	}

	buf = (struct rtl819x_async_write_data *)rtw_zmalloc(sizeof(*buf));
	if (!buf) {
		rc = -ENOMEM;
		goto exit;
	}

	urb = usb_alloc_urb(0, GFP_ATOMIC);
	if (!urb) {
		rtw_mfree((u8*)buf, sizeof(*buf));
		rc = -ENOMEM;
		goto exit;
	}

	dr = &buf->dr;

	dr->bRequestType = reqtype;
	dr->bRequest = request;
	dr->wValue = cpu_to_le16(value);
	dr->wIndex = cpu_to_le16(index);
	dr->wLength = cpu_to_le16(len);

	memcpy(buf, pdata, len);

	usb_fill_control_urb(urb, udev, pipe, (unsigned char *)dr, buf, len,
		_usbctrl_vendorreq_async_callback, buf);

	rc = usb_submit_urb(urb, GFP_ATOMIC);
	if (rc < 0) {
		rtw_mfree((u8*)buf, sizeof(*buf));
		usb_free_urb(urb);
	}

exit:
	return rc;
}

int usb_write_async(struct usb_device *udev, u32 addr, void *pdata, u16 len)
{
	u8 request;
	u8 requesttype;
	u16 wvalue;
	u16 index;

	int ret;

	requesttype = VENDOR_WRITE;/* write_out */
	request = REALTEK_USB_VENQT_CMD_REQ;
	index = REALTEK_USB_VENQT_CMD_IDX;/* n/a */

	wvalue = (u16)(addr&0x0000ffff);

	ret = _usbctrl_vendorreq_async_write(udev, request, wvalue, index, pdata, len, requesttype);

	return ret;
}

int usb_async_write8(struct intf_hdl *pintfhdl, u32 addr, u8 val)
{
	u8 data;
	int ret;
	struct dvobj_priv  *pdvobjpriv = (struct dvobj_priv  *)pintfhdl->pintf_dev;
	struct usb_device *udev=pdvobjpriv->pusbdev;

	;
	data = val;
	ret = usb_write_async(udev, addr, &data, 1);
	;

	return ret;
}

int usb_async_write16(struct intf_hdl *pintfhdl, u32 addr, u16 val)
{
	u16 data;
	int ret;
	struct dvobj_priv  *pdvobjpriv = (struct dvobj_priv  *)pintfhdl->pintf_dev;
	struct usb_device *udev=pdvobjpriv->pusbdev;

	;
	data = val;
	ret = usb_write_async(udev, addr, &data, 2);
	;

	return ret;
}

int usb_async_write32(struct intf_hdl *pintfhdl, u32 addr, u32 val)
{
	u32 data;
	int ret;
	struct dvobj_priv  *pdvobjpriv = (struct dvobj_priv  *)pintfhdl->pintf_dev;
	struct usb_device *udev=pdvobjpriv->pusbdev;

	;
	data = val;
	ret = usb_write_async(udev, addr, &data, 4);
	;

	return ret;
}
#endif /* CONFIG_USB_SUPPORT_ASYNC_VDN_REQ */

unsigned int ffaddr2pipehdl(struct dvobj_priv *pdvobj, u32 addr)
{
	unsigned int pipe=0, ep_num=0;
	struct usb_device *pusbd = pdvobj->pusbdev;

	if (addr == RECV_BULK_IN_ADDR) {
		pipe=usb_rcvbulkpipe(pusbd, pdvobj->RtInPipe[0]);

	} else if (addr == RECV_INT_IN_ADDR) {
		pipe=usb_rcvbulkpipe(pusbd, pdvobj->RtInPipe[1]);

	} else if (addr < HW_QUEUE_ENTRY) {
		ep_num = pdvobj->Queue2Pipe[addr];
		pipe = usb_sndbulkpipe(pusbd, ep_num);
	}

	return pipe;
}

struct zero_bulkout_context{
	void *pbuf;
	void *purb;
	void *pirp;
	void *padapter;
};

static void usb_bulkout_zero_complete(struct urb *purb, struct pt_regs *regs)
{
	struct zero_bulkout_context *pcontext = (struct zero_bulkout_context *)purb->context;

	/* DBG_8192C("+usb_bulkout_zero_complete\n"); */

	if (pcontext)
	{
		if (pcontext->pbuf)
		{
			rtw_mfree(pcontext->pbuf, sizeof(int));
		}

		if (pcontext->purb && (pcontext->purb==purb))
		{
			usb_free_urb(pcontext->purb);
		}


		rtw_mfree((u8*)pcontext, sizeof(struct zero_bulkout_context));
	}


}

static u32 usb_bulkout_zero(struct intf_hdl *pintfhdl, u32 addr)
{
	int pipe, status, len;
	u32 ret;
	unsigned char *pbuf;
	struct zero_bulkout_context *pcontext;
	struct urb *	purb = NULL;
	struct adapter *padapter = (struct adapter *)pintfhdl->padapter;
	struct dvobj_priv *pdvobj = adapter_to_dvobj(padapter);
	struct usb_device *pusbd = pdvobj->pusbdev;

	/* DBG_88E("%s\n", __func__); */


	if ((padapter->bDriverStopped) || (padapter->bSurpriseRemoved) ||(dvobj_to_pwrctl(pdvobj)->pnp_bstop_trx))
	{
		return _FAIL;
	}


	pcontext = (struct zero_bulkout_context *)rtw_zmalloc(sizeof(struct zero_bulkout_context));

	pbuf = (unsigned char *)rtw_zmalloc(sizeof(int));
	purb = usb_alloc_urb(0, GFP_ATOMIC);

	len = 0;
	pcontext->pbuf = pbuf;
	pcontext->purb = purb;
	pcontext->pirp = NULL;
	pcontext->padapter = padapter;


	/* translate DMA FIFO addr to pipehandle */
	/* pipe = ffaddr2pipehdl(pdvobj, addr); */

	usb_fill_bulk_urb(purb, pusbd, pipe,
				pbuf,
				len,
				usb_bulkout_zero_complete,
				pcontext);/* context is pcontext */

	status = usb_submit_urb(purb, GFP_ATOMIC);

	if (!status)
	{
		ret= _SUCCESS;
	}
	else
	{
		ret= _FAIL;
	}


	return _SUCCESS;

}

void usb_read_mem(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *rmem)
{

}

void usb_write_mem(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *wmem)
{

}


void usb_read_port_cancel(struct intf_hdl *pintfhdl)
{
	int i;
	struct recv_buf *precvbuf;
	struct adapter	*padapter = pintfhdl->padapter;
	precvbuf = (struct recv_buf *)padapter->recvpriv.precv_buf;

	DBG_88E("%s\n", __func__);

	padapter->bReadPortCancel = true;

	for (i=0; i < NR_RECVBUFF ; i++) {

		precvbuf->reuse = true;
		if (precvbuf->purb)	 {
			/* DBG_8192C("usb_read_port_cancel : usb_kill_urb\n"); */
			usb_kill_urb(precvbuf->purb);
		}
		precvbuf++;
	}

#ifdef CONFIG_USB_INTERRUPT_IN_PIPE
	usb_kill_urb(padapter->recvpriv.int_in_urb);
#endif
}

static void usb_write_port_complete(struct urb *purb, struct pt_regs *regs)
{
	unsigned long irqL;
	int i;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)purb->context;
	/* struct xmit_frame *pxmitframe = (struct xmit_frame *)pxmitbuf->priv_data; */
	/* struct adapter			*padapter = pxmitframe->padapter; */
	struct adapter	*padapter = pxmitbuf->padapter;
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	/* struct pkt_attrib *pattrib = &pxmitframe->attrib; */

;

	switch (pxmitbuf->flags)
	{
		case VO_QUEUE_INX:
			pxmitpriv->voq_cnt--;
			break;
		case VI_QUEUE_INX:
			pxmitpriv->viq_cnt--;
			break;
		case BE_QUEUE_INX:
			pxmitpriv->beq_cnt--;
			break;
		case BK_QUEUE_INX:
			pxmitpriv->bkq_cnt--;
			break;
		case HIGH_QUEUE_INX:
#ifdef CONFIG_AP_MODE
			rtw_chk_hi_queue_cmd(padapter);
#endif
			break;
		default:
			break;
	}


	if (padapter->bSurpriseRemoved || padapter->bDriverStopped ||padapter->bWritePortCancel)
	{
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_write_port_complete:bDriverStopped(%d) OR bSurpriseRemoved(%d)", padapter->bDriverStopped, padapter->bSurpriseRemoved));
		DBG_8192C("%s(): TX Warning! bDriverStopped(%d) OR bSurpriseRemoved(%d) bWritePortCancel(%d) pxmitbuf->ext_tag(%x)\n",
		__FUNCTION__,padapter->bDriverStopped, padapter->bSurpriseRemoved,padapter->bReadPortCancel,pxmitbuf->ext_tag);

		goto check_completion;
	}


	if (purb->status== 0) {

	} else {
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_write_port_complete : purb->status(%d) != 0\n", purb->status));
		DBG_88E("###=> urb_write_port_complete status(%d)\n",purb->status);
		if ((purb->status==-EPIPE)||(purb->status==-EPROTO))
		{
			/* usb_clear_halt(pusbdev, purb->pipe); */
			/* msleep(10); */
			sreset_set_wifi_error_status(padapter, USB_WRITE_PORT_FAIL);
		} else if (purb->status == -EINPROGRESS) {
			RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_write_port_complete: EINPROGESS\n"));
			goto check_completion;

		} else if (purb->status == -ENOENT) {
			DBG_88E("%s: -ENOENT\n", __func__);
			goto check_completion;

		} else if (purb->status == -ECONNRESET) {
			DBG_88E("%s: -ECONNRESET\n", __func__);
			goto check_completion;

		} else if (purb->status == -ESHUTDOWN) {
			RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_write_port_complete: ESHUTDOWN\n"));
			padapter->bDriverStopped=true;
			RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_write_port_complete:bDriverStopped=true\n"));

			goto check_completion;
		}
		else
		{
			padapter->bSurpriseRemoved=true;
			DBG_8192C("bSurpriseRemoved=true\n");
			/* rtl8192cu_trigger_gpio_0(padapter); */
			RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_write_port_complete:bSurpriseRemoved=true\n"));

			goto check_completion;
		}
	}

	{
		HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
		pHalData->srestpriv.last_tx_complete_time = jiffies;
	}
check_completion:
	_enter_critical(&pxmitpriv->lock_sctx, &irqL);
	rtw_sctx_done_err(&pxmitbuf->sctx,
		purb->status ? RTW_SCTX_DONE_WRITE_PORT_ERR : RTW_SCTX_DONE_SUCCESS);
	_exit_critical(&pxmitpriv->lock_sctx, &irqL);

	rtw_free_xmitbuf(pxmitpriv, pxmitbuf);

	/* if (rtw_txframes_pending(padapter)) */
	{
		tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
	}

;

}

u32 usb_write_port(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *wmem)
{
	unsigned long irqL;
	unsigned int pipe;
	int status;
	u32 ret = _FAIL, bwritezero = false;
	struct urb *purb = NULL;
	struct adapter *padapter = (struct adapter *)pintfhdl->padapter;
	struct dvobj_priv	*pdvobj = adapter_to_dvobj(padapter);
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)wmem;
	struct xmit_frame *pxmitframe = (struct xmit_frame *)pxmitbuf->priv_data;
	struct usb_device *pusbd = pdvobj->pusbdev;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;

	RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("+usb_write_port\n"));

	if ((padapter->bDriverStopped) || (padapter->bSurpriseRemoved) ||(dvobj_to_pwrctl(pdvobj)->pnp_bstop_trx)) {
		#ifdef DBG_TX
		DBG_88E(" DBG_TX %s:%d bDriverStopped%d, bSurpriseRemoved:%d, pnp_bstop_trx:%d\n",__FUNCTION__, __LINE__
			,padapter->bDriverStopped, padapter->bSurpriseRemoved, dvobj_to_pwrctl(pdvobj)->pnp_bstop_trx );
		#endif
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_write_port:( padapter->bDriverStopped ||padapter->bSurpriseRemoved ||pwrctl->pnp_bstop_trx)!!!\n"));
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_TX_DENY);
		goto exit;
	}

	_enter_critical(&pxmitpriv->lock, &irqL);

	switch (addr)
	{
		case VO_QUEUE_INX:
			pxmitpriv->voq_cnt++;
			pxmitbuf->flags = VO_QUEUE_INX;
			break;
		case VI_QUEUE_INX:
			pxmitpriv->viq_cnt++;
			pxmitbuf->flags = VI_QUEUE_INX;
			break;
		case BE_QUEUE_INX:
			pxmitpriv->beq_cnt++;
			pxmitbuf->flags = BE_QUEUE_INX;
			break;
		case BK_QUEUE_INX:
			pxmitpriv->bkq_cnt++;
			pxmitbuf->flags = BK_QUEUE_INX;
			break;
		case HIGH_QUEUE_INX:
			pxmitbuf->flags = HIGH_QUEUE_INX;
			break;
		default:
			pxmitbuf->flags = MGT_QUEUE_INX;
			break;
	}

	_exit_critical(&pxmitpriv->lock, &irqL);

	#ifdef DBG_TRX_STA_PKTS
	{
		struct sta_info *psta = NULL;
		struct sta_priv *pstapriv = &padapter->stapriv;
		int bmcast = IS_MCAST(pattrib->dst);
		u8 agg_num = 1;

		if (pxmitframe->agg_num>1)
			agg_num = pxmitframe->agg_num;

		if (bmcast)
			psta = rtw_get_bcmc_stainfo(padapter);

		else
			psta = rtw_get_stainfo(pstapriv, pattrib->dst);
		if (psta) {
			switch (pattrib->priority) {
			case 1:
			case 2:
				psta->tx_bk_cnt += agg_num;
				break;
			case 4:
			case 5:
				psta->tx_vi_cnt += agg_num;
				break;
			case 6:
			case 7:
				psta->tx_vo_cnt += agg_num;
				break;
			case 0:
			case 3:
			default:
				psta->tx_be_cnt += agg_num;
				break;
			}
		}
	}
	#endif

	purb	= pxmitbuf->pxmit_urb[0];

	/* translate DMA FIFO addr to pipehandle */
	pipe = ffaddr2pipehdl(pdvobj, addr);

#ifdef CONFIG_REDUCE_USB_TX_INT
	if ( (pxmitpriv->free_xmitbuf_cnt%NR_XMITBUFF == 0)
		|| (pxmitbuf->ext_tag == true) )
	{
		purb->transfer_flags  &=  (~URB_NO_INTERRUPT);
	} else {
		purb->transfer_flags  |=  URB_NO_INTERRUPT;
		/* DBG_8192C("URB_NO_INTERRUPT "); */
	}
#endif


	usb_fill_bulk_urb(purb, pusbd, pipe,
				pxmitframe->buf_addr, /*  pxmitbuf->pbuf */
				cnt,
				usb_write_port_complete,
				pxmitbuf);/* context is pxmitbuf */
	status = usb_submit_urb(purb, GFP_ATOMIC);
	if (!status) {
		HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
		pHalData->srestpriv.last_tx_time = jiffies;
	} else {
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_WRITE_PORT_ERR);
		DBG_88E("usb_write_port, status=%d\n", status);
		RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("usb_write_port(): usb_submit_urb, status=%x\n", status));

		switch (status) {
		case -ENODEV:
			padapter->bDriverStopped=true;
			break;
		default:
			break;
		}
		goto exit;
	}

	ret= _SUCCESS;

/*    Commented by Albert 2009/10/13 */
/*    We add the URB_ZERO_PACKET flag to urb so that the host will send the zero packet automatically. */
/*
	if (bwritezero == true)
	{
		usb_bulkout_zero(pintfhdl, addr);
	}
*/

	RT_TRACE(_module_hci_ops_os_c_,_drv_err_,("-usb_write_port\n"));

exit:
	if (ret != _SUCCESS)
		rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
;
	return ret;

}

void usb_write_port_cancel(struct intf_hdl *pintfhdl)
{
	int i, j;
	struct adapter	*padapter = pintfhdl->padapter;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)padapter->xmitpriv.pxmitbuf;

	DBG_88E("%s\n", __func__);

	padapter->bWritePortCancel = true;

	for (i=0; i<NR_XMITBUFF; i++) {
		for (j=0; j<8; j++) {
			if (pxmitbuf->pxmit_urb[j]) {
				usb_kill_urb(pxmitbuf->pxmit_urb[j]);
			}
		}
		pxmitbuf++;
	}

	pxmitbuf = (struct xmit_buf*)padapter->xmitpriv.pxmit_extbuf;
	for (i = 0; i < NR_XMIT_EXTBUFF; i++) {
		for (j=0; j<8; j++) {
			if (pxmitbuf->pxmit_urb[j]) {
				usb_kill_urb(pxmitbuf->pxmit_urb[j]);
			}
		}
		pxmitbuf++;
	}
}
