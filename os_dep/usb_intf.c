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
#define _HCI_INTF_C_

#include <osdep_service.h>
#include <drv_types.h>
#include <recv_osdep.h>
#include <xmit_osdep.h>
#include <hal_intf.h>
#include <rtw_version.h>
#include <linux/usb.h>
#include <osdep_intf.h>

#include <usb_vendor_req.h>
#include <usb_ops.h>
#include <usb_osintf.h>
#include <usb_hal.h>
#include <rtw_ioctl.h>

int ui_pid[3] = {0, 0, 0};

static int rtw_suspend(struct usb_interface *intf, pm_message_t message);
static int rtw_resume(struct usb_interface *intf);


static int rtw_drv_init(struct usb_interface *pusb_intf, const struct usb_device_id *pdid);
static void rtw_dev_remove(struct usb_interface *pusb_intf);


#define USB_VENDER_ID_REALTEK		0x0bda

/* DID_USB_v916_20130116 */
static struct usb_device_id rtw_usb_id_tbl[] = {
	/*=== Realtek demoboard ===*/
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8179)}, /* 8188EUS */
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x0179)}, /* 8188ETV */
	/*=== Customer ID ===*/
	/****** 8188EUS ********/
	{USB_DEVICE(0x07B8, 0x8179)}, /* Abocom - Abocom */
	{USB_DEVICE(0x2001, 0x330F)}, /* DLink DWA-125 REV D1 */
        {USB_DEVICE(0x2001, 0x3310)}, /* Dlink DWA-123 REV D1 */
	{}	/* Terminating entry */
};

MODULE_DEVICE_TABLE(usb, rtw_usb_id_tbl);

static struct specific_device_id specific_device_id_tbl[] = {
	{}		/* empty table for now */
};

struct rtw_usb_drv {
	struct usb_driver usbdrv;
	int drv_registered;
	struct mutex hw_init_mutex;
};

static struct rtw_usb_drv rtl8188e_usb_drv = {
	.usbdrv.name = (char *)"r8188eu",
	.usbdrv.probe = rtw_drv_init,
	.usbdrv.disconnect = rtw_dev_remove,
	.usbdrv.id_table = rtw_usb_id_tbl,
	.usbdrv.suspend =  rtw_suspend,
	.usbdrv.resume = rtw_resume,
	.usbdrv.reset_resume   = rtw_resume,
};

static struct rtw_usb_drv *usb_drv = &rtl8188e_usb_drv;

