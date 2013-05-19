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
#ifndef _RTW_MP_H_
#define _RTW_MP_H_

#ifndef PLATFORM_WINDOWS
//	00 - Success
//	11 - Error
#define STATUS_SUCCESS				(0x00000000L)
#define STATUS_PENDING				(0x00000103L)

#define STATUS_UNSUCCESSFUL			(0xC0000001L)
#define STATUS_INSUFFICIENT_RESOURCES		(0xC000009AL)
#define STATUS_NOT_SUPPORTED			(0xC00000BBL)

#define NDIS_STATUS_SUCCESS			((NDIS_STATUS)STATUS_SUCCESS)
#define NDIS_STATUS_PENDING			((NDIS_STATUS)STATUS_PENDING)
#define NDIS_STATUS_NOT_RECOGNIZED		((NDIS_STATUS)0x00010001L)
#define NDIS_STATUS_NOT_COPIED			((NDIS_STATUS)0x00010002L)
#define NDIS_STATUS_NOT_ACCEPTED		((NDIS_STATUS)0x00010003L)
#define NDIS_STATUS_CALL_ACTIVE			((NDIS_STATUS)0x00010007L)

#define NDIS_STATUS_FAILURE			((NDIS_STATUS)STATUS_UNSUCCESSFUL)
#define NDIS_STATUS_RESOURCES			((NDIS_STATUS)STATUS_INSUFFICIENT_RESOURCES)
#define NDIS_STATUS_CLOSING			((NDIS_STATUS)0xC0010002L)
#define NDIS_STATUS_BAD_VERSION			((NDIS_STATUS)0xC0010004L)
#define NDIS_STATUS_BAD_CHARACTERISTICS		((NDIS_STATUS)0xC0010005L)
#define NDIS_STATUS_ADAPTER_NOT_FOUND		((NDIS_STATUS)0xC0010006L)
#define NDIS_STATUS_OPEN_FAILED			((NDIS_STATUS)0xC0010007L)
#define NDIS_STATUS_DEVICE_FAILED		((NDIS_STATUS)0xC0010008L)
#define NDIS_STATUS_MULTICAST_FULL		((NDIS_STATUS)0xC0010009L)
#define NDIS_STATUS_MULTICAST_EXISTS		((NDIS_STATUS)0xC001000AL)
#define NDIS_STATUS_MULTICAST_NOT_FOUND		((NDIS_STATUS)0xC001000BL)
#define NDIS_STATUS_REQUEST_ABORTED		((NDIS_STATUS)0xC001000CL)
#define NDIS_STATUS_RESET_IN_PROGRESS		((NDIS_STATUS)0xC001000DL)
#define NDIS_STATUS_CLOSING_INDICATING		((NDIS_STATUS)0xC001000EL)
#define NDIS_STATUS_NOT_SUPPORTED		((NDIS_STATUS)STATUS_NOT_SUPPORTED)
#define NDIS_STATUS_INVALID_PACKET		((NDIS_STATUS)0xC001000FL)
#define NDIS_STATUS_OPEN_LIST_FULL		((NDIS_STATUS)0xC0010010L)
#define NDIS_STATUS_ADAPTER_NOT_READY		((NDIS_STATUS)0xC0010011L)
#define NDIS_STATUS_ADAPTER_NOT_OPEN		((NDIS_STATUS)0xC0010012L)
#define NDIS_STATUS_NOT_INDICATING		((NDIS_STATUS)0xC0010013L)
#define NDIS_STATUS_INVALID_LENGTH		((NDIS_STATUS)0xC0010014L)
#define NDIS_STATUS_INVALID_DATA		((NDIS_STATUS)0xC0010015L)
#define NDIS_STATUS_BUFFER_TOO_SHORT		((NDIS_STATUS)0xC0010016L)
#define NDIS_STATUS_INVALID_OID			((NDIS_STATUS)0xC0010017L)
#define NDIS_STATUS_ADAPTER_REMOVED		((NDIS_STATUS)0xC0010018L)
#define NDIS_STATUS_UNSUPPORTED_MEDIA		((NDIS_STATUS)0xC0010019L)
#define NDIS_STATUS_GROUP_ADDRESS_IN_USE	((NDIS_STATUS)0xC001001AL)
#define NDIS_STATUS_FILE_NOT_FOUND		((NDIS_STATUS)0xC001001BL)
#define NDIS_STATUS_ERROR_READING_FILE		((NDIS_STATUS)0xC001001CL)
#define NDIS_STATUS_ALREADY_MAPPED		((NDIS_STATUS)0xC001001DL)
#define NDIS_STATUS_RESOURCE_CONFLICT		((NDIS_STATUS)0xC001001EL)
#define NDIS_STATUS_NO_CABLE			((NDIS_STATUS)0xC001001FL)

