// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


/* ************************************************************
 * include files
 * ************************************************************ */
#include "mp_precomp.h"
#include "phydm_precomp.h"

void
phydm_check_adaptivity(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ADAPTIVITY_STATISTICS	*adaptivity = (struct _ADAPTIVITY_STATISTICS *)phydm_get_structure(p_dm_odm, PHYDM_ADAPTIVITY);

	if (p_dm_odm->support_ability & ODM_BB_ADAPTIVITY) {
		{
			if (adaptivity->dynamic_link_adaptivity || adaptivity->acs_for_adaptivity) {
				if (p_dm_odm->is_linked && adaptivity->is_check == false) {
					phydm_nhm_counter_statistics(p_dm_odm);
					phydm_check_environment(p_dm_odm);
				} else if (!p_dm_odm->is_linked)
					adaptivity->is_check = false;
			} else {
				p_dm_odm->adaptivity_enable = true;

				if (p_dm_odm->support_ic_type & (ODM_IC_11AC_GAIN_IDX_EDCCA | ODM_IC_11N_GAIN_IDX_EDCCA))
					p_dm_odm->adaptivity_flag = false;
				else
					p_dm_odm->adaptivity_flag = true;
			}
		}
	} else {
		p_dm_odm->adaptivity_enable = false;
		p_dm_odm->adaptivity_flag = false;
	}
}

void
phydm_nhm_counter_statistics_init(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES) {
		/*PHY parameters initialize for n series*/
		odm_write_2byte(p_dm_odm, ODM_REG_CCX_PERIOD_11N + 2, 0xC350);			/*0x894[31:16]=0x0xC350	Time duration for NHM unit: us, 0xc350=200ms*/
		odm_write_2byte(p_dm_odm, ODM_REG_NHM_TH9_TH10_11N + 2, 0xffff);		/*0x890[31:16]=0xffff		th_9, th_10*/
		odm_write_4byte(p_dm_odm, ODM_REG_NHM_TH3_TO_TH0_11N, 0xffffff50);		/*0x898=0xffffff52			th_3, th_2, th_1, th_0*/
		odm_write_4byte(p_dm_odm, ODM_REG_NHM_TH7_TO_TH4_11N, 0xffffffff);		/*0x89c=0xffffffff			th_7, th_6, th_5, th_4*/
		odm_set_bb_reg(p_dm_odm, ODM_REG_FPGA0_IQK_11N, MASKBYTE0, 0xff);		/*0xe28[7:0]=0xff			th_8*/
		odm_set_bb_reg(p_dm_odm, ODM_REG_NHM_TH9_TH10_11N, BIT(10) | BIT9 | BIT8, 0x1);	/*0x890[10:8]=1			ignoreCCA ignore PHYTXON enable CCX*/
		odm_set_bb_reg(p_dm_odm, ODM_REG_OFDM_FA_RSTC_11N, BIT(7), 0x1);			/*0xc0c[7]=1				max power among all RX ants*/
	}
	else if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES) {
		/*PHY parameters initialize for ac series*/
		odm_write_2byte(p_dm_odm, ODM_REG_CCX_PERIOD_11AC + 2, 0xC350);			/*0x990[31:16]=0xC350	Time duration for NHM unit: us, 0xc350=200ms*/
		odm_write_2byte(p_dm_odm, ODM_REG_NHM_TH9_TH10_11AC + 2, 0xffff);		/*0x994[31:16]=0xffff		th_9, th_10*/
		odm_write_4byte(p_dm_odm, ODM_REG_NHM_TH3_TO_TH0_11AC, 0xffffff50);	/*0x998=0xffffff52			th_3, th_2, th_1, th_0*/
		odm_write_4byte(p_dm_odm, ODM_REG_NHM_TH7_TO_TH4_11AC, 0xffffffff);	/*0x99c=0xffffffff			th_7, th_6, th_5, th_4*/
		odm_set_bb_reg(p_dm_odm, ODM_REG_NHM_TH8_11AC, MASKBYTE0, 0xff);		/*0x9a0[7:0]=0xff			th_8*/
		odm_set_bb_reg(p_dm_odm, ODM_REG_NHM_TH9_TH10_11AC, BIT(8) | BIT9 | BIT10, 0x1); /*0x994[10:8]=1			ignoreCCA ignore PHYTXON	enable CCX*/
		odm_set_bb_reg(p_dm_odm, ODM_REG_NHM_9E8_11AC, BIT(0), 0x1);				/*0x9e8[7]=1				max power among all RX ants*/

	}
}

void
phydm_nhm_counter_statistics(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	if (!(p_dm_odm->support_ability & ODM_BB_NHM_CNT))
		return;

	/*Get NHM report*/
	phydm_get_nhm_counter_statistics(p_dm_odm);

	/*Reset NHM counter*/
	phydm_nhm_counter_statistics_reset(p_dm_odm);
}

