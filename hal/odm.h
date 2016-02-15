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


#ifndef	__HALDMOUTSRC_H__
#define __HALDMOUTSRC_H__

/*  */
/*  Definition  */
/*  */
/*  */
/*  2011/09/22 MH Define all team supprt ability. */
/*  */

/*  */
/*  2011/09/22 MH Define for all teams. Please Define the constan in your precomp header. */
/*  */
/* define		DM_ODM_SUPPORT_AP			0 */
/* define		DM_ODM_SUPPORT_ADSL			0 */
/* define		DM_ODM_SUPPORT_CE			0 */
/* define		DM_ODM_SUPPORT_MP			1 */

/*  */
/*  2011/09/28 MH Define ODM SW team support flag. */
/*  */



/*  */
/*  Antenna Switch Relative Definition. */
/*  */

/*  */
/*  20100503 Joseph: */
/*  Add new function SwAntDivCheck8192C(). */
/*  This is the main function of Antenna diversity function before link. */
/*  Mainly, it just retains last scan result and scan again. */
/*  After that, it compares the scan result to see which one gets better RSSI. */
/*  It selects antenna with better receiving power and returns better scan result. */
/*  */
#define	TP_MODE		0
#define	RSSI_MODE		1
#define	TRAFFIC_LOW	0
#define	TRAFFIC_HIGH	1


/*  */
/* 3 Tx Power Tracking */
/* 3============================================================ */
#define		DPK_DELTA_MAPPING_NUM	13
#define		index_mapping_HP_NUM	15	


/*  */
/* 3 PSD Handler */
/* 3============================================================ */

#define	AFH_PSD		1	/* 0:normal PSD scan, 1: only do 20 pts PSD */
#define	MODE_40M		0	/* 0:20M, 1:40M */
#define	PSD_TH2		3  
#define	PSD_CHMIN		20   /*  Minimum channel number for BT AFH */
#define	SIR_STEP_SIZE	3
#define   Smooth_Size_1 	5
#define	Smooth_TH_1	3
#define   Smooth_Size_2 	10
#define	Smooth_TH_2	4
#define   Smooth_Size_3 	20
#define	Smooth_TH_3	4
#define   Smooth_Step_Size 5
#define	Adaptive_SIR	1
#define	PSD_RESCAN		4
#define	PSD_SCAN_INTERVAL	700 /* ms */



/* 8723A High Power IGI Setting */
#define		DM_DIG_HIGH_PWR_IGI_LOWER_BOUND	0x22
#define  		DM_DIG_Gmode_HIGH_PWR_IGI_LOWER_BOUND 0x28
#define		DM_DIG_HIGH_PWR_THRESHOLD	0x3a
#define		DM_DIG_LOW_PWR_THRESHOLD	0x14

/* ANT Test */
#define 		ANTTESTALL		0x00		/* Ant A or B will be Testing    */
#define		ANTTESTA		0x01		/* Ant A will be Testing	 */
#define		ANTTESTB		0x02		/* Ant B will be testing */

/*  LPS define */
#define DM_DIG_FA_TH0_LPS				4 /*  4 in lps */
#define DM_DIG_FA_TH1_LPS				15 /*  15 lps */
#define DM_DIG_FA_TH2_LPS				30 /*  30 lps */
#define RSSI_OFFSET_DIG					0x05;

/* ANT Test */
#define 		ANTTESTALL		0x00		/* Ant A or B will be Testing    */
#define		ANTTESTA		0x01		/* Ant A will be Testing	 */
#define		ANTTESTB		0x02		/* Ant B will be testing */


/*  */
/*  structure and define */
/*  */

/*  */
/*  2011/09/20 MH Add for AP/ADSLpseudo DM structuer requirement. */
/*  We need to remove to other position??? */
/*  */
typedef		struct rtl8192cd_priv {
	u8		temp;

}rtl8192cd_priv, *prtl8192cd_priv;

typedef struct _Dynamic_Initial_Gain_Threshold_
{
	u8		Dig_Enable_Flag;
	u8		Dig_Ext_Port_Stage;
	
	int		   	RssiLowThresh;
	int		    	RssiHighThresh;

	u32		FALowThresh;
	u32		FAHighThresh;

	u8		CurSTAConnectState;
	u8		PreSTAConnectState;
	u8		CurMultiSTAConnectState;

	u8		PreIGValue;
	u8		CurIGValue;
	u8		BT30_CurIGI;
	u8		BackupIGValue;

	s8		BackoffVal;
	s8		BackoffVal_range_max;
	s8		BackoffVal_range_min;
	u8		rx_gain_range_max;
	u8		rx_gain_range_min;
	u8		Rssi_val_min;

	u8		PreCCK_CCAThres;
	u8		CurCCK_CCAThres;
	u8		PreCCKPDState;
	u8		CurCCKPDState;

	u8		LargeFAHit;
	u8		ForbiddenIGI;
	u32		Recover_cnt;

	u8		DIG_Dynamic_MIN_0;
	u8		DIG_Dynamic_MIN_1;
	bool		bMediaConnect_0;
	bool		bMediaConnect_1;

	u32		AntDiv_RSSI_max;
	u32		RSSI_max;
}DIG_T,*pDIG_T;

typedef struct _Dynamic_Power_Saving_
{
	u8		PreCCAState;
	u8		CurCCAState;

	u8		PreRFState;
	u8		CurRFState;

	int		    	Rssi_val_min;
	
	u8		initialize;
	u32		Reg874,RegC70,Reg85C,RegA74;
	
}PS_T,*pPS_T;

typedef struct false_ALARM_STATISTICS{
	u32	Cnt_Parity_Fail;
	u32	Cnt_Rate_Illegal;
	u32	Cnt_Crc8_fail;
	u32	Cnt_Mcs_fail;
	u32	Cnt_Ofdm_fail;
	u32	Cnt_Cck_fail;
	u32	Cnt_all;
	u32	Cnt_Fast_Fsync;
	u32	Cnt_SB_Search_fail;
	u32	Cnt_OFDM_CCA;
	u32	Cnt_CCK_CCA;
	u32	Cnt_CCA_all;
	u32	Cnt_BW_USC;	/* Gary */
	u32	Cnt_BW_LSC;	/* Gary */
}false_ALARM_STATISTICS, *Pfalse_ALARM_STATISTICS;

typedef struct _Dynamic_Primary_CCA{
	u8		PriCCA_flag;
	u8		intf_flag;
	u8		intf_type;  
	u8		DupRTS_flag;
	u8		Monitor_flag;
}Pri_CCA_T, *pPri_CCA_T;
	
typedef struct _RX_High_Power_
{
	u8		RXHP_flag;
	u8		PSD_func_trigger;
	u8		PSD_bitmap_RXHP[80];
	u8		Pre_IGI;
	u8		Cur_IGI;
	u8		Pre_pw_th;
	u8		Cur_pw_th;
	bool		First_time_enter;
	bool		RXHP_enable;
	u8		TP_Mode;
	RT_TIMER	PSDTimer;
}RXHP_T, *pRXHP_T;

#define ASSOCIATE_ENTRY_NUM					32 /*  Max size of AsocEntry[]. */
#define	ODM_ASSOCIATE_ENTRY_NUM				ASSOCIATE_ENTRY_NUM

/*  This indicates two different the steps.  */
/*  In SWAW_STEP_PEAK, driver needs to switch antenna and listen to the signal on the air. */
/*  In SWAW_STEP_DETERMINE, driver just compares the signal captured in SWAW_STEP_PEAK */
/*  with original RSSI to determine if it is necessary to switch antenna. */
#define SWAW_STEP_PEAK		0
#define SWAW_STEP_DETERMINE	1

