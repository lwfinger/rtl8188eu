/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


#ifndef _FW_HEADER_8188E_T_H
#define _FW_HEADER_8188E_T_H

#ifdef LOAD_FW_HEADER_FROM_DRIVER
#if (defined(CONFIG_AP_WOWLAN))
extern u8 array_mp_8188e_t_fw_ap[15502];
extern u32 array_length_mp_8188e_t_fw_ap;
#endif

extern u8 array_mp_8188e_t_fw_nic[15262];
extern u32 array_length_mp_8188e_t_fw_nic;
extern u8 array_mp_8188e_t_fw_nic_89em[14364];
extern u32 array_length_mp_8188e_t_fw_nic_89em;
#ifdef CONFIG_WOWLAN
extern u8 array_mp_8188e_t_fw_wowlan[16388];
extern u32 array_length_mp_8188e_t_fw_wowlan;
#endif /*CONFIG_WOWLAN*/
#endif /* end of LOAD_FW_HEADER_FROM_DRIVER */

#endif

