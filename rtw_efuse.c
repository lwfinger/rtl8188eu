// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#define _RTW_EFUSE_C_

#include <drv_types.h>
#include <hal_data.h>

#include "efuse_mask.h"

/*------------------------Define local variable------------------------------*/
u8	fakeEfuseBank = {0};
u32	fakeEfuseUsedBytes = {0};
u8	fakeEfuseContent[EFUSE_MAX_HW_SIZE] = {0};
u8	fakeEfuseInitMap[EFUSE_MAX_MAP_LEN] = {0};
u8	fakeEfuseModifiedMap[EFUSE_MAX_MAP_LEN] = {0};

u32	BTEfuseUsedBytes = {0};
u8	BTEfuseContent[EFUSE_MAX_BT_BANK][EFUSE_MAX_HW_SIZE];
u8	BTEfuseInitMap[EFUSE_BT_MAX_MAP_LEN] = {0};
u8	BTEfuseModifiedMap[EFUSE_BT_MAX_MAP_LEN] = {0};

u32	fakeBTEfuseUsedBytes = {0};
u8	fakeBTEfuseContent[EFUSE_MAX_BT_BANK][EFUSE_MAX_HW_SIZE];
u8	fakeBTEfuseInitMap[EFUSE_BT_MAX_MAP_LEN] = {0};
u8	fakeBTEfuseModifiedMap[EFUSE_BT_MAX_MAP_LEN] = {0};

u8	maskfileBuffer[64];
/*------------------------Define local variable------------------------------*/
static bool rtw_file_efuse_IsMasked(PADAPTER pAdapter, u16 Offset)
{
	int r = Offset / 16;
	int c = (Offset % 16) / 2;
	int result = 0;

	if (pAdapter->registrypriv.boffefusemask)
		return false;

	if (c < 4) /* Upper double word */
		result = (maskfileBuffer[r] & (0x10 << c));
	else
		result = (maskfileBuffer[r] & (0x01 << (c - 4)));

	return (result > 0) ? 0 : 1;
}

static bool efuse_IsMasked(PADAPTER pAdapter, u16 Offset)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(pAdapter);

	if (pAdapter->registrypriv.boffefusemask)
		return false;

	if (IS_HARDWARE_TYPE_8188E(pAdapter))
		return (IS_MASKED(8188E, _MUSB, Offset)) ? true : false;

	return false;
}

void rtw_efuse_mask_array(PADAPTER pAdapter, u8 *pArray)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(pAdapter);

	if (IS_HARDWARE_TYPE_8188E(pAdapter))
		GET_MASK_ARRAY(8188E, _MUSB, pArray);
}

u16 rtw_get_efuse_mask_arraylen(PADAPTER pAdapter)
{
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	if (IS_HARDWARE_TYPE_8188E(pAdapter))
		return GET_MASK_ARRAY_LEN(8188E, _MUSB);
	return 0;
}

u8 rtw_efuse_mask_map_read(PADAPTER padapter, u16 addr, u16 cnts, u8 *data)
{
	u8	ret = _SUCCESS;
	u16	mapLen = 0, i = 0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, false);

	ret = rtw_efuse_map_read(padapter, addr, cnts , data);

	if (padapter->registrypriv.boffefusemask == 0) {

		for (i = 0; i < cnts; i++) {
			if (padapter->registrypriv.bFileMaskEfuse == true) {
				if (rtw_file_efuse_IsMasked(padapter, addr + i)) /*use file efuse mask.*/
					data[i] = 0xff;
			} else {
				/*RTW_INFO(" %s , data[%d] = %x\n", __func__, i, data[i]);*/
				if (efuse_IsMasked(padapter, addr + i)) {
					data[i] = 0xff;
					/*RTW_INFO(" %s ,mask data[%d] = %x\n", __func__, i, data[i]);*/
				}
			}
		}

	}
	return ret;

}


/* ------------------------------------------------------------------------------ */
#define REG_EFUSE_CTRL		0x0030
#define EFUSE_CTRL			REG_EFUSE_CTRL		/* E-Fuse Control. */
/* ------------------------------------------------------------------------------ */

static void efuse_PreUpdateAction(
	PADAPTER	pAdapter,
	u32 *	BackupRegs)
{
}

static void efuse_PostUpdateAction(
	PADAPTER	pAdapter,
	u32 *	BackupRegs)
{
}


bool
Efuse_Read1ByteFromFakeContent(
		PADAPTER	pAdapter,
		u16		Offset,
	u8		*Value);
bool
Efuse_Read1ByteFromFakeContent(
		PADAPTER	pAdapter,
		u16		Offset,
	u8		*Value)
{
	if (Offset >= EFUSE_MAX_HW_SIZE)
		return false;
	/* DbgPrint("Read fake content, offset = %d\n", Offset); */
	if (fakeEfuseBank == 0)
		*Value = fakeEfuseContent[Offset];
	else
		*Value = fakeBTEfuseContent[fakeEfuseBank - 1][Offset];
	return true;
}

bool
Efuse_Write1ByteToFakeContent(
		PADAPTER	pAdapter,
		u16		Offset,
		u8		Value);
bool
Efuse_Write1ByteToFakeContent(
		PADAPTER	pAdapter,
		u16		Offset,
		u8		Value)
{
	if (Offset >= EFUSE_MAX_HW_SIZE)
		return false;
	if (fakeEfuseBank == 0)
		fakeEfuseContent[Offset] = Value;
	else
		fakeBTEfuseContent[fakeEfuseBank - 1][Offset] = Value;
	return true;
}

