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
#define _RTW_EFUSE_C_

#include <drv_types.h>
#include <hal_data.h>

#include "../hal/efuse_mask.h"

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
		return FALSE;

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
		return FALSE;

	if (IS_HARDWARE_TYPE_8188E(pAdapter))
		return (IS_MASKED(8188E, _MUSB, Offset)) ? TRUE : FALSE;

	return FALSE;
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

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, _FALSE);

	ret = rtw_efuse_map_read(padapter, addr, cnts , data);

	if (padapter->registrypriv.boffefusemask == 0) {

		for (i = 0; i < cnts; i++) {
			if (padapter->registrypriv.bFileMaskEfuse == _TRUE) {
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


#ifdef RTW_HALMAC
#include "../../hal/hal_halmac.h"

void Efuse_PowerSwitch(PADAPTER adapter, u8 write, u8 pwrstate)
{
}

void BTEfuse_PowerSwitch(PADAPTER adapter, u8 write, u8 pwrstate)
{
}

u8 efuse_GetCurrentSize(PADAPTER adapter, u16 *size)
{
	*size = 0;

	return _FAIL;
}

u16 efuse_GetMaxSize(PADAPTER adapter)
{
	struct dvobj_priv *d;
	u32 size = 0;
	int err;

	d = adapter_to_dvobj(adapter);
	err = rtw_halmac_get_physical_efuse_size(d, &size);
	if (err)
		return 0;

	return size;
}

u16 efuse_GetavailableSize(PADAPTER adapter)
{
	struct dvobj_priv *d;
	u32 size = 0;
	int err;

	d = adapter_to_dvobj(adapter);
	err = rtw_halmac_get_available_efuse_size(d, &size);
	if (err)
		return 0;

	return size;
}


u8 efuse_bt_GetCurrentSize(PADAPTER adapter, u16 *usesize)
{
	u8 *efuse_map;

	*usesize = 0;
	efuse_map = rtw_malloc(EFUSE_BT_MAP_LEN);
	if (efuse_map == NULL) {
		RTW_DBG("%s: malloc FAIL\n", __func__);
		return _FAIL;
	}

	/* for get bt phy efuse last use byte */
	hal_ReadEFuse_BT_logic_map(adapter, 0x00, EFUSE_BT_MAP_LEN, efuse_map);
	*usesize = fakeBTEfuseUsedBytes;

	if (efuse_map)
		rtw_mfree(efuse_map, EFUSE_BT_MAP_LEN);

	return _SUCCESS;
}

u16 efuse_bt_GetMaxSize(PADAPTER adapter)
{
	return EFUSE_BT_REAL_CONTENT_LEN;
}

void EFUSE_GetEfuseDefinition(PADAPTER adapter, u8 efusetype, u8 type, void *out, bool test)
{
	struct dvobj_priv *d;
	u32 v32 = 0;


	d = adapter_to_dvobj(adapter);

	if (adapter->hal_func.EFUSEGetEfuseDefinition) {
		adapter->hal_func.EFUSEGetEfuseDefinition(adapter, efusetype, type, out, test);
		return;
	}

	if (EFUSE_WIFI == efusetype) {
		switch (type) {
		case TYPE_EFUSE_MAP_LEN:
			rtw_halmac_get_logical_efuse_size(d, &v32);
			*(u16 *)out = (u16)v32;
			return;

		case TYPE_EFUSE_REAL_CONTENT_LEN:	
			rtw_halmac_get_physical_efuse_size(d, &v32);
			*(u16 *)out = (u16)v32;
			return;
		}
	} else if (EFUSE_BT == efusetype) {
		switch (type) {
		case TYPE_EFUSE_MAP_LEN:
			*(u16 *)out = EFUSE_BT_MAP_LEN;
			return;

		case TYPE_EFUSE_REAL_CONTENT_LEN:
			*(u16 *)out = EFUSE_BT_REAL_CONTENT_LEN;
			return;
		}
	}
}

/*
 * read/write raw efuse data
 */
u8 rtw_efuse_access(PADAPTER adapter, u8 write, u16 addr, u16 cnts, u8 *data)
{
	struct dvobj_priv *d;
	u8 *efuse = NULL;
	u32 size, i;
	int err;


	d = adapter_to_dvobj(adapter);
	err = rtw_halmac_get_physical_efuse_size(d, &size);
	if (err)
		size = EFUSE_MAX_SIZE;

	if ((addr + cnts) > size)
		return _FAIL;

	if (_TRUE == write) {
		err = rtw_halmac_write_physical_efuse(d, addr, cnts, data);
		if (err)
			return _FAIL;
	} else {
		if (cnts > 16)
			efuse = rtw_zmalloc(size);

		if (efuse) {
			err = rtw_halmac_read_physical_efuse_map(d, efuse, size);
			if (err) {
				rtw_mfree(efuse, size);
				return _FAIL;
			}

			_rtw_memcpy(data, efuse + addr, cnts);
			rtw_mfree(efuse, size);
		} else {
			err = rtw_halmac_read_physical_efuse(d, addr, cnts, data);
			if (err)
				return _FAIL;
		}
	}

	return _SUCCESS;
}

static inline void dump_buf(u8 *buf, u32 len)
{
	u32 i;

	RTW_INFO("-----------------Len %d----------------\n", len);
	for (i = 0; i < len; i++)
		RTW_INFO("%2.2x-", *(buf + i));
	RTW_INFO("\n");
}

/*
 * read/write raw efuse data
 */
u8 rtw_efuse_bt_access(PADAPTER adapter, u8 write, u16 addr, u16 cnts, u8 *data)
{
	struct dvobj_priv *d;
	u8 *efuse = NULL;
	u32 size, i;
	int err = _FAIL;


	d = adapter_to_dvobj(adapter);

	size = EFUSE_BT_REAL_CONTENT_LEN;

	if ((addr + cnts) > size)
		return _FAIL;

	if (_TRUE == write) {
		err = rtw_halmac_write_bt_physical_efuse(d, addr, cnts, data);
		if (err == -1) {
			RTW_ERR("%s: rtw_halmac_write_bt_physical_efuse fail!\n", __func__);
			return _FAIL;
		}
		RTW_INFO("%s: rtw_halmac_write_bt_physical_efuse OK! data 0x%x\n", __func__, *data);
	} else {
		efuse = rtw_zmalloc(size);

		if (efuse) {
			err = rtw_halmac_read_bt_physical_efuse_map(d, efuse, size);
			
			if (err == -1) {
				RTW_ERR("%s: rtw_halmac_read_bt_physical_efuse_map fail!\n", __func__);
				rtw_mfree(efuse, size);
				return _FAIL;
			}
			dump_buf(efuse + addr, cnts);

			_rtw_memcpy(data, efuse + addr, cnts);

			RTW_INFO("%s: rtw_halmac_read_bt_physical_efuse_map ok! data 0x%x\n", __func__, *data);
			rtw_mfree(efuse, size);
		}
	}

	return _SUCCESS;
}

u8 rtw_efuse_map_read(PADAPTER adapter, u16 addr, u16 cnts, u8 *data)
{
	struct dvobj_priv *d;
	u8 *efuse = NULL;
	u32 size, i;
	int err;


	d = adapter_to_dvobj(adapter);
	err = rtw_halmac_get_logical_efuse_size(d, &size);
	if (err)
		return _FAIL;

	/* size error handle */
	if ((addr + cnts) > size) {
		if (addr < size)
			cnts = size - addr;
		else
			return _FAIL;
	}

	if (cnts > 16)
		efuse = rtw_zmalloc(size);

	if (efuse) {
		err = rtw_halmac_read_logical_efuse_map(d, efuse, size);
		if (err) {
			rtw_mfree(efuse, size);
			return _FAIL;
		}

		_rtw_memcpy(data, efuse + addr, cnts);
		rtw_mfree(efuse, size);
	} else {
		err = rtw_halmac_read_logical_efuse(d, addr, cnts, data);
		if (err)
			return _FAIL;
	}

	return _SUCCESS;
}

u8 rtw_efuse_map_write(PADAPTER adapter, u16 addr, u16 cnts, u8 *data)
{
	struct dvobj_priv *d;
	u8 *efuse = NULL;
	u32 size, i;
	int err;
	u8 mask_buf[64] = "";
	u16 mask_len = sizeof(u8) * rtw_get_efuse_mask_arraylen(adapter);

	d = adapter_to_dvobj(adapter);
	err = rtw_halmac_get_logical_efuse_size(d, &size);
	if (err)
		return _FAIL;

	if ((addr + cnts) > size)
		return _FAIL;

	efuse = rtw_zmalloc(size);
	if (!efuse)
		return _FAIL;

	err = rtw_halmac_read_logical_efuse_map(d, efuse, size);
	if (err) {
		rtw_mfree(efuse, size);
		return _FAIL;
	}

	_rtw_memcpy(efuse + addr, data, cnts);

	if (adapter->registrypriv.boffefusemask == 0) {
		RTW_INFO("Use mask Array Len: %d\n", mask_len);

		if (mask_len != 0) {
			if (adapter->registrypriv.bFileMaskEfuse == _TRUE)
				_rtw_memcpy(mask_buf, maskfileBuffer, mask_len);
			else
				rtw_efuse_mask_array(adapter, mask_buf);

			err = rtw_halmac_write_logical_efuse_map(d, efuse, size, mask_buf, mask_len);
		} else
			err = rtw_halmac_write_logical_efuse_map(d, efuse, size, NULL, 0);
	} else {
		_rtw_memset(mask_buf, 0xFF, sizeof(mask_buf));
		RTW_INFO("Efuse mask off\n");
		err = rtw_halmac_write_logical_efuse_map(d, efuse, size, mask_buf, size/16);
	}

	if (err) {
		rtw_mfree(efuse, size);
		return _FAIL;
	}

	rtw_mfree(efuse, size);

	return _SUCCESS;
}

int Efuse_PgPacketRead(PADAPTER adapter, u8 offset, u8 *data, bool test)
{
	return _FALSE;
}

int Efuse_PgPacketWrite(PADAPTER adapter, u8 offset, u8 word_en, u8 *data, bool test)
{
	return _FALSE;
}

u8 rtw_BT_efuse_map_read(PADAPTER adapter, u16 addr, u16 cnts, u8 *data)
{
	hal_ReadEFuse_BT_logic_map(adapter,addr, cnts, data);

	return _SUCCESS;
}

u8 rtw_BT_efuse_map_write(PADAPTER adapter, u16 addr, u16 cnts, u8 *data)
{
#define RT_ASSERT_RET(expr)									\
	if (!(expr)) {										\
		RTW_INFO("Assertion failed! %s at ......\n", #expr);				\
		RTW_INFO("	  ......%s,%s, line=%d\n",__FILE__, __func__, __LINE__);	\
		return _FAIL;	\
	}

	u8	offset, word_en;
	u8	*map;
	u8	newdata[PGPKT_DATA_SIZE];
	s32 i = 0, j = 0, idx;
	u8	ret = _SUCCESS;
	u16 mapLen = 1024;

	if ((addr + cnts) > mapLen)
		return _FAIL;

	RT_ASSERT_RET(PGPKT_DATA_SIZE == 8); /* have to be 8 byte alignment */
	RT_ASSERT_RET((mapLen & 0x7) == 0); /* have to be PGPKT_DATA_SIZE alignment for memcpy */

	map = rtw_zmalloc(mapLen);
	if (map == NULL)
		return _FAIL;

	ret = rtw_BT_efuse_map_read(adapter, 0, mapLen, map);
	if (ret == _FAIL)
		goto exit;
	RTW_INFO("OFFSET\tVALUE(hex)\n");
	for (i = 0; i < mapLen; i += 16) { /* set 512 because the iwpriv's extra size have limit 0x7FF */
		RTW_INFO("0x%03x\t", i);
		for (j = 0; j < 8; j++)
			RTW_INFO("%02X ", map[i + j]);
		RTW_INFO("\t");
		for (; j < 16; j++)
			RTW_INFO("%02X ", map[i + j]);
		RTW_INFO("\n");
	}
	RTW_INFO("\n");

	idx = 0;
	offset = (addr >> 3);
	while (idx < cnts) {
		word_en = 0xF;
		j = (addr + idx) & 0x7;
		_rtw_memcpy(newdata, &map[offset << 3], PGPKT_DATA_SIZE);
		for (i = j; i < PGPKT_DATA_SIZE && idx < cnts; i++, idx++) {
			if (data[idx] != map[addr + idx]) {
				word_en &= ~BIT(i >> 1);
				newdata[i] = data[idx];
			}
		}

		if (word_en != 0xF) {
			ret = EfusePgPacketWrite_BT(adapter, offset, word_en, newdata, _FALSE);
			RTW_INFO("offset=%x\n", offset);
			RTW_INFO("word_en=%x\n", word_en);
			RTW_INFO("%s: data=", __func__);
			for (i = 0; i < PGPKT_DATA_SIZE; i++)
				RTW_INFO("0x%02X ", newdata[i]);
			RTW_INFO("\n");
			if (ret == _FAIL)
				break;
		}
		offset++;
	}
exit:
	rtw_mfree(map, mapLen);
	return _SUCCESS;
}

void hal_ReadEFuse_BT_logic_map(
	PADAPTER	padapter,
	u16			_offset,
	u16			_size_byte,
	u8			*pbuf
)
{

	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(padapter);
	PEFUSE_HAL		pEfuseHal = &pHalData->EfuseHal;

	u8	*efuseTbl, *phyefuse;
	u8	bank;
	u16	eFuse_Addr = 0;
	u8	efuseHeader, efuseExtHdr, efuseData;
	u8	offset, wden;
	u16	i, total, used;
	u8	efuse_usage;


	/* */
	/* Do NOT excess total size of EFuse table. Added by Roger, 2008.11.10. */
	/* */
	if ((_offset + _size_byte) > EFUSE_BT_MAP_LEN) {
		RTW_INFO("%s: Invalid offset(%#x) with read bytes(%#x)!!\n", __func__, _offset, _size_byte);
		return;
	}

	efuseTbl = rtw_malloc(EFUSE_BT_MAP_LEN);
	phyefuse = rtw_malloc(EFUSE_BT_REAL_CONTENT_LEN);
	if (efuseTbl == NULL || phyefuse == NULL) {
		RTW_INFO("%s: efuseTbl or phyefuse malloc fail!\n", __func__);
		goto exit;
	}

	/* 0xff will be efuse default value instead of 0x00. */
	_rtw_memset(efuseTbl, 0xFF, EFUSE_BT_MAP_LEN);
	_rtw_memset(phyefuse, 0xFF, EFUSE_BT_REAL_CONTENT_LEN);

	if (rtw_efuse_bt_access(padapter, _FALSE, 0, EFUSE_BT_REAL_CONTENT_LEN, phyefuse))
		dump_buf(phyefuse, EFUSE_BT_REAL_BANK_CONTENT_LEN);
	
	total = BANK_NUM;
	for (bank = 1; bank <= total; bank++) { /* 8723d Max bake 0~2 */
		eFuse_Addr = 0;

		while (AVAILABLE_EFUSE_ADDR(eFuse_Addr)) {
			/* ReadEFuseByte(padapter, eFuse_Addr++, &efuseHeader, bPseudoTest); */
			efuseHeader = phyefuse[eFuse_Addr++];

			if (efuseHeader == 0xFF)
				break;
			RTW_INFO("%s: efuse[%#X]=0x%02x (header)\n", __func__, (((bank - 1) * EFUSE_BT_REAL_CONTENT_LEN) + eFuse_Addr - 1), efuseHeader);

			/* Check PG header for section num. */
			if (EXT_HEADER(efuseHeader)) {	/* extended header */
				offset = GET_HDR_OFFSET_2_0(efuseHeader);
				RTW_INFO("%s: extended header offset_2_0=0x%X\n", __func__, offset);

				/* ReadEFuseByte(padapter, eFuse_Addr++, &efuseExtHdr, bPseudoTest); */
				efuseExtHdr = phyefuse[eFuse_Addr++];

				RTW_INFO("%s: efuse[%#X]=0x%02x (ext header)\n", __func__, (((bank - 1) * EFUSE_BT_REAL_CONTENT_LEN) + eFuse_Addr - 1), efuseExtHdr);
				if (ALL_WORDS_DISABLED(efuseExtHdr))
					continue;

				offset |= ((efuseExtHdr & 0xF0) >> 1);
				wden = (efuseExtHdr & 0x0F);
			} else {
				offset = ((efuseHeader >> 4) & 0x0f);
				wden = (efuseHeader & 0x0f);
			}

			if (offset < EFUSE_BT_MAX_SECTION) {
				u16 addr;

				/* Get word enable value from PG header */
				RTW_INFO("%s: Offset=%d Worden=%#X\n", __func__, offset, wden);

				addr = offset * PGPKT_DATA_SIZE;
				for (i = 0; i < EFUSE_MAX_WORD_UNIT; i++) {
					/* Check word enable condition in the section */
					if (!(wden & (0x01 << i))) {
						efuseData = 0;
						/* ReadEFuseByte(padapter, eFuse_Addr++, &efuseData, bPseudoTest); */
						efuseData = phyefuse[eFuse_Addr++];

						RTW_INFO("%s: efuse[%#X]=0x%02X\n", __func__, eFuse_Addr - 1, efuseData);
						efuseTbl[addr] = efuseData;

						efuseData = 0;
						/* ReadEFuseByte(padapter, eFuse_Addr++, &efuseData, bPseudoTest); */
						efuseData = phyefuse[eFuse_Addr++];

						RTW_INFO("%s: efuse[%#X]=0x%02X\n", __func__, eFuse_Addr - 1, efuseData);
						efuseTbl[addr + 1] = efuseData;
					}
					addr += 2;
				}
			} else {
				RTW_INFO("%s: offset(%d) is illegal!!\n", __func__, offset);
				eFuse_Addr += Efuse_CalculateWordCnts(wden) * 2;
			}
		}

		if ((eFuse_Addr - 1) < total) {
			RTW_INFO("%s: bank(%d) data end at %#x\n", __func__, bank, eFuse_Addr - 1);
			break;
		}
	}

	/* switch bank back to bank 0 for later BT and wifi use. */
	//hal_EfuseSwitchToBank(padapter, 0, bPseudoTest);

	/* Copy from Efuse map to output pointer memory!!! */
	for (i = 0; i < _size_byte; i++)
		pbuf[i] = efuseTbl[_offset + i];
	/* Calculate Efuse utilization */
	total = EFUSE_BT_REAL_BANK_CONTENT_LEN;

	used = eFuse_Addr - 1;

	if (total)
		efuse_usage = (u8)((used * 100) / total);
	else
		efuse_usage = 100;

	fakeBTEfuseUsedBytes = used;
	RTW_INFO("%s: BTEfuseUsed last Bytes = %#x\n", __func__, fakeBTEfuseUsedBytes);

exit:
	if (efuseTbl)
		rtw_mfree(efuseTbl, EFUSE_BT_MAP_LEN);
	if (phyefuse)
		rtw_mfree(phyefuse, EFUSE_BT_REAL_BANK_CONTENT_LEN);
}


static u8 hal_EfusePartialWriteCheck(
	PADAPTER		padapter,
	u8				efuseType,
	u16				*pAddr,
	PPGPKT_STRUCT	pTargetPkt,
	u8				bPseudoTest)
{
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(padapter);
	PEFUSE_HAL		pEfuseHal = &pHalData->EfuseHal;
	u8	bRet = _FALSE;
	u16	startAddr = 0, efuse_max_available_len = EFUSE_BT_REAL_BANK_CONTENT_LEN, efuse_max = EFUSE_BT_REAL_BANK_CONTENT_LEN;
	u8	efuse_data = 0;

	startAddr = (u16)fakeBTEfuseUsedBytes;

	startAddr %= efuse_max;
	RTW_INFO("%s: startAddr=%#X\n", __func__, startAddr);

	while (1) {
		if (startAddr >= efuse_max_available_len) {
			bRet = _FALSE;
			RTW_INFO("%s: startAddr(%d) >= efuse_max_available_len(%d)\n",
				__func__, startAddr, efuse_max_available_len);
			break;
		}
		if (rtw_efuse_bt_access(padapter, _FALSE, startAddr, 1, &efuse_data)&& (efuse_data != 0xFF)) {
			bRet = _FALSE;
			RTW_INFO("%s: Something Wrong! last bytes(%#X=0x%02X) is not 0xFF\n",
				 __func__, startAddr, efuse_data);
			break;
		} else {
			/* not used header, 0xff */
			*pAddr = startAddr;
			/*			RTW_INFO("%s: Started from unused header offset=%d\n", __func__, startAddr)); */
			bRet = _TRUE;
			break;
		}
	}

	return bRet;
}


static u8 hal_EfusePgPacketWrite2ByteHeader(
	PADAPTER		padapter,
	u8				efuseType,
	u16				*pAddr,
	PPGPKT_STRUCT	pTargetPkt,
	u8				bPseudoTest)
{
	u16	efuse_addr, efuse_max_available_len = EFUSE_BT_REAL_BANK_CONTENT_LEN;
	u8	pg_header = 0, tmp_header = 0;
	u8	repeatcnt = 0;

	/*	RTW_INFO("%s\n", __func__); */

	efuse_addr = *pAddr;
	if (efuse_addr >= efuse_max_available_len) {
		RTW_INFO("%s: addr(%d) over avaliable(%d)!!\n", __func__, efuse_addr, efuse_max_available_len);
		return _FALSE;
	}

	pg_header = ((pTargetPkt->offset & 0x07) << 5) | 0x0F;
	/*	RTW_INFO("%s: pg_header=0x%x\n", __func__, pg_header); */

	do {
		
		rtw_efuse_bt_access(padapter, _TRUE, efuse_addr, 1, &pg_header);
		rtw_efuse_bt_access(padapter, _FALSE, efuse_addr, 1, &tmp_header);

		if (tmp_header != 0xFF)
			break;
		if (repeatcnt++ > EFUSE_REPEAT_THRESHOLD_) {
			RTW_INFO("%s: Repeat over limit for pg_header!!\n", __func__);
			return _FALSE;
		}
	} while (1);

	if (tmp_header != pg_header) {
		RTW_ERR("%s: PG Header Fail!!(pg=0x%02X read=0x%02X)\n", __func__, pg_header, tmp_header);
		return _FALSE;
	}

	/* to write ext_header */
	efuse_addr++;
	pg_header = ((pTargetPkt->offset & 0x78) << 1) | pTargetPkt->word_en;

	do {
		rtw_efuse_bt_access(padapter, _TRUE, efuse_addr, 1, &pg_header);
		rtw_efuse_bt_access(padapter, _FALSE, efuse_addr, 1, &tmp_header);

		if (tmp_header != 0xFF)
			break;
		if (repeatcnt++ > EFUSE_REPEAT_THRESHOLD_) {
			RTW_INFO("%s: Repeat over limit for ext_header!!\n", __func__);
			return _FALSE;
		}
	} while (1);

	if (tmp_header != pg_header) {	/* offset PG fail */
		RTW_ERR("%s: PG EXT Header Fail!!(pg=0x%02X read=0x%02X)\n", __func__, pg_header, tmp_header);
		return _FALSE;
	}

	*pAddr = efuse_addr;

	return _TRUE;
}


static u8 hal_EfusePgPacketWrite1ByteHeader(
	PADAPTER		pAdapter,
	u8				efuseType,
	u16				*pAddr,
	PPGPKT_STRUCT	pTargetPkt,
	u8				bPseudoTest)
{
	u8	bRet = _FALSE;
	u8	pg_header = 0, tmp_header = 0;
	u16	efuse_addr = *pAddr;
	u8	repeatcnt = 0;


	/*	RTW_INFO("%s\n", __func__); */
	pg_header = ((pTargetPkt->offset << 4) & 0xf0) | pTargetPkt->word_en;

	do {
		rtw_efuse_bt_access(pAdapter, _TRUE, efuse_addr, 1, &pg_header);
		rtw_efuse_bt_access(pAdapter, _FALSE, efuse_addr, 1, &tmp_header);

		if (tmp_header != 0xFF)
			break;
		if (repeatcnt++ > EFUSE_REPEAT_THRESHOLD_) {
			RTW_INFO("%s: Repeat over limit for pg_header!!\n", __func__);
			return _FALSE;
		}
	} while (1);

	if (tmp_header != pg_header) {
		RTW_ERR("%s: PG Header Fail!!(pg=0x%02X read=0x%02X)\n", __func__, pg_header, tmp_header);
		return _FALSE;
	}

	*pAddr = efuse_addr;

	return _TRUE;
}

static u8 hal_EfusePgPacketWriteHeader(
	PADAPTER		padapter,
	u8				efuseType,
	u16				*pAddr,
	PPGPKT_STRUCT	pTargetPkt,
	u8				bPseudoTest)
{
	u8 bRet = _FALSE;

	if (pTargetPkt->offset >= EFUSE_MAX_SECTION_BASE)
		bRet = hal_EfusePgPacketWrite2ByteHeader(padapter, efuseType, pAddr, pTargetPkt, bPseudoTest);
	else
		bRet = hal_EfusePgPacketWrite1ByteHeader(padapter, efuseType, pAddr, pTargetPkt, bPseudoTest);

	return bRet;
}


static u8
Hal_EfuseWordEnableDataWrite(
	PADAPTER	padapter,
	u16			efuse_addr,
	u8			word_en,
	u8			*data,
	u8			bPseudoTest)
{
	u16	tmpaddr = 0;
	u16	start_addr = efuse_addr;
	u8	badworden = 0x0F;
	u8	tmpdata[PGPKT_DATA_SIZE];


	/*	RTW_INFO("%s: efuse_addr=%#x word_en=%#x\n", __func__, efuse_addr, word_en); */
	_rtw_memset(tmpdata, 0xFF, PGPKT_DATA_SIZE);

	if (!(word_en & BIT(0))) {
		tmpaddr = start_addr;
		rtw_efuse_bt_access(padapter, _TRUE, start_addr++, 1, &data[0]);
		rtw_efuse_bt_access(padapter, _TRUE, start_addr++, 1, &data[1]);
		rtw_efuse_bt_access(padapter, _FALSE, tmpaddr, 1, &tmpdata[0]);
		rtw_efuse_bt_access(padapter, _FALSE, tmpaddr + 1, 1, &tmpdata[1]);
		if ((data[0] != tmpdata[0]) || (data[1] != tmpdata[1]))
			badworden &= (~BIT(0));
	}
	if (!(word_en & BIT(1))) {
		tmpaddr = start_addr;
		rtw_efuse_bt_access(padapter, _TRUE, start_addr++, 1, &data[2]);
		rtw_efuse_bt_access(padapter, _TRUE, start_addr++, 1, &data[3]);
		rtw_efuse_bt_access(padapter, _FALSE, tmpaddr, 1, &tmpdata[2]);
		rtw_efuse_bt_access(padapter, _FALSE, tmpaddr + 1, 1, &tmpdata[3]);
		if ((data[2] != tmpdata[2]) || (data[3] != tmpdata[3]))
			badworden &= (~BIT(1));
	}
	if (!(word_en & BIT(2))) {
		tmpaddr = start_addr;
		rtw_efuse_bt_access(padapter, _TRUE, start_addr++, 1, &data[4]);
		rtw_efuse_bt_access(padapter, _TRUE, start_addr++, 1, &data[5]);
		rtw_efuse_bt_access(padapter, _FALSE, tmpaddr, 1, &tmpdata[4]);
		rtw_efuse_bt_access(padapter, _FALSE, tmpaddr + 1, 1, &tmpdata[5]);
		if ((data[4] != tmpdata[4]) || (data[5] != tmpdata[5]))
			badworden &= (~BIT(2));
	}
	if (!(word_en & BIT(3))) {
		tmpaddr = start_addr;
		rtw_efuse_bt_access(padapter, _TRUE, start_addr++, 1, &data[6]);
		rtw_efuse_bt_access(padapter, _TRUE, start_addr++, 1, &data[7]);
		rtw_efuse_bt_access(padapter, _FALSE, tmpaddr, 1, &tmpdata[6]);
		rtw_efuse_bt_access(padapter, _FALSE, tmpaddr + 1, 1, &tmpdata[7]);

		if ((data[6] != tmpdata[6]) || (data[7] != tmpdata[7]))
			badworden &= (~BIT(3));
	}

	return badworden;
}

static void
hal_EfuseConstructPGPkt(
	u8				offset,
	u8				word_en,
	u8				*pData,
	PPGPKT_STRUCT	pTargetPkt)
{
	_rtw_memset(pTargetPkt->data, 0xFF, PGPKT_DATA_SIZE);
	pTargetPkt->offset = offset;
	pTargetPkt->word_en = word_en;
	efuse_WordEnableDataRead(word_en, pData, pTargetPkt->data);
	pTargetPkt->word_cnts = Efuse_CalculateWordCnts(pTargetPkt->word_en);
}

static u8
hal_EfusePgPacketWriteData(
	PADAPTER		pAdapter,
	u8				efuseType,
	u16				*pAddr,
	PPGPKT_STRUCT	pTargetPkt,
	u8				bPseudoTest)
{
	u16	efuse_addr;
	u8	badworden;

	efuse_addr = *pAddr;
	badworden = Hal_EfuseWordEnableDataWrite(pAdapter, efuse_addr + 1, pTargetPkt->word_en, pTargetPkt->data, bPseudoTest);
	if (badworden != 0x0F) {
		RTW_INFO("%s: Fail!!\n", __func__);
		return _FALSE;
	} else
		RTW_INFO("%s: OK!!\n", __func__);

	return _TRUE;
}

/* ***********************************************************
 *				Efuse related code
 * *********************************************************** */
static u8
hal_EfuseSwitchToBank(
	PADAPTER	padapter,
	u8			bank,
	u8			bPseudoTest)
{
	u8 bRet = _FALSE;
	u32 value32 = 0;
#ifdef HAL_EFUSE_MEMORY
	PHAL_DATA_TYPE pHalData = GET_HAL_DATA(padapter);
	PEFUSE_HAL pEfuseHal = &pHalData->EfuseHal;
#endif


	RTW_INFO("%s: Efuse switch bank to %d\n", __func__, bank);
	if (bPseudoTest) {
#ifdef HAL_EFUSE_MEMORY
		pEfuseHal->fakeEfuseBank = bank;
#else
		fakeEfuseBank = bank;
#endif
		bRet = _TRUE;
	} else {
		value32 = rtw_read32(padapter, 0x34);
		bRet = _TRUE;
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
			bRet = _FALSE;
			break;
		}
		rtw_write32(padapter, 0x34, value32);
	}

	return bRet;
}


#define EFUSE_CTRL				0x30		/* E-Fuse Control. */

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
		bResult = _TRUE;
	} else {
		*data = 0xff;
		bResult = _FALSE;
		RTW_INFO("%s: [ERROR] addr=0x%x bResult=%d time out 1s !!!\n", __func__, addr, bResult);
		RTW_INFO("%s: [ERROR] EFUSE_CTRL =0x%08x !!!\n", __func__, rtw_read32(pAdapter, EFUSE_CTRL));
	}

	return bResult;
}


static u16
hal_EfuseGetCurrentSize_BT(
	PADAPTER	padapter,
	u8			bPseudoTest)
{
#ifdef HAL_EFUSE_MEMORY
	PHAL_DATA_TYPE	pHalData = GET_HAL_DATA(padapter);
	PEFUSE_HAL		pEfuseHal = &pHalData->EfuseHal;
#endif
	u16 btusedbytes;
	u16	efuse_addr;
	u8	bank, startBank;
	u8	hoffset = 0, hworden = 0;
	u8	efuse_data, word_cnts = 0;
	u16	retU2 = 0;
	u8 bContinual = _TRUE;


	btusedbytes = fakeBTEfuseUsedBytes;

	efuse_addr = (u16)((btusedbytes % EFUSE_BT_REAL_BANK_CONTENT_LEN));
	startBank = (u8)(1 + (btusedbytes / EFUSE_BT_REAL_BANK_CONTENT_LEN));

	RTW_INFO("%s: start from bank=%d addr=0x%X\n", __func__, startBank, efuse_addr);
	retU2 = EFUSE_BT_REAL_CONTENT_LEN - EFUSE_PROTECT_BYTES_BANK;

	for (bank = startBank; bank < 3; bank++) {
		if (hal_EfuseSwitchToBank(padapter, bank, bPseudoTest) == _FALSE) {
			RTW_ERR("%s: switch bank(%d) Fail!!\n", __func__, bank);
			/* bank = EFUSE_MAX_BANK; */
			break;
		}

		/* only when bank is switched we have to reset the efuse_addr. */
		if (bank != startBank)
			efuse_addr = 0;


		while (AVAILABLE_EFUSE_ADDR(efuse_addr)) {
			if (rtw_efuse_bt_access(padapter, _FALSE, efuse_addr, 1, &efuse_data) == _FALSE) {
				RTW_ERR("%s: efuse_OneByteRead Fail! addr=0x%X !!\n", __func__, efuse_addr);
				/* bank = EFUSE_MAX_BANK; */
				break;
			}
			RTW_INFO("%s: efuse_OneByteRead ! addr=0x%X !efuse_data=0x%X! bank =%d\n", __func__, efuse_addr, efuse_data, bank);

			if (efuse_data == 0xFF)
				break;

			if (EXT_HEADER(efuse_data)) {
				hoffset = GET_HDR_OFFSET_2_0(efuse_data);
				efuse_addr++;
				rtw_efuse_bt_access(padapter, _FALSE, efuse_addr, 1, &efuse_data);
				RTW_INFO("%s: efuse_OneByteRead EXT_HEADER ! addr=0x%X !efuse_data=0x%X! bank =%d\n", __func__, efuse_addr, efuse_data, bank);

				if (ALL_WORDS_DISABLED(efuse_data)) {
					efuse_addr++;
					continue;
				}

				/*				hoffset = ((hoffset & 0xE0) >> 5) | ((efuse_data & 0xF0) >> 1); */
				hoffset |= ((efuse_data & 0xF0) >> 1);
				hworden = efuse_data & 0x0F;
			} else {
				hoffset = (efuse_data >> 4) & 0x0F;
				hworden =  efuse_data & 0x0F;
			}

			RTW_INFO(FUNC_ADPT_FMT": Offset=%d Worden=%#X\n",
				 FUNC_ADPT_ARG(padapter), hoffset, hworden);

			word_cnts = Efuse_CalculateWordCnts(hworden);
			/* read next header */
			efuse_addr += (word_cnts * 2) + 1;
		}
		/* Check if we need to check next bank efuse */
		if (efuse_addr < retU2)
			break;/* don't need to check next bank. */
	}
	retU2 = ((bank - 1) * EFUSE_BT_REAL_BANK_CONTENT_LEN) + efuse_addr;

	fakeBTEfuseUsedBytes = retU2;
	RTW_INFO("%s: CurrentSize=%d\n", __func__, retU2);
	return retU2;
}


static u8
hal_BT_EfusePgCheckAvailableAddr(
	PADAPTER	pAdapter,
	u8		bPseudoTest)
{
	u16	max_available = EFUSE_BT_REAL_CONTENT_LEN - EFUSE_PROTECT_BYTES_BANK;
	u16	current_size = 0;

	 RTW_INFO("%s: max_available=%d\n", __func__, max_available);
	current_size = hal_EfuseGetCurrentSize_BT(pAdapter, bPseudoTest);
	if (current_size >= max_available) {
		RTW_INFO("%s: Error!! current_size(%d)>max_available(%d)\n", __func__, current_size, max_available);
		return _FALSE;
	}
	return _TRUE;
}

u8 EfusePgPacketWrite_BT(
	PADAPTER	pAdapter,
	u8			offset,
	u8			word_en,
	u8			*pData,
	u8			bPseudoTest)
{
	PGPKT_STRUCT targetPkt;
	u16 startAddr = 0;
	u8 efuseType = EFUSE_BT;

	if (!hal_BT_EfusePgCheckAvailableAddr(pAdapter, bPseudoTest))
		return _FALSE;

	hal_EfuseConstructPGPkt(offset, word_en, pData, &targetPkt);

	if (!hal_EfusePartialWriteCheck(pAdapter, efuseType, &startAddr, &targetPkt, bPseudoTest))
		return _FALSE;

	if (!hal_EfusePgPacketWriteHeader(pAdapter, efuseType, &startAddr, &targetPkt, bPseudoTest))
		return _FALSE;

	if (!hal_EfusePgPacketWriteData(pAdapter, efuseType, &startAddr, &targetPkt, bPseudoTest))
		return _FALSE;

	return _TRUE;
}


#else /* !RTW_HALMAC */
/* ------------------------------------------------------------------------------ */
#define REG_EFUSE_CTRL		0x0030
#define EFUSE_CTRL			REG_EFUSE_CTRL		/* E-Fuse Control. */
/* ------------------------------------------------------------------------------ */

static void efuse_PreUpdateAction(
	PADAPTER	pAdapter,
	pu4Byte	BackupRegs)
{
}

static void efuse_PostUpdateAction(
	PADAPTER	pAdapter,
	pu4Byte	BackupRegs)
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
		return _FALSE;
	/* DbgPrint("Read fake content, offset = %d\n", Offset); */
	if (fakeEfuseBank == 0)
		*Value = fakeEfuseContent[Offset];
	else
		*Value = fakeBTEfuseContent[fakeEfuseBank - 1][Offset];
	return _TRUE;
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
		return _FALSE;
	if (fakeEfuseBank == 0)
		fakeEfuseContent[Offset] = Value;
	else
		fakeBTEfuseContent[fakeEfuseBank - 1][Offset] = Value;
	return _TRUE;
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
	/* u32 start=rtw_get_current_time(); */

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
		bResult = _TRUE;
	} else {
		*data = 0xff;
		bResult = _FALSE;
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
	u8	bResult = _FALSE;
	u32 efuseValue = 0;
	HAL_DATA_TYPE	*pHalData = GET_HAL_DATA(pAdapter);

	/* RTW_INFO("===> EFUSE_OneByteWrite(), addr = %x data=%x\n", addr, data); */
	/* RTW_INFO("===> EFUSE_OneByteWrite() start, 0x34 = 0x%X\n", rtw_read32(pAdapter, EFUSE_TEST)); */

	if (bPseudoTest) {
		bResult = Efuse_Write1ByteToFakeContent(pAdapter, addr, data);
		return bResult;
	}

	Efuse_PowerSwitch(pAdapter, _TRUE, _TRUE);

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
		bResult = _TRUE;
	else {
		bResult = _FALSE;
		RTW_INFO("%s: [ERROR] addr=0x%x ,efuseValue=0x%x ,bResult=%d time out 1s !!!\n",
			 __func__, addr, efuseValue, bResult);
		RTW_INFO("%s: [ERROR] EFUSE_CTRL =0x%08x !!!\n", __func__, rtw_read32(pAdapter, EFUSE_CTRL));
	}

	/* disable Efuse program enable */
	if ((IS_VENDOR_8188E_I_CUT_SERIES(pAdapter)) ||
	    (IS_CHIP_VENDOR_SMIC(pHalData->version_id)))
		phy_set_mac_reg(pAdapter, EFUSE_TEST, BIT(11), 0);

	Efuse_PowerSwitch(pAdapter, _TRUE, _FALSE);

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
	return efuse_OneByteRead(padapter, address, value, _FALSE);
}

