/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
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
#define _RTW_CMD_C_

#include <osdep_service.h>
#include <drv_types.h>
#include <recv_osdep.h>
#include <cmd_osdep.h>
#include <mlme_osdep.h>
#include <rtw_br_ext.h>
#include <rtw_mlme_ext.h>

#ifdef CONFIG_BT_COEXIST
#include <rtl8723a_hal.h>
#endif /*  CONFIG_BT_COEXIST */

/*
Caller and the rtw_cmd_thread can protect cmd_q by spin_lock.
No irqsave is necessary.
*/

int	_rtw_init_cmd_priv (struct	cmd_priv *pcmdpriv)
{
	int res = _SUCCESS;

_func_enter_;

	_rtw_init_sema(&(pcmdpriv->cmd_queue_sema), 0);
	/* _rtw_init_sema(&(pcmdpriv->cmd_done_sema), 0); */
	_rtw_init_sema(&(pcmdpriv->terminate_cmdthread_sema), 0);


	_rtw_init_queue(&(pcmdpriv->cmd_queue));

	/* allocate DMA-able/Non-Page memory for cmd_buf and rsp_buf */

	pcmdpriv->cmd_seq = 1;

	pcmdpriv->cmd_allocated_buf = rtw_zmalloc(MAX_CMDSZ + CMDBUFF_ALIGN_SZ);

	if (pcmdpriv->cmd_allocated_buf == NULL) {
		res = _FAIL;
		goto exit;
	}

	pcmdpriv->cmd_buf = pcmdpriv->cmd_allocated_buf  +  CMDBUFF_ALIGN_SZ - ((size_t)(pcmdpriv->cmd_allocated_buf) & (CMDBUFF_ALIGN_SZ-1));

	pcmdpriv->rsp_allocated_buf = rtw_zmalloc(MAX_RSPSZ + 4);

	if (pcmdpriv->rsp_allocated_buf == NULL) {
		res = _FAIL;
		goto exit;
	}

	pcmdpriv->rsp_buf = pcmdpriv->rsp_allocated_buf  +  4 - ((size_t)(pcmdpriv->rsp_allocated_buf) & 3);

	pcmdpriv->cmd_issued_cnt = 0;
	pcmdpriv->cmd_done_cnt = 0;
	pcmdpriv->rsp_cnt = 0;
exit:
_func_exit_;
	return res;
}

static void c2h_wk_callback(struct work_struct *work);

int _rtw_init_evt_priv(struct evt_priv *pevtpriv)
{
	int res = _SUCCESS;

_func_enter_;

	/* allocate DMA-able/Non-Page memory for cmd_buf and rsp_buf */
	ATOMIC_SET(&pevtpriv->event_seq, 0);
	pevtpriv->evt_done_cnt = 0;

	_init_workitem(&pevtpriv->c2h_wk, c2h_wk_callback, NULL);
	pevtpriv->c2h_wk_alive = false;
	pevtpriv->c2h_queue = rtw_cbuf_alloc(C2H_QUEUE_MAX_LEN+1);

_func_exit_;

	return res;
}

void _rtw_free_evt_priv(struct	evt_priv *pevtpriv)
{
_func_enter_;

	RT_TRACE(_module_rtl871x_cmd_c_, _drv_info_, ("+_rtw_free_evt_priv\n"));

	_cancel_workitem_sync(&pevtpriv->c2h_wk);
	while (pevtpriv->c2h_wk_alive)
		rtw_msleep_os(10);

	while (!rtw_cbuf_empty(pevtpriv->c2h_queue)) {
		void *c2h = rtw_cbuf_pop(pevtpriv->c2h_queue);
		if (c2h != NULL && c2h != (void *)pevtpriv)
			rtw_mfree(c2h, 16);
	}
	RT_TRACE(_module_rtl871x_cmd_c_, _drv_info_, ("-_rtw_free_evt_priv\n"));

_func_exit_;
}

void _rtw_free_cmd_priv (struct	cmd_priv *pcmdpriv)
{
_func_enter_;

	if (pcmdpriv) {
		_rtw_spinlock_free(&(pcmdpriv->cmd_queue.lock));
		_rtw_free_sema(&(pcmdpriv->cmd_queue_sema));
		/* _rtw_free_sema(&(pcmdpriv->cmd_done_sema)); */
		_rtw_free_sema(&(pcmdpriv->terminate_cmdthread_sema));

		if (pcmdpriv->cmd_allocated_buf)
			rtw_mfree(pcmdpriv->cmd_allocated_buf, MAX_CMDSZ + CMDBUFF_ALIGN_SZ);

		if (pcmdpriv->rsp_allocated_buf)
			rtw_mfree(pcmdpriv->rsp_allocated_buf, MAX_RSPSZ + 4);
	}
_func_exit_;
}

/*
Calling Context:

rtw_enqueue_cmd can only be called between kernel thread,
since only spin_lock is used.

ISR/Call-Back functions can't call this sub-function.

*/

int	_rtw_enqueue_cmd(struct __queue *queue, struct cmd_obj *obj)
{
	unsigned long irqL;

_func_enter_;

	if (obj == NULL)
		goto exit;

	/* _enter_critical_bh(&queue->lock, &irqL); */
	_enter_critical(&queue->lock, &irqL);

	rtw_list_insert_tail(&obj->list, &queue->queue);

	/* _exit_critical_bh(&queue->lock, &irqL); */
	_exit_critical(&queue->lock, &irqL);

exit:

_func_exit_;

	return _SUCCESS;
}

struct	cmd_obj	*_rtw_dequeue_cmd(struct __queue *queue)
{
	unsigned long irqL;
	struct cmd_obj *obj;

_func_enter_;

	/* _enter_critical_bh(&(queue->lock), &irqL); */
	_enter_critical(&queue->lock, &irqL);
	if (rtw_is_list_empty(&(queue->queue))) {
		obj = NULL;
	} else {
		obj = LIST_CONTAINOR(get_next(&(queue->queue)), struct cmd_obj, list);
		rtw_list_delete(&obj->list);
	}

	/* _exit_critical_bh(&(queue->lock), &irqL); */
	_exit_critical(&queue->lock, &irqL);

_func_exit_;

	return obj;
}

u32	rtw_init_cmd_priv(struct cmd_priv *pcmdpriv)
{
	u32	res;
_func_enter_;
	res = _rtw_init_cmd_priv (pcmdpriv);
_func_exit_;
	return res;
}

u32	rtw_init_evt_priv (struct	evt_priv *pevtpriv)
{
	int	res;
_func_enter_;
	res = _rtw_init_evt_priv(pevtpriv);
_func_exit_;
	return res;
}

void rtw_free_evt_priv (struct	evt_priv *pevtpriv)
{
_func_enter_;
	RT_TRACE(_module_rtl871x_cmd_c_, _drv_info_, ("rtw_free_evt_priv\n"));
	_rtw_free_evt_priv(pevtpriv);
_func_exit_;
}

void rtw_free_cmd_priv (struct	cmd_priv *pcmdpriv)
{
_func_enter_;
	RT_TRACE(_module_rtl871x_cmd_c_, _drv_info_, ("rtw_free_cmd_priv\n"));
	_rtw_free_cmd_priv(pcmdpriv);
_func_exit_;
}

int rtw_cmd_filter(struct cmd_priv *pcmdpriv, struct cmd_obj *cmd_obj);
int rtw_cmd_filter(struct cmd_priv *pcmdpriv, struct cmd_obj *cmd_obj)
{
	u8 bAllow = false; /* set to true to allow enqueuing cmd when hw_init_completed is false */

	/* To decide allow or not */
	if ((pcmdpriv->padapter->pwrctrlpriv.bHWPwrPindetect) &&
	    (!pcmdpriv->padapter->registrypriv.usbss_enable)) {
		if (cmd_obj->cmdcode == GEN_CMD_CODE(_Set_Drv_Extra)) {
			struct drvextra_cmd_parm	*pdrvextra_cmd_parm = (struct drvextra_cmd_parm	*)cmd_obj->parmbuf;
			if (pdrvextra_cmd_parm->ec_id == POWER_SAVING_CTRL_WK_CID)
				bAllow = true;
		}
	}

	if (cmd_obj->cmdcode == GEN_CMD_CODE(_SetChannelPlan))
		bAllow = true;

	if ((!pcmdpriv->padapter->hw_init_completed && !bAllow) ||
	    !pcmdpriv->cmdthd_running)	/* com_thread not running */
		return _FAIL;
	return _SUCCESS;
}

u32 rtw_enqueue_cmd(struct cmd_priv *pcmdpriv, struct cmd_obj *cmd_obj)
{
	int res = _FAIL;
	struct adapter *padapter = pcmdpriv->padapter;

_func_enter_;

	if (cmd_obj == NULL)
		goto exit;

	cmd_obj->padapter = padapter;

	res = rtw_cmd_filter(pcmdpriv, cmd_obj);
	if (_FAIL == res) {
		rtw_free_cmd_obj(cmd_obj);
		goto exit;
	}

	res = _rtw_enqueue_cmd(&pcmdpriv->cmd_queue, cmd_obj);

	if (res == _SUCCESS)
		_rtw_up_sema(&pcmdpriv->cmd_queue_sema);

exit:

_func_exit_;

	return res;
}

struct	cmd_obj	*rtw_dequeue_cmd(struct cmd_priv *pcmdpriv)
{
	struct cmd_obj *cmd_obj;

_func_enter_;

	cmd_obj = _rtw_dequeue_cmd(&pcmdpriv->cmd_queue);

_func_exit_;
	return cmd_obj;
}

void rtw_cmd_clr_isr(struct	cmd_priv *pcmdpriv)
{
_func_enter_;
	pcmdpriv->cmd_done_cnt++;
	/* _rtw_up_sema(&(pcmdpriv->cmd_done_sema)); */
_func_exit_;
}

void rtw_free_cmd_obj(struct cmd_obj *pcmd)
{
_func_enter_;

	if ((pcmd->cmdcode != _JoinBss_CMD_) && (pcmd->cmdcode != _CreateBss_CMD_)) {
		/* free parmbuf in cmd_obj */
		rtw_mfree((unsigned char *)pcmd->parmbuf, pcmd->cmdsz);
	}

	if (pcmd->rsp != NULL) {
		if (pcmd->rspsz != 0) {
			/* free rsp in cmd_obj */
			rtw_mfree((unsigned char *)pcmd->rsp, pcmd->rspsz);
		}
	}

	/* free cmd_obj */
	rtw_mfree((unsigned char *)pcmd, sizeof(struct cmd_obj));

_func_exit_;
}

