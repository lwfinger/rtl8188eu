/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#ifndef _RTW_SDIO_H_
#define _RTW_SDIO_H_

#include <drv_types.h>		/* struct dvobj_priv and etc. */

u8 rtw_sdio_read_cmd52(struct dvobj_priv *, u32 addr, void *buf, size_t len);
u8 rtw_sdio_read_cmd53(struct dvobj_priv *, u32 addr, void *buf, size_t len);
u8 rtw_sdio_write_cmd52(struct dvobj_priv *, u32 addr, void *buf, size_t len);
u8 rtw_sdio_write_cmd53(struct dvobj_priv *, u32 addr, void *buf, size_t len);
u8 rtw_sdio_f0_read(struct dvobj_priv *, u32 addr, void *buf, size_t len);

#endif /* _RTW_SDIO_H_ */
