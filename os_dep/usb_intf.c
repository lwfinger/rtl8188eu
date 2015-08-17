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

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <recv_osdep.h>
#include <xmit_osdep.h>
#include <hal_intf.h>
#include <rtw_version.h>

#include <usb_vendor_req.h>
#include <usb_ops.h>
#include <usb_osintf.h>
#include <usb_hal.h>

int ui_pid[3] = {0, 0, 0};

static int rtw_suspend(struct usb_interface *intf, pm_message_t message);
static int rtw_resume(struct usb_interface *intf);
int rtw_resume_process(struct adapter *padapter);


static int rtw_drv_init(struct usb_interface *pusb_intf,const struct usb_device_id *pdid);
static void rtw_dev_remove(struct usb_interface *pusb_intf);

static void rtw_dev_shutdown(struct device *dev)
{
	struct usb_interface *usb_intf = container_of(dev, struct usb_interface, dev);
	struct dvobj_priv *dvobj = usb_get_intfdata(usb_intf);
	struct adapter *adapter = dvobj->if1;
	int i;

	DBG_88E("%s\n", __func__);

	for (i = 0; i<dvobj->iface_nums; i++) {
		adapter = dvobj->padapters[i];
		adapter->bSurpriseRemoved = true;
	}

	ATOMIC_SET(&dvobj->continual_io_error, MAX_CONTINUAL_IO_ERR+1);
}

#if (LINUX_VERSION_CODE<=KERNEL_VERSION(2,6,23))
/* Some useful macros to use to create struct usb_device_id */
 #define USB_DEVICE_ID_MATCH_VENDOR			 0x0001
 #define USB_DEVICE_ID_MATCH_PRODUCT			 0x0002
 #define USB_DEVICE_ID_MATCH_DEV_LO			 0x0004
 #define USB_DEVICE_ID_MATCH_DEV_HI			 0x0008
 #define USB_DEVICE_ID_MATCH_DEV_CLASS			 0x0010
 #define USB_DEVICE_ID_MATCH_DEV_SUBCLASS		 0x0020
 #define USB_DEVICE_ID_MATCH_DEV_PROTOCOL		 0x0040
 #define USB_DEVICE_ID_MATCH_INT_CLASS			 0x0080
 #define USB_DEVICE_ID_MATCH_INT_SUBCLASS		 0x0100
 #define USB_DEVICE_ID_MATCH_INT_PROTOCOL		 0x0200
 #define USB_DEVICE_ID_MATCH_INT_NUMBER			 0x0400


#define USB_DEVICE_ID_MATCH_INT_INFO \
				 (USB_DEVICE_ID_MATCH_INT_CLASS | \
				 USB_DEVICE_ID_MATCH_INT_SUBCLASS | \
				 USB_DEVICE_ID_MATCH_INT_PROTOCOL)


#define USB_DEVICE_AND_INTERFACE_INFO(vend, prod, cl, sc, pr) \
		 .match_flags = USB_DEVICE_ID_MATCH_INT_INFO \
				 | USB_DEVICE_ID_MATCH_DEVICE, \
		 .idVendor = (vend), \
		 .idProduct = (prod), \
		 .bInterfaceClass = (cl), \
		 .bInterfaceSubClass = (sc), \
		 .bInterfaceProtocol = (pr)

 /**
  * USB_VENDOR_AND_INTERFACE_INFO - describe a specific usb vendor with a class of usb interfaces
  * @vend: the 16 bit USB Vendor ID
  * @cl: bInterfaceClass value
  * @sc: bInterfaceSubClass value
  * @pr: bInterfaceProtocol value
  *
  * This macro is used to create a struct usb_device_id that matches a
  * specific vendor with a specific class of interfaces.
  *
  * This is especially useful when explicitly matching devices that have
  * vendor specific bDeviceClass values, but standards-compliant interfaces.
  */
#define USB_VENDOR_AND_INTERFACE_INFO(vend, cl, sc, pr) \
		 .match_flags = USB_DEVICE_ID_MATCH_INT_INFO \
				 | USB_DEVICE_ID_MATCH_VENDOR, \
		 .idVendor = (vend), \
		 .bInterfaceClass = (cl), \
		 .bInterfaceSubClass = (sc), \
		 .bInterfaceProtocol = (pr)

/* ----------------------------------------------------------------------- */
#endif

#define USB_VENDER_ID_REALTEK		0x0BDA

#define RTL8188E_USB_IDS \
	/*=== Realtek demoboard ===*/ \
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x8179)}, /* 8188EUS */ \
	{USB_DEVICE(USB_VENDER_ID_REALTEK, 0x0179)}, /* 8188ETV */ \
	/*=== Customer ID ===*/ \
	/****** 8188EUS ********/ \
	{USB_DEVICE(0x07B8, 0x8179)}, /* Abocom - Abocom */

static struct usb_device_id rtw_usb_id_tbl[] ={
	RTL8188E_USB_IDS
	{}	/* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, rtw_usb_id_tbl);


static struct specific_device_id specific_device_id_tbl[] = {
	{.idVendor=USB_VENDER_ID_REALTEK, .idProduct=0x8177, .flags=SPEC_DEV_ID_DISABLE_HT},/* 8188cu 1*1 dongole, (b/g mode only) */
	{.idVendor=USB_VENDER_ID_REALTEK, .idProduct=0x817E, .flags=SPEC_DEV_ID_DISABLE_HT},/* 8188CE-VAU USB minCard (b/g mode only) */
	{.idVendor=0x0b05, .idProduct=0x1791, .flags=SPEC_DEV_ID_DISABLE_HT},
	{.idVendor=0x13D3, .idProduct=0x3311, .flags=SPEC_DEV_ID_DISABLE_HT},
	{.idVendor=0x13D3, .idProduct=0x3359, .flags=SPEC_DEV_ID_DISABLE_HT},/* Russian customer -Azwave (8188CE-VAU  g mode) */
	{}
};

struct rtw_usb_drv {
	struct usb_driver usbdrv;
	int drv_registered;
};

static struct usb_device_id rtl8188e_usb_id_tbl[] ={
	RTL8188E_USB_IDS
	{}	/* Terminating entry */
};

