/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


#ifndef __PHYDM_DFS_H__
#define __PHYDM_DFS_H__

#define DFS_VERSION	"0.0"

/* ============================================================
  Definition
 ============================================================
*/

/*
============================================================
1  structure
 ============================================================
*/

/* ============================================================
  enumeration
 ============================================================
*/

enum phydm_dfs_region_domain {
	PHYDM_DFS_DOMAIN_UNKNOWN = 0,
	PHYDM_DFS_DOMAIN_FCC = 1,
	PHYDM_DFS_DOMAIN_MKK = 2,
	PHYDM_DFS_DOMAIN_ETSI = 3,
};

/*
============================================================
  function prototype
============================================================
*/
#if defined(CONFIG_PHYDM_DFS_MASTER)
	void phydm_radar_detect_reset(void *p_dm_void);
	void phydm_radar_detect_disable(void *p_dm_void);
	void phydm_radar_detect_enable(void *p_dm_void);
	bool phydm_radar_detect(void *p_dm_void);
#endif /* defined(CONFIG_PHYDM_DFS_MASTER) */

bool
phydm_dfs_master_enabled(
	void		*p_dm_void
);

void
phydm_dfs_debug(
	void		*p_dm_void,
	u32		*const argv,
	u32		*_used,
	char		*output,
	u32		*_out_len
);

#endif /*#ifndef __PHYDM_DFS_H__ */
