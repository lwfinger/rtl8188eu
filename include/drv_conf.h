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
#ifndef __DRV_CONF_H__
#define __DRV_CONF_H__
#include "autoconf.h"

//About USB VENDOR REQ
#if defined(CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC) && !defined(CONFIG_USB_VENDOR_REQ_MUTEX)
	#warning "define CONFIG_USB_VENDOR_REQ_MUTEX for CONFIG_USB_VENDOR_REQ_BUFFER_PREALLOC automatically"
	#define CONFIG_USB_VENDOR_REQ_MUTEX
#endif
#if defined(CONFIG_VENDOR_REQ_RETRY) &&  !defined(CONFIG_USB_VENDOR_REQ_MUTEX)
	#warning "define CONFIG_USB_VENDOR_REQ_MUTEX for CONFIG_VENDOR_REQ_RETRY automatically"
	#define CONFIG_USB_VENDOR_REQ_MUTEX
#endif

#endif // __DRV_CONF_H__