static struct rtw_usb_drv rtl8188e_usb_drv = {
	.usbdrv.name = (char*)"rtl8188eu",
	.usbdrv.probe = rtw_drv_init,
	.usbdrv.disconnect = rtw_dev_remove,
	.usbdrv.id_table = rtl8188e_usb_id_tbl,
	.usbdrv.suspend =  rtw_suspend,
	.usbdrv.resume = rtw_resume,
	#if (LINUX_VERSION_CODE > KERNEL_VERSION(2, 6, 22))
	.usbdrv.reset_resume   = rtw_resume,
	#endif
	#ifdef CONFIG_AUTOSUSPEND
	.usbdrv.supports_autosuspend = 1,
	#endif

	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 19))
	.usbdrv.drvwrap.driver.shutdown = rtw_dev_shutdown,
	#else
	.usbdrv.driver.shutdown = rtw_dev_shutdown,
	#endif
};

static struct rtw_usb_drv *usb_drv = &rtl8188e_usb_drv;

static inline int RT_usb_endpoint_dir_in(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN);
}

static inline int RT_usb_endpoint_dir_out(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bEndpointAddress & USB_ENDPOINT_DIR_MASK) == USB_DIR_OUT);
}

static inline int RT_usb_endpoint_xfer_int(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_INT);
}

static inline int RT_usb_endpoint_xfer_bulk(const struct usb_endpoint_descriptor *epd)
{
	return ((epd->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK) == USB_ENDPOINT_XFER_BULK);
}

static inline int RT_usb_endpoint_is_bulk_in(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_bulk(epd) && RT_usb_endpoint_dir_in(epd));
}

static inline int RT_usb_endpoint_is_bulk_out(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_bulk(epd) && RT_usb_endpoint_dir_out(epd));
}

static inline int RT_usb_endpoint_is_int_in(const struct usb_endpoint_descriptor *epd)
{
	return (RT_usb_endpoint_xfer_int(epd) && RT_usb_endpoint_dir_in(epd));
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
	dvobj->usb_vendor_req_buf  =
		(u8 *)N_BYTE_ALIGMENT((SIZE_PTR)(dvobj->usb_alloc_vendor_req_buf ), ALIGNMENT_UNIT);
exit:
	return rst;
}

static u8 rtw_deinit_intf_priv(struct dvobj_priv *dvobj)
{
	u8 rst = _SUCCESS;

	if (dvobj->usb_vendor_req_buf)
		rtw_mfree(dvobj->usb_alloc_vendor_req_buf, MAX_USB_IO_CTL_SIZE);

	_rtw_mutex_free(&dvobj->usb_vendor_req_mutex);

	return rst;
}

static struct dvobj_priv *usb_dvobj_init(struct usb_interface *usb_intf)
{
	int	i;
	u8	val8;
	int	status = _FAIL;
	struct dvobj_priv *pdvobjpriv;
	struct usb_device_descriptor	*pdev_desc;
	struct usb_host_config			*phost_conf;
	struct usb_config_descriptor		*pconf_desc;
	struct usb_host_interface		*phost_iface;
	struct usb_interface_descriptor	*piface_desc;
	struct usb_host_endpoint		*phost_endp;
	struct usb_endpoint_descriptor	*pendp_desc;
	struct usb_device				*pusbd;

;

	if ((pdvobjpriv = (struct dvobj_priv*)rtw_zmalloc(sizeof(*pdvobjpriv))) == NULL) {
		goto exit;
	}

	_rtw_mutex_init(&pdvobjpriv->hw_init_mutex);
	_rtw_mutex_init(&pdvobjpriv->h2c_fwcmd_mutex);
	_rtw_mutex_init(&pdvobjpriv->setch_mutex);
	_rtw_mutex_init(&pdvobjpriv->setbw_mutex);
	pdvobjpriv->processing_dev_remove = false;

	pdvobjpriv->pusbintf = usb_intf ;
	pusbd = pdvobjpriv->pusbdev = interface_to_usbdev(usb_intf);
	usb_set_intfdata(usb_intf, pdvobjpriv);

	pdvobjpriv->RtNumInPipes = 0;
	pdvobjpriv->RtNumOutPipes = 0;

	/* padapter->EepromAddressSize = 6; */
	/* pdvobjpriv->nr_endpoint = 6; */

	pdev_desc = &pusbd->descriptor;

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
			DBG_88E("bLength=%x\n",pendp_desc->bLength);
			DBG_88E("bDescriptorType=%x\n",pendp_desc->bDescriptorType);
			DBG_88E("bEndpointAddress=%x\n",pendp_desc->bEndpointAddress);
			DBG_88E("wMaxPacketSize=%d\n",le16_to_cpu(pendp_desc->wMaxPacketSize));
			DBG_88E("bInterval=%x\n",pendp_desc->bInterval);

			if (RT_usb_endpoint_is_bulk_in(pendp_desc)) {
				DBG_88E("RT_usb_endpoint_is_bulk_in = %x\n", RT_usb_endpoint_num(pendp_desc));
				pdvobjpriv->RtInPipe[pdvobjpriv->RtNumInPipes] = RT_usb_endpoint_num(pendp_desc);
				pdvobjpriv->RtNumInPipes++;
			} else if (RT_usb_endpoint_is_int_in(pendp_desc)) {
				DBG_88E("RT_usb_endpoint_is_int_in = %x, Interval = %x\n", RT_usb_endpoint_num(pendp_desc),pendp_desc->bInterval);
				pdvobjpriv->RtInPipe[pdvobjpriv->RtNumInPipes] = RT_usb_endpoint_num(pendp_desc);
				pdvobjpriv->RtNumInPipes++;
			}
			else if (RT_usb_endpoint_is_bulk_out(pendp_desc))
			{
				DBG_88E("RT_usb_endpoint_is_bulk_out = %x\n", RT_usb_endpoint_num(pendp_desc));
				pdvobjpriv->RtOutPipe[pdvobjpriv->RtNumOutPipes] = RT_usb_endpoint_num(pendp_desc);
				pdvobjpriv->RtNumOutPipes++;
			}
			pdvobjpriv->ep_num[i] = RT_usb_endpoint_num(pendp_desc);
		}
	}

	DBG_88E("nr_endpoint=%d, in_num=%d, out_num=%d\n\n", pdvobjpriv->nr_endpoint, pdvobjpriv->RtNumInPipes, pdvobjpriv->RtNumOutPipes);

	if (pusbd->speed == USB_SPEED_HIGH) {
		pdvobjpriv->ishighspeed = true;
		DBG_88E("USB_SPEED_HIGH\n");
	} else {
		pdvobjpriv->ishighspeed = false;
		DBG_88E("NON USB_SPEED_HIGH\n");
	}

	if (rtw_init_intf_priv(pdvobjpriv) == _FAIL) {
		RT_TRACE(_module_os_intfs_c_,_drv_err_,("\n Can't INIT rtw_init_intf_priv\n"));
		goto free_dvobj;
	}

	/* 3 misc */
	_rtw_init_sema(&(pdvobjpriv->usb_suspend_sema), 0);
	rtw_reset_continual_io_error(pdvobjpriv);

	usb_get_dev(pusbd);

	status = _SUCCESS;

