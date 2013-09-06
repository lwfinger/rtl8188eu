/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 *published by the Free Software Foundation.
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
#define _RTW_MP_C_

#include <drv_types.h>

#include "odm_precomp.h"
#include "rtl8188e_hal.h"

u32 read_macreg(struct adapter *padapter, u32 addr, u32 sz)
{
	u32 val = 0;

	switch (sz) {
	case 1:
		val = rtw_read8(padapter, addr);
		break;
	case 2:
		val = rtw_read16(padapter, addr);
		break;
	case 4:
		val = rtw_read32(padapter, addr);
		break;
	default:
		val = 0xffffffff;
		break;
	}

	return val;
}

void write_macreg(struct adapter *padapter, u32 addr, u32 val, u32 sz)
{
	switch (sz) {
	case 1:
		rtw_write8(padapter, addr, (u8)val);
		break;
	case 2:
		rtw_write16(padapter, addr, (u16)val);
		break;
	case 4:
		rtw_write32(padapter, addr, val);
		break;
	default:
		break;
	}
}

u32 read_bbreg(struct adapter *padapter, u32 addr, u32 bitmask)
{
	return rtw_hal_read_bbreg(padapter, addr, bitmask);
}

void write_bbreg(struct adapter *padapter, u32 addr, u32 bitmask, u32 val)
{
	rtw_hal_write_bbreg(padapter, addr, bitmask, val);
}

u32 _read_rfreg(struct adapter *padapter, u8 rfpath, u32 addr, u32 bitmask)
{
	return rtw_hal_read_rfreg(padapter, (enum rf_radio_path)rfpath, addr, bitmask);
}

void _write_rfreg(struct adapter *padapter, u8 rfpath, u32 addr, u32 bitmask, u32 val)
{
	rtw_hal_write_rfreg(padapter, (enum rf_radio_path)rfpath, addr, bitmask, val);
}

u32 read_rfreg(struct adapter *padapter, u8 rfpath, u32 addr)
{
	return _read_rfreg(padapter, (enum rf_radio_path)rfpath, addr, bRFRegOffsetMask);
}

void write_rfreg(struct adapter *padapter, u8 rfpath, u32 addr, u32 val)
{
	_write_rfreg(padapter, (enum rf_radio_path)rfpath, addr, bRFRegOffsetMask, val);
}

static void _init_mp_priv_(struct mp_priv *pmp_priv)
{
	struct wlan_bssid_ex *pnetwork;

	_rtw_memset(pmp_priv, 0, sizeof(struct mp_priv));

	pmp_priv->mode = MP_OFF;

	pmp_priv->channel = 1;
	pmp_priv->bandwidth = HT_CHANNEL_WIDTH_20;
	pmp_priv->prime_channel_offset = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
	pmp_priv->rateidx = MPT_RATE_1M;
	pmp_priv->txpoweridx = 0x2A;

	pmp_priv->antenna_tx = ANTENNA_A;
	pmp_priv->antenna_rx = ANTENNA_AB;

	pmp_priv->check_mp_pkt = 0;

	pmp_priv->tx_pktcount = 0;

	pmp_priv->rx_pktcount = 0;
	pmp_priv->rx_crcerrpktcount = 0;

	pmp_priv->network_macaddr[0] = 0x00;
	pmp_priv->network_macaddr[1] = 0xE0;
	pmp_priv->network_macaddr[2] = 0x4C;
	pmp_priv->network_macaddr[3] = 0x87;
	pmp_priv->network_macaddr[4] = 0x66;
	pmp_priv->network_macaddr[5] = 0x55;

	pnetwork = &pmp_priv->mp_network.network;
	_rtw_memcpy(pnetwork->MacAddress, pmp_priv->network_macaddr, ETH_ALEN);

	pnetwork->Ssid.SsidLength = 8;
	_rtw_memcpy(pnetwork->Ssid.Ssid, "mp_871x", pnetwork->Ssid.SsidLength);
}

static int init_mp_priv_by_os(struct mp_priv *pmp_priv)
{
	int i, res;
	struct mp_xmit_frame *pmp_xmitframe;

	if (pmp_priv == NULL)
		return _FAIL;

	_rtw_init_queue(&pmp_priv->free_mp_xmitqueue);

	pmp_priv->pallocated_mp_xmitframe_buf = NULL;
	pmp_priv->pallocated_mp_xmitframe_buf = rtw_zmalloc(NR_MP_XMITFRAME * sizeof(struct mp_xmit_frame) + 4);
	if (pmp_priv->pallocated_mp_xmitframe_buf == NULL) {
		res = _FAIL;
		goto _exit_init_mp_priv;
	}

	pmp_priv->pmp_xmtframe_buf = pmp_priv->pallocated_mp_xmitframe_buf + 4 - ((size_t)(pmp_priv->pallocated_mp_xmitframe_buf) & 3);

	pmp_xmitframe = (struct mp_xmit_frame *)pmp_priv->pmp_xmtframe_buf;

	for (i = 0; i < NR_MP_XMITFRAME; i++) {
		_rtw_init_listhead(&pmp_xmitframe->list);
		rtw_list_insert_tail(&pmp_xmitframe->list, &pmp_priv->free_mp_xmitqueue.queue);

		pmp_xmitframe->pkt = NULL;
		pmp_xmitframe->frame_tag = MP_FRAMETAG;
		pmp_xmitframe->padapter = pmp_priv->papdater;

		pmp_xmitframe++;
	}

	pmp_priv->free_mp_xmitframe_cnt = NR_MP_XMITFRAME;

	res = _SUCCESS;

_exit_init_mp_priv:

	return res;
}