int rtw_cmd_thread(void *context)
{
	u8 ret;
	struct cmd_obj *pcmd;
	u8 *pcmdbuf;
	u8 (*cmd_hdl)(struct adapter *padapter, u8 *pbuf);
	void (*pcmd_callback)(struct adapter *dev, struct cmd_obj *pcmd);
	struct adapter *padapter = (struct adapter *)context;
	struct cmd_priv *pcmdpriv = &(padapter->cmdpriv);

_func_enter_;

	thread_enter("RTW_CMD_THREAD");

	pcmdbuf = pcmdpriv->cmd_buf;

	pcmdpriv->cmdthd_running = true;
	_rtw_up_sema(&pcmdpriv->terminate_cmdthread_sema);

	RT_TRACE(_module_rtl871x_cmd_c_, _drv_info_, ("start r871x rtw_cmd_thread !!!!\n"));

	while (1) {
		if (_rtw_down_sema(&pcmdpriv->cmd_queue_sema) == _FAIL)
			break;

		if (padapter->bDriverStopped ||
		    padapter->bSurpriseRemoved) {
			DBG_88E("%s: DriverStopped(%d) SurpriseRemoved(%d) break at line %d\n",
				__func__, padapter->bDriverStopped, padapter->bSurpriseRemoved, __LINE__);
			break;
		}
_next:
		if (padapter->bDriverStopped ||
		    padapter->bSurpriseRemoved) {
			DBG_88E("%s: DriverStopped(%d) SurpriseRemoved(%d) break at line %d\n",
				__func__, padapter->bDriverStopped, padapter->bSurpriseRemoved, __LINE__);
			break;
		}

		pcmd = rtw_dequeue_cmd(pcmdpriv);
		if (!pcmd)
			continue;

		if (_FAIL == rtw_cmd_filter(pcmdpriv, pcmd)) {
			pcmd->res = H2C_DROPPED;
			goto post_process;
		}

		pcmdpriv->cmd_issued_cnt++;

		pcmd->cmdsz = _RND4((pcmd->cmdsz));/* _RND4 */

		_rtw_memcpy(pcmdbuf, pcmd->parmbuf, pcmd->cmdsz);

		if (pcmd->cmdcode < (sizeof(wlancmds) / sizeof(struct cmd_hdl))) {
			cmd_hdl = wlancmds[pcmd->cmdcode].h2cfuns;

			if (cmd_hdl) {
				ret = cmd_hdl(pcmd->padapter, pcmdbuf);
				pcmd->res = ret;
			}

			pcmdpriv->cmd_seq++;
		} else {
			pcmd->res = H2C_PARAMETERS_ERROR;
		}

		cmd_hdl = NULL;

post_process:

		/* call callback function for post-processed */
		if (pcmd->cmdcode < (sizeof(rtw_cmd_callback) / sizeof(struct _cmd_callback))) {
			pcmd_callback = rtw_cmd_callback[pcmd->cmdcode].callback;
			if (pcmd_callback == NULL) {
				RT_TRACE(_module_rtl871x_cmd_c_, _drv_info_, ("mlme_cmd_hdl(): pcmd_callback = 0x%p, cmdcode = 0x%x\n", pcmd_callback, pcmd->cmdcode));
				rtw_free_cmd_obj(pcmd);
			} else {
				/* todo: !!! fill rsp_buf to pcmd->rsp if (pcmd->rsp!= NULL) */
				pcmd_callback(pcmd->padapter, pcmd);/* need conider that free cmd_obj in rtw_cmd_callback */
			}
		} else {
			RT_TRACE(_module_rtl871x_cmd_c_, _drv_err_, ("%s: cmdcode = 0x%x callback not defined!\n", __func__, pcmd->cmdcode));
			rtw_free_cmd_obj(pcmd);
		}

		flush_signals_thread();

		goto _next;
	}
	pcmdpriv->cmdthd_running = false;

	/*  free all cmd_obj resources */
	do {
		pcmd = rtw_dequeue_cmd(pcmdpriv);
		if (pcmd == NULL)
			break;

		/* DBG_88E("%s: leaving... drop cmdcode:%u\n", __func__, pcmd->cmdcode); */

		rtw_free_cmd_obj(pcmd);
	} while (1);

	_rtw_up_sema(&pcmdpriv->terminate_cmdthread_sema);

_func_exit_;

	thread_exit();
}

u8 rtw_setstandby_cmd(struct adapter *padapter, uint action)
{
	struct cmd_obj *ph2c;
	struct usb_suspend_parm *psetusbsuspend;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;

	u8 ret = _SUCCESS;

_func_enter_;

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		ret = _FAIL;
		goto exit;
	}

	psetusbsuspend = (struct usb_suspend_parm *)rtw_zmalloc(sizeof(struct usb_suspend_parm));
	if (psetusbsuspend == NULL) {
		rtw_mfree((u8 *)ph2c, sizeof(struct	cmd_obj));
		ret = _FAIL;
		goto exit;
	}

	psetusbsuspend->action = action;

	init_h2fwcmd_w_parm_no_rsp(ph2c, psetusbsuspend, GEN_CMD_CODE(_SetUsbSuspend));

	ret = rtw_enqueue_cmd(pcmdpriv, ph2c);

exit:

_func_exit_;

	return ret;
}

/*
rtw_sitesurvey_cmd(~)
	### NOTE:#### (!!!!)
	MUST TAKE CARE THAT BEFORE CALLING THIS FUNC, YOU SHOULD HAVE LOCKED pmlmepriv->lock
*/
u8 rtw_sitesurvey_cmd(struct adapter  *padapter, struct ndis_802_11_ssid *ssid, int ssid_num,
	struct rtw_ieee80211_channel *ch, int ch_num)
{
	u8 res = _FAIL;
	struct cmd_obj		*ph2c;
	struct sitesurvey_parm	*psurveyPara;
	struct cmd_priv		*pcmdpriv = &padapter->cmdpriv;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;

_func_enter_;

	if (check_fwstate(pmlmepriv, _FW_LINKED) == true) {
		rtw_lps_ctrl_wk_cmd(padapter, LPS_CTRL_SCAN, 1);
	}

	if (check_fwstate(pmlmepriv, _FW_LINKED) == true) {
		p2p_ps_wk_cmd(padapter, P2P_PS_SCAN, 1);
	}

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL)
		return _FAIL;

	psurveyPara = (struct sitesurvey_parm *)rtw_zmalloc(sizeof(struct sitesurvey_parm));
	if (psurveyPara == NULL) {
		rtw_mfree((unsigned char *)ph2c, sizeof(struct cmd_obj));
		return _FAIL;
	}

	rtw_free_network_queue(padapter, false);

	RT_TRACE(_module_rtl871x_cmd_c_, _drv_info_, ("%s: flush network queue\n", __func__));

	init_h2fwcmd_w_parm_no_rsp(ph2c, psurveyPara, GEN_CMD_CODE(_SiteSurvey));

	/* psurveyPara->bsslimit = 48; */
	psurveyPara->scan_mode = pmlmepriv->scan_mode;

	/* prepare ssid list */
	if (ssid) {
		int i;
		for (i = 0; i < ssid_num && i < RTW_SSID_SCAN_AMOUNT; i++) {
			if (ssid[i].SsidLength) {
				_rtw_memcpy(&psurveyPara->ssid[i], &ssid[i], sizeof(struct ndis_802_11_ssid));
				psurveyPara->ssid_num++;
				if (0)
				DBG_88E(FUNC_ADPT_FMT" ssid:(%s, %d)\n", FUNC_ADPT_ARG(padapter),
					psurveyPara->ssid[i].Ssid, psurveyPara->ssid[i].SsidLength);
			}
		}
	}

	/* prepare channel list */
	if (ch) {
		int i;
		for (i = 0; i < ch_num && i < RTW_CHANNEL_SCAN_AMOUNT; i++) {
			if (ch[i].hw_value && !(ch[i].flags & RTW_IEEE80211_CHAN_DISABLED)) {
				_rtw_memcpy(&psurveyPara->ch[i], &ch[i], sizeof(struct rtw_ieee80211_channel));
				psurveyPara->ch_num++;
				if (0)
				DBG_88E(FUNC_ADPT_FMT" ch:%u\n", FUNC_ADPT_ARG(padapter),
					psurveyPara->ch[i].hw_value);
			}
		}
	}

	set_fwstate(pmlmepriv, _FW_UNDER_SURVEY);

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);

	if (res == _SUCCESS) {
		pmlmepriv->scan_start_time = rtw_get_current_time();

		_set_timer(&pmlmepriv->scan_to_timer, SCANNING_TIMEOUT);

		rtw_led_control(padapter, LED_CTL_SITE_SURVEY);

		pmlmepriv->scan_interval = SCAN_INTERVAL;/*  30*2 sec = 60sec */
	} else {
		_clr_fwstate_(pmlmepriv, _FW_UNDER_SURVEY);
	}

_func_exit_;

	return res;
}