#define	TP_MODE		0
#define	RSSI_MODE		1
#define	TRAFFIC_LOW	0
#define	TRAFFIC_HIGH	1

typedef struct _SW_Antenna_Switch_
{
	u8		try_flag;
	s32		PreRSSI;
	u8		CurAntenna;
	u8		PreAntenna;
	u8		RSSI_Trying;
	u8		TestMode;
	u8		bTriggerAntennaSwitch;
	u8		SelectAntennaMap;
	u8		RSSI_target;	

	/*  Before link Antenna Switch check */
	u8		SWAS_NoLink_State;
	u32		SWAS_NoLink_BK_Reg860;
	bool		ANTA_ON;	/* To indicate Ant A is or not */
	bool		ANTB_ON;	/* To indicate Ant B is on or not */

	s32		RSSI_sum_A;
	s32		RSSI_sum_B;
	s32		RSSI_cnt_A;
	s32		RSSI_cnt_B;

	u64		lastTxOkCnt;
	u64		lastRxOkCnt;
	u64 		TXByteCnt_A;
	u64 		TXByteCnt_B;
	u64 		RXByteCnt_A;
	u64 		RXByteCnt_B;
	u8 		TrafficLoad;
	RT_TIMER 	SwAntennaSwitchTimer;
	/* Hybrid Antenna Diversity */
	u32		CCK_Ant1_Cnt[ASSOCIATE_ENTRY_NUM];
	u32		CCK_Ant2_Cnt[ASSOCIATE_ENTRY_NUM];
	u32		OFDM_Ant1_Cnt[ASSOCIATE_ENTRY_NUM];
	u32		OFDM_Ant2_Cnt[ASSOCIATE_ENTRY_NUM];
	u32		RSSI_Ant1_Sum[ASSOCIATE_ENTRY_NUM];
	u32		RSSI_Ant2_Sum[ASSOCIATE_ENTRY_NUM];
	u8		TxAnt[ASSOCIATE_ENTRY_NUM];
	u8		TargetSTA;
	u8		antsel;
	u8		RxIdleAnt;
}SWAT_T, *pSWAT_T;

typedef struct _EDCA_TURBO_ {
	bool bCurrentTurboEDCA;
	bool bIsCurRDLState;
	u32	prv_traffic_idx; /*  edca turbo */

}EDCA_T,*pEDCA_T;

typedef struct _ODM_RATE_ADAPTIVE
{
	u8				Type;				/*  DM_Type_ByFW/DM_Type_ByDriver */
	u8				HighRSSIThresh;		/*  if RSSI > HighRSSIThresh	=> RATRState is DM_RATR_STA_HIGH */
	u8				LowRSSIThresh;		/*  if RSSI <= LowRSSIThresh	=> RATRState is DM_RATR_STA_LOW */
	u8				RATRState;			/*  Current RSSI level, DM_RATR_STA_HIGH/DM_RATR_STA_MIDDLE/DM_RATR_STA_LOW */
	u32				LastRATR;			/*  RATR Register Content */

} ODM_RATE_ADAPTIVE, *PODM_RATE_ADAPTIVE;

#define IQK_MAC_REG_NUM		4
#define IQK_ADDA_REG_NUM		16
#define IQK_BB_REG_NUM_MAX	10
#define IQK_BB_REG_NUM		9
#define HP_THERMAL_NUM		8

#define AVG_THERMAL_NUM		8
#define IQK_Matrix_REG_NUM	8
#define IQK_Matrix_Settings_NUM	1+24+21

#define		DM_Type_ByFW			0
#define		DM_Type_ByDriver		1

/*  */
/*  Declare for common info */
/*  */
/*  Declare for common info */
/*  */
#define MAX_PATH_NUM_92CS		2

typedef struct _ODM_Phy_Status_Info_
{	
	u8		RxPWDBAll;	
	u8		SignalQuality;	 /*  in 0-100 index.  */
	u8		RxMIMOSignalQuality[MAX_PATH_NUM_92CS]; /* EVM */
	u8		RxMIMOSignalStrength[MAX_PATH_NUM_92CS];/*  in 0~100 index */
	s8		RxPower; /*  in dBm Translate from PWdB */
	s8		RecvSignalPower;/*  Real power in dBm for this packet, no beautification and aggregation. Keep this raw info to be used for the other procedures. */
	u8		BTRxRSSIPercentage;	
	u8		SignalStrength; /*  in 0-100 index. */
	u8		RxPwr[MAX_PATH_NUM_92CS];/* per-path's pwdb */
	u8		RxSNR[MAX_PATH_NUM_92CS];/* per-path's SNR	 */
}ODM_PHY_INFO_T,*PODM_PHY_INFO_T;


typedef struct _ODM_Phy_Dbg_Info_
{
	/* ODM Write,debug info */
	s8		RxSNRdB[MAX_PATH_NUM_92CS];
	u64		NumQryPhyStatus;
	u64		NumQryPhyStatusCCK;
	u64		NumQryPhyStatusOFDM;
	u8		NumQryBeaconPkt;
	/* Others */
	s32		RxEVM[MAX_PATH_NUM_92CS];	
	
}ODM_PHY_DBG_INFO_T;


typedef struct _ODM_Per_Pkt_Info_
{
	u8		Rate;	
	u8		StationID;
	bool		bPacketMatchBSSID;
	bool		bPacketToSelf;
	bool		bPacketBeacon;
}ODM_PACKET_INFO_T,*PODM_PACKET_INFO_T;

typedef struct _ODM_Mac_Status_Info_
{
	u8	test;
	
}ODM_MAC_INFO;


typedef enum tag_Dynamic_ODM_Support_Ability_Type
{
	/*  BB Team */
	ODM_DIG				= 0x00000001,
	ODM_HIGH_POWER		= 0x00000002,
	ODM_CCK_CCA_TH		= 0x00000004,
	ODM_FA_STATISTICS		= 0x00000008,
	ODM_RAMASK			= 0x00000010,
	ODM_RSSI_MONITOR		= 0x00000020,
	ODM_SW_ANTDIV		= 0x00000040,
	ODM_HW_ANTDIV		= 0x00000080,
	ODM_BB_PWRSV			= 0x00000100,
	ODM_2TPATHDIV			= 0x00000200,
	ODM_1TPATHDIV			= 0x00000400,
	ODM_PSD2AFH			= 0x00000800
}ODM_Ability_E;

/*  */
/*  2011/20/20 MH For MP driver RT_WLAN_STA =  STA_INFO_T */
/*  Please declare below ODM relative info in your STA info structure. */
/*  */
typedef		struct _ODM_STA_INFO{
	/*  Driver Write */
	bool		bUsed;				/*  record the sta status link or not? */
	u8		IOTPeer;			/*  Enum value.	HT_IOT_PEER_E */

	/*  ODM Write */
	/* 1 PHY_STATUS_INFO */
	u8		RSSI_Path[4];		/*   */
	u8		RSSI_Ave;
	u8		RXEVM[4];
	u8		RXSNR[4];
} ODM_STA_INFO_T, *PODM_STA_INFO_T;

