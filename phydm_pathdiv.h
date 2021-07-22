/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


#ifndef	__PHYDMPATHDIV_H__
#define    __PHYDMPATHDIV_H__
/*#define PATHDIV_VERSION "2.0" //2014.11.04*/
#define PATHDIV_VERSION	"3.1" /*2015.07.29 by YuChen*/

#if (defined(CONFIG_PATH_DIVERSITY))
#define USE_PATH_A_AS_DEFAULT_ANT   /* for 8814 dynamic TX path selection */

#define	NUM_RESET_DTP_PERIOD 5
#define	ANT_DECT_RSSI_TH 3

#define PATH_A 1
#define PATH_B 2
#define PATH_C 3
#define PATH_D 4

#define PHYDM_AUTO_PATH	0
#define PHYDM_FIX_PATH		1

#define NUM_CHOOSE2_FROM4 6
#define NUM_CHOOSE3_FROM4 4


#define		PHYDM_A		 BIT(0)
#define		PHYDM_B		 BIT(1)
#define		PHYDM_C		 BIT(2)
#define		PHYDM_D		 BIT(3)
#define		PHYDM_AB	 (BIT(0) | BIT1)  /* 0 */
#define		PHYDM_AC	 (BIT(0) | BIT2)  /* 1 */
#define		PHYDM_AD	 (BIT(0) | BIT3)  /* 2 */
#define		PHYDM_BC	 (BIT(1) | BIT2)  /* 3 */
#define		PHYDM_BD	 (BIT(1) | BIT3)  /* 4 */
#define		PHYDM_CD	 (BIT(2) | BIT3)  /* 5 */

#define		PHYDM_ABC	 (BIT(0) | BIT1 | BIT2) /* 0*/
#define		PHYDM_ABD	 (BIT(0) | BIT1 | BIT3) /* 1*/
#define		PHYDM_ACD	 (BIT(0) | BIT2 | BIT3) /* 2*/
#define		PHYDM_BCD	 (BIT(1) | BIT2 | BIT3) /* 3*/

#define		PHYDM_ABCD	 (BIT(0) | BIT(1) | BIT(2) | BIT(3))


enum phydm_dtp_state {
	PHYDM_DTP_INIT = 1,
	PHYDM_DTP_RUNNING_1

};

enum phydm_path_div_type {
	PHYDM_2R_PATH_DIV = 1,
	PHYDM_4R_PATH_DIV = 2
};

void
phydm_process_rssi_for_path_div(
	void			*p_dm_void,
	void			*p_phy_info_void,
	void			*p_pkt_info_void
);

struct _ODM_PATH_DIVERSITY_ {
	u8	resp_tx_path;
	u8	path_sel[ODM_ASSOCIATE_ENTRY_NUM];
	u32	path_a_sum[ODM_ASSOCIATE_ENTRY_NUM];
	u32	path_b_sum[ODM_ASSOCIATE_ENTRY_NUM];
	u16	path_a_cnt[ODM_ASSOCIATE_ENTRY_NUM];
	u16	path_b_cnt[ODM_ASSOCIATE_ENTRY_NUM];
	u8	phydm_path_div_type;
#if RTL8814A_SUPPORT

	u32	path_a_sum_all;
	u32	path_b_sum_all;
	u32	path_c_sum_all;
	u32	path_d_sum_all;

	u32	path_a_cnt_all;
	u32	path_b_cnt_all;
	u32	path_c_cnt_all;
	u32	path_d_cnt_all;

	u8	dtp_period;
	bool	is_become_linked;
	bool	is_u3_mode;
	u8	num_tx_path;
	u8	default_path;
	u8	num_candidate;
	u8	ant_candidate_1;
	u8	ant_candidate_2;
	u8	ant_candidate_3;
	u8     phydm_dtp_state;
	u8	dtp_check_patha_counter;
	bool	fix_path_bfer;
	u8	search_space_2[NUM_CHOOSE2_FROM4];
	u8	search_space_3[NUM_CHOOSE3_FROM4];

	u8	pre_tx_path;
	u8	use_path_a_as_default_ant;
	bool is_path_a_exist;

#endif
};


#endif /* #if(defined(CONFIG_PATH_DIVERSITY)) */

void
phydm_c2h_dtp_handler(
	void	*p_dm_void,
	u8   *cmd_buf,
	u8	cmd_len
);

void
odm_path_diversity_init(
	void	*p_dm_void
);

void
odm_path_diversity(
	void	*p_dm_void
);

void
odm_pathdiv_debug(
	void		*p_dm_void,
	u32		*const dm_value,
	u32		*_used,
	char		*output,
	u32		*_out_len
);

#endif		 /* #ifndef  __ODMPATHDIV_H__ */
