
ifeq ($(CONFIG_SUPPORT_OPENWRT),y)
EXTRA_CFLAGS = -I$(src)/../mt7628_wifi/include \
				-I$(src)/../mt7628_wifi/embedded/include \
				-I$(src)/../mt7628_wifi/ate/include
DRV_NAME = mt7628
SRC_DIR = ../mt7628_wifi/embedded
obj-$(CONFIG_MT_AP_SUPPORT) += $(DRV_NAME).o
else
EXTRA_CFLAGS = -Idrivers/net/wireless/mt_wifi_7628/include \
				-Idrivers/net/wireless/mt_wifi_7628/embedded/include \
				-Idrivers/net/wireless/mt_wifi_7628/ate/include
DRV_NAME = mt_wifi_7628
SRC_DIR = ../mt_wifi_7628/embedded
obj-$(CONFIG_MT_AP_SUPPORT) += $(DRV_NAME).o
endif

ifeq ($(CONFIG_MT_DMA_CACHE_OPTIMIZE),y)
    EXTRA_CFLAGS += -DDMA_CACHE_OPTIMIZE
endif

########################################################
# Common files
########################################################
cmm_objs := $(SRC_DIR)/common/crypt_md5.o\
						$(SRC_DIR)/common/crypt_sha2.o\
						$(SRC_DIR)/common/crypt_hmac.o\
						$(SRC_DIR)/common/crypt_aes.o\
						$(SRC_DIR)/common/crypt_arc4.o\
						$(SRC_DIR)/common/mlme.o\
						$(SRC_DIR)/common/cmm_wep.o\
						$(SRC_DIR)/common/action.o\
						$(SRC_DIR)/common/ba_action.o\
						$(SRC_DIR)/../hw_ctrl/cmm_asic.o\
						$(SRC_DIR)/mgmt/mgmt_ht.o\
						$(SRC_DIR)/common/cmm_data.o\
						$(SRC_DIR)/common/rtmp_init.o\
						$(SRC_DIR)/common/rtmp_init_inf.o\
						$(SRC_DIR)/common/cmm_tkip.o\
						$(SRC_DIR)/common/cmm_aes.o\
						$(SRC_DIR)/common/cmm_sync.o\
						$(SRC_DIR)/common/eeprom.o\
						$(SRC_DIR)/common/cmm_sanity.o\
						$(SRC_DIR)/common/cmm_info.o\
						$(SRC_DIR)/common/cmm_cfg.o\
						$(SRC_DIR)/common/cmm_wpa.o\
						$(SRC_DIR)/common/cmm_radar.o\
						$(SRC_DIR)/common/spectrum.o\
						$(SRC_DIR)/common/rtmp_timer.o\
						$(SRC_DIR)/common/rt_channel.o\
						$(SRC_DIR)/common/rt_os_util.o\
						$(SRC_DIR)/common/cmm_profile.o\
						$(SRC_DIR)/common/scan.o\
						$(SRC_DIR)/common/cmm_cmd.o\
						$(SRC_DIR)/common/sys_log.o\
						$(SRC_DIR)/common/txpower.o\
						$(SRC_DIR)/common/op_class.o\
						$(SRC_DIR)/../chips/rtmp_chip.o\
						$(SRC_DIR)/mgmt/mgmt_hw.o\
						$(SRC_DIR)/mgmt/mgmt_entrytb.o\
						$(SRC_DIR)/tx_rx/wdev.o\
						$(SRC_DIR)/tx_rx/wdev_tx.o\
						$(SRC_DIR)/tx_rx/wdev_rx.o\
						$(SRC_DIR)/os/linux/rt_profile.o\
						$(SRC_DIR)/common/vendor.o\
						$(SRC_DIR)/common/mwds.o\
						$(SRC_DIR)/common/routing_tab.o

########################################################
# Rate adaptation related files
########################################################
rate_objs := $(SRC_DIR)/../rate_ctrl/ra_ctrl.o\
						$(SRC_DIR)/../rate_ctrl/alg_legacy.o

ifeq ($(CONFIG_MT_NEW_BW2040_COEXIST_SUPPORT),y)
	EXTRA_CFLAGS += -DNEW_BW2040_COEXIST_SUPPORT
