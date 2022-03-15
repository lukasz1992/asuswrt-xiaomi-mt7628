/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2009, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************


	Module Name:
	mwds.c
	Abstract:
	This is MWDS feature used to process those 4-addr of connected APClient or STA.
	Revision History:
	Who          When          What
    ---------    ----------    ----------------------------------------------
 */
#ifdef MWDS
#include "rt_config.h"

VOID MWDSAPPeerEnable(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY * pEntry,
	IN BOOLEAN bWTBLUpdate)
{
	BSS_STRUCT *pMbss = NULL;
#ifdef WSC_AP_SUPPORT
	BOOLEAN bWPSRunning = FALSE;
#endif /* WSC_AP_SUPPORT */
	BOOLEAN bKeySuccess = FALSE;
	UCHAR ifIndex;

	if (!pEntry || !IS_ENTRY_CLIENT(pEntry))
		return;

	ifIndex = pEntry->func_tb_idx;
	if (ifIndex >= HW_BEACON_MAX_NUM)
		return;
	pMbss = &pAd->ApCfg.MBSSID[ifIndex];
#ifdef WSC_AP_SUPPORT
	if (pMbss &&
		(pMbss->WscControl.WscConfMode != WSC_DISABLE) &&
		(pMbss->WscControl.bWscTrigger == TRUE))
		bWPSRunning = TRUE;
#endif /* WSC_AP_SUPPORT */
    /* To check and remove entry which is created from another side. */
	MWDSProxyEntryDelete(pAd, ifIndex, pEntry->Addr);
	if (
 #ifdef WSC_AP_SUPPORT
		!bWPSRunning &&
#endif /* WSC_AP_SUPPORT */
		pEntry->bSupportMWDS &&
		pEntry->wdev &&
		pEntry->wdev->bSupportMWDS
	) {
	if (bWTBLUpdate) {
		/* Set key material to Asic */
		if (TRUE) {
			pEntry->bEnableMWDS = TRUE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				("MWDSAPPeerEnable enabled MWDS for entry : %02x-%02x-%02x-%02x-%02x-%02x\n",
				PRINT_MAC(pEntry->Addr)));
			bKeySuccess = TRUE;
			} else
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Fail to install WPA Key!\n"));
			} else {
				bKeySuccess = TRUE; /*Open*/
				pEntry->bEnableMWDS = TRUE; /* need to set before RTMP_STA_ENTRY_ADD*/
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					("MWDSAPPeerEnable enabled MWDS for entry : %02x-%02x-%02x-%02x-%02x-%02x\n",
					PRINT_MAC(pEntry->Addr)));
		}
			if (bKeySuccess) {
				SET_MWDS_OPMODE_AP(pEntry);
				MWDSConnEntryUpdate(pAd, ifIndex, pEntry->wcid);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SET_MWDS_OPMODE_AP OK!\n"));
				return;
			}
	}

	MWDSAPPeerDisable(pAd, pEntry);
}

VOID MWDSAPPeerDisable(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY * pEntry)
{
	UCHAR ifIndex;

	if (!pEntry)
		return;

	ifIndex = pEntry->func_tb_idx;
	if (ifIndex >= HW_BEACON_MAX_NUM)
		return;

	if (IS_MWDS_OPMODE_AP(pEntry))
		MWDSConnEntryDelete(pAd, ifIndex, pEntry->wcid);

	if (pEntry->bEnableMWDS)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("MWDSAPPeerDisable: Disable MWDS for entry : %02x-%02x-%02x-%02x-%02x-%02x\n",
			PRINT_MAC(pEntry->Addr)));

	pEntry->bSupportMWDS = FALSE;
	pEntry->bEnableMWDS = FALSE;
	SET_MWDS_OPMODE_NONE(pEntry);
}

