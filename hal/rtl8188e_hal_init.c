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
#define _HAL_INIT_C_

#include <drv_types.h>
#include <rtw_efuse.h>

#include <rtl8188e_hal.h>

#include <rtw_iol.h>

#include <usb_ops.h>

static void iol_mode_enable(struct adapter *padapter, u8 enable)
{
	u8 reg_0xf0 = 0;

	if (enable) {
		/* Enable initial offload */
		reg_0xf0 = rtw_read8(padapter, REG_SYS_CFG);
		rtw_write8(padapter, REG_SYS_CFG, reg_0xf0|SW_OFFLOAD_EN);

		if (!padapter->bFWReady) {
			DBG_88E("bFWReady == false call reset 8051...\n");
			_8051Reset88E(padapter);
		}

	} else {
		/* disable initial offload */
		reg_0xf0 = rtw_read8(padapter, REG_SYS_CFG);
		rtw_write8(padapter, REG_SYS_CFG, reg_0xf0 & ~SW_OFFLOAD_EN);
	}
}

static s32 iol_execute(struct adapter *padapter, u8 control)
{
	s32 status = _FAIL;
	u8 reg_0x88 = 0;
	u32 start = 0, passing_time = 0;

	control = control&0x0f;
	reg_0x88 = rtw_read8(padapter, REG_HMEBOX_E0);
	rtw_write8(padapter, REG_HMEBOX_E0,  reg_0x88|control);

	start = rtw_get_current_time();
	while ((reg_0x88 = rtw_read8(padapter, REG_HMEBOX_E0)) & control &&
	       (passing_time = rtw_get_passing_time_ms(start)) < 1000) {
		;
	}

	reg_0x88 = rtw_read8(padapter, REG_HMEBOX_E0);
	status = (reg_0x88 & control) ? _FAIL : _SUCCESS;
	if (reg_0x88 & control<<4)
		status = _FAIL;
	return status;
}

static s32 iol_InitLLTTable(struct adapter *padapter, u8 txpktbuf_bndy)
{
	s32 rst = _SUCCESS;
	iol_mode_enable(padapter, 1);
	rtw_write8(padapter, REG_TDECTRL+1, txpktbuf_bndy);
	rst = iol_execute(padapter, CMD_INIT_LLT);
	iol_mode_enable(padapter, 0);
	return rst;
}

static void
efuse_phymap_to_logical(u8 *phymap, u16 _offset, u16 _size_byte, u8  *pbuf)
{
	u8 *efuseTbl = NULL;
	u8 rtemp8;
	u16	eFuse_Addr = 0;
	u8 offset, wren;
	u16	i, j;
	u16	**eFuseWord = NULL;
	u16	efuse_utilized = 0;
	u8 u1temp = 0;

	efuseTbl = (u8 *)rtw_zmalloc(EFUSE_MAP_LEN_88E);
	if (efuseTbl == NULL) {
		DBG_88E("%s: alloc efuseTbl fail!\n", __func__);
		goto exit;
	}

	eFuseWord = (u16 **)rtw_malloc2d(EFUSE_MAX_SECTION_88E, EFUSE_MAX_WORD_UNIT, sizeof(u16));
	if (eFuseWord == NULL) {
		DBG_88E("%s: alloc eFuseWord fail!\n", __func__);
		goto exit;
	}

	/*  0. Refresh efuse init map as all oxFF. */
	for (i = 0; i < EFUSE_MAX_SECTION_88E; i++)
		for (j = 0; j < EFUSE_MAX_WORD_UNIT; j++)
			eFuseWord[i][j] = 0xFFFF;

	/*  */
	/*  1. Read the first byte to check if efuse is empty!!! */
	/*  */
	/*  */
	rtemp8 = *(phymap+eFuse_Addr);
	if (rtemp8 != 0xFF) {
		efuse_utilized++;
		eFuse_Addr++;
	} else {
		DBG_88E("EFUSE is empty efuse_Addr-%d efuse_data =%x\n", eFuse_Addr, rtemp8);
		goto exit;
	}

	/*  */
	/*  2. Read real efuse content. Filter PG header and every section data. */
	/*  */
	while ((rtemp8 != 0xFF) && (eFuse_Addr < EFUSE_REAL_CONTENT_LEN_88E)) {
		/*  Check PG header for section num. */
		if ((rtemp8 & 0x1F) == 0x0F) {		/* extended header */
			u1temp = ((rtemp8 & 0xE0) >> 5);
			rtemp8 = *(phymap+eFuse_Addr);
			if ((rtemp8 & 0x0F) == 0x0F) {
				eFuse_Addr++;
				rtemp8 = *(phymap+eFuse_Addr);

				if (rtemp8 != 0xFF && (eFuse_Addr < EFUSE_REAL_CONTENT_LEN_88E))
					eFuse_Addr++;
				continue;
			} else {
				offset = ((rtemp8 & 0xF0) >> 1) | u1temp;
				wren = (rtemp8 & 0x0F);
				eFuse_Addr++;
			}
		} else {
			offset = ((rtemp8 >> 4) & 0x0f);
			wren = (rtemp8 & 0x0f);
		}

		if (offset < EFUSE_MAX_SECTION_88E) {
			/*  Get word enable value from PG header */
			for (i = 0; i < EFUSE_MAX_WORD_UNIT; i++) {
				/*  Check word enable condition in the section */
				if (!(wren & 0x01)) {
					rtemp8 = *(phymap+eFuse_Addr);
					eFuse_Addr++;
					efuse_utilized++;
					eFuseWord[offset][i] = (rtemp8 & 0xff);
					if (eFuse_Addr >= EFUSE_REAL_CONTENT_LEN_88E)
						break;
					rtemp8 = *(phymap+eFuse_Addr);
					eFuse_Addr++;
					efuse_utilized++;
					eFuseWord[offset][i] |= (((u16)rtemp8 << 8) & 0xff00);

					if (eFuse_Addr >= EFUSE_REAL_CONTENT_LEN_88E)
						break;
				}
				wren >>= 1;
			}
		}
		/*  Read next PG header */
		rtemp8 = *(phymap+eFuse_Addr);

		if (rtemp8 != 0xFF && (eFuse_Addr < EFUSE_REAL_CONTENT_LEN_88E)) {
			efuse_utilized++;
			eFuse_Addr++;
		}
	}

	/*  */
	/*  3. Collect 16 sections and 4 word unit into Efuse map. */
	/*  */
	for (i = 0; i < EFUSE_MAX_SECTION_88E; i++) {
		for (j = 0; j < EFUSE_MAX_WORD_UNIT; j++) {
			efuseTbl[(i*8)+(j*2)] = (eFuseWord[i][j] & 0xff);
			efuseTbl[(i*8)+((j*2)+1)] = ((eFuseWord[i][j] >> 8) & 0xff);
		}
	}

	/*  */
	/*  4. Copy from Efuse map to output pointer memory!!! */
	/*  */
	for (i = 0; i < _size_byte; i++)
		pbuf[i] = efuseTbl[_offset+i];

	/*  */
	/*  5. Calculate Efuse utilization. */
	/*  */

exit:
	if (efuseTbl)
		rtw_mfree(efuseTbl, EFUSE_MAP_LEN_88E);

	if (eFuseWord)
		rtw_mfree2d((void *)eFuseWord, EFUSE_MAX_SECTION_88E, EFUSE_MAX_WORD_UNIT, sizeof(u16));
}

static void efuse_read_phymap_from_txpktbuf(
	struct adapter  *adapter,
	int bcnhead,	/* beacon head, where FW store len(2-byte) and efuse physical map. */
	u8 *content,	/* buffer to store efuse physical map */
	u16 *size	/* for efuse content: the max byte to read. will update to byte read */
	)
{
	u16 dbg_addr = 0;
	u32 start  = 0, passing_time = 0;
	u8 reg_0x143 = 0;
	u32 lo32 = 0, hi32 = 0;
	u16 len = 0, count = 0;
	int i = 0;
	u16 limit = *size;

	u8 *pos = content;

	if (bcnhead < 0) /* if not valid */
		bcnhead = rtw_read8(adapter, REG_TDECTRL+1);

	DBG_88E("%s bcnhead:%d\n", __func__, bcnhead);

	rtw_write8(adapter, REG_PKT_BUFF_ACCESS_CTRL, TXPKT_BUF_SELECT);

	dbg_addr = bcnhead*128/8; /* 8-bytes addressing */

	while (1) {
		rtw_write16(adapter, REG_PKTBUF_DBG_ADDR, dbg_addr+i);

		rtw_write8(adapter, REG_TXPKTBUF_DBG, 0);
		start = rtw_get_current_time();
		while (!(reg_0x143 = rtw_read8(adapter, REG_TXPKTBUF_DBG)) &&
		       (passing_time = rtw_get_passing_time_ms(start)) < 1000) {
			DBG_88E("%s polling reg_0x143:0x%02x, reg_0x106:0x%02x\n", __func__, reg_0x143, rtw_read8(adapter, 0x106));
			rtw_usleep_os(100);
		}

		lo32 = rtw_read32(adapter, REG_PKTBUF_DBG_DATA_L);
		hi32 = rtw_read32(adapter, REG_PKTBUF_DBG_DATA_H);

		if (i == 0) {
			u8 lenc[2];
			u16 lenbak, aaabak;
			u16 aaa;
			lenc[0] = rtw_read8(adapter, REG_PKTBUF_DBG_DATA_L);
			lenc[1] = rtw_read8(adapter, REG_PKTBUF_DBG_DATA_L+1);

			aaabak = le16_to_cpup((__le16 *)lenc);
			lenbak = le16_to_cpu(*((__le16 *)lenc));
			aaa = le16_to_cpup((__le16 *)&lo32);
			len = le16_to_cpu(*((__le16 *)&lo32));

			limit = (len-2 < limit) ? len-2 : limit;

			DBG_88E("%s len:%u, lenbak:%u, aaa:%u, aaabak:%u\n", __func__, len, lenbak, aaa, aaabak);

			_rtw_memcpy(pos, ((u8 *)&lo32)+2, (limit >= count+2) ? 2 : limit-count);
			count += (limit >= count+2) ? 2 : limit-count;
			pos = content+count;

		} else {
			_rtw_memcpy(pos, ((u8 *)&lo32), (limit >= count+4) ? 4 : limit-count);
			count += (limit >= count+4) ? 4 : limit-count;
			pos = content+count;
		}

		if (limit > count && len-2 > count) {
			_rtw_memcpy(pos, (u8 *)&hi32, (limit >= count+4) ? 4 : limit-count);
			count += (limit >= count+4) ? 4 : limit-count;
			pos = content+count;
		}

		if (limit <= count || len-2 <= count)
			break;
		i++;
	}
	rtw_write8(adapter, REG_PKT_BUFF_ACCESS_CTRL, DISABLE_TRXPKT_BUF_ACCESS);
	DBG_88E("%s read count:%u\n", __func__, count);
	*size = count;
}

static s32 iol_read_efuse(struct adapter *padapter, u8 txpktbuf_bndy, u16 offset, u16 size_byte, u8 *logical_map)
{
	s32 status = _FAIL;
	u8 physical_map[512];
	u16 size = 512;

	rtw_write8(padapter, REG_TDECTRL+1, txpktbuf_bndy);
	_rtw_memset(physical_map, 0xFF, 512);
	rtw_write8(padapter, REG_PKT_BUFF_ACCESS_CTRL, TXPKT_BUF_SELECT);
	status = iol_execute(padapter, CMD_READ_EFUSE_MAP);
	if (status == _SUCCESS)
		efuse_read_phymap_from_txpktbuf(padapter, txpktbuf_bndy, physical_map, &size);
	efuse_phymap_to_logical(physical_map, offset, size_byte, logical_map);
	return status;
}

s32 rtl8188e_iol_efuse_patch(struct adapter *padapter)
{
	s32	result = _SUCCESS;

	DBG_88E("==> %s\n", __func__);
	if (rtw_IOL_applied(padapter)) {
		iol_mode_enable(padapter, 1);
		result = iol_execute(padapter, CMD_READ_EFUSE_MAP);
		if (result == _SUCCESS)
			result = iol_execute(padapter, CMD_EFUSE_PATCH);

		iol_mode_enable(padapter, 0);
	}
	return result;
}

static s32 iol_ioconfig(struct adapter *padapter, u8 iocfg_bndy)
{
	s32 rst = _SUCCESS;

	rtw_write8(padapter, REG_TDECTRL+1, iocfg_bndy);
	rst = iol_execute(padapter, CMD_IOCONFIG);
	return rst;
}

static int rtl8188e_IOL_exec_cmds_sync(struct adapter *adapter, struct xmit_frame *xmit_frame, u32 max_wating_ms, u32 bndy_cnt)
{
	u8 i;
	int ret = _FAIL;
	struct pkt_attrib *pattrib = &xmit_frame->attrib;

	if (rtw_IOL_append_END_cmd(xmit_frame) != _SUCCESS)
		goto exit;
	if (rtw_usb_bulk_size_boundary(adapter, TXDESC_SIZE+pattrib->last_txcmdsz)) {
		if (rtw_IOL_append_END_cmd(xmit_frame) != _SUCCESS)
			goto exit;
	}

	dump_mgntframe_and_wait(adapter, xmit_frame, max_wating_ms);

	iol_mode_enable(adapter, 1);
	for (i = 0; i < bndy_cnt; i++) {
		u8 page_no = 0;
		page_no = i*2;
		ret = iol_ioconfig(adapter, page_no);
		if (ret != _SUCCESS)
			break;
	}
	iol_mode_enable(adapter, 0);
exit:
	/* restore BCN_HEAD */
	rtw_write8(adapter, REG_TDECTRL+1, 0);
	return ret;
}

