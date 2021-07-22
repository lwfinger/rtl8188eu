// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


/* ************************************************************
 * include files
 * ************************************************************ */
#include "mp_precomp.h"
#include "phydm_precomp.h"

void
phydm_h2C_debug(
	void		*p_dm_void,
	u32		*const dm_value,
	u32		*_used,
	char			*output,
	u32		*_out_len
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8			h2c_parameter[H2C_MAX_LENGTH] = {0};
	u8			phydm_h2c_id = (u8)dm_value[0];
	u8			i;
	u32			used = *_used;
	u32			out_len = *_out_len;

	PHYDM_SNPRINTF((output + used, out_len - used, "Phydm Send H2C_ID (( 0x%x))\n", phydm_h2c_id));
	for (i = 0; i < H2C_MAX_LENGTH; i++) {

		h2c_parameter[i] = (u8)dm_value[i + 1];
		PHYDM_SNPRINTF((output + used, out_len - used, "H2C: Byte[%d] = ((0x%x))\n", i, h2c_parameter[i]));
	}

	odm_fill_h2c_cmd(p_dm_odm, phydm_h2c_id, H2C_MAX_LENGTH, h2c_parameter);

}

#if (defined(CONFIG_RA_DBG_CMD))
void
odm_ra_para_adjust_send_h2c(
	void	*p_dm_void
)
{

	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_			*p_ra_table = &p_dm_odm->dm_ra_table;
	u8			h2c_parameter[6] = {0};

	h2c_parameter[0] = RA_FIRST_MACID;

	if (p_ra_table->ra_para_feedback_req) { /*h2c_parameter[5]=1 ; ask FW for all RA parameters*/
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("[H2C] Ask FW for RA parameter\n"));
		h2c_parameter[5] |= BIT(1); /*ask FW to report RA parameters*/
		h2c_parameter[1] = p_ra_table->para_idx; /*p_ra_table->para_idx;*/
		p_ra_table->ra_para_feedback_req = 0;
	} else {
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("[H2C] Send H2C to FW for modifying RA parameter\n"));

		h2c_parameter[1] =  p_ra_table->para_idx;
		h2c_parameter[2] =  p_ra_table->rate_idx;
		/* [8 bit]*/
		if (p_ra_table->para_idx == RADBG_RTY_PENALTY || p_ra_table->para_idx == RADBG_RATE_UP_RTY_RATIO || p_ra_table->para_idx == RADBG_RATE_DOWN_RTY_RATIO) {
			h2c_parameter[3] = p_ra_table->value;
			h2c_parameter[4] = 0;
		}
		/* [16 bit]*/
		else {
			h2c_parameter[3] = (u8)(((p_ra_table->value_16) & 0xf0) >> 4); /*byte1*/
			h2c_parameter[4] = (u8)((p_ra_table->value_16) & 0x0f);	/*byte0*/
		}
	}
	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" h2c_parameter[1] = 0x%x\n", h2c_parameter[1]));
	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" h2c_parameter[2] = 0x%x\n", h2c_parameter[2]));
	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" h2c_parameter[3] = 0x%x\n", h2c_parameter[3]));
	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" h2c_parameter[4] = 0x%x\n", h2c_parameter[4]));
	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" h2c_parameter[5] = 0x%x\n", h2c_parameter[5]));

	odm_fill_h2c_cmd(p_dm_odm, ODM_H2C_RA_PARA_ADJUST, 6, h2c_parameter);

}


void
odm_ra_para_adjust(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_			*p_ra_table = &p_dm_odm->dm_ra_table;
	u8			rate_idx = p_ra_table->rate_idx;
	u8			value = p_ra_table->value;
	u8			pre_value = 0xff;

	if (p_ra_table->para_idx == RADBG_RTY_PENALTY) {
		pre_value = p_ra_table->RTY_P[rate_idx];
		p_ra_table->RTY_P[rate_idx] = value;
		p_ra_table->RTY_P_modify_note[rate_idx] = 1;
	} else if (p_ra_table->para_idx == RADBG_N_HIGH) {

	} else if (p_ra_table->para_idx == RADBG_N_LOW) {

	} else if (p_ra_table->para_idx == RADBG_RATE_UP_RTY_RATIO) {
		pre_value = p_ra_table->RATE_UP_RTY_RATIO[rate_idx];
		p_ra_table->RATE_UP_RTY_RATIO[rate_idx] = value;
		p_ra_table->RATE_UP_RTY_RATIO_modify_note[rate_idx] = 1;
	} else if (p_ra_table->para_idx == RADBG_RATE_DOWN_RTY_RATIO) {
		pre_value = p_ra_table->RATE_DOWN_RTY_RATIO[rate_idx];
		p_ra_table->RATE_DOWN_RTY_RATIO[rate_idx] = value;
		p_ra_table->RATE_DOWN_RTY_RATIO_modify_note[rate_idx] = 1;
	}
	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("Change RA Papa[%d], rate[ %d ],   ((%d))  ->  ((%d))\n", p_ra_table->para_idx, rate_idx, pre_value, value));
	odm_ra_para_adjust_send_h2c(p_dm_odm);
}

void
phydm_ra_print_msg(
	void		*p_dm_void,
	u8		*value,
	u8		*value_default,
	u8		*modify_note
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_			*p_ra_table = &p_dm_odm->dm_ra_table;
	u32 i;

	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" |rate index| |Current-value| |Default-value| |Modify?|\n"));
	for (i = 0 ; i <= (p_ra_table->rate_length); i++) {
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("     [ %d ]  %10d  %14d  %14s\n", i, value[i], value_default[i], ((modify_note[i] == 1) ? "V" : " .  ")));
	}

}

void
odm_RA_debug(
	void		*p_dm_void,
	u32		*const dm_value
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_			*p_ra_table = &p_dm_odm->dm_ra_table;

	p_ra_table->is_ra_dbg_init = false;

	if (dm_value[0] == 100) { /*1 Print RA Parameters*/
		u8	default_pointer_value;
		u8	*pvalue;
		u8	*pvalue_default;
		u8	*pmodify_note;

		pvalue = pvalue_default = pmodify_note = &default_pointer_value;

		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("\n------------------------------------------------------------------------------------\n"));

		if (dm_value[1] == RADBG_RTY_PENALTY) { /* [1]*/
			ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" [1] RTY_PENALTY\n"));
			pvalue		=	&(p_ra_table->RTY_P[0]);
			pvalue_default	=	&(p_ra_table->RTY_P_default[0]);
			pmodify_note	=	(u8 *)&(p_ra_table->RTY_P_modify_note[0]);
		} else if (dm_value[1] == RADBG_N_HIGH)   /* [2]*/
			ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" [2] N_HIGH\n"));

		else if (dm_value[1] == RADBG_N_LOW)   /*[3]*/
			ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" [3] N_LOW\n"));

		else if (dm_value[1] == RADBG_RATE_UP_RTY_RATIO) { /* [8]*/
			ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" [8] RATE_UP_RTY_RATIO\n"));
			pvalue		=	&(p_ra_table->RATE_UP_RTY_RATIO[0]);
			pvalue_default	=	&(p_ra_table->RATE_UP_RTY_RATIO_default[0]);
			pmodify_note	=	(u8 *)&(p_ra_table->RATE_UP_RTY_RATIO_modify_note[0]);
		} else if (dm_value[1] == RADBG_RATE_DOWN_RTY_RATIO) { /* [9]*/
			ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" [9] RATE_DOWN_RTY_RATIO\n"));
			pvalue		=	&(p_ra_table->RATE_DOWN_RTY_RATIO[0]);
			pvalue_default	=	&(p_ra_table->RATE_DOWN_RTY_RATIO_default[0]);
			pmodify_note	=	(u8 *)&(p_ra_table->RATE_DOWN_RTY_RATIO_modify_note[0]);
		}

		phydm_ra_print_msg(p_dm_odm, pvalue, pvalue_default, pmodify_note);
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("\n------------------------------------------------------------------------------------\n\n"));

	} else if (dm_value[0] == 101) {
		p_ra_table->para_idx = (u8)dm_value[1];

		p_ra_table->ra_para_feedback_req = 1;
		odm_ra_para_adjust_send_h2c(p_dm_odm);
	} else {
		p_ra_table->para_idx = (u8)dm_value[0];
		p_ra_table->rate_idx  = (u8)dm_value[1];
		p_ra_table->value = (u8)dm_value[2];

		odm_ra_para_adjust(p_dm_odm);
	}
}

void
odm_ra_para_adjust_init(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_			*p_ra_table = &p_dm_odm->dm_ra_table;
	u8			i;
	u8			ra_para_pool_u8[3] = { RADBG_RTY_PENALTY,  RADBG_RATE_UP_RTY_RATIO, RADBG_RATE_DOWN_RTY_RATIO};
	u8			rate_size_ht_1ss = 20, rate_size_ht_2ss = 28, rate_size_ht_3ss = 36;	 /*4+8+8+8+8 =36*/
	u8			rate_size_vht_1ss = 10, rate_size_vht_2ss = 20, rate_size_vht_3ss = 30;	 /*10 + 10 +10 =30*/

	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("odm_ra_para_adjust_init\n"));

	if (p_dm_odm->support_ic_type & (ODM_RTL8188F | ODM_RTL8195A | ODM_RTL8703B | ODM_RTL8723B | ODM_RTL8188E | ODM_RTL8723D))
		p_ra_table->rate_length = rate_size_ht_1ss;
	else if (p_dm_odm->support_ic_type & (ODM_RTL8192E | ODM_RTL8197F))
		p_ra_table->rate_length = rate_size_ht_2ss;
	else if (p_dm_odm->support_ic_type & (ODM_RTL8821 | ODM_RTL8881A | ODM_RTL8821C))
		p_ra_table->rate_length = rate_size_ht_1ss + rate_size_vht_1ss;
	else if (p_dm_odm->support_ic_type & (ODM_RTL8812 | ODM_RTL8822B))
		p_ra_table->rate_length = rate_size_ht_2ss + rate_size_vht_2ss;
	else if (p_dm_odm->support_ic_type == ODM_RTL8814A)
		p_ra_table->rate_length = rate_size_ht_3ss + rate_size_vht_3ss;
	else
		p_ra_table->rate_length = rate_size_ht_1ss;

	p_ra_table->is_ra_dbg_init = true;
	for (i = 0; i < 3; i++) {
		p_ra_table->ra_para_feedback_req = 1;
		p_ra_table->para_idx	=	ra_para_pool_u8[i];
		odm_ra_para_adjust_send_h2c(p_dm_odm);
	}
}

#else