#ifdef APCLI_SUPPORT
VOID MWDSAPCliPeerEnable(
	IN PRTMP_ADAPTER pAd,
	IN PAPCLI_STRUCT pApCliEntry,
	IN MAC_TABLE_ENTRY * pEntry,
	IN BOOLEAN bWTBLUpdate)
{

#ifdef WSC_AP_SUPPORT
	BOOLEAN bWPSRunning = FALSE;
#endif /* WSC_AP_SUPPORT */
	BOOLEAN bKeySuccess = FALSE;

	if (!pApCliEntry || !pEntry || !IS_ENTRY_APCLI(pEntry))
		return;

#ifdef WSC_AP_SUPPORT
if (((pApCliEntry->WscControl.WscConfMode != WSC_DISABLE) &&
	(pApCliEntry->WscControl.bWscTrigger == TRUE)))
	bWPSRunning = TRUE;
#endif /* WSC_AP_SUPPORT */

	if (
#ifdef WSC_AP_SUPPORT
		!bWPSRunning &&
#endif /* WSC_AP_SUPPORT */
		pApCliEntry->MlmeAux.bSupportMWDS &&
		pApCliEntry->wdev.bSupportMWDS) {
		if (bWTBLUpdate) {
				/* Set key material to Asic */
			if (TRUE) {
				pApCliEntry->bEnableMWDS = TRUE;
				pEntry->bEnableMWDS = TRUE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					("MWDSAPCliPeerEnable enabled MWDS for entry : %02x-%02x-%02x-%02x-%02x-%02x\n"
					, PRINT_MAC(pEntry->Addr)));
				bKeySuccess = TRUE;
			} else
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Fail to install WPA Key!\n"));
		} else {
			bKeySuccess = TRUE;
			pApCliEntry->bEnableMWDS = TRUE;
			pEntry->bEnableMWDS = TRUE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
					("MWDSAPCliPeerEnable:enabled MWDS for entry :%02x-%02x-%02x-%02x-%02x-%02x\n",
					PRINT_MAC(pEntry->Addr)));
	}
		if (bKeySuccess) {
			SET_MWDS_OPMODE_APCLI(pEntry);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SET_MWDS_OPMODE_APCLI OK!\n"));
			return;
			}
	}

	MWDSAPCliPeerDisable(pAd, pApCliEntry, pEntry);
}

VOID MWDSAPCliPeerDisable(
	IN PRTMP_ADAPTER pAd,
	IN PAPCLI_STRUCT pApCliEntry,
	IN MAC_TABLE_ENTRY * pEntry)
{
	if (!pApCliEntry || !pEntry)
		return;

	if (pEntry->bEnableMWDS)
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("MWDSAPCliPeerDisable: Disable MWDS for entry :%02x-%02x-%02x-%02x-%02x-%02x\n",
			PRINT_MAC(pEntry->Addr)));
	pApCliEntry->bEnableMWDS = FALSE;
	pEntry->bEnableMWDS = FALSE;
	SET_MWDS_OPMODE_NONE(pEntry);
}
#endif /* APCLI_SUPPORT */

INT MWDSEnable(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN isAP,
	IN BOOLEAN isDevOpen)
{
	struct wifi_dev *wdev = NULL;

	if (isAP) {
		if (ifIndex < HW_BEACON_MAX_NUM) {
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			if (!wdev->bSupportMWDS) {
				wdev->bSupportMWDS = TRUE;
				MWDSAPUP(pAd, ifIndex);
			}
		}
	}
#ifdef APCLI_SUPPORT
	else {
			if (ifIndex < MAX_APCLI_NUM) {
				wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
				if (!wdev->bSupportMWDS)
					wdev->bSupportMWDS = TRUE;
			}
	}
#endif /* APCLI_SUPPORT */
	pAd->mwds_interface_count++;
	return TRUE;
}

INT MWDSDisable(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR ifIndex,
	IN BOOLEAN isAP,
	IN BOOLEAN isDevClose)
{
	struct wifi_dev *wdev = NULL;

	if (isAP) {
		if (ifIndex < HW_BEACON_MAX_NUM) {
			wdev = &pAd->ApCfg.MBSSID[ifIndex].wdev;
			if (wdev && wdev->bSupportMWDS) {
				wdev->bSupportMWDS = FALSE;
				MWDSAPDown(pAd, ifIndex);
			}
		}
	}
#ifdef APCLI_SUPPORT
	else {
		if (ifIndex < MAX_APCLI_NUM) {
			wdev = &pAd->ApCfg.ApCliTab[ifIndex].wdev;
			if (wdev && wdev->bSupportMWDS)
				wdev->bSupportMWDS = FALSE;
		}
	}
#endif /* APCLI_SUPPORT */
	pAd->mwds_interface_count--;

	return TRUE;
}

