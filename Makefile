EXTRA_CFLAGS += $(USER_EXTRA_CFLAGS)
EXTRA_CFLAGS += -O1 -g
EXTRA_CFLAGS += -Wno-unused-variable
EXTRA_CFLAGS += -Wno-unused-value
EXTRA_CFLAGS += -Wno-unused-label
EXTRA_CFLAGS += -Wno-unused-parameter
EXTRA_CFLAGS += -Wno-unused-function
EXTRA_CFLAGS += -Wno-unused
EXTRA_CFLAGS += -Wno-uninitialized

GCC_VER_49 := $(shell echo `$(CC) -dumpversion | cut -f1-2 -d.` \>= 4.9 | bc )
ifeq ($(GCC_VER_49),1)
EXTRA_CFLAGS += -Wno-date-time	# Fix compile error && warning on gcc 4.9 and later
endif

EXTRA_CFLAGS += -I$(src)

CONFIG_AUTOCFG_CP = n

########################## Features ###########################
CONFIG_MP_INCLUDED = y
CONFIG_POWER_SAVING = y
CONFIG_EFUSE_CONFIG_FILE = y
CONFIG_TRAFFIC_PROTECT = y
CONFIG_LOAD_PHY_PARA_FROM_FILE = y
CONFIG_RTW_ADAPTIVITY_EN = disable
CONFIG_RTW_ADAPTIVITY_MODE = normal
CONFIG_BR_EXT = y
CONFIG_RTW_NAPI = y
CONFIG_RTW_GRO = y
########################## Debug ###########################
CONFIG_RTW_DEBUG = y
# please refer to "How_to_set_driver_debug_log_level.doc" to set the available level.
CONFIG_RTW_LOG_LEVEL = 2
######################## Wake On Lan ##########################
CONFIG_WAKEUP_GPIO_IDX = default
######### Notify SDIO Host Keep Power During Syspend ##########
CONFIG_RTW_SDIO_PM_KEEP_POWER = y
###################### Platform Related #######################
CONFIG_PLATFORM_I386_PC = y
###############################################################

export TopDIR ?= $(CURDIR)

MSG="Directory .git does not exist indicating that you downloaded the source as a zip file. Only the 'git clone' method is now supported."

########### COMMON  #################################

HCI_NAME = usb

_OS_INTFS_FILES :=	osdep_service.o \
			os_intfs.o \
			usb_intf.o \
			usb_ops2_linux.o \
			ioctl_linux.o \
			xmit_linux.o \
			mlme_linux.o \
			recv_linux.o \
			ioctl_cfg80211.o \
			rtw_cfgvendor.o \
			wifi_regd.o \
			rtw_android.o \
			rtw_proc.o

ifeq ($(CONFIG_MP_INCLUDED), y)
_OS_INTFS_FILES += ioctl_mp.o
endif

_HAL_INTFS_FILES :=	hal_intf.o \
			hal_com.o \
			hal_com_phycfg.o \
			hal_phy.o \
			hal_dm.o \
			hal_btcoex_wifionly.o \
			hal_btcoex.o \
			hal_mp.o \
			hal_mcc.o \
			hal_usb.o \
			hal_usb_led.o

			
_OUTSRC_FILES := phydm_debug.o	\
		phydm_antdiv.o\
		phydm_antdect.o\
		phydm_interface.o\
		phydm_hwconfig.o\
		phydm.o\
		halphyrf_ce.o\
		phydm_edcaturbocheck.o\
		phydm_dig.o\
		phydm_pathdiv.o\
		phydm_rainfo.o\
		phydm_dynamicbbpowersaving.o\
		phydm_powertracking_ce.o\
		phydm_dynamictxpower.o\
		phydm_adaptivity.o\
		phydm_cfotracking.o\
		phydm_noisemonitor.o\
		phydm_acs.o\
		phydm_dfs.o\
		phydm_hal_txbf_api.o\
		phydm_adc_sampling.o\
		phydm_kfree.o\
		phydm_ccx.o

_HAL_INTFS_FILES +=	HalPwrSeqCmd.o \
					Hal8188EPwrSeq.o\
 					rtl8188e_xmit.o\
					rtl8188e_sreset.o

_HAL_INTFS_FILES +=	rtl8188e_hal_init.o \
			rtl8188e_phycfg.o \
			rtl8188e_rf6052.o \
			rtl8188e_dm.o \
			rtl8188e_rxdesc.o \
			rtl8188e_cmd.o \
			hal8188e_s_fw.o \
			hal8188e_t_fw.o \
			usb_halinit.o \
			rtl8188eu_led.o \
			rtl8188eu_xmit.o \
			rtl8188eu_recv.o