static inline int RT_usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd)
{
	return (epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN;
}

static inline int RT_usb_endpoint_dir_out(const struct usb_endpoint_descriptor *epd)
{
	return (epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT;
}

static inline int RT_usb_endpoint_xfer_int(const struct usb_endpoint_descriptor *epd)
{
	return (epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT;
}

static inline int RT_usb_endpoint_xfer_bulk(const struct usb_endpoint_descriptor *epd)
{
	return (epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK;
}

static inline int RT_usb_endpoint_is_bulk_in(const struct usb_endpoint_descriptor *epd)
{
	return RT_usb_endpoint_xfer_bulk(epd) && RT_usb_endpoint_dir_in(epd);
}

static inline int RT_usb_endpoint_is_bulk_out(const struct usb_endpoint_descriptor *epd)
{
	return RT_usb_endpoint_xfer_bulk(epd) && RT_usb_endpoint_dir_out(epd);
}

static inline int usb_endpoint_is_int(const struct usb_endpoint_descriptor *epd)
{
	return RT_usb_endpoint_xfer_int(epd) && RT_usb_endpoint_dir_in(epd);
}

static inline int RT_usb_endpoint_num(const struct usb_endpoint_descriptor *epd)
{
	return epd->bEndpointAddress & USB_ENDPOINT_NUMBER_MASK;
}

static u8 rtw_init_intf_priv(struct dvobj_priv *dvobj)
{
	u8 rst = _SUCCESS;

	_rtw_mutex_init(&dvobj->usb_vendor_req_mutex);

	dvobj->usb_alloc_vendor_req_buf = rtw_zmalloc(MAX_USB_IO_CTL_SIZE);
	if (dvobj->usb_alloc_vendor_req_buf == NULL) {
		DBG_88E("alloc usb_vendor_req_buf failed... /n");
		rst = _FAIL;
		goto exit;
	}
	dvobj->usb_vendor_req_buf = (u8 *)N_BYTE_ALIGMENT((size_t)(dvobj->usb_alloc_vendor_req_buf), ALIGNMENT_UNIT);
exit:
	return rst;
}

static u8 rtw_deinit_intf_priv(struct dvobj_priv *dvobj)
{
	u8 rst = _SUCCESS;

	kfree(dvobj->usb_alloc_vendor_req_buf);
	_rtw_mutex_free(&dvobj->usb_vendor_req_mutex);
	return rst;
}

static struct dvobj_priv *usb_dvobj_init(struct usb_interface *usb_intf)
{
	int	i;
	int	status = _FAIL;
	struct dvobj_priv *pdvobjpriv;
	struct usb_host_config		*phost_conf;
	struct usb_config_descriptor	*pconf_desc;
	struct usb_host_interface	*phost_iface;
	struct usb_interface_descriptor	*piface_desc;
	struct usb_host_endpoint	*phost_endp;
	struct usb_endpoint_descriptor	*pendp_desc;
	struct usb_device	*pusbd;

_func_enter_;

	pdvobjpriv = (struct dvobj_priv *)rtw_zmalloc(sizeof(*pdvobjpriv));
	if (pdvobjpriv == NULL)
		goto exit;

	pdvobjpriv->pusbintf = usb_intf;
	pusbd = interface_to_usbdev(usb_intf);
	pdvobjpriv->pusbdev = pusbd;
	usb_set_intfdata(usb_intf, pdvobjpriv);

	pdvobjpriv->RtNumInPipes = 0;
	pdvobjpriv->RtNumOutPipes = 0;

	phost_conf = pusbd->actconfig;
	pconf_desc = &phost_conf->desc;

	phost_iface = &usb_intf->altsetting[0];
	piface_desc = &phost_iface->desc;

	pdvobjpriv->NumInterfaces = pconf_desc->bNumInterfaces;
	pdvobjpriv->InterfaceNumber = piface_desc->bInterfaceNumber;
	pdvobjpriv->nr_endpoint = piface_desc->bNumEndpoints;

	for (i = 0; i < pdvobjpriv->nr_endpoint; i++) {
		phost_endp = phost_iface->endpoint + i;
		if (phost_endp) {
			pendp_desc = &phost_endp->desc;

			DBG_88E("\nusb_endpoint_descriptor(%d):\n", i);
			DBG_88E("bLength=%x\n", pendp_desc->bLength);
			DBG_88E("bDescriptorType=%x\n",
				pendp_desc->bDescriptorType);
			DBG_88E("bEndpointAddress=%x\n",
				pendp_desc->bEndpointAddress);
			DBG_88E("wMaxPacketSize=%d\n",
				le16_to_cpu(pendp_desc->wMaxPacketSize));
			DBG_88E("bInterval=%x\n", pendp_desc->bInterval);

			if (RT_usb_endpoint_is_bulk_in(pendp_desc)) {
				DBG_88E("RT_usb_endpoint_is_bulk_in = %x\n",
					RT_usb_endpoint_num(pendp_desc));
				pdvobjpriv->RtInPipe[pdvobjpriv->RtNumInPipes] = RT_usb_endpoint_num(pendp_desc);
				pdvobjpriv->RtNumInPipes++;
			} else if (usb_endpoint_is_int(pendp_desc)) {
				DBG_88E("usb_endpoint_is_int = %x, Interval = %x\n",
					RT_usb_endpoint_num(pendp_desc),
					pendp_desc->bInterval);
				pdvobjpriv->RtInPipe[pdvobjpriv->RtNumInPipes] = RT_usb_endpoint_num(pendp_desc);
				pdvobjpriv->RtNumInPipes++;
			} else if (RT_usb_endpoint_is_bulk_out(pendp_desc)) {
				DBG_88E("RT_usb_endpoint_is_bulk_out = %x\n",
					RT_usb_endpoint_num(pendp_desc));
				pdvobjpriv->RtOutPipe[pdvobjpriv->RtNumOutPipes] = RT_usb_endpoint_num(pendp_desc);
				pdvobjpriv->RtNumOutPipes++;
			}
			pdvobjpriv->ep_num[i] = RT_usb_endpoint_num(pendp_desc);
		}
	}

	DBG_88E("nr_endpoint=%d, in_num=%d, out_num=%d\n\n",
		pdvobjpriv->nr_endpoint, pdvobjpriv->RtNumInPipes,
		pdvobjpriv->RtNumOutPipes);

	if (pusbd->speed == USB_SPEED_HIGH) {
		pdvobjpriv->ishighspeed = true;
		DBG_88E("USB_SPEED_HIGH\n");
	} else {
		pdvobjpriv->ishighspeed = false;
		DBG_88E("NON USB_SPEED_HIGH\n");
	}

	if (rtw_init_intf_priv(pdvobjpriv) == _FAIL) {
		RT_TRACE(_module_os_intfs_c_, _drv_err_,
			 ("\n Can't INIT rtw_init_intf_priv\n"));
		goto free_dvobj;
	}

	/* 3 misc */
	_rtw_init_sema(&(pdvobjpriv->usb_suspend_sema), 0);
	rtw_reset_continual_urb_error(pdvobjpriv);

	usb_get_dev(pusbd);

	status = _SUCCESS;

free_dvobj:
	if (status != _SUCCESS && pdvobjpriv) {
		usb_set_intfdata(usb_intf, NULL);
		kfree(pdvobjpriv);
		pdvobjpriv = NULL;
	}
exit:
_func_exit_;
	return pdvobjpriv;
}

static void usb_dvobj_deinit(struct usb_interface *usb_intf)
{
	struct dvobj_priv *dvobj = usb_get_intfdata(usb_intf);

_func_enter_;

	usb_set_intfdata(usb_intf, NULL);
	if (dvobj) {
		/* Modify condition for 92DU DMDP 2010.11.18, by Thomas */
		if ((dvobj->NumInterfaces != 2 &&
		    dvobj->NumInterfaces != 3) ||
		    (dvobj->InterfaceNumber == 1)) {
			if (interface_to_usbdev(usb_intf)->state !=
			    USB_STATE_NOTATTACHED) {
				/* If we didn't unplug usb dongle and
				 * remove/insert module, driver fails
				 * on sitesurvey for the first time when
				 * device is up . Reset usb port for sitesurvey
				 * fail issue. */
				DBG_88E("usb attached..., try to reset usb device\n");
				usb_reset_device(interface_to_usbdev(usb_intf));
			}
		}
		rtw_deinit_intf_priv(dvobj);
		kfree(dvobj);
	}

	usb_put_dev(interface_to_usbdev(usb_intf));

_func_exit_;
}

static void chip_by_usb_id(struct adapter *padapter,
			   const struct usb_device_id *pdid)
{
	padapter->chip_type = NULL_CHIP_TYPE;
	hal_set_hw_type(padapter);
}

static void usb_intf_start(struct adapter *padapter)
{
	RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("+usb_intf_start\n"));

	rtw_hal_inirp_init(padapter);

	RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("-usb_intf_start\n"));
}

static void usb_intf_stop(struct adapter *padapter)
{
	RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("+usb_intf_stop\n"));

	/* disabel_hw_interrupt */
	if (!padapter->bSurpriseRemoved) {
		/* device still exists, so driver can do i/o operation */
		/* TODO: */
		RT_TRACE(_module_hci_intfs_c_, _drv_err_,
			 ("SurpriseRemoved == false\n"));
	}

	/* cancel in irp */
	rtw_hal_inirp_deinit(padapter);

	/* cancel out irp */
	rtw_write_port_cancel(padapter);

	/* todo:cancel other irps */

	RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("-usb_intf_stop\n"));
}