free_dvobj:
	if (status != _SUCCESS && pdvobjpriv) {
		usb_set_intfdata(usb_intf, NULL);
		_rtw_mutex_free(&pdvobjpriv->hw_init_mutex);
		_rtw_mutex_free(&pdvobjpriv->h2c_fwcmd_mutex);
		_rtw_mutex_free(&pdvobjpriv->setch_mutex);
		_rtw_mutex_free(&pdvobjpriv->setbw_mutex);
		rtw_mfree((u8*)pdvobjpriv, sizeof(*pdvobjpriv));
		pdvobjpriv = NULL;
	}
exit:
;
	return pdvobjpriv;
}

static void usb_dvobj_deinit(struct usb_interface *usb_intf)
{
	struct dvobj_priv *dvobj = usb_get_intfdata(usb_intf);

;

	usb_set_intfdata(usb_intf, NULL);
	if (dvobj) {
		/* Modify condition for 92DU DMDP 2010.11.18, by Thomas */
		if ((dvobj->NumInterfaces != 2 && dvobj->NumInterfaces != 3)
			|| (dvobj->InterfaceNumber == 1)) {
			if (interface_to_usbdev(usb_intf)->state != USB_STATE_NOTATTACHED) {
				/* If we didn't unplug usb dongle and remove/insert modlue, driver fails on sitesurvey for the first time when device is up . */
				/* Reset usb port for sitesurvey fail issue. 2009.8.13, by Thomas */
				DBG_88E("usb attached..., try to reset usb device\n");
				usb_reset_device(interface_to_usbdev(usb_intf));
			}
		}
		rtw_deinit_intf_priv(dvobj);
		_rtw_mutex_free(&dvobj->hw_init_mutex);
		_rtw_mutex_free(&dvobj->h2c_fwcmd_mutex);
		_rtw_mutex_free(&dvobj->setch_mutex);
		_rtw_mutex_free(&dvobj->setbw_mutex);
		rtw_mfree((u8*)dvobj, sizeof(*dvobj));
	}

	/* DBG_88E("%s %d\n", __func__, ATOMIC_READ(&usb_intf->dev.kobj.kref.refcount)); */
	usb_put_dev(interface_to_usbdev(usb_intf));

;
}

static void decide_chip_type_by_usb_device_id(struct adapter *padapter, const struct usb_device_id *pdid)
{
	padapter->chip_type = NULL_CHIP_TYPE;
	hal_set_hw_type(padapter);
}

static void usb_intf_start(struct adapter *padapter)
{

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+usb_intf_start\n"));

	rtw_hal_inirp_init(padapter);

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-usb_intf_start\n"));

}

static void usb_intf_stop(struct adapter *padapter)
{

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+usb_intf_stop\n"));

	/* disabel_hw_interrupt */
	if (padapter->bSurpriseRemoved == false)
	{
		/* device still exists, so driver can do i/o operation */
		/* TODO: */
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("SurpriseRemoved==false\n"));
	}

	/* cancel in irp */
	rtw_hal_inirp_deinit(padapter);

	/* cancel out irp */
	rtw_write_port_cancel(padapter);

	/* todo:cancel other irps */

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-usb_intf_stop\n"));

}

void rtw_dev_unload(struct adapter *padapter)
{
	struct net_device *pnetdev= (struct net_device*)padapter->pnetdev;
	u8 val8;
	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+rtw_dev_unload\n"));

	if (padapter->bup == true)
	{
		DBG_88E("===> rtw_dev_unload\n");

		padapter->bDriverStopped = true;
		if (padapter->xmitpriv.ack_tx)
			rtw_ack_tx_done(&padapter->xmitpriv, RTW_SCTX_DONE_DRV_STOP);

		/* s3. */
		if (padapter->intf_stop)
		{
			padapter->intf_stop(padapter);
		}

		/* s4. */
		if (!adapter_to_pwrctl(padapter)->bInternalAutoSuspend )
			rtw_stop_drv_threads(padapter);


		/* s5. */
		if (padapter->bSurpriseRemoved == false)
		{
			rtw_hal_deinit(padapter);
			padapter->bSurpriseRemoved = true;
		}

		padapter->bup = false;
	} else {
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("r871x_dev_unload():padapter->bup == false\n" ));
	}

	DBG_88E("<=== rtw_dev_unload\n");

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-rtw_dev_unload\n"));

}

static void process_spec_devid(const struct usb_device_id *pdid)
{
	u16 vid, pid;
	u32 flags;
	int i;
	int num = sizeof(specific_device_id_tbl)/sizeof(struct specific_device_id);

	for (i=0; i<num; i++)
	{
		vid = specific_device_id_tbl[i].idVendor;
		pid = specific_device_id_tbl[i].idProduct;
		flags = specific_device_id_tbl[i].flags;

		if ((pdid->idVendor==vid) && (pdid->idProduct==pid) && (flags&SPEC_DEV_ID_DISABLE_HT))
		{
			 rtw_ht_enable = 0;
			 rtw_cbw40_enable = 0;
			 rtw_ampdu_enable = 0;
		}
	}
}

