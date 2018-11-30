#ifndef	__HALBTC_OUT_SRC_H__
#define __HALBTC_OUT_SRC_H__


#define		BTC_COEX_OFFLOAD			0
#define		BTC_TMP_BUF_SHORT		20

extern u8	gl_btc_trace_buf[];
#define		BTC_SPRINTF			rsprintf
#define		BTC_TRACE(_MSG_)\
do {\
	if (GLBtcDbgType[COMP_COEX] & BIT(DBG_LOUD)) {\
		RTW_INFO("%s", _MSG_);\
	} \
} while (0)
#define		BT_PrintData(adapter, _MSG_, len, data)	RTW_DBG_DUMP((_MSG_), data, len)


#define		NORMAL_EXEC					FALSE
#define		FORCE_EXEC						TRUE

#define		BTC_RF_OFF					0x0
#define		BTC_RF_ON					0x1

#define		BTC_RF_A					0x0
#define		BTC_RF_B					0x1
#define		BTC_RF_C					0x2
#define		BTC_RF_D					0x3

#define		BTC_SMSP				SINGLEMAC_SINGLEPHY
#define		BTC_DMDP				DUALMAC_DUALPHY
#define		BTC_DMSP				DUALMAC_SINGLEPHY
#define		BTC_MP_UNKNOWN		0xff

#define		BT_COEX_ANT_TYPE_PG			0
#define		BT_COEX_ANT_TYPE_ANTDIV		1
#define		BT_COEX_ANT_TYPE_DETECTED	2

#define		BTC_MIMO_PS_STATIC			0	/* 1ss */
#define		BTC_MIMO_PS_DYNAMIC			1	/* 2ss */

#define		BTC_RATE_DISABLE			0
#define		BTC_RATE_ENABLE				1

/* single Antenna definition */
#define		BTC_ANT_PATH_WIFI			0
#define		BTC_ANT_PATH_BT				1
#define		BTC_ANT_PATH_PTA			2
#define		BTC_ANT_PATH_WIFI5G			3
#define		BTC_ANT_PATH_AUTO			4
/* dual Antenna definition */
#define		BTC_ANT_WIFI_AT_MA	0
#define		BTC_ANT_WIFI_AT_AUX			1
#define		BTC_ANT_WIFI_AT_DIVERSITY	2
/* coupler Antenna definition */
#define		BTC_ANT_WIFI_AT_CPL_MA0
#define		BTC_ANT_WIFI_AT_CPL_AUX		1

typedef enum _BTC_POWERSAVE_TYPE {
	BTC_PS_WIFI_NATIVE			= 0,	/* wifi original power save behavior */
	BTC_PS_LPS_ON				= 1,
	BTC_PS_LPS_OFF				= 2,
	BTC_PS_MAX
} BTC_POWERSAVE_TYPE, *PBTC_POWERSAVE_TYPE;

typedef enum _BTC_BT_REG_TYPE {
	BTC_BT_REG_RF						= 0,
	BTC_BT_REG_MODEM					= 1,
	BTC_BT_REG_BLUEWIZE					= 2,
	BTC_BT_REG_VENDOR					= 3,
	BTC_BT_REG_LE						= 4,
	BTC_BT_REG_MAX
} BTC_BT_REG_TYPE, *PBTC_BT_REG_TYPE;

typedef enum _BTC_CHIP_INTERFACE {
	BTC_INTF_UNKNOWN	= 0,
	BTC_INTF_PCI			= 1,
	BTC_INTF_USB			= 2,
	BTC_INTF_SDIO		= 3,
	BTC_INTF_MAX
} BTC_CHIP_INTERFACE, *PBTC_CHIP_INTERFACE;

typedef enum _BTC_CHIP_TYPE {
	BTC_CHIP_UNDEF		= 0,
	BTC_CHIP_CSR_BC4		= 1,
	BTC_CHIP_CSR_BC8		= 2,
	BTC_CHIP_RTL8723A		= 3,
	BTC_CHIP_RTL8821		= 4,
	BTC_CHIP_RTL8723B		= 5,
	BTC_CHIP_MAX
} BTC_CHIP_TYPE, *PBTC_CHIP_TYPE;

/* following is for wifi link status */
#define		WIFI_STA_CONNECTED				BIT0
#define		WIFI_AP_CONNECTED				BIT1
#define		WIFI_HS_CONNECTED				BIT2
#define		WIFI_P2P_GO_CONNECTED			BIT3
#define		WIFI_P2P_GC_CONNECTED			BIT4

/* following is for command line utility */
#define	CL_SPRINTF	rsprintf
#define	CL_PRINTF	DCMD_Printf

struct btc_board_info {
	/* The following is some board information */
	u8				bt_chip_type;
	u8				pg_ant_num;	/* pg ant number */
	u8				btdm_ant_num;	/* ant number for btdm */
	u8				btdm_ant_num_by_ant_det;	/* ant number for btdm after antenna detection */
	u8				btdm_ant_pos;		/* Bryant Add to indicate Antenna Position for (pg_ant_num = 2) && (btdm_ant_num =1)  (DPDT+1Ant case) */
	u8				single_ant_path;	/* current used for 8723b only, 1=>s0,  0=>s1 */
	bool			tfbga_package;    /* for Antenna detect threshold */
	bool			btdm_ant_det_finish;
	bool			btdm_ant_det_already_init_phydm;
	u8				ant_type;
	u8				rfe_type;
	u8				ant_div_cfg;
	bool			btdm_ant_det_complete_fail;
	u8				ant_det_result;
	bool			ant_det_result_five_complete;
	u32				antdetval;
};

