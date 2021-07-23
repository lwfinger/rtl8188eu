/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#ifndef __HAL_COM_PHYCFG_H__
#define __HAL_COM_PHYCFG_H__

#define		PathA                     			0x0	/* Useless */
#define		PathB			0x1
#define		PathC			0x2
#define		PathD			0x3

typedef enum _RF_TX_NUM {
	RF_1TX = 0,
	RF_2TX,
	RF_3TX,
	RF_4TX,
	RF_MAX_TX_NUM,
	RF_TX_NUM_NONIMPLEMENT,
} RF_TX_NUM;

#define MAX_POWER_INDEX		0x3F

typedef enum _REGULATION_TXPWR_LMT {
	TXPWR_LMT_FCC = 0,
	TXPWR_LMT_MKK = 1,
	TXPWR_LMT_ETSI = 2,
	TXPWR_LMT_WW = 3,

	TXPWR_LMT_MAX_REGULATION_NUM = 4
} REGULATION_TXPWR_LMT;

#define TX_PWR_LMT_REF_VHT_FROM_HT	BIT0
#define TX_PWR_LMT_REF_HT_FROM_VHT	BIT1

/*------------------------------Define structure----------------------------*/
typedef struct _BB_REGISTER_DEFINITION {
	u32 rfintfs;			/* set software control: */
	/*		0x870~0x877[8 bytes] */

	u32 rfintfo; 			/* output data: */
	/*		0x860~0x86f [16 bytes] */

	u32 rfintfe; 			/* output enable: */
	/*		0x860~0x86f [16 bytes] */

	u32 rf3wireOffset;	/* LSSI data: */
	/*		0x840~0x84f [16 bytes] */

	u32 rfHSSIPara2;	/* wire parameter control2 :  */
	/*		0x824~0x827,0x82c~0x82f, 0x834~0x837, 0x83c~0x83f [16 bytes] */

	u32 rfLSSIReadBack;	/* LSSI RF readback data SI mode */
	/*		0x8a0~0x8af [16 bytes] */

	u32 rfLSSIReadBackPi;	/* LSSI RF readback data PI mode 0x8b8-8bc for Path A and B */

} BB_REGISTER_DEFINITION_T, *PBB_REGISTER_DEFINITION_T;


/* ---------------------------------------------------------------------- */
u8
PHY_GetTxPowerByRateBase(
	PADAPTER		Adapter,
	u8				Band,
	u8				RfPath,
	u8				TxNum,
	RATE_SECTION	RateSection
);

void
PHY_GetRateValuesOfTxPowerByRate(
	PADAPTER pAdapter,
	u32 RegAddr,
	u32 BitMask,
	u32 Value,
	u8 *Rate,
	s8 *PwrByRateVal,
	u8 *RateNum
);

u8
PHY_GetRateIndexOfTxPowerByRate(
	u8	Rate
);

void
phy_set_tx_power_index_by_rate_section(
	PADAPTER		pAdapter,
	u8				RFPath,
	u8				Channel,
	u8				RateSection
);

s8
_PHY_GetTxPowerByRate(
	PADAPTER	pAdapter,
	u8			Band,
	u8			RFPath,
	u8			TxNum,
	u8			RateIndex
);

s8
PHY_GetTxPowerByRate(
	PADAPTER	pAdapter,
	u8			Band,
	u8			RFPath,
	u8			TxNum,
	u8			RateIndex
);

#ifdef CONFIG_PHYDM_POWERTRACK_BY_TSSI
s8
PHY_GetTxPowerByRateOriginal(
	PADAPTER	pAdapter,
	u8			Band,
	u8			RFPath,
	u8			TxNum,
	u8			Rate
);
#endif

void
PHY_SetTxPowerByRate(
	PADAPTER	pAdapter,
	u8			Band,
	u8			RFPath,
	u8			TxNum,
	u8			Rate,
	s8			Value
);

void
phy_set_tx_power_level_by_path(
	PADAPTER	Adapter,
	u8			channel,
	u8			path
);

void
PHY_SetTxPowerIndexByRateArray(
	PADAPTER		pAdapter,
	u8				RFPath,
	CHANNEL_WIDTH	BandWidth,
	u8				Channel,
	u8				*Rates,
	u8				RateArraySize
);

void
PHY_InitTxPowerByRate(
	PADAPTER	pAdapter
);

void
phy_store_tx_power_by_rate(
	PADAPTER	pAdapter,
	u32			Band,
	u32			RfPath,
	u32			TxNum,
	u32			RegAddr,
	u32			BitMask,
	u32			Data
);

void
PHY_TxPowerByRateConfiguration(
	PADAPTER			pAdapter
);

u8
PHY_GetTxPowerIndexBase(
	PADAPTER		pAdapter,
	u8				RFPath,
	u8				Rate,
	CHANNEL_WIDTH	BandWidth,
	u8				Channel,
	bool *		bIn24G
);

s8
PHY_GetTxPowerLimit(
	PADAPTER		Adapter,
	u32				RegPwrTblSel,
	BAND_TYPE		Band,
	CHANNEL_WIDTH	Bandwidth,
	u8				RfPath,
	u8				DataRate,
	u8				Channel
);

s8
PHY_GetTxPowerLimit_no_sc(
	PADAPTER			Adapter,
	u32					RegPwrTblSel,
	BAND_TYPE			Band,
	CHANNEL_WIDTH		Bandwidth,
	u8					RfPath,
	u8					DataRate,
	u8					Channel
);

#ifdef CONFIG_PHYDM_POWERTRACK_BY_TSSI
s8
PHY_GetTxPowerLimitOriginal(
	PADAPTER		Adapter,
	u32				RegPwrTblSel,
	BAND_TYPE		Band,
	CHANNEL_WIDTH	Bandwidth,
	u8				RfPath,
	u8				DataRate,
	u8				Channel
);
#endif