static void mp_init_xmit_attrib(struct mp_tx *pmptx, struct adapter *padapter)
{
	struct pkt_attrib *pattrib;
	struct tx_desc *desc;

	/*  init xmitframe attribute */
	pattrib = &pmptx->attrib;
	_rtw_memset(pattrib, 0, sizeof(struct pkt_attrib));
	desc = &pmptx->desc;
	_rtw_memset(desc, 0, TXDESC_SIZE);

	pattrib->ether_type = 0x8712;
	_rtw_memset(pattrib->dst, 0xFF, ETH_ALEN);
	pattrib->ack_policy = 0;
	pattrib->hdrlen = WLAN_HDR_A3_LEN;
	pattrib->subtype = WIFI_DATA;
	pattrib->priority = 0;
	pattrib->qsel = pattrib->priority;
	pattrib->nr_frags = 1;
	pattrib->encrypt = 0;
	pattrib->bswenc = false;
	pattrib->qos_en = false;
}

s32 init_mp_priv(struct adapter *padapter)
{
	struct mp_priv *pmppriv = &padapter->mppriv;

	_init_mp_priv_(pmppriv);
	pmppriv->papdater = padapter;

	pmppriv->tx.stop = 1;
	mp_init_xmit_attrib(&pmppriv->tx, padapter);

	switch (padapter->registrypriv.rf_config) {
	case RF_1T1R:
		pmppriv->antenna_tx = ANTENNA_A;
		pmppriv->antenna_rx = ANTENNA_A;
		break;
	case RF_1T2R:
	default:
		pmppriv->antenna_tx = ANTENNA_A;
		pmppriv->antenna_rx = ANTENNA_AB;
		break;
	case RF_2T2R:
	case RF_2T2R_GREEN:
		pmppriv->antenna_tx = ANTENNA_AB;
		pmppriv->antenna_rx = ANTENNA_AB;
		break;
	case RF_2T4R:
		pmppriv->antenna_tx = ANTENNA_AB;
		pmppriv->antenna_rx = ANTENNA_ABCD;
		break;
	}

	return _SUCCESS;
}

void free_mp_priv(struct mp_priv *pmp_priv)
{
	if (pmp_priv->pallocated_mp_xmitframe_buf) {
		rtw_mfree(pmp_priv->pallocated_mp_xmitframe_buf, 0);
		pmp_priv->pallocated_mp_xmitframe_buf = NULL;
	}
	pmp_priv->pmp_xmtframe_buf = NULL;
}

#define PHY_IQCalibrate(a, b)	PHY_IQCalibrate_8188E(a, b)
#define PHY_LCCalibrate(a)	PHY_LCCalibrate_8188E(a)
#define PHY_SetRFPathSwitch(a, b) PHY_SetRFPathSwitch_8188E(a, b)

s32 MPT_InitializeAdapter(struct adapter *pAdapter, u8 Channel)
{
	struct hal_data_8188e	*pHalData = GET_HAL_DATA(pAdapter);
	s32		rtStatus = _SUCCESS;
	struct mpt_context *pMptCtx = &pAdapter->mppriv.MptCtx;
	struct mlme_priv *pmlmepriv = &pAdapter->mlmepriv;

	/*  HW Initialization for 8190 MPT. */
	/*  SW Initialization for 8190 MP. */
	pMptCtx->bMptDrvUnload = false;
	pMptCtx->bMassProdTest = false;
	pMptCtx->bMptIndexEven = true;	/* default gain index is -6.0db */
	pMptCtx->h2cReqNum = 0x0;
	/* Init mpt event. */
	/* init for BT MP */

	pMptCtx->bMptWorkItemInProgress = false;
	pMptCtx->CurrMptAct = NULL;
	/*  */

	/*  Don't accept any packets */
	rtw_write32(pAdapter, REG_RCR, 0);

	PHY_IQCalibrate(pAdapter, false);
	dm_CheckTXPowerTracking(&pHalData->odmpriv);	/* trigger thermal meter */
	PHY_LCCalibrate(pAdapter);

	pMptCtx->backup0xc50 = (u8)PHY_QueryBBReg(pAdapter, rOFDM0_XAAGCCore1, bMaskByte0);
	pMptCtx->backup0xc58 = (u8)PHY_QueryBBReg(pAdapter, rOFDM0_XBAGCCore1, bMaskByte0);
	pMptCtx->backup0xc30 = (u8)PHY_QueryBBReg(pAdapter, rOFDM0_RxDetector1, bMaskByte0);
	pMptCtx->backup0x52_RF_A = (u8)PHY_QueryRFReg(pAdapter, RF_PATH_A, RF_0x52, 0x000F0);
	pMptCtx->backup0x52_RF_B = (u8)PHY_QueryRFReg(pAdapter, RF_PATH_A, RF_0x52, 0x000F0);

	/* set ant to wifi side in mp mode */
	rtw_write16(pAdapter, 0x870, 0x300);
	rtw_write16(pAdapter, 0x860, 0x110);

	if (pAdapter->registrypriv.mp_mode == 1)
		pmlmepriv->fw_state = WIFI_MP_STATE;

	return	rtStatus;
}

