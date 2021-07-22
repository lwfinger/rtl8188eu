// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


/* ************************************************************
 * include files
 * ************************************************************ */
#include "mp_precomp.h"
#include "phydm_precomp.h"

void
odm_dynamic_tx_power_init(
	void					*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	p_dm_odm->last_dtp_lvl = tx_high_pwr_level_normal;
	p_dm_odm->dynamic_tx_high_power_lvl = tx_high_pwr_level_normal;
	p_dm_odm->tx_agc_ofdm_18_6 = odm_get_bb_reg(p_dm_odm, 0xC24, MASKDWORD); /*TXAGC {18M 12M 9M 6M}*/
}

void
odm_dynamic_tx_power_save_power_index(
	void					*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8		index;
	u32		power_index_reg[6] = {0xc90, 0xc91, 0xc92, 0xc98, 0xc99, 0xc9a};

}

void
odm_dynamic_tx_power_restore_power_index(
	void					*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8			index;
	struct _ADAPTER		*adapter = p_dm_odm->adapter;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(adapter);
	u32			power_index_reg[6] = {0xc90, 0xc91, 0xc92, 0xc98, 0xc99, 0xc9a};
}

void
odm_dynamic_tx_power_write_power_index(
	void					*p_dm_void,
	u8		value)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8			index;
	u32			power_index_reg[6] = {0xc90, 0xc91, 0xc92, 0xc98, 0xc99, 0xc9a};

	for (index = 0; index < 6; index++)
		/* platform_efio_write_1byte(adapter, power_index_reg[index], value); */
		odm_write_1byte(p_dm_odm, power_index_reg[index], value);

}

static void
odm_dynamic_tx_power_nic_ce(
	void					*p_dm_void
)
{
}

void
odm_dynamic_tx_power(
	void					*p_dm_void
)
{
	/*  */
	/* For AP/ADSL use struct rtl8192cd_priv* */
	/* For CE/NIC use struct _ADAPTER* */
	/*  */
	/* struct _ADAPTER*		p_adapter = p_dm_odm->adapter;
	*	struct rtl8192cd_priv*	priv		= p_dm_odm->priv; */
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	if (!(p_dm_odm->support_ability & ODM_BB_DYNAMIC_TXPWR))
		return;
	/*  */
	/* 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate */
	/* at the same time. In the stage2/3, we need to prive universal interface and merge all */
	/* HW dynamic mechanism. */
	/*  */
	switch	(p_dm_odm->support_platform) {
	case	ODM_WIN:
		odm_dynamic_tx_power_nic(p_dm_odm);
		break;
	case	ODM_CE:
		odm_dynamic_tx_power_nic_ce(p_dm_odm);
		break;
	case	ODM_AP:
		odm_dynamic_tx_power_ap(p_dm_odm);
		break;
	default:
		break;
	}


}


void
odm_dynamic_tx_power_nic(
	void					*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	if (!(p_dm_odm->support_ability & ODM_BB_DYNAMIC_TXPWR))
		return;
}

void
odm_dynamic_tx_power_ap(
	void					*p_dm_void

)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
}

void
odm_dynamic_tx_power_8821(
	void			*p_dm_void,
	u8			*p_desc,
	u8			mac_id
)
{
}