endif

ifeq ($(CONFIG_MT_NEW_RATE_ADAPT_SUPPORT),y)
    EXTRA_CFLAGS += -DNEW_RATE_ADAPT_SUPPORT
    rate_objs += $(SRC_DIR)/../rate_ctrl/alg_grp.o

    ifeq ($(CONFIG_MT_AGS_SUPPORT),y)
        EXTRA_CFLAGS += -DAGS_ADAPT_SUPPORT
        rate_objs += $(SRC_DIR)/../rate_ctrl/alg_ags.o
    endif
endif


########################################################
# ASIC related files
########################################################
asic_objs := $(SRC_DIR)/../phy/phy.o

#ifeq ($(CONFIG_RLT_MAC),y)
#    EXTRA_CFLAGS += -DRLT_MAC
#
#    asic_objs += $(SRC_DIR)/../mac/ral_nmac.o
#endif

#ifeq ($(CONFIG_RTMP_MAC),y)
#    EXTRA_CFLAGS += -DRTMP_MAC
#
#    asic_objs += $(SRC_DIR)/../mac/ral_omac.o\
#		$(SRC_DIR)/../chips/rtxx_dft.o
#endif


########################################################
# Spec feature related files
########################################################
spec_objs += $(SRC_DIR)/common/ps.o

ifeq ($(CONFIG_MT_UAPSD),y)
    EXTRA_CFLAGS += -DUAPSD_SUPPORT -DUAPSD_DEBUG
    spec_objs += $(SRC_DIR)/common/uapsd.o
endif

#ifeq ($(CONFIG_RLT_MAC),y)
#	spec_objs += $(SRC_DIR)/common/ral_ps.o
#endif

#ifeq ($(CONFIG_RTMP_MAC),y)
#	spec_objs += $(SRC_DIR)/common/ral_ps.o
#endif

ifeq ($(CONFIG_MT_MAC),y)
	EXTRA_CFLAGS += -DMT_PS
	spec_objs += $(SRC_DIR)/common/mt_ps.o
	spec_objs += $(SRC_DIR)/common/mt_io.o
	spec_objs += $(SRC_DIR)/tx_rx/txs.o
endif

# WSC
ifeq ($(CONFIG_MT_WSC_INCLUDED),y)
    EXTRA_CFLAGS += -DWSC_INCLUDED -DWSC_SINGLE_TRIGGER

	ifneq ($(CONFIG_USER_GOAHEAD_HTTPD),)
        EXTRA_CFLAGS += -DSDK_GOAHEAD_HTTPD
    endif
    
    ifneq ($(CONFIG_USER_LIGHTY),)
        EXTRA_CFLAGS += -DSDK_USER_LIGHTY
    endif
    
    ifneq ($(CONFIG_MT_AP_SUPPORT),)
        EXTRA_CFLAGS += -DWSC_AP_SUPPORT
    endif

    ifneq ($(CONFIG_MT_STA_SUPPORT),)
        EXTRA_CFLAGS += -DWSC_STA_SUPPORT
    endif

    spec_objs += $(SRC_DIR)/common/wsc.o\
                    $(SRC_DIR)/common/wsc_tlv.o\
                    $(SRC_DIR)/common/crypt_dh.o\
                    $(SRC_DIR)/common/crypt_biginteger.o\
                    $(SRC_DIR)/common/wsc_ufd.o

    ifeq ($(CONFIG_MT_WSC_V2_SUPPORT),y)
        EXTRA_CFLAGS += -DWSC_V2_SUPPORT
        spec_objs += $(SRC_DIR)/common/wsc_v2.o
    endif
endif

#EXTRA_CFLAGS += -DANDLINK_FEATURE_SUPPORT
#CONFIG_MIXMODE_SUPPORT = y
ifeq ($(CONFIG_MIXMODE_SUPPORT),y)
    EXTRA_CFLAGS += -DMIXMODE_SUPPORT

    spec_objs += $(SRC_DIR)/ap/ap_mix_mode.o
endif