typedef enum _BTC_DBG_OPCODE {
	BTC_DBG_SET_COEX_NORMAL				= 0x0,
	BTC_DBG_SET_COEX_WIFI_ONLY				= 0x1,
	BTC_DBG_SET_COEX_BT_ONLY				= 0x2,
	BTC_DBG_SET_COEX_DEC_BT_PWR				= 0x3,
	BTC_DBG_SET_COEX_BT_AFH_MAP				= 0x4,
	BTC_DBG_SET_COEX_BT_IGNORE_WLAN_ACT		= 0x5,
	BTC_DBG_SET_COEX_MANUAL_CTRL				= 0x6,
	BTC_DBG_MAX
} BTC_DBG_OPCODE, *PBTC_DBG_OPCODE;

typedef enum _BTC_RSSI_STATE {
	BTC_RSSI_STATE_HIGH						= 0x0,
	BTC_RSSI_STATE_MEDIUM					= 0x1,
	BTC_RSSI_STATE_LOW						= 0x2,
	BTC_RSSI_STATE_STAY_HIGH					= 0x3,
	BTC_RSSI_STATE_STAY_MEDIUM				= 0x4,
	BTC_RSSI_STATE_STAY_LOW					= 0x5,
	BTC_RSSI_MAX
} BTC_RSSI_STATE, *PBTC_RSSI_STATE;
#define	BTC_RSSI_HIGH(_rssi_)	((_rssi_ == BTC_RSSI_STATE_HIGH || _rssi_ == BTC_RSSI_STATE_STAY_HIGH) ? TRUE:FALSE)
#define	BTC_RSSI_MEDIUM(_rssi_)	((_rssi_ == BTC_RSSI_STATE_MEDIUM || _rssi_ == BTC_RSSI_STATE_STAY_MEDIUM) ? TRUE:FALSE)
#define	BTC_RSSI_LOW(_rssi_)	((_rssi_ == BTC_RSSI_STATE_LOW || _rssi_ == BTC_RSSI_STATE_STAY_LOW) ? TRUE:FALSE)

typedef enum _BTC_WIFI_ROLE {
	BTC_ROLE_STATION						= 0x0,
	BTC_ROLE_AP								= 0x1,
	BTC_ROLE_IBSS							= 0x2,
	BTC_ROLE_HS_MODE						= 0x3,
	BTC_ROLE_MAX
} BTC_WIFI_ROLE, *PBTC_WIFI_ROLE;

typedef enum _BTC_WIRELESS_FREQ {
	BTC_FREQ_2_4G					= 0x0,
	BTC_FREQ_5G						= 0x1,
	BTC_FREQ_MAX
} BTC_WIRELESS_FREQ, *PBTC_WIRELESS_FREQ;

typedef enum _BTC_WIFI_BW_MODE {
	BTC_WIFI_BW_LEGACY					= 0x0,
	BTC_WIFI_BW_HT20					= 0x1,
	BTC_WIFI_BW_HT40					= 0x2,
	BTC_WIFI_BW_HT80					= 0x3,
	BTC_WIFI_BW_HT160					= 0x4,
	BTC_WIFI_BW_MAX
} BTC_WIFI_BW_MODE, *PBTC_WIFI_BW_MODE;

typedef enum _BTC_WIFI_TRAFFIC_DIR {
	BTC_WIFI_TRAFFIC_TX					= 0x0,
	BTC_WIFI_TRAFFIC_RX					= 0x1,
	BTC_WIFI_TRAFFIC_MAX
} BTC_WIFI_TRAFFIC_DIR, *PBTC_WIFI_TRAFFIC_DIR;

typedef enum _BTC_WIFI_PNP {
	BTC_WIFI_PNP_WAKE_UP					= 0x0,
	BTC_WIFI_PNP_SLEEP						= 0x1,
	BTC_WIFI_PNP_SLEEP_KEEP_ANT				= 0x2,
	BTC_WIFI_PNP_MAX
} BTC_WIFI_PNP, *PBTC_WIFI_PNP;

typedef enum _BTC_IOT_PEER {
	BTC_IOT_PEER_UNKNOWN = 0,
	BTC_IOT_PEER_REALTEK = 1,
	BTC_IOT_PEER_REALTEK_92SE = 2,
	BTC_IOT_PEER_BROADCOM = 3,
	BTC_IOT_PEER_RALINK = 4,
	BTC_IOT_PEER_ATHEROS = 5,
	BTC_IOT_PEER_CISCO = 6,
	BTC_IOT_PEER_MERU = 7,
	BTC_IOT_PEER_MARVELL = 8,
	BTC_IOT_PEER_REALTEK_SOFTAP = 9, /* peer is RealTek SOFT_AP, by Bohn, 2009.12.17 */
	BTC_IOT_PEER_SELF_SOFTAP = 10, /* Self is SoftAP */
	BTC_IOT_PEER_AIRGO = 11,
	BTC_IOT_PEER_INTEL				= 12,
	BTC_IOT_PEER_RTK_APCLIENT		= 13,
	BTC_IOT_PEER_REALTEK_81XX		= 14,
	BTC_IOT_PEER_REALTEK_WOW		= 15,
	BTC_IOT_PEER_REALTEK_JAGUAR_BCUTAP = 16,
	BTC_IOT_PEER_REALTEK_JAGUAR_CCUTAP = 17,
	BTC_IOT_PEER_MAX,
} BTC_IOT_PEER, *PBTC_IOT_PEER;