void
phydm_RA_debug_PCR(
	void		*p_dm_void,
	u32		*const dm_value,
	u32		*_used,
	char			*output,
	u32		*_out_len
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_			*p_ra_table = &p_dm_odm->dm_ra_table;
	u32 used = *_used;
	u32 out_len = *_out_len;

	if (dm_value[0] == 100) {
		PHYDM_SNPRINTF((output + used, out_len - used, "[Get] PCR RA_threshold_offset = (( %s%d ))\n", ((p_ra_table->RA_threshold_offset == 0) ? " " : ((p_ra_table->RA_offset_direction) ? "+" : "-")), p_ra_table->RA_threshold_offset));
		/**/
	} else if (dm_value[0] == 0) {
		p_ra_table->RA_offset_direction = 0;
		p_ra_table->RA_threshold_offset = (u8)dm_value[1];
		PHYDM_SNPRINTF((output + used, out_len - used, "[Set] PCR RA_threshold_offset = (( -%d ))\n", p_ra_table->RA_threshold_offset));
	} else if (dm_value[0] == 1) {
		p_ra_table->RA_offset_direction = 1;
		p_ra_table->RA_threshold_offset = (u8)dm_value[1];
		PHYDM_SNPRINTF((output + used, out_len - used, "[Set] PCR RA_threshold_offset = (( +%d ))\n", p_ra_table->RA_threshold_offset));
	} else {
		PHYDM_SNPRINTF((output + used, out_len - used, "[Set] Error\n"));
		/**/
	}

}

#endif /*#if (defined(CONFIG_RA_DBG_CMD))*/

void
odm_c2h_ra_para_report_handler(
	void	*p_dm_void,
	u8	*cmd_buf,
	u8	cmd_len
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_			*p_ra_table = &p_dm_odm->dm_ra_table;

	u8	para_idx = cmd_buf[0]; /*Retry Penalty, NH, NL*/
	u8	rate_type_start = cmd_buf[1];
	u8	rate_type_length = cmd_len - 2;
	u8	i;


	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("[ From FW C2H RA Para ]  cmd_buf[0]= (( %d ))\n", cmd_buf[0]));

#if (defined(CONFIG_RA_DBG_CMD))
	if (para_idx == RADBG_RTY_PENALTY) {
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" |rate index|   |RTY Penality index|\n"));

		for (i = 0 ; i < (rate_type_length) ; i++) {
			if (p_ra_table->is_ra_dbg_init)
				p_ra_table->RTY_P_default[rate_type_start + i] = cmd_buf[2 + i];

			p_ra_table->RTY_P[rate_type_start + i] = cmd_buf[2 + i];
			ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("%8d  %15d\n", (rate_type_start + i), p_ra_table->RTY_P[rate_type_start + i]));
		}

	} else	if (para_idx == RADBG_N_HIGH) {
		/**/
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" |rate index|    |N-High|\n"));


	} else if (para_idx == RADBG_N_LOW) {
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" |rate index|   |N-Low|\n"));
		/**/
	} else if (para_idx == RADBG_RATE_UP_RTY_RATIO) {
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" |rate index|   |rate Up RTY Ratio|\n"));

		for (i = 0; i < (rate_type_length); i++) {
			if (p_ra_table->is_ra_dbg_init)
				p_ra_table->RATE_UP_RTY_RATIO_default[rate_type_start + i] = cmd_buf[2 + i];

			p_ra_table->RATE_UP_RTY_RATIO[rate_type_start + i] = cmd_buf[2 + i];
			ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("%8d  %15d\n", (rate_type_start + i), p_ra_table->RATE_UP_RTY_RATIO[rate_type_start + i]));
		}
	} else	 if (para_idx == RADBG_RATE_DOWN_RTY_RATIO) {
		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" |rate index|   |rate Down RTY Ratio|\n"));

		for (i = 0; i < (rate_type_length); i++) {
			if (p_ra_table->is_ra_dbg_init)
				p_ra_table->RATE_DOWN_RTY_RATIO_default[rate_type_start + i] = cmd_buf[2 + i];

			p_ra_table->RATE_DOWN_RTY_RATIO[rate_type_start + i] = cmd_buf[2 + i];
			ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("%8d  %15d\n", (rate_type_start + i), p_ra_table->RATE_DOWN_RTY_RATIO[rate_type_start + i]));
		}
	} else
#endif
		if (para_idx == RADBG_DEBUG_MONITOR1) {
			ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("-------------------------------\n"));
			if (p_dm_odm->support_ic_type & PHYDM_IC_3081_SERIES) {

				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "RSSI =", cmd_buf[1]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x\n", "rate =", cmd_buf[2] & 0x7f));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "SGI =", (cmd_buf[2] & 0x80) >> 7));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "BW =", cmd_buf[3]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "BW_max =", cmd_buf[4]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x\n", "multi_rate0 =", cmd_buf[5]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x\n", "multi_rate1 =", cmd_buf[6]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "DISRA =",	cmd_buf[7]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "VHT_EN =", cmd_buf[8]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "SGI_support =",	cmd_buf[9]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "try_ness =", cmd_buf[10]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x\n", "pre_rate =", cmd_buf[11]));
			} else {
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "RSSI =", cmd_buf[1]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %x\n", "BW =", cmd_buf[2]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "DISRA =", cmd_buf[3]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "VHT_EN =", cmd_buf[4]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "Hightest rate =", cmd_buf[5]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x\n", "Lowest rate =", cmd_buf[6]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x\n", "SGI_support =", cmd_buf[7]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "Rate_ID =",	cmd_buf[8]));;
			}
			ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("-------------------------------\n"));
		} else	 if (para_idx == RADBG_DEBUG_MONITOR2) {
			ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("-------------------------------\n"));
			if (p_dm_odm->support_ic_type & PHYDM_IC_3081_SERIES) {
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "rate_id =", cmd_buf[1]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x\n", "highest_rate =", cmd_buf[2]));
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x\n", "lowest_rate =", cmd_buf[3]));

				for (i = 4; i <= 11; i++)
					ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("RAMASK =  0x%x\n", cmd_buf[i]));
			} else {
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %x%x  %x%x  %x%x  %x%x\n", "RA Mask:",
					cmd_buf[8], cmd_buf[7], cmd_buf[6], cmd_buf[5], cmd_buf[4], cmd_buf[3], cmd_buf[2], cmd_buf[1]));
			}
			ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("-------------------------------\n"));
		} else	 if (para_idx == RADBG_DEBUG_MONITOR3) {

			for (i = 0; i < (cmd_len - 1); i++)
				ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("content[%d] = %d\n", i, cmd_buf[1 + i]));
		} else	 if (para_idx == RADBG_DEBUG_MONITOR4)
			ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  {%d.%d}\n", "RA version =", cmd_buf[1], cmd_buf[2]));
		else if (para_idx == RADBG_DEBUG_MONITOR5) {
			ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x\n", "Current rate =", cmd_buf[1]));
			ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "Retry ratio =", cmd_buf[2]));
			ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  %d\n", "rate down ratio =", cmd_buf[3]));
			ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x\n", "highest rate =", cmd_buf[4]));
			ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  {0x%x 0x%x}\n", "Muti-try =", cmd_buf[5], cmd_buf[6]));
			ODM_RT_TRACE(p_dm_odm, ODM_FW_DEBUG_TRACE, ODM_DBG_LOUD, ("%5s  0x%x%x%x%x%x\n", "RA mask =", cmd_buf[11], cmd_buf[10], cmd_buf[9], cmd_buf[8], cmd_buf[7]));
		}
}

void
phydm_ra_dynamic_retry_count(
	void	*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_		*p_ra_table = &p_dm_odm->dm_ra_table;
	struct sta_info		*p_entry;
	u8	i, retry_offset;
	u32	ma_rx_tp;

	if (!(p_dm_odm->support_ability & ODM_BB_DYNAMIC_ARFR))
		return;

	/*ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("p_dm_odm->pre_b_noisy = %d\n", p_dm_odm->pre_b_noisy ));*/
	if (p_dm_odm->pre_b_noisy != p_dm_odm->noisy_decision) {

		if (p_dm_odm->noisy_decision) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("->Noisy Env. RA fallback value\n"));
			odm_set_mac_reg(p_dm_odm, 0x430, MASKDWORD, 0x0);
			odm_set_mac_reg(p_dm_odm, 0x434, MASKDWORD, 0x04030201);
		} else {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("->Clean Env. RA fallback value\n"));
			odm_set_mac_reg(p_dm_odm, 0x430, MASKDWORD, 0x01000000);
			odm_set_mac_reg(p_dm_odm, 0x434, MASKDWORD, 0x06050402);
		}
		p_dm_odm->pre_b_noisy = p_dm_odm->noisy_decision;
	}
}

#if (defined(CONFIG_RA_DYNAMIC_RTY_LIMIT))

void
phydm_retry_limit_table_bound(
	void	*p_dm_void,
	u8	*retry_limit,
	u8	offset
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_		*p_ra_table = &p_dm_odm->dm_ra_table;

	if (*retry_limit >  offset) {

		*retry_limit -= offset;

		if (*retry_limit < p_ra_table->retrylimit_low)
			*retry_limit = p_ra_table->retrylimit_low;
		else if (*retry_limit > p_ra_table->retrylimit_high)
			*retry_limit = p_ra_table->retrylimit_high;
	} else
		*retry_limit = p_ra_table->retrylimit_low;
}

void
phydm_reset_retry_limit_table(
	void	*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_		*p_ra_table = &p_dm_odm->dm_ra_table;
	u8			i;

#if ((RTL8192E_SUPPORT == 1) || (RTL8723B_SUPPORT == 1) || (RTL8188E_SUPPORT == 1))
	u8 per_rate_retrylimit_table_20M[ODM_RATEMCS15 + 1] = {
		1, 1, 2, 4,					/*CCK*/
		2, 2, 4, 6, 8, 12, 16, 18,		/*OFDM*/
		2, 4, 6, 8, 12, 18, 20, 22,		/*20M HT-1SS*/
		2, 4, 6, 8, 12, 18, 20, 22		/*20M HT-2SS*/
	};
	u8 per_rate_retrylimit_table_40M[ODM_RATEMCS15 + 1] = {
		1, 1, 2, 4,					/*CCK*/
		2, 2, 4, 6, 8, 12, 16, 18,		/*OFDM*/
		4, 8, 12, 16, 24, 32, 32, 32,		/*40M HT-1SS*/
		4, 8, 12, 16, 24, 32, 32, 32		/*40M HT-2SS*/
	};

#elif (RTL8821A_SUPPORT == 1) || (RTL8881A_SUPPORT == 1)

#elif (RTL8812A_SUPPORT == 1)

#elif (RTL8814A_SUPPORT == 1)

#else

#endif

	memcpy(&(p_ra_table->per_rate_retrylimit_20M[0]), &(per_rate_retrylimit_table_20M[0]), ODM_NUM_RATE_IDX);
	memcpy(&(p_ra_table->per_rate_retrylimit_40M[0]), &(per_rate_retrylimit_table_40M[0]), ODM_NUM_RATE_IDX);

	for (i = 0; i < ODM_NUM_RATE_IDX; i++) {
		phydm_retry_limit_table_bound(p_dm_odm, &(p_ra_table->per_rate_retrylimit_20M[i]), 0);
		phydm_retry_limit_table_bound(p_dm_odm, &(p_ra_table->per_rate_retrylimit_40M[i]), 0);
	}
}