/*  */
/*  2011/10/20 MH Define Common info enum for all team. */
/*  */
typedef enum _ODM_Common_Info_Definition
{
/* REMOVED CASE----------- */
	/* ODM_CMNINFO_CCK_HP, */
	/* ODM_CMNINFO_RFPATH_ENABLE,		 Define as ODM write???	 */
	/* ODM_CMNINFO_BT_COEXIST,		 ODM_BT_COEXIST_E */
	/* ODM_CMNINFO_OP_MODE,			ODM_OPERATION_MODE_E */
/* REMOVED CASE----------- */

	/*  */
	/*  Fixed value: */
	/*  */

	/* HOOK BEFORE REG INIT----------- */
	ODM_CMNINFO_PLATFORM = 0,
	ODM_CMNINFO_ABILITY,					/*  ODM_ABILITY_E */
	ODM_CMNINFO_INTERFACE,				/*  ODM_INTERFACE_E */
	ODM_CMNINFO_MP_TEST_CHIP,
	ODM_CMNINFO_IC_TYPE,					/*  ODM_IC_TYPE_E */
	ODM_CMNINFO_CUT_VER,					/*  ODM_CUT_VERSION_E */
	ODM_CMNINFO_FAB_VER,					/*  ODM_FAB_E */
	ODM_CMNINFO_RF_TYPE,					/*  ODM_RF_PATH_E or ODM_RF_TYPE_E? */
	ODM_CMNINFO_BOARD_TYPE,				/*  ODM_BOARD_TYPE_E */
	ODM_CMNINFO_EXT_LNA,					/*  true */
	ODM_CMNINFO_EXT_PA,
	ODM_CMNINFO_EXT_TRSW,
	ODM_CMNINFO_PATCH_ID,				/* CUSTOMER ID */
	ODM_CMNINFO_BINHCT_TEST,
	ODM_CMNINFO_BWIFI_TEST,
	ODM_CMNINFO_SMART_CONCURRENT,
	/* HOOK BEFORE REG INIT-----------	 */


	/*  */
	/*  Dynamic value: */
	/*  */
/*  POINTER REFERENCE----------- */
	ODM_CMNINFO_MAC_PHY_MODE,			/*  ODM_MAC_PHY_MODE_E */
	ODM_CMNINFO_TX_UNI,
	ODM_CMNINFO_RX_UNI,
	ODM_CMNINFO_WM_MODE,				/*  ODM_WIRELESS_MODE_E */
	ODM_CMNINFO_BAND,					/*  ODM_BAND_TYPE_E */
	ODM_CMNINFO_SEC_CHNL_OFFSET,		/*  ODM_SEC_CHNL_OFFSET_E */
	ODM_CMNINFO_SEC_MODE,				/*  ODM_SECURITY_E */
	ODM_CMNINFO_BW,						/*  ODM_BW_E */
	ODM_CMNINFO_CHNL,
	
	ODM_CMNINFO_DMSP_GET_VALUE,
	ODM_CMNINFO_BUDDY_ADAPTOR,
	ODM_CMNINFO_DMSP_IS_MASTER,
	ODM_CMNINFO_SCAN,
	ODM_CMNINFO_POWER_SAVING,
	ODM_CMNINFO_ONE_PATH_CCA,			/*  ODM_CCA_PATH_E */
	ODM_CMNINFO_DRV_STOP,
	ODM_CMNINFO_PNP_IN,
	ODM_CMNINFO_INIT_ON,
	ODM_CMNINFO_ANT_TEST,
	ODM_CMNINFO_NET_CLOSED,
	ODM_CMNINFO_MP_MODE,
/*  POINTER REFERENCE----------- */

/* CALL BY VALUE------------- */
	ODM_CMNINFO_WIFI_DIRECT,
	ODM_CMNINFO_WIFI_DISPLAY,
	ODM_CMNINFO_LINK,
	ODM_CMNINFO_RSSI_MIN,
	ODM_CMNINFO_DBG_COMP,				/*  u64 */
	ODM_CMNINFO_DBG_LEVEL,				/*  u32 */
	ODM_CMNINFO_RA_THRESHOLD_HIGH,		/*  u8 */
	ODM_CMNINFO_RA_THRESHOLD_LOW,		/*  u8 */
	ODM_CMNINFO_RF_ANTENNA_TYPE,		/*  u8 */
	ODM_CMNINFO_BT_DISABLED,
	ODM_CMNINFO_BT_OPERATION,
	ODM_CMNINFO_BT_DIG,
	ODM_CMNINFO_BT_BUSY,					/* Check Bt is using or not */
	ODM_CMNINFO_BT_DISABLE_EDCA,
	ODM_CMNINFO_STATION_STATE,
/* CALL BY VALUE------------- */

	/*  */
	/*  Dynamic ptr array hook itms. */
	/*  */
	ODM_CMNINFO_STA_STATUS,
	ODM_CMNINFO_PHY_STATUS,
	ODM_CMNINFO_MAC_STATUS,
	
	ODM_CMNINFO_MAX,


}ODM_CMNINFO_E;

/*  */
/*  2011/10/20 MH Define ODM support ability.  ODM_CMNINFO_ABILITY */
/*  */
typedef enum _ODM_Support_Ability_Definition
{
	/*  */
	/*  BB ODM section BIT 0-15 */
	/*  */
	ODM_BB_DIG					= BIT0,
	ODM_BB_RA_MASK				= BIT1,
	ODM_BB_DYNAMIC_TXPWR		= BIT2,
	ODM_BB_FA_CNT					= BIT3,
	ODM_BB_RSSI_MONITOR			= BIT4,
	ODM_BB_CCK_PD				= BIT5,
	ODM_BB_ANT_DIV				= BIT6,
	ODM_BB_PWR_SAVE				= BIT7,
	ODM_BB_PWR_TRAIN				= BIT8,
	ODM_BB_RATE_ADAPTIVE			= BIT9,
	ODM_BB_PATH_DIV				= BIT10,
	ODM_BB_PSD					= BIT11,
	ODM_BB_RXHP					= BIT12,
	ODM_BB_ADAPTIVITY				= BIT13,
	ODM_BB_DYNAMIC_ATC			= BIT14,
	
	/*  */
	/*  MAC DM section BIT 16-23 */
	/*  */
	ODM_MAC_EDCA_TURBO			= BIT16,
	ODM_MAC_EARLY_MODE			= BIT17,
	
	/*  */
	/*  RF ODM section BIT 24-31 */
	/*  */
	ODM_RF_TX_PWR_TRACK			= BIT24,
	ODM_RF_RX_GAIN_TRACK			= BIT25,
	ODM_RF_CALIBRATION				= BIT26,
	
}ODM_ABILITY_E;

/* 	ODM_CMNINFO_INTERFACE */
typedef enum tag_ODM_Support_Interface_Definition
{
	ODM_ITRF_PCIE 	=	0x1,
	ODM_ITRF_USB 	=	0x2,
	ODM_ITRF_SDIO 	=	0x4,
	ODM_ITRF_ALL 	=	0x7,
}ODM_INTERFACE_E;

/*  ODM_CMNINFO_IC_TYPE */
typedef enum tag_ODM_Support_IC_Type_Definition
{
	ODM_RTL8192S 	=	BIT0,
	ODM_RTL8192C 	=	BIT1,
	ODM_RTL8192D 	=	BIT2,
	ODM_RTL8723A 	=	BIT3,
	ODM_RTL8188E 	=	BIT4,
	ODM_RTL8812 	=	BIT5,
	ODM_RTL8821 	=	BIT6,
	ODM_RTL8192E 	=	BIT7,	
	ODM_RTL8723B	=	BIT8,
	ODM_RTL8813A	=	BIT9,	
	ODM_RTL8881A 	=	BIT10
}ODM_IC_TYPE_E;