/* for 8723b-d cut large current issue */
typedef enum _BTC_WIFI_COEX_STATE {
	BTC_WIFI_STAT_INIT,
	BTC_WIFI_STAT_IQK,
	BTC_WIFI_STAT_NORMAL_OFF,
	BTC_WIFI_STAT_MP_OFF,
	BTC_WIFI_STAT_NORMAL,
	BTC_WIFI_STAT_ANT_DIV,
	BTC_WIFI_STAT_MAX
} BTC_WIFI_COEX_STATE, *PBTC_WIFI_COEX_STATE;

typedef enum _BTC_ANT_TYPE {
	BTC_ANT_TYPE_0,
	BTC_ANT_TYPE_1,
	BTC_ANT_TYPE_2,
	BTC_ANT_TYPE_3,
	BTC_ANT_TYPE_4,
	BTC_ANT_TYPE_MAX
} BTC_ANT_TYPE, *PBTC_ANT_TYPE;

typedef enum _BTC_VENDOR {
	BTC_VENDOR_LENOVO,
	BTC_VENDOR_ASUS,
	BTC_VENDOR_OTHER
} BTC_VENDOR, *PBTC_VENDOR;


/* defined for BFP_BTC_GET */
typedef enum _BTC_GET_TYPE {
	/* type bool */
	BTC_GET_BL_HS_OPERATION,
	BTC_GET_BL_HS_CONNECTING,
	BTC_GET_BL_WIFI_FW_READY,
	BTC_GET_BL_WIFI_CONNECTED,
	BTC_GET_BL_WIFI_BUSY,
	BTC_GET_BL_WIFI_SCAN,
	BTC_GET_BL_WIFI_LINK,
	BTC_GET_BL_WIFI_ROAM,
	BTC_GET_BL_WIFI_4_WAY_PROGRESS,
	BTC_GET_BL_WIFI_UNDER_5G,
	BTC_GET_BL_WIFI_AP_MODE_ENABLE,
	BTC_GET_BL_WIFI_ENABLE_ENCRYPTION,
	BTC_GET_BL_WIFI_UNDER_B_MODE,
	BTC_GET_BL_EXT_SWITCH,
	BTC_GET_BL_WIFI_IS_IN_MP_MODE,
	BTC_GET_BL_IS_ASUS_8723B,

	/* type s4Byte */
	BTC_GET_S4_WIFI_RSSI,
	BTC_GET_S4_HS_RSSI,

	/* type u32 */
	BTC_GET_U4_WIFI_BW,
	BTC_GET_U4_WIFI_TRAFFIC_DIRECTION,
	BTC_GET_U4_WIFI_FW_VER,
	BTC_GET_U4_WIFI_LINK_STATUS,
	BTC_GET_U4_BT_PATCH_VER,
	BTC_GET_U4_VENDOR,
	BTC_GET_U4_SUPPORTED_VERSION,
	BTC_GET_U4_SUPPORTED_FEATURE,
	BTC_GET_U4_WIFI_IQK_TOTAL,
	BTC_GET_U4_WIFI_IQK_OK,
	BTC_GET_U4_WIFI_IQK_FAIL,

	/* type u8 */
	BTC_GET_U1_WIFI_DOT11_CHNL,
	BTC_GET_U1_WIFI_CENTRAL_CHNL,
	BTC_GET_U1_WIFI_HS_CHNL,
	BTC_GET_U1_WIFI_P2P_CHNL,
	BTC_GET_U1_MAC_PHY_MODE,
	BTC_GET_U1_AP_NUM,
	BTC_GET_U1_ANT_TYPE,
	BTC_GET_U1_IOT_PEER,

	/*===== for 1Ant ======*/
	BTC_GET_U1_LPS_MODE,

	BTC_GET_MAX
} BTC_GET_TYPE, *PBTC_GET_TYPE;

/* defined for BFP_BTC_SET */
typedef enum _BTC_SET_TYPE {
	/* type bool */
	BTC_SET_BL_BT_DISABLE,
	BTC_SET_BL_BT_ENABLE_DISABLE_CHANGE,
	BTC_SET_BL_BT_TRAFFIC_BUSY,
	BTC_SET_BL_BT_LIMITED_DIG,
	BTC_SET_BL_FORCE_TO_ROAM,
	BTC_SET_BL_TO_REJ_AP_AGG_PKT,
	BTC_SET_BL_BT_CTRL_AGG_SIZE,
	BTC_SET_BL_INC_SCAN_DEV_NUM,
	BTC_SET_BL_BT_TX_RX_MASK,
	BTC_SET_BL_MIRACAST_PLUS_BT,

	/* type u8 */
	BTC_SET_U1_RSSI_ADJ_VAL_FOR_AGC_TABLE_ON,
	BTC_SET_U1_AGG_BUF_SIZE,

	/* type trigger some action */
	BTC_SET_ACT_GET_BT_RSSI,
	BTC_SET_ACT_AGGREGATE_CTRL,
	BTC_SET_ACT_ANTPOSREGRISTRY_CTRL,
	/*===== for 1Ant ======*/
	/* type bool */

	/* type u8 */
	BTC_SET_U1_RSSI_ADJ_VAL_FOR_1ANT_COEX_TYPE,
	BTC_SET_U1_LPS_VAL,
	BTC_SET_U1_RPWM_VAL,
	/* type trigger some action */
	BTC_SET_ACT_LEAVE_LPS,
	BTC_SET_ACT_ENTER_LPS,
	BTC_SET_ACT_NORMAL_LPS,
	BTC_SET_ACT_DISABLE_LOW_POWER,
	BTC_SET_ACT_UPDATE_RAMASK,
	BTC_SET_ACT_SEND_MIMO_PS,
	/* BT Coex related */
	BTC_SET_ACT_CTRL_BT_INFO,
	BTC_SET_ACT_CTRL_BT_COEX,
	BTC_SET_ACT_CTRL_8723B_ANT,
	/*=================*/
	BTC_SET_MAX
} BTC_SET_TYPE, *PBTC_SET_TYPE;