void
phydm_ra_dynamic_retry_limit_init(
	void	*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_			*p_ra_table = &p_dm_odm->dm_ra_table;

	p_ra_table->retry_descend_num = RA_RETRY_DESCEND_NUM;
	p_ra_table->retrylimit_low = RA_RETRY_LIMIT_LOW;
	p_ra_table->retrylimit_high = RA_RETRY_LIMIT_HIGH;

	phydm_reset_retry_limit_table(p_dm_odm);

}

#endif

void
phydm_ra_dynamic_retry_limit(
	void	*p_dm_void
)
{
#if (defined(CONFIG_RA_DYNAMIC_RTY_LIMIT))
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_		*p_ra_table = &p_dm_odm->dm_ra_table;
	struct sta_info		*p_entry;
	u8	i, retry_offset;
	u32	ma_rx_tp;


	if (p_dm_odm->pre_number_active_client == p_dm_odm->number_active_client) {

		ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, (" pre_number_active_client ==  number_active_client\n"));
		return;

	} else {
		if (p_dm_odm->number_active_client == 1) {
			phydm_reset_retry_limit_table(p_dm_odm);
			ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("one client only->reset to default value\n"));
		} else {

			retry_offset = p_dm_odm->number_active_client * p_ra_table->retry_descend_num;

			for (i = 0; i < ODM_NUM_RATE_IDX; i++) {

				phydm_retry_limit_table_bound(p_dm_odm, &(p_ra_table->per_rate_retrylimit_20M[i]), retry_offset);
				phydm_retry_limit_table_bound(p_dm_odm, &(p_ra_table->per_rate_retrylimit_40M[i]), retry_offset);
			}
		}
	}
#endif
}

#if (defined(CONFIG_RA_DYNAMIC_RATE_ID))
void
phydm_ra_dynamic_rate_id_on_assoc(
	void	*p_dm_void,
	u8	wireless_mode,
	u8	init_rate_id
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("[ON ASSOC] rf_mode = ((0x%x)), wireless_mode = ((0x%x)), init_rate_id = ((0x%x))\n", p_dm_odm->rf_type, wireless_mode, init_rate_id));

	if ((p_dm_odm->rf_type == ODM_2T2R) | (p_dm_odm->rf_type == ODM_2T2R_GREEN) | (p_dm_odm->rf_type == ODM_2T3R) | (p_dm_odm->rf_type == ODM_2T4R)) {

		if ((p_dm_odm->support_ic_type & (ODM_RTL8812 | ODM_RTL8192E)) &&
		    (wireless_mode & (ODM_WM_N24G | ODM_WM_N5G))
		   ) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("[ON ASSOC] set N-2SS ARFR5 table\n"));
			odm_set_mac_reg(p_dm_odm, 0x4a4, MASKDWORD, 0xfc1ffff);	/*N-2SS, ARFR5, rate_id = 0xe*/
			odm_set_mac_reg(p_dm_odm, 0x4a8, MASKDWORD, 0x0);		/*N-2SS, ARFR5, rate_id = 0xe*/
		} else if ((p_dm_odm->support_ic_type & (ODM_RTL8812)) &&
			(wireless_mode & (ODM_WM_AC_5G | ODM_WM_AC_24G | ODM_WM_AC_ONLY))
			  ) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("[ON ASSOC] set AC-2SS ARFR0 table\n"));
			odm_set_mac_reg(p_dm_odm, 0x444, MASKDWORD, 0x0fff);	/*AC-2SS, ARFR0, rate_id = 0x9*/
			odm_set_mac_reg(p_dm_odm, 0x448, MASKDWORD, 0xff01f000);		/*AC-2SS, ARFR0, rate_id = 0x9*/
		}
	}

}

void
phydm_ra_dynamic_rate_id_init(
	void	*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	if (p_dm_odm->support_ic_type & (ODM_RTL8812 | ODM_RTL8192E)) {

		odm_set_mac_reg(p_dm_odm, 0x4a4, MASKDWORD, 0xfc1ffff);	/*N-2SS, ARFR5, rate_id = 0xe*/
		odm_set_mac_reg(p_dm_odm, 0x4a8, MASKDWORD, 0x0);		/*N-2SS, ARFR5, rate_id = 0xe*/

		odm_set_mac_reg(p_dm_odm, 0x444, MASKDWORD, 0x0fff);		/*AC-2SS, ARFR0, rate_id = 0x9*/
		odm_set_mac_reg(p_dm_odm, 0x448, MASKDWORD, 0xff01f000);	/*AC-2SS, ARFR0, rate_id = 0x9*/
	}
}

void
phydm_update_rate_id(
	void	*p_dm_void,
	u8	rate,
	u8	platform_macid
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_		*p_ra_table = &p_dm_odm->dm_ra_table;
	u8		current_tx_ss;
	u8		rate_idx = rate & 0x7f; /*remove bit7 SGI*/
	u8		wireless_mode;
	u8		phydm_macid;
	struct sta_info	*p_entry;


#if	0
	if (rate_idx >= ODM_RATEVHTSS2MCS0) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("rate[%d]: (( VHT2SS-MCS%d ))\n", platform_macid, (rate_idx - ODM_RATEVHTSS2MCS0)));
		/*dummy for SD4 check patch*/
	} else if (rate_idx >= ODM_RATEVHTSS1MCS0) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("rate[%d]: (( VHT1SS-MCS%d ))\n", platform_macid, (rate_idx - ODM_RATEVHTSS1MCS0)));
		/*dummy for SD4 check patch*/
	} else if (rate_idx >= ODM_RATEMCS0) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("rate[%d]: (( HT-MCS%d ))\n", platform_macid, (rate_idx - ODM_RATEMCS0)));
		/*dummy for SD4 check patch*/
	} else {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("rate[%d]: (( HT-MCS%d ))\n", platform_macid, rate_idx));
		/*dummy for SD4 check patch*/
	}
#endif

	phydm_macid = p_dm_odm->platform2phydm_macid_table[platform_macid];
	p_entry = p_dm_odm->p_odm_sta_info[phydm_macid];

	if (IS_STA_VALID(p_entry)) {
		wireless_mode = p_entry->wireless_mode;

		if ((p_dm_odm->rf_type  == ODM_2T2R) | (p_dm_odm->rf_type  == ODM_2T2R_GREEN) | (p_dm_odm->rf_type  == ODM_2T3R) | (p_dm_odm->rf_type  == ODM_2T4R)) {

			p_entry->ratr_idx = p_entry->ratr_idx_init;
			if (wireless_mode & (ODM_WM_N24G | ODM_WM_N5G)) { /*N mode*/
				if (rate_idx >= ODM_RATEMCS8 && rate_idx <= ODM_RATEMCS15) { /*2SS mode*/

					p_entry->ratr_idx = ARFR_5_RATE_ID;
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("ARFR_5\n"));
				}
			} else if (wireless_mode & (ODM_WM_AC_5G | ODM_WM_AC_24G | ODM_WM_AC_ONLY)) {/*AC mode*/
				if (rate_idx >= ODM_RATEVHTSS2MCS0 && rate_idx <= ODM_RATEVHTSS2MCS9) {/*2SS mode*/

					p_entry->ratr_idx = ARFR_0_RATE_ID;
					ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("ARFR_0\n"));
				}
			}
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("UPdate_RateID[%d]: (( 0x%x ))\n", platform_macid, p_entry->ratr_idx));
		}
	}

}
#endif

void
phydm_print_rate(
	void	*p_dm_void,
	u8	rate,
	u32	dbg_component
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8		legacy_table[12] = {1, 2, 5, 11, 6, 9, 12, 18, 24, 36, 48, 54};
	u8		rate_idx = rate & 0x7f; /*remove bit7 SGI*/
	u8		vht_en = (rate_idx >= ODM_RATEVHTSS1MCS0) ? 1 : 0;
	u8		b_sgi = (rate & 0x80) >> 7;

	ODM_RT_TRACE_F(p_dm_odm, dbg_component, ODM_DBG_LOUD, ("( %s%s%s%s%d%s%s)\n",
		((rate_idx >= ODM_RATEVHTSS1MCS0) && (rate_idx <= ODM_RATEVHTSS1MCS9)) ? "VHT 1ss  " : "",
		((rate_idx >= ODM_RATEVHTSS2MCS0) && (rate_idx <= ODM_RATEVHTSS2MCS9)) ? "VHT 2ss " : "",
		((rate_idx >= ODM_RATEVHTSS3MCS0) && (rate_idx <= ODM_RATEVHTSS3MCS9)) ? "VHT 3ss " : "",
			(rate_idx >= ODM_RATEMCS0) ? "MCS " : "",
		(vht_en) ? ((rate_idx - ODM_RATEVHTSS1MCS0) % 10) : ((rate_idx >= ODM_RATEMCS0) ? (rate_idx - ODM_RATEMCS0) : ((rate_idx <= ODM_RATE54M) ? legacy_table[rate_idx] : 0)),
			(b_sgi) ? "-S" : "  ",
			(rate_idx >= ODM_RATEMCS0) ? "" : "M"));
}

void
phydm_c2h_ra_report_handler(
	void	*p_dm_void,
	u8   *cmd_buf,
	u8   cmd_len
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_		*p_ra_table = &p_dm_odm->dm_ra_table;
	u8	legacy_table[12] = {1, 2, 5, 11, 6, 9, 12, 18, 24, 36, 48, 54};
	u8	macid = cmd_buf[1];
	u8	rate = cmd_buf[0];
	u8	rate_idx = rate & 0x7f; /*remove bit7 SGI*/
	u8	pre_rate = p_ra_table->link_tx_rate[macid];
	u8	rate_order;

	if (cmd_len >= 4) {
		if (cmd_buf[3] == 0) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("TX Init-rate Update[%d]:", macid));
			/**/
		} else if (cmd_buf[3] == 0xff) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("FW Level: Fix rate[%d]:", macid));
			/**/
		} else if (cmd_buf[3] == 1) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("Try Success[%d]:", macid));
			/**/
		} else if (cmd_buf[3] == 2) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("Try Fail & Try Again[%d]:", macid));
			/**/
		} else if (cmd_buf[3] == 3) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("rate Back[%d]:", macid));
			/**/
		} else if (cmd_buf[3] == 4) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("start rate by RSSI[%d]:", macid));
			/**/
		} else if (cmd_buf[3] == 5) {
			ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("Try rate[%d]:", macid));
			/**/
		}
	} else {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("Tx rate Update[%d]:", macid));
		/**/
	}

	/*phydm_print_rate(p_dm_odm, pre_rate_idx, ODM_COMP_RATE_ADAPTIVE);*/
	/*ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, (">\n",macid );*/
	phydm_print_rate(p_dm_odm, rate, ODM_COMP_RATE_ADAPTIVE);

	p_ra_table->link_tx_rate[macid] = rate;

	/*trigger power training*/

	rate_order = phydm_rate_order_compute(p_dm_odm, rate_idx);

	if ((p_dm_odm->is_one_entry_only) ||
	    ((rate_order > p_ra_table->highest_client_tx_order) && (p_ra_table->power_tracking_flag == 1))
	   ) {
		phydm_update_pwr_track(p_dm_odm, rate_idx);
		p_ra_table->power_tracking_flag = 0;
	}

	/*trigger dynamic rate ID*/