void
phydm_get_nhm_counter_statistics(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32		value32 = 0;
	if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES)
		value32 = odm_get_bb_reg(p_dm_odm, ODM_REG_NHM_CNT_11AC, MASKDWORD);
	else if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES)
		value32 = odm_get_bb_reg(p_dm_odm, ODM_REG_NHM_CNT_11N, MASKDWORD);

	p_dm_odm->nhm_cnt_0 = (u8)(value32 & MASKBYTE0);
	p_dm_odm->nhm_cnt_1 = (u8)((value32 & MASKBYTE1) >> 8);

}

void
phydm_nhm_counter_statistics_reset(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES) {
		odm_set_bb_reg(p_dm_odm, ODM_REG_NHM_TH9_TH10_11N, BIT(1), 0);
		odm_set_bb_reg(p_dm_odm, ODM_REG_NHM_TH9_TH10_11N, BIT(1), 1);
	} else if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES) {
		odm_set_bb_reg(p_dm_odm, ODM_REG_NHM_TH9_TH10_11AC, BIT(1), 0);
		odm_set_bb_reg(p_dm_odm, ODM_REG_NHM_TH9_TH10_11AC, BIT(1), 1);
	}
}

void
phydm_set_edcca_threshold(
	void	*p_dm_void,
	s8	H2L,
	s8	L2H
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES)
		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_ECCA_THRESHOLD, MASKBYTE2 | MASKBYTE0, (u32)((u8)L2H | (u8)H2L << 16));
	else if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES)
		odm_set_bb_reg(p_dm_odm, REG_FPGA0_XB_LSSI_READ_BACK, MASKLWORD, (u16)((u8)L2H | (u8)H2L << 8));
}

static void
phydm_set_lna(
	void				*p_dm_void,
	enum phydm_set_lna	type
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	if (p_dm_odm->support_ic_type & (ODM_RTL8188E | ODM_RTL8192E)) {
		if (type == phydm_disable_lna) {
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x0000f);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0x37f82);	/*disable LNA*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);
			if (p_dm_odm->rf_type > ODM_1T1R) {
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x1);
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0x30, 0xfffff, 0x18000);
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0x31, 0xfffff, 0x0000f);
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0x32, 0xfffff, 0x37f82);
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x0);
			}
		} else if (type == phydm_enable_lna) {
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x0000f);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0x77f82);	/*back to normal*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);
			if (p_dm_odm->rf_type > ODM_1T1R) {
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x1);
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0x30, 0xfffff, 0x18000);
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0x31, 0xfffff, 0x0000f);
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0x32, 0xfffff, 0x77f82);
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x0);
			}
		}
	} else if (p_dm_odm->support_ic_type & ODM_RTL8723B) {
		if (type == phydm_disable_lna) {
			/*S0*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x0001f);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0xe6137);	/*disable LNA*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);
			/*S1*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xed, 0x00020, 0x1);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x43, 0xfffff, 0x3008d);	/*select Rx mode and disable LNA*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xed, 0x00020, 0x0);
		} else if (type == phydm_enable_lna) {
			/*S0*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x0001f);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0xe6177);	/*disable LNA*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);
			/*S1*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xed, 0x00020, 0x1);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x43, 0xfffff, 0x300bd);	/*select Rx mode and disable LNA*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xed, 0x00020, 0x0);
		}

	} else if (p_dm_odm->support_ic_type & ODM_RTL8812) {
		if (type == phydm_disable_lna) {
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x3f7ff);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0xc22bf);	/*disable LNA*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);
			if (p_dm_odm->rf_type > ODM_1T1R) {
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x1);
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0x31, 0xfffff, 0x3f7ff);
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0x32, 0xfffff, 0xc22bf);	/*disable LNA*/
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x0);
			}
		} else if (type == phydm_enable_lna) {
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x3f7ff);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0xc26bf);	/*disable LNA*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);
			if (p_dm_odm->rf_type > ODM_1T1R) {
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x1);
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0x31, 0xfffff, 0x3f7ff);
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0x32, 0xfffff, 0xc26bf);	/*disable LNA*/
				odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_B, 0xef, 0x80000, 0x0);
			}
		}
	} else if (p_dm_odm->support_ic_type & (ODM_RTL8821 | ODM_RTL8881A)) {
		if (type == phydm_disable_lna) {
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x0002f);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0xfb09b);	/*disable LNA*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);
		} else if (type == phydm_enable_lna) {
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x1);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x30, 0xfffff, 0x18000);	/*select Rx mode*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x31, 0xfffff, 0x0002f);
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0x32, 0xfffff, 0xfb0bb);	/*disable LNA*/
			odm_set_rf_reg(p_dm_odm, ODM_RF_PATH_A, 0xef, 0x80000, 0x0);
		}
	}
}



