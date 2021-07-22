#ifndef __INC_PHYDM_BEAMFORMING_H
#define __INC_PHYDM_BEAMFORMING_H

#ifndef BEAMFORMING_SUPPORT
	#define	BEAMFORMING_SUPPORT		0
#endif

/*Beamforming Related*/
#include "halcomtxbf.h"
#include "haltxbfjaguar.h"
#include "haltxbfinterface.h"

#define beamforming_gid_paid(adapter, p_tcb)
#define	phydm_acting_determine(p_dm_odm, type)	false
#define beamforming_enter(p_dm_odm, sta_idx)
#define beamforming_leave(p_dm_odm, RA)
#define beamforming_end_fw(p_dm_odm)
#define beamforming_control_v1(p_dm_odm, RA, AID, mode, BW, rate)		true
#define beamforming_control_v2(p_dm_odm, idx, mode, BW, period)		true
#define phydm_beamforming_end_sw(p_dm_odm, _status)
#define beamforming_timer_callback(p_dm_odm)
#define phydm_beamforming_init(p_dm_odm)
#define phydm_beamforming_control_v2(p_dm_odm, _idx, _mode, _BW, _period)	false
#define beamforming_watchdog(p_dm_odm)
#define phydm_beamforming_watchdog(p_dm_odm)

#endif