#define NDIS_STATUS_INVALID_SAP			((NDIS_STATUS)0xC0010020L)
#define NDIS_STATUS_SAP_IN_USE			((NDIS_STATUS)0xC0010021L)
#define NDIS_STATUS_INVALID_ADDRESS		((NDIS_STATUS)0xC0010022L)
#define NDIS_STATUS_VC_NOT_ACTIVATED		((NDIS_STATUS)0xC0010023L)
#define NDIS_STATUS_DEST_OUT_OF_ORDER		((NDIS_STATUS)0xC0010024L)  // cause 27
#define NDIS_STATUS_VC_NOT_AVAILABLE		((NDIS_STATUS)0xC0010025L)  // cause 35,45
#define NDIS_STATUS_CELLRATE_NOT_AVAILABLE	((NDIS_STATUS)0xC0010026L)  // cause 37
#define NDIS_STATUS_INCOMPATABLE_QOS		((NDIS_STATUS)0xC0010027L)  // cause 49
#define NDIS_STATUS_AAL_PARAMS_UNSUPPORTED	((NDIS_STATUS)0xC0010028L)  // cause 93
#define NDIS_STATUS_NO_ROUTE_TO_DESTINATION	((NDIS_STATUS)0xC0010029L)  // cause 3
#endif /* #ifndef PLATFORM_WINDOWS */

typedef enum _ANTENNA_PATH{
		ANTENNA_NONE	= 0x00,
		ANTENNA_D		,
		ANTENNA_C		,
		ANTENNA_CD		,
		ANTENNA_B		,
		ANTENNA_BD		,
		ANTENNA_BC		,
		ANTENNA_BCD	,
		ANTENNA_A		,
		ANTENNA_AD		,
		ANTENNA_AC		,
		ANTENNA_ACD	,
		ANTENNA_AB		,
		ANTENNA_ABD	,
		ANTENNA_ABC	,
		ANTENNA_ABCD
} ANTENNA_PATH;


#define MAX_MP_XMITBUF_SZ	2048
#define NR_MP_XMITFRAME		8

struct mp_xmit_frame
{
	_list	list;

	struct pkt_attrib attrib;

	_pkt *pkt;

	int frame_tag;

	_adapter *padapter;

#ifdef CONFIG_USB_HCI

	//insert urb, irp, and irpcnt info below...
	//max frag_cnt = 8

	u8 *mem_addr;
	u32 sz[8];

#if defined(PLATFORM_OS_XP) || defined(PLATFORM_LINUX)
	PURB pxmit_urb[8];
#endif

#ifdef PLATFORM_OS_XP
	PIRP pxmit_irp[8];
#endif

	u8 bpending[8];
	sint ac_tag[8];
	sint last[8];
	uint irpcnt;
	uint fragcnt;
#endif /* CONFIG_USB_HCI */

	uint mem[(MAX_MP_XMITBUF_SZ >> 2)];
};

struct mp_wiparam
{
	u32 bcompleted;
	u32 act_type;
	u32 io_offset;
	u32 io_value;
};

typedef void(*wi_act_func)(void* padapter);

#ifdef PLATFORM_WINDOWS
struct mp_wi_cntx
{
	u8 bmpdrv_unload;

	// Work Item
	NDIS_WORK_ITEM mp_wi;
	NDIS_EVENT mp_wi_evt;
	_lock mp_wi_lock;
	u8 bmp_wi_progress;
	wi_act_func curractfunc;
	// Variable needed in each implementation of CurrActFunc.
	struct mp_wiparam param;
};
#endif

struct mp_tx
{
	u8 stop;
	u32 count, sended;
	u8 payload;
	struct pkt_attrib attrib;
	struct tx_desc desc;
	u8 *pallocated_buf;
	u8 *buf;
	u32 buf_size, write_size;
	_thread_hdl_ PktTxThread;
};