/*-----------------------------------------------------------------------------
 * Function:	MPT_DeInitAdapter()
 *
 * Overview:	Extra DeInitialization for Mass Production Test.
 *
 * Input:		struct adapter *	pAdapter
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	05/08/2007	MHC		Create Version 0.
 *	05/18/2007	MHC		Add normal driver MPHalt code.
 *
 *---------------------------------------------------------------------------*/
void MPT_DeInitAdapter(struct adapter *pAdapter)
{
	struct mpt_context *pMptCtx = &pAdapter->mppriv.MptCtx;

	pMptCtx->bMptDrvUnload = true;
}

static u8 mpt_ProStartTest(struct adapter *padapter)
{
	struct mpt_context *pMptCtx = &padapter->mppriv.MptCtx;

	pMptCtx->bMassProdTest = true;
	pMptCtx->bStartContTx = false;
	pMptCtx->bCckContTx = false;
	pMptCtx->bOfdmContTx = false;
	pMptCtx->bSingleCarrier = false;
	pMptCtx->bCarrierSuppression = false;
	pMptCtx->bSingleTone = false;

	return _SUCCESS;
}

/*
 * General use
 */
s32 SetPowerTracking(struct adapter *padapter, u8 enable)
{
	Hal_SetPowerTracking(padapter, enable);
	return 0;
}

void GetPowerTracking(struct adapter *padapter, u8 *enable)
{
	Hal_GetPowerTracking(padapter, enable);
}

static void disable_dm(struct adapter *padapter)
{
	u8 v8;


	/* 3 1. disable firmware dynamic mechanism */
	/*  disable Power Training, Rate Adaptive */
	v8 = rtw_read8(padapter, REG_BCN_CTRL);
	v8 &= ~EN_BCN_FUNCTION;
	rtw_write8(padapter, REG_BCN_CTRL, v8);

	/* 3 2. disable driver dynamic mechanism */
	/*  disable Dynamic Initial Gain */
	/*  disable High Power */
	/*  disable Power Tracking */
	Switch_DM_Func(padapter, DYNAMIC_FUNC_DISABLE, false);

	/*  enable APK, LCK and IQK but disable power tracking */
	Switch_DM_Func(padapter, DYNAMIC_RF_CALIBRATION, true);
}