static void rtw_dev_unload(struct adapter *padapter)
{
	RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("+rtw_dev_unload\n"));

	if (padapter->bup) {
		DBG_88E("===> rtw_dev_unload\n");
		padapter->bDriverStopped = true;
		if (padapter->xmitpriv.ack_tx)
			rtw_ack_tx_done(&padapter->xmitpriv, RTW_SCTX_DONE_DRV_STOP);
		/* s3. */
		if (padapter->intf_stop)
			padapter->intf_stop(padapter);
		/* s4. */
		if (!padapter->pwrctrlpriv.bInternalAutoSuspend)
			rtw_stop_drv_threads(padapter);

		/* s5. */
		if (!padapter->bSurpriseRemoved) {
			rtw_hal_deinit(padapter);
			padapter->bSurpriseRemoved = true;
		}

		padapter->bup = false;
	} else {
		RT_TRACE(_module_hci_intfs_c_, _drv_err_,
			 ("r871x_dev_unload():padapter->bup == false\n"));
	}

	DBG_88E("<=== rtw_dev_unload\n");

	RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("-rtw_dev_unload\n"));
}

static void process_spec_devid(const struct usb_device_id *pdid)
{
	u16 vid, pid;
	u32 flags;
	int i;
	int num = sizeof(specific_device_id_tbl) /
		  sizeof(struct specific_device_id);

	for (i = 0; i < num; i++) {
		vid = specific_device_id_tbl[i].idVendor;
		pid = specific_device_id_tbl[i].idProduct;
		flags = specific_device_id_tbl[i].flags;

		if ((pdid->idVendor == vid) && (pdid->idProduct == pid) &&
		    (flags&SPEC_DEV_ID_DISABLE_HT)) {
			rtw_ht_enable = 0;
			rtw_cbw40_enable = 0;
			rtw_ampdu_enable = 0;
		}
	}
}