u8 rtw_setdatarate_cmd(struct adapter *padapter, u8 *rateset)
{
	struct cmd_obj *ph2c;
	struct setdatarate_parm *pbsetdataratepara;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;

_func_enter_;

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}

	pbsetdataratepara = (struct setdatarate_parm *)rtw_zmalloc(sizeof(struct setdatarate_parm));
	if (pbsetdataratepara == NULL) {
		rtw_mfree((u8 *)ph2c, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	init_h2fwcmd_w_parm_no_rsp(ph2c, pbsetdataratepara, GEN_CMD_CODE(_SetDataRate));
	pbsetdataratepara->mac_id = 5;
	_rtw_memcpy(pbsetdataratepara->datarates, rateset, NumRates);
	res = rtw_enqueue_cmd(pcmdpriv, ph2c);
exit:

_func_exit_;

	return res;
}

u8 rtw_setbasicrate_cmd(struct adapter *padapter, u8 *rateset)
{
	struct cmd_obj *ph2c;
	struct setbasicrate_parm *pssetbasicratepara;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;

_func_enter_;

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}
	pssetbasicratepara = (struct setbasicrate_parm *)rtw_zmalloc(sizeof(struct setbasicrate_parm));

	if (pssetbasicratepara == NULL) {
		rtw_mfree((u8 *)ph2c, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	init_h2fwcmd_w_parm_no_rsp(ph2c, pssetbasicratepara, _SetBasicRate_CMD_);

	_rtw_memcpy(pssetbasicratepara->basicrates, rateset, NumRates);

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);
exit:

_func_exit_;

	return res;
}


/*
unsigned char rtw_setphy_cmd(unsigned char  *adapter)

1.  be called only after rtw_update_registrypriv_dev_network(~) or mp testing program
2.  for AdHoc/Ap mode or mp mode?

*/
u8 rtw_setphy_cmd(struct adapter *padapter, u8 modem, u8 ch)
{
	struct cmd_obj *ph2c;
	struct setphy_parm *psetphypara;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;

_func_enter_;

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
		}
	psetphypara = (struct setphy_parm *)rtw_zmalloc(sizeof(struct setphy_parm));

	if (psetphypara == NULL) {
		rtw_mfree((u8 *)ph2c, sizeof(struct	cmd_obj));
		res = _FAIL;
		goto exit;
	}

	init_h2fwcmd_w_parm_no_rsp(ph2c, psetphypara, _SetPhy_CMD_);

	RT_TRACE(_module_rtl871x_cmd_c_, _drv_info_, ("CH =%d, modem =%d", ch, modem));

	psetphypara->modem = modem;
	psetphypara->rfchannel = ch;

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);
exit:
_func_exit_;
	return res;
}

u8 rtw_setbbreg_cmd(struct adapter *padapter, u8 offset, u8 val)
{
	struct cmd_obj *ph2c;
	struct writeBB_parm *pwritebbparm;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;

_func_enter_;
	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
		}
	pwritebbparm = (struct writeBB_parm *)rtw_zmalloc(sizeof(struct writeBB_parm));

	if (pwritebbparm == NULL) {
		rtw_mfree((u8 *)ph2c, sizeof(struct	cmd_obj));
		res = _FAIL;
		goto exit;
	}

	init_h2fwcmd_w_parm_no_rsp(ph2c, pwritebbparm, GEN_CMD_CODE(_SetBBReg));

	pwritebbparm->offset = offset;
	pwritebbparm->value = val;

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);
exit:
_func_exit_;
	return res;
}

u8 rtw_getbbreg_cmd(struct adapter  *padapter, u8 offset, u8 *pval)
{
	struct cmd_obj *ph2c;
	struct readBB_parm *prdbbparm;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;

_func_enter_;
	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
		}
	prdbbparm = (struct readBB_parm *)rtw_zmalloc(sizeof(struct readBB_parm));

	if (prdbbparm == NULL) {
		rtw_mfree((unsigned char *)ph2c, sizeof(struct	cmd_obj));
		return _FAIL;
	}

	_rtw_init_listhead(&ph2c->list);
	ph2c->cmdcode = GEN_CMD_CODE(_GetBBReg);
	ph2c->parmbuf = (unsigned char *)prdbbparm;
	ph2c->cmdsz =  sizeof(struct readBB_parm);
	ph2c->rsp = pval;
	ph2c->rspsz = sizeof(struct readBB_rsp);

	prdbbparm->offset = offset;

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);
exit:
_func_exit_;
	return res;
}

u8 rtw_setrfreg_cmd(struct adapter  *padapter, u8 offset, u32 val)
{
	struct cmd_obj *ph2c;
	struct writeRF_parm *pwriterfparm;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;
_func_enter_;
	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}
	pwriterfparm = (struct writeRF_parm *)rtw_zmalloc(sizeof(struct writeRF_parm));

	if (pwriterfparm == NULL) {
		rtw_mfree((u8 *)ph2c, sizeof(struct	cmd_obj));
		res = _FAIL;
		goto exit;
	}

	init_h2fwcmd_w_parm_no_rsp(ph2c, pwriterfparm, GEN_CMD_CODE(_SetRFReg));

	pwriterfparm->offset = offset;
	pwriterfparm->value = val;

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);
exit:
_func_exit_;
	return res;
}

u8 rtw_getrfreg_cmd(struct adapter  *padapter, u8 offset, u8 *pval)
{
	struct cmd_obj *ph2c;
	struct readRF_parm *prdrfparm;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;

_func_enter_;

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}

	prdrfparm = (struct readRF_parm *)rtw_zmalloc(sizeof(struct readRF_parm));
	if (prdrfparm == NULL) {
		rtw_mfree((u8 *)ph2c, sizeof(struct	cmd_obj));
		res = _FAIL;
		goto exit;
	}

	_rtw_init_listhead(&ph2c->list);
	ph2c->cmdcode = GEN_CMD_CODE(_GetRFReg);
	ph2c->parmbuf = (unsigned char *)prdrfparm;
	ph2c->cmdsz =  sizeof(struct readRF_parm);
	ph2c->rsp = pval;
	ph2c->rspsz = sizeof(struct readRF_rsp);

	prdrfparm->offset = offset;

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);

exit:

_func_exit_;

	return res;
}

void rtw_getbbrfreg_cmdrsp_callback(struct adapter *padapter,  struct cmd_obj *pcmd)
{
 _func_enter_;

	rtw_mfree((unsigned char *)pcmd->parmbuf, pcmd->cmdsz);
	rtw_mfree((unsigned char *)pcmd, sizeof(struct cmd_obj));

	if (padapter->registrypriv.mp_mode == 1)
		padapter->mppriv.workparam.bcompleted = true;
_func_exit_;
}

void rtw_readtssi_cmdrsp_callback(struct adapter *padapter,  struct cmd_obj *pcmd)
{
 _func_enter_;

	rtw_mfree((unsigned char *)pcmd->parmbuf, pcmd->cmdsz);
	rtw_mfree((unsigned char *)pcmd, sizeof(struct cmd_obj));

	if (padapter->registrypriv.mp_mode == 1)
		padapter->mppriv.workparam.bcompleted = true;
_func_exit_;
}

u8 rtw_createbss_cmd(struct adapter  *padapter)
{
	struct cmd_obj *pcmd;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct wlan_bssid_ex *pdev_network = &padapter->registrypriv.dev_network;
	u8	res = _SUCCESS;

_func_enter_;

	rtw_led_control(padapter, LED_CTL_START_TO_LINK);

	if (pmlmepriv->assoc_ssid.SsidLength == 0)
		RT_TRACE(_module_rtl871x_cmd_c_, _drv_info_, (" createbss for Any SSid:%s\n", pmlmepriv->assoc_ssid.Ssid));
	else
		RT_TRACE(_module_rtl871x_cmd_c_, _drv_info_, (" createbss for SSid:%s\n", pmlmepriv->assoc_ssid.Ssid));

	pcmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmd == NULL) {
		res = _FAIL;
		goto exit;
	}

	_rtw_init_listhead(&pcmd->list);
	pcmd->cmdcode = _CreateBss_CMD_;
	pcmd->parmbuf = (unsigned char *)pdev_network;
	pcmd->cmdsz = get_wlan_bssid_ex_sz((struct wlan_bssid_ex *)pdev_network);
	pcmd->rsp = NULL;
	pcmd->rspsz = 0;
	pdev_network->Length = pcmd->cmdsz;
	res = rtw_enqueue_cmd(pcmdpriv, pcmd);
exit:

_func_exit_;

	return res;
}

u8 rtw_createbss_cmd_ex(struct adapter  *padapter, unsigned char *pbss, unsigned int sz)
{
	struct cmd_obj *pcmd;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;

_func_enter_;

	pcmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmd == NULL) {
		res = _FAIL;
		goto exit;
	}

	_rtw_init_listhead(&pcmd->list);
	pcmd->cmdcode = GEN_CMD_CODE(_CreateBss);
	pcmd->parmbuf = pbss;
	pcmd->cmdsz =  sz;
	pcmd->rsp = NULL;
	pcmd->rspsz = 0;

	res = rtw_enqueue_cmd(pcmdpriv, pcmd);

exit:

_func_exit_;

	return res;
}

u8 rtw_joinbss_cmd(struct adapter  *padapter, struct wlan_network *pnetwork)
{
	u8	res = _SUCCESS;
	uint	t_len = 0;
	struct wlan_bssid_ex		*psecnetwork;
	struct cmd_obj		*pcmd;
	struct cmd_priv		*pcmdpriv = &padapter->cmdpriv;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct qos_priv		*pqospriv = &pmlmepriv->qospriv;
	struct security_priv	*psecuritypriv = &padapter->securitypriv;
	struct registry_priv	*pregistrypriv = &padapter->registrypriv;
	struct ht_priv		*phtpriv = &pmlmepriv->htpriv;
	enum ndis_802_11_network_infra ndis_network_mode = pnetwork->network.InfrastructureMode;
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);