#if (defined(CONFIG_RA_DYNAMIC_RATE_ID))
	if (p_dm_odm->support_ic_type & (ODM_RTL8812 | ODM_RTL8192E))
		phydm_update_rate_id(p_dm_odm, rate, macid);
#endif

}

void
odm_rssi_monitor_init(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_		*p_ra_table = &p_dm_odm->dm_ra_table;

	p_ra_table->firstconnect = false;
}

void
odm_ra_post_action_on_assoc(
	void	*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	/*
		p_dm_odm->h2c_rarpt_connect = 1;
		odm_rssi_monitor_check(p_dm_odm);
		p_dm_odm->h2c_rarpt_connect = 0;
	*/
}

void
phydm_init_ra_info(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
}

void
phydm_modify_RA_PCR_threshold(
	void		*p_dm_void,
	u8		RA_offset_direction,
	u8		RA_threshold_offset

)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_			*p_ra_table = &p_dm_odm->dm_ra_table;

	p_ra_table->RA_offset_direction = RA_offset_direction;
	p_ra_table->RA_threshold_offset = RA_threshold_offset;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("Set RA_threshold_offset = (( %s%d ))\n", ((RA_threshold_offset == 0) ? " " : ((RA_offset_direction) ? "+" : "-")), RA_threshold_offset));
}

static void
odm_rssi_monitor_check_mp(
	void	*p_dm_void
)
{
}

/*H2C_RSSI_REPORT*/
static s8 phydm_rssi_report(struct PHY_DM_STRUCT *p_dm_odm, u8 mac_id)
{
	struct _ADAPTER *adapter = p_dm_odm->adapter;
	struct _rate_adaptive_table_			*p_ra_table = &p_dm_odm->dm_ra_table;
	struct dvobj_priv *pdvobjpriv = adapter_to_dvobj(adapter);
	HAL_DATA_TYPE *p_hal_data = GET_HAL_DATA(adapter);
	u8 h2c_parameter[H2C_0X42_LENGTH] = {0};
	u8 UL_DL_STATE = 0, STBC_TX = 0, tx_bf_en = 0;
	u8 cmdlen = H2C_0X42_LENGTH, first_connect = false;
	u64	cur_tx_ok_cnt = 0, cur_rx_ok_cnt = 0;
	struct sta_info *p_entry = p_dm_odm->p_odm_sta_info[mac_id];

	if (!IS_STA_VALID(p_entry))
		return _FAIL;

	if (mac_id != p_entry->mac_id) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("%s mac_id:%u:%u invalid\n", __func__, mac_id, p_entry->mac_id));
		rtw_warn_on(1);
		return _FAIL;
	}

	if (IS_MCAST(p_entry->hwaddr))  /*if(psta->mac_id ==1)*/
		return _FAIL;

	if (p_entry->rssi_stat.undecorated_smoothed_pwdb == (-1)) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("%s mac_id:%u, mac:"MAC_FMT", rssi == -1\n", __func__, p_entry->mac_id, MAC_ARG(p_entry->hwaddr)));
		return _FAIL;
	}

	cur_tx_ok_cnt = pdvobjpriv->traffic_stat.cur_tx_bytes;
	cur_rx_ok_cnt = pdvobjpriv->traffic_stat.cur_rx_bytes;
	if (cur_rx_ok_cnt > (cur_tx_ok_cnt * 6))
		UL_DL_STATE = 1;
	else
		UL_DL_STATE = 0;

#ifdef CONFIG_BEAMFORMING
	{
#if (BEAMFORMING_SUPPORT == 1)
		enum beamforming_cap beamform_cap = phydm_beamforming_get_entry_beam_cap_by_mac_id(p_dm_odm, p_entry->mac_id);
#else/*for drv beamforming*/
		enum beamforming_cap beamform_cap = beamforming_get_entry_beam_cap_by_mac_id(&adapter->mlmepriv, p_entry->mac_id);
#endif

		if (beamform_cap & (BEAMFORMER_CAP_HT_EXPLICIT | BEAMFORMER_CAP_VHT_SU))
			tx_bf_en = 1;
		else
			tx_bf_en = 0;
	}
#endif /*#ifdef CONFIG_BEAMFORMING*/

	if (tx_bf_en)
		STBC_TX = 0;
	else
		STBC_TX = TEST_FLAG(p_entry->htpriv.stbc_cap, STBC_HT_ENABLE_TX);

	h2c_parameter[0] = (u8)(p_entry->mac_id & 0xFF);
	h2c_parameter[2] = p_entry->rssi_stat.undecorated_smoothed_pwdb & 0x7F;

	if (UL_DL_STATE)
		h2c_parameter[3] |= RAINFO_BE_RX_STATE;

	if (tx_bf_en)
		h2c_parameter[3] |= RAINFO_BF_STATE;
	if (STBC_TX)
		h2c_parameter[3] |= RAINFO_STBC_STATE;
	if (p_dm_odm->noisy_decision)
		h2c_parameter[3] |= RAINFO_NOISY_STATE;

	if ((p_entry->ra_rpt_linked == false) && (p_entry->rssi_stat.is_send_rssi == RA_RSSI_STATE_SEND)) {
		h2c_parameter[3] |= RAINFO_INIT_RSSI_RATE_STATE;
		p_entry->ra_rpt_linked = true;
		p_entry->rssi_stat.is_send_rssi = RA_RSSI_STATE_HOLD;
		first_connect = true;
	}

	h2c_parameter[4] = (p_ra_table->RA_threshold_offset & 0x7f) | (p_ra_table->RA_offset_direction << 7);
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("RA_threshold_offset = (( %s%d ))\n", ((p_ra_table->RA_threshold_offset == 0) ? " " : ((p_ra_table->RA_offset_direction) ? "+" : "-")), p_ra_table->RA_threshold_offset));

	if (first_connect) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("%s mac_id:%u, mac:"MAC_FMT", rssi:%d\n", __func__,
			p_entry->mac_id, MAC_ARG(p_entry->hwaddr), p_entry->rssi_stat.undecorated_smoothed_pwdb));

		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("%s RAINFO - TP:%s, TxBF:%s, STBC:%s, Noisy:%s, Firstcont:%s\n", __func__,
			(UL_DL_STATE) ? "DL" : "UL", (tx_bf_en) ? "EN" : "DIS", (STBC_TX) ? "EN" : "DIS",
			(p_dm_odm->noisy_decision) ? "True" : "False", (first_connect) ? "True" : "False"));
	}

	if (p_hal_data->fw_ractrl == true) {
		if (p_dm_odm->support_ic_type == ODM_RTL8188E)
			cmdlen = 3;
		odm_fill_h2c_cmd(p_dm_odm, ODM_H2C_RSSI_REPORT, cmdlen, h2c_parameter);
	} else {
#if ((RATE_ADAPTIVE_SUPPORT == 1))
		if (p_dm_odm->support_ic_type == ODM_RTL8188E)
			odm_ra_set_rssi_8188e(p_dm_odm, (u8)(p_entry->mac_id & 0xFF), p_entry->rssi_stat.undecorated_smoothed_pwdb & 0x7F);
#endif
	}
	return _SUCCESS;
}

static void phydm_ra_rssi_rpt_wk_hdl(void *p_context)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_context;
	int i;
	u8 mac_id = 0xFF;
	struct sta_info	*p_entry = NULL;

	for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
		p_entry = p_dm_odm->p_odm_sta_info[i];
		if (IS_STA_VALID(p_entry)) {
			if (IS_MCAST(p_entry->hwaddr))  /*if(psta->mac_id ==1)*/
				continue;
			if (p_entry->ra_rpt_linked == false) {
				mac_id = i;
				break;
			}
		}
	}
	if (mac_id != 0xFF)
		phydm_rssi_report(p_dm_odm, mac_id);
}
void phydm_ra_rssi_rpt_wk(void *p_context)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_context;

	rtw_run_in_thread_cmd(p_dm_odm->adapter, phydm_ra_rssi_rpt_wk_hdl, p_dm_odm);
}

static void
odm_rssi_monitor_check_ce(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ADAPTER		*adapter = p_dm_odm->adapter;
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(adapter);
	struct sta_info           *p_entry;
	int	i;
	int	tmp_entry_max_pwdb = 0, tmp_entry_min_pwdb = 0xff;
	u8	sta_cnt = 0;

	if (p_dm_odm->is_linked != true)
		return;

	for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {
		p_entry = p_dm_odm->p_odm_sta_info[i];
		if (IS_STA_VALID(p_entry)) {
			if (IS_MCAST(p_entry->hwaddr))  /*if(psta->mac_id ==1)*/
				continue;

			if (p_entry->rssi_stat.undecorated_smoothed_pwdb == (-1))
				continue;

			if (p_entry->rssi_stat.undecorated_smoothed_pwdb < tmp_entry_min_pwdb)
				tmp_entry_min_pwdb = p_entry->rssi_stat.undecorated_smoothed_pwdb;

			if (p_entry->rssi_stat.undecorated_smoothed_pwdb > tmp_entry_max_pwdb)
				tmp_entry_max_pwdb = p_entry->rssi_stat.undecorated_smoothed_pwdb;

			if (phydm_rssi_report(p_dm_odm, i))
				sta_cnt++;
		}
	}

	if (tmp_entry_max_pwdb != 0)	/* If associated entry is found */
		p_hal_data->entry_max_undecorated_smoothed_pwdb = tmp_entry_max_pwdb;
	else
		p_hal_data->entry_max_undecorated_smoothed_pwdb = 0;

	if (tmp_entry_min_pwdb != 0xff) /* If associated entry is found */
		p_hal_data->entry_min_undecorated_smoothed_pwdb = tmp_entry_min_pwdb;
	else
		p_hal_data->entry_min_undecorated_smoothed_pwdb = 0;

	find_minimum_rssi(adapter);/* get pdmpriv->min_undecorated_pwdb_for_dm */

	p_dm_odm->rssi_min = p_hal_data->min_undecorated_pwdb_for_dm;
	/* odm_cmn_info_update(&p_hal_data->odmpriv,ODM_CMNINFO_RSSI_MIN, pdmpriv->min_undecorated_pwdb_for_dm); */
}

static void
odm_rssi_monitor_check_ap(
	void		*p_dm_void
)
{
}

void
odm_rssi_monitor_check(
	void		*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	if (!(p_dm_odm->support_ability & ODM_BB_RSSI_MONITOR))
		return;

	switch	(p_dm_odm->support_platform) {
	case	ODM_WIN:
		odm_rssi_monitor_check_mp(p_dm_odm);
		break;

	case	ODM_CE:
		odm_rssi_monitor_check_ce(p_dm_odm);
		break;

	case	ODM_AP:
		odm_rssi_monitor_check_ap(p_dm_odm);
		break;

	default:
		break;
	}

}