int rtw_hw_suspend(struct adapter *padapter )
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct usb_interface *pusb_intf = adapter_to_dvobj(padapter)->pusbintf;
	struct net_device *pnetdev = padapter->pnetdev;

	if ((!padapter->bup) || (padapter->bDriverStopped)||(padapter->bSurpriseRemoved))
	{
		DBG_88E("padapter->bup=%d bDriverStopped=%d bSurpriseRemoved = %d\n",
			padapter->bup, padapter->bDriverStopped,padapter->bSurpriseRemoved);
		goto error_exit;
	}

	LeaveAllPowerSaveMode(padapter);

	DBG_88E("==> rtw_hw_suspend\n");
	_enter_pwrlock(&pwrpriv->lock);
	pwrpriv->bips_processing = true;
	/* s1. */
	if (pnetdev)
	{
		netif_carrier_off(pnetdev);
		rtw_netif_stop_queue(pnetdev);
	}

	/* s2. */
	rtw_disassoc_cmd(padapter, 500, false);

	/* s2-2.  indicate disconnect to os */
	{
		struct	mlme_priv *pmlmepriv = &padapter->mlmepriv;

		if (check_fwstate(pmlmepriv, _FW_LINKED))
		{
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
	rtw_free_network_queue(padapter,true);
	rtw_ips_dev_unload(padapter);
	pwrpriv->rf_pwrstate = rf_off;
	pwrpriv->bips_processing = false;

	_exit_pwrlock(&pwrpriv->lock);

	return 0;

error_exit:
	DBG_88E("%s, failed\n",__FUNCTION__);
	return (-1);

}

int rtw_hw_resume(struct adapter *padapter)
{
	struct pwrctrl_priv *pwrpriv;
	struct usb_interface *pusb_intf;
	struct net_device *pnetdev;

	if (padapter)/* system resume */
	{
		pwrpriv = adapter_to_pwrctl(padapter);
		pusb_intf = adapter_to_dvobj(padapter)->pusbintf;
		pnetdev = padapter->pnetdev;
		DBG_88E("==> rtw_hw_resume\n");
		_enter_pwrlock(&pwrpriv->lock);
		pwrpriv->bips_processing = true;
		rtw_reset_drv_sw(padapter);

		if (pm_netdev_open(pnetdev,false) != 0) {
			_exit_pwrlock(&pwrpriv->lock);
			goto error_exit;
		}

		netif_device_attach(pnetdev);
		netif_carrier_on(pnetdev);

		if (!rtw_netif_queue_stopped(pnetdev))
			rtw_netif_start_queue(pnetdev);
		else
			rtw_netif_wake_queue(pnetdev);

		pwrpriv->bkeepfwalive = false;
		pwrpriv->brfoffbyhw = false;

		pwrpriv->rf_pwrstate = rf_on;
		pwrpriv->bips_processing = false;

		_exit_pwrlock(&pwrpriv->lock);
	} else {
		goto error_exit;
	}

	return 0;
error_exit:
	DBG_88E("%s, Open net dev failed\n",__FUNCTION__);
	return (-1);
}

static int rtw_suspend(struct usb_interface *pusb_intf, pm_message_t message)
{
	struct dvobj_priv *dvobj = usb_get_intfdata(pusb_intf);
	struct adapter *padapter = dvobj->if1;
	struct net_device *pnetdev = padapter->pnetdev;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct pwrctrl_priv *pwrpriv = dvobj_to_pwrctl(dvobj);
	struct usb_device *usb_dev = interface_to_usbdev(pusb_intf);


	int ret = 0;
	u32 start_time = jiffies;

	;

	DBG_88E("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);


	if (!padapter->bup) {
		u8 bMacPwrCtrlOn = false;
		rtw_hal_get_hwreg(padapter, HW_VAR_APFM_ON_MAC, &bMacPwrCtrlOn);
		if (bMacPwrCtrlOn)
			rtw_hal_power_off(padapter);
	}

	if ((!padapter->bup) || (padapter->bDriverStopped)||(padapter->bSurpriseRemoved))
	{
		DBG_88E("padapter->bup=%d bDriverStopped=%d bSurpriseRemoved = %d\n",
			padapter->bup, padapter->bDriverStopped,padapter->bSurpriseRemoved);
		goto exit;
	}

	if (pwrpriv->bInternalAutoSuspend )
	{
	#ifdef CONFIG_AUTOSUSPEND
		/*  The FW command register update must after MAC and FW init ready. */
		if ((padapter->bFWReady) && (pwrpriv->bHWPwrPindetect ) && (padapter->registrypriv.usbss_enable ))
		{
			u8 bOpen = true;
			rtw_interface_ps_func(padapter,HAL_USB_SELECT_SUSPEND,&bOpen);
		}
	#endif
	}

	pwrpriv->bInSuspend = true;

	_enter_pwrlock(&pwrpriv->lock);
	rtw_suspend_common(padapter);

#ifdef CONFIG_AUTOSUSPEND
	pwrpriv->rf_pwrstate = rf_off;
	pwrpriv->bips_processing = false;
#endif
	_exit_pwrlock(&pwrpriv->lock);

exit:
	DBG_88E("<===  %s return %d.............. in %dms\n", __FUNCTION__
		, ret, rtw_get_passing_time_ms(start_time));

	return ret;
}

static int rtw_resume(struct usb_interface *pusb_intf)
{
	struct dvobj_priv *dvobj = usb_get_intfdata(pusb_intf);
	struct adapter *padapter = dvobj->if1;
	struct net_device *pnetdev = padapter->pnetdev;
	struct pwrctrl_priv *pwrpriv = dvobj_to_pwrctl(dvobj);
	int ret = 0;

	if (pwrpriv->bInternalAutoSuspend ) {
		ret = rtw_resume_process(padapter);
	} else {
		if (rtw_is_earlysuspend_registered(pwrpriv)) {
			/* jeff: bypass resume here, do in late_resume */
			rtw_set_do_late_resume(pwrpriv, true);
		} else {
			ret = rtw_resume_process(padapter);
		}
	}

	return ret;
}

int rtw_resume_process(struct adapter *padapter)
{
	struct net_device *pnetdev;
	struct pwrctrl_priv *pwrpriv;
	int ret = -1;
	u32 start_time = jiffies;
#ifdef CONFIG_BT_COEXIST
	u8 pm_cnt;
#endif	/* ifdef CONFIG_BT_COEXIST */
	;

	DBG_88E("==> %s (%s:%d)\n",__FUNCTION__, current->comm, current->pid);

	if (padapter) {
		pnetdev= padapter->pnetdev;
		pwrpriv = adapter_to_pwrctl(padapter);
	} else {
		goto exit;
	}

	_enter_pwrlock(&pwrpriv->lock);
#ifdef CONFIG_BT_COEXIST
#ifdef CONFIG_AUTOSUSPEND
	#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,32))
	DBG_88E("%s...pm_usage_cnt(%d)  pwrpriv->bAutoResume=%x.  ....\n",__func__,atomic_read(&(adapter_to_dvobj(padapter)->pusbintf->pm_usage_cnt)),pwrpriv->bAutoResume);
	pm_cnt=atomic_read(&(adapter_to_dvobj(padapter)->pusbintf->pm_usage_cnt));
	#else
	DBG_88E("...pm_usage_cnt(%d).....\n", adapter_to_dvobj(padapter)->pusbintf->pm_usage_cnt);
	pm_cnt = adapter_to_dvobj(padapter)->pusbintf->pm_usage_cnt;
	#endif

	DBG_88E("pwrpriv->bAutoResume (%x)\n",pwrpriv->bAutoResume );
	if ( true == pwrpriv->bAutoResume ) {
		pwrpriv->bInternalAutoSuspend = false;
		pwrpriv->bAutoResume=false;
		DBG_88E("pwrpriv->bAutoResume (%x)  pwrpriv->bInternalAutoSuspend(%x)\n",pwrpriv->bAutoResume,pwrpriv->bInternalAutoSuspend );

	}
#endif /* ifdef CONFIG_AUTOSUSPEND */
#endif /* ifdef CONFIG_BT_COEXIST */


	if (rtw_resume_common(padapter)!= 0) {
		DBG_88E("%s rtw_resume_common failed\n",__FUNCTION__);
		_exit_pwrlock(&pwrpriv->lock);
		goto exit;
	}

#ifdef CONFIG_AUTOSUSPEND
	if (pwrpriv->bInternalAutoSuspend )
	{
		#ifdef CONFIG_AUTOSUSPEND
			/*  The FW command register update must after MAC and FW init ready. */
		if ((padapter->bFWReady) && (pwrpriv->bHWPwrPindetect) && (padapter->registrypriv.usbss_enable ))
		{
			u8 bOpen = false;
			rtw_interface_ps_func(padapter,HAL_USB_SELECT_SUSPEND,&bOpen);
		}
		#endif
#ifdef CONFIG_BT_COEXIST
		DBG_88E("pwrpriv->bAutoResume (%x)\n",pwrpriv->bAutoResume );
		if ( true == pwrpriv->bAutoResume ) {
			pwrpriv->bInternalAutoSuspend = false;
			pwrpriv->bAutoResume=false;
			DBG_88E("pwrpriv->bAutoResume (%x)  pwrpriv->bInternalAutoSuspend(%x)\n",pwrpriv->bAutoResume,pwrpriv->bInternalAutoSuspend );
		}

#else	/* ifdef CONFIG_BT_COEXIST */
		pwrpriv->bInternalAutoSuspend = false;
#endif	/* ifdef CONFIG_BT_COEXIST */
		pwrpriv->brfoffbyhw = false;
	}
#endif
	_exit_pwrlock(&pwrpriv->lock);

	if ( padapter->pid[1]!=0) {
		DBG_88E("pid[1]:%d\n",padapter->pid[1]);
		rtw_signal_process(padapter->pid[1], SIGUSR2);
	}

	ret = 0;
exit:
	pwrpriv->bInSuspend = false;
	DBG_88E("<===  %s return %d.............. in %dms\n", __FUNCTION__
		, ret, rtw_get_passing_time_ms(start_time));

	return ret;
}

#ifdef CONFIG_AUTOSUSPEND
void autosuspend_enter(struct adapter* padapter)
{
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	DBG_88E("==>autosuspend_enter...........\n");

	pwrpriv->bInternalAutoSuspend = true;
	pwrpriv->bips_processing = true;

	if (rf_off == pwrpriv->change_rfpwrstate )
	{
#ifndef	CONFIG_BT_COEXIST
		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
		usb_enable_autosuspend(dvobj->pusbdev);
		#else
		dvobj->pusbdev->autosuspend_disabled = 0;/* autosuspend disabled by the user */
		#endif

		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))
			usb_autopm_put_interface(dvobj->pusbintf);
		#elif (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,20))
			usb_autopm_enable(dvobj->pusbintf);
		#else
			usb_autosuspend_device(dvobj->pusbdev, 1);
		#endif
#else	/* ifndef	CONFIG_BT_COEXIST */
		if (1==pwrpriv->autopm_cnt) {
		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
		usb_enable_autosuspend(dvobj->pusbdev);
		#else
		dvobj->pusbdev->autosuspend_disabled = 0;/* autosuspend disabled by the user */
		#endif

		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))
			usb_autopm_put_interface(dvobj->pusbintf);
		#elif (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,20))
			usb_autopm_enable(dvobj->pusbintf);
		#else
			usb_autosuspend_device(dvobj->pusbdev, 1);
		#endif
			pwrpriv->autopm_cnt --;
		}
		else
		DBG_88E("0!=pwrpriv->autopm_cnt[%d]   didn't usb_autopm_put_interface\n", pwrpriv->autopm_cnt);