typedef enum _BTC_DBG_DISP_TYPE {
	BTC_DBG_DISP_COEX_STATISTICS				= 0x0,
	BTC_DBG_DISP_BT_LINK_INFO				= 0x1,
	BTC_DBG_DISP_WIFI_STATUS				= 0x2,
	BTC_DBG_DISP_MAX
} BTC_DBG_DISP_TYPE, *PBTC_DBG_DISP_TYPE;

typedef enum _BTC_NOTIFY_TYPE_IPS {
	BTC_IPS_LEAVE							= 0x0,
	BTC_IPS_ENTER							= 0x1,
	BTC_IPS_MAX
} BTC_NOTIFY_TYPE_IPS, *PBTC_NOTIFY_TYPE_IPS;
typedef enum _BTC_NOTIFY_TYPE_LPS {
	BTC_LPS_DISABLE							= 0x0,
	BTC_LPS_ENABLE							= 0x1,
	BTC_LPS_MAX
} BTC_NOTIFY_TYPE_LPS, *PBTC_NOTIFY_TYPE_LPS;
typedef enum _BTC_NOTIFY_TYPE_SCAN {
	BTC_SCAN_FINISH							= 0x0,
	BTC_SCAN_START							= 0x1,
	BTC_SCAN_START_2G						= 0x2,
	BTC_SCAN_MAX
} BTC_NOTIFY_TYPE_SCAN, *PBTC_NOTIFY_TYPE_SCAN;
typedef enum _BTC_NOTIFY_TYPE_SWITCHBAND {
	BTC_NOT_SWITCH							= 0x0,
	BTC_SWITCH_TO_24G						= 0x1,
	BTC_SWITCH_TO_5G						= 0x2,
	BTC_SWITCH_TO_24G_NOFORSCAN				= 0x3,
	BTC_SWITCH_MAX
} BTC_NOTIFY_TYPE_SWITCHBAND, *PBTC_NOTIFY_TYPE_SWITCHBAND;
typedef enum _BTC_NOTIFY_TYPE_ASSOCIATE {
	BTC_ASSOCIATE_FINISH						= 0x0,
	BTC_ASSOCIATE_START						= 0x1,
	BTC_ASSOCIATE_5G_FINISH						= 0x2,
	BTC_ASSOCIATE_5G_START						= 0x3,
	BTC_ASSOCIATE_MAX
} BTC_NOTIFY_TYPE_ASSOCIATE, *PBTC_NOTIFY_TYPE_ASSOCIATE;
typedef enum _BTC_NOTIFY_TYPE_MEDIA_STATUS {
	BTC_MEDIA_DISCONNECT					= 0x0,
	BTC_MEDIA_CONNECT						= 0x1,
	BTC_MEDIA_MAX
} BTC_NOTIFY_TYPE_MEDIA_STATUS, *PBTC_NOTIFY_TYPE_MEDIA_STATUS;
typedef enum _BTC_NOTIFY_TYPE_SPECIFIC_PACKET {
	BTC_PACKET_UNKNOWN					= 0x0,
	BTC_PACKET_DHCP							= 0x1,
	BTC_PACKET_ARP							= 0x2,
	BTC_PACKET_EAPOL						= 0x3,
	BTC_PACKET_MAX
} BTC_NOTIFY_TYPE_SPECIFIC_PACKET, *PBTC_NOTIFY_TYPE_SPECIFIC_PACKET;
typedef enum _BTC_NOTIFY_TYPE_STACK_OPERATION {
	BTC_STACK_OP_NONE					= 0x0,
	BTC_STACK_OP_INQ_PAGE_PAIR_START		= 0x1,
	BTC_STACK_OP_INQ_PAGE_PAIR_FINISH	= 0x2,
	BTC_STACK_OP_MAX
} BTC_NOTIFY_TYPE_STACK_OPERATION, *PBTC_NOTIFY_TYPE_STACK_OPERATION;

/* Bryant Add */
typedef enum _BTC_ANTENNA_POS {
	BTC_ANTENNA_AT_MAIN_PORT				= 0x1,
	BTC_ANTENNA_AT_AUX_PORT				= 0x2,
} BTC_ANTENNA_POS, *PBTC_ANTENNA_POS;