void rtw_IOL_cmd_tx_pkt_buf_dump(struct adapter *Adapter, int data_len)
{
	u32 fifo_data, reg_140;
	u32 addr, rstatus, loop = 0;
	u16 data_cnts = (data_len/8)+1;
	u8 *pbuf = rtw_zvmalloc(data_len+10);
	DBG_88E("###### %s ######\n", __func__);

	rtw_write8(Adapter, REG_PKT_BUFF_ACCESS_CTRL, TXPKT_BUF_SELECT);
	if (pbuf) {
		for (addr = 0; addr < data_cnts; addr++) {
			rtw_write32(Adapter, 0x140, addr);
			rtw_usleep_os(2);
			loop = 0;
			do {
				rstatus = (reg_140 = rtw_read32(Adapter, REG_PKTBUF_DBG_CTRL)&BIT24);
				if (rstatus) {
					fifo_data = rtw_read32(Adapter, REG_PKTBUF_DBG_DATA_L);
					_rtw_memcpy(pbuf+(addr*8), &fifo_data, 4);

					fifo_data = rtw_read32(Adapter, REG_PKTBUF_DBG_DATA_H);
					_rtw_memcpy(pbuf+(addr*8+4), &fifo_data, 4);
				}
				rtw_usleep_os(2);
			} while (!rstatus && (loop++ < 10));
		}
		rtw_IOL_cmd_buf_dump(Adapter, data_len, pbuf);
		rtw_vmfree(pbuf, data_len+10);
	}
	DBG_88E("###### %s ######\n", __func__);
}

static void _FWDownloadEnable(struct adapter *padapter, bool enable)
{
	u8 tmp;

	if (enable) {
		/*  MCU firmware download enable. */
		tmp = rtw_read8(padapter, REG_MCUFWDL);
		rtw_write8(padapter, REG_MCUFWDL, tmp | 0x01);

		/*  8051 reset */
		tmp = rtw_read8(padapter, REG_MCUFWDL+2);
		rtw_write8(padapter, REG_MCUFWDL+2, tmp&0xf7);
	} else {
		/*  MCU firmware download disable. */
		tmp = rtw_read8(padapter, REG_MCUFWDL);
		rtw_write8(padapter, REG_MCUFWDL, tmp&0xfe);

		/*  Reserved for fw extension. */
		rtw_write8(padapter, REG_MCUFWDL+1, 0x00);
	}
}

#define MAX_REG_BOLCK_SIZE	196

static int _BlockWrite(struct adapter *padapter, void *buffer, u32 buffSize)
{
	int ret = _SUCCESS;
	u32	blockSize_p1 = 4;	/*  (Default) Phase #1 : PCI muse use 4-byte write to download FW */
	u32	blockSize_p2 = 8;	/*  Phase #2 : Use 8-byte, if Phase#1 use big size to write FW. */
	u32	blockSize_p3 = 1;	/*  Phase #3 : Use 1-byte, the remnant of FW image. */
	u32	blockCount_p1 = 0, blockCount_p2 = 0, blockCount_p3 = 0;
	u32	remainSize_p1 = 0, remainSize_p2 = 0;
	u8 *bufferPtr	= (u8 *)buffer;
	u32	i = 0, offset = 0;

	blockSize_p1 = MAX_REG_BOLCK_SIZE;

	/* 3 Phase #1 */
	blockCount_p1 = buffSize / blockSize_p1;
	remainSize_p1 = buffSize % blockSize_p1;

	if (blockCount_p1) {
		RT_TRACE(_module_hal_init_c_, _drv_notice_,
			 ("_BlockWrite: [P1] buffSize(%d) blockSize_p1(%d) blockCount_p1(%d) remainSize_p1(%d)\n",
			 buffSize, blockSize_p1, blockCount_p1, remainSize_p1));
	}

	for (i = 0; i < blockCount_p1; i++) {
		ret = rtw_writeN(padapter, (FW_8188E_START_ADDRESS + i * blockSize_p1), blockSize_p1, (bufferPtr + i * blockSize_p1));
		if (ret == _FAIL)
			goto exit;
	}

	/* 3 Phase #2 */
	if (remainSize_p1) {
		offset = blockCount_p1 * blockSize_p1;

		blockCount_p2 = remainSize_p1/blockSize_p2;
		remainSize_p2 = remainSize_p1%blockSize_p2;

		if (blockCount_p2) {
				RT_TRACE(_module_hal_init_c_, _drv_notice_,
					 ("_BlockWrite: [P2] buffSize_p2(%d) blockSize_p2(%d) blockCount_p2(%d) remainSize_p2(%d)\n",
					 (buffSize-offset), blockSize_p2 , blockCount_p2, remainSize_p2));
		}

		for (i = 0; i < blockCount_p2; i++) {
			ret = rtw_writeN(padapter, (FW_8188E_START_ADDRESS + offset + i*blockSize_p2), blockSize_p2, (bufferPtr + offset + i*blockSize_p2));

			if (ret == _FAIL)
				goto exit;
		}
	}

	/* 3 Phase #3 */
	if (remainSize_p2) {
		offset = (blockCount_p1 * blockSize_p1) + (blockCount_p2 * blockSize_p2);

		blockCount_p3 = remainSize_p2 / blockSize_p3;

		RT_TRACE(_module_hal_init_c_, _drv_notice_,
			 ("_BlockWrite: [P3] buffSize_p3(%d) blockSize_p3(%d) blockCount_p3(%d)\n",
			 (buffSize-offset), blockSize_p3, blockCount_p3));

		for (i = 0; i < blockCount_p3; i++) {
			ret = rtw_write8(padapter, (FW_8188E_START_ADDRESS + offset + i), *(bufferPtr + offset + i));

			if (ret == _FAIL)
				goto exit;
		}
	}

exit:
	return ret;
}

static int _PageWrite(struct adapter *padapter, u32 page, void *buffer, u32 size)
{
	u8 value8;
	u8 u8Page = (u8)(page & 0x07);

	value8 = (rtw_read8(padapter, REG_MCUFWDL+2) & 0xF8) | u8Page;
	rtw_write8(padapter, REG_MCUFWDL+2, value8);

	return _BlockWrite(padapter, buffer, size);
}

static void _FillDummy(u8 *pFwBuf, u32 *pFwLen)
{
	u32	FwLen = *pFwLen;
	u8 remain = (u8)(FwLen%4);

	remain = (remain == 0) ? 0 : (4 - remain);

	while (remain > 0) {
		pFwBuf[FwLen] = 0;
		FwLen++;
		remain--;
	}

	*pFwLen = FwLen;
}

static int _WriteFW(struct adapter *padapter, void *buffer, u32 size)
{
	/*  Since we need dynamic decide method of dwonload fw, so we call this function to get chip version. */
	/*  We can remove _ReadChipVersion from ReadpadapterInfo8192C later. */
	int ret = _SUCCESS;
	u32	pageNums, remainSize;
	u32	page, offset;
	u8 *bufferPtr = (u8 *)buffer;

	pageNums = size / MAX_PAGE_SIZE;
	remainSize = size % MAX_PAGE_SIZE;

	for (page = 0; page < pageNums; page++) {
		offset = page * MAX_PAGE_SIZE;
		ret = _PageWrite(padapter, page, bufferPtr+offset, MAX_PAGE_SIZE);

		if (ret == _FAIL)
			goto exit;
	}
	if (remainSize) {
		offset = pageNums * MAX_PAGE_SIZE;
		page = pageNums;
		ret = _PageWrite(padapter, page, bufferPtr+offset, remainSize);

		if (ret == _FAIL)
			goto exit;
	}
	RT_TRACE(_module_hal_init_c_, _drv_info_, ("_WriteFW Done- for Normal chip.\n"));
exit:
	return ret;
}

void _8051Reset88E(struct adapter *padapter)
{
	u8 u1bTmp;

	u1bTmp = rtw_read8(padapter, REG_SYS_FUNC_EN+1);
	rtw_write8(padapter, REG_SYS_FUNC_EN+1, u1bTmp&(~BIT2));
	rtw_write8(padapter, REG_SYS_FUNC_EN+1, u1bTmp|(BIT2));
	DBG_88E("=====> _8051Reset88E(): 8051 reset success .\n");
}

static s32 _FWFreeToGo(struct adapter *padapter)
{
	u32	counter = 0;
	u32	value32;

	/*  polling CheckSum report */
	do {
		value32 = rtw_read32(padapter, REG_MCUFWDL);
		if (value32 & FWDL_ChkSum_rpt)
			break;
	} while (counter++ < POLLING_READY_TIMEOUT_COUNT);

	if (counter >= POLLING_READY_TIMEOUT_COUNT) {
		DBG_88E("%s: chksum report fail! REG_MCUFWDL:0x%08x\n", __func__, value32);
		return _FAIL;
	}
	DBG_88E("%s: Checksum report OK! REG_MCUFWDL:0x%08x\n", __func__, value32);

	value32 = rtw_read32(padapter, REG_MCUFWDL);
	value32 |= MCUFWDL_RDY;
	value32 &= ~WINTINI_RDY;
	rtw_write32(padapter, REG_MCUFWDL, value32);

	_8051Reset88E(padapter);

	/*  polling for FW ready */
	counter = 0;
	do {
		value32 = rtw_read32(padapter, REG_MCUFWDL);
		if (value32 & WINTINI_RDY) {
			DBG_88E("%s: Polling FW ready success!! REG_MCUFWDL:0x%08x\n", __func__, value32);
			return _SUCCESS;
		}
		rtw_udelay_os(5);
	} while (counter++ < POLLING_READY_TIMEOUT_COUNT);

	DBG_88E("%s: Polling FW ready fail!! REG_MCUFWDL:0x%08x\n", __func__, value32);
	return _FAIL;
}

#define IS_FW_81xxC(padapter)	(((GET_HAL_DATA(padapter))->FirmwareSignature & 0xFFF0) == 0x88C0)

#ifdef CONFIG_WOWLAN
/*  */
/*	Description: */
/*		Download 8192C firmware code. */
/*  */
/*  */
s32 rtl8188e_FirmwareDownload(struct adapter *padapter, bool  bUsedWoWLANFw)
#else
s32 rtl8188e_FirmwareDownload(struct adapter *padapter)
#endif
{
	s32	rtStatus = _SUCCESS;
	u8 writeFW_retry = 0;
	u32 fwdl_start_time;
	struct hal_data_8188e *pHalData = GET_HAL_DATA(padapter);

	u8 *FwImage;
	u32			FwImageLen;
#ifdef CONFIG_WOWLAN
	u8 *FwImageWoWLAN;
	u32			FwImageWoWLANLen;
#endif
	struct rt_firmware *pFirmware = NULL;
	struct rt_firmware_hdr *pFwHdr = NULL;
	u8 *pFirmwareBuf;
	u32			FirmwareLen;

	RT_TRACE(_module_hal_init_c_, _drv_info_, ("+%s\n", __func__));
	pFirmware = (struct rt_firmware *)rtw_zmalloc(sizeof(struct rt_firmware));
	if (!pFirmware) {
		rtStatus = _FAIL;
		goto Exit;
	}

	FwImage = (u8 *)Rtl8188E_FwImageArray;
	FwImageLen = Rtl8188E_FWImgArrayLength;

#ifdef CONFIG_WOWLAN
	FwImageWoWLAN = (u8 *)Rtl8188E_FwWoWImageArray;
	FwImageWoWLANLen = Rtl8188E_FwWoWImgArrayLength;
#endif /* CONFIG_WOWLAN */

	pFirmware->eFWSource = FW_SOURCE_HEADER_FILE;

	switch (pFirmware->eFWSource) {
	case FW_SOURCE_IMG_FILE:
		break;
	case FW_SOURCE_HEADER_FILE:
		if (FwImageLen > FW_8188E_SIZE) {
			rtStatus = _FAIL;
			RT_TRACE(_module_hal_init_c_, _drv_err_, ("Firmware size exceed 0x%X. Check it.\n", FW_8188E_SIZE));
			goto Exit;
		}

		pFirmware->szFwBuffer = FwImage;
		pFirmware->ulFwLength = FwImageLen;
#ifdef CONFIG_WOWLAN
		if (bUsedWoWLANFw) {
			pFirmware->szWoWLANFwBuffer = FwImageWoWLAN;
			pFirmware->ulWoWLANFwLength = FwImageWoWLANLen;
		}
#endif /* CONFIG_WOWLAN */
		break;
	}
#ifdef CONFIG_WOWLAN
	if (bUsedWoWLANFw) {
		pFirmwareBuf = pFirmware->szWoWLANFwBuffer;
		FirmwareLen = pFirmware->ulWoWLANFwLength;
		pFwHdr = (struct rt_firmware_hdr *)pFirmware->szWoWLANFwBuffer;
	} else
#endif
	{
	pFirmwareBuf = pFirmware->szFwBuffer;
	FirmwareLen = pFirmware->ulFwLength;
	DBG_88E_LEVEL(_drv_info_, "+%s: !bUsedWoWLANFw, FmrmwareLen:%d+\n", __func__, FirmwareLen);

	/*  To Check Fw header. Added by tynli. 2009.12.04. */
	pFwHdr = (struct rt_firmware_hdr *)pFirmware->szFwBuffer;
	}

	pHalData->FirmwareVersion =  le16_to_cpu(pFwHdr->Version);
	pHalData->FirmwareSubVersion = pFwHdr->Subversion;
	pHalData->FirmwareSignature = le16_to_cpu(pFwHdr->Signature);

	DBG_88E("%s: fw_ver =%d fw_subver =%d sig = 0x%x\n",
		__func__, pHalData->FirmwareVersion, pHalData->FirmwareSubVersion, pHalData->FirmwareSignature);

	if (IS_FW_HEADER_EXIST(pFwHdr)) {
		/*  Shift 32 bytes for FW header */
		pFirmwareBuf = pFirmwareBuf + 32;
		FirmwareLen = FirmwareLen - 32;
	}

	/*  Suggested by Filen. If 8051 is running in RAM code, driver should inform Fw to reset by itself, */
	/*  or it will cause download Fw fail. 2010.02.01. by tynli. */
	if (rtw_read8(padapter, REG_MCUFWDL) & RAM_DL_SEL) { /* 8051 RAM code */
		rtw_write8(padapter, REG_MCUFWDL, 0x00);
		_8051Reset88E(padapter);
	}

	_FWDownloadEnable(padapter, true);
	fwdl_start_time = rtw_get_current_time();
	while (1) {
		/* reset the FWDL chksum */
		rtw_write8(padapter, REG_MCUFWDL, rtw_read8(padapter, REG_MCUFWDL) | FWDL_ChkSum_rpt);

		rtStatus = _WriteFW(padapter, pFirmwareBuf, FirmwareLen);

		if (rtStatus == _SUCCESS ||
		    (rtw_get_passing_time_ms(fwdl_start_time) > 500 && writeFW_retry++ >= 3))
			break;

		DBG_88E("%s writeFW_retry:%u, time after fwdl_start_time:%ums\n",
			__func__, writeFW_retry, rtw_get_passing_time_ms(fwdl_start_time)
		);
	}
	_FWDownloadEnable(padapter, false);
	if (_SUCCESS != rtStatus) {
		DBG_88E("DL Firmware failed!\n");
		goto Exit;
	}

	rtStatus = _FWFreeToGo(padapter);
	if (_SUCCESS != rtStatus) {
		DBG_88E("DL Firmware failed!\n");
		goto Exit;
	}
	RT_TRACE(_module_hal_init_c_, _drv_info_, ("Firmware is ready to run!\n"));

Exit:

	if (pFirmware)
		rtw_mfree((u8 *)pFirmware, sizeof(struct rt_firmware));

#ifdef CONFIG_WOWLAN
	if (padapter->pwrctrlpriv.wowlan_mode)
		rtl8188e_InitializeFirmwareVars(padapter);
	else
		DBG_88E_LEVEL(_drv_always_, "%s: wowland_mode:%d wowlan_wake_reason:%d\n",
			      __func__, padapter->pwrctrlpriv.wowlan_mode,
			      padapter->pwrctrlpriv.wowlan_wake_reason);
#endif

	return rtStatus;
}

