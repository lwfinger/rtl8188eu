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

#ifndef	__PHYDM_FEATURES_H__
#define __PHYDM_FEATURES

#define	ODM_RECEIVER_BLOCKING_SUPPORT	(ODM_RTL8188E | ODM_RTL8192E)
#define PHYDM_LA_MODE_SUPPORT			0

/*phydm debyg report & tools*/
#define CONFIG_PHYDM_DEBUG_FUNCTION		1

#define	CONFIG_DYNAMIC_RX_PATH	0

#define PHYDM_SUPPORT_EDCA		1
#define SUPPORTABLITY_PHYDMLIZE	1
#define RA_MASK_PHYDMLIZE_CE	1

/*Antenna Diversity*/
#ifdef CONFIG_ANTENNA_DIVERSITY
	#define CONFIG_PHYDM_ANTENNA_DIVERSITY
#endif

#ifdef CONFIG_DFS_MASTER
	#define CONFIG_PHYDM_DFS_MASTER
#endif

#define	CONFIG_RECEIVER_BLOCKING
#define	CONFIG_RA_FW_DBG_CODE	0
#define	CONFIG_BB_POWER_SAVING
#define	CONFIG_BB_TXBF_API

#ifdef CONFIG_BT_COEXIST
	#define BT_SUPPORT      1
#endif

#endif