/* This function initializes the DUT to the MP test mode */
s32 mp_start_test(struct adapter *padapter)
{
	struct wlan_bssid_ex bssid;
	struct sta_info *psta;
	u32 length;
	u8 val8;

	unsigned long irqL;
	s32 res = _SUCCESS;

	struct mp_priv *pmppriv = &padapter->mppriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct wlan_network *tgt_network = &pmlmepriv->cur_network;

	padapter->registrypriv.mp_mode = 1;
	pmppriv->bSetTxPower = 0;		/* for  manually set tx power */

	/* 3 disable dynamic mechanism */
	disable_dm(padapter);

	/* 3 0. update mp_priv */

	if (padapter->registrypriv.rf_config == RF_819X_MAX_TYPE) {
		switch (GET_RF_TYPE(padapter)) {
		case RF_1T1R:
			pmppriv->antenna_tx = ANTENNA_A;
			pmppriv->antenna_rx = ANTENNA_A;
			break;
		case RF_1T2R:
		default:
			pmppriv->antenna_tx = ANTENNA_A;
			pmppriv->antenna_rx = ANTENNA_AB;
			break;
		case RF_2T2R:
		case RF_2T2R_GREEN:
			pmppriv->antenna_tx = ANTENNA_AB;
			pmppriv->antenna_rx = ANTENNA_AB;
			break;
		case RF_2T4R:
			pmppriv->antenna_tx = ANTENNA_AB;
			pmppriv->antenna_rx = ANTENNA_ABCD;
			break;
		}
	}

	mpt_ProStartTest(padapter);

	/* 3 1. initialize a new struct wlan_bssid_ex */
/*	_rtw_memset(&bssid, 0, sizeof(struct wlan_bssid_ex)); */
	_rtw_memcpy(bssid.MacAddress, pmppriv->network_macaddr, ETH_ALEN);
	bssid.Ssid.SsidLength = strlen("mp_pseudo_adhoc");
	_rtw_memcpy(bssid.Ssid.Ssid, (u8 *)"mp_pseudo_adhoc", bssid.Ssid.SsidLength);
	bssid.InfrastructureMode = Ndis802_11IBSS;
	bssid.NetworkTypeInUse = Ndis802_11DS;
	bssid.IELength = 0;

	length = get_wlan_bssid_ex_sz(&bssid);
	if (length % 4)
		bssid.Length = ((length >> 2) + 1) << 2; /* round up to multiple of 4 bytes. */
	else
		bssid.Length = length;

	_enter_critical_bh(&pmlmepriv->lock, &irqL);

	if (check_fwstate(pmlmepriv, WIFI_MP_STATE) == true)
		goto end_of_mp_start_test;

	/* init mp_start_test status */
	if (check_fwstate(pmlmepriv, _FW_LINKED) == true) {
		rtw_disassoc_cmd(padapter, 500, true);
		rtw_indicate_disconnect(padapter);
		rtw_free_assoc_resources(padapter, 1);
	}
	pmppriv->prev_fw_state = get_fwstate(pmlmepriv);
	if (padapter->registrypriv.mp_mode == 1)
		pmlmepriv->fw_state = WIFI_MP_STATE;
	set_fwstate(pmlmepriv, _FW_UNDER_LINKING);

	/* 3 2. create a new psta for mp driver */
	/* clear psta in the cur_network, if any */
	psta = rtw_get_stainfo(&padapter->stapriv, tgt_network->network.MacAddress);
	if (psta)
		rtw_free_stainfo(padapter, psta);

	psta = rtw_alloc_stainfo(&padapter->stapriv, bssid.MacAddress);
	if (psta == NULL) {
		RT_TRACE(_module_mp_, _drv_err_, ("mp_start_test: Can't alloc sta_info!\n"));
		pmlmepriv->fw_state = pmppriv->prev_fw_state;
		res = _FAIL;
		goto end_of_mp_start_test;
	}

	/* 3 3. join psudo AdHoc */
	tgt_network->join_res = 1;
	tgt_network->aid = 1;
	psta->aid = 1;
	_rtw_memcpy(&tgt_network->network, &bssid, length);

	rtw_indicate_connect(padapter);
	_clr_fwstate_(pmlmepriv, _FW_UNDER_LINKING);

end_of_mp_start_test:

	_exit_critical_bh(&pmlmepriv->lock, &irqL);

	if (res == _SUCCESS) {
		/*  set MSR to WIFI_FW_ADHOC_STATE */
		val8 = rtw_read8(padapter, MSR) & 0xFC; /*  0x0102 */
		val8 |= WIFI_FW_ADHOC_STATE;
		rtw_write8(padapter, MSR, val8); /*  Link in ad hoc network */
	}
	return res;
}
/*  */
/* This function change the DUT from the MP test mode into normal mode */
void mp_stop_test(struct adapter *padapter)
{
	struct mp_priv *pmppriv = &padapter->mppriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct wlan_network *tgt_network = &pmlmepriv->cur_network;
	struct sta_info *psta;

	unsigned long irqL;

	if (pmppriv->mode == MP_ON) {
		pmppriv->bSetTxPower = 0;
		_enter_critical_bh(&pmlmepriv->lock, &irqL);
		if (check_fwstate(pmlmepriv, WIFI_MP_STATE) == false)
			goto end_of_mp_stop_test;

		/* 3 1. disconnect psudo AdHoc */
		rtw_indicate_disconnect(padapter);

		/* 3 2. clear psta used in mp test mode. */
		psta = rtw_get_stainfo(&padapter->stapriv, tgt_network->network.MacAddress);
		if (psta)
			rtw_free_stainfo(padapter, psta);

		/* 3 3. return to normal state (default:station mode) */
		pmlmepriv->fw_state = pmppriv->prev_fw_state; /*  WIFI_STATION_STATE; */

		/* flush the cur_network */
		_rtw_memset(tgt_network, 0, sizeof(struct wlan_network));

		_clr_fwstate_(pmlmepriv, WIFI_MP_STATE);

end_of_mp_stop_test:

		_exit_critical_bh(&pmlmepriv->lock, &irqL);
	}
}
/*---------------------------hal\rtl8192c\MPT_Phy.c---------------------------*/
/*-----------------------------------------------------------------------------
 * Function:	mpt_SwitchRfSetting
 *
 * Overview:	Change RF Setting when we siwthc channel/rate/BW for MP.
 *
 * Input:	struct adapter *				pAdapter
 *
 * Output:      NONE
 *
 * Return:      NONE
 *
 * Revised History:
 * When			Who		Remark
 * 01/08/2009	MHC		Suggestion from SD3 Willis for 92S series.
 * 01/09/2009	MHC		Add CCK modification for 40MHZ. Suggestion from SD3.
 *
 *---------------------------------------------------------------------------*/
static void mpt_SwitchRfSetting(struct adapter *pAdapter)
{
	Hal_mpt_SwitchRfSetting(pAdapter);
    }

/*---------------------------hal\rtl8192c\MPT_Phy.c---------------------------*/
/*---------------------------hal\rtl8192c\MPT_HelperFunc.c---------------------------*/
static void MPT_CCKTxPowerAdjust(struct adapter *Adapter, bool bInCH14)
{
	Hal_MPT_CCKTxPowerAdjust(Adapter, bInCH14);
}

static void MPT_CCKTxPowerAdjustbyIndex(struct adapter *pAdapter, bool beven)
{
	Hal_MPT_CCKTxPowerAdjustbyIndex(pAdapter, beven);
	}

/*---------------------------hal\rtl8192c\MPT_HelperFunc.c---------------------------*/

/*
 * SetChannel
 * Description
 *	Use H2C command to change channel,
 *	not only modify rf register, but also other setting need to be done.
 */
void SetChannel(struct adapter *pAdapter)
{
	Hal_SetChannel(pAdapter);
}

/*
 * Notice
 *	Switch bandwitdth may change center frequency(channel)
 */
void SetBandwidth(struct adapter *pAdapter)
{
	Hal_SetBandwidth(pAdapter);
}

static void SetCCKTxPower(struct adapter *pAdapter, u8 *TxPower)
{
	Hal_SetCCKTxPower(pAdapter, TxPower);
}

static void SetOFDMTxPower(struct adapter *pAdapter, u8 *TxPower)
{
	Hal_SetOFDMTxPower(pAdapter, TxPower);
	}


void SetAntenna(struct adapter *pAdapter)
{
	Hal_SetAntenna(pAdapter);
}

void	SetAntennaPathPower(struct adapter *pAdapter)
{
	Hal_SetAntennaPathPower(pAdapter);
}

