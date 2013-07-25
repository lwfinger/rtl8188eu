/******************************************************************************
 *
 * Copyright(c) 2007 - 2011 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#ifndef __WLAN_BSSDEF_H__
#define __WLAN_BSSDEF_H__


#define MAX_IE_SZ	768

#define NDIS_802_11_LENGTH_SSID         32
#define NDIS_802_11_LENGTH_RATES        8
#define NDIS_802_11_LENGTH_RATES_EX     16

#define NDIS_802_11_RSSI long           // in dBm

struct ndis_802_11_ssid {
	ULONG  SsidLength;
	UCHAR  Ssid[32];
};

enum NDIS_802_11_NETWORK_TYPE {
	Ndis802_11FH,
	Ndis802_11DS,
	Ndis802_11OFDM5,
	Ndis802_11OFDM24,
	Ndis802_11NetworkTypeMax    // not a real type, defined as an upper bound
};

struct ndis_802_11_config_fh {
	ULONG           Length;             // Length of structure
	ULONG           HopPattern;         // As defined by 802.11, MSB set
	ULONG           HopSet;             // to one if non-802.11
	ULONG           DwellTime;          // units are Kusec
};


/*
	FW will only save the channel number in DSConfig.
	ODI Handler will convert the channel number to freq. number.
*/
struct ndis_802_11_config {
	ULONG           Length;             // Length of structure
	ULONG           BeaconPeriod;       // units are Kusec
	ULONG           ATIMWindow;         // units are Kusec
	ULONG           DSConfig;           // Frequency, units are kHz
	struct ndis_802_11_config_fh    FHConfig;
};

enum ndis_802_11_network_infra {
    Ndis802_11IBSS,
    Ndis802_11Infrastructure,
    Ndis802_11AutoUnknown,
    Ndis802_11InfrastructureMax,     // Not a real value, defined as upper bound
    Ndis802_11APMode
};

struct ndis_802_11_fixed_ie {
  UCHAR  Timestamp[8];
  USHORT  BeaconInterval;
  USHORT  Capabilities;
};



struct ndis_802_11_var_ie {
	UCHAR  ElementID;
	UCHAR  Length;
	UCHAR  data[1];
};

/*
Length is the 4 bytes multiples of the sume of
	[ETH_ALEN] + 2 + sizeof (struct ndis_802_11_ssid) + sizeof (ULONG)
+   sizeof (NDIS_802_11_RSSI) + sizeof (enum NDIS_802_11_NETWORK_TYPE) + sizeof (struct ndis_802_11_config)
+   NDIS_802_11_LENGTH_RATES_EX + IELength

Except the IELength, all other fields are fixed length. Therefore, we can define a macro to represent the
partial sum.
*/

enum ndis_802_11_auth_mode {
    Ndis802_11AuthModeOpen,
    Ndis802_11AuthModeShared,
    Ndis802_11AuthModeAutoSwitch,
    Ndis802_11AuthModeWPA,
    Ndis802_11AuthModeWPAPSK,
    Ndis802_11AuthModeWPANone,
    Ndis802_11AuthModeWAPI,
    Ndis802_11AuthModeMax               // Not a real mode, defined as upper bound
};

enum ndis_802_11_wep_status {
    Ndis802_11WEPEnabled,
    Ndis802_11Encryption1Enabled = Ndis802_11WEPEnabled,
    Ndis802_11WEPDisabled,
    Ndis802_11EncryptionDisabled = Ndis802_11WEPDisabled,
    Ndis802_11WEPKeyAbsent,
    Ndis802_11Encryption1KeyAbsent = Ndis802_11WEPKeyAbsent,
    Ndis802_11WEPNotSupported,
    Ndis802_11EncryptionNotSupported = Ndis802_11WEPNotSupported,
    Ndis802_11Encryption2Enabled,
    Ndis802_11Encryption2KeyAbsent,
    Ndis802_11Encryption3Enabled,
    Ndis802_11Encryption3KeyAbsent,
    Ndis802_11_EncryptionWAPI
};


