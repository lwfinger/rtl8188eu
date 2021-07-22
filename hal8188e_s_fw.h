/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


#ifndef _FW_HEADER_8188E_S_H
#define _FW_HEADER_8188E_S_H

#ifdef CONFIG_SFW_SUPPORTED

#ifdef LOAD_FW_HEADER_FROM_DRIVER
#if defined(CONFIG_AP_WOWLAN)
extern u8 array_mp_8188e_s_fw_ap[16054];
extern u32 array_length_mp_8188e_s_fw_ap;
#endif

extern u8 array_mp_8188e_s_fw_nic[19206];
extern u32 array_length_mp_8188e_s_fw_nic;
#ifdef CONFIG_WOWLAN
extern u8 array_mp_8188e_s_fw_wowlan[22710];
extern u32 array_length_mp_8188e_s_fw_wowlan;
#endif /*CONFIG_WOWLAN*/
#endif /* end of LOAD_FW_HEADER_FROM_DRIVER */
#endif /* end of CONFIG_SFW_SUPPORTED */

#endif