/* Bryant Add */
typedef enum _BTC_BT_OFFON {
	BTC_BT_OFF				= 0x0,
	BTC_BT_ON				= 0x1,
} BTC_BTOFFON, *PBTC_BT_OFFON;

/*==================================================
For following block is for coex offload
==================================================*/
typedef struct _COL_H2C {
	u8	opcode;
	u8	opcode_ver:4;
	u8	req_num:4;
	u8	buf[1];
} COL_H2C, *PCOL_H2C;

#define	COL_C2H_ACK_HDR_LEN	3
typedef struct _COL_C2H_ACK {
	u8	status;
	u8	opcode_ver:4;
	u8	req_num:4;
	u8	ret_len;
	u8	buf[1];
} COL_C2H_ACK, *PCOL_C2H_ACK;

#define	COL_C2H_IND_HDR_LEN	3
typedef struct _COL_C2H_IND {
	u8	type;
	u8	version;
	u8	length;
	u8	data[1];
} COL_C2H_IND, *PCOL_C2H_IND;

/*============================================
NOTE: for debug message, the following define should match
the strings in coexH2cResultString.
============================================*/
typedef enum _COL_H2C_STATUS {
	/* c2h status */
	COL_STATUS_C2H_OK								= 0x00, /* Wifi received H2C request and check content ok. */
	COL_STATUS_C2H_UNKNOWN							= 0x01,	/* Not handled routine */
	COL_STATUS_C2H_UNKNOWN_OPCODE					= 0x02,	/* Invalid OP code, It means that wifi firmware received an undefiend OP code. */
	COL_STATUS_C2H_OPCODE_VER_MISMATCH			= 0x03, /* Wifi firmware and wifi driver mismatch, need to update wifi driver or wifi or. */
	COL_STATUS_C2H_PARAMETER_ERROR				= 0x04, /* Error paraneter.(ex: parameters = NULL but it should have values) */
	COL_STATUS_C2H_PARAMETER_OUT_OF_RANGE		= 0x05, /* Wifi firmware needs to check the parameters from H2C request and return the status.(ex: ch = 500, it's wrong) */
	/* other COL status start from here */
	COL_STATUS_C2H_REQ_NUM_MISMATCH			, /* c2h req_num mismatch, means this c2h is not we expected. */
	COL_STATUS_H2C_HALMAC_FAIL					, /* HALMAC return fail. */
	COL_STATUS_H2C_TIMT					, /* not received the c2h response from fw */
	COL_STATUS_INVALID_C2H_LEN					, /* invalid coex offload c2h ack length, must >= 3 */
	COL_STATUS_COEX_DATA_OVERFLOW				, /* coex returned length over the c2h ack length. */
	COL_STATUS_MAX
} COL_H2C_STATUS, *PCOL_H2C_STATUS;

#define	COL_MAX_H2C_REQ_NUM		16

#define	COL_H2C_BUF_LEN			20
typedef enum _COL_OPCODE {
	COL_OP_WIFI_STATUS_NOTIFY					= 0x0,
	COL_OP_WIFI_PROGRESS_NOTIFY					= 0x1,
	COL_OP_WIFI_INFO_NOTIFY						= 0x2,
	COL_OP_WIFI_POWER_STATE_NOTIFY				= 0x3,
	COL_OP_SET_CONTROL							= 0x4,
	COL_OP_GET_CONTROL							= 0x5,
	COL_OP_WIFI_OPCODE_MAX
} COL_OPCODE, *PCOL_OPCODE;

typedef enum _COL_IND_TYPE {
	COL_IND_BT_INFO								= 0x0,
	COL_IND_PSTDMA								= 0x1,
	COL_IND_LIMITED_TX_RX						= 0x2,
	COL_IND_COEX_TABLE							= 0x3,
	COL_IND_REQ									= 0x4,
	COL_IND_MAX
} COL_IND_TYPE, *PCOL_IND_TYPE;

typedef struct _COL_SINGLE_H2C_RECORD {
	u8					h2c_buf[COL_H2C_BUF_LEN];	/* the latest sent h2c buffer */
	u32					h2c_len;
	u8					c2h_ack_buf[COL_H2C_BUF_LEN];	/* the latest received c2h buffer */
	u32					c2h_ack_len;
	u32					count;									/* the total number of the sent h2c command */
	u32					status[COL_STATUS_MAX];					/* the c2h status for the sent h2c command */
} COL_SINGLE_H2C_RECORD, *PCOL_SINGLE_H2C_RECORD;

typedef struct _COL_SINGLE_C2H_IND_RECORD {
	u8					ind_buf[COL_H2C_BUF_LEN];	/* the latest received c2h indication buffer */
	u32					ind_len;
	u32					count;									/* the total number of the rcvd c2h indication */
	u32					status[COL_STATUS_MAX];					/* the c2h indication verified status */
} COL_SINGLE_C2H_IND_RECORD, *PCOL_SINGLE_C2H_IND_RECORD;