#define ODM_IC_11N_SERIES		(ODM_RTL8192S|ODM_RTL8192C|ODM_RTL8192D|ODM_RTL8723A|ODM_RTL8188E)
#define ODM_IC_11AC_SERIES		(ODM_RTL8812)

/* ODM_CMNINFO_CUT_VER */
typedef enum tag_ODM_Cut_Version_Definition
{
	ODM_CUT_A 		=	1,
	ODM_CUT_B 		=	2,
	ODM_CUT_C 		=	3,
	ODM_CUT_D 		=	4,
	ODM_CUT_E 		=	5,
	ODM_CUT_F 		=	6,
	ODM_CUT_TEST 	=	7,
}ODM_CUT_VERSION_E;

/*  ODM_CMNINFO_FAB_VER */
typedef enum tag_ODM_Fab_Version_Definition
{
	ODM_TSMC 	=	0,
	ODM_UMC 	=	1,
}ODM_FAB_E;

/*  ODM_CMNINFO_RF_TYPE */
/*  */
/*  For example 1T2R (A+AB = BIT0|BIT4|BIT5) */
/*  */
typedef enum tag_ODM_RF_Path_Bit_Definition
{
	ODM_RF_TX_A 	=	BIT0,
	ODM_RF_TX_B 	=	BIT1,
	ODM_RF_TX_C	=	BIT2,
	ODM_RF_TX_D	=	BIT3,
	ODM_RF_RX_A	=	BIT4,
	ODM_RF_RX_B	=	BIT5,
	ODM_RF_RX_C	=	BIT6,
	ODM_RF_RX_D	=	BIT7,
}ODM_RF_PATH_E;


typedef enum tag_ODM_RF_Type_Definition
{
	ODM_1T1R 	=	0,
	ODM_1T2R 	=	1,
	ODM_2T2R	=	2,
	ODM_2T3R	=	3,
	ODM_2T4R	=	4,
	ODM_3T3R	=	5,
	ODM_3T4R	=	6,
	ODM_4T4R	=	7,
}ODM_RF_TYPE_E;


/*  */
/*  ODM Dynamic common info value definition */
/*  */

/* typedef enum _MACPHY_MODE_8192D{ */
/* 	SINGLEMAC_SINGLEPHY, */
/* 	DUALMAC_DUALPHY, */
/* 	DUALMAC_SINGLEPHY, */
/* MACPHY_MODE_8192D,*PMACPHY_MODE_8192D; */
/*  Above is the original define in MP driver. Please use the same define. THX. */
typedef enum tag_ODM_MAC_PHY_Mode_Definition
{
	ODM_SMSP	= 0,
	ODM_DMSP	= 1,
	ODM_DMDP	= 2,
}ODM_MAC_PHY_MODE_E;


typedef enum tag_BT_Coexist_Definition
{	
	ODM_BT_BUSY 		= 1,
	ODM_BT_ON 			= 2,
	ODM_BT_OFF 		= 3,
	ODM_BT_NONE 		= 4,
}ODM_BT_COEXIST_E;

/*  ODM_CMNINFO_OP_MODE */
typedef enum tag_Operation_Mode_Definition
{
	ODM_NO_LINK 		= BIT0,
	ODM_LINK 			= BIT1,
	ODM_SCAN 			= BIT2,
	ODM_POWERSAVE 	= BIT3,
	ODM_AP_MODE 		= BIT4,
	ODM_CLIENT_MODE	= BIT5,
	ODM_AD_HOC 		= BIT6,
	ODM_WIFI_DIRECT	= BIT7,
	ODM_WIFI_DISPLAY	= BIT8,
}ODM_OPERATION_MODE_E;

/*  ODM_CMNINFO_WM_MODE */
typedef enum tag_Wireless_Mode_Definition
{
	ODM_WM_UNKNOW	= 0x0,
	ODM_WM_B			= BIT0,
	ODM_WM_G			= BIT1,
	ODM_WM_A			= BIT2,
	ODM_WM_N24G		= BIT3,
	ODM_WM_N5G		= BIT4,
	ODM_WM_AUTO		= BIT5,
	ODM_WM_AC		= BIT6,
}ODM_WIRELESS_MODE_E;

/*  ODM_CMNINFO_BAND */
typedef enum tag_Band_Type_Definition
{
	ODM_BAND_2_4G 	= BIT0,
	ODM_BAND_5G 		= BIT1,

}ODM_BAND_TYPE_E;

/*  ODM_CMNINFO_SEC_CHNL_OFFSET */
typedef enum tag_Secondary_Channel_Offset_Definition
{
	ODM_DONT_CARE 	= 0,
	ODM_BELOW 		= 1,
	ODM_ABOVE 			= 2
}ODM_SEC_CHNL_OFFSET_E;

/*  ODM_CMNINFO_SEC_MODE */
typedef enum tag_Security_Definition
{
	ODM_SEC_OPEN 			= 0,
	ODM_SEC_WEP40 		= 1,
	ODM_SEC_TKIP 			= 2,
	ODM_SEC_RESERVE 		= 3,
	ODM_SEC_AESCCMP 		= 4,
	ODM_SEC_WEP104 		= 5,
	ODM_WEP_WPA_MIXED    = 6, /*  WEP + WPA */
	ODM_SEC_SMS4 			= 7,
}ODM_SECURITY_E;

/*  ODM_CMNINFO_BW */
typedef enum tag_Bandwidth_Definition
{	
	ODM_BW20M 		= 0,
	ODM_BW40M 		= 1,
	ODM_BW80M 		= 2,
	ODM_BW160M 		= 3,
	ODM_BW10M 		= 4,
}ODM_BW_E;

/*  ODM_CMNINFO_CHNL */

/*  ODM_CMNINFO_BOARD_TYPE */
typedef enum tag_Board_Definition
{
    ODM_BOARD_DEFAULT  	= 0, 	  /*  The DEFAULT case. */
    ODM_BOARD_MINICARD  	= BIT(0), /*  0 = non-mini card, 1= mini card. */
    ODM_BOARD_SLIM      		= BIT(1), /*  0 = non-slim card, 1 = slim card */
    ODM_BOARD_BT        		= BIT(2), /*  0 = without BT card, 1 = with BT */
    ODM_BOARD_EXT_PA    	= BIT(3), /*  0 = no 2G ext-PA, 1 = existing 2G ext-PA */
    ODM_BOARD_EXT_LNA   	= BIT(4), /*  0 = no 2G ext-LNA, 1 = existing 2G ext-LNA */
    ODM_BOARD_EXT_TRSW  	= BIT(5), /*  0 = no ext-TRSW, 1 = existing ext-TRSW */
    ODM_BOARD_EXT_PA_5G	= BIT(6), /*  0 = no 5G ext-PA, 1 = existing 5G ext-PA */
    ODM_BOARD_EXT_LNA_5G	= BIT(7), /*  0 = no 5G ext-LNA, 1 = existing 5G ext-LNA */
}ODM_BOARD_TYPE_E;

/*  ODM_CMNINFO_ONE_PATH_CCA */
typedef enum tag_CCA_Path
{
	ODM_CCA_2R			= 0,
	ODM_CCA_1R_A			= 1,
	ODM_CCA_1R_B			= 2,	
}ODM_CCA_PATH_E;