# WAPI
#ifeq ($(CONFIG_MT_WAPI_SUPPORT),y)
#    EXTRA_CFLAGS += -DWAPI_SUPPORT
#
#  spec_objs += $(SRC_DIR)/common/wapi.o\
#                $(SRC_DIR)/common/wapi_sms4.o\
#		$(SRC_DIR)/common/wapi_crypt.o
#		
#endif

# ACM
#ifeq ($(CONFIG_MT_WMM_ACM_SUPPORT),y)
#    EXTRA_CFLAGS += -DWMM_ACM_SUPPORT
#
#    spec_objs += $(SRC_DIR)/common/acm_edca.o\
#            $(SRC_DIR)/common/acm_comm.o\
#            $(SRC_DIR)/common/acm_iocl.o
#endif

#PMF
ifeq ($(CONFIG_MT_DOT11W_PMF_SUPPORT),y)
    EXTRA_CFLAGS += -DDOT11W_PMF_SUPPORT -DSOFT_ENCRYPT

    spec_objs += $(SRC_DIR)/common/pmf.o
endif

# LLTD
ifeq ($(CONFIG_MT_LLTD_SUPPORT),y)
    EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

# FTM
#ifeq ($(CONFIG_MT_FTM_SUPPORT),y)
#    EXTRA_CFLAGS += -DFTM_SUPPORT -DFTM_INITIATOR
#    EXTRA_CFLAGS += -DTMR_SUPPORT
#    EXTRA_CFLAGS += -DDOT11K_RRM_SUPPORT
#
#    spec_objs += $(SRC_DIR)/common/ftm.o
#    spec_objs += $(SRC_DIR)/common/tmr.o
#    spec_objs += $(SRC_DIR)/common/rrm_tlv.o \
#				 $(SRC_DIR)/common/rrm.o \
#				 $(SRC_DIR)/common/rrm_sanity.o
#endif

ifeq ($(CONFIG_MT_PASSPOINT_R2),y)
	EXTRA_CFLAGS += -DDOT11U_INTERWORKING -DCONFIG_DOT11V_WNM -DCONFIG_HOTSPOT -DCONFIG_HOTSPOT_R2

	spec_objs += $(SRC_DIR)/common/wnm.o\
				$(SRC_DIR)/common/gas.o\
				$(SRC_DIR)/common/hotspot.o
endif

ifeq ($(CONFIG_MT_TRACE_IPARP),y)
	EXTRA_CFLAGS += -DTRACELOG_IPARP
	spec_objs +=  $(SRC_DIR)/debug/tracelog_iparp.o
endif

# FT
ifeq ($(CONFIG_MT_DOT11R_FT_SUPPORT),y)
EXTRA_CFLAGS += -DDOT11R_FT_SUPPORT
spec_objs += $(SRC_DIR)/common/ft.o\
                    $(SRC_DIR)/common/ft_tlv.o\
                    $(SRC_DIR)/common/ft_iocl.o\
                    $(SRC_DIR)/common/ft_rc.o\
                    $(SRC_DIR)/ap/ap_ftkd.o
endif

# RR
ifeq ($(CONFIG_MT_DOT11K_RRM_SUPPORT),y)
EXTRA_CFLAGS += -DDOT11K_RRM_SUPPORT -DAP_SCAN_SUPPORT -DSCAN_SUPPORT -DAPPLE_11K_IOT
spec_objs += $(SRC_DIR)/common/rrm_tlv.o\
                    $(SRC_DIR)/common/rrm_sanity.o\
                    $(SRC_DIR)/common/rrm.o
endif