#ifdef CONFIG_WOWLAN
void rtl8188e_InitializeFirmwareVars(struct adapter *padapter)
{
	struct hal_data_8188e *pHalData = GET_HAL_DATA(padapter);
	struct pwrctrl_priv *pwrpriv;
	pwrpriv = &padapter->pwrctrlpriv;

	/*  Init Fw LPS related. */
	padapter->pwrctrlpriv.bFwCurrentInPSMode = false;
	/*  Init H2C counter. by tynli. 2009.12.09. */
	pHalData->LastHMEBoxNum = 0;
}

/*  */

/*  */
/*  Description: Prepare some information to Fw for WoWLAN. */
/*					(1) Download wowlan Fw. */
/*					(2) Download RSVD page packets. */
/*					(3) Enable AP offload if needed. */
/*  */
/*  2011.04.12 by tynli. */
/*  */
void
SetFwRelatedForWoWLAN8188ES(
				struct adapter *padapter,
				u8	bHostIsGoingtoSleep
)
{
		int				status = _FAIL;
	/*  */
	/*  1. Before WoWLAN we need to re-download WoWLAN Fw. */
	/*  */
	status = rtl8188e_FirmwareDownload(padapter, bHostIsGoingtoSleep);
	if (status != _SUCCESS) {
		DBG_88E("ConfigFwRelatedForWoWLAN8188ES(): Re-Download Firmware failed!!\n");
		return;
	} else {
		DBG_88E("ConfigFwRelatedForWoWLAN8188ES(): Re-Download Firmware Success !!\n");
	}
	/*  */
	/*  2. Re-Init the variables about Fw related setting. */
	/*  */
	rtl8188e_InitializeFirmwareVars(padapter);
}
#else
void rtl8188e_InitializeFirmwareVars(struct adapter *padapter)
{
	struct hal_data_8188e *pHalData = GET_HAL_DATA(padapter);

	/*  Init Fw LPS related. */
	padapter->pwrctrlpriv.bFwCurrentInPSMode = false;

	/*  Init H2C counter. by tynli. 2009.12.09. */
	pHalData->LastHMEBoxNum = 0;
}
#endif /* CONFIG_WOWLAN */

static void rtl8188e_free_hal_data(struct adapter *padapter)
{
_func_enter_;
	if (padapter->HalData) {
		rtw_mfree(padapter->HalData, sizeof(struct hal_data_8188e));
		padapter->HalData = NULL;
	}
_func_exit_;
}

/*  */
/*			Efuse related code */
/*  */
enum{
		VOLTAGE_V25						= 0x03,
		LDOE25_SHIFT						= 28 ,
	};

static bool
hal_EfusePgPacketWrite2ByteHeader(
		struct adapter *pAdapter,
		u8 efuseType,
		u16				*pAddr,
		struct pgpkt *pTargetPkt,
		bool bPseudoTest);
static bool
hal_EfusePgPacketWrite1ByteHeader(
		struct adapter *pAdapter,
		u8 efuseType,
		u16				*pAddr,
		struct pgpkt *pTargetPkt,
		bool bPseudoTest);
static bool
hal_EfusePgPacketWriteData(
		struct adapter *pAdapter,
		u8 efuseType,
		u16				*pAddr,
		struct pgpkt *pTargetPkt,
		bool bPseudoTest);

static void
hal_EfusePowerSwitch_RTL8188E(
		struct adapter *pAdapter,
		u8 bWrite,
		u8 PwrState)
{
	u8 tempval;
	u16	tmpV16;

	if (PwrState) {
		rtw_write8(pAdapter, REG_EFUSE_ACCESS, EFUSE_ACCESS_ON);

		/*  1.2V Power: From VDDON with Power Cut(0x0000h[15]), defualt valid */
		tmpV16 = rtw_read16(pAdapter, REG_SYS_ISO_CTRL);
		if (!(tmpV16 & PWC_EV12V)) {
			tmpV16 |= PWC_EV12V;
			 rtw_write16(pAdapter, REG_SYS_ISO_CTRL, tmpV16);
		}
		/*  Reset: 0x0000h[28], default valid */
		tmpV16 =  rtw_read16(pAdapter, REG_SYS_FUNC_EN);
		if (!(tmpV16 & FEN_ELDR)) {
			tmpV16 |= FEN_ELDR;
			rtw_write16(pAdapter, REG_SYS_FUNC_EN, tmpV16);
		}

		/*  Clock: Gated(0x0008h[5]) 8M(0x0008h[1]) clock from ANA, default valid */
		tmpV16 = rtw_read16(pAdapter, REG_SYS_CLKR);
		if ((!(tmpV16 & LOADER_CLK_EN))  || (!(tmpV16 & ANA8M))) {
			tmpV16 |= (LOADER_CLK_EN | ANA8M);
			rtw_write16(pAdapter, REG_SYS_CLKR, tmpV16);
		}

		if (bWrite) {
			/*  Enable LDO 2.5V before read/write action */
			tempval = rtw_read8(pAdapter, EFUSE_TEST+3);
			tempval &= 0x0F;
			tempval |= (VOLTAGE_V25 << 4);
			rtw_write8(pAdapter, EFUSE_TEST+3, (tempval | 0x80));
		}
	} else {
		rtw_write8(pAdapter, REG_EFUSE_ACCESS, EFUSE_ACCESS_OFF);

		if (bWrite) {
			/*  Disable LDO 2.5V after read/write action */
			tempval = rtw_read8(pAdapter, EFUSE_TEST+3);
			rtw_write8(pAdapter, EFUSE_TEST+3, (tempval & 0x7F));
		}
	}
}

static void
rtl8188e_EfusePowerSwitch(
		struct adapter *pAdapter,
		u8 bWrite,
		u8 PwrState)
{
	hal_EfusePowerSwitch_RTL8188E(pAdapter, bWrite, PwrState);
}


static bool efuse_read_phymap(
	struct adapter *Adapter,
	u8 *pbuf,	/* buffer to store efuse physical map */
	u16			*size	/* the max byte to read. will update to byte read */
	)
{
	u8 *pos = pbuf;
	u16 limit = *size;
	u16 addr = 0;
	bool reach_end = false;

	/*  */
	/*  Refresh efuse init map as all 0xFF. */
	/*  */
	_rtw_memset(pbuf, 0xFF, limit);

	/*  */
	/*  Read physical efuse content. */
	/*  */
	while (addr < limit) {
		ReadEFuseByte(Adapter, addr, pos, false);
		if (*pos != 0xFF) {
			pos++;
			addr++;
		} else {
			reach_end = true;
			break;
		}
	}
	*size = addr;
	return reach_end;
}

static void
Hal_EfuseReadEFuse88E(
	struct adapter *Adapter,
	u16			_offset,
	u16			_size_byte,
	u8 *pbuf,
		bool bPseudoTest
	)
{
	u8 *efuseTbl = NULL;
	u8 rtemp8[1];
	u16	eFuse_Addr = 0;
	u8 offset, wren;
	u16	i, j;
	u16	**eFuseWord = NULL;
	u16	efuse_utilized = 0;
	u8 u1temp = 0;

	/*  */
	/*  Do NOT excess total size of EFuse table. Added by Roger, 2008.11.10. */
	/*  */
	if ((_offset + _size_byte) > EFUSE_MAP_LEN_88E) {/*  total E-Fuse table is 512bytes */
		DBG_88E("Hal_EfuseReadEFuse88E(): Invalid offset(%#x) with read bytes(%#x)!!\n", _offset, _size_byte);
		goto exit;
	}

	efuseTbl = (u8 *)rtw_zmalloc(EFUSE_MAP_LEN_88E);
	if (efuseTbl == NULL) {
		DBG_88E("%s: alloc efuseTbl fail!\n", __func__);
		goto exit;
	}

	eFuseWord = (u16 **)rtw_malloc2d(EFUSE_MAX_SECTION_88E, EFUSE_MAX_WORD_UNIT, sizeof(u16));
	if (eFuseWord == NULL) {
		DBG_88E("%s: alloc eFuseWord fail!\n", __func__);
		goto exit;
	}

	/*  0. Refresh efuse init map as all oxFF. */
	for (i = 0; i < EFUSE_MAX_SECTION_88E; i++)
		for (j = 0; j < EFUSE_MAX_WORD_UNIT; j++)
			eFuseWord[i][j] = 0xFFFF;

	/*  */
	/*  1. Read the first byte to check if efuse is empty!!! */
	/*  */
	/*  */
	ReadEFuseByte(Adapter, eFuse_Addr, rtemp8, bPseudoTest);
	if (*rtemp8 != 0xFF) {
		efuse_utilized++;
		eFuse_Addr++;
	} else {
		DBG_88E("EFUSE is empty efuse_Addr-%d efuse_data =%x\n", eFuse_Addr, *rtemp8);
		goto exit;
	}

	/*  */
	/*  2. Read real efuse content. Filter PG header and every section data. */
	/*  */
	while ((*rtemp8 != 0xFF) && (eFuse_Addr < EFUSE_REAL_CONTENT_LEN_88E)) {
		/*  Check PG header for section num. */
		if ((*rtemp8 & 0x1F) == 0x0F) {		/* extended header */
			u1temp = ((*rtemp8 & 0xE0) >> 5);

			ReadEFuseByte(Adapter, eFuse_Addr, rtemp8, bPseudoTest);

			if ((*rtemp8 & 0x0F) == 0x0F) {
				eFuse_Addr++;
				ReadEFuseByte(Adapter, eFuse_Addr, rtemp8, bPseudoTest);

				if (*rtemp8 != 0xFF && (eFuse_Addr < EFUSE_REAL_CONTENT_LEN_88E))
					eFuse_Addr++;
				continue;
			} else {
				offset = ((*rtemp8 & 0xF0) >> 1) | u1temp;
				wren = (*rtemp8 & 0x0F);
				eFuse_Addr++;
			}
		} else {
			offset = ((*rtemp8 >> 4) & 0x0f);
			wren = (*rtemp8 & 0x0f);
		}

		if (offset < EFUSE_MAX_SECTION_88E) {
			/*  Get word enable value from PG header */

			for (i = 0; i < EFUSE_MAX_WORD_UNIT; i++) {
				/*  Check word enable condition in the section */
				if (!(wren & 0x01)) {
					ReadEFuseByte(Adapter, eFuse_Addr, rtemp8, bPseudoTest);
					eFuse_Addr++;
					efuse_utilized++;
					eFuseWord[offset][i] = (*rtemp8 & 0xff);
					if (eFuse_Addr >= EFUSE_REAL_CONTENT_LEN_88E)
						break;
					ReadEFuseByte(Adapter, eFuse_Addr, rtemp8, bPseudoTest);
					eFuse_Addr++;
					efuse_utilized++;
					eFuseWord[offset][i] |= (((u16)*rtemp8 << 8) & 0xff00);
					if (eFuse_Addr >= EFUSE_REAL_CONTENT_LEN_88E)
						break;
				}
				wren >>= 1;
			}
		}

		/*  Read next PG header */
		ReadEFuseByte(Adapter, eFuse_Addr, rtemp8, bPseudoTest);

		if (*rtemp8 != 0xFF && (eFuse_Addr < EFUSE_REAL_CONTENT_LEN_88E)) {
			efuse_utilized++;
			eFuse_Addr++;
		}
	}

	/*  3. Collect 16 sections and 4 word unit into Efuse map. */
	for (i = 0; i < EFUSE_MAX_SECTION_88E; i++) {
		for (j = 0; j < EFUSE_MAX_WORD_UNIT; j++) {
			efuseTbl[(i*8)+(j*2)] = (eFuseWord[i][j] & 0xff);
			efuseTbl[(i*8)+((j*2)+1)] = ((eFuseWord[i][j] >> 8) & 0xff);
		}
	}

	/*  4. Copy from Efuse map to output pointer memory!!! */
	for (i = 0; i < _size_byte; i++)
		pbuf[i] = efuseTbl[_offset+i];

	/*  5. Calculate Efuse utilization. */
	rtw_hal_set_hwreg(Adapter, HW_VAR_EFUSE_BYTES, (u8 *)&eFuse_Addr);

exit:
	if (efuseTbl)
		rtw_mfree(efuseTbl, EFUSE_MAP_LEN_88E);

	if (eFuseWord)
		rtw_mfree2d((void *)eFuseWord, EFUSE_MAX_SECTION_88E, EFUSE_MAX_WORD_UNIT, sizeof(u16));
}