typedef struct _BTC_OFFLOAD {
	/* H2C command related */
	u8					h2c_req_num;
	u32					cnt_h2c_sent;
	COL_SINGLE_H2C_RECORD	h2c_record[COL_OP_WIFI_OPCODE_MAX];

	/* C2H Ack related */
	u32					cnt_c2h_ack;
	u32					status[COL_STATUS_MAX];
	struct completion		c2h_event[COL_MAX_H2C_REQ_NUM];	/* for req_num = 1~COL_MAX_H2C_REQ_NUM */
	u8					c2h_ack_buf[COL_MAX_H2C_REQ_NUM][COL_H2C_BUF_LEN];
	u8					c2h_ack_len[COL_MAX_H2C_REQ_NUM];

	/* C2H Indication related */
	u32						cnt_c2h_ind;
	COL_SINGLE_C2H_IND_RECORD	c2h_ind_record[COL_IND_MAX];
	u32						c2h_ind_status[COL_STATUS_MAX];
	u8						c2h_ind_buf[COL_H2C_BUF_LEN];
	u8						c2h_ind_len;
} BTC_OFFLOAD, *PBTC_OFFLOAD;
extern BTC_OFFLOAD				gl_coex_offload;
/*==================================================*/

typedef u8
(*BFP_BTC_R1)(
	void *			pBtcContext,
	u32			RegAddr
	);
typedef u16
(*BFP_BTC_R2)(
	void *			pBtcContext,
	u32			RegAddr
	);
typedef u32
(*BFP_BTC_R4)(
	void *			pBtcContext,
	u32			RegAddr
	);
typedef void
(*BFP_BTC_W1)(
	void *			pBtcContext,
	u32			RegAddr,
	u8			Data
	);
typedef void
(*BFP_BTC_W1_BIT_MASK)(
	void *			pBtcContext,
	u32			regAddr,
	u8			bitMask,
	u8			data1b
	);
typedef void
(*BFP_BTC_W2)(
	void *			pBtcContext,
	u32			RegAddr,
	u16			Data
	);
typedef void
(*BFP_BTC_W4)(
	void *			pBtcContext,
	u32			RegAddr,
	u32			Data
	);
typedef void
(*BFP_BTC_LOCAL_REG_W1)(
	void *			pBtcContext,
	u32			RegAddr,
	u8			Data
	);
typedef void
(*BFP_BTC_SET_BB_REG)(
	void *			pBtcContext,
	u32			RegAddr,
	u32			BitMask,
	u32			Data
	);
typedef u32
(*BFP_BTC_GET_BB_REG)(
	void *			pBtcContext,
	u32			RegAddr,
	u32			BitMask
	);
typedef void
(*BFP_BTC_SET_RF_REG)(
	void *			pBtcContext,
	u8			eRFPath,
	u32			RegAddr,
	u32			BitMask,
	u32			Data
	);
typedef u32
(*BFP_BTC_GET_RF_REG)(
	void *			pBtcContext,
	u8			eRFPath,
	u32			RegAddr,
	u32			BitMask
	);
typedef void
(*BFP_BTC_FILL_H2C)(
	void *			pBtcContext,
	u8			elementId,
	u32			cmdLen,
	u8 *			pCmdBuffer
	);

typedef	bool
(*BFP_BTC_GET)(
	void *			pBtCoexist,
	u8			getType,
	void *			pOutBuf
	);

typedef	bool
(*BFP_BTC_SET)(
	void *			pBtCoexist,
	u8			setType,
	void *			pInBuf
	);
typedef u16
(*BFP_BTC_SET_BT_REG)(
	void *			pBtcContext,
	u8			regType,
	u32			offset,
	u32			value
	);
typedef bool
(*BFP_BTC_SET_BT_ANT_DETECTION)(
	void *			pBtcContext,
	u8			txTime,
	u8			btChnl
	);

typedef bool
(*BFP_BTC_SET_BT_TRX_MASK)(
	void *			pBtcContext,
	u8			bt_trx_mask
	);

typedef u32
(*BFP_BTC_GET_BT_REG)(
	void *			pBtcContext,
	u8			regType,
	u32			offset
	);
typedef void
(*BFP_BTC_DISP_DBG_MSG)(
	void *			pBtCoexist,
	u8			dispType
	);

typedef COL_H2C_STATUS
(*BFP_BTC_COEX_H2C_PROCESS)(
	void *			pBtCoexist,
	u8			opcode,
	u8			opcode_ver,
	u8 *			ph2c_par,
	u8			h2c_par_len
	);

typedef u32
(*BFP_BTC_GET_BT_COEX_SUPPORTED_FEATURE)(
	void *			pBtcContext
	);

typedef u32
(*BFP_BTC_GET_BT_COEX_SUPPORTED_VERSION)(
	void *			pBtcContext
	);

typedef u32
(*BFP_BTC_GET_PHYDM_VERSION)(
	void *			pBtcContext
	);

typedef void
(*BTC_PHYDM_MODIFY_RA_PCR_THRESHLOD)(
	void *		pDM_Odm,
	u8		RA_offset_direction,
	u8		RA_threshold_offset
	);

typedef u32
(*BTC_PHYDM_CMNINFOQUERY)(
		void *	pDM_Odm,
		u8	info_type
	);

typedef u8
(*BFP_BTC_GET_ANT_DET_VAL_FROM_BT)(

	void *			pBtcContext
	);

typedef u8
(*BFP_BTC_GET_BLE_SCAN_TYPE_FROM_BT)(
	void *			pBtcContext
	);

typedef u32
(*BFP_BTC_GET_BLE_SCAN_PARA_FROM_BT)(
	void *			pBtcContext,
	u8			scanType
	);