typedef struct _ODM_RA_Info_
{
	u8 RateID;
	u32 RateMask;
	u32 RAUseRate;
	u8 RateSGI;
	u8 RssiStaRA;
	u8 PreRssiStaRA;
	u8 SGIEnable;
	u8 DecisionRate;
	u8 PreRate;
	u8 HighestRate;
	u8 LowestRate;
	u32 NscUp;
	u32 NscDown;
	u16 RTY[5];
	u32 TOTAL;
	u16 DROP;/* Retry over or drop */
	u16 DROP1;/* LifeTime over */
	u8 Active;
	u16 RptTime;
	u8 RAWaitingCounter;
	u8 RAPendingCounter;	
	u8 PTActive;  /*  on or off */
	u8 PTTryState;  /*  0 trying state, 1 for decision state */
	u8 PTStage;  /*  0~6 */
	u8 PTStopCount; /* Stop PT counter */
	u8 PTPreRate;  /*  if rate change do PT */
	u8 PTPreRssi; /*  if RSSI change 5% do PT */
	u8 PTModeSS;  /*  decide whitch rate should do PT */
	u8 RAstage;  /*  StageRA, decide how many times RA will be done between PT */
	u8 PTSmoothFactor;
} ODM_RA_INFO_T,*PODM_RA_INFO_T;

typedef struct _IQK_MATRIX_REGS_SETTING{
	bool 	bIQKDone;
	s32		Value[1][IQK_Matrix_REG_NUM];
}IQK_MATRIX_REGS_SETTING,*PIQK_MATRIX_REGS_SETTING;

typedef struct ODM_RF_Calibration_Structure
{
	/* for tx power tracking */
	
	u32	RegA24; /*  for TempCCK */
	s32	RegE94;
	s32 	RegE9C;
	s32	RegEB4;
	s32	RegEBC;	

	/* u8 bTXPowerTracking; */
	u8  	TXPowercount;
	bool bTXPowerTrackingInit; 
	bool bTXPowerTracking;
	u8  	TxPowerTrackControl; /* for mp mode, turn off txpwrtracking as default */
	u8  	TM_Trigger;
    	u8  	InternalPA5G[2];	/* pathA / pathB */
	
	u8  	ThermalMeter[2];    /*  ThermalMeter, index 0 for RFIC0, and 1 for RFIC1 */
	u8  	ThermalValue;
	u8  	ThermalValue_LCK;
	u8  	ThermalValue_IQK;
	u8	ThermalValue_DPK;		
	u8	ThermalValue_AVG[AVG_THERMAL_NUM];
	u8	ThermalValue_AVG_index;		
	u8	ThermalValue_RxGain;
	u8	ThermalValue_Crystal;
	u8	ThermalValue_DPKstore;
	u8	ThermalValue_DPKtrack;
	bool	TxPowerTrackingInProgress;
	bool	bDPKenable;
	
	bool	bReloadtxpowerindex;	
	u8 	bRfPiEnable;
	u32 	TXPowerTrackingCallbackCnt; /* cosa add for debug */

	u8 	bCCKinCH14;
	u8 	CCK_index;
	u8 	OFDM_index[2];
	bool bDoneTxpower;
	s8	PowerIndexOffset;
	s8	DeltaPowerIndex;
	s8	DeltaPowerIndexLast;	
	bool bTxPowerChanged;
		
	u8 	ThermalValue_HP[HP_THERMAL_NUM];
	u8 	ThermalValue_HP_index;
	IQK_MATRIX_REGS_SETTING IQKMatrixRegSetting[IQK_Matrix_Settings_NUM];

	u8	Delta_IQK;
	u8	Delta_LCK;
	
	/* for IQK	 */
	u32 	RegC04;
	u32 	Reg874;
	u32 	RegC08;
	u32 	RegB68;
	u32 	RegB6C;
	u32 	Reg870;
	u32 	Reg860;
	u32 	Reg864;
	
	bool	bIQKInitialized;
	bool bLCKInProgress;
	bool	bAntennaDetected;
	u32	ADDA_backup[IQK_ADDA_REG_NUM];
	u32	IQK_MAC_backup[IQK_MAC_REG_NUM];
	u32	IQK_BB_backup_recover[9];
	u32	IQK_BB_backup[IQK_BB_REG_NUM];	

	/* for APK */
	u32 	APKoutput[2][2]; /* path A/B; output1_1a/output1_2a */
	u8 	bAPKdone;
	u8 	bAPKThermalMeterIgnore;
	u8 	bDPdone;
	u8 	bDPPathAOK;
	u8 	bDPPathBOK;
}ODM_RF_CAL_T,*PODM_RF_CAL_T;
/*  */
/*  ODM Dynamic common info value definition */
/*  */

typedef struct _FAST_ANTENNA_TRAINNING_
{
	u8	Bssid[6];
	u8	antsel_rx_keep_0;
	u8	antsel_rx_keep_1;
	u8	antsel_rx_keep_2;
	u32	antSumRSSI[7];
	u32	antRSSIcnt[7];
	u32	antAveRSSI[7];
	u8	FAT_State;
	u32	TrainIdx;
	u8	antsel_a[ODM_ASSOCIATE_ENTRY_NUM];
	u8	antsel_b[ODM_ASSOCIATE_ENTRY_NUM];
	u8	antsel_c[ODM_ASSOCIATE_ENTRY_NUM];
	u32	MainAnt_Sum[ODM_ASSOCIATE_ENTRY_NUM];
	u32	AuxAnt_Sum[ODM_ASSOCIATE_ENTRY_NUM];
	u32	MainAnt_Cnt[ODM_ASSOCIATE_ENTRY_NUM];
	u32	AuxAnt_Cnt[ODM_ASSOCIATE_ENTRY_NUM];
	u8	RxIdleAnt;
	bool		bBecomeLinked;

}FAT_T,*pFAT_T;

typedef enum _FAT_STATE
{
	FAT_NORMAL_STATE			= 0,
	FAT_TRAINING_STATE 		= 1,
}FAT_STATE_E, *PFAT_STATE_E;

typedef enum _ANT_DIV_TYPE
{
	NO_ANTDIV					= 0xFF,	
	CG_TRX_HW_ANTDIV			= 0x01,
	CGCS_RX_HW_ANTDIV 		= 0x02,
	FIXED_HW_ANTDIV         		= 0x03,
	CG_TRX_SMART_ANTDIV		= 0x04,
	CGCS_RX_SW_ANTDIV		= 0x05,
	
}ANT_DIV_TYPE_E, *PANT_DIV_TYPE_E;