void SetTxPower(struct adapter *pAdapter)
{
	Hal_SetTxPower(pAdapter);
	}

static void SetTxAGCOffset(struct adapter *pAdapter, u32 ulTxAGCOffset)
{
	u32 TxAGCOffset_B, TxAGCOffset_C, TxAGCOffset_D, tmpAGC;

	TxAGCOffset_B = (ulTxAGCOffset&0x000000ff);
	TxAGCOffset_C = ((ulTxAGCOffset&0x0000ff00)>>8);
	TxAGCOffset_D = ((ulTxAGCOffset&0x00ff0000)>>16);

	tmpAGC = (TxAGCOffset_D<<8 | TxAGCOffset_C<<4 | TxAGCOffset_B);
	write_bbreg(pAdapter, rFPGA0_TxGainStage,
		    (bXBTxAGC|bXCTxAGC|bXDTxAGC), tmpAGC);
}

void SetDataRate(struct adapter *pAdapter)
{
	Hal_SetDataRate(pAdapter);
}

void MP_PHY_SetRFPathSwitch(struct adapter *pAdapter , bool bMain)
{
	PHY_SetRFPathSwitch(pAdapter, bMain);
}

s32 SetThermalMeter(struct adapter *pAdapter, u8 target_ther)
{
	return Hal_SetThermalMeter(pAdapter, target_ther);
}

static void TriggerRFThermalMeter(struct adapter *pAdapter)
{
	Hal_TriggerRFThermalMeter(pAdapter);
}

static u8 ReadRFThermalMeter(struct adapter *pAdapter)
{
	return Hal_ReadRFThermalMeter(pAdapter);
}

void GetThermalMeter(struct adapter *pAdapter, u8 *value)
{
	Hal_GetThermalMeter(pAdapter, value);
}

void SetSingleCarrierTx(struct adapter *pAdapter, u8 bStart)
{
	PhySetTxPowerLevel(pAdapter);
	Hal_SetSingleCarrierTx(pAdapter, bStart);
}

void SetSingleToneTx(struct adapter *pAdapter, u8 bStart)
{
	PhySetTxPowerLevel(pAdapter);
	Hal_SetSingleToneTx(pAdapter, bStart);
}

void SetCarrierSuppressionTx(struct adapter *pAdapter, u8 bStart)
{
	PhySetTxPowerLevel(pAdapter);
	Hal_SetCarrierSuppressionTx(pAdapter, bStart);
}

static void SetCCKContinuousTx(struct adapter *pAdapter, u8 bStart)
{
	PhySetTxPowerLevel(pAdapter);
	Hal_SetCCKContinuousTx(pAdapter, bStart);
}

static void SetOFDMContinuousTx(struct adapter *pAdapter, u8 bStart)
{
	PhySetTxPowerLevel(pAdapter);
	Hal_SetOFDMContinuousTx(pAdapter, bStart);
} /* mpt_StartOfdmContTx */

void SetContinuousTx(struct adapter *pAdapter, u8 bStart)
{
	PhySetTxPowerLevel(pAdapter);
	Hal_SetContinuousTx(pAdapter, bStart);
}


void PhySetTxPowerLevel(struct adapter *pAdapter)
{
	struct mp_priv *pmp_priv = &pAdapter->mppriv;

	if (pmp_priv->bSetTxPower == 0) /*  for NO manually set power index */
		PHY_SetTxPowerLevel8188E(pAdapter, pmp_priv->channel);
}

/*  */
static void dump_mpframe(struct adapter *padapter, struct xmit_frame *pmpframe)
{
	rtw_hal_mgnt_xmit(padapter, pmpframe);
}

static struct xmit_frame *alloc_mp_xmitframe(struct xmit_priv *pxmitpriv)
{
	struct xmit_frame	*pmpframe;
	struct xmit_buf	*pxmitbuf;

	pmpframe = rtw_alloc_xmitframe(pxmitpriv);
	if (pmpframe == NULL)
		return NULL;

	pxmitbuf = rtw_alloc_xmitbuf(pxmitpriv);
	if (pxmitbuf == NULL) {
		rtw_free_xmitframe(pxmitpriv, pmpframe);
		return NULL;
	}

	pmpframe->frame_tag = MP_FRAMETAG;

	pmpframe->pxmitbuf = pxmitbuf;

	pmpframe->buf_addr = pxmitbuf->pbuf;

	pxmitbuf->priv_data = pmpframe;

	return pmpframe;
}

static int mp_xmit_packet_thread(void *context)
{
	struct xmit_frame	*pxmitframe;
	struct mp_tx		*pmptx;
	struct mp_priv	*pmp_priv;
	struct xmit_priv	*pxmitpriv;
	struct adapter *padapter;

	pmp_priv = (struct mp_priv *)context;
	pmptx = &pmp_priv->tx;
	padapter = pmp_priv->papdater;
	pxmitpriv = &(padapter->xmitpriv);

	thread_enter("RTW_MP_THREAD");

	/* DBG_88E("%s:pkTx Start\n", __func__); */
	while (1) {
		pxmitframe = alloc_mp_xmitframe(pxmitpriv);
		if (pxmitframe == NULL) {
			if (pmptx->stop ||
			    padapter->bSurpriseRemoved ||
			    padapter->bDriverStopped) {
				goto exit;
			} else {
				rtw_msleep_os(1);
				continue;
			}
		}

		_rtw_memcpy((u8 *)(pxmitframe->buf_addr+TXDESC_OFFSET), pmptx->buf, pmptx->write_size);
		_rtw_memcpy(&(pxmitframe->attrib), &(pmptx->attrib), sizeof(struct pkt_attrib));

		dump_mpframe(padapter, pxmitframe);

		pmptx->sended++;
		pmp_priv->tx_pktcount++;

		if (pmptx->stop ||
		    padapter->bSurpriseRemoved ||
		    padapter->bDriverStopped)
			goto exit;
		if ((pmptx->count != 0) &&
		    (pmptx->count == pmptx->sended))
			goto exit;

		flush_signals_thread();
	}

exit:
	/* DBG_88E("%s:pkTx Exit\n", __func__); */
	rtw_mfree(pmptx->pallocated_buf, pmptx->buf_size);
	pmptx->pallocated_buf = NULL;
	pmptx->stop = 1;

	thread_exit();
}