typedef bool
(*BFP_BTC_GET_BT_AFH_MAP_FROM_BT)(
	void *			pBtcContext,
	u8			mapType,
	u8 *			afhMap
	);

struct  btc_bt_info {
	bool					bt_disabled;
	bool				bt_enable_disable_change;
	u8					rssi_adjust_for_agc_table_on;
	u8					rssi_adjust_for_1ant_coex_type;
	bool					pre_bt_ctrl_agg_buf_size;
	bool					bt_ctrl_agg_buf_size;
	bool					pre_reject_agg_pkt;
	bool					reject_agg_pkt;
	bool					increase_scan_dev_num;
	bool					bt_tx_rx_mask;
	u8					pre_agg_buf_size;
	u8					agg_buf_size;
	bool					bt_busy;
	bool					limited_dig;
	u16					bt_hci_ver;
	u16					bt_real_fw_ver;
	u8					bt_fw_ver;
	u32					get_bt_fw_ver_cnt;
	u32					bt_get_fw_ver;
	bool					miracast_plus_bt;

	bool					bt_disable_low_pwr;

	bool					bt_ctrl_lps;
	bool					bt_lps_on;
	bool					force_to_roam;	/* for 1Ant solution */
	u8					lps_val;
	u8					rpwm_val;
	u32					ra_mask;
};

struct btc_stack_info {
	bool					profile_notified;
	u16					hci_version;	/* stack hci version */
	u8					num_of_link;
	bool					bt_link_exist;
	bool					sco_exist;
	bool					acl_exist;
	bool					a2dp_exist;
	bool					hid_exist;
	u8					num_of_hid;
	bool					pan_exist;
	bool					unknown_acl_exist;
	s8					min_bt_rssi;
};

struct btc_bt_link_info {
	bool					bt_link_exist;
	bool					bt_hi_pri_link_exist;
	bool					sco_exist;
	bool					sco_only;
	bool					a2dp_exist;
	bool					a2dp_only;
	bool					hid_exist;
	bool					hid_only;
	bool					pan_exist;
	bool					pan_only;
	bool					slave_role;
	bool					acl_busy;
};

struct btc_statistics {
	u32					cnt_bind;
	u32					cnt_power_on;
	u32					cnt_pre_load_firmware;
	u32					cnt_init_hw_config;
	u32					cnt_init_coex_dm;
	u32					cnt_ips_notify;
	u32					cnt_lps_notify;
	u32					cnt_scan_notify;
	u32					cnt_connect_notify;
	u32					cnt_media_status_notify;
	u32					cnt_specific_packet_notify;
	u32					cnt_bt_info_notify;
	u32					cnt_rf_status_notify;
	u32					cnt_periodical;
	u32					cnt_coex_dm_switch;
	u32					cnt_stack_operation_notify;
	u32					cnt_dbg_ctrl;
};

struct btc_coexist {
	bool				bBinded;		/*make sure only one adapter can bind the data context*/
	void *				Adapter;		/*default adapter*/
	struct  btc_board_info		board_info;
	struct  btc_bt_info			bt_info;		/*some bt info referenced by non-bt module*/
	struct  btc_stack_info		stack_info;
	struct  btc_bt_link_info		bt_link_info;
	BTC_CHIP_INTERFACE		chip_interface;
	void *					odm_priv;

	bool					initilized;
	bool					stop_coex_dm;
	bool					manual_control;
	bool					bdontenterLPS;
	u8 *					cli_buf;
	struct btc_statistics		statistics;
	u8				pwrModeVal[10];

	/* function pointers */
	/* io related */
	BFP_BTC_R1			btc_read_1byte;
	BFP_BTC_W1			btc_write_1byte;
	BFP_BTC_W1_BIT_MASK	btc_write_1byte_bitmask;
	BFP_BTC_R2			btc_read_2byte;
	BFP_BTC_W2			btc_write_2byte;
	BFP_BTC_R4			btc_read_4byte;
	BFP_BTC_W4			btc_write_4byte;
	BFP_BTC_LOCAL_REG_W1	btc_write_local_reg_1byte;
	/* read/write bb related */
	BFP_BTC_SET_BB_REG	btc_set_bb_reg;
	BFP_BTC_GET_BB_REG	btc_get_bb_reg;

	/* read/write rf related */
	BFP_BTC_SET_RF_REG	btc_set_rf_reg;
	BFP_BTC_GET_RF_REG	btc_get_rf_reg;

	/* fill h2c related */
	BFP_BTC_FILL_H2C		btc_fill_h2c;
	/* other */
	BFP_BTC_DISP_DBG_MSG	btc_disp_dbg_msg;
	/* normal get/set related */
	BFP_BTC_GET			btc_get;
	BFP_BTC_SET			btc_set;

	BFP_BTC_GET_BT_REG	btc_get_bt_reg;
	BFP_BTC_SET_BT_REG	btc_set_bt_reg;

	BFP_BTC_SET_BT_ANT_DETECTION	btc_set_bt_ant_detection;

