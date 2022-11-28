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
#include <wifi.h>
#include <osdep_intf.h>
#include <circ_buf.h>
#include <usb_ops.h>
#include <rtl8188e_hal.h>

s32	rtl8188eu_init_xmit_priv(struct adapter *padapter)
{
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);

	tasklet_init(&pxmitpriv->xmit_tasklet,
	     (void(*))rtl8188eu_xmit_tasklet,
	     (unsigned long)padapter);

	return _SUCCESS;
}

void	rtl8188eu_free_xmit_priv(struct adapter *padapter)
{
}

static u8 urb_zero_packet_chk(struct adapter *padapter, int sz)
{
	u8 blnSetTxDescOffset;
	HAL_DATA_TYPE	*pHalData	= GET_HAL_DATA(padapter);
	blnSetTxDescOffset = (((sz + TXDESC_SIZE) %  pHalData->UsbBulkOutSize) == 0)?1:0;

	return blnSetTxDescOffset;
}

static void rtl8188eu_cal_txdesc_chksum(struct tx_desc	*ptxdesc)
{
		u16	*usPtr = (u16*)ptxdesc;
		u32 count = 16;		/*  (32 bytes / 2 bytes per XOR) => 16 times */
		u32 index;
		u16 checksum = 0;

		/* Clear first */
		ptxdesc->txdw7 &= cpu_to_le32(0xffff0000);

		for (index = 0 ; index < count ; index++) {
			checksum = checksum ^ le16_to_cpu(*(__le16 *)(usPtr + index));
		}

		ptxdesc->txdw7 |= cpu_to_le32(0x0000ffff&checksum);

}
/*  */
/*  Description: In normal chip, we should send some packet to Hw which will be used by Fw */
/* 			in FW LPS mode. The function is to fill the Tx descriptor of this packets, then */
/* 			Fw can tell Hw to send these packet derectly. */
/*  */
void rtl8188e_fill_fake_txdesc(
	struct adapter *padapter,
	u8*			pDesc,
	u32			BufferLen,
	u8			IsPsPoll,
	u8			IsBTQosNull)
{
	struct tx_desc *ptxdesc;


	/*  Clear all status */
	ptxdesc = (struct tx_desc*)pDesc;
	memset(pDesc, 0, TXDESC_SIZE);

	/* offset 0 */
	ptxdesc->txdw0 |= cpu_to_le32( OWN | FSG | LSG); /* own, bFirstSeg, bLastSeg; */

	ptxdesc->txdw0 |= cpu_to_le32(((TXDESC_SIZE+OFFSET_SZ)<<OFFSET_SHT)&0x00ff0000); /* 32 bytes for TX Desc */

	ptxdesc->txdw0 |= cpu_to_le32(BufferLen&0x0000ffff); /*  Buffer size + command header */

	/* offset 4 */
	ptxdesc->txdw1 |= cpu_to_le32((QSLT_MGNT<<QSEL_SHT)&0x00001f00); /*  Fixed queue of Mgnt queue */

	/* Set NAVUSEHDR to prevent Ps-poll AId filed to be changed to error vlaue by Hw. */
	if (IsPsPoll)
	{
		ptxdesc->txdw1 |= cpu_to_le32(NAVUSEHDR);
	}
	else
	{
		ptxdesc->txdw4 |= cpu_to_le32(BIT(7)); /*  Hw set sequence number */
		ptxdesc->txdw3 |= cpu_to_le32((8 <<28)); /* set bit3 to 1. Suugested by TimChen. 2009.12.29. */
	}

	if (true == IsBTQosNull)
	{
		ptxdesc->txdw2 |= cpu_to_le32(BIT(23)); /*  BT NULL */
	}

	/* offset 16 */
	ptxdesc->txdw4 |= cpu_to_le32(BIT(8));/* driver uses rate */

	/*  USB interface drop packet if the checksum of descriptor isn't correct. */
	/*  Using this checksum can let hardware recovery from packet bulk out error (e.g. Cancel URC, Bulk out error.). */
	rtl8188eu_cal_txdesc_chksum(ptxdesc);
}

static void fill_txdesc_sectype(struct pkt_attrib *pattrib, struct tx_desc *ptxdesc)
{
	if ((pattrib->encrypt > 0) && !pattrib->bswenc) {
		switch (pattrib->encrypt) {
		/* SEC_TYPE : 0:NO_ENC,1:WEP40/TKIP,2:WAPI,3:AES */
		case _WEP40_:
		case _WEP104_:
			ptxdesc->txdw1 |= cpu_to_le32((0x01<<SEC_TYPE_SHT)&0x00c00000);
			ptxdesc->txdw2 |= cpu_to_le32(0x7 << AMPDU_DENSITY_SHT);
			break;
		case _TKIP_:
		case _TKIP_WTMIC_:
			ptxdesc->txdw1 |= cpu_to_le32((0x01<<SEC_TYPE_SHT)&0x00c00000);
			ptxdesc->txdw2 |= cpu_to_le32(0x7 << AMPDU_DENSITY_SHT);
			break;
		case _AES_:
			ptxdesc->txdw1 |= cpu_to_le32((0x03<<SEC_TYPE_SHT)&0x00c00000);
			ptxdesc->txdw2 |= cpu_to_le32(0x7 << AMPDU_DENSITY_SHT);
			break;
		case _NO_PRIVACY_:
		default:
			break;
		}
	}
}