int rtw_hw_suspend(struct adapter *padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct net_device *pnetdev = padapter->pnetdev;

	_func_enter_;

	if ((!padapter->bup) || (padapter->bDriverStopped) ||
	    (padapter->bSurpriseRemoved)) {
		DBG_88E("padapter->bup=%d bDriverStopped=%d bSurpriseRemoved = %d\n",
			padapter->bup, padapter->bDriverStopped,
			padapter->bSurpriseRemoved);
		goto error_exit;
	}

	if (padapter) { /* system suspend */
		LeaveAllPowerSaveMode(padapter);

		DBG_88E("==> rtw_hw_suspend\n");
		_enter_pwrlock(&pwrpriv->lock);
		pwrpriv->bips_processing = true;
		/* s1. */
		if (pnetdev) {
			netif_carrier_off(pnetdev);
			rtw_netif_stop_queue(pnetdev);
		}

		/* s2. */
		rtw_disassoc_cmd(padapter, 500, false);

		/* s2-2.  indicate disconnect to os */
		{
			struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;

			if (check_fwstate(pmlmepriv, _FW_LINKED)) {
				_clr_fwstate_(pmlmepriv, _FW_LINKED);

				rtw_led_control(padapter, LED_CTL_NO_LINK);

				rtw_os_indicate_disconnect(padapter);

				/* donnot enqueue cmd */
				rtw_lps_ctrl_wk_cmd(padapter, LPS_CTRL_DISCONNECT, 0);
			}
		}
		/* s2-3. */
		rtw_free_assoc_resources(padapter, 1);

		/* s2-4. */
		rtw_free_network_queue(padapter, true);
		rtw_ips_dev_unload(padapter);
		pwrpriv->rf_pwrstate = rf_off;
		pwrpriv->bips_processing = false;

		_exit_pwrlock(&pwrpriv->lock);
	} else {
		goto error_exit;
	}
	_func_exit_;
	return 0;

error_exit:
	DBG_88E("%s, failed\n", __func__);
	return -1;
}

