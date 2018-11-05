#ifndef __HAL_TXBF_INTERFACE_H__
#define __HAL_TXBF_INTERFACE_H__

#define beamforming_get_ndpa_frame(p_dm_odm, _pdu_os)
#define beamforming_get_report_frame(adapter, precv_frame)		RT_STATUS_FAILURE
#define send_fw_ht_ndpa_packet(p_dm_void, RA, BW)
#define send_sw_ht_ndpa_packet(p_dm_void, RA, BW)
#define send_fw_vht_ndpa_packet(p_dm_void, RA, AID, BW)
#define send_sw_vht_ndpa_packet(p_dm_void, RA,	AID, BW)
#define send_sw_vht_gid_mgnt_frame(p_dm_void, RA, idx)
#define send_sw_vht_bf_report_poll(p_dm_void, RA, is_final_poll)
#define send_sw_vht_mu_ndpa_packet(p_dm_void, BW)

#endif