static void fill_txdesc_vcs(struct pkt_attrib *pattrib, __le32 *pdw)
{
	switch (pattrib->vcs_mode)
	{
		case RTS_CTS:
			*pdw |= cpu_to_le32(RTS_EN);
			break;
		case CTS_TO_SELF:
			*pdw |= cpu_to_le32(CTS_2_SELF);
			break;
		case NONE_VCS:
		default:
			break;
	}

	if (pattrib->vcs_mode) {
		*pdw |= cpu_to_le32(HW_RTS_EN);

		/*  Set RTS BW */
		if (pattrib->ht_en)
		{
			*pdw |= (pattrib->bwmode&HT_CHANNEL_WIDTH_40)?	cpu_to_le32(BIT(27)):0;

			if (pattrib->ch_offset == HAL_PRIME_CHNL_OFFSET_LOWER)
				*pdw |= cpu_to_le32((0x01<<28)&0x30000000);
			else if (pattrib->ch_offset == HAL_PRIME_CHNL_OFFSET_UPPER)
				*pdw |= cpu_to_le32((0x02<<28)&0x30000000);
			else if (pattrib->ch_offset == HAL_PRIME_CHNL_OFFSET_DONT_CARE)
				*pdw |= 0;
			else
				*pdw |= cpu_to_le32((0x03<<28)&0x30000000);
		}
	}
}

static void fill_txdesc_phy(struct pkt_attrib *pattrib, __le32 *pdw)
{
	/* DBG_8192C("bwmode=%d, ch_off=%d\n", pattrib->bwmode, pattrib->ch_offset); */

	if (pattrib->ht_en)
	{
		*pdw |= (pattrib->bwmode&HT_CHANNEL_WIDTH_40)?	cpu_to_le32(BIT(25)):0;

		if (pattrib->ch_offset == HAL_PRIME_CHNL_OFFSET_LOWER)
			*pdw |= cpu_to_le32((0x01<<DATA_SC_SHT)&0x003f0000);
		else if (pattrib->ch_offset == HAL_PRIME_CHNL_OFFSET_UPPER)
			*pdw |= cpu_to_le32((0x02<<DATA_SC_SHT)&0x003f0000);
		else if (pattrib->ch_offset == HAL_PRIME_CHNL_OFFSET_DONT_CARE)
			*pdw |= 0;
		else
			*pdw |= cpu_to_le32((0x03<<DATA_SC_SHT)&0x003f0000);
	}
}