_func_enter_;

	rtw_led_control(padapter, LED_CTL_START_TO_LINK);

	if (pmlmepriv->assoc_ssid.SsidLength == 0) {
		RT_TRACE(_module_rtl871x_cmd_c_, _drv_info_, ("+Join cmd: Any SSid\n"));
	} else {
		RT_TRACE(_module_rtl871x_cmd_c_, _drv_notice_, ("+Join cmd: SSid =[%s]\n", pmlmepriv->assoc_ssid.Ssid));
	}

	pcmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (pcmd == NULL) {
		res = _FAIL;
		RT_TRACE(_module_rtl871x_cmd_c_, _drv_err_, ("rtw_joinbss_cmd: memory allocate for cmd_obj fail!!!\n"));
		goto exit;
	}
	/* for IEs is fix buf size */
	t_len = sizeof(struct wlan_bssid_ex);


	/* for hidden ap to set fw_state here */
	if (!check_fwstate(pmlmepriv, WIFI_STATION_STATE|WIFI_ADHOC_STATE)) {
		switch (ndis_network_mode) {
		case Ndis802_11IBSS:
			set_fwstate(pmlmepriv, WIFI_ADHOC_STATE);
			break;
		case Ndis802_11Infrastructure:
			set_fwstate(pmlmepriv, WIFI_STATION_STATE);
			break;
		case Ndis802_11APMode:
		case Ndis802_11AutoUnknown:
		case Ndis802_11InfrastructureMax:
			break;
		}
	}

	psecnetwork = (struct wlan_bssid_ex *)&psecuritypriv->sec_bss;
	if (psecnetwork == NULL) {
		if (pcmd != NULL)
			rtw_mfree((unsigned char *)pcmd, sizeof(struct	cmd_obj));

		res = _FAIL;

		RT_TRACE(_module_rtl871x_cmd_c_, _drv_err_, ("rtw_joinbss_cmd :psecnetwork == NULL!!!\n"));

		goto exit;
	}

	_rtw_memset(psecnetwork, 0, t_len);

	_rtw_memcpy(psecnetwork, &pnetwork->network, get_wlan_bssid_ex_sz(&pnetwork->network));

	psecuritypriv->authenticator_ie[0] = (unsigned char)psecnetwork->IELength;

	if ((psecnetwork->IELength-12) < (256-1)) {
		_rtw_memcpy(&psecuritypriv->authenticator_ie[1], &psecnetwork->IEs[12], psecnetwork->IELength-12);
	} else {
		_rtw_memcpy(&psecuritypriv->authenticator_ie[1], &psecnetwork->IEs[12], (256-1));
	}

	psecnetwork->IELength = 0;
	/*  Added by Albert 2009/02/18 */
	/*  If the the driver wants to use the bssid to create the connection. */
	/*  If not,  we have to copy the connecting AP's MAC address to it so that */
	/*  the driver just has the bssid information for PMKIDList searching. */

	if (!pmlmepriv->assoc_by_bssid)
		_rtw_memcpy(&pmlmepriv->assoc_bssid[0], &pnetwork->network.MacAddress[0], ETH_ALEN);

	psecnetwork->IELength = rtw_restruct_sec_ie(padapter, &pnetwork->network.IEs[0], &psecnetwork->IEs[0], pnetwork->network.IELength);


	pqospriv->qos_option = 0;

	if (pregistrypriv->wmm_enable) {
		u32 tmp_len;

		tmp_len = rtw_restruct_wmm_ie(padapter, &pnetwork->network.IEs[0], &psecnetwork->IEs[0], pnetwork->network.IELength, psecnetwork->IELength);

		if (psecnetwork->IELength != tmp_len) {
			psecnetwork->IELength = tmp_len;
			pqospriv->qos_option = 1; /* There is WMM IE in this corresp. beacon */
		} else {
			pqospriv->qos_option = 0;/* There is no WMM IE in this corresp. beacon */
		}
	}

	phtpriv->ht_option = false;
	if (pregistrypriv->ht_enable) {
		/* 	Added by Albert 2010/06/23 */
		/* 	For the WEP mode, we will use the bg mode to do the connection to avoid some IOT issue. */
		/* 	Especially for Realtek 8192u SoftAP. */
		if ((padapter->securitypriv.dot11PrivacyAlgrthm != _WEP40_) &&
		    (padapter->securitypriv.dot11PrivacyAlgrthm != _WEP104_) &&
		    (padapter->securitypriv.dot11PrivacyAlgrthm != _TKIP_)) {
			/* rtw_restructure_ht_ie */
			rtw_restructure_ht_ie(padapter, &pnetwork->network.IEs[0], &psecnetwork->IEs[0],
									pnetwork->network.IELength, &psecnetwork->IELength);
		}
	}

	pmlmeinfo->assoc_AP_vendor = check_assoc_AP(pnetwork->network.IEs, pnetwork->network.IELength);

	if (pmlmeinfo->assoc_AP_vendor == HT_IOT_PEER_TENDA)
		padapter->pwrctrlpriv.smart_ps = 0;
	else
		padapter->pwrctrlpriv.smart_ps = padapter->registrypriv.smart_ps;

	DBG_88E("%s: smart_ps =%d\n", __func__, padapter->pwrctrlpriv.smart_ps);

	pcmd->cmdsz = get_wlan_bssid_ex_sz(psecnetwork);/* get cmdsz before endian conversion */

	_rtw_init_listhead(&pcmd->list);
	pcmd->cmdcode = _JoinBss_CMD_;/* GEN_CMD_CODE(_JoinBss) */
	pcmd->parmbuf = (unsigned char *)psecnetwork;
	pcmd->rsp = NULL;
	pcmd->rspsz = 0;

	res = rtw_enqueue_cmd(pcmdpriv, pcmd);

exit:

_func_exit_;

	return res;
}

u8 rtw_disassoc_cmd(struct adapter *padapter, u32 deauth_timeout_ms, bool enqueue) /* for sta_mode */
{
	struct cmd_obj *cmdobj = NULL;
	struct disconnect_parm *param = NULL;
	struct cmd_priv *cmdpriv = &padapter->cmdpriv;
	u8 res = _SUCCESS;

_func_enter_;

	RT_TRACE(_module_rtl871x_cmd_c_, _drv_notice_, ("+rtw_disassoc_cmd\n"));

	/* prepare cmd parameter */
	param = (struct disconnect_parm *)rtw_zmalloc(sizeof(*param));
	if (param == NULL) {
		res = _FAIL;
		goto exit;
	}
	param->deauth_timeout_ms = deauth_timeout_ms;

	if (enqueue) {
		/* need enqueue, prepare cmd_obj and enqueue */
		cmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(*cmdobj));
		if (cmdobj == NULL) {
			res = _FAIL;
			rtw_mfree((u8 *)param, sizeof(*param));
			goto exit;
		}
		init_h2fwcmd_w_parm_no_rsp(cmdobj, param, _DisConnect_CMD_);
		res = rtw_enqueue_cmd(cmdpriv, cmdobj);
	} else {
		/* no need to enqueue, do the cmd hdl directly and free cmd parameter */
		if (H2C_SUCCESS != disconnect_hdl(padapter, (u8 *)param))
			res = _FAIL;
		rtw_mfree((u8 *)param, sizeof(*param));
	}

exit:

_func_exit_;

	return res;
}

u8 rtw_setopmode_cmd(struct adapter  *padapter, enum ndis_802_11_network_infra networktype)
{
	struct	cmd_obj *ph2c;
	struct	setopmode_parm *psetop;

	struct	cmd_priv   *pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;

_func_enter_;

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = false;
		goto exit;
	}
	psetop = (struct setopmode_parm *)rtw_zmalloc(sizeof(struct setopmode_parm));

	if (psetop == NULL) {
		rtw_mfree((u8 *)ph2c, sizeof(struct	cmd_obj));
		res = false;
		goto exit;
	}

	init_h2fwcmd_w_parm_no_rsp(ph2c, psetop, _SetOpMode_CMD_);
	psetop->mode = (u8)networktype;

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);

exit:

_func_exit_;

	return res;
}

u8 rtw_setstakey_cmd(struct adapter *padapter, u8 *psta, u8 unicast_key)
{
	struct cmd_obj *ph2c;
	struct set_stakey_parm *psetstakey_para;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	struct set_stakey_rsp *psetstakey_rsp = NULL;

	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	struct sta_info *sta = (struct sta_info *)psta;
	u8	res = _SUCCESS;

_func_enter_;

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}

	psetstakey_para = (struct set_stakey_parm *)rtw_zmalloc(sizeof(struct set_stakey_parm));
	if (psetstakey_para == NULL) {
		rtw_mfree((u8 *)ph2c, sizeof(struct	cmd_obj));
		res = _FAIL;
		goto exit;
	}

	psetstakey_rsp = (struct set_stakey_rsp *)rtw_zmalloc(sizeof(struct set_stakey_rsp));
	if (psetstakey_rsp == NULL) {
		rtw_mfree((u8 *)ph2c, sizeof(struct	cmd_obj));
		rtw_mfree((u8 *)psetstakey_para, sizeof(struct set_stakey_parm));
		res = _FAIL;
		goto exit;
	}

	init_h2fwcmd_w_parm_no_rsp(ph2c, psetstakey_para, _SetStaKey_CMD_);
	ph2c->rsp = (u8 *)psetstakey_rsp;
	ph2c->rspsz = sizeof(struct set_stakey_rsp);

	_rtw_memcpy(psetstakey_para->addr, sta->hwaddr, ETH_ALEN);

	if (check_fwstate(pmlmepriv, WIFI_STATION_STATE))
		psetstakey_para->algorithm = (unsigned char) psecuritypriv->dot11PrivacyAlgrthm;
	else
		GET_ENCRY_ALGO(psecuritypriv, sta, psetstakey_para->algorithm, false);

	if (unicast_key)
		_rtw_memcpy(&psetstakey_para->key, &sta->dot118021x_UncstKey, 16);
	else
		_rtw_memcpy(&psetstakey_para->key, &psecuritypriv->dot118021XGrpKey[psecuritypriv->dot118021XGrpKeyid].skey, 16);

	/* jeff: set this becasue at least sw key is ready */
	padapter->securitypriv.busetkipkey = true;

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);

exit:

_func_exit_;

	return res;
}

u8 rtw_clearstakey_cmd(struct adapter *padapter, u8 *psta, u8 entry, u8 enqueue)
{
	struct cmd_obj *ph2c;
	struct set_stakey_parm	*psetstakey_para;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	struct set_stakey_rsp *psetstakey_rsp = NULL;
	struct sta_info *sta = (struct sta_info *)psta;
	u8	res = _SUCCESS;

_func_enter_;

	if (!enqueue) {
		clear_cam_entry(padapter, entry);
	} else {
		ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
		if (ph2c == NULL) {
			res = _FAIL;
			goto exit;
		}

		psetstakey_para = (struct set_stakey_parm *)rtw_zmalloc(sizeof(struct set_stakey_parm));
		if (psetstakey_para == NULL) {
			rtw_mfree((u8 *)ph2c, sizeof(struct	cmd_obj));
			res = _FAIL;
			goto exit;
		}

		psetstakey_rsp = (struct set_stakey_rsp *)rtw_zmalloc(sizeof(struct set_stakey_rsp));
		if (psetstakey_rsp == NULL) {
			rtw_mfree((u8 *)ph2c, sizeof(struct	cmd_obj));
			rtw_mfree((u8 *)psetstakey_para, sizeof(struct set_stakey_parm));
			res = _FAIL;
			goto exit;
		}

		init_h2fwcmd_w_parm_no_rsp(ph2c, psetstakey_para, _SetStaKey_CMD_);
		ph2c->rsp = (u8 *)psetstakey_rsp;
		ph2c->rspsz = sizeof(struct set_stakey_rsp);

		_rtw_memcpy(psetstakey_para->addr, sta->hwaddr, ETH_ALEN);

		psetstakey_para->algorithm = _NO_PRIVACY_;

		psetstakey_para->id = entry;

		res = rtw_enqueue_cmd(pcmdpriv, ph2c);
	}
exit:

_func_exit_;

	return res;
}