static bool Hal_EfuseSwitchToBank(struct adapter *pAdapter, u8 bank, bool bPseudoTest)
{
	bool bRet = false;
	u32 value32 = 0;

	if (bPseudoTest) {
		fakeEfuseBank = bank;
		bRet = true;
	} else {
		if (IS_HARDWARE_TYPE_8723A(pAdapter) &&
		    INCLUDE_MULTI_FUNC_BT(pAdapter)) {
			value32 = rtw_read32(pAdapter, EFUSE_TEST);
			bRet = true;
			switch (bank) {
			case 0:
				value32 = (value32 & ~EFUSE_SEL_MASK) | EFUSE_SEL(EFUSE_WIFI_SEL_0);
				break;
			case 1:
				value32 = (value32 & ~EFUSE_SEL_MASK) | EFUSE_SEL(EFUSE_BT_SEL_0);
				break;
			case 2:
				value32 = (value32 & ~EFUSE_SEL_MASK) | EFUSE_SEL(EFUSE_BT_SEL_1);
				break;
			case 3:
				value32 = (value32 & ~EFUSE_SEL_MASK) | EFUSE_SEL(EFUSE_BT_SEL_2);
				break;
			default:
				value32 = (value32 & ~EFUSE_SEL_MASK) | EFUSE_SEL(EFUSE_WIFI_SEL_0);
				bRet = false;
				break;
			}
			rtw_write32(pAdapter, EFUSE_TEST, value32);
		} else {
			bRet = true;
		}
	}
	return bRet;
}

static void ReadEFuseByIC(struct adapter *Adapter, u8 efuseType, u16 _offset, u16 _size_byte, u8 *pbuf, bool bPseudoTest)
{
	if (!bPseudoTest) {
		int ret = _FAIL;
		if (rtw_IOL_applied(Adapter)) {
			rtw_hal_power_on(Adapter);

			iol_mode_enable(Adapter, 1);
			ret = iol_read_efuse(Adapter, 0, _offset, _size_byte, pbuf);
			iol_mode_enable(Adapter, 0);

			if (_SUCCESS == ret)
				goto exit;
		}
	}
	Hal_EfuseReadEFuse88E(Adapter, _offset, _size_byte, pbuf, bPseudoTest);

exit:
	return;
}

static void ReadEFuse_Pseudo(struct adapter *Adapter, u8 efuseType, u16 _offset, u16 _size_byte, u8 *pbuf, bool bPseudoTest)
{
	Hal_EfuseReadEFuse88E(Adapter, _offset, _size_byte, pbuf, bPseudoTest);
}

static void rtl8188e_ReadEFuse(struct adapter *Adapter, u8 efuseType,
			       u16 _offset, u16 _size_byte, u8 *pbuf,
			       bool bPseudoTest)
{
	if (bPseudoTest)
		ReadEFuse_Pseudo (Adapter, efuseType, _offset, _size_byte, pbuf, bPseudoTest);
	else
		ReadEFuseByIC(Adapter, efuseType, _offset, _size_byte, pbuf, bPseudoTest);
}

/* Do not support BT */
static void Hal_EFUSEGetEfuseDefinition88E(struct adapter *pAdapter, u8 efuseType, u8 type, void *pOut)
{
	switch (type) {
	case TYPE_EFUSE_MAX_SECTION:
		{
			u8 *pMax_section;
			pMax_section = (u8 *)pOut;
			*pMax_section = EFUSE_MAX_SECTION_88E;
		}
		break;
	case TYPE_EFUSE_REAL_CONTENT_LEN:
		{
			u16 *pu2Tmp;
			pu2Tmp = (u16 *)pOut;
			*pu2Tmp = EFUSE_REAL_CONTENT_LEN_88E;
		}
		break;
	case TYPE_EFUSE_CONTENT_LEN_BANK:
		{
			u16 *pu2Tmp;
			pu2Tmp = (u16 *)pOut;
			*pu2Tmp = EFUSE_REAL_CONTENT_LEN_88E;
		}
		break;
	case TYPE_AVAILABLE_EFUSE_BYTES_BANK:
		{
			u16 *pu2Tmp;
			pu2Tmp = (u16 *)pOut;
			*pu2Tmp = (u16)(EFUSE_REAL_CONTENT_LEN_88E-EFUSE_OOB_PROTECT_BYTES_88E);
		}
		break;
	case TYPE_AVAILABLE_EFUSE_BYTES_TOTAL:
		{
			u16 *pu2Tmp;
			pu2Tmp = (u16 *)pOut;
			*pu2Tmp = (u16)(EFUSE_REAL_CONTENT_LEN_88E-EFUSE_OOB_PROTECT_BYTES_88E);
		}
		break;
	case TYPE_EFUSE_MAP_LEN:
		{
			u16 *pu2Tmp;
			pu2Tmp = (u16 *)pOut;
			*pu2Tmp = (u16)EFUSE_MAP_LEN_88E;
		}
		break;
	case TYPE_EFUSE_PROTECT_BYTES_BANK:
		{
			u8 *pu1Tmp;
			pu1Tmp = (u8 *)pOut;
			*pu1Tmp = (u8)(EFUSE_OOB_PROTECT_BYTES_88E);
		}
		break;
	default:
		{
			u8 *pu1Tmp;
			pu1Tmp = (u8 *)pOut;
			*pu1Tmp = 0;
		}
		break;
	}
}

static void Hal_EFUSEGetEfuseDefinition_Pseudo88E(struct adapter *pAdapter, u8 efuseType, u8 type, void *pOut)
{
	switch (type) {
	case TYPE_EFUSE_MAX_SECTION:
		{
			u8 *pMax_section;
			pMax_section = (u8 *)pOut;
			*pMax_section = EFUSE_MAX_SECTION_88E;
		}
		break;
	case TYPE_EFUSE_REAL_CONTENT_LEN:
		{
			u16 *pu2Tmp;
			pu2Tmp = (u16 *)pOut;
			*pu2Tmp = EFUSE_REAL_CONTENT_LEN_88E;
		}
		break;
	case TYPE_EFUSE_CONTENT_LEN_BANK:
		{
			u16 *pu2Tmp;
			pu2Tmp = (u16 *)pOut;
			*pu2Tmp = EFUSE_REAL_CONTENT_LEN_88E;
		}
		break;
	case TYPE_AVAILABLE_EFUSE_BYTES_BANK:
		{
			u16 *pu2Tmp;
			pu2Tmp = (u16 *)pOut;
			*pu2Tmp = (u16)(EFUSE_REAL_CONTENT_LEN_88E-EFUSE_OOB_PROTECT_BYTES_88E);
		}
		break;
	case TYPE_AVAILABLE_EFUSE_BYTES_TOTAL:
		{
			u16 *pu2Tmp;
			pu2Tmp = (u16 *)pOut;
			*pu2Tmp = (u16)(EFUSE_REAL_CONTENT_LEN_88E-EFUSE_OOB_PROTECT_BYTES_88E);
		}
		break;
	case TYPE_EFUSE_MAP_LEN:
		{
			u16 *pu2Tmp;
			pu2Tmp = (u16 *)pOut;
			*pu2Tmp = (u16)EFUSE_MAP_LEN_88E;
		}
		break;
	case TYPE_EFUSE_PROTECT_BYTES_BANK:
		{
			u8 *pu1Tmp;
			pu1Tmp = (u8 *)pOut;
			*pu1Tmp = (u8)(EFUSE_OOB_PROTECT_BYTES_88E);
		}
		break;
	default:
		{
			u8 *pu1Tmp;
			pu1Tmp = (u8 *)pOut;
			*pu1Tmp = 0;
		}
		break;
	}
}

static void rtl8188e_EFUSE_GetEfuseDefinition(struct adapter *pAdapter, u8 efuseType, u8 type, void *pOut, bool bPseudoTest)
{
	if (bPseudoTest)
		Hal_EFUSEGetEfuseDefinition_Pseudo88E(pAdapter, efuseType, type, pOut);
	else
		Hal_EFUSEGetEfuseDefinition88E(pAdapter, efuseType, type, pOut);
}

static u8 Hal_EfuseWordEnableDataWrite(struct adapter *pAdapter, u16 efuse_addr, u8 word_en, u8 *data, bool bPseudoTest)
{
	u16	tmpaddr = 0;
	u16	start_addr = efuse_addr;
	u8 badworden = 0x0F;
	u8 tmpdata[8];

	_rtw_memset((void *)tmpdata, 0xff, PGPKT_DATA_SIZE);

	if (!(word_en&BIT0)) {
		tmpaddr = start_addr;
		efuse_OneByteWrite(pAdapter, start_addr++, data[0], bPseudoTest);
		efuse_OneByteWrite(pAdapter, start_addr++, data[1], bPseudoTest);

		efuse_OneByteRead(pAdapter, tmpaddr, &tmpdata[0], bPseudoTest);
		efuse_OneByteRead(pAdapter, tmpaddr+1, &tmpdata[1], bPseudoTest);
		if ((data[0] != tmpdata[0]) || (data[1] != tmpdata[1]))
			badworden &= (~BIT0);
	}
	if (!(word_en&BIT1)) {
		tmpaddr = start_addr;
		efuse_OneByteWrite(pAdapter, start_addr++, data[2], bPseudoTest);
		efuse_OneByteWrite(pAdapter, start_addr++, data[3], bPseudoTest);

		efuse_OneByteRead(pAdapter, tmpaddr    , &tmpdata[2], bPseudoTest);
		efuse_OneByteRead(pAdapter, tmpaddr+1, &tmpdata[3], bPseudoTest);
		if ((data[2] != tmpdata[2]) || (data[3] != tmpdata[3]))
			badworden &= (~BIT1);
	}
	if (!(word_en&BIT2)) {
		tmpaddr = start_addr;
		efuse_OneByteWrite(pAdapter, start_addr++, data[4], bPseudoTest);
		efuse_OneByteWrite(pAdapter, start_addr++, data[5], bPseudoTest);

		efuse_OneByteRead(pAdapter, tmpaddr, &tmpdata[4], bPseudoTest);
		efuse_OneByteRead(pAdapter, tmpaddr+1, &tmpdata[5], bPseudoTest);
		if ((data[4] != tmpdata[4]) || (data[5] != tmpdata[5]))
			badworden &= (~BIT2);
	}
	if (!(word_en&BIT3)) {
		tmpaddr = start_addr;
		efuse_OneByteWrite(pAdapter, start_addr++, data[6], bPseudoTest);
		efuse_OneByteWrite(pAdapter, start_addr++, data[7], bPseudoTest);

		efuse_OneByteRead(pAdapter, tmpaddr, &tmpdata[6], bPseudoTest);
		efuse_OneByteRead(pAdapter, tmpaddr+1, &tmpdata[7], bPseudoTest);
		if ((data[6] != tmpdata[6]) || (data[7] != tmpdata[7]))
			badworden &= (~BIT3);
	}
	return badworden;
}

static u8 Hal_EfuseWordEnableDataWrite_Pseudo(struct adapter *pAdapter, u16 efuse_addr, u8 word_en, u8 *data, bool bPseudoTest)
{
	u8 ret;

	ret = Hal_EfuseWordEnableDataWrite(pAdapter, efuse_addr, word_en, data, bPseudoTest);
	return ret;
}

static u8 rtl8188e_Efuse_WordEnableDataWrite(struct adapter *pAdapter, u16 efuse_addr, u8 word_en, u8 *data, bool bPseudoTest)
{
	u8 ret = 0;

	if (bPseudoTest)
		ret = Hal_EfuseWordEnableDataWrite_Pseudo (pAdapter, efuse_addr, word_en, data, bPseudoTest);
	else
		ret = Hal_EfuseWordEnableDataWrite(pAdapter, efuse_addr, word_en, data, bPseudoTest);
	return ret;
}