/*-----------------------------------------------------------------------------
 * Function:	Efuse_PowerSwitch
 *
 * Overview:	When we want to enable write operation, we should change to
 *				pwr on state. When we stop write, we should switch to 500k mode
 *				and disable LDO 2.5V.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/17/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void
Efuse_PowerSwitch(
	PADAPTER	pAdapter,
	u8		bWrite,
	u8		PwrState)
{
	pAdapter->hal_func.EfusePowerSwitch(pAdapter, bWrite, PwrState);
}

void
BTEfuse_PowerSwitch(
	PADAPTER	pAdapter,
	u8		bWrite,
	u8		PwrState)
{
	if (pAdapter->hal_func.BTEfusePowerSwitch)
		pAdapter->hal_func.BTEfusePowerSwitch(pAdapter, bWrite, PwrState);
}

/*-----------------------------------------------------------------------------
 * Function:	efuse_GetCurrentSize
 *
 * Overview:	Get current efuse size!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
u16
Efuse_GetCurrentSize(
	PADAPTER		pAdapter,
	u8			efuseType,
	bool		bPseudoTest)
{
	u16 ret = 0;

	ret = pAdapter->hal_func.EfuseGetCurrentSize(pAdapter, efuseType, bPseudoTest);

	return ret;
}

/*
 *	Description:
 *		Execute E-Fuse read byte operation.
 *		Refered from SD1 Richard.
 *
 *	Assumption:
 *		1. Boot from E-Fuse and successfully auto-load.
 *		2. PASSIVE_LEVEL (USB interface)
 *
 *	Created by Roger, 2008.10.21.
 *   */
void
ReadEFuseByte(
	PADAPTER	Adapter,
	u16			_offset,
	u8			*pbuf,
	bool	bPseudoTest)
{
	u32	value32;
	u8	readbyte;
	u16	retry;
	/* u32 start=jiffies; */

	if (bPseudoTest) {
		Efuse_Read1ByteFromFakeContent(Adapter, _offset, pbuf);
		return;
	}
	/* Write Address */
	rtw_write8(Adapter, EFUSE_CTRL + 1, (_offset & 0xff));
	readbyte = rtw_read8(Adapter, EFUSE_CTRL + 2);
	rtw_write8(Adapter, EFUSE_CTRL + 2, ((_offset >> 8) & 0x03) | (readbyte & 0xfc));

	/* Write bit 32 0 */
	readbyte = rtw_read8(Adapter, EFUSE_CTRL + 3);
	rtw_write8(Adapter, EFUSE_CTRL + 3, (readbyte & 0x7f));

	/* Check bit 32 read-ready */
	retry = 0;
	value32 = rtw_read32(Adapter, EFUSE_CTRL);
	/* while(!(((value32 >> 24) & 0xff) & 0x80)  && (retry<10)) */
	while (!(((value32 >> 24) & 0xff) & 0x80)  && (retry < 10000)) {
		value32 = rtw_read32(Adapter, EFUSE_CTRL);
		retry++;
	}

	/* 20100205 Joseph: Add delay suggested by SD1 Victor. */
	/* This fix the problem that Efuse read error in high temperature condition. */
	/* Designer says that there shall be some delay after ready bit is set, or the */
	/* result will always stay on last data we read. */
	rtw_udelay_os(50);
	value32 = rtw_read32(Adapter, EFUSE_CTRL);

	*pbuf = (u8)(value32 & 0xff);
	/* RTW_INFO("ReadEFuseByte _offset:%08u, in %d ms\n",_offset ,rtw_get_passing_time_ms(start)); */

}

/*
 *	Description:
 *		1. Execute E-Fuse read byte operation according as map offset and
 *		    save to E-Fuse table.
 *		2. Refered from SD1 Richard.
 *
 *	Assumption:
 *		1. Boot from E-Fuse and successfully auto-load.
 *		2. PASSIVE_LEVEL (USB interface)
 *
 *	Created by Roger, 2008.10.21.
 *
 *	2008/12/12 MH	1. Reorganize code flow and reserve bytes. and add description.
 *					2. Add efuse utilization collect.
 *	2008/12/22 MH	Read Efuse must check if we write section 1 data again!!! Sec1
 *					write addr must be after sec5.
 *   */

void
efuse_ReadEFuse(
	PADAPTER	Adapter,
	u8		efuseType,
	u16		_offset,
	u16		_size_byte,
	u8	*pbuf,
	bool	bPseudoTest
);
void
efuse_ReadEFuse(
	PADAPTER	Adapter,
	u8		efuseType,
	u16		_offset,
	u16		_size_byte,
	u8	*pbuf,
	bool	bPseudoTest
)
{
	Adapter->hal_func.ReadEFuse(Adapter, efuseType, _offset, _size_byte, pbuf, bPseudoTest);
}

void
EFUSE_GetEfuseDefinition(
		PADAPTER	pAdapter,
		u8		efuseType,
		u8		type,
		void		*pOut,
		bool		bPseudoTest
)
{
	pAdapter->hal_func.EFUSEGetEfuseDefinition(pAdapter, efuseType, type, pOut, bPseudoTest);
}