INT	Set_Enable_MWDS_Proc(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN Enable,
	IN BOOLEAN isAP)
{
	POS_COOKIE	pObj;
	UCHAR	ifIndex = 0;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (isAP) {
		ifIndex = pObj->ioctl_if;
		pAd->ApCfg.MBSSID[ifIndex].wdev.bDefaultMwdsStatus = (Enable == 0)?FALSE:TRUE;
	}
#ifdef APCLI_SUPPORT
	else {
		if (pObj->ioctl_if_type != INT_APCLI)
			return FALSE;
		ifIndex = pObj->ioctl_if;
		pAd->ApCfg.ApCliTab[ifIndex].wdev.bDefaultMwdsStatus = (Enable == 0)?FALSE:TRUE;
	}
#endif /* APCLI_SUPPORT */

	if (Enable)
		MWDSEnable(pAd, ifIndex, isAP, FALSE);
	else
		MWDSDisable(pAd, ifIndex, isAP, FALSE);

	return TRUE;
}

INT Set_Ap_MWDS_Proc(
	IN PRTMP_ADAPTER pAd,
	IN RTMP_STRING * arg)
{
	UCHAR Enable;

	Enable = simple_strtol(arg, 0, 10);

	return Set_Enable_MWDS_Proc(pAd, Enable, TRUE);
}

#ifdef APCLI_SUPPORT
INT Set_ApCli_MWDS_Proc(
	IN  PRTMP_ADAPTER pAd,
	IN  PSTRING arg
)
{
	UCHAR Enable;
	Enable = simple_strtol(arg, 0, 10);
	return Set_Enable_MWDS_Proc(pAd, Enable, FALSE);
}
#endif

VOID rtmp_read_MWDS_from_file(
	IN PRTMP_ADAPTER pAd,
	RTMP_STRING *tmpbuf,
	RTMP_STRING *buffer)
{
	RTMP_STRING	*tmpptr = NULL;

#ifdef CONFIG_AP_SUPPORT
	/* ApMWDS */
	if (RTMPGetKeyParameter("ApMWDS", tmpbuf, 256, buffer, TRUE)) {
		INT	Value;
		UCHAR i = 0;

		for (i = 0, tmpptr = rstrtok(tmpbuf, ";"); tmpptr; tmpptr = rstrtok(NULL, ";"), i++) {
			if (i >= pAd->ApCfg.BssidNum)
				break;

			Value = (INT) simple_strtol(tmpptr, 0, 10);
			if (Value == 0)
				MWDSDisable(pAd, i, TRUE, FALSE);
			else
				MWDSEnable(pAd, i, TRUE, FALSE);
			pAd->ApCfg.MBSSID[i].wdev.bDefaultMwdsStatus = (Value == 0)?FALSE:TRUE;

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApMWDS=%d\n", Value));
		}
	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef APCLI_SUPPORT
	/* ApCliMWDS */
	if (RTMPGetKeyParameter("ApCliMWDS", tmpbuf, 256, buffer, TRUE)) {
		INT	Value;
		UCHAR i = 0;

		for (i = 0, tmpptr = rstrtok(tmpbuf, ";"); tmpptr; tmpptr = rstrtok(NULL, ";"), i++) {
			if (i >= MAX_APCLI_NUM)
				break;

			Value = (INT) simple_strtol(tmpptr, 0, 10);
			if (Value == 0)
				MWDSDisable(pAd, i, FALSE, FALSE);
			else
				MWDSEnable(pAd, i, FALSE, FALSE);
			pAd->ApCfg.ApCliTab[i].wdev.bDefaultMwdsStatus = (Value == 0)?FALSE:TRUE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("ApCliMWDS=%d\n", Value));
		}
	}
#endif /* APCLI_SUPPORT */
}

#endif /* MWDS */
