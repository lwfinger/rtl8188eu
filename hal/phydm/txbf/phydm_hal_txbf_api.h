/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */

#ifndef	__PHYDM_HAL_TXBF_API_H__
#define __PHYDM_HAL_TXBF_API_H__

#if (defined(CONFIG_BB_TXBF_API))

#define tx_bf_nr(a, b) ((a > b) ? (b) : (a))

u8
beamforming_get_htndp_tx_rate(
	void	*p_dm_void,
	u8	comp_steering_num_of_bfer
);

u8
beamforming_get_vht_ndp_tx_rate(
	void	*p_dm_void,
	u8	comp_steering_num_of_bfer
);

#define phydm_get_beamforming_sounding_info(p_dm_void, troughput, total_bfee_num, tx_rate)
#define phydm_get_ndpa_rate(p_dm_void)
#define phydm_get_mu_bfee_snding_decision(p_dm_void, troughput)

#endif
#endif