/*  11/16/2008 MH Read one byte from real Efuse. */
u8
efuse_OneByteRead(
	PADAPTER	pAdapter,
	u16			addr,
	u8			*data,
	bool		bPseudoTest)
{
	u32	tmpidx = 0;
	u8	bResult;
	u8	readbyte;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	/* RTW_INFO("===> EFUSE_OneByteRead(), addr = %x\n", addr); */
	/* RTW_INFO("===> EFUSE_OneByteRead() start, 0x34 = 0x%X\n", rtw_read32(pAdapter, EFUSE_TEST)); */

	if (bPseudoTest) {
		bResult = Efuse_Read1ByteFromFakeContent(pAdapter, addr, data);
		return bResult;
	}

	if ((IS_VENDOR_8188E_I_CUT_SERIES(pAdapter)) ||
	    (IS_CHIP_VENDOR_SMIC(pHalData->version_id))) {
		/* <20130121, Kordan> For SMIC EFUSE specificatoin. */
		/* 0x34[11]: SW force PGMEN input of efuse to high. (for the bank selected by 0x34[9:8])	 */
		/* phy_set_mac_reg(pAdapter, 0x34, BIT11, 0); */
		rtw_write16(pAdapter, 0x34, rtw_read16(pAdapter, 0x34) & (~BIT11));
	}

	/* -----------------e-fuse reg ctrl --------------------------------- */
	/* address			 */
	rtw_write8(pAdapter, EFUSE_CTRL + 1, (u8)(addr & 0xff));
	rtw_write8(pAdapter, EFUSE_CTRL + 2, ((u8)((addr >> 8) & 0x03)) |
		   (rtw_read8(pAdapter, EFUSE_CTRL + 2) & 0xFC));

	/* rtw_write8(pAdapter, EFUSE_CTRL+3,  0x72); */ /* read cmd	 */
	/* Write bit 32 0 */
	readbyte = rtw_read8(pAdapter, EFUSE_CTRL + 3);
	rtw_write8(pAdapter, EFUSE_CTRL + 3, (readbyte & 0x7f));

	while (!(0x80 & rtw_read8(pAdapter, EFUSE_CTRL + 3)) && (tmpidx < 1000)) {
		rtw_mdelay_os(1);
		tmpidx++;
	}
	if (tmpidx < 100) {
		*data = rtw_read8(pAdapter, EFUSE_CTRL);
		bResult = true;
	} else {
		*data = 0xff;
		bResult = false;
		RTW_INFO("%s: [ERROR] addr=0x%x bResult=%d time out 1s !!!\n", __func__, addr, bResult);
		RTW_INFO("%s: [ERROR] EFUSE_CTRL =0x%08x !!!\n", __func__, rtw_read32(pAdapter, EFUSE_CTRL));
	}

	return bResult;
}

/*  11/16/2008 MH Write one byte to reald Efuse. */
u8
efuse_OneByteWrite(
	PADAPTER	pAdapter,
	u16			addr,
	u8			data,
	bool		bPseudoTest)
{
	u8	tmpidx = 0;
	u8	bResult = false;
	u32 efuseValue = 0;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	/* RTW_INFO("===> EFUSE_OneByteWrite(), addr = %x data=%x\n", addr, data); */
	/* RTW_INFO("===> EFUSE_OneByteWrite() start, 0x34 = 0x%X\n", rtw_read32(pAdapter, EFUSE_TEST)); */

	if (bPseudoTest) {
		bResult = Efuse_Write1ByteToFakeContent(pAdapter, addr, data);
		return bResult;
	}

	Efuse_PowerSwitch(pAdapter, true, true);

	/* -----------------e-fuse reg ctrl ---------------------------------	 */
	/* address			 */


	efuseValue = rtw_read32(pAdapter, EFUSE_CTRL);
	efuseValue |= (BIT21 | BIT31);
	efuseValue &= ~(0x3FFFF);
	efuseValue |= ((addr << 8 | data) & 0x3FFFF);

	/* <20130227, Kordan> 8192E MP chip A-cut had better not set 0x34[11] until B-Cut. */
	if ((IS_VENDOR_8188E_I_CUT_SERIES(pAdapter)) ||
	    (IS_CHIP_VENDOR_SMIC(pHalData->version_id))) {
		/* <20130121, Kordan> For SMIC EFUSE specificatoin. */
		/* 0x34[11]: SW force PGMEN input of efuse to high. (for the bank selected by 0x34[9:8]) */
		/* phy_set_mac_reg(pAdapter, 0x34, BIT11, 1); */
		rtw_write16(pAdapter, 0x34, rtw_read16(pAdapter, 0x34) | (BIT11));
		rtw_write32(pAdapter, EFUSE_CTRL, 0x90600000 | ((addr << 8 | data)));
	} else
		rtw_write32(pAdapter, EFUSE_CTRL, efuseValue);

	rtw_mdelay_os(1);

	while ((0x80 &  rtw_read8(pAdapter, EFUSE_CTRL + 3)) && (tmpidx < 100)) {
		rtw_mdelay_os(1);
		tmpidx++;
	}

	if (tmpidx < 100)
		bResult = true;
	else {
		bResult = false;
		RTW_INFO("%s: [ERROR] addr=0x%x ,efuseValue=0x%x ,bResult=%d time out 1s !!!\n",
			 __func__, addr, efuseValue, bResult);
		RTW_INFO("%s: [ERROR] EFUSE_CTRL =0x%08x !!!\n", __func__, rtw_read32(pAdapter, EFUSE_CTRL));
	}

	/* disable Efuse program enable */
	if ((IS_VENDOR_8188E_I_CUT_SERIES(pAdapter)) ||
	    (IS_CHIP_VENDOR_SMIC(pHalData->version_id)))
		phy_set_mac_reg(pAdapter, EFUSE_TEST, BIT(11), 0);

	Efuse_PowerSwitch(pAdapter, true, false);

	return bResult;
}

int
Efuse_PgPacketRead(PADAPTER	pAdapter,
		   u8			offset,
		   u8			*data,
		   bool		bPseudoTest)
{
	int	ret = 0;

	ret =  pAdapter->hal_func.Efuse_PgPacketRead(pAdapter, offset, data, bPseudoTest);

	return ret;
}

int
Efuse_PgPacketWrite(PADAPTER	pAdapter,
		    u8			offset,
		    u8			word_en,
		    u8			*data,
		    bool		bPseudoTest)
{
	int ret;

	ret =  pAdapter->hal_func.Efuse_PgPacketWrite(pAdapter, offset, word_en, data, bPseudoTest);

	return ret;
}


static int
Efuse_PgPacketWrite_BT(PADAPTER	pAdapter,
		       u8			offset,
		       u8			word_en,
		       u8			*data,
		       bool		bPseudoTest)
{
	int ret;

	ret =  pAdapter->hal_func.Efuse_PgPacketWrite_BT(pAdapter, offset, word_en, data, bPseudoTest);

	return ret;
}


u8
Efuse_WordEnableDataWrite(PADAPTER	pAdapter,
			  u16		efuse_addr,
			  u8		word_en,
			  u8		*data,
			  bool		bPseudoTest)
{
	u8	ret = 0;

	ret =  pAdapter->hal_func.Efuse_WordEnableDataWrite(pAdapter, efuse_addr, word_en, data, bPseudoTest);

	return ret;
}