static s32 update_txdesc(struct xmit_frame *pxmitframe, u8 *pmem, s32 sz ,u8 bagg_pkt)
{
	int	pull=0;
	uint	qsel;
	u8 data_rate,pwr_status,offset;
	struct adapter			*padapter = pxmitframe->padapter;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct pkt_attrib	*pattrib = &pxmitframe->attrib;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct tx_desc	*ptxdesc = (struct tx_desc *)pmem;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &pmlmeext->mlmext_info;
	sint	bmcst = IS_MCAST(pattrib->ra);
#ifdef CONFIG_P2P
	struct wifidirect_info*	pwdinfo = &padapter->wdinfo;
#endif /* CONFIG_P2P */

	if (padapter->registrypriv.mp_mode == 0) {
		if ((!bagg_pkt) &&urb_zero_packet_chk(padapter, sz== 0)) {
			ptxdesc = (struct tx_desc *)(pmem+PACKET_OFFSET_SZ);
			pull = 1;
		}
	}

	memset(ptxdesc, 0, sizeof(struct tx_desc));

        /* 4 offset 0 */
	ptxdesc->txdw0 |= cpu_to_le32(OWN | FSG | LSG);
	ptxdesc->txdw0 |= cpu_to_le32(sz & 0x0000ffff);/* update TXPKTSIZE */

	offset = TXDESC_SIZE + OFFSET_SZ;

	ptxdesc->txdw0 |= cpu_to_le32(((offset) << OFFSET_SHT) & 0x00ff0000);/* 32 bytes for TX Desc */

	if (bmcst) ptxdesc->txdw0 |= cpu_to_le32(BMC);

	if (padapter->registrypriv.mp_mode == 0) {
		if (!bagg_pkt) {
			if ((pull) && (pxmitframe->pkt_offset>0))
				pxmitframe->pkt_offset = pxmitframe->pkt_offset -1;
		}
	}
	/*  pkt_offset, unit:8 bytes padding */
	if (pxmitframe->pkt_offset > 0)
		ptxdesc->txdw1 |= cpu_to_le32((pxmitframe->pkt_offset << 26) & 0x7c000000);

	/* driver uses rate */
	ptxdesc->txdw4 |= cpu_to_le32(USERATE);/* rate control always by driver */

	if ((pxmitframe->frame_tag&0x0f) == DATA_FRAMETAG)
	{
		/* DBG_8192C("pxmitframe->frame_tag == DATA_FRAMETAG\n"); */

		/* offset 4 */
		ptxdesc->txdw1 |= cpu_to_le32(pattrib->mac_id&0x3F);

		qsel = (uint)(pattrib->qsel & 0x0000001f);
		/* DBG_8192C("==> macid(%d) qsel:0x%02x\n",pattrib->mac_id,qsel); */
		ptxdesc->txdw1 |= cpu_to_le32((qsel << QSEL_SHT) & 0x00001f00);

		ptxdesc->txdw1 |= cpu_to_le32((pattrib->raid<< RATE_ID_SHT) & 0x000F0000);

		fill_txdesc_sectype(pattrib, ptxdesc);

		if (pattrib->ampdu_en==true) {
			ptxdesc->txdw2 |= cpu_to_le32(AGG_EN);/* AGG EN */
			ptxdesc->txdw6 = cpu_to_le32(0x6666f800);
		} else {
			ptxdesc->txdw2 |= cpu_to_le32(AGG_BK);/* AGG BK */
		}

		/* offset 8 */


		/* offset 12 */
		ptxdesc->txdw3 |= cpu_to_le32((pattrib->seqnum<< SEQ_SHT)&0x0FFF0000);


		/* offset 16 , offset 20 */
		if (pattrib->qos_en)
			ptxdesc->txdw4 |= cpu_to_le32(QOS);/* QoS */

		/* offset 20 */
		if (pxmitframe->agg_num > 1) {
			/* DBG_8192C("%s agg_num:%d\n",__FUNCTION__,pxmitframe->agg_num ); */
			ptxdesc->txdw5 |= cpu_to_le32((pxmitframe->agg_num << USB_TXAGG_NUM_SHT) & 0xFF000000);
		}

		if ((pattrib->ether_type != 0x888e) &&
		    (pattrib->ether_type != 0x0806) &&
		    (pattrib->ether_type != 0x88b4) &&
		    (pattrib->dhcp_pkt != 1))
		{
			/* Non EAP & ARP & DHCP type data packet */

			fill_txdesc_vcs(pattrib, &ptxdesc->txdw4);
			fill_txdesc_phy(pattrib, &ptxdesc->txdw4);

			ptxdesc->txdw4 |= cpu_to_le32(0x00000008);/* RTS Rate=24M */
			ptxdesc->txdw5 |= cpu_to_le32(0x0001ff00);/* DATA/RTS  Rate FB LMT */

	#if (RATE_ADAPTIVE_SUPPORT == 1)
			if (pattrib->ht_en) {
				if ( ODM_RA_GetShortGI_8188E(&pHalData->odmpriv,pattrib->mac_id))
					ptxdesc->txdw5 |= cpu_to_le32(SGI);/* SGI */
			}

			data_rate =ODM_RA_GetDecisionRate_8188E(&pHalData->odmpriv,pattrib->mac_id);
			/* for debug */
			if (padapter->fix_rate!= 0xFF) {

				data_rate = padapter->fix_rate;
				ptxdesc->txdw4 |= cpu_to_le32(DISDATAFB);
			}

			ptxdesc->txdw5 |= cpu_to_le32(data_rate & 0x3F);

			#if (POWER_TRAINING_ACTIVE==1)
			pwr_status = ODM_RA_GetHwPwrStatus_8188E(&pHalData->odmpriv,pattrib->mac_id);
			ptxdesc->txdw4 |=cpu_to_le32( (pwr_status & 0x7)<< PWR_STATUS_SHT);
			#endif /* POWER_TRAINING_ACTIVE==1) */
	#else/* if (RATE_ADAPTIVE_SUPPORT == 1) */

			if (pattrib->ht_en)
				ptxdesc->txdw5 |= cpu_to_le32(SGI);/* SGI */

			data_rate = 0x13; /* default rate: MCS7 */
			 if (padapter->fix_rate!= 0xFF) {/* rate control by iwpriv */
				data_rate = padapter->fix_rate;
				ptxdesc->txdw4 | cpu_to_le32(DISDATAFB);
			}
			ptxdesc->txdw5 |= cpu_to_le32(data_rate & 0x3F);

	#endif/* if (RATE_ADAPTIVE_SUPPORT == 1) */

		}
		else
		{
			/*  EAP data packet and ARP packet and DHCP. */
			/*  Use the 1M data rate to send the EAP/ARP packet. */
			/*  This will maybe make the handshake smooth. */

			ptxdesc->txdw2 |= cpu_to_le32(AGG_BK);/* AGG BK */

			if (pmlmeinfo->preamble_mode == PREAMBLE_SHORT)
				ptxdesc->txdw4 |= cpu_to_le32(BIT(24));/*  DATA_SHORT */

			ptxdesc->txdw5 |= cpu_to_le32(MRateToHwRate(pmlmeext->tx_rate));
		}
	} else if ((pxmitframe->frame_tag&0x0f)== MGNT_FRAMETAG) {
		/* offset 4 */
		ptxdesc->txdw1 |= cpu_to_le32(pattrib->mac_id&0x3f);

		qsel = (uint)(pattrib->qsel&0x0000001f);
		ptxdesc->txdw1 |= cpu_to_le32((qsel<<QSEL_SHT)&0x00001f00);

		ptxdesc->txdw1 |= cpu_to_le32((pattrib->raid<< RATE_ID_SHT) & 0x000f0000);

		/* offset 8 */
		/* CCX-TXRPT ack for xmit mgmt frames. */
		if (pxmitframe->ack_report) {
			#ifdef DBG_CCX
			static u16 ccx_sw = 0x123;
			ptxdesc->txdw7 |= cpu_to_le32(((ccx_sw)<<16)&0x0fff0000);
			DBG_88E("%s set ccx, sw:0x%03x\n", __func__, ccx_sw);
			ccx_sw = (ccx_sw+1)%0xfff;
			#endif
			ptxdesc->txdw2 |= cpu_to_le32(BIT(19));
		}

		/* offset 12 */
		ptxdesc->txdw3 |= cpu_to_le32((pattrib->seqnum<<SEQ_SHT)&0x0FFF0000);

		/* offset 20 */
		ptxdesc->txdw5 |= cpu_to_le32(RTY_LMT_EN);/* retry limit enable */
		if (pattrib->retry_ctrl == true)
			ptxdesc->txdw5 |= cpu_to_le32(0x00180000);/* retry limit = 6 */
		else
			ptxdesc->txdw5 |= cpu_to_le32(0x00300000);/* retry limit = 12 */

		ptxdesc->txdw5 |= cpu_to_le32(MRateToHwRate(pmlmeext->tx_rate));
	} else if ((pxmitframe->frame_tag&0x0f) == TXAGG_FRAMETAG) {
		DBG_8192C("pxmitframe->frame_tag == TXAGG_FRAMETAG\n");
	} else {
		DBG_8192C("pxmitframe->frame_tag = %d\n", pxmitframe->frame_tag);

		/* offset 4 */
		ptxdesc->txdw1 |= cpu_to_le32((4)&0x3f);/* CAM_ID(MAC_ID) */

		ptxdesc->txdw1 |= cpu_to_le32((6<< RATE_ID_SHT) & 0x000f0000);/* raid */

		/* offset 8 */

		/* offset 12 */
		ptxdesc->txdw3 |= cpu_to_le32((pattrib->seqnum<<SEQ_SHT)&0x0fff0000);

		/* offset 20 */
		ptxdesc->txdw5 |= cpu_to_le32(MRateToHwRate(pmlmeext->tx_rate));
	}

	/*  2009.11.05. tynli_test. Suggested by SD4 Filen for FW LPS. */
	/*  (1) The sequence number of each non-Qos frame / broadcast / multicast / */
	/*  mgnt frame should be controled by Hw because Fw will also send null data */
	/*  which we cannot control when Fw LPS enable. */
	/*  --> default enable non-Qos data sequense number. 2010.06.23. by tynli. */
	/*  (2) Enable HW SEQ control for beacon packet, because we use Hw beacon. */
	/*  (3) Use HW Qos SEQ to control the seq num of Ext port non-Qos packets. */
	/*  2010.06.23. Added by tynli. */
	if (!pattrib->qos_en) {
		ptxdesc->txdw3 |= cpu_to_le32(EN_HWSEQ); /*  Hw set sequence number */
		ptxdesc->txdw4 |= cpu_to_le32(HW_SSN);	/*  Hw set sequence number */
	}

	ODM_SetTxAntByTxInfo_88E(&pHalData->odmpriv, pmem, pattrib->mac_id);

	rtl8188eu_cal_txdesc_chksum(ptxdesc);
	_dbg_dump_tx_info(padapter,pxmitframe->frame_tag,ptxdesc);
	return pull;
}