void
phydm_set_trx_mux(
	void				*p_dm_void,
	enum phydm_trx_mux_type	tx_mode,
	enum phydm_trx_mux_type	rx_mode
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES) {
		odm_set_bb_reg(p_dm_odm, ODM_REG_CCK_RPT_FORMAT_11N, BIT(3) | BIT2 | BIT1, tx_mode);			/*set TXmod to standby mode to remove outside noise affect*/
		odm_set_bb_reg(p_dm_odm, ODM_REG_CCK_RPT_FORMAT_11N, BIT(22) | BIT21 | BIT20, rx_mode);		/*set RXmod to standby mode to remove outside noise affect*/
		if (p_dm_odm->rf_type > ODM_1T1R) {
			odm_set_bb_reg(p_dm_odm, ODM_REG_CCK_RPT_FORMAT_11N_B, BIT(3) | BIT2 | BIT1, tx_mode);		/*set TXmod to standby mode to remove outside noise affect*/
			odm_set_bb_reg(p_dm_odm, ODM_REG_CCK_RPT_FORMAT_11N_B, BIT(22) | BIT21 | BIT20, rx_mode);	/*set RXmod to standby mode to remove outside noise affect*/
		}
	} else if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES) {
		odm_set_bb_reg(p_dm_odm, ODM_REG_TRMUX_11AC, BIT(11) | BIT10 | BIT9 | BIT8, tx_mode);				/*set TXmod to standby mode to remove outside noise affect*/
		odm_set_bb_reg(p_dm_odm, ODM_REG_TRMUX_11AC, BIT(7) | BIT6 | BIT5 | BIT4, rx_mode);				/*set RXmod to standby mode to remove outside noise affect*/
		if (p_dm_odm->rf_type > ODM_1T1R) {
			odm_set_bb_reg(p_dm_odm, ODM_REG_TRMUX_11AC_B, BIT(11) | BIT10 | BIT9 | BIT8, tx_mode);		/*set TXmod to standby mode to remove outside noise affect*/
			odm_set_bb_reg(p_dm_odm, ODM_REG_TRMUX_11AC_B, BIT(7) | BIT6 | BIT5 | BIT4, rx_mode);			/*set RXmod to standby mode to remove outside noise affect*/
		}
	}
}

void
phydm_mac_edcca_state(
	void					*p_dm_void,
	enum phydm_mac_edcca_type		state
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	if (state == phydm_ignore_edcca) {
		odm_set_mac_reg(p_dm_odm, REG_TX_PTCL_CTRL, BIT(15), 1);	/*ignore EDCCA	reg520[15]=1*/
		/*		odm_set_mac_reg(p_dm_odm, REG_RD_CTRL, BIT(11), 0);			*/ /*reg524[11]=0*/
	} else {	/*don't set MAC ignore EDCCA signal*/
		odm_set_mac_reg(p_dm_odm, REG_TX_PTCL_CTRL, BIT(15), 0);	/*don't ignore EDCCA	 reg520[15]=0*/
		/*		odm_set_mac_reg(p_dm_odm, REG_RD_CTRL, BIT(11), 1);			*/ /*reg524[11]=1	*/
	}
	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("EDCCA enable state = %d\n", state));

}

bool
phydm_cal_nhm_cnt(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u16			base = 0;

	base = p_dm_odm->nhm_cnt_0 + p_dm_odm->nhm_cnt_1;

	if (base != 0) {
		p_dm_odm->nhm_cnt_0 = ((p_dm_odm->nhm_cnt_0) << 8) / base;
		p_dm_odm->nhm_cnt_1 = ((p_dm_odm->nhm_cnt_1) << 8) / base;
	}
	if ((p_dm_odm->nhm_cnt_0 - p_dm_odm->nhm_cnt_1) >= 100)
		return true;			/*clean environment*/
	else
		return false;		/*noisy environment*/

}


void
phydm_check_environment(
	void	*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ADAPTIVITY_STATISTICS	*adaptivity = (struct _ADAPTIVITY_STATISTICS *)phydm_get_structure(p_dm_odm, PHYDM_ADAPTIVITY);
	bool	is_clean_environment = false;

	if (adaptivity->is_first_link == true) {
		if (p_dm_odm->support_ic_type & (ODM_IC_11AC_GAIN_IDX_EDCCA | ODM_IC_11N_GAIN_IDX_EDCCA))
			p_dm_odm->adaptivity_flag = false;
		else
			p_dm_odm->adaptivity_flag = true;

		adaptivity->is_first_link = false;
		return;
	} else {
		if (adaptivity->nhm_wait < 3) {		/*Start enter NHM after 4 nhm_wait*/
			adaptivity->nhm_wait++;
			phydm_nhm_counter_statistics(p_dm_odm);
			return;
		} else {
			phydm_nhm_counter_statistics(p_dm_odm);
			is_clean_environment = phydm_cal_nhm_cnt(p_dm_odm);
			if (is_clean_environment == true) {
				p_dm_odm->th_l2h_ini = adaptivity->th_l2h_ini_backup;			/*adaptivity mode*/
				p_dm_odm->th_edcca_hl_diff = adaptivity->th_edcca_hl_diff_backup;

				p_dm_odm->adaptivity_enable = true;

				if (p_dm_odm->support_ic_type & (ODM_IC_11AC_GAIN_IDX_EDCCA | ODM_IC_11N_GAIN_IDX_EDCCA))
					p_dm_odm->adaptivity_flag = false;
				else
					p_dm_odm->adaptivity_flag = true;
			} else {
				if (!adaptivity->acs_for_adaptivity) {
					p_dm_odm->th_l2h_ini = p_dm_odm->th_l2h_ini_mode2;			/*mode2*/
					p_dm_odm->th_edcca_hl_diff = p_dm_odm->th_edcca_hl_diff_mode2;

					p_dm_odm->adaptivity_flag = false;
					p_dm_odm->adaptivity_enable = false;
				}
			}
			adaptivity->nhm_wait = 0;
			adaptivity->is_first_link = true;
			adaptivity->is_check = true;
		}

	}


}