#endif	/* ifndef	CONFIG_BT_COEXIST */
	}
	#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,32))
	DBG_88E("...pm_usage_cnt(%d).....\n", atomic_read(&(dvobj->pusbintf->pm_usage_cnt)));
	#else
	DBG_88E("...pm_usage_cnt(%d).....\n", dvobj->pusbintf->pm_usage_cnt);
	#endif

}
int autoresume_enter(struct adapter* padapter)
{
	int result = _SUCCESS;
	struct pwrctrl_priv *pwrpriv = adapter_to_pwrctl(padapter);
	struct security_priv* psecuritypriv=&(padapter->securitypriv);
	struct mlme_ext_priv	*pmlmeext = &padapter->mlmeextpriv;
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct dvobj_priv *dvobj = adapter_to_dvobj(padapter);

	DBG_88E("====> autoresume_enter\n");

	if (rf_off == pwrpriv->rf_pwrstate )
	{
		pwrpriv->ps_flag = false;
#ifndef	CONFIG_BT_COEXIST
		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))
			if (usb_autopm_get_interface(dvobj->pusbintf) < 0)
			{
				DBG_88E( "can't get autopm: %d\n", result);
				result = _FAIL;
				goto error_exit;
			}
		#elif (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,20))
			usb_autopm_disable(dvobj->pusbintf);
		#else
			usb_autoresume_device(dvobj->pusbdev, 1);
		#endif

		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,32))
		DBG_88E("...pm_usage_cnt(%d).....\n", atomic_read(&(dvobj->pusbintf->pm_usage_cnt)));
		#else
		DBG_88E("...pm_usage_cnt(%d).....\n", dvobj->pusbintf->pm_usage_cnt);
		#endif