/* for non-agg data frame or  management frame */
static s32 rtw_dump_xframe(struct adapter *padapter, struct xmit_frame *pxmitframe)
{
	s32 ret = _SUCCESS;
	s32 inner_ret = _SUCCESS;
	int t, sz, w_sz, pull=0;
	u8 *mem_addr;
	u32 ff_hwaddr;
	struct xmit_buf *pxmitbuf = pxmitframe->pxmitbuf;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	if ((pxmitframe->frame_tag == DATA_FRAMETAG) &&
	    (pxmitframe->attrib.ether_type != 0x0806) &&
	    (pxmitframe->attrib.ether_type != 0x888e) &&
	    (pxmitframe->attrib.ether_type != 0x88b4) &&
	    (pxmitframe->attrib.dhcp_pkt != 1))
		rtw_issue_addbareq_cmd(padapter, pxmitframe);
	mem_addr = pxmitframe->buf_addr;

	RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("rtw_dump_xframe()\n"));

	for (t = 0; t < pattrib->nr_frags; t++) {
		if (inner_ret != _SUCCESS && ret == _SUCCESS)
			ret = _FAIL;

		if (t != (pattrib->nr_frags - 1))
		{
			RT_TRACE(_module_rtl871x_xmit_c_,_drv_err_,("pattrib->nr_frags=%d\n", pattrib->nr_frags));

			sz = pxmitpriv->frag_len;
			sz = sz - 4 - (psecuritypriv->sw_encrypt ? 0 : pattrib->icv_len);
		}
		else /* no frag */
		{
			sz = pattrib->last_txcmdsz;
		}

		pull = update_txdesc(pxmitframe, mem_addr, sz, false);

		if (pull)
		{
			mem_addr += PACKET_OFFSET_SZ; /* pull txdesc head */

			/* pxmitbuf ->pbuf = mem_addr; */
			pxmitframe->buf_addr = mem_addr;

			w_sz = sz + TXDESC_SIZE;
		}
		else
		{
			w_sz = sz + TXDESC_SIZE + PACKET_OFFSET_SZ;
		}
		ff_hwaddr = rtw_get_ff_hwaddr(pxmitframe);

		inner_ret = rtw_write_port(padapter, ff_hwaddr, w_sz, (unsigned char*)pxmitbuf);

		rtw_count_tx_stats(padapter, pxmitframe, sz);

		RT_TRACE(_module_rtl871x_xmit_c_,_drv_info_,("rtw_write_port, w_sz=%d\n", w_sz));
		/* DBG_8192C("rtw_write_port, w_sz=%d, sz=%d, txdesc_sz=%d, tid=%d\n", w_sz, sz, w_sz-sz, pattrib->priority); */

		mem_addr += w_sz;

		mem_addr = (u8 *)RND4(((SIZE_PTR)(mem_addr)));
	}

	rtw_free_xmitframe(pxmitpriv, pxmitframe);

	if  (ret != _SUCCESS)
		rtw_sctx_done_err(&pxmitbuf->sctx, RTW_SCTX_DONE_UNKNOWN);

	return ret;
}

