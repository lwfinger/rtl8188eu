/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#ifndef __INC_HAL8188EPHYCFG_H__
#define __INC_HAL8188EPHYCFG_H__


/*--------------------------Define Parameters-------------------------------*/
#define LOOP_LIMIT				5
#define MAX_STALL_TIME			50		/* us */
#define AntennaDiversityValue		0x80	/* (Adapter->bSoftwareAntennaDiversity ? 0x00 : 0x80) */
#define MAX_TXPWR_IDX_NMODE_92S	63
#define Reset_Cnt_Limit			3

#ifdef CONFIG_PCI_HCI
	#define MAX_AGGR_NUM	0x0B
#else
	#define MAX_AGGR_NUM	0x07
#endif /* CONFIG_PCI_HCI */


/*--------------------------Define Parameters-------------------------------*/


/*------------------------------Define structure----------------------------*/

#define	MAX_TX_COUNT_8188E			1

/* BB/RF related */


/*------------------------------Define structure----------------------------*/


/*------------------------Export global variable----------------------------*/
/*------------------------Export global variable----------------------------*/


/*------------------------Export Marco Definition---------------------------*/
/*------------------------Export Marco Definition---------------------------*/


/*--------------------------Exported Function prototype---------------------*/
/*
 * BB and RF register read/write
 *   */
u32	PHY_QueryBBReg8188E(PADAPTER	Adapter,
			    u32		RegAddr,
			    u32		BitMask);
void	PHY_SetBBReg8188E(PADAPTER	Adapter,
			  u32		RegAddr,
			  u32		BitMask,
			  u32		Data);
u32	PHY_QueryRFReg8188E(PADAPTER	Adapter,
			    u8				eRFPath,
			    u32				RegAddr,
			    u32				BitMask);
void	PHY_SetRFReg8188E(PADAPTER		Adapter,
			  u8				eRFPath,
			  u32				RegAddr,
			  u32				BitMask,
			  u32				Data);

/*
 * Initialization related function
 */
/* MAC/BB/RF HAL config */
int	PHY_MACConfig8188E(PADAPTER	Adapter);
int	PHY_BBConfig8188E(PADAPTER	Adapter);
int	PHY_RFConfig8188E(PADAPTER	Adapter);

/* RF config */
int	rtl8188e_PHY_ConfigRFWithParaFile(PADAPTER Adapter, u8 *pFileName, u8 eRFPath);

/*
 * RF Power setting
 */
/* extern	bool	PHY_SetRFPowerState(PADAPTER			Adapter,
 *									RT_RF_POWER_STATE	eRFPowerState); */

/*
 * BB TX Power R/W
 *   */
void	PHY_GetTxPowerLevel8188E(PADAPTER		Adapter,
				 s32		*powerlevel);
void	PHY_SetTxPowerLevel8188E(PADAPTER		Adapter,
				 u8			channel);
bool	PHY_UpdateTxPowerDbm8188E(PADAPTER	Adapter,
				  int		powerInDbm);

void
PHY_SetTxPowerIndex_8188E(
	PADAPTER			Adapter,
	u32					PowerIndex,
	u8					RFPath,
	u8					Rate
);

u8
PHY_GetTxPowerIndex_8188E(
	PADAPTER		pAdapter,
	u8				RFPath,
	u8				Rate,
	u8				BandWidth,
	u8				Channel,
	struct txpwr_idx_comp *tic
);

/*
 * Switch bandwidth for 8192S
 */
/* extern	void	PHY_SetBWModeCallback8192C(	PRT_TIMER		pTimer	); */
void	PHY_SetBWMode8188E(PADAPTER			pAdapter,
			   CHANNEL_WIDTH	ChnlWidth,
			   unsigned char	Offset);

/*
 * Set FW CMD IO for 8192S.
 */
/* extern	bool HalSetIO8192C(PADAPTER			Adapter,
 *									IO_TYPE				IOType); */

/*
 * Set A2 entry to fw for 8192S
 *   */
extern	void FillA2Entry8192C(PADAPTER			Adapter,
			      u8				index,
			      u8				*val);


/*
 * channel switch related funciton
 */
/* extern	void	PHY_SwChnlCallback8192C(	PRT_TIMER		pTimer	); */
void	PHY_SwChnl8188E(PADAPTER		pAdapter,
			u8			channel);

void
PHY_SetSwChnlBWMode8188E(
	PADAPTER			Adapter,
	u8					channel,
	CHANNEL_WIDTH	Bandwidth,
	u8					Offset40,
	u8					Offset80
);

void
PHY_SetRFEReg_8188E(
	PADAPTER		Adapter
);
/*
 * BB/MAC/RF other monitor API
 *   */
void phy_set_rf_path_switch_8188e(PADAPTER	pAdapter, bool		bMain);

extern	void
PHY_SwitchEphyParameter(
	PADAPTER			Adapter
);

extern	void
PHY_EnableHostClkReq(
	PADAPTER			Adapter
);

bool
SetAntennaConfig92C(
	PADAPTER	Adapter,
	u8		DefaultAnt
);

/*--------------------------Exported Function prototype---------------------*/

/*
 * Initialization related function
 *
 * MAC/BB/RF HAL config */
/* extern s32 PHY_MACConfig8723(PADAPTER padapter);
 * s32 PHY_BBConfig8723(PADAPTER padapter);
 * s32 PHY_RFConfig8723(PADAPTER padapter); */



/* ******************************************************************
 * Note: If SIC_ENABLE under PCIE, because of the slow operation
 *	you should
 * 	2) "#define RTL8723_FPGA_VERIFICATION	1"				in Precomp.h.WlanE.Windows
 * 	3) "#define RTL8190_Download_Firmware_From_Header	0"	in Precomp.h.WlanE.Windows if needed.
 *   */
#if (RTL8188E_FPGAtrue_PHY_VERIFICATION == 1)
	#define	SIC_ENABLE				1
	#define	SIC_HW_SUPPORT		1
#else
	#define	SIC_ENABLE				0
	#define	SIC_HW_SUPPORT		0
#endif
/* ****************************************************************** */


#define	SIC_MAX_POLL_CNT		5

#if (SIC_HW_SUPPORT == 1)
	#define	SIC_CMD_READY			0
	#define	SIC_CMD_PREWRITE		0x1
		#define	SIC_CMD_WRITE			0x40
		#define	SIC_CMD_PREREAD		0x2
		#define	SIC_CMD_READ			0x80
		#define	SIC_CMD_INIT			0xf0
		#define	SIC_INIT_VAL			0xff

		#define	SIC_INIT_REG			0x1b7
		#define	SIC_CMD_REG			0x1EB		/* 1byte */
		#define	SIC_ADDR_REG			0x1E8		/* 1b4~1b5, 2 bytes */
		#define	SIC_DATA_REG			0x1EC		/* 1b0~1b3 */
#else
	#define	SIC_CMD_READY			0
	#define	SIC_CMD_WRITE			1
	#define	SIC_CMD_READ			2

		#define	SIC_CMD_REG			0x1EB		/* 1byte */
		#define	SIC_ADDR_REG			0x1E8		/* 1b9~1ba, 2 bytes */
		#define	SIC_DATA_REG			0x1EC		/* 1bc~1bf */
#endif

#if (SIC_ENABLE == 1)
	void SIC_Init(PADAPTER Adapter);
#endif


#endif /* __INC_HAL8192CPHYCFG_H */