void
phydm_search_pwdb_lower_bound(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ADAPTIVITY_STATISTICS	*adaptivity = (struct _ADAPTIVITY_STATISTICS *)phydm_get_structure(p_dm_odm, PHYDM_ADAPTIVITY);
	u32			value32 = 0, reg_value32 = 0;
	u8			cnt, try_count = 0;
	u8			tx_edcca1 = 0, tx_edcca0 = 0;
	bool			is_adjust = true;
	s8			th_l2h_dmc, th_h2l_dmc, igi_target = 0x32;
	s8			diff;
	u8			IGI = adaptivity->igi_base + 30 + (u8)p_dm_odm->th_l2h_ini - (u8)p_dm_odm->th_edcca_hl_diff;

	if (p_dm_odm->support_ic_type & (ODM_RTL8723B | ODM_RTL8188E | ODM_RTL8192E | ODM_RTL8812 | ODM_RTL8821 | ODM_RTL8881A))
		phydm_set_lna(p_dm_odm, phydm_disable_lna);
	else {
		phydm_set_trx_mux(p_dm_odm, phydm_standby_mode, phydm_standby_mode);
		odm_pause_dig(p_dm_odm, PHYDM_PAUSE, PHYDM_PAUSE_LEVEL_0, 0x7e);
	}

	diff = igi_target - (s8)IGI;
	th_l2h_dmc = p_dm_odm->th_l2h_ini + diff;
	if (th_l2h_dmc > 10)
		th_l2h_dmc = 10;
	th_h2l_dmc = th_l2h_dmc - p_dm_odm->th_edcca_hl_diff;

	phydm_set_edcca_threshold(p_dm_odm, th_h2l_dmc, th_l2h_dmc);
	ODM_delay_ms(30);

	while (is_adjust) {
		if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES) {
			odm_set_bb_reg(p_dm_odm, ODM_REG_DBG_RPT_11N, MASKDWORD, 0x0);
			reg_value32 = odm_get_bb_reg(p_dm_odm, ODM_REG_RPT_11N, MASKDWORD);
		} else if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES) {
			odm_set_bb_reg(p_dm_odm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0x0);
			reg_value32 = odm_get_bb_reg(p_dm_odm, ODM_REG_RPT_11AC, MASKDWORD);
		}
		while (reg_value32 & BIT(3) && try_count < 3) {
			try_count = try_count + 1;
			ODM_delay_ms(3);
			if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES)
				reg_value32 = odm_get_bb_reg(p_dm_odm, ODM_REG_RPT_11N, MASKDWORD);
			else if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES)
				reg_value32 = odm_get_bb_reg(p_dm_odm, ODM_REG_RPT_11AC, MASKDWORD);
		}
		try_count = 0;

		for (cnt = 0; cnt < 20; cnt++) {
			if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES) {
				odm_set_bb_reg(p_dm_odm, ODM_REG_DBG_RPT_11N, MASKDWORD, 0x208);
				value32 = odm_get_bb_reg(p_dm_odm, ODM_REG_RPT_11N, MASKDWORD);
			} else if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES) {
				odm_set_bb_reg(p_dm_odm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0x209);
				value32 = odm_get_bb_reg(p_dm_odm, ODM_REG_RPT_11AC, MASKDWORD);
			}
			if (value32 & BIT(30) && (p_dm_odm->support_ic_type & (ODM_RTL8723B | ODM_RTL8188E)))
				tx_edcca1 = tx_edcca1 + 1;
			else if (value32 & BIT(29))
				tx_edcca1 = tx_edcca1 + 1;
			else
				tx_edcca0 = tx_edcca0 + 1;
		}

		if (tx_edcca1 > 1) {
			IGI = IGI - 1;
			th_l2h_dmc = th_l2h_dmc + 1;
			if (th_l2h_dmc > 10)
				th_l2h_dmc = 10;
			th_h2l_dmc = th_l2h_dmc - p_dm_odm->th_edcca_hl_diff;

			phydm_set_edcca_threshold(p_dm_odm, th_h2l_dmc, th_l2h_dmc);
			if (th_l2h_dmc == 10) {
				is_adjust = false;
				adaptivity->h2l_lb = th_h2l_dmc;
				adaptivity->l2h_lb = th_l2h_dmc;
				p_dm_odm->adaptivity_igi_upper = IGI;
			}

			tx_edcca1 = 0;
			tx_edcca0 = 0;

		} else {
			is_adjust = false;
			adaptivity->h2l_lb = th_h2l_dmc;
			adaptivity->l2h_lb = th_l2h_dmc;
			p_dm_odm->adaptivity_igi_upper = IGI;
			tx_edcca1 = 0;
			tx_edcca0 = 0;
		}
	}

	p_dm_odm->adaptivity_igi_upper = p_dm_odm->adaptivity_igi_upper - p_dm_odm->dc_backoff;
	adaptivity->h2l_lb = adaptivity->h2l_lb + p_dm_odm->dc_backoff;
	adaptivity->l2h_lb = adaptivity->l2h_lb + p_dm_odm->dc_backoff;

	if (p_dm_odm->support_ic_type & (ODM_RTL8723B | ODM_RTL8188E | ODM_RTL8192E | ODM_RTL8812 | ODM_RTL8821 | ODM_RTL8881A))
		phydm_set_lna(p_dm_odm, phydm_enable_lna);
	else {
		phydm_set_trx_mux(p_dm_odm, phydm_tx_mode, phydm_rx_mode);
		odm_pause_dig(p_dm_odm, PHYDM_RESUME, PHYDM_PAUSE_LEVEL_0, NONE);
	}

	phydm_set_edcca_threshold(p_dm_odm, 0x7f, 0x7f);				/*resume to no link state*/
}