static u32 xmitframe_need_length(struct xmit_frame *pxmitframe)
{
	struct pkt_attrib *pattrib = &pxmitframe->attrib;

	u32	len = 0;

	/*  no consider fragement */
	len = pattrib->hdrlen + pattrib->iv_len +
		SNAP_SIZE + sizeof(u16) +
		pattrib->pktlen +
		((pattrib->bswenc) ? pattrib->icv_len : 0);

	if (pattrib->encrypt ==_TKIP_)
		len += 8;

	return len;
}

#define IDEA_CONDITION 1	/*  check all packets before enqueue */
s32 rtl8188eu_xmitframe_complete(struct adapter *padapter, struct xmit_priv *pxmitpriv, struct xmit_buf *pxmitbuf)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	struct xmit_frame *pxmitframe = NULL;
	struct xmit_frame *pfirstframe = NULL;

	/*  aggregate variable */
	struct hw_xmit *phwxmit;
	struct sta_info *psta = NULL;
	struct tx_servq *ptxservq = NULL;

	unsigned long irqL;
	struct list_head *xmitframe_plist = NULL, *xmitframe_phead = NULL;

	u32	pbuf;	/*  next pkt address */
	u32	pbuf_tail;	/*  last pkt tail */
	u32	len;	/*  packet length, except TXDESC_SIZE and PKT_OFFSET */

	u32	bulkSize = pHalData->UsbBulkOutSize;
	u8	descCount;
	u32	bulkPtr;

	/*  dump frame variable */
	u32 ff_hwaddr;

#ifndef IDEA_CONDITION
	int res = _SUCCESS;
#endif

	RT_TRACE(_module_rtl8192c_xmit_c_, _drv_info_, ("+xmitframe_complete\n"));


	/*  check xmitbuffer is ok */
	if (pxmitbuf == NULL) {
		pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);
		if (pxmitbuf == NULL) {
			/* DBG_88E("%s #1, connot alloc xmitbuf!!!!\n",__FUNCTION__); */
			return false;
		}
	}

/* DBG_8192C("%s =====================================\n",__FUNCTION__); */
	/* 3 1. pick up first frame */
	do {
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

		pxmitframe = rtw_dequeue_xframe(pxmitpriv, pxmitpriv->hwxmits, pxmitpriv->hwxmit_entry);
		if (pxmitframe == NULL) {
			/*  no more xmit frame, release xmit buffer */
			/* DBG_8192C("no more xmit frame ,return\n"); */
			rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
			return false;
		}

#ifndef IDEA_CONDITION
		if (pxmitframe->frame_tag != DATA_FRAMETAG) {
			RT_TRACE(_module_rtl8192c_xmit_c_, _drv_err_,
				 ("xmitframe_complete: frame tag(%d) is not DATA_FRAMETAG(%d)!\n",
				  pxmitframe->frame_tag, DATA_FRAMETAG));
/* 			rtw_free_xmitframe(pxmitpriv, pxmitframe); */
			continue;
		}

		/*  TID 0~15 */
		if ((pxmitframe->attrib.priority < 0) ||
		    (pxmitframe->attrib.priority > 15)) {
			RT_TRACE(_module_rtl8192c_xmit_c_, _drv_err_,
				 ("xmitframe_complete: TID(%d) should be 0~15!\n",
				  pxmitframe->attrib.priority));
/* 			rtw_free_xmitframe(pxmitpriv, pxmitframe); */
			continue;
		}
#endif
		/* DBG_8192C("==> pxmitframe->attrib.priority:%d\n",pxmitframe->attrib.priority); */
		pxmitframe->pxmitbuf = pxmitbuf;
		pxmitframe->buf_addr = pxmitbuf->pbuf;
		pxmitbuf->priv_data = pxmitframe;

		pxmitframe->agg_num = 1; /*  alloc xmitframe should assign to 1. */
		pxmitframe->pkt_offset = 1; /*  first frame of aggregation, reserve offset */

		if (rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe) == false) {
			DBG_88E("%s coalesce 1st xmitframe failed\n",__FUNCTION__);
			continue;
		}

		/*  always return ndis_packet after rtw_xmitframe_coalesce */
		rtw_os_xmit_complete(padapter, pxmitframe);

		break;
	} while (1);

	/* 3 2. aggregate same priority and same DA(AP or STA) frames */
	pfirstframe = pxmitframe;
	len = xmitframe_need_length(pfirstframe) + TXDESC_SIZE+(pfirstframe->pkt_offset*PACKET_OFFSET_SZ);
	pbuf_tail = len;
	pbuf = _RND8(pbuf_tail);

	/*  check pkt amount in one bulk */
	descCount = 0;
	bulkPtr = bulkSize;
	if (pbuf < bulkPtr)
		descCount++;
	else {
		descCount = 0;
		bulkPtr = ((pbuf / bulkSize) + 1) * bulkSize; /*  round to next bulkSize */
	}

	/*  dequeue same priority packet from station tx queue */
	/* psta = pfirstframe->attrib.psta; */
	psta = rtw_get_stainfo(&padapter->stapriv, pfirstframe->attrib.ra);
	if (pfirstframe->attrib.psta != psta) {
		DBG_88E("%s, pattrib->psta(%p) != psta(%p)\n", __func__, pfirstframe->attrib.psta, psta);
	}
	if (psta == NULL) {
		DBG_8192C("rtw_xmit_classifier: psta == NULL\n");
	}
	if (!(psta->state &_FW_LINKED)) {
		DBG_88E("%s, psta->state(0x%x) != _FW_LINKED\n", __func__, psta->state);
	}

	switch (pfirstframe->attrib.priority) {
		case 1:
		case 2:
			ptxservq = &psta->sta_xmitpriv.bk_q;
			phwxmit = pxmitpriv->hwxmits + 3;
			break;

		case 4:
		case 5:
			ptxservq = &psta->sta_xmitpriv.vi_q;
			phwxmit = pxmitpriv->hwxmits + 1;
			break;

		case 6:
		case 7:
			ptxservq = &psta->sta_xmitpriv.vo_q;
			phwxmit = pxmitpriv->hwxmits;
			break;

		case 0:
		case 3:
		default:
			ptxservq = &psta->sta_xmitpriv.be_q;
			phwxmit = pxmitpriv->hwxmits + 2;
			break;
	}
