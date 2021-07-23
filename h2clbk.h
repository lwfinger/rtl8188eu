/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */



#define _H2CLBK_H_


void _lbk_cmd(PADAPTER Adapter);

void _lbk_rsp(PADAPTER Adapter);

void _lbk_evt(IN PADAPTER Adapter);

void h2c_event_callback(unsigned char *dev, unsigned char *pbuf);
