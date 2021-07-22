// SPDX-License-Identifier: GPL-2.0
/* Copyright(c) 2007 - 2016 Realtek Corporation. All rights reserved. */


/* ************************************************************
 * include files
 * ************************************************************ */

#include "mp_precomp.h"
#include "phydm_precomp.h"

/*
 * ODM IO Relative API.
 *   */

u8
odm_read_1byte(
	struct PHY_DM_STRUCT		*p_dm_odm,
	u32			reg_addr
)
{
	struct _ADAPTER		*adapter = p_dm_odm->adapter;

	return rtw_read8(adapter, reg_addr);
}

u16
odm_read_2byte(
	struct PHY_DM_STRUCT		*p_dm_odm,
	u32			reg_addr
)
{
	struct _ADAPTER		*adapter = p_dm_odm->adapter;

	return rtw_read16(adapter, reg_addr);
}

u32
odm_read_4byte(
	struct PHY_DM_STRUCT		*p_dm_odm,
	u32			reg_addr
)
{
	struct _ADAPTER		*adapter = p_dm_odm->adapter;

	return rtw_read32(adapter, reg_addr);
}

void
odm_write_1byte(
	struct PHY_DM_STRUCT		*p_dm_odm,
	u32			reg_addr,
	u8			data
)
{
	struct _ADAPTER		*adapter = p_dm_odm->adapter;

	rtw_write8(adapter, reg_addr, data);
}


void
odm_write_2byte(
	struct PHY_DM_STRUCT		*p_dm_odm,
	u32			reg_addr,
	u16			data
)
{
	struct _ADAPTER		*adapter = p_dm_odm->adapter;

	rtw_write16(adapter, reg_addr, data);
}

void
odm_write_4byte(
	struct PHY_DM_STRUCT		*p_dm_odm,
	u32			reg_addr,
	u32			data
)
{
	struct _ADAPTER		*adapter = p_dm_odm->adapter;

	rtw_write32(adapter, reg_addr, data);
}

void
odm_set_mac_reg(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u32		reg_addr,
	u32		bit_mask,
	u32		data
)
{
	phy_set_bb_reg(p_dm_odm->adapter, reg_addr, bit_mask, data);
}

u32
odm_get_mac_reg(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u32		reg_addr,
	u32		bit_mask
)
{
	return phy_query_mac_reg(p_dm_odm->adapter, reg_addr, bit_mask);
}

void
odm_set_bb_reg(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u32		reg_addr,
	u32		bit_mask,
	u32		data
)
{
	phy_set_bb_reg(p_dm_odm->adapter, reg_addr, bit_mask, data);
}

u32
odm_get_bb_reg(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u32		reg_addr,
	u32		bit_mask
)
{
	return phy_query_bb_reg(p_dm_odm->adapter, reg_addr, bit_mask);
}

void
odm_set_rf_reg(
	struct PHY_DM_STRUCT			*p_dm_odm,
	enum odm_rf_radio_path_e	e_rf_path,
	u32				reg_addr,
	u32				bit_mask,
	u32				data
)
{
	phy_set_rf_reg(p_dm_odm->adapter, e_rf_path, reg_addr, bit_mask, data);
}

u32
odm_get_rf_reg(
	struct PHY_DM_STRUCT			*p_dm_odm,
	enum odm_rf_radio_path_e	e_rf_path,
	u32				reg_addr,
	u32				bit_mask
)
{
	return phy_query_rf_reg(p_dm_odm->adapter, e_rf_path, reg_addr, bit_mask);
}

/*
 * ODM Memory relative API.
 *   */
void
odm_allocate_memory(
	struct PHY_DM_STRUCT	*p_dm_odm,
	void **p_ptr,
	u32		length
)
{
	*p_ptr = rtw_zvmalloc(length);
}

/* length could be ignored, used to detect memory leakage. */
void
odm_free_memory(
	struct PHY_DM_STRUCT	*p_dm_odm,
	void		*p_ptr,
	u32		length
)
{
	rtw_vmfree(p_ptr, length);
}