u8 rtw_setrttbl_cmd(struct adapter  *padapter, struct setratable_parm *prate_table)
{
	struct cmd_obj *ph2c;
	struct setratable_parm *psetrttblparm;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;
_func_enter_;

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}
	psetrttblparm = (struct setratable_parm *)rtw_zmalloc(sizeof(struct setratable_parm));

	if (psetrttblparm == NULL) {
		rtw_mfree((unsigned char *)ph2c, sizeof(struct	cmd_obj));
		res = _FAIL;
		goto exit;
	}

	init_h2fwcmd_w_parm_no_rsp(ph2c, psetrttblparm, GEN_CMD_CODE(_SetRaTable));

	_rtw_memcpy(psetrttblparm, prate_table, sizeof(struct setratable_parm));

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);
exit:
_func_exit_;
	return res;
}

u8 rtw_getrttbl_cmd(struct adapter  *padapter, struct getratable_rsp *pval)
{
	struct cmd_obj *ph2c;
	struct getratable_parm *pgetrttblparm;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;
_func_enter_;

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}
	pgetrttblparm = (struct getratable_parm *)rtw_zmalloc(sizeof(struct getratable_parm));

	if (pgetrttblparm == NULL) {
		rtw_mfree((unsigned char *)ph2c, sizeof(struct	cmd_obj));
		res = _FAIL;
		goto exit;
	}

/* 	init_h2fwcmd_w_parm_no_rsp(ph2c, psetrttblparm, GEN_CMD_CODE(_SetRaTable)); */

	_rtw_init_listhead(&ph2c->list);
	ph2c->cmdcode = GEN_CMD_CODE(_GetRaTable);
	ph2c->parmbuf = (unsigned char *)pgetrttblparm;
	ph2c->cmdsz =  sizeof(struct getratable_parm);
	ph2c->rsp = (u8 *)pval;
	ph2c->rspsz = sizeof(struct getratable_rsp);

	pgetrttblparm->rsvd = 0x0;

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);
exit:
_func_exit_;
	return res;
}

u8 rtw_setassocsta_cmd(struct adapter  *padapter, u8 *mac_addr)
{
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	struct cmd_obj *ph2c;
	struct set_assocsta_parm *psetassocsta_para;
	struct set_stakey_rsp *psetassocsta_rsp = NULL;

	u8	res = _SUCCESS;

_func_enter_;

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}

	psetassocsta_para = (struct set_assocsta_parm *)rtw_zmalloc(sizeof(struct set_assocsta_parm));
	if (psetassocsta_para == NULL) {
		rtw_mfree((u8 *)ph2c, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	psetassocsta_rsp = (struct set_stakey_rsp *)rtw_zmalloc(sizeof(struct set_assocsta_rsp));
	if (psetassocsta_rsp == NULL) {
		rtw_mfree((u8 *)ph2c, sizeof(struct	cmd_obj));
		rtw_mfree((u8 *)psetassocsta_para, sizeof(struct set_assocsta_parm));
		return _FAIL;
	}

	init_h2fwcmd_w_parm_no_rsp(ph2c, psetassocsta_para, _SetAssocSta_CMD_);
	ph2c->rsp = (u8 *)psetassocsta_rsp;
	ph2c->rspsz = sizeof(struct set_assocsta_rsp);

	_rtw_memcpy(psetassocsta_para->addr, mac_addr, ETH_ALEN);

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);

exit:

_func_exit_;

	return res;
 }

u8 rtw_addbareq_cmd(struct adapter *padapter, u8 tid, u8 *addr)
{
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;
	struct cmd_obj *ph2c;
	struct addBaReq_parm *paddbareq_parm;
	u8	res = _SUCCESS;

_func_enter_;

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}

	paddbareq_parm = (struct addBaReq_parm *)rtw_zmalloc(sizeof(struct addBaReq_parm));
	if (paddbareq_parm == NULL) {
		rtw_mfree((unsigned char *)ph2c, sizeof(struct	cmd_obj));
		res = _FAIL;
		goto exit;
	}

	paddbareq_parm->tid = tid;
	_rtw_memcpy(paddbareq_parm->addr, addr, ETH_ALEN);

	init_h2fwcmd_w_parm_no_rsp(ph2c, paddbareq_parm, GEN_CMD_CODE(_AddBAReq));

	/* DBG_88E("rtw_addbareq_cmd, tid =%d\n", tid); */

	/* rtw_enqueue_cmd(pcmdpriv, ph2c); */
	res = rtw_enqueue_cmd(pcmdpriv, ph2c);

exit:

_func_exit_;

	return res;
}

u8 rtw_dynamic_chk_wk_cmd(struct adapter *padapter)
{
	struct cmd_obj *ph2c;
	struct drvextra_cmd_parm *pdrvextra_cmd_parm;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;

_func_enter_;

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((unsigned char *)ph2c, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = DYNAMIC_CHK_WK_CID;
	pdrvextra_cmd_parm->type_size = 0;
	pdrvextra_cmd_parm->pbuf = (u8 *)padapter;

	init_h2fwcmd_w_parm_no_rsp(ph2c, pdrvextra_cmd_parm, GEN_CMD_CODE(_Set_Drv_Extra));


	/* rtw_enqueue_cmd(pcmdpriv, ph2c); */
	res = rtw_enqueue_cmd(pcmdpriv, ph2c);
exit:
_func_exit_;
	return res;
}

u8 rtw_set_ch_cmd(struct adapter *padapter, u8 ch, u8 bw, u8 ch_offset, u8 enqueue)
{
	struct cmd_obj *pcmdobj;
	struct set_ch_parm *set_ch_parm;
	struct cmd_priv *pcmdpriv = &padapter->cmdpriv;

	u8 res = _SUCCESS;

_func_enter_;

	DBG_88E(FUNC_NDEV_FMT" ch:%u, bw:%u, ch_offset:%u\n",
		FUNC_NDEV_ARG(padapter->pnetdev), ch, bw, ch_offset);

	/* check input parameter */

	/* prepare cmd parameter */
	set_ch_parm = (struct set_ch_parm *)rtw_zmalloc(sizeof(*set_ch_parm));
	if (set_ch_parm == NULL) {
		res = _FAIL;
		goto exit;
	}
	set_ch_parm->ch = ch;
	set_ch_parm->bw = bw;
	set_ch_parm->ch_offset = ch_offset;

	if (enqueue) {
		/* need enqueue, prepare cmd_obj and enqueue */
		pcmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct	cmd_obj));
		if (pcmdobj == NULL) {
			rtw_mfree((u8 *)set_ch_parm, sizeof(*set_ch_parm));
			res = _FAIL;
			goto exit;
		}

		init_h2fwcmd_w_parm_no_rsp(pcmdobj, set_ch_parm, GEN_CMD_CODE(_SetChannel));
		res = rtw_enqueue_cmd(pcmdpriv, pcmdobj);
	} else {
		/* no need to enqueue, do the cmd hdl directly and free cmd parameter */
		if (H2C_SUCCESS != set_ch_hdl(padapter, (u8 *)set_ch_parm))
			res = _FAIL;

		rtw_mfree((u8 *)set_ch_parm, sizeof(*set_ch_parm));
	}

	/* do something based on res... */

exit:

	DBG_88E(FUNC_NDEV_FMT" res:%u\n", FUNC_NDEV_ARG(padapter->pnetdev), res);

_func_exit_;

	return res;
}

u8 rtw_set_chplan_cmd(struct adapter *padapter, u8 chplan, u8 enqueue)
{
	struct	cmd_obj *pcmdobj;
	struct	SetChannelPlan_param *setChannelPlan_param;
	struct	cmd_priv   *pcmdpriv = &padapter->cmdpriv;

	u8	res = _SUCCESS;

_func_enter_;

	RT_TRACE(_module_rtl871x_cmd_c_, _drv_notice_, ("+rtw_set_chplan_cmd\n"));

	/* check input parameter */
	if (!rtw_is_channel_plan_valid(chplan)) {
		res = _FAIL;
		goto exit;
	}

	/* prepare cmd parameter */
	setChannelPlan_param = (struct	SetChannelPlan_param *)rtw_zmalloc(sizeof(struct SetChannelPlan_param));
	if (setChannelPlan_param == NULL) {
		res = _FAIL;
		goto exit;
	}
	setChannelPlan_param->channel_plan = chplan;

	if (enqueue) {
		/* need enqueue, prepare cmd_obj and enqueue */
		pcmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct	cmd_obj));
		if (pcmdobj == NULL) {
			rtw_mfree((u8 *)setChannelPlan_param, sizeof(struct SetChannelPlan_param));
			res = _FAIL;
			goto exit;
		}

		init_h2fwcmd_w_parm_no_rsp(pcmdobj, setChannelPlan_param, GEN_CMD_CODE(_SetChannelPlan));
		res = rtw_enqueue_cmd(pcmdpriv, pcmdobj);
	} else {
		/* no need to enqueue, do the cmd hdl directly and free cmd parameter */
		if (H2C_SUCCESS != set_chplan_hdl(padapter, (unsigned char *)setChannelPlan_param))
			res = _FAIL;

		rtw_mfree((u8 *)setChannelPlan_param, sizeof(struct SetChannelPlan_param));
	}

	/* do something based on res... */
	if (res == _SUCCESS)
		padapter->mlmepriv.ChannelPlan = chplan;

exit:

_func_exit_;

	return res;
}