static u8 efuse_read8(PADAPTER padapter, u16 address, u8 *value)
{
	return efuse_OneByteRead(padapter, address, value, false);
}

static u8 efuse_write8(PADAPTER padapter, u16 address, u8 *value)
{
	return efuse_OneByteWrite(padapter, address, *value, false);
}

/*
 * read/wirte raw efuse data
 */
u8 rtw_efuse_access(PADAPTER padapter, u8 bWrite, u16 start_addr, u16 cnts, u8 *data)
{
	int i = 0;
	u16	real_content_len = 0, max_available_size = 0;
	u8 res = _FAIL ;
	u8(*rw8)(PADAPTER, u16, u8 *);
	u32	backupRegs[4] = {0};


	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_REAL_CONTENT_LEN, (void *)&real_content_len, false);
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_available_size, false);

	if (start_addr > real_content_len)
		return _FAIL;

	if (bWrite) {
		if ((start_addr + cnts) > max_available_size)
			return _FAIL;
		rw8 = &efuse_write8;
	} else
		rw8 = &efuse_read8;

	efuse_PreUpdateAction(padapter, backupRegs);

	Efuse_PowerSwitch(padapter, bWrite, true);

	/* e-fuse one byte read / write */
	for (i = 0; i < cnts; i++) {
		if (start_addr >= real_content_len) {
			res = _FAIL;
			break;
		}

		res = rw8(padapter, start_addr++, data++);
		if (_FAIL == res)
			break;
	}

	Efuse_PowerSwitch(padapter, bWrite, false);

	efuse_PostUpdateAction(padapter, backupRegs);

	return res;
}
/* ------------------------------------------------------------------------------ */
u16 efuse_GetMaxSize(PADAPTER padapter)
{
	u16	max_size;

	max_size = 0;
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI , TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_size, false);
	return max_size;
}
/* ------------------------------------------------------------------------------ */
u8 efuse_GetCurrentSize(PADAPTER padapter, u16 *size)
{
	Efuse_PowerSwitch(padapter, false, true);
	*size = Efuse_GetCurrentSize(padapter, EFUSE_WIFI, false);
	Efuse_PowerSwitch(padapter, false, false);

	return _SUCCESS;
}
/* ------------------------------------------------------------------------------ */
u16 efuse_bt_GetMaxSize(PADAPTER padapter)
{
	u16	max_size;

	max_size = 0;
	EFUSE_GetEfuseDefinition(padapter, EFUSE_BT , TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_size, false);
	return max_size;
}

u8 efuse_bt_GetCurrentSize(PADAPTER padapter, u16 *size)
{
	Efuse_PowerSwitch(padapter, false, true);
	*size = Efuse_GetCurrentSize(padapter, EFUSE_BT, false);
	Efuse_PowerSwitch(padapter, false, false);

	return _SUCCESS;
}

u8 rtw_efuse_map_read(PADAPTER padapter, u16 addr, u16 cnts, u8 *data)
{
	u16	mapLen = 0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, false);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	Efuse_PowerSwitch(padapter, false, true);

	efuse_ReadEFuse(padapter, EFUSE_WIFI, addr, cnts, data, false);

	Efuse_PowerSwitch(padapter, false, false);

	return _SUCCESS;
}

u8 rtw_BT_efuse_map_read(PADAPTER padapter, u16 addr, u16 cnts, u8 *data)
{
	u16	mapLen = 0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, false);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	Efuse_PowerSwitch(padapter, false, true);

	efuse_ReadEFuse(padapter, EFUSE_BT, addr, cnts, data, false);

	Efuse_PowerSwitch(padapter, false, false);

	return _SUCCESS;
}