void
odm_rate_adaptive_mask_init(
	void	*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ODM_RATE_ADAPTIVE	*p_odm_ra = &p_dm_odm->rate_adaptive;

	p_odm_ra->type = dm_type_by_driver;
	if (p_odm_ra->type == dm_type_by_driver)
		p_dm_odm->is_use_ra_mask = true;
	else
		p_dm_odm->is_use_ra_mask = false;

	p_odm_ra->ratr_state = DM_RATR_STA_INIT;

	p_odm_ra->ldpc_thres = 35;
	p_odm_ra->is_use_ldpc = false;

	p_odm_ra->high_rssi_thresh = 50;
	p_odm_ra->low_rssi_thresh = 20;
}
/*-----------------------------------------------------------------------------
 * Function:	odm_refresh_rate_adaptive_mask()
 *
 * Overview:	Update rate table mask according to rssi
 *
 * Input:		NONE
 *
 * Output:		NONE
 *
 * Return:		NONE
 *
 * Revised History:
 *	When		Who		Remark
 *	05/27/2009	hpfan	Create version 0.
 *
 *---------------------------------------------------------------------------*/
void
odm_refresh_rate_adaptive_mask(
	void	*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_			*p_ra_table = &p_dm_odm->dm_ra_table;

	if (!p_dm_odm->is_linked)
		return;

	if (!(p_dm_odm->support_ability & ODM_BB_RA_MASK)) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_TRACE, ("odm_refresh_rate_adaptive_mask(): Return cos not supported\n"));
		return;
	}

	p_ra_table->force_update_ra_mask_count++;
	/*  */
	/* 2011/09/29 MH In HW integration first stage, we provide 4 different handle to operate */
	/* at the same time. In the stage2/3, we need to prive universal interface and merge all */
	/* HW dynamic mechanism. */
	/*  */
	switch	(p_dm_odm->support_platform) {
	case	ODM_WIN:
		odm_refresh_rate_adaptive_mask_mp(p_dm_odm);
		break;

	case	ODM_CE:
		odm_refresh_rate_adaptive_mask_ce(p_dm_odm);
		break;

	case	ODM_AP:
		odm_refresh_rate_adaptive_mask_apadsl(p_dm_odm);
		break;
	}

}

static u8
phydm_trans_platform_bw(
	void		*p_dm_void,
	u8		BW
)
{
	if (BW == CHANNEL_WIDTH_20)
		BW = PHYDM_BW_20;

	else if (BW == CHANNEL_WIDTH_40)
		BW = PHYDM_BW_40;

	else if (BW == CHANNEL_WIDTH_80)
		BW = PHYDM_BW_80;

	else if (BW == CHANNEL_WIDTH_160)
		BW = PHYDM_BW_160;

	else if (BW == CHANNEL_WIDTH_80_80)
		BW = PHYDM_BW_80_80;

	return BW;
}

static u8
phydm_trans_platform_rf_type(
	void		*p_dm_void,
	u8		rf_type
)
{
	if (rf_type == RF_1T2R)
		rf_type = PHYDM_RF_1T2R;

	else if (rf_type == RF_2T4R)
		rf_type = PHYDM_RF_2T4R;

	else if (rf_type == RF_2T2R)
		rf_type = PHYDM_RF_2T2R;

	else if (rf_type == RF_1T1R)
		rf_type = PHYDM_RF_1T1R;

	else if (rf_type == RF_2T2R_GREEN)
		rf_type = PHYDM_RF_2T2R_GREEN;

	else if (rf_type == RF_3T3R)
		rf_type = PHYDM_RF_3T3R;

	else if (rf_type == RF_4T4R)
		rf_type = PHYDM_RF_4T4R;

	else if (rf_type == RF_2T3R)
		rf_type = PHYDM_RF_1T2R;

	else if (rf_type == RF_3T4R)
		rf_type = PHYDM_RF_3T4R;

	return rf_type;
}

static u32
phydm_trans_platform_wireless_mode(
	void		*p_dm_void,
	u32		wireless_mode
)
{
	if (wireless_mode == WIRELESS_11A)
		wireless_mode = PHYDM_WIRELESS_MODE_A;

	else if (wireless_mode == WIRELESS_11B)
		wireless_mode = PHYDM_WIRELESS_MODE_B;

	else if ((wireless_mode == WIRELESS_11G) || (wireless_mode == WIRELESS_11BG))
		wireless_mode = PHYDM_WIRELESS_MODE_G;

	else if (wireless_mode == WIRELESS_AUTO)
		wireless_mode = PHYDM_WIRELESS_MODE_AUTO;

	else if ((wireless_mode == WIRELESS_11_24N) ||
		 (wireless_mode == WIRELESS_11G_24N) ||
		 (wireless_mode == WIRELESS_11B_24N) ||
		 (wireless_mode == WIRELESS_11BG_24N) ||
		 (wireless_mode == WIRELESS_MODE_24G) ||
		 (wireless_mode == WIRELESS_11ABGN) ||
		 (wireless_mode == WIRELESS_11AGN))
		wireless_mode = PHYDM_WIRELESS_MODE_N_24G;

	else if ((wireless_mode == WIRELESS_11_5N) || (wireless_mode == WIRELESS_11A_5N))
		wireless_mode = PHYDM_WIRELESS_MODE_N_5G;

	else if ((wireless_mode == WIRELESS_11AC) || (wireless_mode == WIRELESS_11_5AC) || (wireless_mode == WIRELESS_MODE_5G))
		wireless_mode = PHYDM_WIRELESS_MODE_AC_5G;

	else if (wireless_mode == WIRELESS_11_24AC)
		wireless_mode = PHYDM_WIRELESS_MODE_AC_24G;

	else if (wireless_mode == WIRELESS_11AC)
		wireless_mode = PHYDM_WIRELESS_MODE_AC_ONLY;

	else if (wireless_mode == WIRELESS_MODE_MAX)
		wireless_mode = PHYDM_WIRELESS_MODE_MAX;
	else
		wireless_mode = PHYDM_WIRELESS_MODE_UNKNOWN;

	return wireless_mode;
}

u8
phydm_vht_en_mapping(
	void			*p_dm_void,
	u32			wireless_mode
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8			vht_en_out = 0;

	if ((wireless_mode == PHYDM_WIRELESS_MODE_AC_5G) ||
	    (wireless_mode == PHYDM_WIRELESS_MODE_AC_24G) ||
	    (wireless_mode == PHYDM_WIRELESS_MODE_AC_ONLY)
	   ) {
		vht_en_out = 1;
		/**/
	}

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("wireless_mode= (( 0x%x )), VHT_EN= (( %d ))\n", wireless_mode, vht_en_out));
	return vht_en_out;
}

u8
phydm_rate_id_mapping(
	void			*p_dm_void,
	u32			wireless_mode,
	u8			rf_type,
	u8			bw
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8			rate_id_idx = 0;
	u8			phydm_BW;
	u8			phydm_rf_type;

	phydm_BW = phydm_trans_platform_bw(p_dm_odm, bw);
	phydm_rf_type = phydm_trans_platform_rf_type(p_dm_odm, rf_type);
	wireless_mode = phydm_trans_platform_wireless_mode(p_dm_odm, wireless_mode);

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("wireless_mode= (( 0x%x )), rf_type = (( 0x%x )), BW = (( 0x%x ))\n",
			wireless_mode, phydm_rf_type, phydm_BW));


	switch (wireless_mode) {

	case PHYDM_WIRELESS_MODE_N_24G:
	{

		if (phydm_BW == PHYDM_BW_40) {

			if (phydm_rf_type == PHYDM_RF_1T1R)
				rate_id_idx = PHYDM_BGN_40M_1SS;
			else if (phydm_rf_type == PHYDM_RF_2T2R)
				rate_id_idx = PHYDM_BGN_40M_2SS;
			else
				rate_id_idx = PHYDM_ARFR5_N_3SS;

		} else {

			if (phydm_rf_type == PHYDM_RF_1T1R)
				rate_id_idx = PHYDM_BGN_20M_1SS;
			else if (phydm_rf_type == PHYDM_RF_2T2R)
				rate_id_idx = PHYDM_BGN_20M_2SS;
			else
				rate_id_idx = PHYDM_ARFR5_N_3SS;
		}
	}
	break;

	case PHYDM_WIRELESS_MODE_N_5G:
	{
		if (phydm_rf_type == PHYDM_RF_1T1R)
			rate_id_idx = PHYDM_GN_N1SS;
		else if (phydm_rf_type == PHYDM_RF_2T2R)
			rate_id_idx = PHYDM_GN_N2SS;
		else
			rate_id_idx = PHYDM_ARFR5_N_3SS;
	}

	break;

	case PHYDM_WIRELESS_MODE_G:
		rate_id_idx = PHYDM_BG;
		break;

	case PHYDM_WIRELESS_MODE_A:
		rate_id_idx = PHYDM_G;
		break;

	case PHYDM_WIRELESS_MODE_B:
		rate_id_idx = PHYDM_B_20M;
		break;


	case PHYDM_WIRELESS_MODE_AC_5G:
	case PHYDM_WIRELESS_MODE_AC_ONLY:
	{
		if (phydm_rf_type == PHYDM_RF_1T1R)
			rate_id_idx = PHYDM_ARFR1_AC_1SS;
		else if (phydm_rf_type == PHYDM_RF_2T2R)
			rate_id_idx = PHYDM_ARFR0_AC_2SS;
		else
			rate_id_idx = PHYDM_ARFR4_AC_3SS;
	}
	break;

	case PHYDM_WIRELESS_MODE_AC_24G:
	{
		/*Becareful to set "Lowest rate" while using PHYDM_ARFR4_AC_3SS in 2.4G/5G*/
		if (phydm_BW >= PHYDM_BW_80) {
			if (phydm_rf_type == PHYDM_RF_1T1R)
				rate_id_idx = PHYDM_ARFR1_AC_1SS;
			else if (phydm_rf_type == PHYDM_RF_2T2R)
				rate_id_idx = PHYDM_ARFR0_AC_2SS;
			else
				rate_id_idx = PHYDM_ARFR4_AC_3SS;
		} else {

			if (phydm_rf_type == PHYDM_RF_1T1R)
				rate_id_idx = PHYDM_ARFR2_AC_2G_1SS;
			else if (phydm_rf_type == PHYDM_RF_2T2R)
				rate_id_idx = PHYDM_ARFR3_AC_2G_2SS;
			else
				rate_id_idx = PHYDM_ARFR4_AC_3SS;
		}
	}
	break;

	default:
		rate_id_idx = 0;
		break;
	}

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("RA rate ID = (( 0x%x ))\n", rate_id_idx));

	return rate_id_idx;
}

