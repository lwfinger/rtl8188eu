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
//***** temporarily flag *******

//for FPGA VERIFICATION config
#define RTL8188E_FPGA_true_PHY_VERIFICATION 0

//***** temporarily flag *******
/*
 * Public  General Config
 */
#define AUTOCONF_INCLUDED
#define RTL871X_MODULE_NAME "88EU"
#define DRV_NAME "rtl8188eu"

#define CONFIG_RTL8188E

#ifdef CONFIG_IOCTL_CFG80211
	#define CONFIG_CFG80211_FORCE_COMPATIBLE_2_6_37_UNDER
	#define CONFIG_SET_SCAN_DENY_TIMER
#endif

/*
 * Internal  General Config
 */


#define CONFIG_EMBEDDED_FWIMG

#define CONFIG_80211N_HT

#define CONFIG_SUPPORT_USB_INT

#define CONFIG_IPS
#define SUPPORT_HW_RFOFF_DETECTED

#define CONFIG_AP_MODE
#ifdef CONFIG_AP_MODE
	#ifdef CONFIG_INTERRUPT_BASED_TXBCN
	#define CONFIG_INTERRUPT_BASED_TXBCN_BCN_OK_ERR
	#endif

	#define CONFIG_NATIVEAP_MLME
#endif

#define CONFIG_P2P
#ifdef CONFIG_P2P

	#define CONFIG_P2P_REMOVE_GROUP_INFO

	#define CONFIG_P2P_PS
#endif

#define RTW_NOTCH_FILTER 0 /* 0:Disable, 1:Enable, */

#define CONFIG_BR_EXT_BRNAME	"br0"

/*
 * Interface  Related Config
 */

#define CONFIG_PREALLOC_RECV_SKB

/*
 * USB VENDOR REQ BUFFER ALLOCATION METHOD
 * if not set we'll use function local variable (stack memory)
 */
#define CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC

#define CONFIG_USB_VENDOR_REQ_MUTEX
#define CONFIG_VENDOR_REQ_RETRY

/*
 * HAL  Related Config
 */

#define RTL8188E_RX_PACKET_INCLUDE_CRC	0

#define SUPPORTED_BLOCK_IO


//#define CONFIG_ONLY_ONE_OUT_EP_TO_LOW	0

#define CONFIG_OUT_EP_WIFI_MODE	0

#define ENABLE_USB_DROP_INCORRECT_OUT	0


//#define RTL8192CU_ADHOC_WORKAROUND_SETTING

#define DISABLE_BB_RF	0

//#define RTL8191C_FPGA_NETWORKTYPE_ADHOC 0

#ifdef CONFIG_MP_INCLUDED
	#define MP_DRIVER 1
	#define CONFIG_MP_IWPRIV_SUPPORT
#else
	#define MP_DRIVER 0
#endif


/*
 * Platform  Related Config
 */
/*
 * Outsource  Related Config
 */

#define		RTL8192CE_SUPPORT				0
#define		RTL8192CU_SUPPORT			0
#define		RTL8192C_SUPPORT				(RTL8192CE_SUPPORT|RTL8192CU_SUPPORT)

#define		RTL8192DE_SUPPORT				0
#define		RTL8192DU_SUPPORT			0
#define		RTL8192D_SUPPORT				(RTL8192DE_SUPPORT|RTL8192DU_SUPPORT)

#define		RTL8723AU_SUPPORT				0
#define		RTL8723AS_SUPPORT				0
#define		RTL8723AE_SUPPORT				0
#define		RTL8723A_SUPPORT				(RTL8723AU_SUPPORT|RTL8723AS_SUPPORT|RTL8723AE_SUPPORT)

#define		RTL8723_FPGA_VERIFICATION		0

#define RTL8188EE_SUPPORT				0
#define RTL8188EU_SUPPORT				1
#define RTL8188ES_SUPPORT				0
#define RTL8188E_SUPPORT				(RTL8188EE_SUPPORT|RTL8188EU_SUPPORT|RTL8188ES_SUPPORT)
#define RTL8188E_FOR_TEST_CHIP			0
#define RATE_ADAPTIVE_SUPPORT			1
#define POWER_TRAINING_ACTIVE			1

#ifdef CONFIG_TX_EARLY_MODE
#define	RTL8188E_EARLY_MODE_PKT_NUM_10	0
#endif

#define CONFIG_80211D

/*
 * Debug Related Config
 */
#define DBG	1

#define CONFIG_PROC_DEBUG