static bool
phydm_re_search_condition(
	void				*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	/*struct _ADAPTIVITY_STATISTICS*	adaptivity = (struct _ADAPTIVITY_STATISTICS*)phydm_get_structure(p_dm_odm, PHYDM_ADAPTIVITY);*/
	u8			adaptivity_igi_upper;
	u8			count = 0;
	/*s8		TH_L2H_dmc, IGI_target = 0x32;*/
	/*s8		diff;*/

	adaptivity_igi_upper = p_dm_odm->adaptivity_igi_upper + p_dm_odm->dc_backoff;

	/*TH_L2H_dmc = 10;*/

	/*diff = TH_L2H_dmc - p_dm_odm->TH_L2H_ini;*/
	/*lowest_IGI_upper = IGI_target - diff;*/

	/*if ((adaptivity_igi_upper - lowest_IGI_upper) <= 5)*/
	if (adaptivity_igi_upper <= 0x26 && count < 3) {
		count = count + 1;
		return true;
	}
	else
		return false;

}

void
phydm_adaptivity_info_init(
	void				*p_dm_void,
	enum phydm_adapinfo_e	cmn_info,
	u32				value
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ADAPTIVITY_STATISTICS	*adaptivity = (struct _ADAPTIVITY_STATISTICS *)phydm_get_structure(p_dm_odm, PHYDM_ADAPTIVITY);

	switch (cmn_info)	{
	case PHYDM_ADAPINFO_CARRIER_SENSE_ENABLE:
		p_dm_odm->carrier_sense_enable = (bool)value;
		break;

	case PHYDM_ADAPINFO_DCBACKOFF:
		p_dm_odm->dc_backoff = (u8)value;
		break;

	case PHYDM_ADAPINFO_DYNAMICLINKADAPTIVITY:
		adaptivity->dynamic_link_adaptivity = (bool)value;
		break;

	case PHYDM_ADAPINFO_TH_L2H_INI:
		p_dm_odm->th_l2h_ini = (s8)value;
		break;

	case PHYDM_ADAPINFO_TH_EDCCA_HL_DIFF:
		p_dm_odm->th_edcca_hl_diff = (s8)value;
		break;

	case PHYDM_ADAPINFO_AP_NUM_TH:
		adaptivity->ap_num_th = (u8)value;
		break;

	default:
		break;

	}

}