#if defined(CONFIG_RTL8192C) || defined(CONFIG_RTL8192D) || defined(CONFIG_RTL8723A) || defined(CONFIG_RTL8188E)
#ifdef CONFIG_RTL8192C
#include <Hal8192CPhyCfg.h>
#endif
#ifdef CONFIG_RTL8192D
#include <Hal8192DPhyCfg.h>
#endif
#ifdef CONFIG_RTL8723A
#include <Hal8723APhyCfg.h>
#endif
#ifdef CONFIG_RTL8188E
#include <Hal8188EPhyCfg.h>
#endif

#define MP_MAX_LINES		1000
#define MP_MAX_LINES_BYTES	256
#define u1Byte u8
#define s1Byte s8
#define u4Byte u32
#define s4Byte s32
#define u1Byte		u8
#define pu1Byte			u8*

#define u2Byte		u16
#define pu2Byte			u16*

#define u4Byte		u32
#define pu4Byte			u32*

#define u8Byte		u64
#define pu8Byte			u64*

#define s1Byte		s8
#define ps1Byte			s8*

#define s2Byte		s16
#define ps2Byte			s16*

#define s4Byte		s32
#define ps4Byte			s32*

#define s8Byte		s64
#define ps8Byte			s64*

#define UCHAR u8
#define USHORT u16
#define UINT u32
#define ULONG u32
#define PULONG u32*



typedef void (*MPT_WORK_ITEM_HANDLER)(IN void * Adapter);
typedef struct _MPT_CONTEXT
{
	// Indicate if we have started Mass Production Test.
	bool			bMassProdTest;

	// Indicate if the driver is unloading or unloaded.
	bool			bMptDrvUnload;

	_sema			MPh2c_Sema;
	_timer			MPh2c_timeout_timer;
// Event used to sync H2c for BT control

	bool		MptH2cRspEvent;
	bool		MptBtC2hEvent;
	bool		bMPh2c_timeout;

	/* 8190 PCI does not support NDIS_WORK_ITEM. */
	// Work Item for Mass Production Test.
	//NDIS_WORK_ITEM	MptWorkItem;
//	RT_WORK_ITEM		MptWorkItem;
	// Event used to sync the case unloading driver and MptWorkItem is still in progress.
//	NDIS_EVENT		MptWorkItemEvent;
	// To protect the following variables.
//	NDIS_SPIN_LOCK		MptWorkItemSpinLock;
	// Indicate a MptWorkItem is scheduled and not yet finished.
	bool			bMptWorkItemInProgress;
	// An instance which implements function and context of MptWorkItem.
	MPT_WORK_ITEM_HANDLER	CurrMptAct;

	// 1=Start, 0=Stop from UI.
	ULONG			MptTestStart;
	// _TEST_MODE, defined in MPT_Req2.h
	ULONG			MptTestItem;
	// Variable needed in each implementation of CurrMptAct.
	ULONG			MptActType;	// Type of action performed in CurrMptAct.
	// The Offset of IO operation is depend of MptActType.
	ULONG			MptIoOffset;
	// The Value of IO operation is depend of MptActType.
	ULONG			MptIoValue;
	// The RfPath of IO operation is depend of MptActType.
	ULONG			MptRfPath;

	WIRELESS_MODE		MptWirelessModeToSw;	// Wireless mode to switch.
	u8			MptChannelToSw;		// Channel to switch.
	u8			MptInitGainToSet;	// Initial gain to set.
	//ULONG			bMptAntennaA;		// TRUE if we want to use antenna A.
	ULONG			MptBandWidth;		// bandwidth to switch.
	ULONG			MptRateIndex;		// rate index.
	// Register value kept for Single Carrier Tx test.
	u8			btMpCckTxPower;
	// Register value kept for Single Carrier Tx test.
	u8			btMpOfdmTxPower;
	// For MP Tx Power index
	u8			TxPwrLevel[2];	// rf-A, rf-B

	// Content of RCR Regsiter for Mass Production Test.
	ULONG			MptRCR;
	// TRUE if we only receive packets with specific pattern.
	bool			bMptFilterPattern;
	// Rx OK count, statistics used in Mass Production Test.
	ULONG			MptRxOkCnt;
	// Rx CRC32 error count, statistics used in Mass Production Test.
	ULONG			MptRxCrcErrCnt;

	bool			bCckContTx;	// TRUE if we are in CCK Continuous Tx test.
	bool			bOfdmContTx;	// TRUE if we are in OFDM Continuous Tx test.
	bool			bStartContTx;	// TRUE if we have start Continuous Tx test.
	// TRUE if we are in Single Carrier Tx test.
	bool			bSingleCarrier;
	// TRUE if we are in Carrier Suppression Tx Test.
	bool			bCarrierSuppression;
	//TRUE if we are in Single Tone Tx test.
	bool			bSingleTone;

	// ACK counter asked by K.Y..
	bool			bMptEnableAckCounter;
	ULONG			MptAckCounter;

	// SD3 Willis For 8192S to save 1T/2T RF table for ACUT	Only fro ACUT delete later ~~~!
	//s1Byte		BufOfLines[2][MAX_LINES_HWCONFIG_TXT][MAX_BYTES_LINE_HWCONFIG_TXT];
	//s1Byte			BufOfLines[2][MP_MAX_LINES][MP_MAX_LINES_BYTES];
	//s4Byte			RfReadLine[2];

	u8		APK_bound[2];	//for APK	path A/path B
	bool		bMptIndexEven;

	u8		backup0xc50;
	u8		backup0xc58;
	u8		backup0xc30;
	u8		backup0x52_RF_A;
	u8		backup0x52_RF_B;

	u1Byte			h2cReqNum;
	u1Byte			c2hBuf[20];

    u1Byte          btInBuf[100];
	ULONG			mptOutLen;
    u1Byte          mptOutBuf[100];

}MPT_CONTEXT, *PMPT_CONTEXT;
#endif
//#endif