/* ------------------------------------------------------------------------------ */
u8 rtw_efuse_map_write(PADAPTER padapter, u16 addr, u16 cnts, u8 *data)
{
#define RT_ASSERT_RET(expr)												\
	if (!(expr)) {															\
		RTW_INFO("Assertion failed! %s at ......\n", #expr);							\
		RTW_INFO("      ......%s,%s, line=%d\n",__FILE__, __func__, __LINE__);	\
		return _FAIL;	\
	}

	u8	offset, word_en;
	u8	*map;
	u8	newdata[PGPKT_DATA_SIZE];
	s32	i, j, idx, chk_total_byte;
	u8	ret = _SUCCESS;
	u16	mapLen = 0, startAddr = 0, efuse_max_available_len = 0;
	u32	backupRegs[4] = {0};
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(padapter);
	PEFUSE_HAL	pEfuseHal = &pHalData->EfuseHal;


	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, false);
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, &efuse_max_available_len, false);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	RT_ASSERT_RET(PGPKT_DATA_SIZE == 8); /* have to be 8 byte alignment */
	RT_ASSERT_RET((mapLen & 0x7) == 0); /* have to be PGPKT_DATA_SIZE alignment for memcpy */

	map = rtw_zmalloc(mapLen);
	if (map == NULL)
		return _FAIL;

	memset(map, 0xFF, mapLen);

	ret = rtw_efuse_map_read(padapter, 0, mapLen, map);
	if (ret == _FAIL)
		goto exit;

	if (padapter->registrypriv.boffefusemask == 0) {
		for (i = 0; i < cnts; i++) {
			if (padapter->registrypriv.bFileMaskEfuse == true) {
				if (rtw_file_efuse_IsMasked(padapter, addr + i))	/*use file efuse mask. */
					data[i] = map[addr + i];
			} else {
				if (efuse_IsMasked(padapter, addr + i))
					data[i] = map[addr + i];
			}
			RTW_INFO("%s , data[%d] = %x, map[addr+i]= %x\n", __func__, i, data[i], map[addr + i]);
		}
	}
	/*Efuse_PowerSwitch(padapter, true, true);*/

	chk_total_byte = 0;
	idx = 0;
	offset = (addr >> 3);

	while (idx < cnts) {
		word_en = 0xF;
		j = (addr + idx) & 0x7;
		for (i = j; i < PGPKT_DATA_SIZE && idx < cnts; i++, idx++) {
			if (data[idx] != map[addr + idx])
				word_en &= ~BIT(i >> 1);
		}

		if (word_en != 0xF) {
			chk_total_byte += Efuse_CalculateWordCnts(word_en) * 2;

			if (offset >= EFUSE_MAX_SECTION_BASE) /* Over EFUSE_MAX_SECTION 16 for 2 ByteHeader */
				chk_total_byte += 2;
			else
				chk_total_byte += 1;
		}

		offset++;
	}

	RTW_INFO("Total PG bytes Count = %d\n", chk_total_byte);
	rtw_hal_get_hwreg(padapter, HW_VAR_EFUSE_BYTES, (u8 *)&startAddr);

	if (startAddr == 0) {
		startAddr = Efuse_GetCurrentSize(padapter, EFUSE_WIFI, false);
		RTW_INFO("%s: Efuse_GetCurrentSize startAddr=%#X\n", __func__, startAddr);
	}
	RTW_DBG("%s: startAddr=%#X\n", __func__, startAddr);

	if ((startAddr + chk_total_byte) >= efuse_max_available_len) {
		RTW_INFO("%s: startAddr(0x%X) + PG data len %d >= efuse_max_available_len(0x%X)\n",
			 __func__, startAddr, chk_total_byte, efuse_max_available_len);
		ret = _FAIL;
		goto exit;
	}

	efuse_PreUpdateAction(padapter, backupRegs);

	idx = 0;
	offset = (addr >> 3);
	while (idx < cnts) {
		word_en = 0xF;
		j = (addr + idx) & 0x7;
		memcpy(newdata, &map[offset << 3], PGPKT_DATA_SIZE);
		for (i = j; i < PGPKT_DATA_SIZE && idx < cnts; i++, idx++) {
			if (data[idx] != map[addr + idx]) {
				word_en &= ~BIT(i >> 1);
				newdata[i] = data[idx];
			}
		}

		if (word_en != 0xF) {
			ret = Efuse_PgPacketWrite(padapter, offset, word_en, newdata, false);
			RTW_INFO("offset=%x\n", offset);
			RTW_INFO("word_en=%x\n", word_en);

			for (i = 0; i < PGPKT_DATA_SIZE; i++)
				RTW_INFO("data=%x \t", newdata[i]);
			if (ret == _FAIL)
				break;
		}

		offset++;
	}

	/*Efuse_PowerSwitch(padapter, true, false);*/

	efuse_PostUpdateAction(padapter, backupRegs);

exit:

	rtw_mfree(map, mapLen);

	return ret;
}


u8 rtw_BT_efuse_map_write(PADAPTER padapter, u16 addr, u16 cnts, u8 *data)
{
#define RT_ASSERT_RET(expr)												\
	if (!(expr)) {															\
		RTW_INFO("Assertion failed! %s at ......\n", #expr);							\
		RTW_INFO("      ......%s,%s, line=%d\n",__FILE__, __func__, __LINE__);	\
		return _FAIL;	\
	}

	u8	offset, word_en;
	u8	*map;
	u8	newdata[PGPKT_DATA_SIZE];
	s32	i = 0, j = 0, idx;
	u8	ret = _SUCCESS;
	u16	mapLen = 0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, false);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	RT_ASSERT_RET(PGPKT_DATA_SIZE == 8); /* have to be 8 byte alignment */
	RT_ASSERT_RET((mapLen & 0x7) == 0); /* have to be PGPKT_DATA_SIZE alignment for memcpy */

	map = rtw_zmalloc(mapLen);
	if (map == NULL)
		return _FAIL;

	ret = rtw_BT_efuse_map_read(padapter, 0, mapLen, map);
	if (ret == _FAIL)
		goto exit;
	RTW_INFO("OFFSET\tVALUE(hex)\n");
	for (i = 0; i < 1024; i += 16) { /* set 512 because the iwpriv's extra size have limit 0x7FF */
		RTW_INFO("0x%03x\t", i);
		for (j = 0; j < 8; j++)
			RTW_INFO("%02X ", map[i + j]);
		RTW_INFO("\t");
		for (; j < 16; j++)
			RTW_INFO("%02X ", map[i + j]);
		RTW_INFO("\n");
	}
	RTW_INFO("\n");
	Efuse_PowerSwitch(padapter, true, true);

	idx = 0;
	offset = (addr >> 3);
	while (idx < cnts) {
		word_en = 0xF;
		j = (addr + idx) & 0x7;
		memcpy(newdata, &map[offset << 3], PGPKT_DATA_SIZE);
		for (i = j; i < PGPKT_DATA_SIZE && idx < cnts; i++, idx++) {
			if (data[idx] != map[addr + idx]) {
				word_en &= ~BIT(i >> 1);
				newdata[i] = data[idx];
			}
		}

		if (word_en != 0xF) {
			RTW_INFO("offset=%x\n", offset);
			RTW_INFO("word_en=%x\n", word_en);
			RTW_INFO("%s: data=", __func__);
			for (i = 0; i < PGPKT_DATA_SIZE; i++)
				RTW_INFO("0x%02X ", newdata[i]);
			RTW_INFO("\n");
			ret = Efuse_PgPacketWrite_BT(padapter, offset, word_en, newdata, false);
			if (ret == _FAIL)
				break;
		}

		offset++;
	}

	Efuse_PowerSwitch(padapter, true, false);

exit:

	rtw_mfree(map, mapLen);

	return ret;
}

