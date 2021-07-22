// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


/* ************************************************************
 * include files
 * ************************************************************ */
#include "mp_precomp.h"
#include "phydm_precomp.h"

#if (defined(CONFIG_PATH_DIVERSITY))
void odm_pathdiv_debug(void		*p_dm_void,
	u32		*const dm_value,
	u32		*_used,
	char			*output,
	u32		*_out_len
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ODM_PATH_DIVERSITY_			*p_dm_path_div  = &(p_dm_odm->dm_path_div);
	u32 used = *_used;
	u32 out_len = *_out_len;

	p_dm_odm->path_select = (dm_value[0] & 0xf);
	PHYDM_SNPRINTF((output + used, out_len - used, "Path_select = (( 0x%x ))\n", p_dm_odm->path_select));

	/* 2 [Fix path] */
	if (p_dm_odm->path_select != PHYDM_AUTO_PATH) {
		PHYDM_SNPRINTF((output + used, out_len - used, "Trun on path  [%s%s%s%s]\n",
				((p_dm_odm->path_select) & 0x1) ? "A" : "",
				((p_dm_odm->path_select) & 0x2) ? "B" : "",
				((p_dm_odm->path_select) & 0x4) ? "C" : "",
				((p_dm_odm->path_select) & 0x8) ? "D" : ""));

		phydm_dtp_fix_tx_path(p_dm_odm, p_dm_odm->path_select);
	} else
		PHYDM_SNPRINTF((output + used, out_len - used, "%s\n", "Auto path"));
}

#endif /*  #if(defined(CONFIG_PATH_DIVERSITY)) */

void
phydm_c2h_dtp_handler(
	void	*p_dm_void,
	u8   *cmd_buf,
	u8	cmd_len
)
{
#if (defined(CONFIG_PATH_DIVERSITY))
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	struct _ODM_PATH_DIVERSITY_		*p_dm_path_div  = &(p_dm_odm->dm_path_div);

	u8  macid = cmd_buf[0];
	u8  target = cmd_buf[1];
	u8  nsc_1 = cmd_buf[2];
	u8  nsc_2 = cmd_buf[3];
	u8  nsc_3 = cmd_buf[4];

	ODM_RT_TRACE(p_dm_odm, ODM_COMP_PATH_DIV, ODM_DBG_LOUD, ("Target_candidate = (( %d ))\n", target));
#endif
}

void
odm_path_diversity(
	void	*p_dm_void
)
{
#if (defined(CONFIG_PATH_DIVERSITY))
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	if (!(p_dm_odm->support_ability & ODM_BB_PATH_DIV)) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_PATH_DIV, ODM_DBG_LOUD, ("Return: Not Support PathDiv\n"));
		return;
	}
#endif
}

void
odm_path_diversity_init(
	void	*p_dm_void
)
{
#if (defined(CONFIG_PATH_DIVERSITY))
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;

	/*p_dm_odm->support_ability |= ODM_BB_PATH_DIV;*/

	if (p_dm_odm->mp_mode == true)
		return;

	if (!(p_dm_odm->support_ability & ODM_BB_PATH_DIV)) {
		ODM_RT_TRACE(p_dm_odm, ODM_COMP_PATH_DIV, ODM_DBG_LOUD, ("Return: Not Support PathDiv\n"));
		return;
	}
#endif
}