_HAL_INTFS_FILES += usb_ops_linux.o

	_HAL_INTFS_FILES +=HalEfuseMask8188E_USB.o

_OUTSRC_FILES += halhwimg8188e_mac.o\
		halhwimg8188e_bb.o\
		halhwimg8188e_rf.o\
		halphyrf_8188e_ce.o\
		phydm_regconfig8188e.o\
		hal8188erateadaptive.o\
		phydm_rtl8188e.o


########### END OF PATH  #################################

ifeq ($(CONFIG_USB_AUTOSUSPEND), y)
EXTRA_CFLAGS += -DCONFIG_USB_AUTOSUSPEND
endif

ifeq ($(CONFIG_MP_INCLUDED), y)
#MODULE_NAME := 8188eu_mp
EXTRA_CFLAGS += -DCONFIG_MP_INCLUDED
endif

ifeq ($(CONFIG_POWER_SAVING), y)
EXTRA_CFLAGS += -DCONFIG_POWER_SAVING
endif

ifeq ($(CONFIG_HW_PWRP_DETECTION), y)
EXTRA_CFLAGS += -DCONFIG_HW_PWRP_DETECTION
endif

ifeq ($(CONFIG_WIFI_TEST), y)
EXTRA_CFLAGS += -DCONFIG_WIFI_TEST
endif

ifeq ($(CONFIG_BT_COEXIST), y)
EXTRA_CFLAGS += -DCONFIG_BT_COEXIST
endif

ifeq ($(CONFIG_INTEL_WIDI), y)
EXTRA_CFLAGS += -DCONFIG_INTEL_WIDI
endif

ifeq ($(CONFIG_WAPI_SUPPORT), y)
EXTRA_CFLAGS += -DCONFIG_WAPI_SUPPORT
endif


ifeq ($(CONFIG_EFUSE_CONFIG_FILE), y)
EXTRA_CFLAGS += -DCONFIG_EFUSE_CONFIG_FILE

#EFUSE_MAP_PATH
USER_EFUSE_MAP_PATH ?=
ifneq ($(USER_EFUSE_MAP_PATH),)
EXTRA_CFLAGS += -DEFUSE_MAP_PATH=\"$(USER_EFUSE_MAP_PATH)\"
else ifeq (8188eu, 8189es)
EXTRA_CFLAGS += -DEFUSE_MAP_PATH=\"/system/etc/wifi/wifi_efuse_8189e.map\"
else ifeq (8188eu, 8723bs)
EXTRA_CFLAGS += -DEFUSE_MAP_PATH=\"/system/etc/wifi/wifi_efuse_8723bs.map\"
else
EXTRA_CFLAGS += -DEFUSE_MAP_PATH=\"/system/etc/wifi/wifi_efuse_8188eu.map\"
endif

#WIFIMAC_PATH
USER_WIFIMAC_PATH ?=
ifneq ($(USER_WIFIMAC_PATH),)
EXTRA_CFLAGS += -DWIFIMAC_PATH=\"$(USER_WIFIMAC_PATH)\"
else
EXTRA_CFLAGS += -DWIFIMAC_PATH=\"/data/wifimac.txt\"
endif

endif

ifeq ($(CONFIG_EXT_CLK), y)
EXTRA_CFLAGS += -DCONFIG_EXT_CLK
endif

ifeq ($(CONFIG_TRAFFIC_PROTECT), y)
EXTRA_CFLAGS += -DCONFIG_TRAFFIC_PROTECT
endif

ifeq ($(CONFIG_LOAD_PHY_PARA_FROM_FILE), y)
EXTRA_CFLAGS += -DCONFIG_LOAD_PHY_PARA_FROM_FILE
EXTRA_CFLAGS += -DREALTEK_CONFIG_PATH=\"\"
endif

ifeq ($(CONFIG_TXPWR_BY_RATE_EN), n)
EXTRA_CFLAGS += -DCONFIG_TXPWR_BY_RATE_EN=0
else ifeq ($(CONFIG_TXPWR_BY_RATE_EN), y)
EXTRA_CFLAGS += -DCONFIG_TXPWR_BY_RATE_EN=1
else ifeq ($(CONFIG_TXPWR_BY_RATE_EN), auto)
EXTRA_CFLAGS += -DCONFIG_TXPWR_BY_RATE_EN=2
endif