void fill_txdesc_for_mp(struct adapter *padapter, struct tx_desc *ptxdesc)
{
	struct mp_priv *pmp_priv = &padapter->mppriv;
	_rtw_memcpy(ptxdesc, &(pmp_priv->tx.desc), TXDESC_SIZE);
}

void SetPacketTx(struct adapter *padapter)
{
	u8 *ptr, *pkt_start, *pkt_end;
	u32 pkt_size;
	struct tx_desc *desc;
	struct rtw_ieee80211_hdr *hdr;
	u8 payload;
	s32 bmcast;
	struct pkt_attrib *pattrib;
	struct mp_priv *pmp_priv;


	pmp_priv = &padapter->mppriv;
	if (pmp_priv->tx.stop)
		return;
	pmp_priv->tx.sended = 0;
	pmp_priv->tx.stop = 0;
	pmp_priv->tx_pktcount = 0;

	/* 3 1. update_attrib() */
	pattrib = &pmp_priv->tx.attrib;
	_rtw_memcpy(pattrib->src, padapter->eeprompriv.mac_addr, ETH_ALEN);
	_rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);
	_rtw_memcpy(pattrib->ra, pattrib->dst, ETH_ALEN);
	bmcast = IS_MCAST(pattrib->ra);
	if (bmcast) {
		pattrib->mac_id = 1;
		pattrib->psta = rtw_get_bcmc_stainfo(padapter);
	} else {
		pattrib->mac_id = 0;
		pattrib->psta = rtw_get_stainfo(&padapter->stapriv, get_bssid(&padapter->mlmepriv));
	}

	pattrib->last_txcmdsz = pattrib->hdrlen + pattrib->pktlen;

	/* 3 2. allocate xmit buffer */
	pkt_size = pattrib->last_txcmdsz;

	if (pmp_priv->tx.pallocated_buf)
		rtw_mfree(pmp_priv->tx.pallocated_buf, pmp_priv->tx.buf_size);
	pmp_priv->tx.write_size = pkt_size;
	pmp_priv->tx.buf_size = pkt_size + XMITBUF_ALIGN_SZ;
	pmp_priv->tx.pallocated_buf = rtw_zmalloc(pmp_priv->tx.buf_size);
	if (pmp_priv->tx.pallocated_buf == NULL) {
		DBG_88E("%s: malloc(%d) fail!!\n", __func__, pmp_priv->tx.buf_size);
		return;
	}
	pmp_priv->tx.buf = (u8 *)N_BYTE_ALIGMENT((size_t)(pmp_priv->tx.pallocated_buf), XMITBUF_ALIGN_SZ);
	ptr = pmp_priv->tx.buf;

	desc = &(pmp_priv->tx.desc);
	_rtw_memset(desc, 0, TXDESC_SIZE);
	pkt_start = ptr;
	pkt_end = pkt_start + pkt_size;

	/* 3 3. init TX descriptor */
	/*  offset 0 */
	desc->txdw0 |= cpu_to_le32(OWN | FSG | LSG);
	desc->txdw0 |= cpu_to_le32(pkt_size & 0x0000FFFF); /*  packet size */
	desc->txdw0 |= cpu_to_le32(((TXDESC_SIZE + OFFSET_SZ) << OFFSET_SHT) & 0x00FF0000); /* 32 bytes for TX Desc */
	if (bmcast)
		desc->txdw0 |= cpu_to_le32(BMC); /*  broadcast packet */

	desc->txdw1 |= cpu_to_le32((0x01 << 26) & 0xff000000);
	/*  offset 4 */
		desc->txdw1 |= cpu_to_le32((pattrib->mac_id) & 0x3F); /* CAM_ID(MAC_ID) */
	desc->txdw1 |= cpu_to_le32((pattrib->qsel << QSEL_SHT) & 0x00001F00); /*  Queue Select, TID */

	desc->txdw1 |= cpu_to_le32((pattrib->raid << RATE_ID_SHT) & 0x000F0000); /*  Rate Adaptive ID */
	/*  offset 8 */
	/*  offset 12 */

	desc->txdw3 |= cpu_to_le32((pattrib->seqnum<<16)&0x0fff0000);

	/*  offset 16 */
	desc->txdw4 |= cpu_to_le32(HW_SSN);
	desc->txdw4 |= cpu_to_le32(USERATE);
	desc->txdw4 |= cpu_to_le32(DISDATAFB);

	if (pmp_priv->preamble) {
		if (pmp_priv->rateidx <=  MPT_RATE_54M)
			desc->txdw4 |= cpu_to_le32(DATA_SHORT); /*  CCK Short Preamble */
	}
	if (pmp_priv->bandwidth == HT_CHANNEL_WIDTH_40)
		desc->txdw4 |= cpu_to_le32(DATA_BW);

	/*  offset 20 */
	desc->txdw5 |= cpu_to_le32(pmp_priv->rateidx & 0x0000001F);

	if (pmp_priv->preamble) {
		if (pmp_priv->rateidx > MPT_RATE_54M)
			desc->txdw5 |= cpu_to_le32(SGI); /*  MCS Short Guard Interval */
	}
	desc->txdw5 |= cpu_to_le32(RTY_LMT_EN); /*  retry limit enable */
	desc->txdw5 |= cpu_to_le32(0x00180000); /*  DATA/RTS Rate Fallback Limit */

	/* 3 4. make wlan header, make_wlanhdr() */
	hdr = (struct rtw_ieee80211_hdr *)pkt_start;
	SetFrameSubType(&hdr->frame_ctl, pattrib->subtype);
	_rtw_memcpy(hdr->addr1, pattrib->dst, ETH_ALEN); /*  DA */
	_rtw_memcpy(hdr->addr2, pattrib->src, ETH_ALEN); /*  SA */
	_rtw_memcpy(hdr->addr3, get_bssid(&padapter->mlmepriv), ETH_ALEN); /*  RA, BSSID */

	/* 3 5. make payload */
	ptr = pkt_start + pattrib->hdrlen;

	switch (pmp_priv->tx.payload) {
	case 0:
		payload = 0x00;
		break;
	case 1:
		payload = 0x5a;
		break;
	case 2:
		payload = 0xa5;
		break;
	case 3:
		payload = 0xff;
		break;
	default:
		payload = 0x00;
		break;
	}

	_rtw_memset(ptr, payload, pkt_end - ptr);

	/* 3 6. start thread */
	pmp_priv->tx.PktTxThread = kthread_run(mp_xmit_packet_thread, pmp_priv, "RTW_MP_THREAD");
	if (IS_ERR(pmp_priv->tx.PktTxThread))
		DBG_88E("Create PktTx Thread Fail !!!!!\n");
}