/*-----------------------------------------------------------------------------
 * Function:	Efuse_ReadAllMap
 *
 * Overview:	Read All Efuse content
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/11/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void
Efuse_ReadAllMap(
		PADAPTER	pAdapter,
		u8		efuseType,
	u8		*Efuse,
		bool		bPseudoTest);
void
Efuse_ReadAllMap(
		PADAPTER	pAdapter,
		u8		efuseType,
	u8		*Efuse,
		bool		bPseudoTest)
{
	u16	mapLen = 0;

	Efuse_PowerSwitch(pAdapter, false, true);

	EFUSE_GetEfuseDefinition(pAdapter, efuseType, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, bPseudoTest);

	efuse_ReadEFuse(pAdapter, efuseType, 0, mapLen, Efuse, bPseudoTest);

	Efuse_PowerSwitch(pAdapter, false, false);
}

/*-----------------------------------------------------------------------------
 * Function:	efuse_ShadowRead1Byte
 *			efuse_ShadowRead2Byte
 *			efuse_ShadowRead4Byte
 *
 * Overview:	Read from efuse init map by one/two/four bytes !!!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
static void
efuse_ShadowRead1Byte(
	PADAPTER	pAdapter,
	u16		Offset,
	u8		*Value)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(pAdapter);

	*Value = pHalData->efuse_eeprom_data[Offset];

}	/* EFUSE_ShadowRead1Byte */

/* ---------------Read Two Bytes */
static void
efuse_ShadowRead2Byte(
	PADAPTER	pAdapter,
	u16		Offset,
	u16		*Value)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(pAdapter);

	*Value = pHalData->efuse_eeprom_data[Offset];
	*Value |= pHalData->efuse_eeprom_data[Offset + 1] << 8;

}	/* EFUSE_ShadowRead2Byte */

/* ---------------Read Four Bytes */
static void
efuse_ShadowRead4Byte(
	PADAPTER	pAdapter,
	u16		Offset,
	u32		*Value)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(pAdapter);

	*Value = pHalData->efuse_eeprom_data[Offset];
	*Value |= pHalData->efuse_eeprom_data[Offset + 1] << 8;
	*Value |= pHalData->efuse_eeprom_data[Offset + 2] << 16;
	*Value |= pHalData->efuse_eeprom_data[Offset + 3] << 24;

}	/* efuse_ShadowRead4Byte */


/*-----------------------------------------------------------------------------
 * Function:	efuse_ShadowWrite1Byte
 *			efuse_ShadowWrite2Byte
 *			efuse_ShadowWrite4Byte
 *
 * Overview:	Write efuse modify map by one/two/four byte.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/

static void
efuse_ShadowWrite1Byte(
	PADAPTER	pAdapter,
	u16		Offset,
	u8		Value)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(pAdapter);

	pHalData->efuse_eeprom_data[Offset] = Value;

}	/* efuse_ShadowWrite1Byte */

/* ---------------Write Two Bytes */
static void
efuse_ShadowWrite2Byte(
	PADAPTER	pAdapter,
	u16		Offset,
	u16		Value)
{

	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(pAdapter);


	pHalData->efuse_eeprom_data[Offset] = Value & 0x00FF;
	pHalData->efuse_eeprom_data[Offset + 1] = Value >> 8;

}	/* efuse_ShadowWrite1Byte */

/* ---------------Write Four Bytes */
static void
efuse_ShadowWrite4Byte(
	PADAPTER	pAdapter,
	u16		Offset,
	u32		Value)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(pAdapter);

	pHalData->efuse_eeprom_data[Offset] = (u8)(Value & 0x000000FF);
	pHalData->efuse_eeprom_data[Offset + 1] = (u8)((Value >> 8) & 0x0000FF);
	pHalData->efuse_eeprom_data[Offset + 2] = (u8)((Value >> 16) & 0x00FF);
	pHalData->efuse_eeprom_data[Offset + 3] = (u8)((Value >> 24) & 0xFF);

}	/* efuse_ShadowWrite1Byte */


/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowRead
 *
 * Overview:	Read from efuse init map !!!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void
EFUSE_ShadowRead(
		PADAPTER	pAdapter,
		u8		Type,
		u16		Offset,
	u32		*Value)
{
	if (Type == 1)
		efuse_ShadowRead1Byte(pAdapter, Offset, (u8 *)Value);
	else if (Type == 2)
		efuse_ShadowRead2Byte(pAdapter, Offset, (u16 *)Value);
	else if (Type == 4)
		efuse_ShadowRead4Byte(pAdapter, Offset, (u32 *)Value);

}	/* EFUSE_ShadowRead */

/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowWrite
 *
 * Overview:	Write efuse modify map for later update operation to use!!!!!
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/12/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void
EFUSE_ShadowWrite(
	PADAPTER	pAdapter,
	u8		Type,
	u16		Offset,
	u32		Value);
void
EFUSE_ShadowWrite(
	PADAPTER	pAdapter,
	u8		Type,
	u16		Offset,
	u32		Value)
{
#if (MP_DRIVER == 0)
	return;
#endif
	if (pAdapter->registrypriv.mp_mode == 0)
		return;


	if (Type == 1)
		efuse_ShadowWrite1Byte(pAdapter, Offset, (u8)Value);
	else if (Type == 2)
		efuse_ShadowWrite2Byte(pAdapter, Offset, (u16)Value);
	else if (Type == 4)
		efuse_ShadowWrite4Byte(pAdapter, Offset, (u32)Value);

}	/* EFUSE_ShadowWrite */

void
Efuse_InitSomeVar(
		PADAPTER	pAdapter
);
void
Efuse_InitSomeVar(
		PADAPTER	pAdapter
)
{
	u8 i;

	memset((void *)&fakeEfuseContent[0], 0xff, EFUSE_MAX_HW_SIZE);
	memset((void *)&fakeEfuseInitMap[0], 0xff, EFUSE_MAX_MAP_LEN);
	memset((void *)&fakeEfuseModifiedMap[0], 0xff, EFUSE_MAX_MAP_LEN);

	for (i = 0; i < EFUSE_MAX_BT_BANK; i++)
		memset((void *)&BTEfuseContent[i][0], 0xff, EFUSE_MAX_HW_SIZE);
	memset((void *)&BTEfuseInitMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);
	memset((void *)&BTEfuseModifiedMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);

	for (i = 0; i < EFUSE_MAX_BT_BANK; i++)
		memset((void *)&fakeBTEfuseContent[i][0], 0xff, EFUSE_MAX_HW_SIZE);
	memset((void *)&fakeBTEfuseInitMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);
	memset((void *)&fakeBTEfuseModifiedMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);
}

