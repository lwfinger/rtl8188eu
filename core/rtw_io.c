// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

/*

The purpose of rtw_io.c

a. provides the API

b. provides the protocol engine

c. provides the software interface between caller and the hardware interface


Compiler Flag Option:

2. CONFIG_USB_HCI:
   a. USE_ASYNC_IRP: Both sync/async operations are provided.
Only sync read/rtw_write_mem operations are provided.

jackson@realtek.com.tw

*/

#define _RTW_IO_C_

#include <drv_types.h>
#include <hal_data.h>

	#define rtw_le16_to_cpu(val)		le16_to_cpu(val)
	#define rtw_le32_to_cpu(val)		le32_to_cpu(val)
	#define rtw_cpu_to_le16(val)		cpu_to_le16(val)
	#define rtw_cpu_to_le32(val)		cpu_to_le32(val)

u8 _rtw_read8(_adapter *adapter, u32 addr)
{
	u8 r_val;
	/* struct	io_queue  	*pio_queue = (struct io_queue *)adapter->pio_queue; */
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl		*pintfhdl = &(pio_priv->intf);
	u8(*_read8)(struct intf_hdl *pintfhdl, u32 addr);
	_read8 = pintfhdl->io_ops._read8;

	r_val = _read8(pintfhdl, addr);
	return r_val;
}

u16 _rtw_read16(_adapter *adapter, u32 addr)
{
	/* struct	io_queue  	*pio_queue = (struct io_queue *)adapter->pio_queue; */
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl		*pintfhdl = &(pio_priv->intf);
	u16(*_read16)(struct intf_hdl *pintfhdl, u32 addr);
	_read16 = pintfhdl->io_ops._read16;

	return _read16(pintfhdl, addr);
}

u32 _rtw_read32(_adapter *adapter, u32 addr)
{
	/* struct	io_queue  	*pio_queue = (struct io_queue *)adapter->pio_queue; */
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl		*pintfhdl = &(pio_priv->intf);
	u32(*_read32)(struct intf_hdl *pintfhdl, u32 addr);
	_read32 = pintfhdl->io_ops._read32;

	return _read32(pintfhdl, addr);
}

int _rtw_write8(_adapter *adapter, u32 addr, u8 val)
{
	/* struct	io_queue  	*pio_queue = (struct io_queue *)adapter->pio_queue; */
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl		*pintfhdl = &(pio_priv->intf);
	int (*_write8)(struct intf_hdl *pintfhdl, u32 addr, u8 val);
	int ret;
	_write8 = pintfhdl->io_ops._write8;

	ret = _write8(pintfhdl, addr, val);

	return RTW_STATUS_CODE(ret);
}
int _rtw_write16(_adapter *adapter, u32 addr, u16 val)
{
	/* struct	io_queue  	*pio_queue = (struct io_queue *)adapter->pio_queue; */
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl		*pintfhdl = &(pio_priv->intf);
	int (*_write16)(struct intf_hdl *pintfhdl, u32 addr, __le16 val);
	int ret;
	__le16 outval;

	_write16 = pintfhdl->io_ops._write16;

	outval = rtw_cpu_to_le16(val);
	ret = _write16(pintfhdl, addr, outval);

	return RTW_STATUS_CODE(ret);
}
int _rtw_write32(_adapter *adapter, u32 addr, u32 val)
{
	/* struct	io_queue  	*pio_queue = (struct io_queue *)adapter->pio_queue; */
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl		*pintfhdl = &(pio_priv->intf);
	int (*_write32)(struct intf_hdl *pintfhdl, u32 addr, __le32 val);
	int ret;
	__le32 outval;

	_write32 = pintfhdl->io_ops._write32;

	outval = rtw_cpu_to_le32(val);
	ret = _write32(pintfhdl, addr, outval);

	return RTW_STATUS_CODE(ret);
}