/* DBG_8192C("==> pkt_no=%d,pkt_len=%d,len=%d,RND8_LEN=%d,pkt_offset=0x%02x\n", */
	/* pxmitframe->agg_num,pxmitframe->attrib.last_txcmdsz,len,pbuf,pxmitframe->pkt_offset ); */

	spin_lock_bh(&pxmitpriv->lock);

	xmitframe_phead = get_list_head(&ptxservq->sta_pending);
	xmitframe_plist = get_next(xmitframe_phead);

	while (rtw_end_of_queue_search(xmitframe_phead, xmitframe_plist) == false)
	{
		pxmitframe = LIST_CONTAINOR(xmitframe_plist, struct xmit_frame, list);
		xmitframe_plist = get_next(xmitframe_plist);

		pxmitframe->agg_num = 0; /*  not first frame of aggregation */
		pxmitframe->pkt_offset = 0; /*  not first frame of aggregation, no need to reserve offset */

		len = xmitframe_need_length(pxmitframe) + TXDESC_SIZE +(pxmitframe->pkt_offset*PACKET_OFFSET_SZ);

		if (_RND8(pbuf + len) > MAX_XMITBUF_SZ) {
			pxmitframe->agg_num = 1;
			pxmitframe->pkt_offset = 1;
			break;
		}
		rtw_list_delete(&pxmitframe->list);
		ptxservq->qcnt--;
		phwxmit->accnt--;

#ifndef IDEA_CONDITION
		/*  suppose only data frames would be in queue */
		if (pxmitframe->frame_tag != DATA_FRAMETAG) {
			RT_TRACE(_module_rtl8192c_xmit_c_, _drv_err_,
				 ("xmitframe_complete: frame tag(%d) is not DATA_FRAMETAG(%d)!\n",
				  pxmitframe->frame_tag, DATA_FRAMETAG));
			rtw_free_xmitframe(pxmitpriv, pxmitframe);
			continue;
		}

		/*  TID 0~15 */
		if ((pxmitframe->attrib.priority < 0) ||
		    (pxmitframe->attrib.priority > 15)) {
			RT_TRACE(_module_rtl8192c_xmit_c_, _drv_err_,
				 ("xmitframe_complete: TID(%d) should be 0~15!\n",
				  pxmitframe->attrib.priority));
			rtw_free_xmitframe(pxmitpriv, pxmitframe);
			continue;
		}
#endif

/* 		pxmitframe->pxmitbuf = pxmitbuf; */
		pxmitframe->buf_addr = pxmitbuf->pbuf + pbuf;

		if (rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe) == false) {
			DBG_88E("%s coalesce failed\n",__FUNCTION__);
			rtw_free_xmitframe(pxmitpriv, pxmitframe);
			continue;
		}

		/* DBG_8192C("==> pxmitframe->attrib.priority:%d\n",pxmitframe->attrib.priority); */
		/*  always return ndis_packet after rtw_xmitframe_coalesce */
		rtw_os_xmit_complete(padapter, pxmitframe);

		/*  (len - TXDESC_SIZE) == pxmitframe->attrib.last_txcmdsz */
		update_txdesc(pxmitframe, pxmitframe->buf_addr, pxmitframe->attrib.last_txcmdsz,true);

		/*  don't need xmitframe any more */
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

		/*  handle pointer and stop condition */
		pbuf_tail = pbuf + len;
		pbuf = _RND8(pbuf_tail);


		pfirstframe->agg_num++;
		if (MAX_TX_AGG_PACKET_NUMBER == pfirstframe->agg_num)
			break;

		if (pbuf < bulkPtr) {
			descCount++;
			if (descCount == pHalData->UsbTxAggDescNum)
				break;
		} else {
			descCount = 0;
			bulkPtr = ((pbuf / bulkSize) + 1) * bulkSize;
		}
	}/* end while ( aggregate same priority and same DA(AP or STA) frames) */


	if (_rtw_queue_empty(&ptxservq->sta_pending) == true)
		rtw_list_delete(&ptxservq->tx_pending);

	spin_unlock_bh(&pxmitpriv->lock);
	if ((pfirstframe->attrib.ether_type != 0x0806) &&
	    (pfirstframe->attrib.ether_type != 0x888e) &&
	    (pfirstframe->attrib.ether_type != 0x88b4) &&
	    (pfirstframe->attrib.dhcp_pkt != 1))
	{
		rtw_issue_addbareq_cmd(padapter, pfirstframe);
	}
	/* 3 3. update first frame txdesc */
	if ((pbuf_tail % bulkSize) == 0) {
		/*  remove pkt_offset */
		pbuf_tail -= PACKET_OFFSET_SZ;
		pfirstframe->buf_addr += PACKET_OFFSET_SZ;
		pfirstframe->pkt_offset--;
	}

	update_txdesc(pfirstframe, pfirstframe->buf_addr, pfirstframe->attrib.last_txcmdsz,true);

	/* 3 4. write xmit buffer to USB FIFO */
	ff_hwaddr = rtw_get_ff_hwaddr(pfirstframe);
	/*  xmit address == ((xmit_frame*)pxmitbuf->priv_data)->buf_addr */
	rtw_write_port(padapter, ff_hwaddr, pbuf_tail, (u8*)pxmitbuf);


	/* 3 5. update statisitc */
	pbuf_tail -= (pfirstframe->agg_num * TXDESC_SIZE);
	pbuf_tail -= (pfirstframe->pkt_offset * PACKET_OFFSET_SZ);


	rtw_count_tx_stats(padapter, pfirstframe, pbuf_tail);

	rtw_free_xmitframe(pxmitpriv, pfirstframe);

	return true;
}