void
phydm_update_hal_ra_mask(
	void			*p_dm_void,
	u32			wireless_mode,
	u8			rf_type,
	u8			BW,
	u8			mimo_ps_enable,
	u8			disable_cck_rate,
	u32			*ratr_bitmap_msb_in,
	u32			*ratr_bitmap_lsb_in,
	u8			tx_rate_level
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u32			mask_rate_threshold;
	u8			phydm_rf_type;
	u8			phydm_BW;
	u32			ratr_bitmap = *ratr_bitmap_lsb_in, ratr_bitmap_msb = *ratr_bitmap_msb_in;

	wireless_mode = phydm_trans_platform_wireless_mode(p_dm_odm, wireless_mode);

	phydm_rf_type = phydm_trans_platform_rf_type(p_dm_odm, rf_type);
	phydm_BW = phydm_trans_platform_bw(p_dm_odm, BW);

	/*ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("phydm_rf_type = (( %x )), rf_type = (( %x ))\n", phydm_rf_type, rf_type));*/
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("Platfoem original RA Mask = (( 0x %x | %x ))\n", ratr_bitmap_msb, ratr_bitmap));

	switch (wireless_mode) {

	case PHYDM_WIRELESS_MODE_B:
	{
		ratr_bitmap &= 0x0000000f;
	}
	break;

	case PHYDM_WIRELESS_MODE_G:
	{
		ratr_bitmap &= 0x00000ff5;
	}
	break;

	case PHYDM_WIRELESS_MODE_A:
	{
		ratr_bitmap &= 0x00000ff0;
	}
	break;

	case PHYDM_WIRELESS_MODE_N_24G:
	case PHYDM_WIRELESS_MODE_N_5G:
	{
		if (mimo_ps_enable)
			phydm_rf_type = PHYDM_RF_1T1R;

		if (phydm_rf_type == PHYDM_RF_1T1R) {

			if (phydm_BW == PHYDM_BW_40)
				ratr_bitmap &= 0x000ff015;
			else
				ratr_bitmap &= 0x000ff005;
		} else if (phydm_rf_type == PHYDM_RF_2T2R || phydm_rf_type == PHYDM_RF_2T4R || phydm_rf_type == PHYDM_RF_2T3R) {

			if (phydm_BW == PHYDM_BW_40)
				ratr_bitmap &= 0x0ffff015;
			else
				ratr_bitmap &= 0x0ffff005;
		} else { /*3T*/

			ratr_bitmap &= 0xfffff015;
			ratr_bitmap_msb &= 0xf;
		}
	}
	break;

	case PHYDM_WIRELESS_MODE_AC_24G:
	{
		if (phydm_rf_type == PHYDM_RF_1T1R)
			ratr_bitmap &= 0x003ff015;
		else if (phydm_rf_type == PHYDM_RF_2T2R || phydm_rf_type == PHYDM_RF_2T4R || phydm_rf_type == PHYDM_RF_2T3R)
			ratr_bitmap &= 0xfffff015;
		else {/*3T*/

			ratr_bitmap &= 0xfffff010;
			ratr_bitmap_msb &= 0x3ff;
		}

		if (phydm_BW == PHYDM_BW_20) {/* AC 20MHz doesn't support MCS9 */
			ratr_bitmap &= 0x7fdfffff;
			ratr_bitmap_msb &= 0x1ff;
		}
	}
	break;

	case PHYDM_WIRELESS_MODE_AC_5G:
	{
		if (phydm_rf_type == PHYDM_RF_1T1R)
			ratr_bitmap &= 0x003ff010;
		else if (phydm_rf_type == PHYDM_RF_2T2R || phydm_rf_type == PHYDM_RF_2T4R || phydm_rf_type == PHYDM_RF_2T3R)
			ratr_bitmap &= 0xfffff010;
		else {/*3T*/

			ratr_bitmap &= 0xfffff010;
			ratr_bitmap_msb &= 0x3ff;
		}

		if (phydm_BW == PHYDM_BW_20) {/* AC 20MHz doesn't support MCS9 */
			ratr_bitmap &= 0x7fdfffff;
			ratr_bitmap_msb &= 0x1ff;
		}
	}
	break;

	default:
		break;
	}

	if (wireless_mode != PHYDM_WIRELESS_MODE_B) {

		if (tx_rate_level == 0)
			ratr_bitmap &=  0xffffffff;
		else if (tx_rate_level == 1)
			ratr_bitmap &=  0xfffffff0;
		else if (tx_rate_level == 2)
			ratr_bitmap &=  0xffffefe0;
		else if (tx_rate_level == 3)
			ratr_bitmap &=  0xffffcfc0;
		else if (tx_rate_level == 4)
			ratr_bitmap &=  0xffff8f80;
		else if (tx_rate_level >= 5)
			ratr_bitmap &=  0xffff0f00;

	}

	if (disable_cck_rate)
		ratr_bitmap &= 0xfffffff0;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("wireless_mode= (( 0x%x )), rf_type = (( 0x%x )), BW = (( 0x%x )), MimoPs_en = (( %d )), tx_rate_level= (( 0x%x ))\n",
		wireless_mode, phydm_rf_type, phydm_BW, mimo_ps_enable, tx_rate_level));

	/*ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("111 Phydm modified RA Mask = (( 0x %x | %x ))\n", ratr_bitmap_msb, ratr_bitmap));*/

	*ratr_bitmap_lsb_in = ratr_bitmap;
	*ratr_bitmap_msb_in = ratr_bitmap_msb;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("Phydm modified RA Mask = (( 0x %x | %x ))\n", *ratr_bitmap_msb_in, *ratr_bitmap_lsb_in));

}

u8
phydm_RA_level_decision(
	void			*p_dm_void,
	u32			rssi,
	u8			ratr_state
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8	ra_lowest_rate;
	u8	ra_rate_floor_table[RA_FLOOR_TABLE_SIZE] = {20, 34, 38, 42, 46, 50, 100}; /*MCS0 ~ MCS4 , VHT1SS MCS0 ~ MCS4 , G 6M~24M*/
	u8	new_ratr_state = 0;
	u8	i;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("curr RA level = ((%d)), Rate_floor_table ori [ %d , %d, %d , %d, %d, %d]\n", ratr_state,
		ra_rate_floor_table[0], ra_rate_floor_table[1], ra_rate_floor_table[2], ra_rate_floor_table[3], ra_rate_floor_table[4], ra_rate_floor_table[5]));

	for (i = 0; i < RA_FLOOR_TABLE_SIZE; i++) {

		if (i >= (ratr_state))
			ra_rate_floor_table[i] += RA_FLOOR_UP_GAP;
	}

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("RSSI = ((%d)), Rate_floor_table_mod [ %d , %d, %d , %d, %d, %d]\n",
		rssi, ra_rate_floor_table[0], ra_rate_floor_table[1], ra_rate_floor_table[2], ra_rate_floor_table[3], ra_rate_floor_table[4], ra_rate_floor_table[5]));

	for (i = 0; i < RA_FLOOR_TABLE_SIZE; i++) {

		if (rssi < ra_rate_floor_table[i]) {
			new_ratr_state = i;
			break;
		}
	}



	return	new_ratr_state;

}

void
odm_refresh_rate_adaptive_mask_mp(
	void		*p_dm_void
)
{
}

void
odm_refresh_rate_adaptive_mask_ce(
	void	*p_dm_void
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_			*p_ra_table = &p_dm_odm->dm_ra_table;
	struct _ADAPTER	*p_adapter	 =  p_dm_odm->adapter;
	struct _ODM_RATE_ADAPTIVE		*p_ra = &p_dm_odm->rate_adaptive;
	u32		i;
	struct sta_info *p_entry;
	u8		ratr_state_new;

	if (RTW_CANNOT_RUN(p_adapter)) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_TRACE, ("<---- odm_refresh_rate_adaptive_mask(): driver is going to unload\n"));
		return;
	}

	if (!p_dm_odm->is_use_ra_mask) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("<---- odm_refresh_rate_adaptive_mask(): driver does not control rate adaptive mask\n"));
		return;
	}

	for (i = 0; i < ODM_ASSOCIATE_ENTRY_NUM; i++) {

		p_entry = p_dm_odm->p_odm_sta_info[i];

		if (IS_STA_VALID(p_entry)) {

			if (IS_MCAST(p_entry->hwaddr))
				continue;

#if ((RTL8812A_SUPPORT == 1) || (RTL8821A_SUPPORT == 1))
			if ((p_dm_odm->support_ic_type == ODM_RTL8812) || (p_dm_odm->support_ic_type == ODM_RTL8821)) {
				if (p_entry->rssi_stat.undecorated_smoothed_pwdb < p_ra->ldpc_thres) {
					p_ra->is_use_ldpc = true;
					p_ra->is_lower_rts_rate = true;
					if ((p_dm_odm->support_ic_type == ODM_RTL8821) && (p_dm_odm->cut_version == ODM_CUT_A))
						set_ra_ldpc_8812(p_entry, true);
					/* dbg_print("RSSI=%d, is_use_ldpc = true\n", p_hal_data->undecorated_smoothed_pwdb); */
				} else if (p_entry->rssi_stat.undecorated_smoothed_pwdb > (p_ra->ldpc_thres - 5)) {
					p_ra->is_use_ldpc = false;
					p_ra->is_lower_rts_rate = false;
					if ((p_dm_odm->support_ic_type == ODM_RTL8821) && (p_dm_odm->cut_version == ODM_CUT_A))
						set_ra_ldpc_8812(p_entry, false);
					/* dbg_print("RSSI=%d, is_use_ldpc = false\n", p_hal_data->undecorated_smoothed_pwdb); */
				}
			}
#endif

#if RA_MASK_PHYDMLIZE_CE
			ratr_state_new = phydm_RA_level_decision(p_dm_odm, p_entry->rssi_stat.undecorated_smoothed_pwdb, p_entry->rssi_level);

			if ((p_entry->rssi_level != ratr_state_new) || (p_ra_table->force_update_ra_mask_count >= FORCED_UPDATE_RAMASK_PERIOD)) {

				p_ra_table->force_update_ra_mask_count = 0;
				/*ODM_PRINT_ADDR(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("Target AP addr :"), pstat->hwaddr);*/
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("Update Tx RA Level: ((%x)) -> ((%x)),  RSSI = ((%d))\n",
					p_entry->rssi_level, ratr_state_new, p_entry->rssi_stat.undecorated_smoothed_pwdb));

				p_entry->rssi_level = ratr_state_new;
				rtw_hal_update_ra_mask(p_entry, p_entry->rssi_level, false);
			} else {
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("Stay in RA level  = (( %d ))\n\n", ratr_state_new));
				/**/
			}
#else
			if (odm_ra_state_check(p_dm_odm, p_entry->rssi_stat.undecorated_smoothed_pwdb, false, &p_entry->rssi_level)) {
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("RSSI:%d, RSSI_LEVEL:%d\n", p_entry->rssi_stat.undecorated_smoothed_pwdb, p_entry->rssi_level));
				/* printk("RSSI:%d, RSSI_LEVEL:%d\n", pstat->rssi_stat.undecorated_smoothed_pwdb, pstat->rssi_level); */
				rtw_hal_update_ra_mask(p_entry, p_entry->rssi_level, false);
			} else if (p_dm_odm->is_change_state) {
				ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("Change Power Training state, is_disable_power_training = %d\n", p_dm_odm->is_disable_power_training));
				rtw_hal_update_ra_mask(p_entry, p_entry->rssi_level, false);
			}
#endif

		}
	}
}

void
odm_refresh_rate_adaptive_mask_apadsl(
	void	*p_dm_void
)
{
}