/*  */
/*  2011/09/22 MH Copy from SD4 defined structure. We use to support PHY DM integration. */
/*  */
typedef  struct DM_Out_Source_Dynamic_Mechanism_Structure
{
	/* RT_TIMER 	FastAntTrainingTimer; */
	/*  */
	/* 	Add for different team use temporarily */
	/*  */
	struct adapter *		Adapter;		/*  For CE/NIC team */
	prtl8192cd_priv	priv;			/*  For AP/ADSL team */
	/*  WHen you use Adapter or priv pointer, you must make sure the pointer is ready. */
	bool			odm_ready;

	rtl8192cd_priv		fake_priv;
	u64			DebugComponents;
	u32			DebugLevel;
	
/*  ODM HANDLE, DRIVER NEEDS NOT TO HOOK------ */
	bool			bCckHighPower; 
	u8			RFPathRxEnable;		/*  ODM_CMNINFO_RFPATH_ENABLE */
	u8			ControlChannel;
/*  ODM HANDLE, DRIVER NEEDS NOT TO HOOK------ */

/* REMOVED COMMON INFO---------- */
	/* u8				PseudoMacPhyMode; */
	/* bool			*BTCoexist; */
	/* bool			PseudoBtCoexist; */
	/* u8				OPMode; */
	/* bool			bAPMode; */
	/* bool			bClientMode; */
	/* bool			bAdHocMode; */
	/* bool			bSlaveOfDMSP; */
/* REMOVED COMMON INFO---------- */


/* 1  COMMON INFORMATION */

	/*  */
	/*  Init Value */
	/*  */
/* HOOK BEFORE REG INIT----------- */
	/*  ODM Platform info AP/ADSL/CE/MP = 1/2/3/4 */
	u8			SupportPlatform;		
	/*  ODM Support Ability DIG/RATR/TX_PWR_TRACK/ ¡K¡K = 1/2/3/¡K */
	u32			SupportAbility;
	/*  ODM PCIE/USB/SDIO/GSPI = 0/1/2/3 */
	u8			SupportInterface;			
	/*  ODM composite or independent. Bit oriented/ 92C+92D+ .... or any other type = 1/2/3/... */
	u32			SupportICType;	
	/*  Cut Version TestChip/A-cut/B-cut... = 0/1/2/3/... */
	u8			CutVersion;
	/*  Fab Version TSMC/UMC = 0/1 */
	u8			FabVersion;
	/*  RF Type 4T4R/3T3R/2T2R/1T2R/1T1R/... */
	u8			RFType;
	/*  Board Type Normal/HighPower/MiniCard/SLIM/Combo/... = 0/1/2/3/4/... */
	u8			BoardType;
	/*  with external LNA  NO/Yes = 0/1 */
	u8			ExtLNA;
	/*  with external PA  NO/Yes = 0/1 */
	u8			ExtPA;
	/*  with external TRSW  NO/Yes = 0/1 */
	u8			ExtTRSW;
	u8			PatchID; /* Customer ID */
	bool			bInHctTest;
	bool			bWIFITest;

	bool			bDualMacSmartConcurrent;
	u32			BK_SupportAbility;
	u8			AntDivType;
/* HOOK BEFORE REG INIT----------- */

	/*  */
	/*  Dynamic Value */
	/* 	 */
/*  POINTER REFERENCE----------- */

	u8			u8_temp;
	bool			bool_temp;
	struct adapter *_temp;
	
	/*  MAC PHY Mode SMSP/DMSP/DMDP = 0/1/2 */
	u8			*pMacPhyMode;
	/* TX Unicast byte count */
	u64			*pNumTxBytesUnicast;
	/* RX Unicast byte count */
	u64			*pNumRxBytesUnicast;
	/*  Wireless mode B/G/A/N = BIT0/BIT1/BIT2/BIT3 */
	u8			*pWirelessMode; /* ODM_WIRELESS_MODE_E */
	/*  Frequence band 2.4G/5G = 0/1 */
	u8			*pBandType;
	/*  Secondary channel offset don't_care/below/above = 0/1/2 */
	u8			*pSecChOffset;
	/*  Security mode Open/WEP/AES/TKIP = 0/1/2/3 */
	u8			*pSecurity;
	/*  BW info 20M/40M/80M = 0/1/2 */
	u8			*pBandWidth;
 	/*  Central channel location Ch1/Ch2/.... */
	u8			*pChannel;	/* central channel number */
	/*  Common info for 92D DMSP */
	
	bool			*pbGetValueFromOtherMac;
	struct adapter *		*pBuddyAdapter;
	bool			*pbMasterOfDMSP; /* MAC0: master, MAC1: slave */
	/*  Common info for Status */
	bool			*pbScanInProcess;
	bool			*pbPowerSaving;
	/*  CCA Path 2-path/path-A/path-B = 0/1/2; using ODM_CCA_PATH_E. */
	u8			*pOnePathCCA;
	/* pMgntInfo->AntennaTest */
	u8			*pAntennaTest;
	bool			*pbNet_closed;
/*  POINTER REFERENCE----------- */
	/*  */
/* CALL BY VALUE------------- */
	bool			bLinkInProcess;
	bool			bWIFI_Direct;
	bool			bWIFI_Display;
	bool			bLinked;
	bool			bsta_state;
	u8			RSSI_Min;	
	u8          	InterfaceIndex; /*  Add for 92D  dual MAC: 0--Mac0 1--Mac1 */
	bool         	bIsMPChip;
	bool			bOneEntryOnly;
	/*  Common info for BTDM */
	bool			bBtDisabled;			/*  BT is disabled */
	bool			bBtConnectProcess;	/*  BT HS is under connection progress. */
	u8			btHsRssi;				/*  BT HS mode wifi rssi value. */
	bool			bBtHsOperation;		/*  BT HS mode is under progress */
	u8			btHsDigVal;			/*  use BT rssi to decide the DIG value */
	bool			bBtDisableEdcaTurbo;	/*  Under some condition, don't enable the EDCA Turbo */
	bool			bBtLimitedDig;   		/*  BT is busy. */
/* CALL BY VALUE------------- */
	u8			RSSI_A;
	u8			RSSI_B;
	u64			RSSI_TRSW;	
	u64			RSSI_TRSW_H;
	u64			RSSI_TRSW_L;	
	u64			RSSI_TRSW_iso;

	u8			RxRate;
	bool			StopDIG;
	u8			TxRate;
	u8			LinkedInterval;
	u8			preChannel;
	u32			TxagcOffsetValueA;
	bool			IsTxagcOffsetPositiveA;
	u32			TxagcOffsetValueB;
	bool			IsTxagcOffsetPositiveB;
	u64			lastTxOkCnt;
	u64			lastRxOkCnt;
	u32			BbSwingOffsetA;
	bool			IsBbSwingOffsetPositiveA;
	u32			BbSwingOffsetB;
	bool			IsBbSwingOffsetPositiveB;
	s8			TH_L2H_ini;
	s8			TH_EDCCA_HL_diff;
	u32			IGI_Base;
	u32			IGI_target;
	bool			ForceEDCCA;
	u8			AdapEn_RSSI;
	u8			AntType;
	u8			antdiv_rssi;
	u8			antdiv_period;
	u32			Force_TH_H;
	u32			Force_TH_L;
	u8			IGI_LowerBound;
	
	/* 2 Define STA info. */
	/*  _ODM_STA_INFO */
	/*  2012/01/12 MH For MP, we need to reduce one array pointer for default port.?? */
	PSTA_INFO_T		pODM_StaInfo[ODM_ASSOCIATE_ENTRY_NUM];

#if (RATE_ADAPTIVE_SUPPORT == 1)
	u16 			CurrminRptTime;
	ODM_RA_INFO_T   RAInfo[ODM_ASSOCIATE_ENTRY_NUM]; /* Use MacID as array index. STA MacID=0, VWiFi Client MacID={1, ODM_ASSOCIATE_ENTRY_NUM-1} */
#endif
	/*  */
	/*  2012/02/14 MH Add to share 88E ra with other SW team. */
	/*  We need to colelct all support abilit to a proper area. */
	/*  */
	bool				RaSupport88E;

	/*  Define ........... */

	/*  Latest packet phy info (ODM write) */
	ODM_PHY_DBG_INFO_T	 PhyDbgInfo;
	/* PHY_INFO_88E		PhyInfo; */

	/*  Latest packet phy info (ODM write) */
	ODM_MAC_INFO		*pMacInfo;
	/* MAC_INFO_88E		MacInfo; */

	/*  Different Team independt structure?? */

	/*  */
	/* TX_RTP_CMN		TX_retrpo; */
	/* TX_RTP_88E		TX_retrpo; */
	/* TX_RTP_8195		TX_retrpo; */

	/*  */
	/* ODM Structure */
	/*  */
	FAT_T		DM_FatTable;
	DIG_T		DM_DigTable;
	PS_T		DM_PSTable;
	Pri_CCA_T	DM_PriCCA;
	RXHP_T		DM_RXHP_Table;
	false_ALARM_STATISTICS	FalseAlmCnt;
	false_ALARM_STATISTICS	FlaseAlmCntBuddyAdapter;
	SWAT_T		DM_SWAT_Table;
	bool		RSSI_test;
	
	EDCA_T		DM_EDCA_Table;
	u32		WMMEDCA_BE;
	/*  Copy from SD4 structure */
	/*  */
	/*  ================================================== */
	/*  */

	bool			*pbDriverStopped;
	bool			*pbDriverIsGoingToPnpSetPowerSleep;
	bool			*pinit_adpt_in_progress;

	/* PSD */
	bool			bUserAssignLevel;
	RT_TIMER 		PSDTimer;
	u8			RSSI_BT;			/* come from BT */
	bool			bPSDinProcess;
	bool			bDMInitialGainEnable;
	
	/* for rate adaptive, in fact,  88c/92c fw will handle this */
	u8			bUseRAMask;

	ODM_RATE_ADAPTIVE	RateAdaptive;


	ODM_RF_CAL_T	RFCalibrateInfo;
	
	/*  */
	/*  TX power tracking */
	/*  */
	u8			BbSwingIdxOfdm;
	u8			BbSwingIdxOfdmCurrent;
	u8			BbSwingIdxOfdmBase;
	bool			BbSwingFlagOfdm;
	u8			BbSwingIdxCck;
	u8			BbSwingIdxCckCurrent;
	u8			BbSwingIdxCckBase;
	u8			DefaultOfdmIndex;
	u8			DefaultCckIndex;	
	bool			BbSwingFlagCck;
	
	
	u8			*mp_mode; 
	/*  */
	/*  ODM system resource. */
	/*  */

	/*  ODM relative time. */
	RT_TIMER 				PathDivSwitchTimer;
	/* 2011.09.27 add for Path Diversity */
	RT_TIMER				CCKPathDiversityTimer;
	RT_TIMER 	FastAntTrainingTimer;
	
	/*  ODM relative workitem. */
} DM_ODM_T, *PDM_ODM_T;		/*  DM_Dynamic_Mechanism_Structure */

 typedef enum _ODM_RF_CONTENT{
	odm_radioa_txt = 0x1000,
	odm_radiob_txt = 0x1001,
	odm_radioc_txt = 0x1002,
	odm_radiod_txt = 0x1003
} ODM_RF_CONTENT;

