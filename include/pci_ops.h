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
#ifndef __PCI_OPS_H_
#define __PCI_OPS_H_

#include <drv_conf.h>
#include <osdep_service.h>
#include <drv_types.h>
#include <osdep_intf.h>


u32	rtl8188ee_init_desc_ring(struct adapter * padapter);
u32	rtl8188ee_free_desc_ring(struct adapter * padapter);
void	rtl8188ee_reset_desc_ring(struct adapter * padapter);
#ifdef CONFIG_64BIT_DMA
u8	PlatformEnable88EEDMA64(struct adapter *Adapter);
#endif
int	rtl8188ee_interrupt(struct adapter *Adapter);
void	rtl8188ee_xmit_tasklet(void *priv);
void	rtl8188ee_recv_tasklet(void *priv);
void	rtl8188ee_prepare_bcn_tasklet(void *priv);
void	rtl8188ee_set_intf_ops(struct _io_ops	*pops);
#define pci_set_intf_ops	rtl8188ee_set_intf_ops

#endif