/*  11/16/2008 MH Add description. Get current efuse area enabled word!!. */
u8
Efuse_CalculateWordCnts(u8	word_en)
{
	u8 word_cnts = 0;
	if (!(word_en & BIT(0)))
		word_cnts++; /* 0 : write enable */
	if (!(word_en & BIT(1)))
		word_cnts++;
	if (!(word_en & BIT(2)))
		word_cnts++;
	if (!(word_en & BIT(3)))
		word_cnts++;
	return word_cnts;
}

/*-----------------------------------------------------------------------------
 * Function:	efuse_WordEnableDataRead
 *
 * Overview:	Read allowed word in current efuse section data.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/16/2008	MHC		Create Version 0.
 * 11/21/2008	MHC		Fix Write bug when we only enable late word.
 *
 *---------------------------------------------------------------------------*/
void
efuse_WordEnableDataRead(u8	word_en,
			 u8	*sourdata,
			 u8	*targetdata)
{
	if (!(word_en & BIT(0))) {
		targetdata[0] = sourdata[0];
		targetdata[1] = sourdata[1];
	}
	if (!(word_en & BIT(1))) {
		targetdata[2] = sourdata[2];
		targetdata[3] = sourdata[3];
	}
	if (!(word_en & BIT(2))) {
		targetdata[4] = sourdata[4];
		targetdata[5] = sourdata[5];
	}
	if (!(word_en & BIT(3))) {
		targetdata[6] = sourdata[6];
		targetdata[7] = sourdata[7];
	}
}

/*-----------------------------------------------------------------------------
 * Function:	EFUSE_ShadowMapUpdate
 *
 * Overview:	Transfer current EFUSE content to shadow init and modify map.
 *
 * Input:       NONE
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 11/13/2008	MHC		Create Version 0.
 *
 *---------------------------------------------------------------------------*/
void EFUSE_ShadowMapUpdate(
	PADAPTER	pAdapter,
	u8		efuseType,
	bool	bPseudoTest)
{
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(pAdapter);
	u16	mapLen = 0;
	EFUSE_GetEfuseDefinition(pAdapter, efuseType, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, bPseudoTest);

	if (pHalData->bautoload_fail_flag == true)
		memset(pHalData->efuse_eeprom_data, 0xFF, mapLen);
	else {
#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
		if (_SUCCESS != retriveAdaptorInfoFile(pAdapter->registrypriv.adaptor_info_caching_file_path, pHalData->efuse_eeprom_data)) {
#endif

			Efuse_ReadAllMap(pAdapter, efuseType, pHalData->efuse_eeprom_data, bPseudoTest);

#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE
			storeAdaptorInfoFile(pAdapter->registrypriv.adaptor_info_caching_file_path, pHalData->efuse_eeprom_data);
		}
#endif
	}

	rtw_dump_cur_efuse(pAdapter);
} /* EFUSE_ShadowMapUpdate */

const u8 _mac_hidden_max_bw_to_hal_bw_cap[MAC_HIDDEN_MAX_BW_NUM] = {
	0,
	0,
	(BW_CAP_160M | BW_CAP_80M | BW_CAP_40M | BW_CAP_20M | BW_CAP_10M | BW_CAP_5M),
	(BW_CAP_5M),
	(BW_CAP_10M | BW_CAP_5M),
	(BW_CAP_20M | BW_CAP_10M | BW_CAP_5M),
	(BW_CAP_40M | BW_CAP_20M | BW_CAP_10M | BW_CAP_5M),
	(BW_CAP_80M | BW_CAP_40M | BW_CAP_20M | BW_CAP_10M | BW_CAP_5M),
};

const u8 _mac_hidden_proto_to_hal_proto_cap[MAC_HIDDEN_PROTOCOL_NUM] = {
	0,
	0,
	(PROTO_CAP_11N | PROTO_CAP_11G | PROTO_CAP_11B),
	(PROTO_CAP_11AC | PROTO_CAP_11N | PROTO_CAP_11G | PROTO_CAP_11B),
};

u8 mac_hidden_wl_func_to_hal_wl_func(u8 func)
{
	u8 wl_func = 0;

	if (func & BIT0)
		wl_func |= WL_FUNC_MIRACAST;
	if (func & BIT1)
		wl_func |= WL_FUNC_P2P;
	if (func & BIT2)
		wl_func |= WL_FUNC_TDLS;
	if (func & BIT3)
		wl_func |= WL_FUNC_FTM;

	return wl_func;
}

#ifdef CONFIG_ADAPTOR_INFO_CACHING_FILE

int isAdaptorInfoFileValid(void)
{
	return true;
}

int storeAdaptorInfoFile(char *path, u8 *efuse_data)
{
	int ret = _SUCCESS;

	if (path && efuse_data) {
		ret = rtw_store_to_file(path, efuse_data, EEPROM_MAX_SIZE_512);
		if (ret == EEPROM_MAX_SIZE)
			ret = _SUCCESS;
		else
			ret = _FAIL;
	} else {
		RTW_INFO("%s NULL pointer\n", __func__);
		ret =  _FAIL;
	}
	return ret;
}

int retriveAdaptorInfoFile(char *path, u8 *efuse_data)
{
	int ret = _SUCCESS;
	mm_segment_t oldfs;
	struct file *fp;

	if (path && efuse_data) {

		ret = rtw_retrieve_from_file(path, efuse_data, EEPROM_MAX_SIZE);

		if (ret == EEPROM_MAX_SIZE)
			ret = _SUCCESS;
		else
			ret = _FAIL;
	} else {
		RTW_INFO("%s NULL pointer\n", __func__);
		ret = _FAIL;
	}
	return ret;
}
#endif /* CONFIG_ADAPTOR_INFO_CACHING_FILE */