typedef enum _ODM_BB_Config_Type{
    CONFIG_BB_PHY_REG,   
    CONFIG_BB_AGC_TAB,   
    CONFIG_BB_AGC_TAB_2G,
    CONFIG_BB_AGC_TAB_5G, 
    CONFIG_BB_PHY_REG_PG,  
} ODM_BB_Config_Type, *PODM_BB_Config_Type;

/*  Status code */
typedef enum _RT_STATUS{
	RT_STATUS_SUCCESS,
	RT_STATUS_FAILURE,
	RT_STATUS_PENDING,
	RT_STATUS_RESOURCE,
	RT_STATUS_INVALID_CONTEXT,
	RT_STATUS_INVALID_PARAMETER,
	RT_STATUS_NOT_SUPPORT,
	RT_STATUS_OS_API_FAILED,
}RT_STATUS,*PRT_STATUS;

/* 3=========================================================== */
/* 3 DIG */
/* 3=========================================================== */

typedef enum tag_Dynamic_Init_Gain_Operation_Type_Definition
{
	DIG_TYPE_THRESH_HIGH	= 0,
	DIG_TYPE_THRESH_LOW	= 1,
	DIG_TYPE_BACKOFF		= 2,
	DIG_TYPE_RX_GAIN_MIN	= 3,
	DIG_TYPE_RX_GAIN_MAX	= 4,
	DIG_TYPE_ENABLE 		= 5,
	DIG_TYPE_DISABLE 		= 6,	
	DIG_OP_TYPE_MAX
}DM_DIG_OP_E;
/*
typedef enum tag_CCK_Packet_Detection_Threshold_Type_Definition
{
	CCK_PD_STAGE_LowRssi = 0,
	CCK_PD_STAGE_HighRssi = 1,
	CCK_PD_STAGE_MAX = 3,
}DM_CCK_PDTH_E;

typedef enum tag_DIG_EXT_PORT_ALGO_Definition
{
	DIG_EXT_PORT_STAGE_0 = 0,
	DIG_EXT_PORT_STAGE_1 = 1,
	DIG_EXT_PORT_STAGE_2 = 2,
	DIG_EXT_PORT_STAGE_3 = 3,
	DIG_EXT_PORT_STAGE_MAX = 4,
}DM_DIG_EXT_PORT_ALG_E;

typedef enum tag_DIG_Connect_Definition
{
	DIG_STA_DISCONNECT = 0,	
	DIG_STA_CONNECT = 1,
	DIG_STA_BEFORE_CONNECT = 2,
	DIG_MultiSTA_DISCONNECT = 3,
	DIG_MultiSTA_CONNECT = 4,
	DIG_CONNECT_MAX
}DM_DIG_CONNECT_E;


#define DM_MultiSTA_InitGainChangeNotify(Event) {DM_DigTable.CurMultiSTAConnectState = Event;}

#define DM_MultiSTA_InitGainChangeNotify_CONNECT(_ADAPTER)	\
	DM_MultiSTA_InitGainChangeNotify(DIG_MultiSTA_CONNECT)

#define DM_MultiSTA_InitGainChangeNotify_DISCONNECT(_ADAPTER)	\
	DM_MultiSTA_InitGainChangeNotify(DIG_MultiSTA_DISCONNECT)
*/

#define		DM_DIG_THRESH_HIGH			40
#define		DM_DIG_THRESH_LOW			35

#define		DMfalseALARM_THRESH_LOW	400
#define		DMfalseALARM_THRESH_HIGH	1000

#define		DM_DIG_MAX_NIC				0x4A
#define		DM_DIG_MIN_NIC				0x1e /* 0x22/0x1c */

#define		DM_DIG_MAX_AP					0x32
#define		DM_DIG_MIN_AP					0x20

#define		DM_DIG_MAX_NIC_HP			0x46
#define		DM_DIG_MIN_NIC_HP				0x2e

#define		DM_DIG_MAX_AP_HP				0x42
#define		DM_DIG_MIN_AP_HP				0x30

/* vivi 92c&92d has different definition, 20110504 */
/* this is for 92c */
#define		DM_DIG_FA_TH0				0x200/* 0x20 */
#define		DM_DIG_FA_TH1				0x300/* 0x100 */
#define		DM_DIG_FA_TH2				0x400/* 0x200 */
/* this is for 92d */
#define		DM_DIG_FA_TH0_92D			0x100
#define		DM_DIG_FA_TH1_92D			0x400
#define		DM_DIG_FA_TH2_92D			0x600

#define		DM_DIG_BACKOFF_MAX			12
#define		DM_DIG_BACKOFF_MIN			-4
#define		DM_DIG_BACKOFF_DEFAULT		10