#else	/* ifndef	CONFIG_BT_COEXIST */
		pwrpriv->bAutoResume=true;
		if (0==pwrpriv->autopm_cnt) {
		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))
			if (usb_autopm_get_interface(dvobj->pusbintf) < 0)
			{
				DBG_88E( "can't get autopm: %d\n", result);
				result = _FAIL;
				goto error_exit;
			}
		#elif (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,20))
			usb_autopm_disable(dvobj->pusbintf);
		#else
			usb_autoresume_device(dvobj->pusbdev, 1);
		#endif
		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,32))
			DBG_88E("...pm_usage_cnt(%d).....\n", atomic_read(&(dvobj->pusbintf->pm_usage_cnt)));
		#else
			DBG_88E("...pm_usage_cnt(%d).....\n", dvobj->pusbintf->pm_usage_cnt);
		#endif
			pwrpriv->autopm_cnt++;
		}
		else
			DBG_88E("0!=pwrpriv->autopm_cnt[%d]   didn't usb_autopm_get_interface\n",pwrpriv->autopm_cnt);
#endif /* ifndef	CONFIG_BT_COEXIST */
	}
	DBG_88E("<==== autoresume_enter\n");
error_exit:

	return result;
}
#endif

/*
 * drv_init() - a device potentially for us
 *
 * notes: drv_init() is called when the bus driver has located a card for us to support.
 *        We accept the new device by returning 0.
*/

static struct adapter  *rtw_sw_export = NULL;

static struct adapter *rtw_usb_if1_init(struct dvobj_priv *dvobj,
	struct usb_interface *pusb_intf, const struct usb_device_id *pdid)
{
	struct adapter *padapter = NULL;
	struct net_device *pnetdev = NULL;
	int status = _FAIL;

	if ((padapter = (struct adapter *)rtw_zvmalloc(sizeof(*padapter))) == NULL) {
		goto exit;
	}
	padapter->dvobj = dvobj;
	dvobj->if1 = padapter;

	padapter->bDriverStopped=true;

	dvobj->padapters[dvobj->iface_nums++] = padapter;
	padapter->iface_id = IFACE_ID0;

	#ifndef RTW_DVOBJ_CHIP_HW_TYPE
	/* step 1-1., decide the chip_type via vid/pid */
	padapter->interface_type = RTW_USB;
	decide_chip_type_by_usb_device_id(padapter, pdid);
	#endif

	if (rtw_handle_dualmac(padapter, 1) != _SUCCESS)
		goto free_adapter;

	if ((pnetdev = rtw_init_netdev(padapter)) == NULL) {
		goto handle_dualmac;
	}
	SET_NETDEV_DEV(pnetdev, dvobj_to_dev(dvobj));
	padapter = rtw_netdev_priv(pnetdev);

	if (rtw_wdev_alloc(padapter, dvobj_to_dev(dvobj)) != 0) {
		goto handle_dualmac;
	}

	/* step 2. hook HalFunc, allocate HalData */
	hal_set_hal_ops(padapter);

	padapter->intf_start=&usb_intf_start;
	padapter->intf_stop=&usb_intf_stop;

	/* step init_io_priv */
	rtw_init_io_priv(padapter, usb_set_intf_ops);

	/* step read_chip_version */
	rtw_hal_read_chip_version(padapter);

	/* step usb endpoint mapping */
	rtw_hal_chip_configure(padapter);

	/* step read efuse/eeprom data and get mac_addr */
	rtw_hal_read_chip_info(padapter);

	/* step 5. */
	if (rtw_init_drv_sw(padapter) ==_FAIL) {
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("Initialize driver software resource Failed!\n"));
		goto free_hal_data;
	}

#ifdef CONFIG_PM
#if (LINUX_VERSION_CODE > KERNEL_VERSION(2,6,18))
	if (adapter_to_pwrctl(padapter)->bSupportRemoteWakeup)
	{
		dvobj->pusbdev->do_remote_wakeup=1;
		pusb_intf->needs_remote_wakeup = 1;
		device_init_wakeup(&pusb_intf->dev, 1);
		DBG_88E("\n  pwrctrlpriv.bSupportRemoteWakeup~~~~~~\n");
		DBG_88E("\n  pwrctrlpriv.bSupportRemoteWakeup~~~[%d]~~~\n",device_may_wakeup(&pusb_intf->dev));
	}
#endif
#endif

#ifdef CONFIG_AUTOSUSPEND
	if ( padapter->registrypriv.power_mgnt != PS_MODE_ACTIVE )
	{
		if (padapter->registrypriv.usbss_enable ) {	/* autosuspend (2s delay) */
			#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,38))
			dvobj->pusbdev->dev.power.autosuspend_delay = 0 * HZ;/* 15 * HZ; idle-delay time */
			#else
			dvobj->pusbdev->autosuspend_delay = 0 * HZ;/* 15 * HZ; idle-delay time */
			#endif

			#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,35))
			usb_enable_autosuspend(dvobj->pusbdev);
			#elif  (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,22) && LINUX_VERSION_CODE<=KERNEL_VERSION(2,6,34))
			padapter->bDisableAutosuspend = dvobj->pusbdev->autosuspend_disabled ;
			dvobj->pusbdev->autosuspend_disabled = 0;/* autosuspend disabled by the user */
			#endif

			#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,32))
			DBG_88E("%s...pm_usage_cnt(%d).....\n",__FUNCTION__,atomic_read(&(dvobj->pusbintf ->pm_usage_cnt)));
			#else
			DBG_88E("%s...pm_usage_cnt(%d).....\n",__FUNCTION__,dvobj->pusbintf ->pm_usage_cnt);
			#endif
		}
	}
#endif
	/* 2012-07-11 Move here to prevent the 8723AS-VAU BT auto suspend influence */
	#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))
			if (usb_autopm_get_interface(pusb_intf) < 0)
				{
					DBG_88E( "can't get autopm:\n");
				}
	#endif
#ifdef	CONFIG_BT_COEXIST
	adapter_to_pwrctl(padapter)->autopm_cnt=1;
#endif

	/*  set mac addr */
	rtw_macaddr_cfg(padapter->eeprompriv.mac_addr);
#ifdef CONFIG_P2P
	rtw_init_wifidirect_addrs(padapter, padapter->eeprompriv.mac_addr, padapter->eeprompriv.mac_addr);