static s32 xmitframe_direct(struct adapter *padapter, struct xmit_frame *pxmitframe)
{
	s32 res = _SUCCESS;
/* DBG_8192C("==> %s\n",__FUNCTION__); */

	res = rtw_xmitframe_coalesce(padapter, pxmitframe->pkt, pxmitframe);
	if (res == _SUCCESS) {
		rtw_dump_xframe(padapter, pxmitframe);
	}
	else {
		DBG_8192C("==> %s xmitframe_coalsece failed\n",__FUNCTION__);
	}

	return res;
}

/*
 * Return
 *	true	dump packet directly
 *	false	enqueue packet
 */
static s32 pre_xmitframe(struct adapter *padapter, struct xmit_frame *pxmitframe)
{
        unsigned long irqL;
	s32 res;
	struct xmit_buf *pxmitbuf = NULL;
	struct xmit_priv *pxmitpriv = &padapter->xmitpriv;
	struct pkt_attrib *pattrib = &pxmitframe->attrib;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;

	spin_lock_bh(&pxmitpriv->lock);

	if (rtw_txframes_sta_ac_pending(padapter, pattrib) > 0)
		goto enqueue;

	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY|_FW_UNDER_LINKING) == true)
		goto enqueue;

	pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);
	if (pxmitbuf == NULL)
		goto enqueue;

	spin_unlock_bh(&pxmitpriv->lock);

	pxmitframe->pxmitbuf = pxmitbuf;
	pxmitframe->buf_addr = pxmitbuf->pbuf;
	pxmitbuf->priv_data = pxmitframe;

	if (xmitframe_direct(padapter, pxmitframe) != _SUCCESS) {
		rtw_free_xmitbuf(pxmitpriv, pxmitbuf);
		rtw_free_xmitframe(pxmitpriv, pxmitframe);
	}

	return true;

enqueue:
	res = rtw_xmitframe_enqueue(padapter, pxmitframe);
	spin_unlock_bh(&pxmitpriv->lock);

	if (res != _SUCCESS) {
		RT_TRACE(_module_xmit_osdep_c_, _drv_err_, ("pre_xmitframe: enqueue xmitframe fail\n"));
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

		/*  Trick, make the statistics correct */
		pxmitpriv->tx_pkts--;
		pxmitpriv->tx_drop++;
		return true;
	}

	return false;
}

s32 rtl8188eu_mgnt_xmit(struct adapter *padapter, struct xmit_frame *pmgntframe)
{
	return rtw_dump_xframe(padapter, pmgntframe);
}

/*
 * Return
 *	true	dump packet directly ok
 *	false	temporary can't transmit packets to hardware
 */
s32 rtl8188eu_hal_xmit(struct adapter *padapter, struct xmit_frame *pxmitframe)
{
	return pre_xmitframe(padapter, pxmitframe);
}

s32	rtl8188eu_hal_xmitframe_enqueue(struct adapter *padapter, struct xmit_frame *pxmitframe)
{
	struct xmit_priv	*pxmitpriv = &padapter->xmitpriv;
	s32 err;

	if ((err=rtw_xmitframe_enqueue(padapter, pxmitframe)) != _SUCCESS)
	{
		rtw_free_xmitframe(pxmitpriv, pxmitframe);

		/*  Trick, make the statistics correct */
		pxmitpriv->tx_pkts--;
		pxmitpriv->tx_drop++;
	}
	else
	{
		tasklet_hi_schedule(&pxmitpriv->xmit_tasklet);
	}

	return err;

}