int _rtw_writeN(_adapter *adapter, u32 addr , u32 length , u8 *pdata)
{
	/* struct	io_queue  	*pio_queue = (struct io_queue *)adapter->pio_queue; */
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl	*pintfhdl = (struct intf_hdl *)(&(pio_priv->intf));
	int (*_writeN)(struct intf_hdl *pintfhdl, u32 addr, u32 length, u8 *pdata);
	int ret;
	_writeN = pintfhdl->io_ops._writeN;

	ret = _writeN(pintfhdl, addr, length, pdata);

	return RTW_STATUS_CODE(ret);
}

int _rtw_write8_async(_adapter *adapter, u32 addr, u8 val)
{
	/* struct	io_queue  	*pio_queue = (struct io_queue *)adapter->pio_queue; */
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl		*pintfhdl = &(pio_priv->intf);
	int (*_write8_async)(struct intf_hdl *pintfhdl, u32 addr, u8 val);
	int ret;
	_write8_async = pintfhdl->io_ops._write8_async;

	ret = _write8_async(pintfhdl, addr, val);

	return RTW_STATUS_CODE(ret);
}
int _rtw_write16_async(_adapter *adapter, u32 addr, u16 val)
{
	/* struct	io_queue  	*pio_queue = (struct io_queue *)adapter->pio_queue; */
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl		*pintfhdl = &(pio_priv->intf);
	int (*_write16_async)(struct intf_hdl *pintfhdl, u32 addr, __le16 val);
	int ret;
	__le16 outval;

	_write16_async = pintfhdl->io_ops._write16_async;
	outval = rtw_cpu_to_le16(val);
	ret = _write16_async(pintfhdl, addr, outval);

	return RTW_STATUS_CODE(ret);
}
int _rtw_write32_async(_adapter *adapter, u32 addr, u32 val)
{
	/* struct	io_queue  	*pio_queue = (struct io_queue *)adapter->pio_queue; */
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl		*pintfhdl = &(pio_priv->intf);
	int (*_write32_async)(struct intf_hdl *pintfhdl, u32 addr, __le32 val);
	int ret;
	__le32 outval;

	_write32_async = pintfhdl->io_ops._write32_async;
	outval = rtw_cpu_to_le32(val);
	ret = _write32_async(pintfhdl, addr, outval);

	return RTW_STATUS_CODE(ret);
}

void _rtw_read_mem(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem)
{
	void (*_read_mem)(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *pmem);
	/* struct	io_queue  	*pio_queue = (struct io_queue *)adapter->pio_queue; */
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl		*pintfhdl = &(pio_priv->intf);


	if (RTW_CANNOT_RUN(adapter)) {
		return;
	}

	_read_mem = pintfhdl->io_ops._read_mem;

	_read_mem(pintfhdl, addr, cnt, pmem);


}

void _rtw_write_mem(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem)
{
	void (*_write_mem)(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *pmem);
	/* struct	io_queue  	*pio_queue = (struct io_queue *)adapter->pio_queue; */
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl		*pintfhdl = &(pio_priv->intf);


	_write_mem = pintfhdl->io_ops._write_mem;

	_write_mem(pintfhdl, addr, cnt, pmem);


}

void _rtw_read_port(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem)
{
	u32(*_read_port)(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *pmem);
	/* struct	io_queue  	*pio_queue = (struct io_queue *)adapter->pio_queue; */
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl		*pintfhdl = &(pio_priv->intf);


	if (RTW_CANNOT_RUN(adapter)) {
		return;
	}

	_read_port = pintfhdl->io_ops._read_port;

	_read_port(pintfhdl, addr, cnt, pmem);


}

void _rtw_read_port_cancel(_adapter *adapter)
{
	void (*_read_port_cancel)(struct intf_hdl *pintfhdl);
	struct io_priv *pio_priv = &adapter->iopriv;
	struct intf_hdl *pintfhdl = &(pio_priv->intf);

	_read_port_cancel = pintfhdl->io_ops._read_port_cancel;

	RTW_DISABLE_FUNC(adapter, DF_RX_BIT);

	if (_read_port_cancel)
		_read_port_cancel(pintfhdl);
}