#endif
	DBG_88E("bDriverStopped:%d, bSurpriseRemoved:%d, bup:%d, hw_init_completed:%d\n"
		, padapter->bDriverStopped
		, padapter->bSurpriseRemoved
		, padapter->bup
		, padapter->hw_init_completed
	);

	status = _SUCCESS;

free_hal_data:
	if (status != _SUCCESS && padapter->HalData)
		kfree(padapter->HalData);
free_wdev:
	if (status != _SUCCESS) {
		rtw_wdev_unregister(padapter->rtw_wdev);
		rtw_wdev_free(padapter->rtw_wdev);
	}
handle_dualmac:
	if (status != _SUCCESS)
		rtw_handle_dualmac(padapter, 0);
free_adapter:
	if (status != _SUCCESS) {
		if (pnetdev)
			rtw_free_netdev(pnetdev);
		else if (padapter)
			rtw_vmfree((u8*)padapter, sizeof(*padapter));
		padapter = NULL;
	}
exit:
	return padapter;
}

static void rtw_usb_if1_deinit(struct adapter *if1)
{
	struct net_device *pnetdev = if1->pnetdev;
	struct mlme_priv *pmlmepriv= &if1->mlmepriv;
	struct pwrctrl_priv *pwrctl = adapter_to_pwrctl(if1);

	if (check_fwstate(pmlmepriv, _FW_LINKED))
		rtw_disassoc_cmd(if1, 0, false);


#ifdef CONFIG_AP_MODE
	free_mlme_ap_info(if1);
	#ifdef CONFIG_HOSTAPD_MLME
	hostapd_mode_unload(if1);
	#endif
#endif
	rtw_cancel_all_timer(if1);

	rtw_dev_unload(if1);

	DBG_88E("+r871xu_dev_remove, hw_init_completed=%d\n", if1->hw_init_completed);

	rtw_handle_dualmac(if1, 0);

	if (if1->rtw_wdev) {
		/* rtw_wdev_unregister(if1->rtw_wdev); */
		rtw_wdev_free(if1->rtw_wdev);
	}

#ifdef CONFIG_BT_COEXIST
	if (1 == pwrctl->autopm_cnt) {
		#if (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,33))
			usb_autopm_put_interface(adapter_to_dvobj(if1)->pusbintf);
		#elif (LINUX_VERSION_CODE>=KERNEL_VERSION(2,6,20))
			usb_autopm_enable(adapter_to_dvobj(if1)->pusbintf);
		#else
			usb_autosuspend_device(adapter_to_dvobj(if1)->pusbdev, 1);
		#endif
		pwrctl->autopm_cnt --;
	}
#endif

	rtw_free_drv_sw(if1);

	if (pnetdev)
		rtw_free_netdev(pnetdev);
}

static void dump_usb_interface(struct usb_interface *usb_intf)
{
	int	i;
	u8	val8;

	struct usb_device				*udev = interface_to_usbdev(usb_intf);
	struct usb_device_descriptor	*dev_desc = &udev->descriptor;

	struct usb_host_config			*act_conf = udev->actconfig;
	struct usb_config_descriptor	*act_conf_desc = &act_conf->desc;

	struct usb_host_interface		*host_iface;
	struct usb_interface_descriptor	*iface_desc;
	struct usb_host_endpoint		*host_endp;
	struct usb_endpoint_descriptor	*endp_desc;

	DBG_88E("usb_interface:%p, usb_device:%p(num:%d, path:%s), usb_device_descriptor:%p\n", usb_intf, udev, udev->devnum, udev->devpath, dev_desc);
	DBG_88E("bLength:%u\n", dev_desc->bLength);
	DBG_88E("bDescriptorType:0x%02x\n", dev_desc->bDescriptorType);
	DBG_88E("bcdUSB:0x%04x\n", le16_to_cpu(dev_desc->bcdUSB));
	DBG_88E("bDeviceClass:0x%02x\n", dev_desc->bDeviceClass);
	DBG_88E("bDeviceSubClass:0x%02x\n", dev_desc->bDeviceSubClass);
	DBG_88E("bDeviceProtocol:0x%02x\n", dev_desc->bDeviceProtocol);
	DBG_88E("bMaxPacketSize0:%u\n", dev_desc->bMaxPacketSize0);
	DBG_88E("idVendor:0x%04x\n", le16_to_cpu(dev_desc->idVendor));
	DBG_88E("idProduct:0x%04x\n", le16_to_cpu(dev_desc->idProduct));
	DBG_88E("bcdDevice:0x%04x\n", le16_to_cpu(dev_desc->bcdDevice));
	DBG_88E("iManufacturer:0x02%x\n", dev_desc->iManufacturer);
	DBG_88E("iProduct:0x%02x\n", dev_desc->iProduct);
	DBG_88E("iSerialNumber:0x%02x\n", dev_desc->iSerialNumber);
	DBG_88E("bNumConfigurations:%u\n", dev_desc->bNumConfigurations);

	DBG_88E("\nact_conf_desc:%p\n", act_conf_desc);
	DBG_88E("bLength:%u\n", act_conf_desc->bLength);
	DBG_88E("bDescriptorType:0x%02x\n", act_conf_desc->bDescriptorType);
	DBG_88E("wTotalLength:%u\n", le16_to_cpu(act_conf_desc->wTotalLength));
	DBG_88E("bNumInterfaces:%u\n", act_conf_desc->bNumInterfaces);
	DBG_88E("bConfigurationValue:0x%02x\n", act_conf_desc->bConfigurationValue);
	DBG_88E("iConfiguration:0x%02x\n", act_conf_desc->iConfiguration);
	DBG_88E("bmAttributes:0x%02x\n", act_conf_desc->bmAttributes);
	DBG_88E("bMaxPower=%u\n", act_conf_desc->bMaxPower);

	DBG_88E("****** num of altsetting = (%d) ******/\n", usb_intf->num_altsetting);
	/* Get he host side alternate setting (the current alternate setting) for this interface*/
	host_iface = usb_intf->cur_altsetting;
	iface_desc = &host_iface->desc;

	DBG_88E("\nusb_interface_descriptor:%p:\n", iface_desc);
	DBG_88E("bLength:%u\n", iface_desc->bLength);
	DBG_88E("bDescriptorType:0x%02x\n", iface_desc->bDescriptorType);
	DBG_88E("bInterfaceNumber:0x%02x\n", iface_desc->bInterfaceNumber);
	DBG_88E("bAlternateSetting=%x\n", iface_desc->bAlternateSetting);
	DBG_88E("bNumEndpoints=%x\n", iface_desc->bNumEndpoints);
	DBG_88E("bInterfaceClass=%x\n", iface_desc->bInterfaceClass);
	DBG_88E("bInterfaceSubClass=%x\n", iface_desc->bInterfaceSubClass);
	DBG_88E("bInterfaceProtocol=%x\n", iface_desc->bInterfaceProtocol);
	DBG_88E("iInterface=%x\n", iface_desc->iInterface);

	for (i = 0; i < iface_desc->bNumEndpoints; i++)
	{
		host_endp = host_iface->endpoint + i;
		if (host_endp)
		{
			endp_desc = &host_endp->desc;

			DBG_88E("\nusb_endpoint_descriptor(%d):\n", i);
			DBG_88E("bLength=%x\n",endp_desc->bLength);
			DBG_88E("bDescriptorType=%x\n",endp_desc->bDescriptorType);
			DBG_88E("bEndpointAddress=%x\n",endp_desc->bEndpointAddress);
			DBG_88E("bmAttributes=%x\n",endp_desc->bmAttributes);
			DBG_88E("wMaxPacketSize=%x\n",endp_desc->wMaxPacketSize);
			DBG_88E("wMaxPacketSize=%x\n",le16_to_cpu(endp_desc->wMaxPacketSize));
			DBG_88E("bInterval=%x\n",endp_desc->bInterval);

			if (RT_usb_endpoint_is_bulk_in(endp_desc))
			{
				DBG_88E("RT_usb_endpoint_is_bulk_in = %x\n", RT_usb_endpoint_num(endp_desc));
			}
			else if (RT_usb_endpoint_is_int_in(endp_desc))
			{
				DBG_88E("RT_usb_endpoint_is_int_in = %x, Interval = %x\n", RT_usb_endpoint_num(endp_desc),endp_desc->bInterval);
			}
			else if (RT_usb_endpoint_is_bulk_out(endp_desc))
			{
				DBG_88E("RT_usb_endpoint_is_bulk_out = %x\n", RT_usb_endpoint_num(endp_desc));
			}
		}
	}

	if (udev->speed == USB_SPEED_HIGH)
		DBG_88E("USB_SPEED_HIGH\n");
	else
		DBG_88E("NON USB_SPEED_HIGH\n");
}