void SetPacketRx(struct adapter *pAdapter, u8 bStartRx)
{
	struct hal_data_8188e	*pHalData = GET_HAL_DATA(pAdapter);

	if (bStartRx) {
		/*  Accept CRC error and destination address */
		pHalData->ReceiveConfig = AAP | APM | AM | AB | APP_ICV | ADF | AMF | HTC_LOC_CTRL | APP_MIC | APP_PHYSTS;

		pHalData->ReceiveConfig |= ACRC32;

		rtw_write32(pAdapter, REG_RCR, pHalData->ReceiveConfig);

		/*  Accept all data frames */
		rtw_write16(pAdapter, REG_RXFLTMAP2, 0xFFFF);
	} else {
		rtw_write32(pAdapter, REG_RCR, 0);
	}
}

void ResetPhyRxPktCount(struct adapter *pAdapter)
{
	u32 i, phyrx_set = 0;

	for (i = 0; i <= 0xF; i++) {
		phyrx_set = 0;
		phyrx_set |= _RXERR_RPT_SEL(i);	/* select */
		phyrx_set |= RXERR_RPT_RST;	/*  set counter to zero */
		rtw_write32(pAdapter, REG_RXERR_RPT, phyrx_set);
	}
}

static u32 GetPhyRxPktCounts(struct adapter *pAdapter, u32 selbit)
{
	/* selection */
	u32 phyrx_set = 0, count = 0;

	phyrx_set = _RXERR_RPT_SEL(selbit & 0xF);
	rtw_write32(pAdapter, REG_RXERR_RPT, phyrx_set);

	/* Read packet count */
	count = rtw_read32(pAdapter, REG_RXERR_RPT) & RXERR_COUNTER_MASK;

	return count;
}

u32 GetPhyRxPktReceived(struct adapter *pAdapter)
{
	u32 OFDM_cnt = 0, CCK_cnt = 0, HT_cnt = 0;

	OFDM_cnt = GetPhyRxPktCounts(pAdapter, RXERR_TYPE_OFDM_MPDU_OK);
	CCK_cnt = GetPhyRxPktCounts(pAdapter, RXERR_TYPE_CCK_MPDU_OK);
	HT_cnt = GetPhyRxPktCounts(pAdapter, RXERR_TYPE_HT_MPDU_OK);

	return OFDM_cnt + CCK_cnt + HT_cnt;
}

u32 GetPhyRxPktCRC32Error(struct adapter *pAdapter)
{
	u32 OFDM_cnt = 0, CCK_cnt = 0, HT_cnt = 0;

	OFDM_cnt = GetPhyRxPktCounts(pAdapter, RXERR_TYPE_OFDM_MPDU_FAIL);
	CCK_cnt = GetPhyRxPktCounts(pAdapter, RXERR_TYPE_CCK_MPDU_FAIL);
	HT_cnt = GetPhyRxPktCounts(pAdapter, RXERR_TYPE_HT_MPDU_FAIL);

	return OFDM_cnt + CCK_cnt + HT_cnt;
}