#define NDIS_802_11_AI_REQFI_CAPABILITIES      1
#define NDIS_802_11_AI_REQFI_LISTENINTERVAL    2
#define NDIS_802_11_AI_REQFI_CURRENTAPADDRESS  4

#define NDIS_802_11_AI_RESFI_CAPABILITIES      1
#define NDIS_802_11_AI_RESFI_STATUSCODE        2
#define NDIS_802_11_AI_RESFI_ASSOCIATIONID     4

struct ndis_802_11_ai_reqfi {
    USHORT Capabilities;
    USHORT ListenInterval;
    unsigned char CurrentAPAddress[ETH_ALEN];
};

struct ndis_802_11_ai_resfi {
    USHORT Capabilities;
    USHORT StatusCode;
    USHORT AssociationId;
};

struct ndis_802_11_assoc_info {
    ULONG                   Length;
    USHORT                  AvailableRequestFixedIEs;
    struct ndis_802_11_ai_reqfi    RequestFixedIEs;
    ULONG                   RequestIELength;
    ULONG                   OffsetRequestIEs;
    USHORT                  AvailableResponseFixedIEs;
    struct ndis_802_11_ai_resfi    ResponseFixedIEs;
    ULONG                   ResponseIELength;
    ULONG                   OffsetResponseIEs;
};

enum ndis_802_11_reload_def {
	Ndis802_11ReloadWEPKeys
};


// Key mapping keys require a BSSID
struct ndis_802_11_key {
	ULONG           Length;             // Length of this structure
	ULONG           KeyIndex;
	ULONG           KeyLength;          // length of key in bytes
	unsigned char BSSID[ETH_ALEN];
	unsigned long long KeyRSC;
	UCHAR           KeyMaterial[32];     // variable length depending on above field
};

struct ndis_802_11_remove_key {
	ULONG                   Length;        // Length of this structure
	ULONG                   KeyIndex;
	unsigned char BSSID[ETH_ALEN];
};

struct ndis_802_11_wep {
	ULONG     Length;        // Length of this structure
	ULONG     KeyIndex;      // 0 is the per-client key, 1-N are the global keys
	ULONG     KeyLength;     // length of key in bytes
	UCHAR     KeyMaterial[16];// variable length depending on above field
};

struct ndis_802_11_auth_req {
    ULONG Length;            // Length of structure
    unsigned char Bssid[ETH_ALEN];
    ULONG Flags;
};

enum ndis_802_11_status_type {
	Ndis802_11StatusType_Authentication,
	Ndis802_11StatusType_MediaStreamMode,
	Ndis802_11StatusType_PMKID_CandidateList,
	Ndis802_11StatusTypeMax    // not a real type, defined as an upper bound
};

struct ndis_802_11_status_ind {
    enum ndis_802_11_status_type StatusType;
};

// mask for authentication/integrity fields
#define NDIS_802_11_AUTH_REQUEST_AUTH_FIELDS        0x0f
#define NDIS_802_11_AUTH_REQUEST_REAUTH			0x01
#define NDIS_802_11_AUTH_REQUEST_KEYUPDATE		0x02
#define NDIS_802_11_AUTH_REQUEST_PAIRWISE_ERROR		0x06
#define NDIS_802_11_AUTH_REQUEST_GROUP_ERROR		0x0E

// MIC check time, 60 seconds.
#define MIC_CHECK_TIME	60000000

struct ndis_802_11_auth_evt {
    struct ndis_802_11_status_ind       Status;
    struct ndis_802_11_auth_req  Request[1];
};

struct ndis_802_11_test {
    ULONG Length;
    ULONG Type;
    union
    {
        struct ndis_802_11_auth_evt AuthenticationEvent;
        NDIS_802_11_RSSI RssiTrigger;
    }tt;
};


#ifndef Ndis802_11APMode
#define Ndis802_11APMode (Ndis802_11InfrastructureMax+1)
#endif

struct wlan_phy_info {
	u8	SignalStrength;//(in percentage)
	u8	SignalQuality;//(in percentage)
	u8	Optimum_antenna;  //for Antenna diversity
	u8	Reserved_0;
};