void
PHY_ConvertTxPowerLimitToPowerIndex(
	PADAPTER			Adapter
);

void
PHY_InitTxPowerLimit(
	PADAPTER			Adapter
);

s8
PHY_GetTxPowerTrackingOffset(
	PADAPTER	pAdapter,
	u8			Rate,
	u8			RFPath
);

struct txpwr_idx_comp {
	u8 base;
	s8 by_rate;
	s8 limit;
	s8 tpt;
	s8 ebias;
};

u8
phy_get_tx_power_index(
	PADAPTER			pAdapter,
	u8					RFPath,
	u8					Rate,
	CHANNEL_WIDTH		BandWidth,
	u8					Channel
);

void
PHY_SetTxPowerIndex(
	PADAPTER		pAdapter,
	u32				PowerIndex,
	u8				RFPath,
	u8				Rate
);

void dump_tx_power_idx_title(void *sel, _adapter *adapter);
void dump_tx_power_idx_by_path_rs(void *sel, _adapter *adapter, u8 rfpath, u8 rs);
void dump_tx_power_idx(void *sel, _adapter *adapter);

bool phy_is_tx_power_limit_needed(_adapter *adapter);
bool phy_is_tx_power_by_rate_needed(_adapter *adapter);
int phy_load_tx_power_by_rate(_adapter *adapter, u8 chk_file);
int phy_load_tx_power_limit(_adapter *adapter, u8 chk_file);
void phy_load_tx_power_ext_info(_adapter *adapter, u8 chk_file);
void phy_reload_tx_power_ext_info(_adapter *adapter);
void phy_reload_default_tx_power_ext_info(_adapter *adapter);

const struct map_t *hal_pg_txpwr_def_info(_adapter *adapter);

void dump_pg_txpwr_info_2g(void *sel, TxPowerInfo24G *txpwr_info, u8 rfpath_num, u8 max_tx_cnt);
void dump_pg_txpwr_info_5g(void *sel, TxPowerInfo5G *txpwr_info, u8 rfpath_num, u8 max_tx_cnt);

void dump_hal_txpwr_info_2g(void *sel, _adapter *adapter, u8 rfpath_num, u8 max_tx_cnt);
void dump_hal_txpwr_info_5g(void *sel, _adapter *adapter, u8 rfpath_num, u8 max_tx_cnt);

void hal_load_txpwr_info(
	_adapter *adapter,
	TxPowerInfo24G *pwr_info_2g,
	TxPowerInfo5G *pwr_info_5g,
	u8 *pg_data
);

void dump_tx_power_ext_info(void *sel, _adapter *adapter);
void dump_target_tx_power(void *sel, _adapter *adapter);
void dump_tx_power_by_rate(void *sel, _adapter *adapter);
void dump_tx_power_limit(void *sel, _adapter *adapter);

int rtw_get_phy_file_path(_adapter *adapter, const char *file_name);

#ifdef CONFIG_LOAD_PHY_PARA_FROM_FILE
#define MAC_FILE_FW_NIC			"FW_NIC.bin"
#define MAC_FILE_FW_WW_IMG		"FW_WoWLAN.bin"
#define PHY_FILE_MAC_REG		"MAC_REG.txt"

#define PHY_FILE_AGC_TAB		"AGC_TAB.txt"
#define PHY_FILE_PHY_REG		"PHY_REG.txt"
#define PHY_FILE_PHY_REG_MP		"PHY_REG_MP.txt"
#define PHY_FILE_PHY_REG_PG		"PHY_REG_PG.txt"

#define PHY_FILE_RADIO_A		"RadioA.txt"
#define PHY_FILE_RADIO_B		"RadioB.txt"
#define PHY_FILE_RADIO_C		"RadioC.txt"
#define PHY_FILE_RADIO_D		"RadioD.txt"
#define PHY_FILE_TXPWR_TRACK	"TxPowerTrack.txt"
#define PHY_FILE_TXPWR_LMT		"TXPWR_LMT.txt"

#define PHY_FILE_WIFI_ANT_ISOLATION	"wifi_ant_isolation.txt"

#define MAX_PARA_FILE_BUF_LEN	25600

#define LOAD_MAC_PARA_FILE				BIT0
#define LOAD_BB_PARA_FILE					BIT1
#define LOAD_BB_PG_PARA_FILE				BIT2
#define LOAD_BB_MP_PARA_FILE				BIT3
#define LOAD_RF_PARA_FILE					BIT4
#define LOAD_RF_TXPWR_TRACK_PARA_FILE	BIT5
#define LOAD_RF_TXPWR_LMT_PARA_FILE		BIT6

int phy_ConfigMACWithParaFile(PADAPTER	Adapter, char	*pFileName);
int phy_ConfigBBWithParaFile(PADAPTER	Adapter, char	*pFileName, u32	ConfigType);
int phy_ConfigBBWithPgParaFile(PADAPTER	Adapter, const char *pFileName);
int phy_ConfigBBWithMpParaFile(PADAPTER	Adapter, char	*pFileName);
int PHY_ConfigRFWithParaFile(PADAPTER	Adapter, char	*pFileName, u8	eRFPath);
int PHY_ConfigRFWithTxPwrTrackParaFile(PADAPTER	Adapter, char	*pFileName);
int PHY_ConfigRFWithPowerLimitTableParaFile(PADAPTER	Adapter, const char *pFileName);
void phy_free_filebuf_mask(_adapter *padapter, u8 mask);
void phy_free_filebuf(_adapter *padapter);
#endif /* CONFIG_LOAD_PHY_PARA_FROM_FILE */

#endif /* __HAL_COMMON_H__ */