int rtw_hw_resume(struct adapter *padapter)
{
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	struct net_device *pnetdev = padapter->pnetdev;

	_func_enter_;

	if (padapter) { /* system resume */
		DBG_88E("==> rtw_hw_resume\n");
		_enter_pwrlock(&pwrpriv->lock);
		pwrpriv->bips_processing = true;
		rtw_reset_drv_sw(padapter);

		if (pm_netdev_open(pnetdev, false) != 0) {
			_exit_pwrlock(&pwrpriv->lock);
			goto error_exit;
		}

		netif_device_attach(pnetdev);
		netif_carrier_on(pnetdev);

		if (!netif_queue_stopped(pnetdev))
			netif_start_queue(pnetdev);
		else
			netif_wake_queue(pnetdev);

		pwrpriv->bkeepfwalive = false;
		pwrpriv->brfoffbyhw = false;

		pwrpriv->rf_pwrstate = rf_on;
		pwrpriv->bips_processing = false;

		_exit_pwrlock(&pwrpriv->lock);
	} else {
		goto error_exit;
	}

	_func_exit_;

	return 0;
error_exit:
	DBG_88E("%s, Open net dev failed\n", __func__);
	return -1;
}

static int rtw_suspend(struct usb_interface *pusb_intf, pm_message_t message)
{
	struct dvobj_priv *dvobj = usb_get_intfdata(pusb_intf);
	struct adapter *padapter = dvobj->if1;
	struct net_device *pnetdev = padapter->pnetdev;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;

	int ret = 0;
	u32 start_time = rtw_get_current_time();

	_func_enter_;

	DBG_88E("==> %s (%s:%d)\n", __func__, current->comm, current->pid);

	if ((!padapter->bup) || (padapter->bDriverStopped) ||
	    (padapter->bSurpriseRemoved)) {
		DBG_88E("padapter->bup=%d bDriverStopped=%d bSurpriseRemoved = %d\n",
			padapter->bup, padapter->bDriverStopped,
			padapter->bSurpriseRemoved);
		goto exit;
	}

	pwrpriv->bInSuspend = true;
	rtw_cancel_all_timer(padapter);
	LeaveAllPowerSaveMode(padapter);

	_enter_pwrlock(&pwrpriv->lock);
	/* s1. */
	if (pnetdev) {
		netif_carrier_off(pnetdev);
		rtw_netif_stop_queue(pnetdev);
	}

	/* s2. */
	rtw_disassoc_cmd(padapter, 0, false);

	if (check_fwstate(pmlmepriv, WIFI_STATION_STATE) &&
	    check_fwstate(pmlmepriv, _FW_LINKED)) {
		DBG_88E("%s:%d %s(%pM), length:%d assoc_ssid.length:%d\n",
			__func__, __LINE__,
			pmlmepriv->cur_network.network.Ssid.Ssid,
			pmlmepriv->cur_network.network.MacAddress,
			pmlmepriv->cur_network.network.Ssid.SsidLength,
			pmlmepriv->assoc_ssid.SsidLength);

		pmlmepriv->to_roaming = 1;
	}
	/* s2-2.  indicate disconnect to os */
	rtw_indicate_disconnect(padapter);
	/* s2-3. */
	rtw_free_assoc_resources(padapter, 1);
	/* s2-4. */
	rtw_free_network_queue(padapter, true);

	rtw_dev_unload(padapter);
	_exit_pwrlock(&pwrpriv->lock);

	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY))
		rtw_indicate_scan_done(padapter, 1);

	if (check_fwstate(pmlmepriv, _FW_UNDER_LINKING))
		rtw_indicate_disconnect(padapter);