struct wlan_bcn_info {
	/* these infor get from rtw_get_encrypt_info when
	 *	 * translate scan to UI */
	u8 encryp_protocol;//ENCRYP_PROTOCOL_E: OPEN/WEP/WPA/WPA2/WAPI
	int group_cipher; //WPA/WPA2 group cipher
	int pairwise_cipher;////WPA/WPA2/WEP pairwise cipher
	int is_8021x;

	/* bwmode 20/40 and ch_offset UP/LOW */
	unsigned short	ht_cap_info;
	unsigned char	ht_info_infos_0;
};

/* temporally add #pragma pack for structure alignment issue of
*   struct wlan_bssid_ex and get_struct wlan_bssid_ex_sz()
*/
struct wlan_bssid_ex {
	ULONG  Length;
	unsigned char MacAddress[ETH_ALEN];
	UCHAR  Reserved[2];//[0]: IS beacon frame
	struct ndis_802_11_ssid  Ssid;
	ULONG  Privacy;
	NDIS_802_11_RSSI  Rssi;//(in dBM,raw data ,get from PHY)
	enum  NDIS_802_11_NETWORK_TYPE  NetworkTypeInUse;
	struct ndis_802_11_config  Configuration;
	enum ndis_802_11_network_infra  InfrastructureMode;
	unsigned char SupportedRates[NDIS_802_11_LENGTH_RATES_EX];
	struct wlan_phy_info	PhyInfo;
	ULONG  IELength;
	UCHAR  IEs[MAX_IE_SZ];	//(timestamp, beacon interval, and capability information)
} __packed;

static inline uint get_wlan_bssid_ex_sz(struct wlan_bssid_ex *bss)
{
	return sizeof(struct wlan_bssid_ex) - MAX_IE_SZ + bss->IELength;
}

struct	wlan_network {
	struct list_head list;
	int	network_type;	//refer to ieee80211.h for WIRELESS_11A/B/G
	int	fixed;			// set to fixed when not to be removed as site-surveying
	unsigned long	last_scanned; //timestamp for the network
	int	aid;			//will only be valid when a BSS is joinned.
	int	join_res;
	struct wlan_bssid_ex	network; //must be the last item
	struct wlan_bcn_info	BcnInfo;
};

enum VRTL_CARRIER_SENSE
{
	DISABLE_VCS,
	ENABLE_VCS,
	AUTO_VCS
};

enum VCS_TYPE
{
	NONE_VCS,
	RTS_CTS,
	CTS_TO_SELF
};

#define PWR_CAM 0
#define PWR_MINPS 1
#define PWR_MAXPS 2
#define PWR_UAPSD 3
#define PWR_VOIP 4

enum UAPSD_MAX_SP
{
	NO_LIMIT,
       TWO_MSDU,
       FOUR_MSDU,
       SIX_MSDU
};

#define NUM_PRE_AUTH_KEY 16
#define NUM_PMKID_CACHE NUM_PRE_AUTH_KEY

/*
*	WPA2
*/

struct pmkid_candidate {
	unsigned char BSSID[ETH_ALEN];
	ULONG Flags;
};

struct ndis_802_11_pmkid_list {
	ULONG Version;       // Version of the structure
	ULONG NumCandidates; // No. of pmkid candidates
	struct pmkid_candidate CandidateList[1];
};

struct ndis_802_11_auth_encrypt {
	enum ndis_802_11_auth_mode AuthModeSupported;
	enum ndis_802_11_wep_status EncryptStatusSupported;
};

struct ndis_802_11_cap {
	ULONG  Length;
	ULONG  Version;
	ULONG  NoOfPMKIDs;
	ULONG  NoOfAuthEncryptPairsSupported;
	struct ndis_802_11_auth_encrypt AuthenticationEncryptionSupported[1];
};

u8 key_2char2num(u8 hch, u8 lch);
u8 key_char2num(u8 ch);
u8 str_2char2num(u8 hch, u8 lch);

#endif //#ifndef WLAN_BSSDEF_H_