//#define RTPRIV_IOCTL_MP					( SIOCIWFIRSTPRIV + 0x17)
enum {
	WRITE_REG = 1,
	READ_REG,
	WRITE_RF,
	READ_RF,
	MP_START,
	MP_STOP,
	MP_RATE,
	MP_CHANNEL,
	MP_BANDWIDTH,
	MP_TXPOWER,
	MP_ANT_TX,
	MP_ANT_RX,
	MP_CTX,
	MP_QUERY,
	MP_ARX,
	MP_PSD,
	MP_PWRTRK,
	MP_THER,
	MP_IOCTL,
	EFUSE_GET,
	EFUSE_SET,
	MP_RESET_STATS,
	MP_DUMP,
	MP_PHYPARA,
	MP_SetRFPathSwh,
	MP_QueryDrvStats,
	MP_SetBT,
	CTA_TEST,
	MP_NULL,
};

struct mp_priv
{
	_adapter *papdater;

	//Testing Flag
	u32 mode;//0 for normal type packet, 1 for loopback packet (16bytes TXCMD)

	u32 prev_fw_state;

	//OID cmd handler
	struct mp_wiparam workparam;
//	u8 act_in_progress;

	//Tx Section
	u8 TID;
	u32 tx_pktcount;
	struct mp_tx tx;

	//Rx Section
	u32 rx_pktcount;
	u32 rx_crcerrpktcount;
	u32 rx_pktloss;

	struct recv_stat rxstat;

	//RF/BB relative
	u8 channel;
	u8 bandwidth;
	u8 prime_channel_offset;
	u8 txpoweridx;
	u8 txpoweridx_b;
	u8 rateidx;
	u32 preamble;
//	u8 modem;
	u32 CrystalCap;
//	u32 curr_crystalcap;

	u16 antenna_tx;
	u16 antenna_rx;
//	u8 curr_rfpath;

	u8 check_mp_pkt;

	u8 bSetTxPower;
//	uint ForcedDataRate;

	struct wlan_network mp_network;
	NDIS_802_11_MAC_ADDRESS network_macaddr;

#ifdef PLATFORM_WINDOWS
	u32 rx_testcnt;
	u32 rx_testcnt1;
	u32 rx_testcnt2;
	u32 tx_testcnt;
	u32 tx_testcnt1;

	struct mp_wi_cntx wi_cntx;

	u8 h2c_result;
	u8 h2c_seqnum;
	u16 h2c_cmdcode;
	u8 h2c_resp_parambuf[512];
	_lock h2c_lock;
	_lock wkitm_lock;
	u32 h2c_cmdcnt;
	NDIS_EVENT h2c_cmd_evt;
	NDIS_EVENT c2h_set;
	NDIS_EVENT h2c_clr;
	NDIS_EVENT cpwm_int;

	NDIS_EVENT scsir_full_evt;
	NDIS_EVENT scsiw_empty_evt;
#endif

	u8 *pallocated_mp_xmitframe_buf;
	u8 *pmp_xmtframe_buf;
	_queue free_mp_xmitqueue;
	u32 free_mp_xmitframe_cnt;

	MPT_CONTEXT MptCtx;
};

