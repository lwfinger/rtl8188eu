/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


/*Image2HeaderVersion: 2.18*/
#if (RTL8188E_SUPPORT == 1)
#ifndef __INC_MP_MAC_HW_IMG_8188E_H
#define __INC_MP_MAC_HW_IMG_8188E_H


/******************************************************************************
*                           MAC_REG.TXT
******************************************************************************/

void
odm_read_and_config_mp_8188e_mac_reg(/* TC: Test Chip, MP: MP Chip*/
	struct PHY_DM_STRUCT  *p_dm_odm
);
u32 odm_get_version_mp_8188e_mac_reg(void);

#endif
#endif /* end of HWIMG_SUPPORT*/