void
odm_move_memory(
	struct PHY_DM_STRUCT	*p_dm_odm,
	void		*p_dest,
	void		*p_src,
	u32		length
)
{
	memcpy(p_dest, p_src, length);
}

void odm_memory_set(
	struct PHY_DM_STRUCT	*p_dm_odm,
	void		*pbuf,
	s8		value,
	u32		length
)
{
	memset(pbuf, value, length);
}

s32 odm_compare_memory(
	struct PHY_DM_STRUCT		*p_dm_odm,
	void           *p_buf1,
	void           *p_buf2,
	u32          length
)
{
	return !memcmp(p_buf1, p_buf2, length);
}

/*
 * ODM MISC relative API.
 *   */
void
odm_acquire_spin_lock(
	struct PHY_DM_STRUCT			*p_dm_odm,
	enum rt_spinlock_type	type
)
{
	struct _ADAPTER *adapter = p_dm_odm->adapter;

	rtw_odm_acquirespinlock(adapter, type);
}

void
odm_release_spin_lock(
	struct PHY_DM_STRUCT			*p_dm_odm,
	enum rt_spinlock_type	type
)
{
	struct _ADAPTER *adapter = p_dm_odm->adapter;
	rtw_odm_releasespinlock(adapter, type);
}

/*
 * ODM Timer relative API.
 *   */
void
odm_stall_execution(
	u32	us_delay
)
{
	rtw_udelay_os(us_delay);
}

void
ODM_delay_ms(u32	ms)
{
	rtw_mdelay_os(ms);
}

void
ODM_delay_us(u32	us)
{
	rtw_udelay_os(us);
}

void
ODM_sleep_ms(u32	ms)
{
	rtw_msleep_os(ms);
}

void
ODM_sleep_us(u32	us)
{
	rtw_usleep_os(us);
}