/* 3=========================================================== */
/* 3 AGC RX High Power Mode */
/* 3=========================================================== */
#define          LNA_Low_Gain_1                      0x64
#define          LNA_Low_Gain_2                      0x5A
#define          LNA_Low_Gain_3                      0x58

#define          FA_RXHP_TH1                           5000
#define          FA_RXHP_TH2                           1500
#define          FA_RXHP_TH3                             800
#define          FA_RXHP_TH4                             600
#define          FA_RXHP_TH5                             500

/* 3=========================================================== */
/* 3 EDCA */
/* 3=========================================================== */

/* 3=========================================================== */
/* 3 Dynamic Tx Power */
/* 3=========================================================== */
/* Dynamic Tx Power Control Threshold */
#define		TX_POWER_NEAR_FIELD_THRESH_LVL2	74
#define		TX_POWER_NEAR_FIELD_THRESH_LVL1	67
#define		TX_POWER_NEAR_FIELD_THRESH_AP		0x3F

#define		TxHighPwrLevel_Normal		0	
#define		TxHighPwrLevel_Level1		1
#define		TxHighPwrLevel_Level2		2
#define		TxHighPwrLevel_BT1			3
#define		TxHighPwrLevel_BT2			4
#define		TxHighPwrLevel_15			5
#define		TxHighPwrLevel_35			6
#define		TxHighPwrLevel_50			7
#define		TxHighPwrLevel_70			8
#define		TxHighPwrLevel_100			9

/* 3=========================================================== */
/* 3 Tx Power Tracking */
/* 3=========================================================== */

/* 3=========================================================== */
/* 3 Rate Adaptive */
/* 3=========================================================== */
#define		DM_RATR_STA_INIT			0
#define		DM_RATR_STA_HIGH			1
#define 		DM_RATR_STA_MIDDLE		2
#define 		DM_RATR_STA_LOW			3

/* 3=========================================================== */
/* 3 BB Power Save */
/* 3=========================================================== */


typedef enum tag_1R_CCA_Type_Definition
{
	CCA_1R =0,
	CCA_2R = 1,
	CCA_MAX = 2,
}DM_1R_CCA_E;

typedef enum tag_RF_Type_Definition
{
	RF_Save =0,
	RF_Normal = 1,
	RF_MAX = 2,
}DM_RF_E;

/* 3=========================================================== */
/* 3 Antenna Diversity */
/* 3=========================================================== */
typedef enum tag_SW_Antenna_Switch_Definition
{
	Antenna_A = 1,
	Antenna_B = 2,
	Antenna_MAX = 3,
}DM_SWAS_E;


/*  Maximal number of antenna detection mechanism needs to perform, added by Roger, 2011.12.28. */
#define	MAX_ANTENNA_DETECTION_CNT	10 

/*  */
/*  Extern Global Variables. */
/*  */
#define	OFDM_TABLE_SIZE_92C 	37
#define	OFDM_TABLE_SIZE_92D 	43
#define	CCK_TABLE_SIZE		33

extern	u32 OFDMSwingTable[OFDM_TABLE_SIZE_92D];
extern	u8 CCKSwingTable_Ch1_Ch13[CCK_TABLE_SIZE][8];
extern	u8 CCKSwingTable_Ch14 [CCK_TABLE_SIZE][8];



/*  */
/*  check Sta pointer valid or not */
/*  */
#define IS_STA_VALID(pSta)		(pSta)

/*  20100514 Joseph: Add definition for antenna switching test after link. */
/*  This indicates two different the steps.  */
/*  In SWAW_STEP_PEAK, driver needs to switch antenna and listen to the signal on the air. */
/*  In SWAW_STEP_DETERMINE, driver just compares the signal captured in SWAW_STEP_PEAK */
/*  with original RSSI to determine if it is necessary to switch antenna. */
#define SWAW_STEP_PEAK		0
#define SWAW_STEP_DETERMINE	1

void ODM_Write_DIG(PDM_ODM_T	pDM_Odm, u8	CurrentIGI);
void ODM_Write_CCK_CCA_Thres(PDM_ODM_T	pDM_Odm, u8	CurCCK_CCAThres);

void
ODM_SetAntenna(
	PDM_ODM_T	pDM_Odm,
	u8		Antenna);


#define dm_RF_Saving	ODM_RF_Saving
void ODM_RF_Saving(	PDM_ODM_T	pDM_Odm,
							u8		bForceInNormal );

#define SwAntDivRestAfterLink	ODM_SwAntDivRestAfterLink
void ODM_SwAntDivRestAfterLink(	PDM_ODM_T	pDM_Odm);

#define dm_CheckTXPowerTracking 	ODM_TXPowerTrackingCheck
void	
ODM_TXPowerTrackingCheck(
		PDM_ODM_T		pDM_Odm
	);

bool 
ODM_RAStateCheck(
		PDM_ODM_T		pDM_Odm,
		s32			RSSI,
		bool			bForceUpdate,
		u8 *			pRATRState
	);

#define dm_SWAW_RSSI_Check	ODM_SwAntDivChkPerPktRssi
void ODM_SwAntDivChkPerPktRssi(	
	PDM_ODM_T		pDM_Odm,
	u8			StationID,
	PODM_PHY_INFO_T pPhyInfo
	);

u32 ConvertTo_dB(u32 Value);

u32
GetPSDData(
	PDM_ODM_T	pDM_Odm,
	unsigned int 	point,
	u8 initial_gain_psd);

void
odm_DIGbyRSSI_LPS(
		PDM_ODM_T		pDM_Odm
	);

u32 ODM_Get_Rate_Bitmap(
	PDM_ODM_T	pDM_Odm,
	u32		macid,
	u32 		ra_mask,	
	u8 		rssi_level);
	
void ODM_DMInit(PDM_ODM_T	pDM_Odm);

void
ODM_DMWatchdog(
		PDM_ODM_T	pDM_Odm		/*  For common use in the future */
	);

void
ODM_CmnInfoInit(
		PDM_ODM_T		pDM_Odm,
		ODM_CMNINFO_E	CmnInfo,
		u32			Value	
	);

void
ODM_CmnInfoHook(
		PDM_ODM_T		pDM_Odm,
		ODM_CMNINFO_E	CmnInfo,
		void *			pValue	
	);

void
ODM_CmnInfoPtrArrayHook(
		PDM_ODM_T		pDM_Odm,
		ODM_CMNINFO_E	CmnInfo,
		u16			Index,
		void *			pValue	
	);

void
ODM_CmnInfoUpdate(
		PDM_ODM_T		pDM_Odm,
		u32			CmnInfo,
		u64			Value	
	);

void 
ODM_InitAllTimers(
    PDM_ODM_T	pDM_Odm 
    );

void 
ODM_CancelAllTimers(
    PDM_ODM_T    pDM_Odm 
    );

void
ODM_ReleaseAllTimers(
    PDM_ODM_T	pDM_Odm 
    );

void
ODM_ResetIQKResult(
    PDM_ODM_T pDM_Odm 
    );

void
ODM_AntselStatistics_88C(
		PDM_ODM_T		pDM_Odm,
		u8			MacId,
		u32			PWDBAll,
		bool			isCCKrate
);

void
ODM_SingleDualAntennaDefaultSetting(
		PDM_ODM_T		pDM_Odm
	);

bool
ODM_SingleDualAntennaDetection(
		PDM_ODM_T		pDM_Odm,
		u8			mode
	);


void odm_dtc(PDM_ODM_T pDM_Odm);

#endif
