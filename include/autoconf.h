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

/*
 * Public  General Config
 */
#define RTL871X_MODULE_NAME "88EU"
#define DRV_NAME "rtl8188eu"

/*
 * Internal  General Config
 */

#define CONFIG_AP_MODE

#define CONFIG_P2P

#define RTW_NOTCH_FILTER 0 /* 0:Disable, 1:Enable, */

#define CONFIG_BR_EXT		/*  Enable NAT2.5 support for STA mode interface with a L2 Bridge */
#ifdef CONFIG_BR_EXT
#define CONFIG_BR_EXT_BRNAME	"br0"
#endif	/*  CONFIG_BR_EXT */

/*
 * HAL  Related Config
 */

#define CONFIG_OUT_EP_WIFI_MODE	0

#define DISABLE_BB_RF	0

#define MP_DRIVER 0


/*
 * Outsource  Related Config
 */

#define RATE_ADAPTIVE_SUPPORT			1
#define POWER_TRAINING_ACTIVE			1

#define CONFIG_80211D

/*
 * Debug Related Config
 */
#define DBG	1

#define CONFIG_PROC_DEBUG