exit:
	DBG_88E("<===  %s return %d.............. in %dms\n", __func__
		, ret, rtw_get_passing_time_ms(start_time));

	_func_exit_;
	return ret;
}

static int rtw_resume(struct usb_interface *pusb_intf)
{
	struct dvobj_priv *dvobj = usb_get_intfdata(pusb_intf);
	struct adapter *padapter = dvobj->if1;
	struct pwrctrl_priv *pwrpriv = &padapter->pwrctrlpriv;
	 int ret = 0;

	if (pwrpriv->bInternalAutoSuspend)
		ret = rtw_resume_process(padapter);
	else
		ret = rtw_resume_process(padapter);
	return ret;
}

int rtw_resume_process(struct adapter *padapter)
{
	struct net_device *pnetdev;
	struct pwrctrl_priv *pwrpriv = NULL;
	int ret = -1;
	u32 start_time = rtw_get_current_time();
	_func_enter_;

	DBG_88E("==> %s (%s:%d)\n", __func__, current->comm, current->pid);

	if (padapter) {
		pnetdev = padapter->pnetdev;
		pwrpriv = &padapter->pwrctrlpriv;
	} else {
		goto exit;
	}

	_enter_pwrlock(&pwrpriv->lock);
	rtw_reset_drv_sw(padapter);
	if (pwrpriv)
		pwrpriv->bkeepfwalive = false;

	DBG_88E("bkeepfwalive(%x)\n", pwrpriv->bkeepfwalive);
	if (pm_netdev_open(pnetdev, true) != 0)
		goto exit;

	netif_device_attach(pnetdev);
	netif_carrier_on(pnetdev);

	_exit_pwrlock(&pwrpriv->lock);

	if (padapter->pid[1] != 0) {
		DBG_88E("pid[1]:%d\n", padapter->pid[1]);
		rtw_signal_process(padapter->pid[1], SIGUSR2);
	}

	rtw_roaming(padapter, NULL);

	ret = 0;
exit:
	if (pwrpriv)
		pwrpriv->bInSuspend = false;
	DBG_88E("<===  %s return %d.............. in %dms\n", __func__,
		ret, rtw_get_passing_time_ms(start_time));

	_func_exit_;

	return ret;
}

/*
 * drv_init() - a device potentially for us
 *
 * notes: drv_init() is called when the bus driver has located
 * a card for us to support.
 *        We accept the new device by returning 0.
 */

static struct adapter *rtw_usb_if1_init(struct dvobj_priv *dvobj,
	struct usb_interface *pusb_intf, const struct usb_device_id *pdid)
{
	struct adapter *padapter = NULL;
	struct net_device *pnetdev = NULL;
	int status = _FAIL;

	padapter = (struct adapter *)rtw_zvmalloc(sizeof(*padapter));
	if (padapter == NULL)
		goto exit;
	padapter->dvobj = dvobj;
	dvobj->if1 = padapter;

	padapter->bDriverStopped = true;

	padapter->hw_init_mutex = &usb_drv->hw_init_mutex;

	/* step 1-1., decide the chip_type via vid/pid */
	padapter->interface_type = RTW_USB;
	chip_by_usb_id(padapter, pdid);

	if (rtw_handle_dualmac(padapter, 1) != _SUCCESS)
		goto free_adapter;

	pnetdev = rtw_init_netdev(padapter);
	if (pnetdev == NULL)
		goto handle_dualmac;
	SET_NETDEV_DEV(pnetdev, dvobj_to_dev(dvobj));
	padapter = rtw_netdev_priv(pnetdev);