static int rtw_drv_init(struct usb_interface *pusb_intf, const struct usb_device_id *pdid)
{
	int i;
	struct adapter *if1 = NULL, *if2 = NULL;
	int status;
	struct dvobj_priv *dvobj;

	RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("+rtw_drv_init\n"));

	/* step 0. */
	process_spec_devid(pdid);

	/* Initialize dvobj_priv */
	if ((dvobj = usb_dvobj_init(pusb_intf)) == NULL) {
		RT_TRACE(_module_hci_intfs_c_, _drv_err_, ("initialize device object priv Failed!\n"));
		goto exit;
	}

	#ifdef RTW_DVOBJ_CHIP_HW_TYPE
	decide_chip_type_by_usb_device_id(dvobj, pdid);
	#endif

	if ((if1 = rtw_usb_if1_init(dvobj, pusb_intf, pdid)) == NULL) {
		DBG_88E("rtw_init_primary_adapter Failed!\n");
		goto free_dvobj;
	}
	if (ui_pid[1]!=0) {
		DBG_88E("ui_pid[1]:%d\n",ui_pid[1]);
		rtw_signal_process(ui_pid[1], SIGUSR2);
	}

	/* dev_alloc_name && register_netdev */
	if ((status = rtw_drv_register_netdev(if1)) != _SUCCESS) {
		goto free_if2;
	}

#ifdef CONFIG_HOSTAPD_MLME
	hostapd_mode_init(if1);
#endif

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-871x_drv - drv_init, success!\n"));

	status = _SUCCESS;

free_if2:
free_if1:
	if (status != _SUCCESS && if1) {
		rtw_usb_if1_deinit(if1);
	}
free_dvobj:
	if (status != _SUCCESS)
		usb_dvobj_deinit(pusb_intf);
exit:
	return status == _SUCCESS?0:-ENODEV;
}
/*
 * dev_remove() - our device is being removed
*/
/* rmmod module & unplug(SurpriseRemoved) will call r871xu_dev_remove() => how to recognize both */
static void rtw_dev_remove(struct usb_interface *pusb_intf)
{
	struct dvobj_priv *dvobj = usb_get_intfdata(pusb_intf);
	struct adapter *padapter = dvobj->if1;
	struct net_device *pnetdev = padapter->pnetdev;
	struct mlme_priv *pmlmepriv= &padapter->mlmepriv;

;

	DBG_88E("+rtw_dev_remove\n");
	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+dev_remove()\n"));
	dvobj->processing_dev_remove = true;
	rtw_unregister_netdevs(dvobj);

	if (usb_drv->drv_registered == true) {
		/* DBG_88E("r871xu_dev_remove():padapter->bSurpriseRemoved == true\n"); */
		padapter->bSurpriseRemoved = true;
	}

	rtw_pm_set_ips(padapter, IPS_NONE);
	rtw_pm_set_lps(padapter, PS_MODE_ACTIVE);

	LeaveAllPowerSaveMode(padapter);

	rtw_usb_if1_deinit(padapter);

	usb_dvobj_deinit(pusb_intf);

	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("-dev_remove()\n"));
	DBG_88E("-r871xu_dev_remove, done\n");
}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,24))
extern int console_suspend_enabled;
#endif

static int __init rtw_drv_entry(void)
{
	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+rtw_drv_entry\n"));

	DBG_88E(DRV_NAME " driver version=%s\n", DRIVERVERSION);
	DBG_88E("build time: %s %s\n", __DATE__, __TIME__);

	rtw_suspend_lock_init();

	usb_drv->drv_registered = true;
	return usb_register(&usb_drv->usbdrv);
}

static void __exit rtw_drv_halt(void)
{
	RT_TRACE(_module_hci_intfs_c_,_drv_err_,("+rtw_drv_halt\n"));
	DBG_88E("+rtw_drv_halt\n");

	usb_drv->drv_registered = false;
	usb_deregister(&usb_drv->usbdrv);

	rtw_suspend_lock_uninit();
	DBG_88E("-rtw_drv_halt\n");

	rtw_mstat_dump();
}


module_init(rtw_drv_entry);
module_exit(rtw_drv_halt);