void
odm_refresh_basic_rate_mask(
	void	*p_dm_void
)
{
}

u8
phydm_rate_order_compute(
	void	*p_dm_void,
	u8	rate_idx
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8		rate_order = 0;

	if (rate_idx >= ODM_RATEVHTSS4MCS0) {

		rate_idx -= ODM_RATEVHTSS4MCS0;
		/**/
	} else if (rate_idx >= ODM_RATEVHTSS3MCS0) {

		rate_idx -= ODM_RATEVHTSS3MCS0;
		/**/
	} else if (rate_idx >= ODM_RATEVHTSS2MCS0) {

		rate_idx -= ODM_RATEVHTSS2MCS0;
		/**/
	} else if (rate_idx >= ODM_RATEVHTSS1MCS0) {

		rate_idx -= ODM_RATEVHTSS1MCS0;
		/**/
	} else if (rate_idx >= ODM_RATEMCS24) {

		rate_idx -= ODM_RATEMCS24;
		/**/
	} else if (rate_idx >= ODM_RATEMCS16) {

		rate_idx -= ODM_RATEMCS16;
		/**/
	} else if (rate_idx >= ODM_RATEMCS8) {

		rate_idx -= ODM_RATEMCS8;
		/**/
	}
	rate_order = rate_idx;

	return rate_order;

}

static void
phydm_ra_common_info_update(
	void	*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_		*p_ra_table = &p_dm_odm->dm_ra_table;
	u16		macid;
	u8		rate_order_tmp;
	u8		cnt = 0;

	p_ra_table->highest_client_tx_order = 0;
	p_ra_table->power_tracking_flag = 1;

	if (p_dm_odm->number_linked_client != 0) {
		for (macid = 0; macid < ODM_ASSOCIATE_ENTRY_NUM; macid++) {

			rate_order_tmp = phydm_rate_order_compute(p_dm_odm, ((p_ra_table->link_tx_rate[macid]) & 0x7f));

			if (rate_order_tmp >= (p_ra_table->highest_client_tx_order)) {
				p_ra_table->highest_client_tx_order = rate_order_tmp;
				p_ra_table->highest_client_tx_rate_order = macid;
			}

			cnt++;

			if (cnt == p_dm_odm->number_linked_client)
				break;
		}
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("MACID[%d], Highest Tx order Update for power traking: %d\n", (p_ra_table->highest_client_tx_rate_order), (p_ra_table->highest_client_tx_order)));
	}
}

void
phydm_ra_info_watchdog(
	void	*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	phydm_ra_common_info_update(p_dm_odm);
	phydm_ra_dynamic_retry_limit(p_dm_odm);
	phydm_ra_dynamic_retry_count(p_dm_odm);
	odm_refresh_rate_adaptive_mask(p_dm_odm);
	odm_refresh_basic_rate_mask(p_dm_odm);
}

void
phydm_ra_info_init(
	void	*p_dm_void
)
{
	struct PHY_DM_STRUCT	*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _rate_adaptive_table_		*p_ra_table = &p_dm_odm->dm_ra_table;

	p_ra_table->highest_client_tx_rate_order = 0;
	p_ra_table->highest_client_tx_order = 0;
	p_ra_table->RA_threshold_offset = 0;
	p_ra_table->RA_offset_direction = 0;

#if (defined(CONFIG_RA_DYNAMIC_RTY_LIMIT))
	phydm_ra_dynamic_retry_limit_init(p_dm_odm);
#endif

#if (defined(CONFIG_RA_DYNAMIC_RATE_ID))
	phydm_ra_dynamic_rate_id_init(p_dm_odm);
#endif
#if (defined(CONFIG_RA_DBG_CMD))
	odm_ra_para_adjust_init(p_dm_odm);
#endif

}

u8
odm_find_rts_rate(
	void			*p_dm_void,
	u8			tx_rate,
	bool			is_erp_protect
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8	rts_ini_rate = ODM_RATE6M;

	if (is_erp_protect) /* use CCK rate as RTS*/
		rts_ini_rate = ODM_RATE1M;
	else {
		switch (tx_rate) {
		case ODM_RATEVHTSS3MCS9:
		case ODM_RATEVHTSS3MCS8:
		case ODM_RATEVHTSS3MCS7:
		case ODM_RATEVHTSS3MCS6:
		case ODM_RATEVHTSS3MCS5:
		case ODM_RATEVHTSS3MCS4:
		case ODM_RATEVHTSS3MCS3:
		case ODM_RATEVHTSS2MCS9:
		case ODM_RATEVHTSS2MCS8:
		case ODM_RATEVHTSS2MCS7:
		case ODM_RATEVHTSS2MCS6:
		case ODM_RATEVHTSS2MCS5:
		case ODM_RATEVHTSS2MCS4:
		case ODM_RATEVHTSS2MCS3:
		case ODM_RATEVHTSS1MCS9:
		case ODM_RATEVHTSS1MCS8:
		case ODM_RATEVHTSS1MCS7:
		case ODM_RATEVHTSS1MCS6:
		case ODM_RATEVHTSS1MCS5:
		case ODM_RATEVHTSS1MCS4:
		case ODM_RATEVHTSS1MCS3:
		case ODM_RATEMCS15:
		case ODM_RATEMCS14:
		case ODM_RATEMCS13:
		case ODM_RATEMCS12:
		case ODM_RATEMCS11:
		case ODM_RATEMCS7:
		case ODM_RATEMCS6:
		case ODM_RATEMCS5:
		case ODM_RATEMCS4:
		case ODM_RATEMCS3:
		case ODM_RATE54M:
		case ODM_RATE48M:
		case ODM_RATE36M:
		case ODM_RATE24M:
			rts_ini_rate = ODM_RATE24M;
			break;
		case ODM_RATEVHTSS3MCS2:
		case ODM_RATEVHTSS3MCS1:
		case ODM_RATEVHTSS2MCS2:
		case ODM_RATEVHTSS2MCS1:
		case ODM_RATEVHTSS1MCS2:
		case ODM_RATEVHTSS1MCS1:
		case ODM_RATEMCS10:
		case ODM_RATEMCS9:
		case ODM_RATEMCS2:
		case ODM_RATEMCS1:
		case ODM_RATE18M:
		case ODM_RATE12M:
			rts_ini_rate = ODM_RATE12M;
			break;
		case ODM_RATEVHTSS3MCS0:
		case ODM_RATEVHTSS2MCS0:
		case ODM_RATEVHTSS1MCS0:
		case ODM_RATEMCS8:
		case ODM_RATEMCS0:
		case ODM_RATE9M:
		case ODM_RATE6M:
			rts_ini_rate = ODM_RATE6M;
			break;
		case ODM_RATE11M:
		case ODM_RATE5_5M:
		case ODM_RATE2M:
		case ODM_RATE1M:
			rts_ini_rate = ODM_RATE1M;
			break;
		default:
			rts_ini_rate = ODM_RATE6M;
			break;
		}
	}

	if (*p_dm_odm->p_band_type == 1) {
		if (rts_ini_rate < ODM_RATE6M)
			rts_ini_rate = ODM_RATE6M;
	}
	return rts_ini_rate;

}

static void
odm_set_ra_dm_arfb_by_noisy(
	struct PHY_DM_STRUCT	*p_dm_odm
)
{
}

void
odm_update_noisy_state(
	void		*p_dm_void,
	bool		is_noisy_state_from_c2h
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	if (p_dm_odm->support_ic_type == ODM_RTL8821  || p_dm_odm->support_ic_type == ODM_RTL8812  ||
	    p_dm_odm->support_ic_type == ODM_RTL8723B || p_dm_odm->support_ic_type == ODM_RTL8192E || p_dm_odm->support_ic_type == ODM_RTL8188E || p_dm_odm->support_ic_type == ODM_RTL8723D)
		p_dm_odm->is_noisy_state = is_noisy_state_from_c2h;
	odm_set_ra_dm_arfb_by_noisy(p_dm_odm);
};

void
phydm_update_pwr_track(
	void		*p_dm_void,
	u8		rate
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8			path_idx = 0;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_TX_PWR_TRACK, ODM_DBG_LOUD, ("Pwr Track Get rate=0x%x\n", rate));

	p_dm_odm->tx_rate = rate;
}


static void
find_minimum_rssi(
	struct _ADAPTER	*p_adapter
)
{
	HAL_DATA_TYPE	*p_hal_data = GET_HAL_DATA(p_adapter);
	struct PHY_DM_STRUCT		*p_dm_odm = &(p_hal_data->odmpriv);

	/*Determine the minimum RSSI*/

	if ((p_dm_odm->is_linked != true) &&
	    (p_hal_data->entry_min_undecorated_smoothed_pwdb == 0)) {
		p_hal_data->min_undecorated_pwdb_for_dm = 0;
		/*ODM_RT_TRACE(p_dm_odm,COMP_BB_POWERSAVING, DBG_LOUD, ("Not connected to any\n"));*/
	} else
		p_hal_data->min_undecorated_pwdb_for_dm = p_hal_data->entry_min_undecorated_smoothed_pwdb;

	/*DBG_8192C("%s=>min_undecorated_pwdb_for_dm(%d)\n",__func__,pdmpriv->min_undecorated_pwdb_for_dm);*/
	/*ODM_RT_TRACE(p_dm_odm,COMP_DIG, DBG_LOUD, ("min_undecorated_pwdb_for_dm =%d\n",p_hal_data->min_undecorated_pwdb_for_dm));*/
}