/* reg 0x808[9:0]: FFT data x */
/* reg 0x808[22]:  0  -->  1  to get 1 FFT data y */
/* reg 0x8B4[15:0]: FFT data y report */
static u32 rtw_GetPSDData(struct adapter *pAdapter, u32 point)
{
	int psd_val;


	psd_val = rtw_read32(pAdapter, 0x808);
	psd_val &= 0xFFBFFC00;
	psd_val |= point;

	rtw_write32(pAdapter, 0x808, psd_val);
	rtw_mdelay_os(1);
	psd_val |= 0x00400000;

	rtw_write32(pAdapter, 0x808, psd_val);
	rtw_mdelay_os(1);
	psd_val = rtw_read32(pAdapter, 0x8B4);

	psd_val &= 0x0000FFFF;

	return psd_val;
}

/*
 *pts	start_point_min		stop_point_max
 * 128	64			64 + 128 = 192
 * 256	128			128 + 256 = 384
 * 512	256			256 + 512 = 768
 * 1024	512			512 + 1024 = 1536
 *
 */
u32 mp_query_psd(struct adapter *pAdapter, u8 *data)
{
	u32 i, psd_pts = 0, psd_start = 0, psd_stop = 0;
	u32 psd_data = 0;


	if (!netif_running(pAdapter->pnetdev)) {
		RT_TRACE(_module_mp_, _drv_warning_, ("mp_query_psd: Fail! interface not opened!\n"));
		return 0;
	}

	if (check_fwstate(&pAdapter->mlmepriv, WIFI_MP_STATE) == false) {
		RT_TRACE(_module_mp_, _drv_warning_, ("mp_query_psd: Fail! not in MP mode!\n"));
		return 0;
	}

	if (strlen(data) == 0) { /* default value */
		psd_pts = 128;
		psd_start = 64;
		psd_stop = 128;
	} else {
		sscanf(data, "pts =%d, start =%d, stop =%d", &psd_pts, &psd_start, &psd_stop);
	}

	_rtw_memset(data, '\0', sizeof(data));

	i = psd_start;
	while (i < psd_stop) {
		if (i >= psd_pts) {
			psd_data = rtw_GetPSDData(pAdapter, i-psd_pts);
		} else {
			psd_data = rtw_GetPSDData(pAdapter, i);
		}
		sprintf(data, "%s%x ", data, psd_data);
		i++;
	}

	rtw_msleep_os(100);
	return strlen(data)+1;
}

void _rtw_mp_xmit_priv(struct xmit_priv *pxmitpriv)
{
	   int i, res;
	  struct adapter *padapter = pxmitpriv->adapter;
	struct xmit_buf *pxmitbuf = (struct xmit_buf *)pxmitpriv->pxmitbuf;

	u32 max_xmit_extbuf_size = MAX_XMIT_EXTBUF_SZ;
	u32 num_xmit_extbuf = NR_XMIT_EXTBUFF;
	if (padapter->registrypriv.mp_mode == 0) {
		max_xmit_extbuf_size = MAX_XMIT_EXTBUF_SZ;
		num_xmit_extbuf = NR_XMIT_EXTBUFF;
	} else {
		max_xmit_extbuf_size = 20000;
		num_xmit_extbuf = 1;
	}

	pxmitbuf = (struct xmit_buf *)pxmitpriv->pxmit_extbuf;
	for (i = 0; i < num_xmit_extbuf; i++) {
		rtw_os_xmit_resource_free(padapter, pxmitbuf, (max_xmit_extbuf_size + XMITBUF_ALIGN_SZ));

		pxmitbuf++;
	}

	if (pxmitpriv->pallocated_xmit_extbuf)
		rtw_vmfree(pxmitpriv->pallocated_xmit_extbuf, num_xmit_extbuf * sizeof(struct xmit_buf) + 4);

	if (padapter->registrypriv.mp_mode == 0) {
		max_xmit_extbuf_size = 20000;
		num_xmit_extbuf = 1;
	} else {
		max_xmit_extbuf_size = MAX_XMIT_EXTBUF_SZ;
		num_xmit_extbuf = NR_XMIT_EXTBUFF;
	}

	/*  Init xmit extension buff */
	_rtw_init_queue(&pxmitpriv->free_xmit_extbuf_queue);

	pxmitpriv->pallocated_xmit_extbuf = rtw_zvmalloc(num_xmit_extbuf * sizeof(struct xmit_buf) + 4);

	if (pxmitpriv->pallocated_xmit_extbuf  == NULL) {
		RT_TRACE(_module_rtl871x_xmit_c_, _drv_err_, ("alloc xmit_extbuf fail!\n"));
		res = _FAIL;
		goto exit;
	}

	pxmitpriv->pxmit_extbuf = (u8 *)N_BYTE_ALIGMENT((size_t)(pxmitpriv->pallocated_xmit_extbuf), 4);

	pxmitbuf = (struct xmit_buf *)pxmitpriv->pxmit_extbuf;

	for (i = 0; i < num_xmit_extbuf; i++) {
		_rtw_init_listhead(&pxmitbuf->list);

		pxmitbuf->priv_data = NULL;
		pxmitbuf->padapter = padapter;
		pxmitbuf->ext_tag = true;

		res = rtw_os_xmit_resource_alloc(padapter, pxmitbuf, max_xmit_extbuf_size + XMITBUF_ALIGN_SZ);
		if (res == _FAIL) {
			res = _FAIL;
			goto exit;
		}

		rtw_list_insert_tail(&pxmitbuf->list, &(pxmitpriv->free_xmit_extbuf_queue.queue));
		pxmitbuf++;
	}

	pxmitpriv->free_xmit_extbuf_cnt = num_xmit_extbuf;

exit:
	;
}