void
odm_set_timer(
	struct PHY_DM_STRUCT		*p_dm_odm,
	struct timer_list		*p_timer,
	u32			ms_delay
)
{
	_set_timer(p_timer, ms_delay); /* ms */
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(4, 15, 0)
void
odm_initialize_timer(
	struct PHY_DM_STRUCT			*p_dm_odm,
	struct timer_list			*p_timer,
	void	*call_back_func,
	void				*p_context,
	const char			*sz_id
)
{
	struct _ADAPTER *adapter = p_dm_odm->adapter;
	_init_timer(p_timer, adapter->pnetdev, call_back_func, p_dm_odm);
}
#endif

void
odm_cancel_timer(
	struct PHY_DM_STRUCT		*p_dm_odm,
	struct timer_list		*p_timer
)
{
	_cancel_timer_ex(p_timer);
}

void
odm_release_timer(
	struct PHY_DM_STRUCT		*p_dm_odm,
	struct timer_list		*p_timer
)
{
}

static u8
phydm_trans_h2c_id(
	struct PHY_DM_STRUCT	*p_dm_odm,
	u8		phydm_h2c_id
)
{
	u8 platform_h2c_id = phydm_h2c_id;

	switch (phydm_h2c_id) {
	/* 1 [0] */
	case ODM_H2C_RSSI_REPORT:
		platform_h2c_id = H2C_RSSI_SETTING;
		break;
	/* 1 [3] */
	case ODM_H2C_WIFI_CALIBRATION:
		break;
	/* 1 [4] */
	case ODM_H2C_IQ_CALIBRATION:
		break;
	/* 1 [5] */
	case ODM_H2C_RA_PARA_ADJUST:
		break;
	/* 1 [6] */
	case PHYDM_H2C_DYNAMIC_TX_PATH:
		break;
	/* [7]*/
	case PHYDM_H2C_FW_TRACE_EN:
		platform_h2c_id = 0x49;
		break;
	case PHYDM_H2C_TXBF:
		break;
	case PHYDM_H2C_MU:
		break;
	default:
		platform_h2c_id = phydm_h2c_id;
		break;
	}
	return platform_h2c_id;
}

/*ODM FW relative API.*/

void
odm_fill_h2c_cmd(
	struct PHY_DM_STRUCT		*p_dm_odm,
	u8			phydm_h2c_id,
	u32			cmd_len,
	u8			*p_cmd_buffer
)
{
	struct _ADAPTER	*adapter = p_dm_odm->adapter;
	u8		platform_h2c_id;

	platform_h2c_id = phydm_trans_h2c_id(p_dm_odm, phydm_h2c_id);

	ODM_RT_TRACE(p_dm_odm, PHYDM_COMP_RA_DBG, ODM_DBG_LOUD, ("[H2C]  platform_h2c_id = ((0x%x))\n", platform_h2c_id));

	rtw_hal_fill_h2c_cmd(adapter, platform_h2c_id, cmd_len, p_cmd_buffer);
}

u8
phydm_c2H_content_parsing(
	void			*p_dm_void,
	u8			c2h_cmd_id,
	u8			c2h_cmd_len,
	u8			*tmp_buf
)
{
	struct PHY_DM_STRUCT		*p_dm_odm = (struct PHY_DM_STRUCT *)p_dm_void;
	u8	extend_c2h_sub_id = 0;
	u8	find_c2h_cmd = true;

	switch (c2h_cmd_id) {
	case PHYDM_C2H_DBG:
		phydm_fw_trace_handler(p_dm_odm, tmp_buf, c2h_cmd_len);
		break;
	case PHYDM_C2H_RA_RPT:
		phydm_c2h_ra_report_handler(p_dm_odm, tmp_buf, c2h_cmd_len);
		break;
	case PHYDM_C2H_RA_PARA_RPT:
		odm_c2h_ra_para_report_handler(p_dm_odm, tmp_buf, c2h_cmd_len);
		break;
	case PHYDM_C2H_DYNAMIC_TX_PATH_RPT:
		if (p_dm_odm->support_ic_type & (ODM_RTL8814A))
			phydm_c2h_dtp_handler(p_dm_odm, tmp_buf, c2h_cmd_len);
		break;
	case PHYDM_C2H_IQK_FINISH:
		break;
	case PHYDM_C2H_DBG_CODE:
		phydm_fw_trace_handler_code(p_dm_odm, tmp_buf, c2h_cmd_len);
		break;

	case PHYDM_C2H_EXTEND:
		extend_c2h_sub_id = tmp_buf[0];
		if (extend_c2h_sub_id == PHYDM_EXTEND_C2H_DBG_PRINT)
			phydm_fw_trace_handler_8051(p_dm_odm, tmp_buf, c2h_cmd_len);

		break;
	default:
		find_c2h_cmd = false;
		break;
	}
	return find_c2h_cmd;
}

u64
odm_get_current_time(
	struct PHY_DM_STRUCT		*p_dm_odm
)
{
	return (u64)jiffies;
}

u64
odm_get_progressing_time(
	struct PHY_DM_STRUCT		*p_dm_odm,
	u64			start_time
)
{
	return rtw_get_passing_time_ms((u32)start_time);
}

void
phydm_set_hw_reg_handler_interface (
	struct PHY_DM_STRUCT		*p_dm_odm,
	u8				RegName,
	u8				*val
	)
{
	struct _ADAPTER *adapter = p_dm_odm->adapter;

	adapter->hal_func.set_hw_reg_handler(adapter, RegName, val);
}

void
phydm_get_hal_def_var_handler_interface (
	struct PHY_DM_STRUCT		*p_dm_odm,
	enum _HAL_DEF_VARIABLE		e_variable,
	void						*p_value
	)
{
	struct _ADAPTER *adapter = p_dm_odm->adapter;

	adapter->hal_func.get_hal_def_var_handler(adapter, e_variable, p_value);
}