void
phydm_adaptivity_init(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ADAPTIVITY_STATISTICS	*adaptivity = (struct _ADAPTIVITY_STATISTICS *)phydm_get_structure(p_dm_odm, PHYDM_ADAPTIVITY);
	s8	igi_target = 0x32;

	if (p_dm_odm->carrier_sense_enable == false) {
		if (p_dm_odm->th_l2h_ini == 0)
			p_dm_odm->th_l2h_ini = 0xf5;
	} else
		p_dm_odm->th_l2h_ini = 0xa;

	if (p_dm_odm->th_edcca_hl_diff == 0)
		p_dm_odm->th_edcca_hl_diff = 7;
	if (p_dm_odm->wifi_test == true || p_dm_odm->mp_mode == true)
		p_dm_odm->edcca_enable = false;		/*even no adaptivity, we still enable EDCCA, AP side use mib control*/
	else
		p_dm_odm->edcca_enable = true;

	p_dm_odm->adaptivity_igi_upper = 0;
	p_dm_odm->adaptivity_enable = false;	/*use this flag to decide enable or disable*/

	p_dm_odm->th_l2h_ini_mode2 = 20;
	p_dm_odm->th_edcca_hl_diff_mode2 = 8;
	adaptivity->th_l2h_ini_backup = p_dm_odm->th_l2h_ini;
	adaptivity->th_edcca_hl_diff_backup = p_dm_odm->th_edcca_hl_diff;

	adaptivity->igi_base = 0x32;
	adaptivity->igi_target = 0x1c;
	adaptivity->h2l_lb = 0;
	adaptivity->l2h_lb = 0;
	adaptivity->nhm_wait = 0;
	adaptivity->is_check = false;
	adaptivity->is_first_link = true;
	adaptivity->adajust_igi_level = 0;
	adaptivity->is_stop_edcca = false;
	adaptivity->backup_h2l = 0;
	adaptivity->backup_l2h = 0;

	phydm_mac_edcca_state(p_dm_odm, phydm_dont_ignore_edcca);

	/*Search pwdB lower bound*/
	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES)
		odm_set_bb_reg(p_dm_odm, ODM_REG_DBG_RPT_11N, MASKDWORD, 0x208);
	else if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES)
		odm_set_bb_reg(p_dm_odm, ODM_REG_DBG_RPT_11AC, MASKDWORD, 0x209);

	if (p_dm_odm->support_ic_type & ODM_IC_11N_GAIN_IDX_EDCCA) {
		/*odm_set_bb_reg(p_dm_odm, ODM_REG_EDCCA_DOWN_OPT_11N, BIT(12) | BIT11 | BIT10, 0x7);*/		/*interfernce need > 2^x us, and then EDCCA will be 1*/
		if (p_dm_odm->support_ic_type & ODM_RTL8197F) {
			odm_set_bb_reg(p_dm_odm, ODM_REG_PAGE_B1_97F, BIT(30), 0x1);								/*set to page B1*/
			odm_set_bb_reg(p_dm_odm, ODM_REG_EDCCA_DCNF_97F, BIT(27) | BIT26, 0x1);		/*0:rx_dfir, 1: dcnf_out, 2 :rx_iq, 3: rx_nbi_nf_out*/
			odm_set_bb_reg(p_dm_odm, ODM_REG_PAGE_B1_97F, BIT(30), 0x0);
		} else
			odm_set_bb_reg(p_dm_odm, ODM_REG_EDCCA_DCNF_11N, BIT(21) | BIT20, 0x1);		/*0:rx_dfir, 1: dcnf_out, 2 :rx_iq, 3: rx_nbi_nf_out*/
	}
	if (p_dm_odm->support_ic_type & ODM_IC_11AC_GAIN_IDX_EDCCA) {		/*8814a no need to find pwdB lower bound, maybe*/
		/*odm_set_bb_reg(p_dm_odm, ODM_REG_EDCCA_DOWN_OPT, BIT(30) | BIT29 | BIT28, 0x7);*/		/*interfernce need > 2^x us, and then EDCCA will be 1*/
		odm_set_bb_reg(p_dm_odm, ODM_REG_ACBB_EDCCA_ENHANCE, BIT(29) | BIT28, 0x1);		/*0:rx_dfir, 1: dcnf_out, 2 :rx_iq, 3: rx_nbi_nf_out*/
	}

	if (!(p_dm_odm->support_ic_type & (ODM_IC_11AC_GAIN_IDX_EDCCA | ODM_IC_11N_GAIN_IDX_EDCCA))) {
		phydm_search_pwdb_lower_bound(p_dm_odm);
		if (phydm_re_search_condition(p_dm_odm))
			phydm_search_pwdb_lower_bound(p_dm_odm);
	}

	/*we need to consider PwdB upper bound for 8814 later IC*/
	adaptivity->adajust_igi_level = (u8)((p_dm_odm->th_l2h_ini + igi_target) - pwdb_upper_bound + dfir_loss);	/*IGI = L2H - PwdB - dfir_loss*/

	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("th_l2h_ini = 0x%x, th_edcca_hl_diff = 0x%x, adaptivity->adajust_igi_level = 0x%x\n", p_dm_odm->th_l2h_ini, p_dm_odm->th_edcca_hl_diff, adaptivity->adajust_igi_level));

	/*Check this later on Windows*/
	/*phydm_set_edcca_threshold_api(p_dm_odm, p_dm_dig_table->cur_ig_value);*/

}