typedef struct _IOCMD_STRUCT_ {
	u8	cmdclass;
	u16	value;
	u8	index;
}IOCMD_STRUCT;

struct rf_reg_param {
	u32 path;
	u32 offset;
	u32 value;
};

struct bb_reg_param {
	u32 offset;
	u32 value;
};
//=======================================================================

#define LOWER	_TRUE
#define RAISE	_FALSE

/* Hardware Registers */
#define BB_REG_BASE_ADDR		0x800

/* MP variables */
typedef enum _MP_MODE_ {
	MP_OFF,
	MP_ON,
	MP_ERR,
	MP_CONTINUOUS_TX,
	MP_SINGLE_CARRIER_TX,
	MP_CARRIER_SUPPRISSION_TX,
	MP_SINGLE_TONE_TX,
	MP_PACKET_TX,
	MP_PACKET_RX
} MP_MODE;


#define MAX_RF_PATH_NUMS	RF_PATH_MAX


extern u8 mpdatarate[NumRates];

/* MP set force data rate base on the definition. */
typedef enum _MPT_RATE_INDEX
{
	/* CCK rate. */
	MPT_RATE_1M,	/* 0 */
	MPT_RATE_2M,
	MPT_RATE_55M,
	MPT_RATE_11M,	/* 3 */

	/* OFDM rate. */
	MPT_RATE_6M,	/* 4 */
	MPT_RATE_9M,
	MPT_RATE_12M,
	MPT_RATE_18M,
	MPT_RATE_24M,
	MPT_RATE_36M,
	MPT_RATE_48M,
	MPT_RATE_54M,	/* 11 */

	/* HT rate. */
	MPT_RATE_MCS0,	/* 12 */
	MPT_RATE_MCS1,
	MPT_RATE_MCS2,
	MPT_RATE_MCS3,
	MPT_RATE_MCS4,
	MPT_RATE_MCS5,
	MPT_RATE_MCS6,
	MPT_RATE_MCS7,	/* 19 */
	MPT_RATE_MCS8,
	MPT_RATE_MCS9,
	MPT_RATE_MCS10,
	MPT_RATE_MCS11,
	MPT_RATE_MCS12,
	MPT_RATE_MCS13,
	MPT_RATE_MCS14,
	MPT_RATE_MCS15,	/* 27 */
	MPT_RATE_LAST
}MPT_RATE_E, *PMPT_RATE_E;

#define MAX_TX_PWR_INDEX_N_MODE 64	// 0x3F

typedef enum _POWER_MODE_ {
	POWER_LOW = 0,
	POWER_NORMAL
}POWER_MODE;


#define RX_PKT_BROADCAST	1
#define RX_PKT_DEST_ADDR	2
#define RX_PKT_PHY_MATCH	3

typedef enum _ENCRY_CTRL_STATE_ {
	HW_CONTROL,		//hw encryption& decryption
	SW_CONTROL,		//sw encryption& decryption
	HW_ENCRY_SW_DECRY,	//hw encryption & sw decryption
	SW_ENCRY_HW_DECRY	//sw encryption & hw decryption
}ENCRY_CTRL_STATE;


//=======================================================================
//extern struct mp_xmit_frame *alloc_mp_xmitframe(struct mp_priv *pmp_priv);
//extern int free_mp_xmitframe(struct xmit_priv *pxmitpriv, struct mp_xmit_frame *pmp_xmitframe);

extern s32 init_mp_priv(PADAPTER padapter);
extern void free_mp_priv(struct mp_priv *pmp_priv);
extern s32 MPT_InitializeAdapter(PADAPTER padapter, u8 Channel);
extern void MPT_DeInitAdapter(PADAPTER padapter);
extern s32 mp_start_test(PADAPTER padapter);
extern void mp_stop_test(PADAPTER padapter);

//=======================================================================

extern u32 _read_rfreg(PADAPTER padapter, u8 rfpath, u32 addr, u32 bitmask);
extern void _write_rfreg(PADAPTER padapter, u8 rfpath, u32 addr, u32 bitmask, u32 val);

extern u32 read_macreg(_adapter *padapter, u32 addr, u32 sz);
extern void write_macreg(_adapter *padapter, u32 addr, u32 val, u32 sz);
extern u32 read_bbreg(_adapter *padapter, u32 addr, u32 bitmask);
extern void write_bbreg(_adapter *padapter, u32 addr, u32 bitmask, u32 val);
extern u32 read_rfreg(PADAPTER padapter, u8 rfpath, u32 addr);
extern void write_rfreg(PADAPTER padapter, u8 rfpath, u32 addr, u32 val);