#SNIFFER
ifeq ($(CONFIG_MT_SNIFFER_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_SNIFFER_SUPPORT

    spec_objs += $(SRC_DIR)/sniffer/sniffer_radiotap.o\
                 $(SRC_DIR)/sniffer/sniffer_prism.o
endif

#CFG
ifeq ($(CONFIG_MT_CFG80211_SUPPORT),y)
    EXTRA_CFLAGS += -DRT_CFG80211_SUPPORT -DEXT_BUILD_CHANNEL_LIST

    spec_objs += $(SRC_DIR)/os/linux/cfg80211/cfg80211_scan.o\
        $(SRC_DIR)/os/linux/cfg80211/cfg80211_rx.o\
        $(SRC_DIR)/os/linux/cfg80211/cfg80211_tx.o\
        $(SRC_DIR)/os/linux/cfg80211/cfg80211_inf.o\
        $(SRC_DIR)/os/linux/cfg80211/cfg80211_ap.o\
        $(SRC_DIR)/os/linux/cfg80211/cfg80211drv.o\
                $(SRC_DIR)/os/linux/cfg80211/cfg80211_util.o\
                $(SRC_DIR)/os/linux/cfg80211/cfg80211.o
endif
#MAP
ifeq ($(CONFIG_MT_MAP_SUPPORT),y)
    EXTRA_CFLAGS += -DA4_CONN -DROUTING_TAB_SUPPORT -DWAPP_SUPPORT -DMAP_SUPPORT
    spec_objs += $(SRC_DIR)/common/map.o
    spec_objs += $(SRC_DIR)/common/a4_conn.o
    spec_objs += $(SRC_DIR)/common/routing_tab.o
    spec_objs +=  $(SRC_DIR)/wapp/wapp.o
endif
#MBO
ifeq ($(CONFIG_MT_MBO_SUPPORT),y)
    EXTRA_CFLAGS += -DMBO_SUPPORT -DWAPP_SUPPORT

    spec_objs += $(SRC_DIR)/common/mbo.o
    spec_objs +=  $(SRC_DIR)/wapp/wapp.o
endif
#
# Common Feature related files
#
func_objs :=

ifeq ($(CONFIG_MT_IGMP_SNOOP_SUPPORT),y)
    EXTRA_CFLAGS += -DIGMP_SNOOP_SUPPORT

    func_objs += $(SRC_DIR)/common/igmp_snoop.o
endif

ifeq ($(CONFIG_MT_BLOCK_NET_IF),y)
    EXTRA_CFLAGS += -DBLOCK_NET_IF

    func_objs += $(SRC_DIR)/common/netif_block.o
endif

ifeq ($(CONFIG_MT_SINGLE_SKU),y)
    EXTRA_CFLAGS += -DSINGLE_SKU_V2
endif

ifeq ($(CONFIG_MT_LED_CONTROL_SUPPORT),y)
    EXTRA_CFLAGS += -DLED_CONTROL_SUPPORT
    ifeq ($(CONFIG_MT_WSC_INCLUDED),y)
        EXTRA_CFLAGS += -DWSC_LED_SUPPORT
    endif

    func_objs += $(SRC_DIR)/common/rt_led.o
endif

ifeq ($(CONFIG_MT_SMART_CARRIER_SENSE_SUPPORT),y)
    EXTRA_CFLAGS += -DSMART_CARRIER_SENSE_SUPPORT
endif

########################################################
# AP feature related files
########################################################
ap_objs := $(SRC_DIR)/ap/ap.o\
            $(SRC_DIR)/ap/ap_assoc.o\
            $(SRC_DIR)/ap/ap_auth.o\
            $(SRC_DIR)/ap/ap_connect.o\
            $(SRC_DIR)/ap/ap_mlme.o\
            $(SRC_DIR)/ap/ap_sanity.o\
            $(SRC_DIR)/ap/ap_sync.o\
            $(SRC_DIR)/ap/ap_wpa.o\
            $(SRC_DIR)/ap/ap_data.o\
            $(SRC_DIR)/ap/ap_autoChSel.o\
            $(SRC_DIR)/ap/ap_qload.o\
            $(SRC_DIR)/ap/ap_cfg.o\
            $(SRC_DIR)/ap/ap_nps.o\
            $(SRC_DIR)/os/linux/ap_ioctl.o            

ifeq ($(CONFIG_MT_QOS_DLS_SUPPORT),y)
    EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
    ap_objs += $(SRC_DIR)/ap/ap_dls.o
endif

ifeq ($(CONFIG_MT_MBSS_SUPPORT),y)
    EXTRA_CFLAGS += -DMBSS_SUPPORT

    ifeq ($(CONFIG_MT_NEW_MBSSID_MODE),y)
        EXTRA_CFLAGS += -DNEW_MBSSID_MODE
        ifeq ($(CONFIG_MT_ENHANCE_NEW_MBSSID_MODE),y)
            EXTRA_CFLAGS += -DENHANCE_NEW_MBSSID_MODE
        endif
    endif

    ap_objs += $(SRC_DIR)/ap/ap_mbss.o\
            $(SRC_DIR)/ap/ap_mbss_inf.o
endif


ifeq ($(CONFIG_MT_WDS_SUPPORT),y)
    EXTRA_CFLAGS += -DWDS_SUPPORT

    ap_objs += $(SRC_DIR)/ap/ap_wds.o\
            $(SRC_DIR)/ap/ap_wds_inf.o\
            $(SRC_DIR)/common/client_wds.o
endif

ifeq ($(CONFIG_MT_APCLI_SUPPORT),y)
    EXTRA_CFLAGS += -DAPCLI_SUPPORT -DMAT_SUPPORT -DAPCLI_AUTO_CONNECT_SUPPORT -DAPCLI_AUTO_BW_TMP
	#EXTRA_CFLAGS += -DAPCLI_CONNECTION_TRIAL -DMULTI_APCLI_SUPPORT -DAPCLI_CERT_SUPPORT

    ap_objs += $(SRC_DIR)/ap/ap_apcli.o\
            $(SRC_DIR)/ap/ap_apcli_inf.o\
            $(SRC_DIR)/ap/apcli_assoc.o\
            $(SRC_DIR)/ap/apcli_auth.o\
            $(SRC_DIR)/ap/apcli_ctrl.o\
            $(SRC_DIR)/ap/apcli_sync.o\
            $(SRC_DIR)/common/cmm_mat.o\
            $(SRC_DIR)/common/cmm_mat_iparp.o\
            $(SRC_DIR)/common/cmm_mat_pppoe.o\
            $(SRC_DIR)/common/cmm_mat_ipv6.o

    ifeq ($(CONFIG_MT_MAC_REPEATER_SUPPORT),y)
        EXTRA_CFLAGS += -DMAC_REPEATER_SUPPORT

        ap_objs += $(SRC_DIR)/ap/ap_repeater.o
    endif

    ifeq ($(CONFIG_MT_APCLI_AUTO_BW_SUPPORT),y)
	EXTRA_CFLAGS += -DAPCLI_AUTO_BW_SUPPORT
	ap_objs += $(SRC_DIR)/ap/apcli_autobw.o
    endif	

    ifeq ($(CONFIG_MT_APCLI_CERT_SUPPORT),y)
	EXTRA_CFLAGS += -DAPCLI_CERT_SUPPORT
    endif
    
endif

ifeq ($(CONFIG_MT_IDS_SUPPORT),y)
    EXTRA_CFLAGS += -DIDS_SUPPORT

    ap_objs += $(SRC_DIR)/ap/ap_ids.o
endif

#ifeq ($(CONFIG_MT_NINTENDO_AP),y)
#    EXTRA_CFLAGS += -DNINTENDO_AP
#
#    ap_objs += $(SRC_DIR)/ap/ap_nintendo.o
#endif

#ifeq ($(CONFIG_MT_COC_SUPPORT),y)
#   EXTRA_CFLAGS += -DGREENAP_SUPPORT -DCOC_SUPPORT
#
#    ap_objs += $(SRC_DIR)/../hw_ctrl/greenap.o
#endif

ifeq ($(CONFIG_MT_ATE_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_ATE -DCONFIG_QA -DCONFIG_RT2880_ATE_CMD_NEW
endif

ifeq ($(CONFIG_MT_CON_WPS_SUPPORT), y)
    EXTRA_CFLAGS += -DCON_WPS -DCON_WPS_AP_SAME_UUID
endif

ifeq ($(CONFIG_MT_AIRPLAY_SUPPORT),y)
    EXTRA_CFLAGS += -DAIRPLAY_SUPPORT
endif

ifeq ($(CONFIG_MT_BAND_STEERING),y)
    EXTRA_CFLAGS += -DBAND_STEERING
    ap_objs +=  $(SRC_DIR)/ap/ap_band_steering.o
endif


ifeq ($(CONFIG_MT_OFFCHANNEL_SCAN_FEATURE),y)
    EXTRA_CFLAGS += -DOFFCHANNEL_SCAN_FEATURE
endif

#ifeq ($(CONFIG_MT_SNMP),y)
#EXTRA_CFLAGS += -DSNMP_SUPPORT
#endif

#ifeq ($(CONFIG_MT_EXT_CHANNEL_LIST),y)
#EXTRA_CFLAGS += -DEXT_BUILD_CHANNEL_LIST
#endif

ifeq ($(CONFIG_MT_MEMORY_OPTIMIZATION),y)
EXTRA_CFLAGS += -DMEMORY_OPTIMIZATION
endif

########################################################
# Linux system related files
########################################################
os_objs := $(SRC_DIR)/os/linux/rt_proc.o\
            $(SRC_DIR)/os/linux/rt_linux.o\
            $(SRC_DIR)/os/linux/rt_profile.o\
            $(SRC_DIR)/os/linux/rt_main_dev.o

#for MT7628 + MT7637E autobuild symbols conflict
ifeq ($(CONFIG_MT_TXRX_HOOK),y)
os_objs += $(SRC_DIR)/os/linux/rt_txrx_hook.o
endif

#ifeq ($(CONFIG_MT_WIFI_WORK_QUEUE_BH),y)
#    EXTRA_CFLAGS += -DWORKQUEUE_BH
#endif

#ifeq ($(CONFIG_MT_KTHREAD),y)
#    EXTRA_CFLAGS += -DKTHREAD_SUPPORT
#endif


########################################################
# chip related files
########################################################
ifeq ($(CONFIG_RALINK_MT7628),y)
EXTRA_CFLAGS += -DMT7628 -DMT_BBP -DMT_RF -DRTMP_RBUS_SUPPORT -DRTMP_RF_RW_SUPPORT -DMT_MAC -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT
EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT -DDMA_SCH_SUPPORT -DRTMP_EFUSE_SUPPORT
EXTRA_CFLAGS += -DCONFIG_ANDES_SUPPORT
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC
EXTRA_CFLAGS += -DNEW_MBSSID_MODE -DENHANCE_NEW_MBSSID_MODE
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
EXTRA_CFLAGS += -DFIFO_EXT_SUPPORT
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT
EXTRA_CFLAGS += -DUSE_BMC -DTHERMAL_PROTECT_SUPPORT -DCAL_FREE_IC_SUPPORT -DTRAFFIC_BASED_TXOP
EXTRA_CFLAGS += -DDATA_QUEUE_RESERVE -DEAPOL_QUEUE_SUPPORT -DDMA_RESET_SUPPORT -DWTBL_RATE_DEBUG -DCONFIG_BA_REORDER_MONITOR
EXTRA_CFLAGS += -DEDCCA_RB

chip_objs += $(SRC_DIR)/../chips/mt7628.o\
		$(SRC_DIR)/../hw_ctrl/cmm_asic_mt.o\
		$(SRC_DIR)/../hw_ctrl/cmm_chip_mt.o\
		$(SRC_DIR)/../mac/mt_mac.o\
		$(SRC_DIR)/mcu/mcu.o\
		$(SRC_DIR)/mcu/andes_core.o\
		$(SRC_DIR)/mcu/andes_mt.o\
		$(SRC_DIR)/mcu/led.o\
		$(SRC_DIR)/mcu/hal.o\
		$(SRC_DIR)/../phy/mt_rf.o\
		$(SRC_DIR)/../phy/rf.o\
		$(SRC_DIR)/../phy/mt_phy.o\
		$(SRC_DIR)/common/cmm_mac_pci.o\
		$(SRC_DIR)/common/cmm_data_pci.o\
		$(SRC_DIR)/common/ee_prom.o\
		$(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
		$(SRC_DIR)/hif/hif_pci.o\
		$(SRC_DIR)/os/linux/rbus_main_dev.o
endif


#
# Root 
#

$(DRV_NAME)-objs += $(ap_objs) $(cmm_objs) $(asic_objs) $(chip_objs) $(rate_objs)\
                    $(spec_objs) $(func_objs) $(os_objs)

$(DRV_NAME)-objs += $(SRC_DIR)/common/eeprom.o\
					$(SRC_DIR)/common/ee_flash.o\
					$(SRC_DIR)/common/ee_efuse.o

$(DRV_NAME)-objs += $(SRC_DIR)/common/cmm_mac_pci.o
$(DRV_NAME)-objs += $(SRC_DIR)/common/cmm_data_pci.o

$(DRV_NAME)-objs += $(SRC_DIR)/os/linux/rt_pci_rbus.o\
                    $(SRC_DIR)/os/linux/rt_rbus_pci_drv.o\
                    $(SRC_DIR)/os/linux/rt_rbus_pci_util.o

ifeq ($(CONFIG_MT_ATE_SUPPORT),y)
$(DRV_NAME)-objs += $(SRC_DIR)/../ate/ate_agent.o\
                    $(SRC_DIR)/../ate/qa_agent.o\
                    $(SRC_DIR)/../ate/mt_mac/mt_ate.o
endif

###################
#  CFLAGS
##################
EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT  -DLINUX \
               -Wall -Wstrict-prototypes -Wno-trigraphs -Wframe-larger-than=4096
#-DDBG_DIAGNOSE -DDBG_RX_MCS -DDBG_TX_MCS

EXTRA_CFLAGS += -DCONFIG_AP_SUPPORT -DSCAN_SUPPORT -DAP_SCAN_SUPPORT
EXTRA_CFLAGS += -DDOT11_N_SUPPORT -DDOT11N_DRAFT3 -DSTATS_COUNT_SUPPORT -DIAPP_SUPPORT -DDOT1X_SUPPORT
EXTRA_CFLAGS += -DIP_ASSEMBLY
EXTRA_CFLAGS += -DCONFIG_RA_NAT_NONE
EXTRA_CFLAGS += -DDBG

#provide busy time statistics for every TBTT */
#EXTRA_CFLAGS += -DQLOAD_FUNC_BUSY_TIME_STATS 

# provide busy time alarm mechanism 
# use the function to avoid to locate in some noise environments 
#EXTRA_CFLAGS += -DQLOAD_FUNC_BUSY_TIME_ALARM

ifeq ($(CONFIG_MT_ROAMING_ENHANCE_SUPPORT),y)
ccflags-y += -DROAMING_ENHANCE_SUPPORT
endif

ifeq ($(CONFIG_MT_SECURITY_IMPROVEMENT_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_SECURITY_IMPROVEMENT_SUPPORT
    EXTRA_CFLAGS += -DAPCLI_SECURITY_IMPROVEMENT_SUPPORT
endif

ifeq ($(CONFIG_MT_WPA3_SUPPORT),y)
    EXTRA_CFLAGS += -DDOT11_SAE_SUPPORT
    SRC_EMBEDDED_DIR = $(SRC_DIR)
    spec_objs += $(SRC_EMBEDDED_DIR)/common/sae.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/crypt_bignum.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/crypt_biginteger.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/bn_lib.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/ecc.o
    EXTRA_CFLAGS += -DDOT11_SAE_OPENSSL_BN
    ifeq ($(CONFIG_MT_APCLI_SUPPORT),y)
        EXTRA_CFLAGS += -DAPCLI_SAE_SUPPORT
    endif
endif

ifeq ($(CONFIG_MT_OWE_SUPPORT),y)
    EXTRA_CFLAGS += -DCONFIG_OWE_SUPPORT -DDOT11_SAE_OPENSSL_BN
    SRC_EMBEDDED_DIR = $(SRC_DIR)
    spec_objs += $(SRC_EMBEDDED_DIR)/common/owe.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/crypt_bignum.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/crypt_biginteger.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/bn_lib.o
    spec_objs += $(SRC_EMBEDDED_DIR)/common/ecc.o
    ifeq ($(CONFIG_MT_APCLI_SUPPORT),y)
        EXTRA_CFLAGS += -DAPCLI_OWE_SUPPORT
    endif
endif

MODULE_FLAGS=$(EXTRA_CFLAGS)
export MODULE_FLAGS
obj-m+=$(SRC_DIR)/tools/plug_in/