static u8 efuse_write8(PADAPTER padapter, u16 address, u8 *value)
{
	return efuse_OneByteWrite(padapter, address, *value, _FALSE);
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


	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_REAL_CONTENT_LEN, (void *)&real_content_len, _FALSE);
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_available_size, _FALSE);

	if (start_addr > real_content_len)
		return _FAIL;

	if (_TRUE == bWrite) {
		if ((start_addr + cnts) > max_available_size)
			return _FAIL;
		rw8 = &efuse_write8;
	} else
		rw8 = &efuse_read8;

	efuse_PreUpdateAction(padapter, backupRegs);

	Efuse_PowerSwitch(padapter, bWrite, _TRUE);

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

	Efuse_PowerSwitch(padapter, bWrite, _FALSE);

	efuse_PostUpdateAction(padapter, backupRegs);

	return res;
}
/* ------------------------------------------------------------------------------ */
u16 efuse_GetMaxSize(PADAPTER padapter)
{
	u16	max_size;

	max_size = 0;
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI , TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_size, _FALSE);
	return max_size;
}
/* ------------------------------------------------------------------------------ */
u8 efuse_GetCurrentSize(PADAPTER padapter, u16 *size)
{
	Efuse_PowerSwitch(padapter, _FALSE, _TRUE);
	*size = Efuse_GetCurrentSize(padapter, EFUSE_WIFI, _FALSE);
	Efuse_PowerSwitch(padapter, _FALSE, _FALSE);

	return _SUCCESS;
}
/* ------------------------------------------------------------------------------ */
u16 efuse_bt_GetMaxSize(PADAPTER padapter)
{
	u16	max_size;

	max_size = 0;
	EFUSE_GetEfuseDefinition(padapter, EFUSE_BT , TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, (void *)&max_size, _FALSE);
	return max_size;
}