u8 rtw_led_blink_cmd(struct adapter *padapter, struct LED_871x *pLed)
{
	struct	cmd_obj *pcmdobj;
	struct	LedBlink_param *ledBlink_param;
	struct	cmd_priv   *pcmdpriv = &padapter->cmdpriv;

	u8	res = _SUCCESS;

_func_enter_;

	RT_TRACE(_module_rtl871x_cmd_c_, _drv_notice_, ("+rtw_led_blink_cmd\n"));

	pcmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct	cmd_obj));
	if (pcmdobj == NULL) {
		res = _FAIL;
		goto exit;
	}

	ledBlink_param = (struct	LedBlink_param *)rtw_zmalloc(sizeof(struct	LedBlink_param));
	if (ledBlink_param == NULL) {
		rtw_mfree((u8 *)pcmdobj, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	ledBlink_param->pLed = pLed;

	init_h2fwcmd_w_parm_no_rsp(pcmdobj, ledBlink_param, GEN_CMD_CODE(_LedBlink));
	res = rtw_enqueue_cmd(pcmdpriv, pcmdobj);

exit:

_func_exit_;

	return res;
}

u8 rtw_set_csa_cmd(struct adapter *padapter, u8 new_ch_no)
{
	struct	cmd_obj *pcmdobj;
	struct	SetChannelSwitch_param *setChannelSwitch_param;
	struct	cmd_priv   *pcmdpriv = &padapter->cmdpriv;

	u8	res = _SUCCESS;

_func_enter_;

	RT_TRACE(_module_rtl871x_cmd_c_, _drv_notice_, ("+rtw_set_csa_cmd\n"));

	pcmdobj = (struct cmd_obj *)rtw_zmalloc(sizeof(struct	cmd_obj));
	if (pcmdobj == NULL) {
		res = _FAIL;
		goto exit;
	}

	setChannelSwitch_param = (struct SetChannelSwitch_param *)rtw_zmalloc(sizeof(struct	SetChannelSwitch_param));
	if (setChannelSwitch_param == NULL) {
		rtw_mfree((u8 *)pcmdobj, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	setChannelSwitch_param->new_ch_no = new_ch_no;

	init_h2fwcmd_w_parm_no_rsp(pcmdobj, setChannelSwitch_param, GEN_CMD_CODE(_SetChannelSwitch));
	res = rtw_enqueue_cmd(pcmdpriv, pcmdobj);

exit:

_func_exit_;

	return res;
}

u8 rtw_tdls_cmd(struct adapter *padapter, u8 *addr, u8 option)
{
	return _SUCCESS;
}

static void traffic_status_watchdog(struct adapter *padapter)
{
	u8	bEnterPS;
	u8	bBusyTraffic = false, bTxBusyTraffic = false, bRxBusyTraffic = false;
	u8	bHigherBusyTraffic = false, bHigherBusyRxTraffic = false, bHigherBusyTxTraffic = false;
	struct mlme_priv		*pmlmepriv = &(padapter->mlmepriv);

	/*  */
	/*  Determine if our traffic is busy now */
	/*  */
	if (check_fwstate(pmlmepriv, _FW_LINKED)) {
#ifdef CONFIG_BT_COEXIST
		if (pmlmepriv->LinkDetectInfo.NumRxOkInPeriod > 50 ||
		    pmlmepriv->LinkDetectInfo.NumTxOkInPeriod > 50)
#else /*  !CONFIG_BT_COEXIST */
		if (pmlmepriv->LinkDetectInfo.NumRxOkInPeriod > 100 ||
		    pmlmepriv->LinkDetectInfo.NumTxOkInPeriod > 100)
#endif /*  !CONFIG_BT_COEXIST */
		{
			bBusyTraffic = true;

			if (pmlmepriv->LinkDetectInfo.NumRxOkInPeriod > pmlmepriv->LinkDetectInfo.NumTxOkInPeriod)
				bRxBusyTraffic = true;
			else
				bTxBusyTraffic = true;
		}

		/*  Higher Tx/Rx data. */
		if (pmlmepriv->LinkDetectInfo.NumRxOkInPeriod > 4000 ||
		    pmlmepriv->LinkDetectInfo.NumTxOkInPeriod > 4000) {
			bHigherBusyTraffic = true;

			if (pmlmepriv->LinkDetectInfo.NumRxOkInPeriod > pmlmepriv->LinkDetectInfo.NumTxOkInPeriod)
				bHigherBusyRxTraffic = true;
			else
				bHigherBusyTxTraffic = true;
		}

#ifdef CONFIG_BT_COEXIST
		if (BT_1Ant(padapter) == false)
#endif
		{
		/*  check traffic for  powersaving. */
		if (((pmlmepriv->LinkDetectInfo.NumRxUnicastOkInPeriod + pmlmepriv->LinkDetectInfo.NumTxOkInPeriod) > 8) ||
		    (pmlmepriv->LinkDetectInfo.NumRxUnicastOkInPeriod > 2))
			bEnterPS = false;
		else
			bEnterPS = true;

		/*  LeisurePS only work in infra mode. */
		if (bEnterPS)
			LPS_Enter(padapter);
		else
			LPS_Leave(padapter);
		}
	} else {
		LPS_Leave(padapter);
	}

	pmlmepriv->LinkDetectInfo.NumRxOkInPeriod = 0;
	pmlmepriv->LinkDetectInfo.NumTxOkInPeriod = 0;
	pmlmepriv->LinkDetectInfo.NumRxUnicastOkInPeriod = 0;
	pmlmepriv->LinkDetectInfo.bBusyTraffic = bBusyTraffic;
	pmlmepriv->LinkDetectInfo.bTxBusyTraffic = bTxBusyTraffic;
	pmlmepriv->LinkDetectInfo.bRxBusyTraffic = bRxBusyTraffic;
	pmlmepriv->LinkDetectInfo.bHigherBusyTraffic = bHigherBusyTraffic;
	pmlmepriv->LinkDetectInfo.bHigherBusyRxTraffic = bHigherBusyRxTraffic;
	pmlmepriv->LinkDetectInfo.bHigherBusyTxTraffic = bHigherBusyTxTraffic;
}

void dynamic_chk_wk_hdl(struct adapter *padapter, u8 *pbuf, int sz);
void dynamic_chk_wk_hdl(struct adapter *padapter, u8 *pbuf, int sz)
{
	struct mlme_priv *pmlmepriv;

	padapter = (struct adapter *)pbuf;
	pmlmepriv = &(padapter->mlmepriv);

#ifdef CONFIG_AP_MODE
	if (check_fwstate(pmlmepriv, WIFI_AP_STATE) == true)
		expire_timeout_chk(padapter);
#endif

	rtw_hal_sreset_xmit_status_check(padapter);

	linked_status_chk(padapter);
	traffic_status_watchdog(padapter);

	rtw_hal_dm_watchdog(padapter);

#ifdef CONFIG_BT_COEXIST
	/*  BT-Coexist */
	BT_CoexistMechanism(padapter);
#endif
}

void lps_ctrl_wk_hdl(struct adapter *padapter, u8 lps_ctrl_type)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);
	u8	mstatus;

_func_enter_;

	if ((check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == true) ||
	    (check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == true))
		return;

	switch (lps_ctrl_type) {
	case LPS_CTRL_SCAN:
#ifdef CONFIG_BT_COEXIST
		BT_WifiScanNotify(padapter, true);
		if (BT_1Ant(padapter) == false)
#endif
		{
			if (check_fwstate(pmlmepriv, _FW_LINKED) == true) {
				/* connect */
				LPS_Leave(padapter);
			}
		}
		break;
	case LPS_CTRL_JOINBSS:
		LPS_Leave(padapter);
		break;
	case LPS_CTRL_CONNECT:
		mstatus = 1;/* connect */
		/*  Reset LPS Setting */
		padapter->pwrctrlpriv.LpsIdleCount = 0;
		rtw_hal_set_hwreg(padapter, HW_VAR_H2C_FW_JOINBSSRPT, (u8 *)(&mstatus));
#ifdef CONFIG_BT_COEXIST
		BT_WifiMediaStatusNotify(padapter, mstatus);
#endif
		break;
	case LPS_CTRL_DISCONNECT:
		mstatus = 0;/* disconnect */
#ifdef CONFIG_BT_COEXIST
		BT_WifiMediaStatusNotify(padapter, mstatus);
		if (BT_1Ant(padapter) == false)
#endif
		{
			LPS_Leave(padapter);
		}
		rtw_hal_set_hwreg(padapter, HW_VAR_H2C_FW_JOINBSSRPT, (u8 *)(&mstatus));
		break;
	case LPS_CTRL_SPECIAL_PACKET:
		/* DBG_88E("LPS_CTRL_SPECIAL_PACKET\n"); */
		pwrpriv->DelayLPSLastTimeStamp = rtw_get_current_time();
#ifdef CONFIG_BT_COEXIST
		BT_SpecialPacketNotify(padapter);
		if (BT_1Ant(padapter) == false)
#endif
		{
			LPS_Leave(padapter);
		}
		break;
	case LPS_CTRL_LEAVE:
#ifdef CONFIG_BT_COEXIST
		BT_LpsLeave(padapter);
		if (BT_1Ant(padapter) == false)
#endif
		{
			LPS_Leave(padapter);
		}
		break;
	default:
		break;
	}

_func_exit_;
}

u8 rtw_lps_ctrl_wk_cmd(struct adapter *padapter, u8 lps_ctrl_type, u8 enqueue)
{
	struct cmd_obj	*ph2c;
	struct drvextra_cmd_parm	*pdrvextra_cmd_parm;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;
	/* struct pwrctrl_priv *pwrctrlpriv = &padapter->pwrctrlpriv; */
	u8	res = _SUCCESS;

_func_enter_;

	/* if (!pwrctrlpriv->bLeisurePs) */
	/* 	return res; */

	if (enqueue) {
		ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
		if (ph2c == NULL) {
			res = _FAIL;
			goto exit;
		}

		pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
		if (pdrvextra_cmd_parm == NULL) {
			rtw_mfree((unsigned char *)ph2c, sizeof(struct cmd_obj));
			res = _FAIL;
			goto exit;
		}

		pdrvextra_cmd_parm->ec_id = LPS_CTRL_WK_CID;
		pdrvextra_cmd_parm->type_size = lps_ctrl_type;
		pdrvextra_cmd_parm->pbuf = NULL;

		init_h2fwcmd_w_parm_no_rsp(ph2c, pdrvextra_cmd_parm, GEN_CMD_CODE(_Set_Drv_Extra));

		res = rtw_enqueue_cmd(pcmdpriv, ph2c);
	} else {
		lps_ctrl_wk_hdl(padapter, lps_ctrl_type);
	}

exit:

_func_exit_;

	return res;
}

static void rpt_timer_setting_wk_hdl(struct adapter *padapter, u16 min_time)
{
	rtw_hal_set_hwreg(padapter, HW_VAR_RPT_TIMER_SETTING, (u8 *)(&min_time));
}

u8 rtw_rpt_timer_cfg_cmd(struct adapter *padapter, u16 min_time)
{
	struct cmd_obj		*ph2c;
	struct drvextra_cmd_parm	*pdrvextra_cmd_parm;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;

	u8	res = _SUCCESS;

_func_enter_;
	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((unsigned char *)ph2c, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = RTP_TIMER_CFG_WK_CID;
	pdrvextra_cmd_parm->type_size = min_time;
	pdrvextra_cmd_parm->pbuf = NULL;
	init_h2fwcmd_w_parm_no_rsp(ph2c, pdrvextra_cmd_parm, GEN_CMD_CODE(_Set_Drv_Extra));
	res = rtw_enqueue_cmd(pcmdpriv, ph2c);
exit:

_func_exit_;

	return res;
}

static void antenna_select_wk_hdl(struct adapter *padapter, u8 antenna)
{
	rtw_hal_set_hwreg(padapter, HW_VAR_ANTENNA_DIVERSITY_SELECT, (u8 *)(&antenna));
}

u8 rtw_antenna_select_cmd(struct adapter *padapter, u8 antenna, u8 enqueue)
{
	struct cmd_obj		*ph2c;
	struct drvextra_cmd_parm	*pdrvextra_cmd_parm;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;
	u8	support_ant_div;
	u8	res = _SUCCESS;

_func_enter_;
	rtw_hal_get_def_var(padapter, HAL_DEF_IS_SUPPORT_ANT_DIV, &support_ant_div);
	if (!support_ant_div)
		return res;

	if (enqueue) {
		ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
		if (ph2c == NULL) {
			res = _FAIL;
			goto exit;
		}

		pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
		if (pdrvextra_cmd_parm == NULL) {
			rtw_mfree((unsigned char *)ph2c, sizeof(struct cmd_obj));
			res = _FAIL;
			goto exit;
		}

		pdrvextra_cmd_parm->ec_id = ANT_SELECT_WK_CID;
		pdrvextra_cmd_parm->type_size = antenna;
		pdrvextra_cmd_parm->pbuf = NULL;
		init_h2fwcmd_w_parm_no_rsp(ph2c, pdrvextra_cmd_parm, GEN_CMD_CODE(_Set_Drv_Extra));

		res = rtw_enqueue_cmd(pcmdpriv, ph2c);
	} else {
		antenna_select_wk_hdl(padapter, antenna);
	}
exit:

_func_exit_;

	return res;
}

void power_saving_wk_hdl(struct adapter *padapter, u8 *pbuf, int sz)
{
	 rtw_ps_processor(padapter);
}

#ifdef CONFIG_P2P
u8 p2p_protocol_wk_cmd(struct adapter *padapter, int intCmdType)
{
	struct cmd_obj	*ph2c;
	struct drvextra_cmd_parm	*pdrvextra_cmd_parm;
	struct wifidirect_info	*pwdinfo = &(padapter->wdinfo);
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;

_func_enter_;

	if (rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
		return res;

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((unsigned char *)ph2c, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = P2P_PROTO_WK_CID;
	pdrvextra_cmd_parm->type_size = intCmdType;	/* 	As the command tppe. */
	pdrvextra_cmd_parm->pbuf = NULL;		/* 	Must be NULL here */

	init_h2fwcmd_w_parm_no_rsp(ph2c, pdrvextra_cmd_parm, GEN_CMD_CODE(_Set_Drv_Extra));

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);

exit:

_func_exit_;

	return res;
}
#endif /* CONFIG_P2P */

u8 rtw_ps_cmd(struct adapter *padapter)
{
	struct cmd_obj		*ppscmd;
	struct drvextra_cmd_parm	*pdrvextra_cmd_parm;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;

	u8	res = _SUCCESS;
_func_enter_;

	ppscmd = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ppscmd == NULL) {
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((unsigned char *)ppscmd, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = POWER_SAVING_CTRL_WK_CID;
	pdrvextra_cmd_parm->pbuf = NULL;
	init_h2fwcmd_w_parm_no_rsp(ppscmd, pdrvextra_cmd_parm, GEN_CMD_CODE(_Set_Drv_Extra));

	res = rtw_enqueue_cmd(pcmdpriv, ppscmd);

exit:

_func_exit_;

	return res;
}

#ifdef CONFIG_AP_MODE

static void rtw_chk_hi_queue_hdl(struct adapter *padapter)
{
	int cnt = 0;
	struct sta_info *psta_bmc;
	struct sta_priv *pstapriv = &padapter->stapriv;

	psta_bmc = rtw_get_bcmc_stainfo(padapter);
	if (!psta_bmc)
		return;

	if (psta_bmc->sleepq_len == 0) {
		u8 val = 0;

		/* while ((rtw_read32(padapter, 0x414)&0x00ffff00)!= 0) */
		/* while ((rtw_read32(padapter, 0x414)&0x0000ff00)!= 0) */

		rtw_hal_get_hwreg(padapter, HW_VAR_CHK_HI_QUEUE_EMPTY, &val);

		while (!val) {
			rtw_msleep_os(100);

			cnt++;

			if (cnt > 10)
				break;

			rtw_hal_get_hwreg(padapter, HW_VAR_CHK_HI_QUEUE_EMPTY, &val);
		}

		if (cnt <= 10) {
			pstapriv->tim_bitmap &= ~BIT(0);
			pstapriv->sta_dz_bitmap &= ~BIT(0);

			update_beacon(padapter, _TIM_IE_, NULL, false);
		} else { /* re check again */
			rtw_chk_hi_queue_cmd(padapter);
		}
	}
}

u8 rtw_chk_hi_queue_cmd(struct adapter *padapter)
{
	struct cmd_obj	*ph2c;
	struct drvextra_cmd_parm	*pdrvextra_cmd_parm;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((unsigned char *)ph2c, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = CHECK_HIQ_WK_CID;
	pdrvextra_cmd_parm->type_size = 0;
	pdrvextra_cmd_parm->pbuf = NULL;

	init_h2fwcmd_w_parm_no_rsp(ph2c, pdrvextra_cmd_parm, GEN_CMD_CODE(_Set_Drv_Extra));

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);
exit:
	return res;
}
#endif

u8 rtw_c2h_wk_cmd(struct adapter *padapter, u8 *c2h_evt)
{
	struct cmd_obj *ph2c;
	struct drvextra_cmd_parm *pdrvextra_cmd_parm;
	struct cmd_priv	*pcmdpriv = &padapter->cmdpriv;
	u8	res = _SUCCESS;

	ph2c = (struct cmd_obj *)rtw_zmalloc(sizeof(struct cmd_obj));
	if (ph2c == NULL) {
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm = (struct drvextra_cmd_parm *)rtw_zmalloc(sizeof(struct drvextra_cmd_parm));
	if (pdrvextra_cmd_parm == NULL) {
		rtw_mfree((u8 *)ph2c, sizeof(struct cmd_obj));
		res = _FAIL;
		goto exit;
	}

	pdrvextra_cmd_parm->ec_id = C2H_WK_CID;
	pdrvextra_cmd_parm->type_size = c2h_evt ? 16 : 0;
	pdrvextra_cmd_parm->pbuf = c2h_evt;

	init_h2fwcmd_w_parm_no_rsp(ph2c, pdrvextra_cmd_parm, GEN_CMD_CODE(_Set_Drv_Extra));

	res = rtw_enqueue_cmd(pcmdpriv, ph2c);

exit:

	return res;
}

static s32 c2h_evt_hdl(struct adapter *adapter, struct c2h_evt_hdr *c2h_evt, c2h_id_filter filter)
{
	s32 ret = _FAIL;
	u8 buf[16];

	if (!c2h_evt) {
		/* No c2h event in cmd_obj, read c2h event before handling*/
		if (c2h_evt_read(adapter, buf) == _SUCCESS) {
			c2h_evt = (struct c2h_evt_hdr *)buf;

			if (filter && filter(c2h_evt->id) == false)
				goto exit;

			ret = rtw_hal_c2h_handler(adapter, c2h_evt);
		}
	} else {
		if (filter && filter(c2h_evt->id) == false)
			goto exit;

		ret = rtw_hal_c2h_handler(adapter, c2h_evt);
	}
exit:
	return ret;
}

static void c2h_wk_callback(struct work_struct *work)
{
	struct evt_priv *evtpriv = container_of(work, struct evt_priv, c2h_wk);
	struct adapter *adapter = container_of(evtpriv, struct adapter, evtpriv);
	struct c2h_evt_hdr *c2h_evt;
	c2h_id_filter ccx_id_filter = rtw_hal_c2h_id_filter_ccx(adapter);

	evtpriv->c2h_wk_alive = true;

	while (!rtw_cbuf_empty(evtpriv->c2h_queue)) {
		if ((c2h_evt = (struct c2h_evt_hdr *)rtw_cbuf_pop(evtpriv->c2h_queue)) != NULL) {
			/* This C2H event is read, clear it */
			c2h_evt_clear(adapter);
		} else if ((c2h_evt = (struct c2h_evt_hdr *)rtw_malloc(16)) != NULL) {
			/* This C2H event is not read, read & clear now */
			if (c2h_evt_read(adapter, (u8 *)c2h_evt) != _SUCCESS)
				continue;
		}

		/* Special pointer to trigger c2h_evt_clear only */
		if ((void *)c2h_evt == (void *)evtpriv)
			continue;

		if (!c2h_evt_exist(c2h_evt)) {
			rtw_mfree((u8 *)c2h_evt, 16);
			continue;
		}

		if (ccx_id_filter(c2h_evt->id) == true) {
			/* Handle CCX report here */
			rtw_hal_c2h_handler(adapter, c2h_evt);
			rtw_mfree((u8 *)c2h_evt, 16);
		} else {
			/* Enqueue into cmd_thread for others */
			rtw_c2h_wk_cmd(adapter, (u8 *)c2h_evt);
		}
	}

	evtpriv->c2h_wk_alive = false;
}

u8 rtw_drvextra_cmd_hdl(struct adapter *padapter, unsigned char *pbuf)
{
	struct drvextra_cmd_parm *pdrvextra_cmd;

	if (!pbuf)
		return H2C_PARAMETERS_ERROR;

	pdrvextra_cmd = (struct drvextra_cmd_parm *)pbuf;

	switch (pdrvextra_cmd->ec_id) {
	case DYNAMIC_CHK_WK_CID:
		dynamic_chk_wk_hdl(padapter, pdrvextra_cmd->pbuf, pdrvextra_cmd->type_size);
		break;
	case POWER_SAVING_CTRL_WK_CID:
		power_saving_wk_hdl(padapter, pdrvextra_cmd->pbuf, pdrvextra_cmd->type_size);
		break;
	case LPS_CTRL_WK_CID:
		lps_ctrl_wk_hdl(padapter, (u8)pdrvextra_cmd->type_size);
		break;
	case RTP_TIMER_CFG_WK_CID:
		rpt_timer_setting_wk_hdl(padapter, pdrvextra_cmd->type_size);
		break;
	case ANT_SELECT_WK_CID:
		antenna_select_wk_hdl(padapter, pdrvextra_cmd->type_size);
		break;
	case P2P_PS_WK_CID:
		p2p_ps_wk_hdl(padapter, pdrvextra_cmd->type_size);
		break;
	case P2P_PROTO_WK_CID:
		/* 	Commented by Albert 2011/07/01 */
		/* 	I used the type_size as the type command */
		p2p_protocol_wk_hdl(padapter, pdrvextra_cmd->type_size);
		break;
#ifdef CONFIG_AP_MODE
	case CHECK_HIQ_WK_CID:
		rtw_chk_hi_queue_hdl(padapter);
		break;
#endif /* CONFIG_AP_MODE */
	case C2H_WK_CID:
		c2h_evt_hdl(padapter, (struct c2h_evt_hdr *)pdrvextra_cmd->pbuf, NULL);
		break;
	default:
		break;
	}

	if (pdrvextra_cmd->pbuf && pdrvextra_cmd->type_size > 0)
		rtw_mfree(pdrvextra_cmd->pbuf, pdrvextra_cmd->type_size);

	return H2C_SUCCESS;
}

void rtw_survey_cmd_callback(struct adapter *padapter,  struct cmd_obj *pcmd)
{
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;

_func_enter_;

	if (pcmd->res == H2C_DROPPED) {
		/* TODO: cancel timer and do timeout handler directly... */
		/* need to make timeout handlerOS independent */
		_set_timer(&pmlmepriv->scan_to_timer, 1);
		} else if (pcmd->res != H2C_SUCCESS) {
		_set_timer(&pmlmepriv->scan_to_timer, 1);
		RT_TRACE(_module_rtl871x_cmd_c_, _drv_err_, ("\n ********Error: MgntActrtw_set_802_11_bssid_LIST_SCAN Fail ************\n\n."));
	}

	/*  free cmd */
	rtw_free_cmd_obj(pcmd);

_func_exit_;
}
void rtw_disassoc_cmd_callback(struct adapter *padapter, struct cmd_obj *pcmd)
{
	unsigned long	irqL;
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;

_func_enter_;

	if (pcmd->res != H2C_SUCCESS) {
		_enter_critical_bh(&pmlmepriv->lock, &irqL);
		set_fwstate(pmlmepriv, _FW_LINKED);
		_exit_critical_bh(&pmlmepriv->lock, &irqL);

		RT_TRACE(_module_rtl871x_cmd_c_, _drv_err_, ("\n ***Error: disconnect_cmd_callback Fail ***\n."));

		goto exit;
	} else /* clear bridge database */
		nat25_db_cleanup(padapter);

	/*  free cmd */
	rtw_free_cmd_obj(pcmd);

exit:

_func_exit_;
}


void rtw_joinbss_cmd_callback(struct adapter *padapter,  struct cmd_obj *pcmd)
{
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;

_func_enter_;

	if (pcmd->res == H2C_DROPPED) {
		/* TODO: cancel timer and do timeout handler directly... */
		/* need to make timeout handlerOS independent */
		_set_timer(&pmlmepriv->assoc_timer, 1);
	} else if (pcmd->res != H2C_SUCCESS) {
		RT_TRACE(_module_rtl871x_cmd_c_, _drv_err_, ("********Error:rtw_select_and_join_from_scanned_queue Wait Sema  Fail ************\n"));
		_set_timer(&pmlmepriv->assoc_timer, 1);
	}

	rtw_free_cmd_obj(pcmd);

_func_exit_;
}

void rtw_createbss_cmd_callback(struct adapter *padapter, struct cmd_obj *pcmd)
{
	unsigned long irqL;
	u8 timer_cancelled;
	struct sta_info *psta = NULL;
	struct wlan_network *pwlan = NULL;
	struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct wlan_bssid_ex *pnetwork = (struct wlan_bssid_ex *)pcmd->parmbuf;
	struct wlan_network *tgt_network = &(pmlmepriv->cur_network);

_func_enter_;

	if ((pcmd->res != H2C_SUCCESS)) {
		RT_TRACE(_module_rtl871x_cmd_c_, _drv_err_, ("\n ********Error: rtw_createbss_cmd_callback  Fail ************\n\n."));
		_set_timer(&pmlmepriv->assoc_timer, 1);
	}

	_cancel_timer(&pmlmepriv->assoc_timer, &timer_cancelled);

	_enter_critical_bh(&pmlmepriv->lock, &irqL);

	if (check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
		psta = rtw_get_stainfo(&padapter->stapriv, pnetwork->MacAddress);
		if (!psta) {
			psta = rtw_alloc_stainfo(&padapter->stapriv, pnetwork->MacAddress);
			if (psta == NULL) {
				RT_TRACE(_module_rtl871x_cmd_c_, _drv_err_, ("\nCan't alloc sta_info when createbss_cmd_callback\n"));
				goto createbss_cmd_fail ;
			}
		}

		rtw_indicate_connect(padapter);
	} else {
		unsigned long	irqL;

		pwlan = _rtw_alloc_network(pmlmepriv);
		_enter_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);
		if (pwlan == NULL) {
			pwlan = rtw_get_oldest_wlan_network(&pmlmepriv->scanned_queue);
			if (pwlan == NULL) {
				RT_TRACE(_module_rtl871x_cmd_c_, _drv_err_, ("\n Error:  can't get pwlan in rtw_joinbss_event_callback\n"));
				_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);
				goto createbss_cmd_fail;
			}
			pwlan->last_scanned = rtw_get_current_time();
		} else {
			rtw_list_insert_tail(&(pwlan->list), &pmlmepriv->scanned_queue.queue);
		}

		pnetwork->Length = get_wlan_bssid_ex_sz(pnetwork);
		_rtw_memcpy(&(pwlan->network), pnetwork, pnetwork->Length);

		_rtw_memcpy(&tgt_network->network, pnetwork, (get_wlan_bssid_ex_sz(pnetwork)));

		_clr_fwstate_(pmlmepriv, _FW_UNDER_LINKING);

		_exit_critical_bh(&(pmlmepriv->scanned_queue.lock), &irqL);
		/*  we will set _FW_LINKED when there is one more sat to join us (rtw_stassoc_event_callback) */
	}

createbss_cmd_fail:

	_exit_critical_bh(&pmlmepriv->lock, &irqL);

	rtw_free_cmd_obj(pcmd);

_func_exit_;
}

void rtw_setstaKey_cmdrsp_callback(struct adapter *padapter,  struct cmd_obj *pcmd)
{
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct set_stakey_rsp *psetstakey_rsp = (struct set_stakey_rsp *)(pcmd->rsp);
	struct sta_info *psta = rtw_get_stainfo(pstapriv, psetstakey_rsp->addr);

_func_enter_;

	if (psta == NULL) {
		RT_TRACE(_module_rtl871x_cmd_c_, _drv_err_, ("\nERROR: rtw_setstaKey_cmdrsp_callback => can't get sta_info\n\n"));
		goto exit;
	}
exit:
	rtw_free_cmd_obj(pcmd);
_func_exit_;
}

void rtw_setassocsta_cmdrsp_callback(struct adapter *padapter,  struct cmd_obj *pcmd)
{
	unsigned long	irqL;
	struct sta_priv *pstapriv = &padapter->stapriv;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct set_assocsta_parm *passocsta_parm = (struct set_assocsta_parm *)(pcmd->parmbuf);
	struct set_assocsta_rsp *passocsta_rsp = (struct set_assocsta_rsp *)(pcmd->rsp);
	struct sta_info *psta = rtw_get_stainfo(pstapriv, passocsta_parm->addr);

_func_enter_;

	if (psta == NULL) {
		RT_TRACE(_module_rtl871x_cmd_c_, _drv_err_, ("\nERROR: setassocsta_cmdrsp_callbac => can't get sta_info\n\n"));
		goto exit;
	}

	psta->aid = passocsta_rsp->cam_id;
	psta->mac_id = passocsta_rsp->cam_id;

	_enter_critical_bh(&pmlmepriv->lock, &irqL);

	if ((check_fwstate(pmlmepriv, WIFI_MP_STATE) == true) && (check_fwstate(pmlmepriv, _FW_UNDER_LINKING) == true))
		_clr_fwstate_(pmlmepriv, _FW_UNDER_LINKING);

	set_fwstate(pmlmepriv, _FW_LINKED);
	_exit_critical_bh(&pmlmepriv->lock, &irqL);

exit:
	rtw_free_cmd_obj(pcmd);

_func_exit_;
}

void rtw_getrttbl_cmd_cmdrsp_callback(struct adapter *padapter,  struct cmd_obj *pcmd)
{
_func_enter_;

	rtw_free_cmd_obj(pcmd);
	if (padapter->registrypriv.mp_mode == 1)
		padapter->mppriv.workparam.bcompleted = true;

_func_exit_;
}