static u16 hal_EfuseGetCurrentSize_8188e(struct adapter *pAdapter, bool bPseudoTest)
{
	int	bContinual = true;
	u16	efuse_addr = 0;
	u8 hoffset = 0, hworden = 0;
	u8 efuse_data, word_cnts = 0;

	if (bPseudoTest)
		efuse_addr = (u16)(fakeEfuseUsedBytes);
	else
		rtw_hal_get_hwreg(pAdapter, HW_VAR_EFUSE_BYTES, (u8 *)&efuse_addr);

	while (bContinual &&
	       efuse_OneByteRead(pAdapter, efuse_addr, &efuse_data, bPseudoTest) &&
	       AVAILABLE_EFUSE_ADDR(efuse_addr)) {
		if (efuse_data != 0xFF) {
			if ((efuse_data&0x1F) == 0x0F) {		/* extended header */
				hoffset = efuse_data;
				efuse_addr++;
				efuse_OneByteRead(pAdapter, efuse_addr, &efuse_data, bPseudoTest);
				if ((efuse_data & 0x0F) == 0x0F) {
					efuse_addr++;
					continue;
				} else {
					hoffset = ((hoffset & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
					hworden = efuse_data & 0x0F;
				}
			} else {
				hoffset = (efuse_data>>4) & 0x0F;
				hworden =  efuse_data & 0x0F;
			}
			word_cnts = Efuse_CalculateWordCnts(hworden);
			/* read next header */
			efuse_addr = efuse_addr + (word_cnts*2)+1;
		} else {
			bContinual = false;
		}
	}

	if (bPseudoTest)
		fakeEfuseUsedBytes = efuse_addr;
	else
		rtw_hal_set_hwreg(pAdapter, HW_VAR_EFUSE_BYTES, (u8 *)&efuse_addr);

	return efuse_addr;
}

static u16 Hal_EfuseGetCurrentSize_Pseudo(struct adapter *pAdapter, bool bPseudoTest)
{
	u16	ret = 0;

	ret = hal_EfuseGetCurrentSize_8188e(pAdapter, bPseudoTest);
	return ret;
}

static u16 rtl8188e_EfuseGetCurrentSize(struct adapter *pAdapter, u8 efuseType, bool bPseudoTest)
{
	u16	ret = 0;

	if (bPseudoTest)
		ret = Hal_EfuseGetCurrentSize_Pseudo(pAdapter, bPseudoTest);
	else
		ret = hal_EfuseGetCurrentSize_8188e(pAdapter, bPseudoTest);
	return ret;
}

static int hal_EfusePgPacketRead_8188e(struct adapter *pAdapter, u8 offset, u8 *data, bool bPseudoTest)
{
	u8 ReadState = PG_STATE_HEADER;
	int	bContinual = true;
	int	bDataEmpty = true;
	u8 efuse_data, word_cnts = 0;
	u16	efuse_addr = 0;
	u8 hoffset = 0, hworden = 0;
	u8 tmpidx = 0;
	u8 tmpdata[8];
	u8 max_section = 0;
	u8 tmp_header = 0;

	EFUSE_GetEfuseDefinition(pAdapter, EFUSE_WIFI, TYPE_EFUSE_MAX_SECTION, (void *)&max_section, bPseudoTest);

	if (data == NULL)
		return false;
	if (offset > max_section)
		return false;

	_rtw_memset((void *)data, 0xff, sizeof(u8)*PGPKT_DATA_SIZE);
	_rtw_memset((void *)tmpdata, 0xff, sizeof(u8)*PGPKT_DATA_SIZE);

	/*  <Roger_TODO> Efuse has been pre-programmed dummy 5Bytes at the end of Efuse by CP. */
	/*  Skip dummy parts to prevent unexpected data read from Efuse. */
	/*  By pass right now. 2009.02.19. */
	while (bContinual && AVAILABLE_EFUSE_ADDR(efuse_addr)) {
		/*   Header Read ------------- */
		if (ReadState & PG_STATE_HEADER) {
			if (efuse_OneByteRead(pAdapter, efuse_addr, &efuse_data, bPseudoTest) && (efuse_data != 0xFF)) {
				if (EXT_HEADER(efuse_data)) {
					tmp_header = efuse_data;
					efuse_addr++;
					efuse_OneByteRead(pAdapter, efuse_addr, &efuse_data, bPseudoTest);
					if (!ALL_WORDS_DISABLED(efuse_data)) {
						hoffset = ((tmp_header & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
						hworden = efuse_data & 0x0F;
					} else {
						DBG_88E("Error, All words disabled\n");
						efuse_addr++;
						continue;
					}
				} else {
					hoffset = (efuse_data>>4) & 0x0F;
					hworden =  efuse_data & 0x0F;
				}
				word_cnts = Efuse_CalculateWordCnts(hworden);
				bDataEmpty = true;

				if (hoffset == offset) {
					for (tmpidx = 0; tmpidx < word_cnts*2; tmpidx++) {
						if (efuse_OneByteRead(pAdapter, efuse_addr+1+tmpidx, &efuse_data, bPseudoTest)) {
							tmpdata[tmpidx] = efuse_data;
							if (efuse_data != 0xff)
								bDataEmpty = false;
						}
					}
					if (bDataEmpty == false) {
						ReadState = PG_STATE_DATA;
					} else {/* read next header */
						efuse_addr = efuse_addr + (word_cnts*2)+1;
						ReadState = PG_STATE_HEADER;
					}
				} else {/* read next header */
					efuse_addr = efuse_addr + (word_cnts*2)+1;
					ReadState = PG_STATE_HEADER;
				}
			} else {
				bContinual = false;
			}
		} else if (ReadState & PG_STATE_DATA) {
		/*   Data section Read ------------- */
			efuse_WordEnableDataRead(hworden, tmpdata, data);
			efuse_addr = efuse_addr + (word_cnts*2)+1;
			ReadState = PG_STATE_HEADER;
		}

	}

	if ((data[0] == 0xff) && (data[1] == 0xff) && (data[2] == 0xff)  && (data[3] == 0xff) &&
	    (data[4] == 0xff) && (data[5] == 0xff) && (data[6] == 0xff)  && (data[7] == 0xff))
		return false;
	else
		return true;
}

static int Hal_EfusePgPacketRead(struct adapter *pAdapter, u8 offset, u8 *data, bool bPseudoTest)
{
	int	ret;

	ret = hal_EfusePgPacketRead_8188e(pAdapter, offset, data, bPseudoTest);
	return ret;
}

static int Hal_EfusePgPacketRead_Pseudo(struct adapter *pAdapter, u8 offset, u8 *data, bool bPseudoTest)
{
	int	ret;

	ret = hal_EfusePgPacketRead_8188e(pAdapter, offset, data, bPseudoTest);
	return ret;
}

static int rtl8188e_Efuse_PgPacketRead(struct adapter *pAdapter, u8 offset, u8 *data, bool bPseudoTest)
{
	int	ret;

	if (bPseudoTest)
		ret = Hal_EfusePgPacketRead_Pseudo (pAdapter, offset, data, bPseudoTest);
	else
		ret = Hal_EfusePgPacketRead(pAdapter, offset, data, bPseudoTest);
	return ret;
}

static bool hal_EfuseFixHeaderProcess(struct adapter *pAdapter, u8 efuseType, struct pgpkt *pFixPkt, u16 *pAddr, bool bPseudoTest)
{
	u8 originaldata[8], badworden = 0;
	u16	efuse_addr = *pAddr;
	u32	PgWriteSuccess = 0;

	_rtw_memset((void *)originaldata, 0xff, 8);

	if (Efuse_PgPacketRead(pAdapter, pFixPkt->offset, originaldata, bPseudoTest)) {
		/* check if data exist */
		badworden = Efuse_WordEnableDataWrite(pAdapter, efuse_addr+1, pFixPkt->word_en, originaldata, bPseudoTest);

		if (badworden != 0xf) {	/*  write fail */
			PgWriteSuccess = Efuse_PgPacketWrite(pAdapter, pFixPkt->offset, badworden, originaldata, bPseudoTest);

			if (!PgWriteSuccess)
				return false;
			else
				efuse_addr = Efuse_GetCurrentSize(pAdapter, efuseType, bPseudoTest);
		} else {
			efuse_addr = efuse_addr + (pFixPkt->word_cnts*2) + 1;
		}
	} else {
		efuse_addr = efuse_addr + (pFixPkt->word_cnts*2) + 1;
	}
	*pAddr = efuse_addr;
	return true;
}

static bool hal_EfusePgPacketWrite2ByteHeader(struct adapter *pAdapter, u8 efuseType, u16 *pAddr, struct pgpkt *pTargetPkt, bool bPseudoTest)
{
	bool bRet = false;
	u16	efuse_addr = *pAddr, efuse_max_available_len = 0;
	u8 pg_header = 0, tmp_header = 0, pg_header_temp = 0;
	u8 repeatcnt = 0;

	EFUSE_GetEfuseDefinition(pAdapter, efuseType, TYPE_AVAILABLE_EFUSE_BYTES_BANK, (void *)&efuse_max_available_len, bPseudoTest);

	while (efuse_addr < efuse_max_available_len) {
		pg_header = ((pTargetPkt->offset & 0x07) << 5) | 0x0F;
		efuse_OneByteWrite(pAdapter, efuse_addr, pg_header, bPseudoTest);
		efuse_OneByteRead(pAdapter, efuse_addr, &tmp_header, bPseudoTest);

		while (tmp_header == 0xFF) {
			if (repeatcnt++ > EFUSE_REPEAT_THRESHOLD_)
				return false;

			efuse_OneByteWrite(pAdapter, efuse_addr, pg_header, bPseudoTest);
			efuse_OneByteRead(pAdapter, efuse_addr, &tmp_header, bPseudoTest);
		}

		/* to write ext_header */
		if (tmp_header == pg_header) {
			efuse_addr++;
			pg_header_temp = pg_header;
			pg_header = ((pTargetPkt->offset & 0x78) << 1) | pTargetPkt->word_en;

			efuse_OneByteWrite(pAdapter, efuse_addr, pg_header, bPseudoTest);
			efuse_OneByteRead(pAdapter, efuse_addr, &tmp_header, bPseudoTest);

			while (tmp_header == 0xFF) {
				if (repeatcnt++ > EFUSE_REPEAT_THRESHOLD_)
					return false;

				efuse_OneByteWrite(pAdapter, efuse_addr, pg_header, bPseudoTest);
				efuse_OneByteRead(pAdapter, efuse_addr, &tmp_header, bPseudoTest);
			}

			if ((tmp_header & 0x0F) == 0x0F) {	/* word_en PG fail */
				if (repeatcnt++ > EFUSE_REPEAT_THRESHOLD_) {
					return false;
				} else {
					efuse_addr++;
					continue;
				}
			} else if (pg_header != tmp_header) {	/* offset PG fail */
				struct pgpkt	fixPkt;
				fixPkt.offset = ((pg_header_temp & 0xE0) >> 5) | ((tmp_header & 0xF0) >> 1);
				fixPkt.word_en = tmp_header & 0x0F;
				fixPkt.word_cnts = Efuse_CalculateWordCnts(fixPkt.word_en);
				if (!hal_EfuseFixHeaderProcess(pAdapter, efuseType, &fixPkt, &efuse_addr, bPseudoTest))
					return false;
			} else {
				bRet = true;
				break;
			}
		} else if ((tmp_header & 0x1F) == 0x0F) {		/* wrong extended header */
			efuse_addr += 2;
			continue;
		}
	}

	*pAddr = efuse_addr;
	return bRet;
}

static bool hal_EfusePgPacketWrite1ByteHeader(struct adapter *pAdapter, u8 efuseType, u16 *pAddr, struct pgpkt *pTargetPkt, bool bPseudoTest)
{
	bool bRet = false;
	u8 pg_header = 0, tmp_header = 0;
	u16	efuse_addr = *pAddr;
	u8 repeatcnt = 0;

	pg_header = ((pTargetPkt->offset << 4) & 0xf0) | pTargetPkt->word_en;

	efuse_OneByteWrite(pAdapter, efuse_addr, pg_header, bPseudoTest);
	efuse_OneByteRead(pAdapter, efuse_addr, &tmp_header, bPseudoTest);

	while (tmp_header == 0xFF) {
		if (repeatcnt++ > EFUSE_REPEAT_THRESHOLD_)
			return false;
		efuse_OneByteWrite(pAdapter, efuse_addr, pg_header, bPseudoTest);
		efuse_OneByteRead(pAdapter, efuse_addr, &tmp_header, bPseudoTest);
	}

	if (pg_header == tmp_header) {
		bRet = true;
	} else {
		struct pgpkt	fixPkt;
		fixPkt.offset = (tmp_header>>4) & 0x0F;
		fixPkt.word_en = tmp_header & 0x0F;
		fixPkt.word_cnts = Efuse_CalculateWordCnts(fixPkt.word_en);
		if (!hal_EfuseFixHeaderProcess(pAdapter, efuseType, &fixPkt, &efuse_addr, bPseudoTest))
			return false;
	}

	*pAddr = efuse_addr;
	return bRet;
}

static bool hal_EfusePgPacketWriteData(struct adapter *pAdapter, u8 efuseType, u16 *pAddr, struct pgpkt *pTargetPkt, bool bPseudoTest)
{
	u16	efuse_addr = *pAddr;
	u8 badworden = 0;
	u32	PgWriteSuccess = 0;

	badworden = 0x0f;
	badworden = Efuse_WordEnableDataWrite(pAdapter, efuse_addr+1, pTargetPkt->word_en, pTargetPkt->data, bPseudoTest);
	if (badworden == 0x0F) {
		/*  write ok */
		return true;
	} else {
		/* reorganize other pg packet */
		PgWriteSuccess = Efuse_PgPacketWrite(pAdapter, pTargetPkt->offset, badworden, pTargetPkt->data, bPseudoTest);
		if (!PgWriteSuccess)
			return false;
		else
			return true;
	}
}

static bool
hal_EfusePgPacketWriteHeader(
				struct adapter *pAdapter,
				u8 efuseType,
				u16				*pAddr,
				struct pgpkt *pTargetPkt,
				bool bPseudoTest)
{
	bool bRet = false;

	if (pTargetPkt->offset >= EFUSE_MAX_SECTION_BASE)
		bRet = hal_EfusePgPacketWrite2ByteHeader(pAdapter, efuseType, pAddr, pTargetPkt, bPseudoTest);
	else
		bRet = hal_EfusePgPacketWrite1ByteHeader(pAdapter, efuseType, pAddr, pTargetPkt, bPseudoTest);

	return bRet;
}

static bool
wordEnMatched(
		struct pgpkt *pTargetPkt,
		struct pgpkt *pCurPkt,
		u8 *pWden
)
{
	u8 match_word_en = 0x0F;	/*  default all words are disabled */

	/*  check if the same words are enabled both target and current PG packet */
	if (((pTargetPkt->word_en & BIT0) == 0) &&
	    ((pCurPkt->word_en & BIT0) == 0))
		match_word_en &= ~BIT0;				/*  enable word 0 */
	if (((pTargetPkt->word_en & BIT1) == 0) &&
	    ((pCurPkt->word_en & BIT1) == 0))
		match_word_en &= ~BIT1;				/*  enable word 1 */
	if (((pTargetPkt->word_en & BIT2) == 0) &&
	    ((pCurPkt->word_en & BIT2) == 0))
		match_word_en &= ~BIT2;				/*  enable word 2 */
	if (((pTargetPkt->word_en & BIT3) == 0) &&
	    ((pCurPkt->word_en & BIT3) == 0))
		match_word_en &= ~BIT3;				/*  enable word 3 */

	*pWden = match_word_en;

	if (match_word_en != 0xf)
		return true;
	else
		return false;
}

static bool hal_EfuseCheckIfDatafollowed(struct adapter *pAdapter, u8 word_cnts, u16 startAddr, bool bPseudoTest)
{
	bool bRet = false;
	u8 i, efuse_data;

	for (i = 0; i < (word_cnts*2); i++) {
		if (efuse_OneByteRead(pAdapter, (startAddr+i), &efuse_data, bPseudoTest) && (efuse_data != 0xFF))
			bRet = true;
	}
	return bRet;
}

static bool hal_EfusePartialWriteCheck(struct adapter *pAdapter, u8 efuseType, u16 *pAddr, struct pgpkt *pTargetPkt, bool bPseudoTest)
{
	bool bRet = false;
	u8 i, efuse_data = 0, cur_header = 0;
	u8 matched_wden = 0, badworden = 0;
	u16	startAddr = 0, efuse_max_available_len = 0, efuse_max = 0;
	struct pgpkt curPkt;

	EFUSE_GetEfuseDefinition(pAdapter, efuseType, TYPE_AVAILABLE_EFUSE_BYTES_BANK, (void *)&efuse_max_available_len, bPseudoTest);
	EFUSE_GetEfuseDefinition(pAdapter, efuseType, TYPE_EFUSE_REAL_CONTENT_LEN, (void *)&efuse_max, bPseudoTest);

	if (efuseType == EFUSE_WIFI) {
		if (bPseudoTest) {
			startAddr = (u16)(fakeEfuseUsedBytes%EFUSE_REAL_CONTENT_LEN);
		} else {
			rtw_hal_get_hwreg(pAdapter, HW_VAR_EFUSE_BYTES, (u8 *)&startAddr);
			startAddr %= EFUSE_REAL_CONTENT_LEN;
		}
	} else {
		if (bPseudoTest)
			startAddr = (u16)(fakeBTEfuseUsedBytes%EFUSE_REAL_CONTENT_LEN);
		else
			startAddr = (u16)(BTEfuseUsedBytes%EFUSE_REAL_CONTENT_LEN);
	}

	while (1) {
		if (startAddr >= efuse_max_available_len) {
			bRet = false;
			break;
		}

		if (efuse_OneByteRead(pAdapter, startAddr, &efuse_data, bPseudoTest) && (efuse_data != 0xFF)) {
			if (EXT_HEADER(efuse_data)) {
				cur_header = efuse_data;
				startAddr++;
				efuse_OneByteRead(pAdapter, startAddr, &efuse_data, bPseudoTest);
				if (ALL_WORDS_DISABLED(efuse_data)) {
					bRet = false;
					break;
				} else {
					curPkt.offset = ((cur_header & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1);
					curPkt.word_en = efuse_data & 0x0F;
				}
			} else {
				cur_header  =  efuse_data;
				curPkt.offset = (cur_header>>4) & 0x0F;
				curPkt.word_en = cur_header & 0x0F;
			}

			curPkt.word_cnts = Efuse_CalculateWordCnts(curPkt.word_en);
			/*  if same header is found but no data followed */
			/*  write some part of data followed by the header. */
			if ((curPkt.offset == pTargetPkt->offset) &&
			    (!hal_EfuseCheckIfDatafollowed(pAdapter, curPkt.word_cnts, startAddr+1, bPseudoTest)) &&
			    wordEnMatched(pTargetPkt, &curPkt, &matched_wden)) {
				/*  Here to write partial data */
				badworden = Efuse_WordEnableDataWrite(pAdapter, startAddr+1, matched_wden, pTargetPkt->data, bPseudoTest);
				if (badworden != 0x0F) {
					u32	PgWriteSuccess = 0;
					/*  if write fail on some words, write these bad words again */

					PgWriteSuccess = Efuse_PgPacketWrite(pAdapter, pTargetPkt->offset, badworden, pTargetPkt->data, bPseudoTest);

					if (!PgWriteSuccess) {
						bRet = false;	/*  write fail, return */
						break;
					}
				}
				/*  partial write ok, update the target packet for later use */
				for (i = 0; i < 4; i++) {
					if ((matched_wden & (0x1<<i)) == 0)	/*  this word has been written */
						pTargetPkt->word_en |= (0x1<<i);	/*  disable the word */
				}
				pTargetPkt->word_cnts = Efuse_CalculateWordCnts(pTargetPkt->word_en);
			}
			/*  read from next header */
			startAddr = startAddr + (curPkt.word_cnts*2) + 1;
		} else {
			/*  not used header, 0xff */
			*pAddr = startAddr;
			bRet = true;
			break;
		}
	}
	return bRet;
}

static bool
hal_EfusePgCheckAvailableAddr(
		struct adapter *pAdapter,
		u8 efuseType,
		bool bPseudoTest
	)
{
	u16	efuse_max_available_len = 0;

	/* Change to check TYPE_EFUSE_MAP_LEN , beacuse 8188E raw 256, logic map over 256. */
	EFUSE_GetEfuseDefinition(pAdapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&efuse_max_available_len, false);

	if (Efuse_GetCurrentSize(pAdapter, efuseType, bPseudoTest) >= efuse_max_available_len)
		return false;
	return true;
}

static void hal_EfuseConstructPGPkt(u8 offset, u8 word_en, u8 *pData, struct pgpkt *pTargetPkt)
{
	_rtw_memset((void *)pTargetPkt->data, 0xFF, sizeof(u8)*8);
	pTargetPkt->offset = offset;
	pTargetPkt->word_en = word_en;
	efuse_WordEnableDataRead(word_en, pData, pTargetPkt->data);
	pTargetPkt->word_cnts = Efuse_CalculateWordCnts(pTargetPkt->word_en);
}

static bool hal_EfusePgPacketWrite_BT(struct adapter *pAdapter, u8 offset, u8 word_en, u8 *pData, bool bPseudoTest)
{
	struct pgpkt	targetPkt;
	u16	startAddr = 0;
	u8 efuseType = EFUSE_BT;

	if (!hal_EfusePgCheckAvailableAddr(pAdapter, efuseType, bPseudoTest))
		return false;

	hal_EfuseConstructPGPkt(offset, word_en, pData, &targetPkt);

	if (!hal_EfusePartialWriteCheck(pAdapter, efuseType, &startAddr, &targetPkt, bPseudoTest))
		return false;

	if (!hal_EfusePgPacketWriteHeader(pAdapter, efuseType, &startAddr, &targetPkt, bPseudoTest))
		return false;

	if (!hal_EfusePgPacketWriteData(pAdapter, efuseType, &startAddr, &targetPkt, bPseudoTest))
		return false;

	return true;
}

static bool hal_EfusePgPacketWrite_8188e(struct adapter *pAdapter, u8 offset, u8 word_en, u8 *pData, bool bPseudoTest)
{
	struct pgpkt	targetPkt;
	u16			startAddr = 0;
	u8 efuseType = EFUSE_WIFI;

	if (!hal_EfusePgCheckAvailableAddr(pAdapter, efuseType, bPseudoTest))
		return false;

	hal_EfuseConstructPGPkt(offset, word_en, pData, &targetPkt);

	if (!hal_EfusePartialWriteCheck(pAdapter, efuseType, &startAddr, &targetPkt, bPseudoTest))
		return false;

	if (!hal_EfusePgPacketWriteHeader(pAdapter, efuseType, &startAddr, &targetPkt, bPseudoTest))
		return false;

	if (!hal_EfusePgPacketWriteData(pAdapter, efuseType, &startAddr, &targetPkt, bPseudoTest))
		return false;

	return true;
}

static int Hal_EfusePgPacketWrite_Pseudo(struct adapter *pAdapter, u8 offset, u8 word_en, u8 *data, bool bPseudoTest)
{
	int ret;

	ret = hal_EfusePgPacketWrite_8188e(pAdapter, offset, word_en, data, bPseudoTest);
	return ret;
}

static int Hal_EfusePgPacketWrite(struct adapter *pAdapter, u8 offset, u8 word_en, u8 *data, bool bPseudoTest)
{
	int	ret = 0;
	ret = hal_EfusePgPacketWrite_8188e(pAdapter, offset, word_en, data, bPseudoTest);

	return ret;
}

static int rtl8188e_Efuse_PgPacketWrite(struct adapter *pAdapter, u8 offset, u8 word_en, u8 *data, bool bPseudoTest)
{
	int	ret;

	if (bPseudoTest)
		ret = Hal_EfusePgPacketWrite_Pseudo (pAdapter, offset, word_en, data, bPseudoTest);
	else
		ret = Hal_EfusePgPacketWrite(pAdapter, offset, word_en, data, bPseudoTest);
	return ret;
}

static struct HAL_VERSION ReadChipVersion8188E(struct adapter *padapter)
{
	u32				value32;
	struct HAL_VERSION		ChipVersion;
	struct hal_data_8188e	*pHalData;

	pHalData = GET_HAL_DATA(padapter);

	value32 = rtw_read32(padapter, REG_SYS_CFG);
	ChipVersion.ICType = CHIP_8188E;
	ChipVersion.ChipType = ((value32 & RTL_ID) ? TEST_CHIP : NORMAL_CHIP);

	ChipVersion.RFType = RF_TYPE_1T1R;
	ChipVersion.VendorType = ((value32 & VENDOR_ID) ? CHIP_VENDOR_UMC : CHIP_VENDOR_TSMC);
	ChipVersion.CUTVersion = (value32 & CHIP_VER_RTL_MASK)>>CHIP_VER_RTL_SHIFT; /*  IC version (CUT) */

	/*  For regulator mode. by tynli. 2011.01.14 */
	pHalData->RegulatorMode = ((value32 & TRP_BT_EN) ? RT_LDO_REGULATOR : RT_SWITCHING_REGULATOR);

	ChipVersion.ROMVer = 0;	/*  ROM code version. */
	pHalData->MultiFunc = RT_MULTI_FUNC_NONE;

	dump_chip_info(ChipVersion);

	pHalData->VersionID = ChipVersion;

	if (IS_1T2R(ChipVersion)) {
		pHalData->rf_type = RF_1T2R;
		pHalData->NumTotalRFPath = 2;
	} else if (IS_2T2R(ChipVersion)) {
		pHalData->rf_type = RF_2T2R;
		pHalData->NumTotalRFPath = 2;
	} else{
		pHalData->rf_type = RF_1T1R;
		pHalData->NumTotalRFPath = 1;
	}

	MSG_88E("RF_Type is %x!!\n", pHalData->rf_type);

	return ChipVersion;
}

static void rtl8188e_read_chip_version(struct adapter *padapter)
{
	ReadChipVersion8188E(padapter);
}

static void rtl8188e_GetHalODMVar(struct adapter *Adapter, enum hal_odm_variable eVariable, void *pValue1, bool bSet)
{
	switch (eVariable) {
	case HAL_ODM_STA_INFO:
		break;
	default:
		break;
	}
}

static void rtl8188e_SetHalODMVar(struct adapter *Adapter, enum hal_odm_variable eVariable, void *pValue1, bool bSet)
{
	struct hal_data_8188e	*pHalData = GET_HAL_DATA(Adapter);
	struct odm_dm_struct *podmpriv = &pHalData->odmpriv;
	switch (eVariable) {
	case HAL_ODM_STA_INFO:
		{
			struct sta_info *psta = (struct sta_info *)pValue1;
			if (bSet) {
				DBG_88E("### Set STA_(%d) info\n", psta->mac_id);
				ODM_CmnInfoPtrArrayHook(podmpriv, ODM_CMNINFO_STA_STATUS, psta->mac_id, psta);
				ODM_RAInfo_Init(podmpriv, psta->mac_id);
			} else {
				DBG_88E("### Clean STA_(%d) info\n", psta->mac_id);
				ODM_CmnInfoPtrArrayHook(podmpriv, ODM_CMNINFO_STA_STATUS, psta->mac_id, NULL);
		       }
		}
		break;
	case HAL_ODM_P2P_STATE:
			ODM_CmnInfoUpdate(podmpriv, ODM_CMNINFO_WIFI_DIRECT, bSet);
		break;
	case HAL_ODM_WIFI_DISPLAY_STATE:
			ODM_CmnInfoUpdate(podmpriv, ODM_CMNINFO_WIFI_DISPLAY, bSet);
		break;
	default:
		break;
	}
}

void rtl8188e_clone_haldata(struct adapter *dst_adapter, struct adapter *src_adapter)
{
	_rtw_memcpy(dst_adapter->HalData, src_adapter->HalData, dst_adapter->hal_data_sz);
}

void rtl8188e_start_thread(struct adapter *padapter)
{
}

void rtl8188e_stop_thread(struct adapter *padapter)
{
}

static void hal_notch_filter_8188e(struct adapter *adapter, bool enable)
{
	if (enable) {
		DBG_88E("Enable notch filter\n");
		rtw_write8(adapter, rOFDM0_RxDSP+1, rtw_read8(adapter, rOFDM0_RxDSP+1) | BIT1);
	} else {
		DBG_88E("Disable notch filter\n");
		rtw_write8(adapter, rOFDM0_RxDSP+1, rtw_read8(adapter, rOFDM0_RxDSP+1) & ~BIT1);
	}
}
void rtl8188e_set_hal_ops(struct hal_ops *pHalFunc)
{
	pHalFunc->free_hal_data = &rtl8188e_free_hal_data;

	pHalFunc->dm_init = &rtl8188e_init_dm_priv;
	pHalFunc->dm_deinit = &rtl8188e_deinit_dm_priv;

	pHalFunc->read_chip_version = &rtl8188e_read_chip_version;

	pHalFunc->set_bwmode_handler = &PHY_SetBWMode8188E;
	pHalFunc->set_channel_handler = &PHY_SwChnl8188E;

	pHalFunc->hal_dm_watchdog = &rtl8188e_HalDmWatchDog;

	pHalFunc->Add_RateATid = &rtl8188e_Add_RateATid;
	pHalFunc->run_thread = &rtl8188e_start_thread;
	pHalFunc->cancel_thread = &rtl8188e_stop_thread;

	pHalFunc->AntDivBeforeLinkHandler = &AntDivBeforeLink8188E;
	pHalFunc->AntDivCompareHandler = &AntDivCompare8188E;
	pHalFunc->read_bbreg = &rtl8188e_PHY_QueryBBReg;
	pHalFunc->write_bbreg = &rtl8188e_PHY_SetBBReg;
	pHalFunc->read_rfreg = &rtl8188e_PHY_QueryRFReg;
	pHalFunc->write_rfreg = &rtl8188e_PHY_SetRFReg;

	/*  Efuse related function */
	pHalFunc->EfusePowerSwitch = &rtl8188e_EfusePowerSwitch;
	pHalFunc->ReadEFuse = &rtl8188e_ReadEFuse;
	pHalFunc->EFUSEGetEfuseDefinition = &rtl8188e_EFUSE_GetEfuseDefinition;
	pHalFunc->EfuseGetCurrentSize = &rtl8188e_EfuseGetCurrentSize;
	pHalFunc->Efuse_PgPacketRead = &rtl8188e_Efuse_PgPacketRead;
	pHalFunc->Efuse_PgPacketWrite = &rtl8188e_Efuse_PgPacketWrite;
	pHalFunc->Efuse_WordEnableDataWrite = &rtl8188e_Efuse_WordEnableDataWrite;

	pHalFunc->sreset_init_value = &sreset_init_value;
	pHalFunc->sreset_reset_value = &sreset_reset_value;
	pHalFunc->silentreset = &rtl8188e_silentreset_for_specific_platform;
	pHalFunc->sreset_xmit_status_check = &rtl8188e_sreset_xmit_status_check;
	pHalFunc->sreset_linked_status_check  = &rtl8188e_sreset_linked_status_check;
	pHalFunc->sreset_get_wifi_status  = &sreset_get_wifi_status;

	pHalFunc->GetHalODMVarHandler = &rtl8188e_GetHalODMVar;
	pHalFunc->SetHalODMVarHandler = &rtl8188e_SetHalODMVar;

	pHalFunc->IOL_exec_cmds_sync = &rtl8188e_IOL_exec_cmds_sync;

	pHalFunc->hal_notch_filter = &hal_notch_filter_8188e;
}

u8 GetEEPROMSize8188E(struct adapter *padapter)
{
	u8 size = 0;
	u32	cr;

	cr = rtw_read16(padapter, REG_9346CR);
	/*  6: EEPROM used is 93C46, 4: boot from E-Fuse. */
	size = (cr & BOOT_FROM_EEPROM) ? 6 : 4;

	MSG_88E("EEPROM type is %s\n", size == 4 ? "E-FUSE" : "93C46");

	return size;
}

/*  */
/*  */
/*  LLT R/W/Init function */
/*  */
/*  */
static s32 _LLTWrite(struct adapter *padapter, u32 address, u32 data)
{
	s32	status = _SUCCESS;
	s32	count = 0;
	u32	value = _LLT_INIT_ADDR(address) | _LLT_INIT_DATA(data) | _LLT_OP(_LLT_WRITE_ACCESS);
	u16	LLTReg = REG_LLT_INIT;

	rtw_write32(padapter, LLTReg, value);

	/* polling */
	do {
		value = rtw_read32(padapter, LLTReg);
		if (_LLT_NO_ACTIVE == _LLT_OP_VALUE(value))
			break;

		if (count > POLLING_LLT_THRESHOLD) {
			RT_TRACE(_module_hal_init_c_, _drv_err_, ("Failed to polling write LLT done at address %d!\n", address));
			status = _FAIL;
			break;
		}
	} while (count++);

	return status;
}

s32 InitLLTTable(struct adapter *padapter, u8 txpktbuf_bndy)
{
	s32	status = _FAIL;
	u32	i;
	u32	Last_Entry_Of_TxPktBuf = LAST_ENTRY_OF_TX_PKT_BUFFER;/*  176, 22k */

	if (rtw_IOL_applied(padapter)) {
		status = iol_InitLLTTable(padapter, txpktbuf_bndy);
	} else {
		for (i = 0; i < (txpktbuf_bndy - 1); i++) {
			status = _LLTWrite(padapter, i, i + 1);
			if (_SUCCESS != status)
				return status;
		}

		/*  end of list */
		status = _LLTWrite(padapter, (txpktbuf_bndy - 1), 0xFF);
		if (_SUCCESS != status)
			return status;

		/*  Make the other pages as ring buffer */
		/*  This ring buffer is used as beacon buffer if we config this MAC as two MAC transfer. */
		/*  Otherwise used as local loopback buffer. */
		for (i = txpktbuf_bndy; i < Last_Entry_Of_TxPktBuf; i++) {
			status = _LLTWrite(padapter, i, (i + 1));
			if (_SUCCESS != status)
				return status;
		}

		/*  Let last entry point to the start entry of ring buffer */
		status = _LLTWrite(padapter, Last_Entry_Of_TxPktBuf, txpktbuf_bndy);
		if (_SUCCESS != status) {
			return status;
		}
	}

	return status;
}

void
Hal_InitPGData88E(struct adapter *padapter)
{
	struct eeprom_priv *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);

	if (!pEEPROM->bautoload_fail_flag) { /*  autoload OK. */
		if (!is_boot_from_eeprom(padapter)) {
			/*  Read EFUSE real map to shadow. */
			EFUSE_ShadowMapUpdate(padapter, EFUSE_WIFI, false);
		}
	} else {/* autoload fail */
		RT_TRACE(_module_hci_hal_init_c_, _drv_notice_, ("AutoLoad Fail reported from CR9346!!\n"));
		/* update to default value 0xFF */
		if (!is_boot_from_eeprom(padapter))
			EFUSE_ShadowMapUpdate(padapter, EFUSE_WIFI, false);
	}
}

void
Hal_EfuseParseIDCode88E(
		struct adapter *padapter,
		u8 *hwinfo
	)
{
	struct eeprom_priv *pEEPROM = GET_EEPROM_EFUSE_PRIV(padapter);
	u16			EEPROMId;

	/*  Checl 0x8129 again for making sure autoload status!! */
	EEPROMId = le16_to_cpu(*((__le16 *)hwinfo));
	if (EEPROMId != RTL_EEPROM_ID) {
		DBG_88E("EEPROM ID(%#x) is invalid!!\n", EEPROMId);
		pEEPROM->bautoload_fail_flag = true;
	} else {
		pEEPROM->bautoload_fail_flag = false;
	}

	DBG_88E("EEPROM ID = 0x%04x\n", EEPROMId);
}

static void
Hal_EEValueCheck(
			u8 EEType,
			void *pInValue,
			void *pOutValue
	)
{
	switch (EEType) {
	case EETYPE_TX_PWR:
		{
			s8	*pIn, *pOut;
			pIn = (u8 *)pInValue;
			pOut = (u8 *)pOutValue;
			if (*pIn >= 0 && *pIn <= 63) {
				*pOut = *pIn;
			} else {
				RT_TRACE(_module_hci_hal_init_c_, _drv_err_, ("EETYPE_TX_PWR, value =%d is invalid, set to default = 0x%x\n",
					 *pIn, EEPROM_Default_TxPowerLevel));
				*pOut = EEPROM_Default_TxPowerLevel;
			}
		}
		break;
	default:
		break;
	}
}

static void Hal_ReadPowerValueFromPROM_8188E(struct txpowerinfo24g *pwrInfo24G, u8 *PROMContent, bool AutoLoadFail)
{
	u32 rfPath, eeAddr = EEPROM_TX_PWR_INX_88E, group, TxCount = 0;

	_rtw_memset(pwrInfo24G, 0, sizeof(struct txpowerinfo24g));

	if (AutoLoadFail) {
		for (rfPath = 0; rfPath < MAX_RF_PATH; rfPath++) {
			/* 2.4G default value */
			for (group = 0; group < MAX_CHNL_GROUP_24G; group++) {
				pwrInfo24G->IndexCCK_Base[rfPath][group] =	EEPROM_DEFAULT_24G_INDEX;
				pwrInfo24G->IndexBW40_Base[rfPath][group] =	EEPROM_DEFAULT_24G_INDEX;
			}
			for (TxCount = 0; TxCount < MAX_TX_COUNT; TxCount++) {
				if (TxCount == 0) {
					pwrInfo24G->BW20_Diff[rfPath][0] = EEPROM_DEFAULT_24G_HT20_DIFF;
					pwrInfo24G->OFDM_Diff[rfPath][0] = EEPROM_DEFAULT_24G_OFDM_DIFF;
				} else {
					pwrInfo24G->BW20_Diff[rfPath][TxCount] = EEPROM_DEFAULT_DIFF;
					pwrInfo24G->BW40_Diff[rfPath][TxCount] = EEPROM_DEFAULT_DIFF;
					pwrInfo24G->CCK_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] = EEPROM_DEFAULT_DIFF;
				}
			}
		}
		return;
	}

	for (rfPath = 0; rfPath < MAX_RF_PATH; rfPath++) {
		/* 2.4G default value */
		for (group = 0; group < MAX_CHNL_GROUP_24G; group++) {
			pwrInfo24G->IndexCCK_Base[rfPath][group] =	PROMContent[eeAddr++];
			if (pwrInfo24G->IndexCCK_Base[rfPath][group] == 0xFF)
				pwrInfo24G->IndexCCK_Base[rfPath][group] = EEPROM_DEFAULT_24G_INDEX;
		}
		for (group = 0; group < MAX_CHNL_GROUP_24G-1; group++) {
			pwrInfo24G->IndexBW40_Base[rfPath][group] =	PROMContent[eeAddr++];
			if (pwrInfo24G->IndexBW40_Base[rfPath][group] == 0xFF)
				pwrInfo24G->IndexBW40_Base[rfPath][group] =	EEPROM_DEFAULT_24G_INDEX;
		}
		for (TxCount = 0; TxCount < MAX_TX_COUNT; TxCount++) {
			if (TxCount == 0) {
				pwrInfo24G->BW40_Diff[rfPath][TxCount] = 0;
				if (PROMContent[eeAddr] == 0xFF) {
					pwrInfo24G->BW20_Diff[rfPath][TxCount] = EEPROM_DEFAULT_24G_HT20_DIFF;
				} else {
					pwrInfo24G->BW20_Diff[rfPath][TxCount] = (PROMContent[eeAddr]&0xf0)>>4;
					if (pwrInfo24G->BW20_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo24G->BW20_Diff[rfPath][TxCount] |= 0xF0;
				}

				if (PROMContent[eeAddr] == 0xFF) {
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_24G_OFDM_DIFF;
				} else {
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0x0f);
					if (pwrInfo24G->OFDM_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo24G->OFDM_Diff[rfPath][TxCount] |= 0xF0;
				}
				pwrInfo24G->CCK_Diff[rfPath][TxCount] = 0;
				eeAddr++;
			} else {
				if (PROMContent[eeAddr] == 0xFF) {
					pwrInfo24G->BW40_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
				} else {
					pwrInfo24G->BW40_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0xf0)>>4;
					if (pwrInfo24G->BW40_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo24G->BW40_Diff[rfPath][TxCount] |= 0xF0;
				}

				if (PROMContent[eeAddr] == 0xFF) {
					pwrInfo24G->BW20_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
				} else {
					pwrInfo24G->BW20_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0x0f);
					if (pwrInfo24G->BW20_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo24G->BW20_Diff[rfPath][TxCount] |= 0xF0;
				}
				eeAddr++;

				if (PROMContent[eeAddr] == 0xFF) {
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] = EEPROM_DEFAULT_DIFF;
				} else {
					pwrInfo24G->OFDM_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0xf0)>>4;
					if (pwrInfo24G->OFDM_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo24G->OFDM_Diff[rfPath][TxCount] |= 0xF0;
				}

				if (PROMContent[eeAddr] == 0xFF) {
					pwrInfo24G->CCK_Diff[rfPath][TxCount] =	EEPROM_DEFAULT_DIFF;
				} else {
					pwrInfo24G->CCK_Diff[rfPath][TxCount] =	(PROMContent[eeAddr]&0x0f);
					if (pwrInfo24G->CCK_Diff[rfPath][TxCount] & BIT3)		/* 4bit sign number to 8 bit sign number */
						pwrInfo24G->CCK_Diff[rfPath][TxCount] |= 0xF0;
				}
				eeAddr++;
			}
		}
	}
}

static u8 Hal_GetChnlGroup(u8 chnl)
{
	u8 group = 0;

	if (chnl < 3)			/*  Cjanel 1-3 */
		group = 0;
	else if (chnl < 9)		/*  Channel 4-9 */
		group = 1;
	else					/*  Channel 10-14 */
		group = 2;

	return group;
}
static u8 Hal_GetChnlGroup88E(u8 chnl, u8 *pGroup)
{
	u8 bIn24G = true;

	if (chnl <= 14) {
		bIn24G = true;

		if (chnl < 3)			/*  Chanel 1-2 */
			*pGroup = 0;
		else if (chnl < 6)		/*  Channel 3-5 */
			*pGroup = 1;
		else	 if (chnl < 9)		/*  Channel 6-8 */
			*pGroup = 2;
		else if (chnl < 12)		/*  Channel 9-11 */
			*pGroup = 3;
		else if (chnl < 14)		/*  Channel 12-13 */
			*pGroup = 4;
		else if (chnl == 14)		/*  Channel 14 */
			*pGroup = 5;
	} else {
		bIn24G = false;

		if (chnl <= 40)
			*pGroup = 0;
		else if (chnl <= 48)
			*pGroup = 1;
		else	 if (chnl <= 56)
			*pGroup = 2;
		else if (chnl <= 64)
			*pGroup = 3;
		else if (chnl <= 104)
			*pGroup = 4;
		else if (chnl <= 112)
			*pGroup = 5;
		else if (chnl <= 120)
			*pGroup = 5;
		else if (chnl <= 128)
			*pGroup = 6;
		else if (chnl <= 136)
			*pGroup = 7;
		else if (chnl <= 144)
			*pGroup = 8;
		else if (chnl <= 153)
			*pGroup = 9;
		else if (chnl <= 161)
			*pGroup = 10;
		else if (chnl <= 177)
			*pGroup = 11;
	}
	return bIn24G;
}

void Hal_ReadPowerSavingMode88E(struct adapter *padapter, u8 *hwinfo, bool AutoLoadFail)
{
	if (AutoLoadFail) {
		padapter->pwrctrlpriv.bHWPowerdown = false;
		padapter->pwrctrlpriv.bSupportRemoteWakeup = false;
	} else {
		/* hw power down mode selection , 0:rf-off / 1:power down */

		if (padapter->registrypriv.hwpdn_mode == 2)
			padapter->pwrctrlpriv.bHWPowerdown = (hwinfo[EEPROM_RF_FEATURE_OPTION_88E] & BIT4);
		else
			padapter->pwrctrlpriv.bHWPowerdown = padapter->registrypriv.hwpdn_mode;

		/*  decide hw if support remote wakeup function */
		/*  if hw supported, 8051 (SIE) will generate WeakUP signal(D+/D- toggle) when autoresume */
		padapter->pwrctrlpriv.bSupportRemoteWakeup = (hwinfo[EEPROM_USB_OPTIONAL_FUNCTION0] & BIT1) ? true : false;

		DBG_88E("%s...bHWPwrPindetect(%x)-bHWPowerdown(%x) , bSupportRemoteWakeup(%x)\n", __func__,
		padapter->pwrctrlpriv.bHWPwrPindetect, padapter->pwrctrlpriv.bHWPowerdown , padapter->pwrctrlpriv.bSupportRemoteWakeup);

		DBG_88E("### PS params =>  power_mgnt(%x), usbss_enable(%x) ###\n", padapter->registrypriv.power_mgnt, padapter->registrypriv.usbss_enable);
	}
}

void Hal_ReadTxPowerInfo88E(struct adapter *padapter, u8 *PROMContent, bool AutoLoadFail)
{
	struct hal_data_8188e	*pHalData = GET_HAL_DATA(padapter);
	struct txpowerinfo24g pwrInfo24G;
	u8 rfPath, ch, group;
	u8 bIn24G, TxCount;

	Hal_ReadPowerValueFromPROM_8188E(&pwrInfo24G, PROMContent, AutoLoadFail);

	if (!AutoLoadFail)
		pHalData->bTXPowerDataReadFromEEPORM = true;

	for (rfPath = 0; rfPath < pHalData->NumTotalRFPath; rfPath++) {
		for (ch = 0; ch < CHANNEL_MAX_NUMBER; ch++) {
			bIn24G = Hal_GetChnlGroup88E(ch, &group);
			if (bIn24G) {
				pHalData->Index24G_CCK_Base[rfPath][ch] = pwrInfo24G.IndexCCK_Base[rfPath][group];
				if (ch == 14)
					pHalData->Index24G_BW40_Base[rfPath][ch] = pwrInfo24G.IndexBW40_Base[rfPath][4];
				else
					pHalData->Index24G_BW40_Base[rfPath][ch] = pwrInfo24G.IndexBW40_Base[rfPath][group];
			}
			if (bIn24G) {
				DBG_88E("======= Path %d, Channel %d =======\n", rfPath, ch);
				DBG_88E("Index24G_CCK_Base[%d][%d] = 0x%x\n", rfPath, ch , pHalData->Index24G_CCK_Base[rfPath][ch]);
				DBG_88E("Index24G_BW40_Base[%d][%d] = 0x%x\n", rfPath, ch , pHalData->Index24G_BW40_Base[rfPath][ch]);
			}
		}
		for (TxCount = 0; TxCount < MAX_TX_COUNT; TxCount++) {
			pHalData->CCK_24G_Diff[rfPath][TxCount] = pwrInfo24G.CCK_Diff[rfPath][TxCount];
			pHalData->OFDM_24G_Diff[rfPath][TxCount] = pwrInfo24G.OFDM_Diff[rfPath][TxCount];
			pHalData->BW20_24G_Diff[rfPath][TxCount] = pwrInfo24G.BW20_Diff[rfPath][TxCount];
			pHalData->BW40_24G_Diff[rfPath][TxCount] = pwrInfo24G.BW40_Diff[rfPath][TxCount];
			DBG_88E("======= TxCount %d =======\n", TxCount);
			DBG_88E("CCK_24G_Diff[%d][%d] = %d\n", rfPath, TxCount, pHalData->CCK_24G_Diff[rfPath][TxCount]);
			DBG_88E("OFDM_24G_Diff[%d][%d] = %d\n", rfPath, TxCount, pHalData->OFDM_24G_Diff[rfPath][TxCount]);
			DBG_88E("BW20_24G_Diff[%d][%d] = %d\n", rfPath, TxCount, pHalData->BW20_24G_Diff[rfPath][TxCount]);
			DBG_88E("BW40_24G_Diff[%d][%d] = %d\n", rfPath, TxCount, pHalData->BW40_24G_Diff[rfPath][TxCount]);
		}
	}

	/*  2010/10/19 MH Add Regulator recognize for CU. */
	if (!AutoLoadFail) {
		pHalData->EEPROMRegulatory = (PROMContent[EEPROM_RF_BOARD_OPTION_88E]&0x7);	/* bit0~2 */
		if (PROMContent[EEPROM_RF_BOARD_OPTION_88E] == 0xFF)
			pHalData->EEPROMRegulatory = (EEPROM_DEFAULT_BOARD_OPTION&0x7);	/* bit0~2 */
	} else {
		pHalData->EEPROMRegulatory = 0;
	}
	DBG_88E("EEPROMRegulatory = 0x%x\n", pHalData->EEPROMRegulatory);
}

void Hal_EfuseParseXtal_8188E(struct adapter *pAdapter, u8 *hwinfo, bool AutoLoadFail)
{
	struct hal_data_8188e	*pHalData = GET_HAL_DATA(pAdapter);

	if (!AutoLoadFail) {
		pHalData->CrystalCap = hwinfo[EEPROM_XTAL_88E];
		if (pHalData->CrystalCap == 0xFF)
			pHalData->CrystalCap = EEPROM_Default_CrystalCap_88E;
	} else {
		pHalData->CrystalCap = EEPROM_Default_CrystalCap_88E;
	}
	DBG_88E("CrystalCap: 0x%2x\n", pHalData->CrystalCap);
}

void Hal_EfuseParseBoardType88E(struct adapter *pAdapter, u8 *hwinfo, bool AutoLoadFail)
{
	struct hal_data_8188e *pHalData = GET_HAL_DATA(pAdapter);

	if (!AutoLoadFail)
		pHalData->BoardType = ((hwinfo[EEPROM_RF_BOARD_OPTION_88E]&0xE0)>>5);
	else
		pHalData->BoardType = 0;
	DBG_88E("Board Type: 0x%2x\n", pHalData->BoardType);
}

void Hal_EfuseParseEEPROMVer88E(struct adapter *padapter, u8 *hwinfo, bool AutoLoadFail)
{
	struct hal_data_8188e *pHalData = GET_HAL_DATA(padapter);

	if (!AutoLoadFail) {
		pHalData->EEPROMVersion = hwinfo[EEPROM_VERSION_88E];
		if (pHalData->EEPROMVersion == 0xFF)
			pHalData->EEPROMVersion = EEPROM_Default_Version;
	} else {
		pHalData->EEPROMVersion = 1;
	}
	RT_TRACE(_module_hci_hal_init_c_, _drv_info_,
		 ("Hal_EfuseParseEEPROMVer(), EEVer = %d\n",
		 pHalData->EEPROMVersion));
}

void rtl8188e_EfuseParseChnlPlan(struct adapter *padapter, u8 *hwinfo, bool AutoLoadFail)
{
	padapter->mlmepriv.ChannelPlan =
		 hal_com_get_channel_plan(padapter,
					  hwinfo ? hwinfo[EEPROM_ChannelPlan_88E] : 0xFF,
					  padapter->registrypriv.channel_plan,
					  RT_CHANNEL_DOMAIN_WORLD_WIDE_13, AutoLoadFail);

	DBG_88E("mlmepriv.ChannelPlan = 0x%02x\n", padapter->mlmepriv.ChannelPlan);
}

void Hal_EfuseParseCustomerID88E(struct adapter *padapter, u8 *hwinfo, bool AutoLoadFail)
{
	struct hal_data_8188e	*pHalData = GET_HAL_DATA(padapter);

	if (!AutoLoadFail) {
		pHalData->EEPROMCustomerID = hwinfo[EEPROM_CUSTOMERID_88E];
	} else {
		pHalData->EEPROMCustomerID = 0;
		pHalData->EEPROMSubCustomerID = 0;
	}
	DBG_88E("EEPROM Customer ID: 0x%2x\n", pHalData->EEPROMCustomerID);
}

void Hal_ReadAntennaDiversity88E(struct adapter *pAdapter, u8 *PROMContent, bool AutoLoadFail)
{
	struct hal_data_8188e	*pHalData = GET_HAL_DATA(pAdapter);
	struct registry_priv	*registry_par = &pAdapter->registrypriv;

	if (!AutoLoadFail) {
		/*  Antenna Diversity setting. */
		if (registry_par->antdiv_cfg == 2) { /*  2:By EFUSE */
			pHalData->AntDivCfg = (PROMContent[EEPROM_RF_BOARD_OPTION_88E]&0x18)>>3;
			if (PROMContent[EEPROM_RF_BOARD_OPTION_88E] == 0xFF)
				pHalData->AntDivCfg = (EEPROM_DEFAULT_BOARD_OPTION&0x18)>>3;;
		} else {
			pHalData->AntDivCfg = registry_par->antdiv_cfg;  /*  0:OFF , 1:ON, 2:By EFUSE */
		}

		if (registry_par->antdiv_type == 0) {
			/* If TRxAntDivType is AUTO in advanced setting, use EFUSE value instead. */
			pHalData->TRxAntDivType = PROMContent[EEPROM_RF_ANTENNA_OPT_88E];
			if (pHalData->TRxAntDivType == 0xFF)
				pHalData->TRxAntDivType = CG_TRX_HW_ANTDIV; /*  For 88EE, 1Tx and 1RxCG are fixed.(1Ant, Tx and RxCG are both on aux port) */
		} else {
			pHalData->TRxAntDivType = registry_par->antdiv_type;
		}

		if (pHalData->TRxAntDivType == CG_TRX_HW_ANTDIV || pHalData->TRxAntDivType == CGCS_RX_HW_ANTDIV)
			pHalData->AntDivCfg = 1; /*  0xC1[3] is ignored. */
	} else {
		pHalData->AntDivCfg = 0;
		pHalData->TRxAntDivType = pHalData->TRxAntDivType; /*  The value in the driver setting of device manager. */
	}
	DBG_88E("EEPROM : AntDivCfg = %x, TRxAntDivType = %x\n", pHalData->AntDivCfg, pHalData->TRxAntDivType);
}

void Hal_ReadThermalMeter_88E(struct adapter *Adapter, u8 *PROMContent, bool AutoloadFail)
{
	struct hal_data_8188e	*pHalData = GET_HAL_DATA(Adapter);

	/*  ThermalMeter from EEPROM */
	if (!AutoloadFail)
		pHalData->EEPROMThermalMeter = PROMContent[EEPROM_THERMAL_METER_88E];
	else
		pHalData->EEPROMThermalMeter = EEPROM_Default_ThermalMeter_88E;

	if (pHalData->EEPROMThermalMeter == 0xff || AutoloadFail) {
		pHalData->bAPKThermalMeterIgnore = true;
		pHalData->EEPROMThermalMeter = EEPROM_Default_ThermalMeter_88E;
	}
	DBG_88E("ThermalMeter = 0x%x\n", pHalData->EEPROMThermalMeter);
}

void Hal_InitChannelPlan(struct adapter *padapter)
{
}

bool HalDetectPwrDownMode88E(struct adapter *Adapter)
{
	u8 tmpvalue = 0;
	struct hal_data_8188e *pHalData = GET_HAL_DATA(Adapter);
	struct pwrctrl_priv *pwrctrlpriv = &Adapter->pwrctrlpriv;

	EFUSE_ShadowRead(Adapter, 1, EEPROM_RF_FEATURE_OPTION_88E, (u32 *)&tmpvalue);

	/*  2010/08/25 MH INF priority > PDN Efuse value. */
	if (tmpvalue & BIT(4) && pwrctrlpriv->reg_pdnmode)
		pHalData->pwrdown = true;
	else
		pHalData->pwrdown = false;

	DBG_88E("HalDetectPwrDownMode(): PDN =%d\n", pHalData->pwrdown);

	return pHalData->pwrdown;
}	/*  HalDetectPwrDownMode */

#ifdef CONFIG_WOWLAN
void Hal_DetectWoWMode(struct adapter *pAdapter)
{
	pAdapter->pwrctrlpriv.bSupportRemoteWakeup = true;
	DBG_88E("%s\n", __func__);
}
#endif

/*  This function is used only for 92C to set REG_BCN_CTRL(0x550) register. */
/*  We just reserve the value of the register in variable pHalData->RegBcnCtrlVal and then operate */
/*  the value of the register via atomic operation. */
/*  This prevents from race condition when setting this register. */
/*  The value of pHalData->RegBcnCtrlVal is initialized in HwConfigureRTL8192CE() function. */

void SetBcnCtrlReg(struct adapter *padapter, u8 SetBits, u8 ClearBits)
{
	struct hal_data_8188e *pHalData;

	pHalData = GET_HAL_DATA(padapter);

	pHalData->RegBcnCtrlVal |= SetBits;
	pHalData->RegBcnCtrlVal &= ~ClearBits;

	rtw_write8(padapter, REG_BCN_CTRL, (u8)pHalData->RegBcnCtrlVal);
}