u8 efuse_bt_GetCurrentSize(PADAPTER padapter, u16 *size)
{
	Efuse_PowerSwitch(padapter, _FALSE, _TRUE);
	*size = Efuse_GetCurrentSize(padapter, EFUSE_BT, _FALSE);
	Efuse_PowerSwitch(padapter, _FALSE, _FALSE);

	return _SUCCESS;
}

u8 rtw_efuse_map_read(PADAPTER padapter, u16 addr, u16 cnts, u8 *data)
{
	u16	mapLen = 0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, _FALSE);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	Efuse_PowerSwitch(padapter, _FALSE, _TRUE);

	efuse_ReadEFuse(padapter, EFUSE_WIFI, addr, cnts, data, _FALSE);

	Efuse_PowerSwitch(padapter, _FALSE, _FALSE);

	return _SUCCESS;
}

u8 rtw_BT_efuse_map_read(PADAPTER padapter, u16 addr, u16 cnts, u8 *data)
{
	u16	mapLen = 0;

	EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, _FALSE);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	Efuse_PowerSwitch(padapter, _FALSE, _TRUE);

	efuse_ReadEFuse(padapter, EFUSE_BT, addr, cnts, data, _FALSE);

	Efuse_PowerSwitch(padapter, _FALSE, _FALSE);

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


	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, _FALSE);
	EFUSE_GetEfuseDefinition(padapter, EFUSE_WIFI, TYPE_AVAILABLE_EFUSE_BYTES_TOTAL, &efuse_max_available_len, _FALSE);

	if ((addr + cnts) > mapLen)
		return _FAIL;

	RT_ASSERT_RET(PGPKT_DATA_SIZE == 8); /* have to be 8 byte alignment */
	RT_ASSERT_RET((mapLen & 0x7) == 0); /* have to be PGPKT_DATA_SIZE alignment for memcpy */

	map = rtw_zmalloc(mapLen);
	if (map == NULL)
		return _FAIL;

	_rtw_memset(map, 0xFF, mapLen);

	ret = rtw_efuse_map_read(padapter, 0, mapLen, map);
	if (ret == _FAIL)
		goto exit;

	if (padapter->registrypriv.boffefusemask == 0) {
		for (i = 0; i < cnts; i++) {
			if (padapter->registrypriv.bFileMaskEfuse == _TRUE) {
				if (rtw_file_efuse_IsMasked(padapter, addr + i))	/*use file efuse mask. */
					data[i] = map[addr + i];
			} else {
				if (efuse_IsMasked(padapter, addr + i))
					data[i] = map[addr + i];
			}
			RTW_INFO("%s , data[%d] = %x, map[addr+i]= %x\n", __func__, i, data[i], map[addr + i]);
		}
	}
	/*Efuse_PowerSwitch(padapter, _TRUE, _TRUE);*/

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
		startAddr = Efuse_GetCurrentSize(padapter, EFUSE_WIFI, _FALSE);
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
		_rtw_memcpy(newdata, &map[offset << 3], PGPKT_DATA_SIZE);
		for (i = j; i < PGPKT_DATA_SIZE && idx < cnts; i++, idx++) {
			if (data[idx] != map[addr + idx]) {
				word_en &= ~BIT(i >> 1);
				newdata[i] = data[idx];
			}
		}

		if (word_en != 0xF) {
			ret = Efuse_PgPacketWrite(padapter, offset, word_en, newdata, _FALSE);
			RTW_INFO("offset=%x\n", offset);
			RTW_INFO("word_en=%x\n", word_en);

			for (i = 0; i < PGPKT_DATA_SIZE; i++)
				RTW_INFO("data=%x \t", newdata[i]);
			if (ret == _FAIL)
				break;
		}

		offset++;
	}

	/*Efuse_PowerSwitch(padapter, _TRUE, _FALSE);*/

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

	EFUSE_GetEfuseDefinition(padapter, EFUSE_BT, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, _FALSE);

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
	Efuse_PowerSwitch(padapter, _TRUE, _TRUE);

	idx = 0;
	offset = (addr >> 3);
	while (idx < cnts) {
		word_en = 0xF;
		j = (addr + idx) & 0x7;
		_rtw_memcpy(newdata, &map[offset << 3], PGPKT_DATA_SIZE);
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
			ret = Efuse_PgPacketWrite_BT(padapter, offset, word_en, newdata, _FALSE);
			if (ret == _FAIL)
				break;
		}

		offset++;
	}

	Efuse_PowerSwitch(padapter, _TRUE, _FALSE);

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

	Efuse_PowerSwitch(pAdapter, _FALSE, _TRUE);

	EFUSE_GetEfuseDefinition(pAdapter, efuseType, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, bPseudoTest);

	efuse_ReadEFuse(pAdapter, efuseType, 0, mapLen, Efuse, bPseudoTest);

	Efuse_PowerSwitch(pAdapter, _FALSE, _FALSE);
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

	_rtw_memset((void *)&fakeEfuseContent[0], 0xff, EFUSE_MAX_HW_SIZE);
	_rtw_memset((void *)&fakeEfuseInitMap[0], 0xff, EFUSE_MAX_MAP_LEN);
	_rtw_memset((void *)&fakeEfuseModifiedMap[0], 0xff, EFUSE_MAX_MAP_LEN);

	for (i = 0; i < EFUSE_MAX_BT_BANK; i++)
		_rtw_memset((void *)&BTEfuseContent[i][0], EFUSE_MAX_HW_SIZE, 0xff);
	_rtw_memset((void *)&BTEfuseInitMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);
	_rtw_memset((void *)&BTEfuseModifiedMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);

	for (i = 0; i < EFUSE_MAX_BT_BANK; i++)
		_rtw_memset((void *)&fakeBTEfuseContent[i][0], 0xff, EFUSE_MAX_HW_SIZE);
	_rtw_memset((void *)&fakeBTEfuseInitMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);
	_rtw_memset((void *)&fakeBTEfuseModifiedMap[0], 0xff, EFUSE_BT_MAX_MAP_LEN);
}
#endif /* !RTW_HALMAC */
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
#ifdef RTW_HALMAC
	u8 *efuse_map = NULL;
	int err;


	mapLen = EEPROM_MAX_SIZE;
	efuse_map = pHalData->efuse_eeprom_data;
	/* efuse default content is 0xFF */
	_rtw_memset(efuse_map, 0xFF, EEPROM_MAX_SIZE);

	EFUSE_GetEfuseDefinition(pAdapter, efuseType, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, bPseudoTest);
	if (!mapLen) {
		RTW_WARN("%s: <ERROR> fail to get efuse size!\n", __func__);
		mapLen = EEPROM_MAX_SIZE;
	}
	if (mapLen > EEPROM_MAX_SIZE) {
		RTW_WARN("%s: <ERROR> size of efuse data(%d) is large than expected(%d)!\n",
			 __func__, mapLen, EEPROM_MAX_SIZE);
		mapLen = EEPROM_MAX_SIZE;
	}

	if (pHalData->bautoload_fail_flag == _FALSE) {
		err = rtw_halmac_read_logical_efuse_map(adapter_to_dvobj(pAdapter), efuse_map, mapLen);
		if (err)
			RTW_ERR("%s: <ERROR> fail to get efuse map!\n", __func__);
	}