ifeq ($(CONFIG_TXPWR_LIMIT_EN), n)
EXTRA_CFLAGS += -DCONFIG_TXPWR_LIMIT_EN=0
else ifeq ($(CONFIG_TXPWR_LIMIT_EN), y)
EXTRA_CFLAGS += -DCONFIG_TXPWR_LIMIT_EN=1
else ifeq ($(CONFIG_TXPWR_LIMIT_EN), auto)
EXTRA_CFLAGS += -DCONFIG_TXPWR_LIMIT_EN=2
endif

ifeq ($(CONFIG_CALIBRATE_TX_POWER_BY_REGULATORY), y)
EXTRA_CFLAGS += -DCONFIG_CALIBRATE_TX_POWER_BY_REGULATORY
endif

ifeq ($(CONFIG_CALIBRATE_TX_POWER_TO_MAX), y)
EXTRA_CFLAGS += -DCONFIG_CALIBRATE_TX_POWER_TO_MAX
endif

ifeq ($(CONFIG_RTW_ADAPTIVITY_EN), disable)
EXTRA_CFLAGS += -DCONFIG_RTW_ADAPTIVITY_EN=0
else ifeq ($(CONFIG_RTW_ADAPTIVITY_EN), enable)
EXTRA_CFLAGS += -DCONFIG_RTW_ADAPTIVITY_EN=1
endif

ifeq ($(CONFIG_RTW_ADAPTIVITY_MODE), normal)
EXTRA_CFLAGS += -DCONFIG_RTW_ADAPTIVITY_MODE=0
else ifeq ($(CONFIG_RTW_ADAPTIVITY_MODE), carrier_sense)
EXTRA_CFLAGS += -DCONFIG_RTW_ADAPTIVITY_MODE=1
endif

ifeq ($(CONFIG_SIGNAL_SCALE_MAPPING), y)
EXTRA_CFLAGS += -DCONFIG_SIGNAL_SCALE_MAPPING
endif

ifeq ($(CONFIG_80211W), y)
EXTRA_CFLAGS += -DCONFIG_IEEE80211W
endif

ifeq ($(CONFIG_WOWLAN), y)
EXTRA_CFLAGS += -DCONFIG_WOWLAN
ifeq ($(CONFIG_DEFAULT_PATTERNS_EN), y)
EXTRA_CFLAGS += -DCONFIG_DEFAULT_PATTERNS_EN
endif
endif

ifeq ($(CONFIG_AP_WOWLAN), y)
EXTRA_CFLAGS += -DCONFIG_AP_WOWLAN
endif

ifeq ($(CONFIG_PNO_SUPPORT), y)
EXTRA_CFLAGS += -DCONFIG_PNO_SUPPORT
ifeq ($(CONFIG_PNO_SET_DEBUG), y)
EXTRA_CFLAGS += -DCONFIG_PNO_SET_DEBUG
endif
endif

ifeq ($(CONFIG_GPIO_WAKEUP), y)
EXTRA_CFLAGS += -DCONFIG_GPIO_WAKEUP
ifeq ($(CONFIG_HIGH_ACTIVE), y)
EXTRA_CFLAGS += -DHIGH_ACTIVE=1
else
EXTRA_CFLAGS += -DHIGH_ACTIVE=0
endif
endif

ifneq ($(CONFIG_WAKEUP_GPIO_IDX), default)
EXTRA_CFLAGS += -DWAKEUP_GPIO_IDX=$(CONFIG_WAKEUP_GPIO_IDX)
endif

ifeq ($(CONFIG_REDUCE_TX_CPU_LOADING), y)
EXTRA_CFLAGS += -DCONFIG_REDUCE_TX_CPU_LOADING
endif

ifeq ($(CONFIG_BR_EXT), y)
BR_NAME = br0
EXTRA_CFLAGS += -DCONFIG_BR_EXT
EXTRA_CFLAGS += '-DCONFIG_BR_EXT_BRNAME="'$(BR_NAME)'"'
endif


ifeq ($(CONFIG_TDLS), y)
EXTRA_CFLAGS += -DCONFIG_TDLS
endif

ifeq ($(CONFIG_WIFI_MONITOR), y)
EXTRA_CFLAGS += -DCONFIG_WIFI_MONITOR
endif

ifeq ($(CONFIG_MCC_MODE), y)
EXTRA_CFLAGS += -DCONFIG_MCC_MODE
endif

ifeq ($(CONFIG_RTW_NAPI), y)
EXTRA_CFLAGS += -DCONFIG_RTW_NAPI
endif

ifeq ($(CONFIG_RTW_GRO), y)
EXTRA_CFLAGS += -DCONFIG_RTW_GRO
endif