	/* step 2. hook HalFunc, allocate HalData */
	hal_set_hal_ops(padapter);

	padapter->intf_start = &usb_intf_start;
	padapter->intf_stop = &usb_intf_stop;

	/* step init_io_priv */
	rtw_init_io_priv(padapter, usb_set_intf_ops);

	/* step read_chip_version */
	rtw_hal_read_chip_version(padapter);

	/* step usb endpoint mapping */
	rtw_hal_chip_configure(padapter);

	/* step read efuse/eeprom data and get mac_addr */
	rtw_hal_read_chip_info(padapter);

	/* step 5. */
	if (rtw_init_drv_sw(padapter) == _FAIL) {
		RT_TRACE(_module_hci_intfs_c_, _drv_err_,
			 ("Initialize driver software resource Failed!\n"));
		goto free_hal_data;
	}

#ifdef CONFIG_PM
	if (padapter->pwrctrlpriv.bSupportRemoteWakeup) {
		dvobj->pusbdev->do_remote_wakeup = 1;
		pusb_intf->needs_remote_wakeup = 1;
		device_init_wakeup(&pusb_intf->dev, 1);
		DBG_88E("\n  padapter->pwrctrlpriv.bSupportRemoteWakeup~~~~~~\n");
		DBG_88E("\n  padapter->pwrctrlpriv.bSupportRemoteWakeup~~~[%d]~~~\n",
			device_may_wakeup(&pusb_intf->dev));
	}
#endif

	/* 2012-07-11 Move here to prevent the 8723AS-VAU BT auto
	 * suspend influence */
	if (usb_autopm_get_interface(pusb_intf) < 0)
			DBG_88E("can't get autopm:\n");

	/*  alloc dev name after read efuse. */
	rtw_init_netdev_name(pnetdev, padapter->registrypriv.ifname);
	rtw_macaddr_cfg(padapter->eeprompriv.mac_addr);
#ifdef CONFIG_88EU_P2P
	rtw_init_wifidirect_addrs(padapter, padapter->eeprompriv.mac_addr,
				  padapter->eeprompriv.mac_addr);
#endif
	memcpy(pnetdev->dev_addr, padapter->eeprompriv.mac_addr, ETH_ALEN);
	DBG_88E("MAC Address from pnetdev->dev_addr =  %pM\n",
		pnetdev->dev_addr);

	/* step 6. Tell the network stack we exist */
	if (register_netdev(pnetdev) != 0) {
		RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("register_netdev() failed\n"));
		goto free_hal_data;
	}

	DBG_88E("bDriverStopped:%d, bSurpriseRemoved:%d, bup:%d, hw_init_completed:%d\n"
		, padapter->bDriverStopped
		, padapter->bSurpriseRemoved
		, padapter->bup
		, padapter->hw_init_completed
	);

	status = _SUCCESS;

free_hal_data:
	if (status != _SUCCESS)
		kfree(padapter->HalData);
handle_dualmac:
	if (status != _SUCCESS)
		rtw_handle_dualmac(padapter, 0);
free_adapter:
	if (status != _SUCCESS) {
		if (pnetdev)
			rtw_free_netdev(pnetdev);
		else if (padapter)
			rtw_vmfree((u8 *)padapter, sizeof(*padapter));
		padapter = NULL;
	}
exit:
	return padapter;
}

static void rtw_usb_if1_deinit(struct adapter *if1)
{
	struct net_device *pnetdev = if1->pnetdev;
	struct mlme_priv *pmlmepriv = &if1->mlmepriv;

	if (check_fwstate(pmlmepriv, _FW_LINKED))
		rtw_disassoc_cmd(if1, 0, false);

#ifdef CONFIG_88EU_AP_MODE
	free_mlme_ap_info(if1);
#endif

	if (if1->DriverState != DRIVER_DISAPPEAR) {
		if (pnetdev) {
			/* will call netdev_close() */
			unregister_netdev(pnetdev);
			rtw_proc_remove_one(pnetdev);
		}
	}
	rtw_cancel_all_timer(if1);

	rtw_dev_unload(if1);
	DBG_88E("+r871xu_dev_remove, hw_init_completed=%d\n",
		if1->hw_init_completed);
	rtw_handle_dualmac(if1, 0);
	rtw_free_drv_sw(if1);
	if (pnetdev)
		rtw_free_netdev(pnetdev);
}