extern void	SetChannel(PADAPTER pAdapter);
extern void	SetBandwidth(PADAPTER pAdapter);
extern void	SetTxPower(PADAPTER pAdapter);
extern void	SetAntennaPathPower(PADAPTER pAdapter);
//extern void	SetTxAGCOffset(PADAPTER pAdapter, u32 ulTxAGCOffset);
extern void	SetDataRate(PADAPTER pAdapter);

extern void	SetAntenna(PADAPTER pAdapter);

//extern void	SetCrystalCap(PADAPTER pAdapter);

extern s32	SetThermalMeter(PADAPTER pAdapter, u8 target_ther);
extern void	GetThermalMeter(PADAPTER pAdapter, u8 *value);

extern void	SetContinuousTx(PADAPTER pAdapter, u8 bStart);
extern void	SetSingleCarrierTx(PADAPTER pAdapter, u8 bStart);
extern void	SetSingleToneTx(PADAPTER pAdapter, u8 bStart);
extern void	SetCarrierSuppressionTx(PADAPTER pAdapter, u8 bStart);
extern void PhySetTxPowerLevel(PADAPTER pAdapter);

extern void	fill_txdesc_for_mp(PADAPTER padapter, struct tx_desc *ptxdesc);
extern void	SetPacketTx(PADAPTER padapter);
extern void	SetPacketRx(PADAPTER pAdapter, u8 bStartRx);

extern void	ResetPhyRxPktCount(PADAPTER pAdapter);
extern u32	GetPhyRxPktReceived(PADAPTER pAdapter);
extern u32	GetPhyRxPktCRC32Error(PADAPTER pAdapter);

extern s32	SetPowerTracking(PADAPTER padapter, u8 enable);
extern void	GetPowerTracking(PADAPTER padapter, u8 *enable);

extern u32	mp_query_psd(PADAPTER pAdapter, u8 *data);


extern void Hal_SetAntenna(PADAPTER pAdapter);
extern void Hal_SetBandwidth(PADAPTER pAdapter);

extern void Hal_SetTxPower(PADAPTER pAdapter);
extern void Hal_SetCarrierSuppressionTx(PADAPTER pAdapter, u8 bStart);
extern void Hal_SetSingleToneTx ( PADAPTER pAdapter , u8 bStart );
extern void Hal_SetSingleCarrierTx (PADAPTER pAdapter, u8 bStart);
extern void Hal_SetContinuousTx (PADAPTER pAdapter, u8 bStart);
extern void Hal_SetBandwidth(PADAPTER pAdapter);

extern void Hal_SetDataRate(PADAPTER pAdapter);
extern void Hal_SetChannel(PADAPTER pAdapter);
extern void Hal_SetAntennaPathPower(PADAPTER pAdapter);
extern s32 Hal_SetThermalMeter(PADAPTER pAdapter, u8 target_ther);
extern s32 Hal_SetPowerTracking(PADAPTER padapter, u8 enable);
extern void Hal_GetPowerTracking(PADAPTER padapter, u8 * enable);
extern void Hal_GetThermalMeter(PADAPTER pAdapter, u8 *value);
extern void Hal_mpt_SwitchRfSetting(PADAPTER pAdapter);
extern void Hal_MPT_CCKTxPowerAdjust(PADAPTER Adapter, bool bInCH14);
extern void Hal_MPT_CCKTxPowerAdjustbyIndex(PADAPTER pAdapter, bool beven);
extern void Hal_SetCCKTxPower(PADAPTER pAdapter, u8 * TxPower);
extern void Hal_SetOFDMTxPower(PADAPTER pAdapter, u8 * TxPower);
extern void Hal_TriggerRFThermalMeter(PADAPTER pAdapter);
extern u8 Hal_ReadRFThermalMeter(PADAPTER pAdapter);
extern void Hal_SetCCKContinuousTx(PADAPTER pAdapter, u8 bStart);
extern void Hal_SetOFDMContinuousTx(PADAPTER pAdapter, u8 bStart);
extern void Hal_ProSetCrystalCap (PADAPTER pAdapter , u32 CrystalCapVal);
extern void _rtw_mp_xmit_priv(struct xmit_priv *pxmitpriv);
extern void MP_PHY_SetRFPathSwitch(PADAPTER pAdapter ,bool bMain);

#endif //_RTW_MP_H_
