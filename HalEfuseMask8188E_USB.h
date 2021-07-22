/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */




/******************************************************************************
*                           MUSB.TXT
******************************************************************************/


u16
EFUSE_GetArrayLen_MP_8188E_MUSB(void);

void
EFUSE_GetMaskArray_MP_8188E_MUSB(
	u8 * Array
);

bool
EFUSE_IsAddressMasked_MP_8188E_MUSB(/* TC: Test Chip, MP: MP Chip */
	u16  Offset
);