static int rtw_drv_init(struct usb_interface *pusb_intf, const struct usb_device_id *pdid)
{
	struct adapter *if1 = NULL;
	int status;
	struct dvobj_priv *dvobj;

	RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("+rtw_drv_init\n"));

	/* step 0. */
	process_spec_devid(pdid);

	/* Initialize dvobj_priv */
	dvobj = usb_dvobj_init(pusb_intf);
	if (dvobj == NULL) {
		RT_TRACE(_module_hci_intfs_c_, _drv_err_,
			 ("initialize device object priv Failed!\n"));
		goto exit;
	}

	if1 = rtw_usb_if1_init(dvobj, pusb_intf, pdid);
	if (if1 == NULL) {
		DBG_88E("rtw_init_primarystruct adapter Failed!\n");
		goto free_dvobj;
	}

	if (ui_pid[1] != 0) {
		DBG_88E("ui_pid[1]:%d\n", ui_pid[1]);
		rtw_signal_process(ui_pid[1], SIGUSR2);
	}

	RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("-871x_drv - drv_init, success!\n"));

	status = _SUCCESS;

	if (status != _SUCCESS && if1)
		rtw_usb_if1_deinit(if1);
free_dvobj:
	if (status != _SUCCESS)
		usb_dvobj_deinit(pusb_intf);
exit:
	return status == _SUCCESS ? 0 : -ENODEV;
}

/*
 * dev_remove() - our device is being removed
*/
/* rmmod module & unplug(SurpriseRemoved) will call r871xu_dev_remove() => how to recognize both */
static void rtw_dev_remove(struct usb_interface *pusb_intf)
{
	struct dvobj_priv *dvobj = usb_get_intfdata(pusb_intf);
	struct adapter *padapter = dvobj->if1;

_func_enter_;

	DBG_88E("+rtw_dev_remove\n");
	RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("+dev_remove()\n"));

	if (usb_drv->drv_registered)
		padapter->bSurpriseRemoved = true;

	rtw_pm_set_ips(padapter, IPS_NONE);
	rtw_pm_set_lps(padapter, PS_MODE_ACTIVE);

	LeaveAllPowerSaveMode(padapter);

	rtw_usb_if1_deinit(padapter);

	usb_dvobj_deinit(pusb_intf);

	RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("-dev_remove()\n"));
	DBG_88E("-r871xu_dev_remove, done\n");
_func_exit_;

	return;
}

static int __init rtw_drv_entry(void)
{
	RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("+rtw_drv_entry\n"));

	DBG_88E(DRV_NAME " driver version=%s\n", DRIVERVERSION);
	DBG_88E("build time: %s %s\n", __DATE__, __TIME__);

	rtw_suspend_lock_init();

	_rtw_mutex_init(&usb_drv->hw_init_mutex);

	usb_drv->drv_registered = true;
	return usb_register(&usb_drv->usbdrv);
}

static void __exit rtw_drv_halt(void)
{
	RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("+rtw_drv_halt\n"));
	DBG_88E("+rtw_drv_halt\n");

	rtw_suspend_lock_uninit();

	usb_drv->drv_registered = false;
	usb_deregister(&usb_drv->usbdrv);

	_rtw_mutex_free(&usb_drv->hw_init_mutex);
	DBG_88E("-rtw_drv_halt\n");
}

module_init(rtw_drv_entry);
module_exit(rtw_drv_halt);