u32 _rtw_write_port(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem)
{
	u32(*_write_port)(struct intf_hdl *pintfhdl, u32 addr, u32 cnt, u8 *pmem);
	/* struct	io_queue  	*pio_queue = (struct io_queue *)adapter->pio_queue; */
	struct io_priv *pio_priv = &adapter->iopriv;
	struct	intf_hdl		*pintfhdl = &(pio_priv->intf);
	u32 ret = _SUCCESS;


	_write_port = pintfhdl->io_ops._write_port;

	ret = _write_port(pintfhdl, addr, cnt, pmem);


	return ret;
}

u32 _rtw_write_port_and_wait(_adapter *adapter, u32 addr, u32 cnt, u8 *pmem, int timeout_ms)
{
	int ret = _SUCCESS;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)pmem;
	struct submit_ctx sctx;

	rtw_sctx_init(&sctx, timeout_ms);
	pxmitbuf->sctx = &sctx;

	ret = _rtw_write_port(adapter, addr, cnt, pmem);

	if (ret == _SUCCESS)
		ret = rtw_sctx_wait(&sctx, __func__);

	return ret;
}

void _rtw_write_port_cancel(_adapter *adapter)
{
	void (*_write_port_cancel)(struct intf_hdl *pintfhdl);
	struct io_priv *pio_priv = &adapter->iopriv;
	struct intf_hdl *pintfhdl = &(pio_priv->intf);

	_write_port_cancel = pintfhdl->io_ops._write_port_cancel;

	RTW_DISABLE_FUNC(adapter, DF_TX_BIT);

	if (_write_port_cancel)
		_write_port_cancel(pintfhdl);
}
int rtw_init_io_priv(_adapter *padapter, void (*set_intf_ops)(_adapter *padapter, struct _io_ops *pops))
{
	struct io_priv	*piopriv = &padapter->iopriv;
	struct intf_hdl *pintf = &piopriv->intf;

	if (set_intf_ops == NULL)
		return _FAIL;

	piopriv->padapter = padapter;
	pintf->padapter = padapter;
	pintf->pintf_dev = adapter_to_dvobj(padapter);

	set_intf_ops(padapter, &pintf->io_ops);

	return _SUCCESS;
}

/*
* Increase and check if the continual_io_error of this @param dvobjprive is larger than MAX_CONTINUAL_IO_ERR
* @return true:
* @return false:
*/
int rtw_inc_and_chk_continual_io_error(struct dvobj_priv *dvobj)
{
	int ret = false;
	int value;

	value = ATOMIC_INC_RETURN(&dvobj->continual_io_error);
	if (value > MAX_CONTINUAL_IO_ERR) {
		RTW_INFO("[dvobj:%p][ERROR] continual_io_error:%d > %d\n", dvobj, value, MAX_CONTINUAL_IO_ERR);
		ret = true;
	} else {
		/* RTW_INFO("[dvobj:%p] continual_io_error:%d\n", dvobj, value); */
	}
	return ret;
}

/*
* Set the continual_io_error of this @param dvobjprive to 0
*/
void rtw_reset_continual_io_error(struct dvobj_priv *dvobj)
{
	ATOMIC_SET(&dvobj->continual_io_error, 0);
}

#ifdef DBG_IO

u32 read_sniff_ranges[][2] = {
	/* {0x520, 0x523}, */
};

u32 write_sniff_ranges[][2] = {
	/* {0x520, 0x523}, */
	/* {0x4c, 0x4c}, */
};

int read_sniff_num = sizeof(read_sniff_ranges) / sizeof(u32) / 2;
int write_sniff_num = sizeof(write_sniff_ranges) / sizeof(u32) / 2;

bool match_read_sniff_ranges(u32 addr, u16 len)
{
	int i;
	for (i = 0; i < read_sniff_num; i++) {
		if (addr + len > read_sniff_ranges[i][0] && addr <= read_sniff_ranges[i][1])
			return true;
	}

	return false;
}

bool match_write_sniff_ranges(u32 addr, u16 len)
{
	int i;
	for (i = 0; i < write_sniff_num; i++) {
		if (addr + len > write_sniff_ranges[i][0] && addr <= write_sniff_ranges[i][1])
			return true;
	}

	return false;
}

struct rf_sniff_ent {
	u8 path;
	u16 reg;
	u32 mask;
};

