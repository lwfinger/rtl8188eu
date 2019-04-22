// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


/* #include "Mp_Precomp.h" */
/* #include "../odm_precomp.h" */

#include <drv_types.h>

#include "HalEfuseMask8188E_USB.h"


/******************************************************************************
*                           MUSB.TXT
******************************************************************************/

static u8 Array_MP_8188E_MUSB[] = {
	0xFF,
	0xF3,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x0F,
	0xF1,
	0xFF,
	0xFF,
	0xFF,
	0x00,
	0x00,
	0x00,
	0xF7,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,
	0x00,

};

u16
EFUSE_GetArrayLen_MP_8188E_MUSB(void)
{
	return sizeof(Array_MP_8188E_MUSB) / sizeof(u8);
}

void
EFUSE_GetMaskArray_MP_8188E_MUSB(
	u8 * Array
)
{
	u16 len = EFUSE_GetArrayLen_MP_8188E_MUSB(), i = 0;

	for (i = 0; i < len; ++i)
		Array[i] = Array_MP_8188E_MUSB[i];
}
bool
EFUSE_IsAddressMasked_MP_8188E_MUSB(
	u16  Offset
)
{
	int r = Offset / 16;
	int c = (Offset % 16) / 2;
	int result = 0;

	if (c < 4) /* Upper double word */
		result = (Array_MP_8188E_MUSB[r] & (0x10 << c));
	else
		result = (Array_MP_8188E_MUSB[r] & (0x01 << (c - 4)));

	return (result > 0) ? 0 : 1;
}