ifeq ($(CONFIG_MP_VHT_HW_TX_MODE), y)
EXTRA_CFLAGS += -DCONFIG_MP_VHT_HW_TX_MODE
ifeq ($(CONFIG_PLATFORM_I386_PC), y)
## For I386 X86 ToolChain use Hardware FLOATING
EXTRA_CFLAGS += -mhard-float
else
## For ARM ToolChain use Hardware FLOATING
EXTRA_CFLAGS += -mfloat-abi=hard
endif
endif

ifeq ($(CONFIG_APPEND_VENDOR_IE_ENABLE), y)
EXTRA_CFLAGS += -DCONFIG_APPEND_VENDOR_IE_ENABLE
endif

ifeq ($(CONFIG_RTW_DEBUG), y)
EXTRA_CFLAGS += -DCONFIG_RTW_DEBUG
EXTRA_CFLAGS += -DRTW_LOG_LEVEL=$(CONFIG_RTW_LOG_LEVEL)
endif

EXTRA_CFLAGS += -DDM_ODM_SUPPORT_TYPE=0x04

EXTRA_CFLAGS += -DCONFIG_IOCTL_CFG80211
EXTRA_CFLAGS += -DRTW_USE_CFG80211_STA_EVENT

SUBARCH := $(shell uname -m | sed -e "s/i.86/i386/; s/ppc.*/powerpc/; s/armv.l/arm/; s/aarch64/arm64/;")
ARCH ?= $(SUBARCH)
CROSS_COMPILE ?=
KVER  ?= $(if $(KERNELRELEASE),$(KERNELRELEASE),$(shell uname -r))
KSRC := /lib/modules/$(KVER)/build
MODDESTDIR := /lib/modules/$(KVER)/kernel/drivers/net/wireless/
INSTALL_PREFIX :=

ifeq ($(CONFIG_MULTIDRV), y)


MODULE_NAME := rtw_usb

endif

USER_MODULE_NAME ?=
ifneq ($(USER_MODULE_NAME),)
MODULE_NAME := $(USER_MODULE_NAME)
endif

ifneq ($(KERNELRELEASE),)

rtk_core :=	rtw_cmd.o \
		rtw_security.o \
		rtw_debug.o \
		rtw_io.o \
		rtw_ioctl_query.o \
		rtw_ioctl_set.o \
		rtw_ieee80211.o \
		rtw_mlme.o \
		rtw_mlme_ext.o \
		rtw_mi.o \
		rtw_wlan_util.o \
		rtw_pwrctrl.o \
		rtw_rf.o \
		rtw_recv.o \
		rtw_sta_mgt.o \
		rtw_ap.o \
		rtw_xmit.o	\
		rtw_p2p.o \
		rtw_tdls.o \
		rtw_br_ext.o \
		rtw_iol.o \
		rtw_sreset.o \
		rtw_btcoex_wifionly.o \
		rtw_btcoex.o \
		rtw_beamforming.o \
		rtw_odm.o \
		rtw_efuse.o 

8188eu-y += $(rtk_core)

8188eu-$(CONFIG_INTEL_WIDI) += rtw_intel_widi.o

8188eu-$(CONFIG_WAPI_SUPPORT) += rtw_wapi.o	\
					rtw_wapi_sms4.o

8188eu-y += $(_OS_INTFS_FILES)
8188eu-y += $(_HAL_INTFS_FILES)
8188eu-y += $(_OUTSRC_FILES)

8188eu-$(CONFIG_MP_INCLUDED) += rtw_mp.o

ifeq ($(CONFIG_RTL8723B), y)
8188eu-$(CONFIG_MP_INCLUDED)+= rtw_bt_mp.o
endif

obj-m := 8188eu.o

else

all: test modules

test:
	@if [ !  -e  ./.git ] ; then echo $(MSG); exit 1; fi;

modules:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KSRC) M=$(CURDIR)  modules

strip:
	$(CROSS_COMPILE)strip 8188eu.ko --strip-unneeded

install:
	@mkdir -p $(MODDESTDIR)
	install -p -m 644 8188eu.ko  $(MODDESTDIR)
	/sbin/depmod -a ${KVER}

uninstall:
	rm -f $(MODDESTDIR)/8188eu.ko
	/sbin/depmod -a ${KVER}

config_r:
	@echo "make config"
	/bin/bash script/Configure script/config.in


.PHONY: modules clean

clean:
	#$(MAKE) -C $(KSRC) M=$(CURDIR) clean
	cd core ; rm -fr *.mod.c *.mod *.o .*.cmd *.ko
	rm -fr Module.symvers ; rm -fr Module.markers ; rm -fr modules.order
	rm -fr *.mod.c *.mod *.o .*.cmd *.ko *~
	rm -fr .tmp_versions
endif