struct rf_sniff_ent rf_read_sniff_ranges[] = {
	/* example for all path addr 0x55 with all RF Reg mask */
	/* {MAX_RF_PATH, 0x55, bRFRegOffsetMask}, */
};

struct rf_sniff_ent rf_write_sniff_ranges[] = {
	/* example for all path addr 0x55 with all RF Reg mask */
	/* {MAX_RF_PATH, 0x55, bRFRegOffsetMask}, */
};

int rf_read_sniff_num = sizeof(rf_read_sniff_ranges) / sizeof(struct rf_sniff_ent);
int rf_write_sniff_num = sizeof(rf_write_sniff_ranges) / sizeof(struct rf_sniff_ent);

bool match_rf_read_sniff_ranges(u8 path, u32 addr, u32 mask)
{
	int i;

	for (i = 0; i < rf_read_sniff_num; i++) {
		if (rf_read_sniff_ranges[i].path == MAX_RF_PATH || rf_read_sniff_ranges[i].path == path)
			if (addr == rf_read_sniff_ranges[i].reg && (mask & rf_read_sniff_ranges[i].mask))
				return true;
	}

	return false;
}

bool match_rf_write_sniff_ranges(u8 path, u32 addr, u32 mask)
{
	int i;

	for (i = 0; i < rf_write_sniff_num; i++) {
		if (rf_write_sniff_ranges[i].path == MAX_RF_PATH || rf_write_sniff_ranges[i].path == path)
			if (addr == rf_write_sniff_ranges[i].reg && (mask & rf_write_sniff_ranges[i].mask))
				return true;
	}

	return false;
}

u8 dbg_rtw_read8(_adapter *adapter, u32 addr, const char *caller, const int line)
{
	u8 val = _rtw_read8(adapter, addr);

	if (match_read_sniff_ranges(addr, 1))
		RTW_INFO("DBG_IO %s:%d rtw_read8(0x%04x) return 0x%02x\n", caller, line, addr, val);

	return val;
}

u16 dbg_rtw_read16(_adapter *adapter, u32 addr, const char *caller, const int line)
{
	u16 val = _rtw_read16(adapter, addr);

	if (match_read_sniff_ranges(addr, 2))
		RTW_INFO("DBG_IO %s:%d rtw_read16(0x%04x) return 0x%04x\n", caller, line, addr, val);

	return val;
}

u32 dbg_rtw_read32(_adapter *adapter, u32 addr, const char *caller, const int line)
{
	u32 val = _rtw_read32(adapter, addr);

	if (match_read_sniff_ranges(addr, 4))
		RTW_INFO("DBG_IO %s:%d rtw_read32(0x%04x) return 0x%08x\n", caller, line, addr, val);

	return val;
}

int dbg_rtw_write8(_adapter *adapter, u32 addr, u8 val, const char *caller, const int line)
{
	if (match_write_sniff_ranges(addr, 1))
		RTW_INFO("DBG_IO %s:%d rtw_write8(0x%04x, 0x%02x)\n", caller, line, addr, val);

	return _rtw_write8(adapter, addr, val);
}
int dbg_rtw_write16(_adapter *adapter, u32 addr, u16 val, const char *caller, const int line)
{
	if (match_write_sniff_ranges(addr, 2))
		RTW_INFO("DBG_IO %s:%d rtw_write16(0x%04x, 0x%04x)\n", caller, line, addr, val);

	return _rtw_write16(adapter, addr, val);
}
int dbg_rtw_write32(_adapter *adapter, u32 addr, u32 val, const char *caller, const int line)
{
	if (match_write_sniff_ranges(addr, 4))
		RTW_INFO("DBG_IO %s:%d rtw_write32(0x%04x, 0x%08x)\n", caller, line, addr, val);

	return _rtw_write32(adapter, addr, val);
}
int dbg_rtw_writeN(_adapter *adapter, u32 addr , u32 length , u8 *data, const char *caller, const int line)
{
	if (match_write_sniff_ranges(addr, length))
		RTW_INFO("DBG_IO %s:%d rtw_writeN(0x%04x, %u)\n", caller, line, addr, length);

	return _rtw_writeN(adapter, addr, length, data);
}

#endif
