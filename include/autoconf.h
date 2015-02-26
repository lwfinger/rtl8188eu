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

#define CONFIG_EMBEDDED_FWIMG

 #define CONFIG_SUPPORT_USB_INT

	#define CONFIG_IOL

#define CONFIG_AP_MODE
#ifdef CONFIG_AP_MODE
	#define CONFIG_FIND_BEST_CHANNEL
#endif

#define CONFIG_P2P
#ifdef CONFIG_P2P
	//The CONFIG_WFD is for supporting the Wi-Fi display
	#define CONFIG_WFD

	#ifndef CONFIG_WIFI_TEST
		#define CONFIG_P2P_REMOVE_GROUP_INFO
	#endif

	#define CONFIG_P2P_PS
	#define P2P_OP_CHECK_SOCIAL_CH
#endif

#define CONFIG_SKB_COPY	//for amsdu

//#define CONFIG_LED
#ifdef CONFIG_LED
	#define CONFIG_SW_LED
#endif // CONFIG_LED

#ifdef CONFIG_IOL
	#define CONFIG_IOL_NEW_GENERATION
	#define CONFIG_IOL_READ_EFUSE_MAP
	#define CONFIG_IOL_EFUSE_PATCH
#endif


#define USB_INTERFERENCE_ISSUE // this should be checked in all usb interface
#define CONFIG_GLOBAL_UI_PID

#define CONFIG_LAYER2_ROAMING
#define CONFIG_LAYER2_ROAMING_RESUME
#define CONFIG_LONG_DELAY_ISSUE
#define CONFIG_NEW_SIGNAL_STAT_PROCESS
#define RTW_NOTCH_FILTER 0 /* 0:Disable, 1:Enable, */
#define CONFIG_DEAUTH_BEFORE_CONNECT

#define CONFIG_BR_EXT		// Enable NAT2.5 support for STA mode interface with a L2 Bridge
#ifdef CONFIG_BR_EXT
#define CONFIG_BR_EXT_BRNAME	"br0"
#endif	// CONFIG_BR_EXT

/*
 * Interface  Related Config
 */

#ifndef CONFIG_MINIMAL_MEMORY_USAGE
	#define CONFIG_USB_TX_AGGREGATION
	#define CONFIG_USB_RX_AGGREGATION
#endif

#define CONFIG_PREALLOC_RECV_SKB

/*
 * CONFIG_USE_USB_BUFFER_ALLOC_XX uses Linux USB Buffer alloc API and is for Linux platform only now!
 */
#ifdef CONFIG_USE_USB_BUFFER_ALLOC_RX
#undef CONFIG_PREALLOC_RECV_SKB
#endif

/*
 * USB VENDOR REQ BUFFER ALLOCATION METHOD
 * if not set we'll use function local variable (stack memory)
 */
#define CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC

/*
 * HAL  Related Config
 */

#define RTL8188E_RX_PACKET_INCLUDE_CRC	0

#define SUPPORTED_BLOCK_IO
#define CONFIG_REGULATORY_CTRL

#define CONFIG_OUT_EP_WIFI_MODE	0

#define ENABLE_USB_DROP_INCORRECT_OUT

#define DISABLE_BB_RF	0

#define MP_DRIVER 0


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
//#if (RTL8188E_SUPPORT==1)
#define RATE_ADAPTIVE_SUPPORT			1
#define POWER_TRAINING_ACTIVE			1

//#endif

#ifdef CONFIG_USB_TX_AGGREGATION
//#define	CONFIG_TX_EARLY_MODE
#endif

#ifdef CONFIG_TX_EARLY_MODE
#define	RTL8188E_EARLY_MODE_PKT_NUM_10	0
#endif

#define CONFIG_80211D

#define CONFIG_ATTEMPT_TO_FIX_AP_BEACON_ERROR

/*
 * Debug Related Config
 */
#define DBG	1

#define CONFIG_PROC_DEBUG

#define DBG_CONFIG_ERROR_DETECT
#define DBG_CONFIG_ERROR_RESET