u64
phydm_get_rate_bitmap_ex(
	void		*p_dm_void,
	u32		macid,
	u64		ra_mask,
	u8		rssi_level,
	u64	*dm_ra_mask,
	u8	*dm_rte_id
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct sta_info	*p_entry;
	u64	rate_bitmap = 0;
	u8	wireless_mode;

	p_entry = p_dm_odm->p_odm_sta_info[macid];
	if (!IS_STA_VALID(p_entry))
		return ra_mask;
	wireless_mode = p_entry->wireless_mode;
	switch (wireless_mode) {
	case ODM_WM_B:
		if (ra_mask & 0x000000000000000c) /* 11M or 5.5M enable */
			rate_bitmap = 0x000000000000000d;
		else
			rate_bitmap = 0x000000000000000f;
		break;

	case (ODM_WM_G):
	case (ODM_WM_A):
		if (rssi_level == DM_RATR_STA_HIGH)
			rate_bitmap = 0x0000000000000f00;
		else
			rate_bitmap = 0x0000000000000ff0;
		break;

	case (ODM_WM_B|ODM_WM_G):
		if (rssi_level == DM_RATR_STA_HIGH)
			rate_bitmap = 0x0000000000000f00;
		else if (rssi_level == DM_RATR_STA_MIDDLE)
			rate_bitmap = 0x0000000000000ff0;
		else
			rate_bitmap = 0x0000000000000ff5;
		break;

	case (ODM_WM_B|ODM_WM_G|ODM_WM_N24G):
	case (ODM_WM_B|ODM_WM_N24G):
	case (ODM_WM_G|ODM_WM_N24G):
	case (ODM_WM_A|ODM_WM_N5G):
	{
		if (p_dm_odm->rf_type == ODM_1T2R || p_dm_odm->rf_type == ODM_1T1R) {
			if (rssi_level == DM_RATR_STA_HIGH)
				rate_bitmap = 0x00000000000f0000;
			else if (rssi_level == DM_RATR_STA_MIDDLE)
				rate_bitmap = 0x00000000000ff000;
			else {
				if (*(p_dm_odm->p_band_width) == ODM_BW40M)
					rate_bitmap = 0x00000000000ff015;
				else
					rate_bitmap = 0x00000000000ff005;
			}
		} else if (p_dm_odm->rf_type == ODM_2T2R  || p_dm_odm->rf_type == ODM_2T3R  || p_dm_odm->rf_type == ODM_2T4R) {
			if (rssi_level == DM_RATR_STA_HIGH)
				rate_bitmap = 0x000000000f8f0000;
			else if (rssi_level == DM_RATR_STA_MIDDLE)
				rate_bitmap = 0x000000000f8ff000;
			else {
				if (*(p_dm_odm->p_band_width) == ODM_BW40M)
					rate_bitmap = 0x000000000f8ff015;
				else
					rate_bitmap = 0x000000000f8ff005;
			}
		} else {
			if (rssi_level == DM_RATR_STA_HIGH)
				rate_bitmap = 0x0000000f0f0f0000L;
			else if (rssi_level == DM_RATR_STA_MIDDLE)
				rate_bitmap = 0x0000000fcfcfe000L;
			else {
				if (*(p_dm_odm->p_band_width) == ODM_BW40M)
					rate_bitmap = 0x0000000ffffff015L;
				else
					rate_bitmap = 0x0000000ffffff005L;
			}
		}
	}
	break;

	case (ODM_WM_AC|ODM_WM_G):
		if (rssi_level == 1)
			rate_bitmap = 0x00000000fc3f0000L;
		else if (rssi_level == 2)
			rate_bitmap = 0x00000000fffff000L;
		else
			rate_bitmap = 0x00000000ffffffffL;
		break;

	case (ODM_WM_AC|ODM_WM_A):

		if (p_dm_odm->rf_type == ODM_1T2R || p_dm_odm->rf_type == ODM_1T1R) {
			if (rssi_level == 1)				/* add by Gary for ac-series */
				rate_bitmap = 0x00000000003f8000L;
			else if (rssi_level == 2)
				rate_bitmap = 0x00000000003fe000L;
			else
				rate_bitmap = 0x00000000003ff010L;
		} else if (p_dm_odm->rf_type == ODM_2T2R  || p_dm_odm->rf_type == ODM_2T3R  || p_dm_odm->rf_type == ODM_2T4R) {
			if (rssi_level == 1)				/* add by Gary for ac-series */
				rate_bitmap = 0x00000000fe3f8000L;       /* VHT 2SS MCS3~9 */
			else if (rssi_level == 2)
				rate_bitmap = 0x00000000fffff000L;       /* VHT 2SS MCS0~9 */
			else
				rate_bitmap = 0x00000000fffff010L;       /* All */
		} else {
			if (rssi_level == 1)				/* add by Gary for ac-series */
				rate_bitmap = 0x000003f8fe3f8000ULL;       /* VHT 3SS MCS3~9 */
			else if (rssi_level == 2)
				rate_bitmap = 0x000003fffffff000ULL;       /* VHT3SS MCS0~9 */
			else
				rate_bitmap = 0x000003fffffff010ULL;       /* All */
		}
		break;

	default:
		if (p_dm_odm->rf_type == ODM_1T2R || p_dm_odm->rf_type == ODM_1T1R)
			rate_bitmap = 0x00000000000fffff;
		else if (p_dm_odm->rf_type == ODM_2T2R  || p_dm_odm->rf_type == ODM_2T3R  || p_dm_odm->rf_type == ODM_2T4R)
			rate_bitmap = 0x000000000fffffff;
		else
			rate_bitmap = 0x0000003fffffffffULL;
		break;

	}
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, (" ==> rssi_level:0x%02x, wireless_mode:0x%02x, rate_bitmap:0x%016llx\n", rssi_level, wireless_mode, rate_bitmap));

	return ra_mask & rate_bitmap;
}


u32
odm_get_rate_bitmap(
	void		*p_dm_void,
	u32		macid,
	u32		ra_mask,
	u8		rssi_level
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct sta_info	*p_entry;
	u32	rate_bitmap = 0;
	u8	wireless_mode;
	/* u8 	wireless_mode =*(p_dm_odm->p_wireless_mode); */


	p_entry = p_dm_odm->p_odm_sta_info[macid];
	if (!IS_STA_VALID(p_entry))
		return ra_mask;

	wireless_mode = p_entry->wireless_mode;

	switch (wireless_mode) {
	case ODM_WM_B:
		if (ra_mask & 0x0000000c)		/* 11M or 5.5M enable */
			rate_bitmap = 0x0000000d;
		else
			rate_bitmap = 0x0000000f;
		break;

	case (ODM_WM_G):
	case (ODM_WM_A):
		if (rssi_level == DM_RATR_STA_HIGH)
			rate_bitmap = 0x00000f00;
		else
			rate_bitmap = 0x00000ff0;
		break;

	case (ODM_WM_B|ODM_WM_G):
		if (rssi_level == DM_RATR_STA_HIGH)
			rate_bitmap = 0x00000f00;
		else if (rssi_level == DM_RATR_STA_MIDDLE)
			rate_bitmap = 0x00000ff0;
		else
			rate_bitmap = 0x00000ff5;
		break;

	case (ODM_WM_B|ODM_WM_G|ODM_WM_N24G):
	case (ODM_WM_B|ODM_WM_N24G):
	case (ODM_WM_G|ODM_WM_N24G):
	case (ODM_WM_A|ODM_WM_N5G):
	{
		if (p_dm_odm->rf_type == ODM_1T2R || p_dm_odm->rf_type == ODM_1T1R) {
			if (rssi_level == DM_RATR_STA_HIGH)
				rate_bitmap = 0x000f0000;
			else if (rssi_level == DM_RATR_STA_MIDDLE)
				rate_bitmap = 0x000ff000;
			else {
				if (*(p_dm_odm->p_band_width) == ODM_BW40M)
					rate_bitmap = 0x000ff015;
				else
					rate_bitmap = 0x000ff005;
			}
		} else {
			if (rssi_level == DM_RATR_STA_HIGH)
				rate_bitmap = 0x0f8f0000;
			else if (rssi_level == DM_RATR_STA_MIDDLE)
				rate_bitmap = 0x0f8ff000;
			else {
				if (*(p_dm_odm->p_band_width) == ODM_BW40M)
					rate_bitmap = 0x0f8ff015;
				else
					rate_bitmap = 0x0f8ff005;
			}
		}
	}
	break;

	case (ODM_WM_AC|ODM_WM_G):
		if (rssi_level == 1)
			rate_bitmap = 0xfc3f0000;
		else if (rssi_level == 2)
			rate_bitmap = 0xfffff000;
		else
			rate_bitmap = 0xffffffff;
		break;

	case (ODM_WM_AC|ODM_WM_A):

		if (p_dm_odm->rf_type == RF_1T1R) {
			if (rssi_level == 1)				/* add by Gary for ac-series */
				rate_bitmap = 0x003f8000;
			else if (rssi_level == 2)
				rate_bitmap = 0x003ff000;
			else
				rate_bitmap = 0x003ff010;
		} else {
			if (rssi_level == 1)				/* add by Gary for ac-series */
				rate_bitmap = 0xfe3f8000;       /* VHT 2SS MCS3~9 */
			else if (rssi_level == 2)
				rate_bitmap = 0xfffff000;       /* VHT 2SS MCS0~9 */
			else
				rate_bitmap = 0xfffff010;       /* All */
		}
		break;

	default:
		if (p_dm_odm->rf_type == RF_1T2R)
			rate_bitmap = 0x000fffff;
		else
			rate_bitmap = 0x0fffffff;
		break;

	}

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RATE_ADAPTIVE, ODM_DBG_LOUD, ("%s ==> rssi_level:0x%02x, wireless_mode:0x%02x, rate_bitmap:0x%08x\n", __func__, rssi_level, wireless_mode, rate_bitmap));
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, (" ==> rssi_level:0x%02x, wireless_mode:0x%02x, rate_bitmap:0x%08x\n", rssi_level, wireless_mode, rate_bitmap));

	return ra_mask & rate_bitmap;

}

/* RA_MASK_PHYDMLIZE, will delete it later*/

#if (RA_MASK_PHYDMLIZE_CE || RA_MASK_PHYDMLIZE_AP || RA_MASK_PHYDMLIZE_WIN)

bool
odm_ra_state_check(
	void			*p_dm_void,
	s32			RSSI,
	bool			is_force_update,
	u8			*p_ra_tr_state
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ODM_RATE_ADAPTIVE *p_ra = &p_dm_odm->rate_adaptive;
	const u8 go_up_gap = 5;
	u8 high_rssi_thresh_for_ra = p_ra->high_rssi_thresh;
	u8 low_rssi_thresh_for_ra = p_ra->low_rssi_thresh;
	u8 ratr_state;

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("RSSI= (( %d )), Current_RSSI_level = (( %d ))\n", RSSI, *p_ra_tr_state));
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("[Ori RA RSSI Thresh]  High= (( %d )), Low = (( %d ))\n", high_rssi_thresh_for_ra, low_rssi_thresh_for_ra));
	/* threshold Adjustment:*/
	/* when RSSI state trends to go up one or two levels, make sure RSSI is high enough.*/
	/* Here go_up_gap is added to solve the boundary's level alternation issue.*/
	switch (*p_ra_tr_state) {
	case DM_RATR_STA_INIT:
	case DM_RATR_STA_HIGH:
		break;
	case DM_RATR_STA_MIDDLE:
		high_rssi_thresh_for_ra += go_up_gap;
		break;
	case DM_RATR_STA_LOW:
		high_rssi_thresh_for_ra += go_up_gap;
		low_rssi_thresh_for_ra += go_up_gap;
		break;
	default:
		ODM_RT_ASSERT(p_dm_odm, false, ("wrong rssi level setting %d !", *p_ra_tr_state));
		break;
	}

	/* Decide ratr_state by RSSI.*/
	if (RSSI > high_rssi_thresh_for_ra)
		ratr_state = DM_RATR_STA_HIGH;
	else if (RSSI > low_rssi_thresh_for_ra)
		ratr_state = DM_RATR_STA_MIDDLE;

	else
		ratr_state = DM_RATR_STA_LOW;
	ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("[Mod RA RSSI Thresh]  High= (( %d )), Low = (( %d ))\n", high_rssi_thresh_for_ra, low_rssi_thresh_for_ra));
	if (*p_ra_tr_state != ratr_state || is_force_update) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_RA_MASK, ODM_DBG_LOUD, ("[RSSI Level Update] %d->%d\n", *p_ra_tr_state, ratr_state));
		*p_ra_tr_state = ratr_state;
		return true;
	}

	return false;
}

#endif