u8 rtw_efuse_file_read(PADAPTER padapter, u8 *filepatch, u8 *buf, u32 len)
{
	char *ptmpbuf = NULL, *ptr;
	u8 val8;
	u32 count, i, j;
	int err;
	u32 bufsize = 4096;

	ptmpbuf = rtw_zmalloc(bufsize);
	if (ptmpbuf == NULL)
		return false;

	count = rtw_retrieve_from_file(filepatch, ptmpbuf, bufsize);
	if (count <= 100) {
		rtw_mfree(ptmpbuf, bufsize);
		RTW_ERR("%s, filepatch %s, size=%d, FAIL!!\n", __func__, filepatch, count);
		return false;
	}

	i = 0;
	j = 0;
	ptr = ptmpbuf;
	while ((j < len) && (i < count)) {
		if (ptmpbuf[i] == '\0')
			break;
	
		ptr = strpbrk(&ptmpbuf[i], " \t\n\r");
		if (ptr) {
			if (ptr == &ptmpbuf[i]) {
				i++;
				continue;
			}

			/* Add string terminating null */
			*ptr = 0;
		} else {
			ptr = &ptmpbuf[count-1];
		}

		err = sscanf(&ptmpbuf[i], "%hhx", &val8);
		if (err != 1) {
			RTW_WARN("Something wrong to parse efuse file, string=%s\n", &ptmpbuf[i]);
		} else {
			buf[j] = val8;
			RTW_DBG("i=%d, j=%d, 0x%02x\n", i, j, buf[j]);
			j++;
		}

		i = ptr - ptmpbuf + 1;
	}

	rtw_mfree(ptmpbuf, bufsize);
	RTW_INFO("%s, filepatch %s, size=%d, done\n", __func__, filepatch, count);
	return true;
}

#ifdef CONFIG_EFUSE_CONFIG_FILE
u32 rtw_read_efuse_from_file(const char *path, u8 *buf, int map_size)
{
	u32 i;
	u8 c;
	u8 temp[3];
	u8 temp_i;
	u8 end = false;
	u32 ret = _FAIL;

	u8 *file_data = NULL;
	u32 file_size, read_size, pos = 0;
	u8 *map = NULL;

	if (rtw_is_file_readable_with_size(path, &file_size) != true) {
		RTW_PRINT("%s %s is not readable\n", __func__, path);
		goto exit;
	}

	file_data = vmalloc(file_size);
	if (!file_data) {
		RTW_ERR("%s vmalloc(%d) fail\n", __func__, file_size);
		goto exit;
	}

	read_size = rtw_retrieve_from_file(path, file_data, file_size);
	if (read_size == 0) {
		RTW_ERR("%s read from %s fail\n", __func__, path);
		goto exit;
	}

	map = vmalloc(map_size);
	if (!map) {
		RTW_ERR("%s vmalloc(%d) fail\n", __func__, map_size);
		goto exit;
	}
	memset(map, 0xff, map_size);

	temp[2] = 0; /* end of string '\0' */

	for (i = 0 ; i < map_size ; i++) {
		temp_i = 0;

		while (1) {
			if (pos >= read_size) {
				end = true;
				break;
			}
			c = file_data[pos++];

			/* bypass spece or eol or null before first hex digit */
			if (temp_i == 0 && (is_eol(c) == true || is_space(c) == true || is_null(c) == true))
				continue;

			if (IsHexDigit(c) == false) {
				RTW_ERR("%s invalid 8-bit hex format for offset:0x%03x\n", __func__, i);
				goto exit;
			}

			temp[temp_i++] = c;

			if (temp_i == 2) {
				/* parse value */
				if (sscanf(temp, "%hhx", &map[i]) != 1) {
					RTW_ERR("%s sscanf fail for offset:0x%03x\n", __func__, i);
					goto exit;
				}
				break;
			}
		}

		if (end == true) {
			if (temp_i != 0) {
				RTW_ERR("%s incomplete 8-bit hex format for offset:0x%03x\n", __func__, i);
				goto exit;
			}
			break;
		}
	}

	RTW_PRINT("efuse file:%s, 0x%03x byte content read\n", path, i);

	memcpy(buf, map, map_size);

	ret = _SUCCESS;

exit:
	if (file_data)
		rtw_vmfree(file_data, file_size);
	if (map)
		rtw_vmfree(map, map_size);

	return ret;
}

u32 rtw_read_macaddr_from_file(const char *path, u8 *buf)
{
	u32 i;
	u8 temp[3];
	u32 ret = _FAIL;

	u8 file_data[17];
	u32 read_size, pos = 0;
	u8 addr[ETH_ALEN];

	if (rtw_is_file_readable(path) != true) {
		RTW_PRINT("%s %s is not readable\n", __func__, path);
		goto exit;
	}

	read_size = rtw_retrieve_from_file(path, file_data, 17);
	if (read_size != 17) {
		RTW_ERR("%s read from %s fail\n", __func__, path);
		goto exit;
	}

	temp[2] = 0; /* end of string '\0' */

	for (i = 0 ; i < ETH_ALEN ; i++) {
		if (IsHexDigit(file_data[i * 3]) == false || IsHexDigit(file_data[i * 3 + 1]) == false) {
			RTW_ERR("%s invalid 8-bit hex format for address offset:%u\n", __func__, i);
			goto exit;
		}

		if (i < ETH_ALEN - 1 && file_data[i * 3 + 2] != ':') {
			RTW_ERR("%s invalid separator after address offset:%u\n", __func__, i);
			goto exit;
		}

		temp[0] = file_data[i * 3];
		temp[1] = file_data[i * 3 + 1];
		if (sscanf(temp, "%hhx", &addr[i]) != 1) {
			RTW_ERR("%s sscanf fail for address offset:0x%03x\n", __func__, i);
			goto exit;
		}
	}

	memcpy(buf, addr, ETH_ALEN);

	RTW_PRINT("wifi_mac file: %s\n", path);
#ifdef CONFIG_RTW_DEBUG
	RTW_INFO(MAC_FMT"\n", MAC_ARG(buf));
#endif

	ret = _SUCCESS;

exit:
	return ret;
}
#endif /* CONFIG_EFUSE_CONFIG_FILE */