	BFP_BTC_COEX_H2C_PROCESS	btc_coex_h2c_process;
	BFP_BTC_SET_BT_TRX_MASK		btc_set_bt_trx_mask;
	BFP_BTC_GET_BT_COEX_SUPPORTED_FEATURE btc_get_bt_coex_supported_feature;
	BFP_BTC_GET_BT_COEX_SUPPORTED_VERSION btc_get_bt_coex_supported_version;
	BFP_BTC_GET_PHYDM_VERSION		btc_get_bt_phydm_version;
	BTC_PHYDM_MODIFY_RA_PCR_THRESHLOD	btc_phydm_modify_RA_PCR_threshold;
	BTC_PHYDM_CMNINFOQUERY				btc_phydm_query_PHY_counter;
	BFP_BTC_GET_ANT_DET_VAL_FROM_BT		btc_get_ant_det_val_from_bt;
	BFP_BTC_GET_BLE_SCAN_TYPE_FROM_BT	btc_get_ble_scan_type_from_bt;
	BFP_BTC_GET_BLE_SCAN_PARA_FROM_BT	btc_get_ble_scan_para_from_bt;
	BFP_BTC_GET_BT_AFH_MAP_FROM_BT		btc_get_bt_afh_map_from_bt;
};
typedef struct btc_coexist *PBTC_COEXIST;

extern struct btc_coexist	GLBtCoexist;

bool
EXhalbtcoutsrc_InitlizeVariables(
	void *		Adapter
	);
void
EXhalbtcoutsrc_PowerOnSetting(
	PBTC_COEXIST		pBtCoexist
	);
void
EXhalbtcoutsrc_PreLoadFirmware(
	PBTC_COEXIST		pBtCoexist
	);
void
EXhalbtcoutsrc_InitHwConfig(
	PBTC_COEXIST		pBtCoexist,
	bool				bWifiOnly
	);
void
EXhalbtcoutsrc_InitCoexDm(
	PBTC_COEXIST		pBtCoexist
	);
void
EXhalbtcoutsrc_IpsNotify(
	PBTC_COEXIST		pBtCoexist,
	u8			type
	);
void
EXhalbtcoutsrc_LpsNotify(
	PBTC_COEXIST		pBtCoexist,
	u8			type
	);
void
EXhalbtcoutsrc_ScanNotify(
	PBTC_COEXIST		pBtCoexist,
	u8			type
	);
void
EXhalbtcoutsrc_SetAntennaPathNotify(
	PBTC_COEXIST	pBtCoexist,
	u8			type
	);
void
EXhalbtcoutsrc_ConnectNotify(
	PBTC_COEXIST		pBtCoexist,
	u8			action
	);
void
EXhalbtcoutsrc_MediaStatusNotify(
	PBTC_COEXIST		pBtCoexist,
	RT_MEDIA_STATUS	mediaStatus
	);
void
EXhalbtcoutsrc_SpecificPacketNotify(
	PBTC_COEXIST		pBtCoexist,
	u8			pktType
	);
void
EXhalbtcoutsrc_BtInfoNotify(
	PBTC_COEXIST		pBtCoexist,
	u8 *			tmpBuf,
	u8			length
	);
void
EXhalbtcoutsrc_RfStatusNotify(
	PBTC_COEXIST		pBtCoexist,
	u8				type
	);
void
EXhalbtcoutsrc_StackOperationNotify(
	PBTC_COEXIST		pBtCoexist,
	u8			type
	);
void
EXhalbtcoutsrc_HaltNotify(
	PBTC_COEXIST		pBtCoexist
	);
void
EXhalbtcoutsrc_PnpNotify(
	PBTC_COEXIST		pBtCoexist,
	u8			pnpState
	);
void
EXhalbtcoutsrc_CoexDmSwitch(
	PBTC_COEXIST		pBtCoexist
	);
void
EXhalbtcoutsrc_Periodical(
	PBTC_COEXIST		pBtCoexist
	);
void
EXhalbtcoutsrc_DbgControl(
	PBTC_COEXIST			pBtCoexist,
	u8				opCode,
	u8				opLen,
	u8 *				pData
	);
void
EXhalbtcoutsrc_AntennaDetection(
	PBTC_COEXIST			pBtCoexist,
	u32					centFreq,
	u32					offset,
	u32					span,
	u32					seconds
	);
void
EXhalbtcoutsrc_StackUpdateProfileInfo(
	void
	);
void
EXhalbtcoutsrc_SetHciVersion(
	u16	hciVersion
	);
void
EXhalbtcoutsrc_SetBtPatchVersion(
	u16	btHciVersion,
	u16	btPatchVersion
	);
void
EXhalbtcoutsrc_UpdateMinBtRssi(
	s8	btRssi
	);
void
EXhalbtcoutsrc_SetChipType(
	u8		chipType
	);
void
EXhalbtcoutsrc_SetAntNum(
	u8		type,
	u8		antNum
	);
void
EXhalbtcoutsrc_SetSingleAntPath(
	u8		singleAntPath
	);
void
EXhalbtcoutsrc_DisplayBtCoexInfo(
	PBTC_COEXIST		pBtCoexist
	);
void
EXhalbtcoutsrc_DisplayAntDetection(
	PBTC_COEXIST		pBtCoexist
	);

#define	MASKBYTE0		0xff
#define	MASKBYTE1		0xff00
#define	MASKBYTE2		0xff0000
#define	MASKBYTE3		0xff000000
#define	MASKHWORD	0xffff0000
#define	MASKLWORD		0x0000ffff
#define	MASKDWORD	0xffffffff
#define	MASK12BITS		0xfff
#define	MASKH4BITS		0xf0000000
#define	MASKOFDM_D	0xffc00000
#define	MASKCCK		0x3f3f3f3f

#endif