#ifdef  CONFIG_HOSTAPD_MLME

static void rtl8188eu_hostap_mgnt_xmit_cb(struct urb *urb)
{
	struct sk_buff *skb = (struct sk_buff *)urb->context;

	/* DBG_8192C("%s\n", __FUNCTION__); */

	rtw_skb_free(skb);
}

s32 rtl8188eu_hostap_mgnt_xmit_entry(struct adapter *padapter, _pkt *pkt)
{
	u16 fc;
	int rc, len, pipe;
	unsigned int bmcst, tid, qsel;
	struct sk_buff *skb, *pxmit_skb;
	struct urb *urb;
	unsigned char *pxmitbuf;
	struct tx_desc *ptxdesc;
	struct rtw_ieee80211_hdr *tx_hdr;
	struct hostapd_priv *phostapdpriv = padapter->phostapdpriv;
	struct net_device *pnetdev = padapter->pnetdev;
	HAL_DATA_TYPE *pHalData = GET_HAL_DATA(padapter);
	struct dvobj_priv *pdvobj = adapter_to_dvobj(padapter);


	/* DBG_8192C("%s\n", __FUNCTION__); */

	skb = pkt;

	len = skb->len;
	tx_hdr = (struct rtw_ieee80211_hdr *)(skb->data);
	fc = le16_to_cpu(tx_hdr->frame_ctl);
	bmcst = IS_MCAST(tx_hdr->addr1);

	if ((fc & RTW_IEEE80211_FCTL_FTYPE) != RTW_IEEE80211_FTYPE_MGMT)
		goto _exit;

	pxmit_skb = rtw_skb_alloc(len + TXDESC_SIZE);

	if (!pxmit_skb)
		goto _exit;

	pxmitbuf = pxmit_skb->data;

	urb = usb_alloc_urb(0, GFP_ATOMIC);
	if (!urb) {
		goto _exit;
	}

	/*  ----- fill tx desc ----- */
	ptxdesc = (struct tx_desc *)pxmitbuf;
	memset(ptxdesc, 0, sizeof(*ptxdesc));

	/* offset 0 */
	ptxdesc->txdw0 |= cpu_to_le32(len&0x0000ffff);
	ptxdesc->txdw0 |= cpu_to_le32(((TXDESC_SIZE+OFFSET_SZ)<<OFFSET_SHT)&0x00ff0000);/* default = 32 bytes for TX Desc */
	ptxdesc->txdw0 |= cpu_to_le32(OWN | FSG | LSG);

	if (bmcst)
	{
		ptxdesc->txdw0 |= cpu_to_le32(BIT(24));
	}

	/* offset 4 */
	ptxdesc->txdw1 |= cpu_to_le32(0x00);/* MAC_ID */

	ptxdesc->txdw1 |= cpu_to_le32((0x12<<QSEL_SHT)&0x00001f00);

	ptxdesc->txdw1 |= cpu_to_le32((0x06<< 16) & 0x000f0000);/* b mode */

	/* offset 8 */

	/* offset 12 */
	ptxdesc->txdw3 |= cpu_to_le32((le16_to_cpu(tx_hdr->seq_ctl)<<16)&0xffff0000);

	/* offset 16 */
	ptxdesc->txdw4 |= cpu_to_le32(BIT(8));/* driver uses rate */

	/* offset 20 */


	/* HW append seq */
	ptxdesc->txdw4 |= cpu_to_le32(BIT(7)); /*  Hw set sequence number */
	ptxdesc->txdw3 |= cpu_to_le32((8 <<28)); /* set bit3 to 1. Suugested by TimChen. 2009.12.29. */


	rtl8188eu_cal_txdesc_chksum(ptxdesc);
	/*  ----- end of fill tx desc ----- */

	/*  */
	skb_put(pxmit_skb, len + TXDESC_SIZE);
	pxmitbuf = pxmitbuf + TXDESC_SIZE;
	memcpy(pxmitbuf, skb->data, len);

	/* DBG_8192C("mgnt_xmit, len=%x\n", pxmit_skb->len); */


	/*  ----- prepare urb for submit ----- */

	/* translate DMA FIFO addr to pipehandle */
	/* pipe = ffaddr2pipehdl(pdvobj, MGT_QUEUE_INX); */
	pipe = usb_sndbulkpipe(pdvobj->pusbdev, pHalData->Queue2EPNum[(u8)MGT_QUEUE_INX]&0x0f);

	usb_fill_bulk_urb(urb, pdvobj->pusbdev, pipe,
			  pxmit_skb->data, pxmit_skb->len, rtl8192cu_hostap_mgnt_xmit_cb, pxmit_skb);

	urb->transfer_flags |= URB_ZERO_PACKET;
	usb_anchor_urb(urb, &phostapdpriv->anchored);
	rc = usb_submit_urb(urb, GFP_ATOMIC);
	if (rc < 0) {
		usb_unanchor_urb(urb);
		kfree_skb(skb);
	}
	usb_free_urb(urb);


_exit:

	rtw_skb_free(skb);
	return 0;

}
#endif