void
phydm_adaptivity(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _dynamic_initial_gain_threshold_			*p_dm_dig_table = &p_dm_odm->dm_dig_table;
	u8			IGI = p_dm_dig_table->cur_ig_value;
	s8			th_l2h_dmc, th_h2l_dmc;
	s8			diff = 0, igi_target;
	struct _ADAPTIVITY_STATISTICS	*adaptivity = (struct _ADAPTIVITY_STATISTICS *)phydm_get_structure(p_dm_odm, PHYDM_ADAPTIVITY);

	if ((p_dm_odm->edcca_enable == false) || (adaptivity->is_stop_edcca == true)) {
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("Disable EDCCA!!!\n"));
		return;
	}

	if (!(p_dm_odm->support_ability & ODM_BB_ADAPTIVITY)) {
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("adaptivity disable, enable EDCCA mode!!!\n"));
		p_dm_odm->th_l2h_ini = p_dm_odm->th_l2h_ini_mode2;
		p_dm_odm->th_edcca_hl_diff = p_dm_odm->th_edcca_hl_diff_mode2;
	}
	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("odm_Adaptivity() =====>\n"));
	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("igi_base=0x%x, th_l2h_ini = %d, th_edcca_hl_diff = %d\n",
		adaptivity->igi_base, p_dm_odm->th_l2h_ini, p_dm_odm->th_edcca_hl_diff));
	if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES) {
		/*fix AC series when enable EDCCA hang issue*/
		odm_set_bb_reg(p_dm_odm, 0x800, BIT(10), 1);	/*ADC_mask disable*/
		odm_set_bb_reg(p_dm_odm, 0x800, BIT(10), 0);	/*ADC_mask enable*/
	}
	if (*p_dm_odm->p_band_width == ODM_BW20M)		/*CHANNEL_WIDTH_20*/
		igi_target = adaptivity->igi_base;
	else if (*p_dm_odm->p_band_width == ODM_BW40M)
		igi_target = adaptivity->igi_base + 2;
	else if (*p_dm_odm->p_band_width == ODM_BW80M)
		igi_target = adaptivity->igi_base + 2;
	else
		igi_target = adaptivity->igi_base;
	adaptivity->igi_target = (u8) igi_target;

	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("band_width=%s, igi_target=0x%x, dynamic_link_adaptivity = %d, acs_for_adaptivity = %d\n",
		(*p_dm_odm->p_band_width == ODM_BW80M) ? "80M" : ((*p_dm_odm->p_band_width == ODM_BW40M) ? "40M" : "20M"), igi_target, adaptivity->dynamic_link_adaptivity, adaptivity->acs_for_adaptivity));
	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("RSSI_min = %d, adaptivity->adajust_igi_level= 0x%x, adaptivity_flag = %d, adaptivity_enable = %d\n",
		p_dm_odm->rssi_min, adaptivity->adajust_igi_level, p_dm_odm->adaptivity_flag, p_dm_odm->adaptivity_enable));

	if ((adaptivity->dynamic_link_adaptivity == true) && (!p_dm_odm->is_linked) && (p_dm_odm->adaptivity_enable == false)) {
		phydm_set_edcca_threshold(p_dm_odm, 0x7f, 0x7f);
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("In DynamicLink mode(noisy) and No link, Turn off EDCCA!!\n"));
		return;
	}

	if (p_dm_odm->support_ic_type & (ODM_IC_11AC_GAIN_IDX_EDCCA | ODM_IC_11N_GAIN_IDX_EDCCA)) {
		if ((adaptivity->adajust_igi_level > IGI) && (p_dm_odm->adaptivity_enable == true))
			diff = adaptivity->adajust_igi_level - IGI;

		th_l2h_dmc = p_dm_odm->th_l2h_ini - diff + igi_target;
		th_h2l_dmc = th_l2h_dmc - p_dm_odm->th_edcca_hl_diff;
	} else	{
		diff = igi_target - (s8)IGI;
		th_l2h_dmc = p_dm_odm->th_l2h_ini + diff;
		if (th_l2h_dmc > 10 && (p_dm_odm->adaptivity_enable == true))
			th_l2h_dmc = 10;

		th_h2l_dmc = th_l2h_dmc - p_dm_odm->th_edcca_hl_diff;

		/*replace lower bound to prevent EDCCA always equal 1*/
		if (th_h2l_dmc < adaptivity->h2l_lb)
			th_h2l_dmc = adaptivity->h2l_lb;
		if (th_l2h_dmc < adaptivity->l2h_lb)
			th_l2h_dmc = adaptivity->l2h_lb;
	}
	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("IGI=0x%x, th_l2h_dmc = %d, th_h2l_dmc = %d\n", IGI, th_l2h_dmc, th_h2l_dmc));
	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("adaptivity_igi_upper=0x%x, h2l_lb = 0x%x, l2h_lb = 0x%x\n", p_dm_odm->adaptivity_igi_upper, adaptivity->h2l_lb, adaptivity->l2h_lb));

	phydm_set_edcca_threshold(p_dm_odm, th_h2l_dmc, th_l2h_dmc);

	if (p_dm_odm->adaptivity_enable == true)
		odm_set_mac_reg(p_dm_odm, REG_RD_CTRL, BIT(11), 1);

	return;
}