#else /* !RTW_HALMAC */
	EFUSE_GetEfuseDefinition(pAdapter, efuseType, TYPE_EFUSE_MAP_LEN, (void *)&mapLen, bPseudoTest);

	if (pHalData->bautoload_fail_flag == _TRUE)
		_rtw_memset(pHalData->efuse_eeprom_data, 0xFF, mapLen);
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

	/* PlatformMoveMemory((void *)&pHalData->EfuseMap[EFUSE_MODIFY_MAP][0], */
	/* (void *)&pHalData->EfuseMap[EFUSE_INIT_MAP][0], mapLen); */
#endif /* !RTW_HALMAC */

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
	return _TRUE;
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
		return _FALSE;

	count = rtw_retrieve_from_file(filepatch, ptmpbuf, bufsize);
	if (count <= 100) {
		rtw_mfree(ptmpbuf, bufsize);
		RTW_ERR("%s, filepatch %s, size=%d, FAIL!!\n", __func__, filepatch, count);
		return _FALSE;
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
	return _TRUE;
}

#ifdef CONFIG_EFUSE_CONFIG_FILE
u32 rtw_read_efuse_from_file(const char *path, u8 *buf, int map_size)
{
	u32 i;
	u8 c;
	u8 temp[3];
	u8 temp_i;
	u8 end = _FALSE;
	u32 ret = _FAIL;

	u8 *file_data = NULL;
	u32 file_size, read_size, pos = 0;
	u8 *map = NULL;

	if (rtw_is_file_readable_with_size(path, &file_size) != _TRUE) {
		RTW_PRINT("%s %s is not readable\n", __func__, path);
		goto exit;
	}

	file_data = rtw_vmalloc(file_size);
	if (!file_data) {
		RTW_ERR("%s rtw_vmalloc(%d) fail\n", __func__, file_size);
		goto exit;
	}

	read_size = rtw_retrieve_from_file(path, file_data, file_size);
	if (read_size == 0) {
		RTW_ERR("%s read from %s fail\n", __func__, path);
		goto exit;
	}

	map = rtw_vmalloc(map_size);
	if (!map) {
		RTW_ERR("%s rtw_vmalloc(%d) fail\n", __func__, map_size);
		goto exit;
	}
	_rtw_memset(map, 0xff, map_size);

	temp[2] = 0; /* end of string '\0' */

	for (i = 0 ; i < map_size ; i++) {
		temp_i = 0;

		while (1) {
			if (pos >= read_size) {
				end = _TRUE;
				break;
			}
			c = file_data[pos++];

			/* bypass spece or eol or null before first hex digit */
			if (temp_i == 0 && (is_eol(c) == _TRUE || is_space(c) == _TRUE || is_null(c) == _TRUE))
				continue;

			if (IsHexDigit(c) == _FALSE) {
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

		if (end == _TRUE) {
			if (temp_i != 0) {
				RTW_ERR("%s incomplete 8-bit hex format for offset:0x%03x\n", __func__, i);
				goto exit;
			}
			break;
		}
	}

	RTW_PRINT("efuse file:%s, 0x%03x byte content read\n", path, i);

	_rtw_memcpy(buf, map, map_size);

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

	if (rtw_is_file_readable(path) != _TRUE) {
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
		if (IsHexDigit(file_data[i * 3]) == _FALSE || IsHexDigit(file_data[i * 3 + 1]) == _FALSE) {
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

	_rtw_memcpy(buf, addr, ETH_ALEN);

	RTW_PRINT("wifi_mac file: %s\n", path);
#ifdef CONFIG_RTW_DEBUG
	RTW_INFO(MAC_FMT"\n", MAC_ARG(buf));
#endif

	ret = _SUCCESS;

exit:
	return ret;
}
#endif /* CONFIG_EFUSE_CONFIG_FILE */