/*This API is for solving USB can't Tx problem due to USB3.0 interference in 2.4G*/
void
phydm_pause_edcca(
	void	*p_dm_void,
	bool	is_pasue_edcca
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ADAPTIVITY_STATISTICS	*adaptivity = (struct _ADAPTIVITY_STATISTICS *)phydm_get_structure(p_dm_odm, PHYDM_ADAPTIVITY);
	struct _dynamic_initial_gain_threshold_	*p_dm_dig_table = &p_dm_odm->dm_dig_table;
	u8	IGI = p_dm_dig_table->cur_ig_value;
	s8	diff = 0;

	if (is_pasue_edcca) {
		adaptivity->is_stop_edcca = true;
		if (p_dm_odm->support_ic_type & (ODM_IC_11AC_GAIN_IDX_EDCCA | ODM_IC_11N_GAIN_IDX_EDCCA)) {
			if (adaptivity->adajust_igi_level > IGI)
				diff = adaptivity->adajust_igi_level - IGI;

			adaptivity->backup_l2h = p_dm_odm->th_l2h_ini - diff + adaptivity->igi_target;
			adaptivity->backup_h2l = adaptivity->backup_l2h - p_dm_odm->th_edcca_hl_diff;
		} else {
			diff = adaptivity->igi_target - (s8)IGI;
			adaptivity->backup_l2h = p_dm_odm->th_l2h_ini + diff;
			if (adaptivity->backup_l2h > 10)
				adaptivity->backup_l2h = 10;

			adaptivity->backup_h2l = adaptivity->backup_l2h - p_dm_odm->th_edcca_hl_diff;

			/*replace lower bound to prevent EDCCA always equal 1*/
			if (adaptivity->backup_h2l < adaptivity->h2l_lb)
				adaptivity->backup_h2l = adaptivity->h2l_lb;
			if (adaptivity->backup_l2h < adaptivity->l2h_lb)
				adaptivity->backup_l2h = adaptivity->l2h_lb;
		}
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("pauseEDCCA : L2Hbak = 0x%x, H2Lbak = 0x%x, IGI = 0x%x\n", adaptivity->backup_l2h, adaptivity->backup_h2l, IGI));

		/*Disable EDCCA*/
		phydm_pause_edcca_work_item_callback(p_dm_odm);
	} else {

		adaptivity->is_stop_edcca = false;
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("resumeEDCCA : L2Hbak = 0x%x, H2Lbak = 0x%x, IGI = 0x%x\n", adaptivity->backup_l2h, adaptivity->backup_h2l, IGI));
		/*Resume EDCCA*/
		phydm_resume_edcca_work_item_callback(p_dm_odm);
	}
}


void
phydm_pause_edcca_work_item_callback(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES)
		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_ECCA_THRESHOLD, MASKBYTE2 | MASKBYTE0, (u32)(0x7f | 0x7f << 16));
	else if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES)
		odm_set_bb_reg(p_dm_odm, REG_FPGA0_XB_LSSI_READ_BACK, MASKLWORD, (u16)(0x7f | 0x7f << 8));
}

void
phydm_resume_edcca_work_item_callback(
	void			*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ADAPTIVITY_STATISTICS	*adaptivity = (struct _ADAPTIVITY_STATISTICS *)phydm_get_structure(p_dm_odm, PHYDM_ADAPTIVITY);

	if (p_dm_odm->support_ic_type & ODM_IC_11N_SERIES)
		odm_set_bb_reg(p_dm_odm, REG_OFDM_0_ECCA_THRESHOLD, MASKBYTE2 | MASKBYTE0, (u32)((u8)adaptivity->backup_l2h | (u8)adaptivity->backup_h2l << 16));
	else if (p_dm_odm->support_ic_type & ODM_IC_11AC_SERIES)
		odm_set_bb_reg(p_dm_odm, REG_FPGA0_XB_LSSI_READ_BACK, MASKLWORD, (u16)((u8)adaptivity->backup_l2h | (u8)adaptivity->backup_h2l << 8));
}


void
phydm_set_edcca_threshold_api(
	void	*p_dm_void,
	u8	IGI
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ADAPTIVITY_STATISTICS	*adaptivity = (struct _ADAPTIVITY_STATISTICS *)phydm_get_structure(p_dm_odm, PHYDM_ADAPTIVITY);
	s8			th_l2h_dmc, th_h2l_dmc;
	s8			diff = 0, igi_target = 0x32;

	if (p_dm_odm->support_ability & ODM_BB_ADAPTIVITY) {
		if (p_dm_odm->support_ic_type & (ODM_IC_11AC_GAIN_IDX_EDCCA | ODM_IC_11N_GAIN_IDX_EDCCA)) {
			if (adaptivity->adajust_igi_level > IGI)
				diff = adaptivity->adajust_igi_level - IGI;

			th_l2h_dmc = p_dm_odm->th_l2h_ini - diff + igi_target;
			th_h2l_dmc = th_l2h_dmc - p_dm_odm->th_edcca_hl_diff;
		} else	{
			diff = igi_target - (s8)IGI;
			th_l2h_dmc = p_dm_odm->th_l2h_ini + diff;
			if (th_l2h_dmc > 10)
				th_l2h_dmc = 10;

			th_h2l_dmc = th_l2h_dmc - p_dm_odm->th_edcca_hl_diff;

			/*replace lower bound to prevent EDCCA always equal 1*/
			if (th_h2l_dmc < adaptivity->h2l_lb)
				th_h2l_dmc = adaptivity->h2l_lb;
			if (th_l2h_dmc < adaptivity->l2h_lb)
				th_l2h_dmc = adaptivity->l2h_lb;
		}
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("API :IGI=0x%x, th_l2h_dmc = %d, th_h2l_dmc = %d\n", IGI, th_l2h_dmc, th_h2l_dmc));
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_ADAPTIVITY, ODM_DBG_LOUD, ("API :adaptivity_igi_upper=0x%x, h2l_lb = 0x%x, l2h_lb = 0x%x\n", p_dm_odm->adaptivity_igi_upper, adaptivity->h2l_lb, adaptivity->l2h_lb));

		phydm_set_edcca_threshold(p_dm_odm, th_h2l_dmc, th_l2h_dmc);
	}

}
