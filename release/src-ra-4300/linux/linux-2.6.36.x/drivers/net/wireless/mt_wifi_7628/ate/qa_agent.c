/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name:
	qa_agent.c
*/

#include "rt_config.h"

static INT ResponseToQA(
	HQA_CMD_FRAME *HqaCmdFrame,
	RTMP_IOCTL_INPUT_STRUCT	*WRQ,
	INT32 Length,
	INT32 Status)
{
	INT ret = 0;

	HqaCmdFrame->Length = OS_HTONS((Length));

	Status = OS_HTONS((Status));
	NdisMoveMemory(HqaCmdFrame->Data, &Status, 2);

	WRQ->u.data.length = sizeof((HqaCmdFrame)->MagicNo) + sizeof((HqaCmdFrame)->Type)
							+ sizeof((HqaCmdFrame)->Id) + sizeof((HqaCmdFrame)->Length)
							+ sizeof((HqaCmdFrame)->Sequence) + OS_NTOHS((HqaCmdFrame)->Length);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("WRQ->u.data.length = %d\n", WRQ->u.data.length));

	if (copy_to_user(WRQ->u.data.pointer, (UCHAR *)(HqaCmdFrame), WRQ->u.data.length))
	{

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("copy_to_user() fail in %s\n", __FUNCTION__));
		ret = (-EFAULT);
	}
	else
	{
	}

	return ret;
}


static INT32 HQA_OpenAdapter(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ATECtrl->bQAEnabled = TRUE;

	/* Prepare feedback as soon as we can to avoid QA timeout. */
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	Ret = ATEOp->ATEStart(pAd);

	return Ret;
}


static INT32 HQA_CloseAdapter(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ATECtrl->bQAEnabled = FALSE;

	/* Prepare feedback as soon as we can to avoid QA timeout. */
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	Ret = RTMP_AP_STOP_ATE(pAd, ATEOp);

	return Ret;
}


static INT32 HQA_StartTx(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 TxCount;
	UINT16 TxLength;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	NdisMoveMemory(&TxCount, HqaCmdFrame->Data, 4);
	TxCount = OS_NTOHL(TxCount);

	NdisMoveMemory(&TxLength, HqaCmdFrame->Data + 4, 2);
	TxLength = OS_NTOHS(TxLength);

	if (TxCount == 0)
		ATECtrl->TxCount = 0xFFFFFFFF;
	else
		ATECtrl->TxCount = TxCount;
	
	if(TxLength>MAX_TEST_PKT_LEN)
		TxLength = MAX_TEST_PKT_LEN;
	if(TxLength<MIN_TEST_PKT_LEN)
		TxLength = MIN_TEST_PKT_LEN;

	ATECtrl->TxLength = TxLength;
	ATECtrl->bQATxStart = TRUE;

	Ret = ATEOp->StartTx(pAd);

	if (ATECtrl->bQATxStart == TRUE)
		ATECtrl->TxStatus = 1;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_StartTxExt(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_StartTxContiTx(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_StartTxCarrier(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_StartRx(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ATECtrl->bQARxStart = TRUE;

	Ret = ATEOp->StartRx(pAd);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_StopTx(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 Mode;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	Mode = ATECtrl->Mode;
	ATECtrl->Mode &= ATE_TXSTOP;
	ATECtrl->bQATxStart = FALSE;

	Ret = ATEOp->StopTx(pAd, Mode);

	ATECtrl->TxStatus = 0;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_StopContiTx(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    MtAsicStopContinousTx(pAd);
	ATECtrl->Mode &= ~ATE_TXCONT;
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_StopTxCarrier(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_StopRx(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ATECtrl->Mode &= ATE_RXSTOP;
	ATECtrl->bQARxStart = FALSE;

	Ret = ATEOp->StopRx(pAd);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_SetTxPath(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    INT16 Value = 0;
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	NdisMoveMemory((PUCHAR)&Value, HqaCmdFrame->Data, 2);
	Value = OS_NTOHS(Value);

    Ret = ATEOp->SetTxAntenna(pAd, (CHAR)Value);

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_SetRxPath(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    INT16 Value = 0;
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	NdisMoveMemory((PUCHAR)&Value, HqaCmdFrame->Data, 2);
	Value = OS_NTOHS(Value);

    Ret = ATEOp->SetRxAntenna(pAd, (CHAR)Value);

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_SetTxIPG(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    INT32 Value = 0;
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	NdisMoveMemory((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	
	if(Value>15)
		Value = 15;
	else if(Value == 0)
		Value = 1;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s, val:%d,0x%04x\n", __FUNCTION__,Value,Value));

    ATEOp->SetAIFS(pAd, (UINT32)Value);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;

}


static INT32 HQA_SetTxPower0(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT16 Value = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	NdisMoveMemory((PUCHAR)&Value, HqaCmdFrame->Data, 2);
	Value = OS_NTOHS(Value);

	Ret = ATEOp->SetTxPower0(pAd, Value);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HAQ_SetTxPower1(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT16 Value = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	NdisMoveMemory((PUCHAR)&Value, HqaCmdFrame->Data, 2);
	Value = OS_NTOHS(Value);

	Ret = ATEOp->SetTxPower1(pAd, Value);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static HQA_CMD_HANDLER HQA_CMD_SET0[] =
{
	/* cmd id start from 0x1000 */
	HQA_OpenAdapter,	/* 0x1000 */
	HQA_CloseAdapter,	/* 0x1001 */
	HQA_StartTx,		/* 0x1002 */
	HQA_StartTxExt,		/* 0x1003 */
	HQA_StartTxContiTx,	/* 0x1004 */
	HQA_StartTxCarrier,	/* 0x1005 */
	HQA_StartRx,		/* 0x1006 */
	HQA_StopTx,			/* 0x1007 */
	HQA_StopContiTx,	/* 0x1008 */
	HQA_StopTxCarrier,	/* 0x1009 */
	HQA_StopRx,			/* 0x100A */
	HQA_SetTxPath,		/* 0x100B */
	HQA_SetRxPath,		/* 0x100C */
	HQA_SetTxIPG,		/* 0x100D */
	HQA_SetTxPower0,	/* 0x100E */
	HAQ_SetTxPower1,	/* 0x100F */
};


static INT32 HQA_SetChannel(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Value;
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	NdisMoveMemory((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = OS_NTOHL(Value);

	ATECtrl->Channel = Value;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Channel = %d, BW = %d\n", __FUNCTION__, ATECtrl->Channel, ATECtrl->BW));

	if (ATECtrl->BW == BW_20)
		ATECtrl->ControlChl = ATECtrl->Channel;
	else if (ATECtrl->BW == BW_40)
		ATECtrl->ControlChl = ATECtrl->Channel - 2;

	Ret = ATEOp->SetChannel(pAd, Value);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_SetPreamble(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    INT32 Value = 0;
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    NdisMoveMemory((PUCHAR)&Value, HqaCmdFrame->Data, 4);
    Value = OS_NTOHL(Value);
    ATECtrl->PhyMode = (UCHAR)Value;

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_SetRate(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    INT32 Value = 0;
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	NdisMoveMemory((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = OS_NTOHL(Value);
    ATECtrl->Mcs = (UCHAR)Value;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, value:0x%x\n", __FUNCTION__, ATECtrl->Mcs));
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_SetNss(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetSystemBW(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Value;
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	NdisMoveMemory((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = OS_NTOHL(Value);

	ATECtrl->BW = Value;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Channel = %d, BW = %d\n", __FUNCTION__, ATECtrl->Channel, ATECtrl->BW));

	if (ATECtrl->BW == BW_20)
		ATECtrl->ControlChl = ATECtrl->Channel;
	else if (ATECtrl->BW == BW_40)
		ATECtrl->ControlChl = ATECtrl->Channel - 2;

	Ret = ATEOp->SetBW(pAd, Value);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_SetPerPktBW(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetPrimaryBW(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetFreqOffset(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    INT32 Value = 0;
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	NdisMoveMemory((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = OS_NTOHL(Value);

    ATEOp->SetTxFreqOffset(pAd, (UINT32)Value);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetAutoResponder(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
	
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetTssiOnOff(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    INT32 Value = 0, WFSel;
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    /* ON/OFF:4 WF Sel:4 */
	NdisMoveMemory((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = OS_NTOHL(Value);

	NdisMoveMemory((PUCHAR)&WFSel, HqaCmdFrame->Data + 4, 4);
	WFSel = OS_NTOHL(WFSel);

    ATEOp->SetTSSI(pAd, (CHAR)WFSel, (CHAR)Value);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_SetRxHighLowTemperatureCompensation(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static HQA_CMD_HANDLER HQA_CMD_SET1[] =
{
	/* cmd id start from 0x1100 */
	HQA_SetChannel,				/* 0x1100 */
	HQA_SetPreamble,			/* 0x1101 */
	HQA_SetRate,				/* 0x1102 */
	HQA_SetNss,					/* 0x1103 */
	HQA_SetSystemBW,			/* 0x1104 */
	HQA_SetPerPktBW,			/* 0x1105 */
	HQA_SetPrimaryBW,			/* 0x1106 */
	HQA_SetFreqOffset,			/* 0x1107 */
	HQA_SetAutoResponder,		/* 0x1108 */
	HQA_SetTssiOnOff,			/* 0x1109 */
	HQA_SetRxHighLowTemperatureCompensation,	/* 0x110A */
};


static INT32 HQA_ResetTxRxCounter(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
		
	MtAsicGetRxStat(pAd, HQA_RX_RESET_PHY_COUNT); 
	MtAsicGetRxStat(pAd, HQA_RX_RESET_MAC_COUNT); 

    ATECtrl->RxTotalCnt = 0;
    ATECtrl->TxDoneCount = 0;
    ATECtrl->RxMacFCSErrCount = 0;
    ATECtrl->RxMacMdrdyCount = 0;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_GetStatistics(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_GetRxOKData(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_GetRxOKOther(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_GetRxAllPktCount(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_GetTxTransmitted(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 Value = 0;
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

    Value = ATECtrl->TxDoneCount;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: TxDoneCount = %d\n", __FUNCTION__, ATECtrl->TxDoneCount));

	Value = OS_HTONL(Value);

	NdisMoveMemory(HqaCmdFrame->Data + 2, &Value, 4);

	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	return Ret;
}


static INT32 HQA_GetHwCounter(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_CalibrationOperation(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_GPIORegRead(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Offset, Value;

	NdisMoveMemory(&Offset, HqaCmdFrame->Data, 4);
	Offset = OS_NTOHL(Offset);

	mt_mac_GPIO_range_mapping(pAd, &Offset);
	RTMP_IO_READ32(pAd, Offset, &Value);
	Value = OS_HTONL(Value);
	NdisMoveMemory(HqaCmdFrame->Data + 2, &Value, 4);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: Offset = %x, Value = %x\n", __func__, Offset, Value));

	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);

	return Ret;
}


static INT32 HQA_GPIORegWrite(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Offset, Value;

	NdisMoveMemory(&Offset, HqaCmdFrame->Data, 4);
	NdisMoveMemory(&Value, HqaCmdFrame->Data + 4, 4);

	Offset = OS_NTOHL(Offset);
	Value = OS_NTOHL(Value);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: Offset = %x, Value = %x\n", __func__, Offset, Value));

	mt_mac_GPIO_range_mapping(pAd, &Offset);
	RTMP_IO_WRITE32(pAd, Offset, Value);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static HQA_CMD_HANDLER HQA_CMD_SET2[] =
{
	/* cmd id start from 0x1200 */
	HQA_ResetTxRxCounter,	/* 0x1200 */
	HQA_GetStatistics,		/* 0x1201 */
	HQA_GetRxOKData,		/* 0x1202 */
	HQA_GetRxOKOther,		/* 0x1203 */
	HQA_GetRxAllPktCount,	/* 0x1204 */
	HQA_GetTxTransmitted,	/* 0x1205 */
	HQA_GetHwCounter,		/* 0x1206 */
	HQA_CalibrationOperation,	/* 0x1207 */
	NULL,					/* 0x1208 */
	NULL,					/* 0x1209 */
	NULL,					/* 0x120A */
	NULL,					/* 0x120B */
	NULL,					/* 0x120C */
	NULL,					/* 0x120D */
	HQA_GPIORegRead,		/* 0x120E */
	HQA_GPIORegWrite,		/* 0x120F */
};


static VOID NdisMoveMemory_exs(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, ULONG len)
{
	ULONG i;
	{
		USHORT *pDst, *pSrc;

		pDst = (USHORT *) dst;
		pSrc = (USHORT *) src;

		for (i =0; i < (len >> 1); i++)
		{
			*pDst = OS_NTOHS(*pSrc);
			pDst++;
			pSrc++;
		}

		if ((len % 2) != 0)
		{
			NdisMoveMemory(pDst, pSrc, (len % 2));
			*pDst = OS_NTOHS(*pDst);
		}
	}
	return;
}


static INT32 HQA_MacBbpRegRead(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Offset, Value;
	BOOLEAN IsFound;

	NdisMoveMemory(&Offset, HqaCmdFrame->Data, 4);
	Offset = OS_NTOHL(Offset);

	IsFound = mt_mac_cr_range_mapping(pAd, &Offset);

	if (!IsFound)
	{
		UINT32 RemapBase, RemapOffset;
		UINT32 RestoreValue;
		RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
		RemapBase = GET_REMAP_2_BASE(Offset) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(Offset);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
	}
	else
	{
		RTMP_IO_READ32(pAd, Offset, &Value);
	}

	Value = OS_HTONL(Value);
	NdisMoveMemory(HqaCmdFrame->Data + 2, &Value, 4);
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Offset = %x, Value = %x\n", __FUNCTION__, Offset, Value));

	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);

	return Ret;
}


static INT32 HQA_MacBbpRegWrite(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Offset, Value;
	BOOLEAN IsFound;

	NdisMoveMemory(&Offset, HqaCmdFrame->Data, 4);
	NdisMoveMemory(&Value, HqaCmdFrame->Data + 4, 4);

	Offset = OS_NTOHL(Offset);
	Value = OS_NTOHL(Value);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Offset = %x, Value = %x\n", __FUNCTION__, Offset, Value));

	IsFound = mt_mac_cr_range_mapping(pAd, &Offset);

	if (!IsFound)
	{
		UINT32 RemapBase, RemapOffset;
		UINT32 RestoreValue;
		RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
		RemapBase = GET_REMAP_2_BASE(Offset) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(Offset);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		RTMP_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
	}
	else
	{
		RTMP_IO_WRITE32(pAd, Offset, Value);
	}

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


VOID RTMP_IO_READ_BULK(PRTMP_ADAPTER pAd, UCHAR *Dst, UINT32 Offset, UINT32 Len)
{
	UINT32 Index, Value = 0;
	UCHAR *pDst;
	BOOLEAN IsFound;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("\n\n"));

	for (Index = 0 ; Index < (Len >> 2); Index++)
	{
		pDst = (Dst + (Index << 2));

		IsFound = mt_mac_cr_range_mapping(pAd, &Offset);

		if (!IsFound)
		{
			UINT32 RemapBase, RemapOffset;
			UINT32 RestoreValue;
			RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
			RemapBase = GET_REMAP_2_BASE(Offset) << 19;
			RemapOffset = GET_REMAP_2_OFFSET(Offset);
			RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
			RTMP_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
			RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
		}
		else
		{
			RTMP_IO_READ32(pAd, Offset, &Value);
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s: Offset = %x, Value = %x\n", __FUNCTION__, Offset, Value));

		Value = OS_HTONL(Value);
		memmove(pDst, &Value, 4);
		Offset += 4;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("\n\n"));

	return;
}


static INT32 HQA_MACBbpRegBulkRead(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Offset;
	UINT16 Len, Tmp;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	NdisMoveMemory(&Offset, HqaCmdFrame->Data, 4);
	Offset = OS_NTOHL(Offset);

	NdisMoveMemory(&Len, HqaCmdFrame->Data + 4, 2);
	Len = OS_NTOHS(Len);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Offset = %x, Len(unit: 4bytes) = %d\n", __FUNCTION__, Offset, Len));

	if (Len > 371)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : length requested is too large, make it smaller\n", __FUNCTION__));
		HqaCmdFrame->Length = OS_HTONS(2);
		Tmp = OS_HTONS(1);
		NdisMoveMemory(HqaCmdFrame->Data, &Tmp, 2);

		return -EFAULT;
	}

	RTMP_IO_READ_BULK(pAd, HqaCmdFrame->Data + 2, Offset, (Len << 2));/* unit in four bytes*/

	ResponseToQA(HqaCmdFrame, WRQ, 2 + (Len << 2), Ret);

	return Ret;
}


static INT32 HQA_RfRegBulkRead(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0, Index;
	UINT32 WfSel, Offset, Length, Value;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	NdisMoveMemory(&WfSel, HqaCmdFrame->Data, 4);
	WfSel = OS_NTOHL(WfSel);

	NdisMoveMemory(&Offset, HqaCmdFrame->Data + 4, 4);
	Offset = OS_NTOHL(Offset);

	NdisMoveMemory(&Length,  HqaCmdFrame->Data + 8, 4);
	Length = OS_NTOHL(Length);

	for (Index = 0; Index < Length; Index++)
	{
		Ret = ATEOp->RfRegRead(pAd, WfSel, Offset + Index * 4, &Value);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Wfsel = %d, Offset = %x, Value = %x\n", __FUNCTION__, WfSel,
																Offset + Index * 4, Value));

		if (Ret)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Wfsel = %d, Offset = %x, Value = %x fail\n", WfSel,
																Offset + Index * 4, Value));
			break;
		}

		Value = OS_HTONL(Value);
		NdisMoveMemory(HqaCmdFrame->Data + 2 + (Index * 4), &Value, 4);
	}

	ResponseToQA(HqaCmdFrame, WRQ, 2 + (Length * 4), Ret);

	return Ret;
}


static INT32 HQA_RfRegBulkWrite(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0, Index;
	UINT32 WfSel, Offset, Length, Value;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	NdisMoveMemory(&WfSel, HqaCmdFrame->Data, 4);
	WfSel = OS_NTOHL(WfSel);

	NdisMoveMemory(&Offset, HqaCmdFrame->Data + 4, 4);
	Offset = OS_NTOHL(Offset);

	NdisMoveMemory(&Length,  HqaCmdFrame->Data + 8, 4);
	Length = OS_NTOHL(Length);

	for (Index = 0; Index < Length; Index++)
	{
		NdisMoveMemory(&Value, HqaCmdFrame->Data + 12 + (Index * 4), 4);
		Value = OS_NTOHL(Value);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Wfsel = %d, Offset = %x, Value = %x\n", __FUNCTION__,
												WfSel,Offset + Index * 4, Value));

		Ret = ATEOp->RfRegWrite(pAd, WfSel, Offset + Index * 4, Value);

		if (Ret)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Wfsel = %d, Offset = %x, Value = %x fail\n", WfSel,
																Offset + Index * 4, Value));
			break;
		}
	}

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_ReadEEPROM(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 Offset=0, Value=0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	NdisMoveMemory(&Offset, HqaCmdFrame->Data, 2);
	Offset = OS_NTOHS(Offset);

	RT28xx_EEPROM_READ16(pAd, Offset, Value);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("e2p r %02Xh = 0x%02X\n"
		, (Offset & 0x00FF), (Value & 0x00FF)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("e2p r %02Xh = 0x%02X\n"
		, (Offset & 0x00FF)+1, (Value & 0xFF00)>>8));

	Value = OS_HTONS(Value);

	NdisMoveMemory(HqaCmdFrame->Data + 2, &Value, 2);

	ResponseToQA(HqaCmdFrame, WRQ, 4, Ret);

	return Ret;
}


static INT32 HQA_WriteEEPROM(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 Offset=0, Value=0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	NdisMoveMemory(&Offset, HqaCmdFrame->Data, 2);
	Offset = OS_NTOHS(Offset);

	NdisMoveMemory(&Value, HqaCmdFrame->Data + 2, 2);
	Value = OS_NTOHS(Value);
	RT28xx_EEPROM_WRITE16(pAd, Offset, Value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("e2p w 0x%04X = 0x%04X\n", Offset, Value));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("e2p w %02Xh = 0x%02X\n"
		, (Offset & 0x00FF), (Value & 0x00FF)));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("e2p w %02Xh = 0x%02X\n"
		, (Offset & 0x00FF) + 1, (Value & 0xFF00)>>8));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_ReadBulkEEPROM(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 Offset;
	UINT16 Len;
	UINT16 Buffer[EEPROM_SIZE >> 1];

	NdisMoveMemory(&Offset, HqaCmdFrame->Data, 2);
	Offset = OS_NTOHS(Offset);

	NdisMoveMemory(&Len, HqaCmdFrame->Data + 2, 2);
	Len = OS_NTOHS(Len);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Offset = %x, Length = %x\n", __FUNCTION__, Offset, Len));

	EEReadAll(pAd, (UINT16 *)Buffer);

	if (Offset + Len <= EEPROM_SIZE)
		NdisMoveMemory_exs(pAd, HqaCmdFrame->Data + 2, (UCHAR *)Buffer+Offset, Len);
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : exceed EEPROM size\n", __FUNCTION__));
		Len = 0;
		Ret = -1;
	}

	ResponseToQA(HqaCmdFrame, WRQ, 2 + Len, Ret);

	return Ret;
}


static VOID EEWriteBulk(PRTMP_ADAPTER pAd, UINT16 *Data, UINT16 Offset, UINT16 Length)
{
	UINT16 Pos;
	UINT16 Value;
	UINT16 Len = Length;

	for (Pos = 0; Pos < (Len >> 1);)
	{
		Value = Data[Pos];
		RT28xx_EEPROM_WRITE16(pAd, Offset + (Pos * 2), Value);
		Pos++;
	}

	return;
}


static INT32 HQA_WriteBulkEEPROM(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	USHORT Offset;
	USHORT Len;
	USHORT Buffer[EEPROM_SIZE >> 1];

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	NdisMoveMemory(&Offset, HqaCmdFrame->Data, 2);
	Offset = OS_NTOHS(Offset);

	NdisMoveMemory(&Len, HqaCmdFrame->Data + 2, 2);
	Len = OS_NTOHS(Len);

	NdisMoveMemory_exs(pAd, (UCHAR *)Buffer + Offset, (UCHAR *)HqaCmdFrame->Data + 4, Len);

#if defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT)
    if (Len == 16) {
        NdisMoveMemory(pAd->eebuf + Offset, (UCHAR *)Buffer + Offset, Len);
    }
    if (Offset == 0x1f0)
        rtmp_ee_flash_write_all(pAd, (PUSHORT)pAd->eebuf);

    if (Len != 16)
#endif
    {
        if ((Offset + Len) <= EEPROM_SIZE)
        {
            EEWriteBulk(pAd,(UINT16 *)(((UCHAR *)Buffer) + Offset), Offset, Len);
        }
        else
        {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : exceed EEPROM size(%d)\n", __FUNCTION__, EEPROM_SIZE));
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("Offset = %u\n", Offset));
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("Length = %u\n", Len));
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("Offset + Length=%u\n", (Offset + Len)));
        }
    }

	ResponseToQA(HqaCmdFrame, WRQ, 2 + Len, Ret);

	return Ret;
}


#ifdef RTMP_EFUSE_SUPPORT
static INT32 HQA_CheckEfuseMode(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Value;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	if (pAd->bUseEfuse)
		Value = 1;
	else
		Value = 0;


	Value = OS_HTONL(Value);
	NdisMoveMemory(HqaCmdFrame->Data + 2, &Value, 4);

	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	return Ret;
}


static INT32 HQA_GetFreeEfuseBlock(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Value;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	eFuseGetFreeBlockCount(pAd, &Value);

	Value = OS_HTONL(Value);
	NdisMoveMemory(HqaCmdFrame->Data + 2, &Value, 4);

	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	return Ret;
}


static INT32 HQA_GetEfuseBlockNr(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_WriteEFuseFromBuffer(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}
#endif /* RTMP_EFUSE_SUPPORT */


static HQA_CMD_HANDLER HQA_CMD_SET3[] =
{
	/* cmd id start from 0x1300 */
	HQA_MacBbpRegRead,		/* 0x1300 */
	HQA_MacBbpRegWrite,		/* 0x1301 */
	HQA_MACBbpRegBulkRead,	/* 0x1302 */
	HQA_RfRegBulkRead,		/* 0x1303 */
	HQA_RfRegBulkWrite,		/* 0x1304 */
	HQA_ReadEEPROM,			/* 0x1305 */
	HQA_WriteEEPROM,		/* 0x1306 */
	HQA_ReadBulkEEPROM,		/* 0x1307 */
	HQA_WriteBulkEEPROM,	/* 0x1308 */
#ifdef RTMP_EFUSE_SUPPORT
	HQA_CheckEfuseMode,		/* 0x1309 */
	HQA_GetFreeEfuseBlock,	/* 0x130A */
	HQA_GetEfuseBlockNr,	/* 0x130B */
	HQA_WriteEFuseFromBuffer, /* 0x130C */
#endif /* RTMP_EFUSE_SUPPORT */
};


static INT32 HQA_ReadTempReferenceValue(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

VOID HQA_GetThermalValue_CB(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EXT_EVENT_GET_SENSOR_RESULT_T *EventExtCmdResult = (struct _EXT_EVENT_GET_SENSOR_RESULT_T *)Data;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	EventExtCmdResult->u4SensorResult = le2cpu32(EventExtCmdResult->u4SensorResult);
    
	ATECtrl->thermal_val = EventExtCmdResult->u4SensorResult;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, value: 0x%x\n", __FUNCTION__, EventExtCmdResult->u4SensorResult));
	RTMP_OS_COMPLETE(&ATECtrl->cmd_done);
}

static INT32 HQA_GetThermalValue(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;	
	UINT32 temperature=0; 

#if defined(MT7603) || defined(MT7628)
	temperature = MtAsicGetThemalSensor(pAd, 0);
	temperature = OS_HTONL(temperature);
	NdisMoveMemory(HqaCmdFrame->Data + 2, &temperature, 4);
#else
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	CmdGetThermalSensorResult(pAd, 0,&temperature); /* 0: get temperature; 1: get adc */

	Ret = RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&ATECtrl->cmd_done, ATECtrl->cmd_expire);
	ATECtrl->thermal_val = OS_HTONL(ATECtrl->thermal_val);
	NdisMoveMemory(HqaCmdFrame->Data + 2, &ATECtrl->thermal_val, 4);
#endif /* MT7603 || MT7628 */
	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	return Ret;
}


static INT32 HQA_SetSideBandOption(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static HQA_CMD_HANDLER HQA_CMD_SET4[] =
{
	/* cmd id start from 0x1400 */
	HQA_ReadTempReferenceValue, /* 0x1400 */
	HQA_GetThermalValue,		/* 0x1401 */
	HQA_SetSideBandOption,		/* 0x1402 */
};


static INT32 HQA_GetFWInfo(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	Ret = ATEOp->GetFWInfo(pAd, HqaCmdFrame->Data + 2);

    ResponseToQA(HqaCmdFrame, WRQ, 38, Ret);
	return Ret;
}


static INT32 HQA_GetRxStatisticsAll(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 value = 0;
	UINT32 IBRssi0, IBRssi1, WBRssi0, WBRssi1;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	HQA_RX_STAT HqaRxStat;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
	memset(&HqaRxStat, 0, sizeof(HQA_RX_STAT));
        /* Get latest FCSErr. sync iwpriv & HQA same FCSErr result */
	NICUpdateRawCounters(pAd);
	HqaRxStat.MacFCSErr = OS_NTOHL(ATECtrl->RxMacFCSErrCount);

	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_MAC_MDRDYCNT);
	ATECtrl->RxMacMdrdyCount += value;
	HqaRxStat.MacMdrdy= OS_NTOHL(ATECtrl->RxMacMdrdyCount);

	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PHY_FCSERRCNT);
	HqaRxStat.FCSErr_OFDM = OS_NTOHL(value >> 16);
	HqaRxStat.FCSErr_CCK = OS_NTOHL(value & 0xFFFF);

	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PD);
	HqaRxStat.OFDM_PD = OS_NTOHL(value >> 16);
	HqaRxStat.CCK_PD = OS_NTOHL(value & 0xFFFF);

	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_CCK_SIG_SFD);
	HqaRxStat.CCK_SIG_Err = OS_NTOHL(value >> 16);
	HqaRxStat.CCK_SFD_Err = OS_NTOHL(value & 0xFFFF);

	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_OFDM_SIG_TAG);
	HqaRxStat.OFDM_SIG_Err = OS_NTOHL(value >> 16);
	HqaRxStat.OFDM_TAG_Err = OS_NTOHL(value & 0xFFFF);

	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_RSSI);
	IBRssi0 = (value&0xFF000000) >> 24;
	if (IBRssi0 > 128)
		IBRssi0 -=256;
	WBRssi0 = (value&0x00FF0000) >> 16;
	if (WBRssi0 > 128)
		WBRssi0 -=256;
	IBRssi1 = (value&0x0000FF00) >> 8;
	if (IBRssi1 > 128)
		IBRssi1 -=256;
	WBRssi1 = (value&0x000000FF);
	if (WBRssi1 > 128)
		WBRssi1 -=256;
	HqaRxStat.IB_RSSSI0 = OS_NTOHL(IBRssi0);
	HqaRxStat.WB_RSSSI0 = OS_NTOHL(WBRssi0);
	HqaRxStat.IB_RSSSI1 = OS_NTOHL(IBRssi1);
	HqaRxStat.WB_RSSSI1 = OS_NTOHL(WBRssi1);

	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PHY_MDRDYCNT);
	HqaRxStat.PhyMdrdyOFDM = OS_NTOHL(value >>16);
	HqaRxStat.PhyMdrdyCCK = OS_NTOHL(value & 0xFFFF);

	HqaRxStat.DriverRxCount = OS_NTOHL(ATECtrl->RxTotalCnt);
	HqaRxStat.RCPI0 = OS_NTOHL(ATECtrl->RCPI0);
	HqaRxStat.RCPI1 = OS_NTOHL(ATECtrl->RCPI1);
	HqaRxStat.FreqOffsetFromRX = OS_NTOHL(ATECtrl->FreqOffsetFromRx);
        HqaRxStat.RSSI0 = OS_NTOHL(ATECtrl->RSSI0);
	HqaRxStat.RSSI1 = OS_NTOHL(ATECtrl->RSSI1);

	NdisMoveMemory(HqaCmdFrame->Data + 2, &(HqaRxStat), sizeof(HQA_RX_STAT));

	ResponseToQA(HqaCmdFrame, WRQ, (2+sizeof(HQA_RX_STAT)), Ret);
	return Ret;
}


static INT32 HQA_StartContinousTx(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
    INT32 Value = 0 , WFSel = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
	if(ATECtrl->Mode & fATE_TXCONT_ENABLE)
		return Ret;

    /* Modulation:4 BW:4 PRI_CH:4 RATE:4 WFSel:4 */
    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
    Value = OS_NTOHL(Value);
    ATECtrl->PhyMode = (UCHAR)Value;

    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
    Value = OS_NTOHL(Value);
    ATECtrl->BW = (UCHAR)Value;

    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 8, 4);
    Value = OS_NTOHL(Value);
    //ATECtrl->ControlChl = (UCHAR)Value; //TODO QA Tool send this parameter with 0, which leads to error in channel switch, mask temperarily

    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 12, 4);
    Value = OS_NTOHL(Value);
    ATECtrl->Mcs = (UCHAR)Value;

    NdisMoveMemory((PUCHAR)&WFSel, (PUCHAR)&HqaCmdFrame->Data + 16, 4);
    WFSel = OS_NTOHL(WFSel);
	//ATECtrl->TxAntennaSel = WFSel;

    ATEOp->StartContinousTx(pAd, (CHAR)WFSel);
	ATECtrl->Mode |= ATE_TXCONT;

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetSTBC(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    INT32 Value = 0;
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
    Value = OS_NTOHL(Value);

    ATECtrl->Stbc = (UCHAR)Value;

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetShortGI(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    INT32 Value = 0;
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
    Value = OS_NTOHL(Value);

    ATECtrl->Sgi= (UCHAR)Value;

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetDPD(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    INT32 Value = 0, WFSel;
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    /* ON/OFF:4 WF Sel:4 */
	NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
	Value = OS_NTOHL(Value);

	NdisMoveMemory((PUCHAR)&WFSel, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
	WFSel = OS_NTOHL(WFSel);

    ATEOp->SetDPD(pAd, (CHAR)WFSel, (CHAR)Value);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_StartContiTxTone(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    INT32 Value = 0;
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
    Value = OS_NTOHL(Value);

	ATEOp->StartTxTone(pAd, Value); 
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_StopContiTxTone(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ATEOp->StopTxTone(pAd); 
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_CalibrationTestMode(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 Value = 0;
	INT32 Ret = 0;
	UINT8  Mode = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
    Value = OS_NTOHL(Value);
	if (Value == 0){
		if(ATECtrl->Mode & ATE_FFT)
			ATECtrl->Mode &= ~ATE_FFT;
        Mode = OPERATION_NORMAL_MODE;
	}else if (Value == 1){
        Mode = OPERATION_RFTEST_MODE;
	}else
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s Mode = %d error!!!\n",__FUNCTION__, Value));

    CmdRfTest(pAd, ACTION_SWITCH_TO_RFTEST, Mode, 0);
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_DoCalibrationTestItem(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    INT32 Value = 0;
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
    Value = OS_NTOHL(Value);

    CmdRfTest(pAd, ACTION_IN_RFTEST, 0, Value);

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_eFusePhysicalWrite(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
    /* ToDo */
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
    return Ret;
}

static INT32 HQA_eFusePhysicalRead(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
    /* ToDo */
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
    return Ret;
}

static INT32 HQA_eFuseLogicalRead(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
    /* ToDo */
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
    return Ret;
}

static INT32 HQA_eFuseLogicalWrite(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
    /* ToDo */
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
    return Ret;
}

static INT32 HQA_TMRSetting(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
    /* ToDo */
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
    return Ret;
}

static INT32 HQA_GetRxSNR(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0, Value = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    Value = OS_NTOHL(ATECtrl->SNR0);
    NdisMoveMemory(HqaCmdFrame->Data + 2, &Value, 4);

    Value = OS_NTOHL(ATECtrl->SNR1);
    NdisMoveMemory(HqaCmdFrame->Data + 6, &Value, 4);

	ResponseToQA(HqaCmdFrame, WRQ, 10, Ret);
    return Ret;
}


static INT32 HQA_WriteBufferDone(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 Value = 0;
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
    Value = OS_NTOHL(Value);

	switch (Value)
	{
		case E2P_EFUSE_MODE:
			Set_EepromBufferWriteBack_Proc(pAd, "1");
			break;
		case E2P_FLASH_MODE:
			Set_EepromBufferWriteBack_Proc(pAd, "2");
			break;
		case E2P_EEPROM_MODE:
			Set_EepromBufferWriteBack_Proc(pAd, "3");
			break;
		case E2P_BIN_MODE:
			Set_EepromBufferWriteBack_Proc(pAd, "4");
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Unknow write back mode(%d)\n", __FUNCTION__, Value));
	}

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
    return Ret;

}


static INT32 HQA_FFT(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 Value = 0;
	INT32 Ret = 0;

    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
    Value = OS_NTOHL(Value);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: %d\n", __FUNCTION__, Value));
	Ret = ATEOp->SetFFTMode(pAd, Value);

	return Ret;
}

static INT32 HQA_SetTxTonePower(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 Value = 0;
	INT32 Ret = 0;
	INT32 pwr1 = 0;
	INT32 pwr2 = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
    pwr1 = OS_NTOHL(Value);

    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
    pwr2 = OS_NTOHL(Value);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, pwr1:%d, pwr2:%d \n", __FUNCTION__,pwr1,pwr2));
	ATEOp->SetTxTonePower(pAd, pwr1, pwr2);

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MPSSetSeqData(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	INT32 i = 0;	
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;
	UINT32 band_idx = 0;

    len = OS_NTOHS(HqaCmdFrame->Length)/4 - 1;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s::len:%u\n", __FUNCTION__, len));
	if((len>1024) || (len == 0))
		goto MPS_SEQ_DATA_RET;

	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32)*(len));
	if(Ret == NDIS_STATUS_FAILURE)
		goto MPS_SEQ_DATA_RET;
	
	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = OS_NTOHL(band_idx);

	for(i=0;i<len;i++){
	    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4 + 4*i, 4);
    	mps_setting[i] = OS_NTOHL(Value);
	}
	
	ATEOp->MPSSetParm(pAd, MPS_SEQDATA, band_idx, len, mps_setting);
	os_free_mem(pAd, mps_setting);

MPS_SEQ_DATA_RET:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, len:%u, MPS_CNT:%u\n", __FUNCTION__,len, ATECtrl->mps_cb.mps_cnt));
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MPSSetPayloadLength(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	INT32 i = 0;	
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;
	UINT32 band_idx = 0;

    len = OS_NTOHS(HqaCmdFrame->Length)/4 - 1;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s::len:%u\n", __FUNCTION__, len));
	if((len>1024) || (len == 0))
		goto MPS_PKT_LEN_RET;
	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32)*(len));
	if(Ret == NDIS_STATUS_FAILURE)
		goto MPS_PKT_LEN_RET;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = OS_NTOHL(band_idx);

	for(i=0;i<len;i++){
	    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4 + 4*i, 4);
    	mps_setting[i] = OS_NTOHL(Value);
	}
	
	ATEOp->MPSSetParm(pAd, MPS_PAYLOAD_LEN, band_idx, len, mps_setting);
	os_free_mem(pAd, mps_setting);

MPS_PKT_LEN_RET:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, len:%u, MPS_CNT:%u\n", __FUNCTION__,len, ATECtrl->mps_cb.mps_cnt));
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MPSSetPacketCount(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	INT32 i = 0;	
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;
	UINT32 band_idx = 0;

    len = OS_NTOHS(HqaCmdFrame->Length)/4 - 1;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s::len:%u\n", __FUNCTION__, len));
	if((len>1024) || (len == 0))
		goto MPS_PKT_CNT_RET;
	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32)*(len));
	if(Ret == NDIS_STATUS_FAILURE)
		goto MPS_PKT_CNT_RET;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = OS_NTOHL(band_idx);
	for(i=0;i<len;i++){
	    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4 + 4*i, 4);
    	mps_setting[i] = OS_NTOHL(Value);
	}
	
	ATEOp->MPSSetParm(pAd, MPS_TX_COUNT, band_idx, len, mps_setting);
	os_free_mem(pAd, mps_setting);

MPS_PKT_CNT_RET:
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, len:%u, MPS_CNT:%u\n", __FUNCTION__,len, ATECtrl->mps_cb.mps_cnt));
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MPSSetPowerGain(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	INT32 i = 0;	
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;	
	UINT32 band_idx = 0;

	len = OS_NTOHS(HqaCmdFrame->Length)/4 - 1;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s::len:%u\n", __FUNCTION__, len));
	if((len>1024) || (len == 0))
		goto MPS_SET_PWR_RET;
	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32)*(len));
	if(Ret == NDIS_STATUS_FAILURE)
		goto MPS_SET_PWR_RET;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = OS_NTOHL(band_idx);
	
	for(i=0;i<len;i++){
	    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4 + 4*i, 4);
    	mps_setting[i] = OS_NTOHL(Value);
	}
	
	ATEOp->MPSSetParm(pAd, MPS_PWR_GAIN, band_idx, len, mps_setting);
	os_free_mem(pAd, mps_setting);

MPS_SET_PWR_RET:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, len:%u, MPS_CNT:%u\n", __FUNCTION__,len, ATECtrl->mps_cb.mps_cnt));
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MPSStart(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;	
	INT32 Ret = 0;
	UINT32 band_idx = 0;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = OS_NTOHL(band_idx);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s::\n", __FUNCTION__));

	Ret = ATEOp->MPSTxStart(pAd, 0);
	if(WRQ)
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MPSStop(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 band_idx = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s\n", __FUNCTION__));
	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = OS_NTOHL(band_idx);
	Ret = ATEOp->MPSTxStop(pAd, band_idx);
	if(WRQ)
	    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_CheckEfuseModeType(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 val = 0;
	INT32 Ret = 0;
	//ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	NdisMoveMemory(HqaCmdFrame->Data + 2, &(val), sizeof(val));
	ResponseToQA(HqaCmdFrame, WRQ, 2+sizeof(val), Ret);
	return Ret;
}

static INT32 HQA_CheckEfuseNativeModeType(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 val = 0;
	INT32 Ret = 0;
	//ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, %x\n", __FUNCTION__, pAd->E2pAccessMode));
	val = OS_HTONL(pAd->E2pAccessMode);	//Fix me::after Star update code, get backup one
	NdisMoveMemory(HqaCmdFrame->Data + 2, &(val), sizeof(val));
	ResponseToQA(HqaCmdFrame, WRQ, 2+sizeof(val), Ret);
	return Ret;
}

static HQA_CMD_HANDLER HQA_CMD_SET5[] =
{
	/* cmd id start from 0x1500 */
	HQA_GetFWInfo,				/* 0x1500 */
	HQA_StartContinousTx,		/* 0x1501 */
	HQA_SetSTBC,				/* 0x1502 */
	HQA_SetShortGI,				/* 0x1503 */
	HQA_SetDPD,					/* 0x1504 */
	HQA_SetTssiOnOff,			/* 0x1505 */
	HQA_GetRxStatisticsAll,		/* 0x1506 */
	HQA_StartContiTxTone,		/* 0x1507 */
	HQA_StopContiTxTone,        /* 0x1508 */
	HQA_CalibrationTestMode,    /* 0x1509 */
	HQA_DoCalibrationTestItem,  /* 0x150A */
	HQA_eFusePhysicalWrite,     /* 0x150B */
	HQA_eFusePhysicalRead,      /* 0x150C */
	HQA_eFuseLogicalRead,       /* 0x150D */
	HQA_eFuseLogicalWrite,      /* 0x150E */
	HQA_TMRSetting,             /* 0x150F */
	HQA_GetRxSNR,               /* 0x1510 */
	HQA_WriteBufferDone,		/* 0x1511 */
	HQA_FFT,					/* 0x1512 */
	HQA_SetTxTonePower,			/* 0x1513 */
	NULL,
	HQA_MPSSetSeqData,			/* 0x1515 */
	HQA_MPSSetPayloadLength,	/* 0x1516 */
	HQA_MPSSetPacketCount,		/* 0x1517 */
	HQA_MPSSetPowerGain,		/* 0x1518 */
	HQA_MPSStart,				/* 0x1519 */
	HQA_MPSStop,				/* 0x151A */
	NULL,						/* 0x151B */
	NULL,						/* 0x151C */
	NULL,						/* 0x151D */
	NULL,						/* 0x151E */
	NULL,						/* 0x151F */
	NULL,						/* 0x1520 */
	NULL,						/* 0x1521 */
	HQA_CheckEfuseModeType,		/* 0x1522 */
	HQA_CheckEfuseNativeModeType,/* 0x1523 */
};

static INT32 hqa_set_channel_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UINT32 val = 0;
	//ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s\n", __FUNCTION__));
    NdisMoveMemory((PUCHAR)&val, (PUCHAR)&cmd_frame->Data, 4);
   	val = OS_NTOHL(val);

	NdisMoveMemory(cmd_frame->Data + 2, cmd_frame->Data, 4);	//Resp require ext_cmd id
	if(wrq)
	    ResponseToQA(cmd_frame, wrq, 6, ret);
	return ret;
}

static INT32 hqa_set_txcontent_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	//ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s\n", __FUNCTION__));

	NdisMoveMemory(cmd_frame->Data + 2, cmd_frame->Data, 4);	//Resp require ext_cmd id
	if(wrq)
	    ResponseToQA(cmd_frame, wrq, 6, ret);
	return ret;
}

static INT32 hqa_start_tx_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	//ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s\n", __FUNCTION__));

	NdisMoveMemory(cmd_frame->Data + 2, cmd_frame->Data, 4);	//Resp require ext_cmd id
	if(wrq)
	    ResponseToQA(cmd_frame, wrq, 6, ret);
	return ret;
}

static INT32 hqa_start_rx_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
	UINT32 param_num = 0;
	UINT32 band_idx = 0;
	UCHAR own_mac[MAC_ADDR_LEN];
	NdisZeroMemory(own_mac, MAC_ADDR_LEN);
	NdisMoveMemory((PUCHAR)&param_num, (PUCHAR)cmd_frame->Data + 4, sizeof(param_num));
	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)cmd_frame->Data + 8, sizeof(band_idx));
	NdisMoveMemory((PUCHAR)own_mac, (PUCHAR)cmd_frame->Data + 12, MAC_ADDR_LEN);
	param_num = OS_NTOHL(param_num);
	band_idx = OS_NTOHL(band_idx);

	ret = ate_ops->SetAutoResp(pAd, band_idx, own_mac, 1);
	ret = ate_ops->StartRx(pAd);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, param num:%u, band_sel:%u, mac:%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__,param_num, band_idx, own_mac[0], own_mac[1], own_mac[2], own_mac[3], own_mac[4], own_mac[5]));

	NdisMoveMemory(cmd_frame->Data + 2, cmd_frame->Data, 4);	//Resp require ext_cmd id
	if(wrq)
	    ResponseToQA(cmd_frame, wrq, 6, ret);
	return ret;
}

static INT32 hqa_stop_rx_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	//ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s\n", __FUNCTION__));

	NdisMoveMemory(cmd_frame->Data + 2, cmd_frame->Data, 4);	//Resp require ext_cmd id
	if(wrq)
	    ResponseToQA(cmd_frame, wrq, 6, ret);
	return ret;
}

static HQA_CMD_HANDLER hqa_ext_cmd_set[] =
{
	NULL,
	hqa_set_channel_ext,	/* 0x00000001 */
	hqa_set_txcontent_ext,	/* 0x00000002 */
	hqa_start_tx_ext,	/* 0x00000003 */
	hqa_start_rx_ext,	/* 0x00000004 */
	hqa_stop_rx_ext,	/* 0x00000005 */
};

static INT32 hqa_ext_cmds(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	INT32 idx = 0;
	//ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);

	NdisMoveMemory((PUCHAR)&idx, (PUCHAR)&cmd_frame->Data, 4);
	idx = OS_NTOHL(idx);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s %d\n", __FUNCTION__, idx));
	
	if (hqa_ext_cmd_set[idx] != NULL)
		ret = (*hqa_ext_cmd_set[idx])(pAd, wrq, cmd_frame);
	else
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s cmd idx %d is not supported\n", __FUNCTION__, idx));

	return ret;
}

static HQA_CMD_HANDLER HQA_CMD_SET6[] =
{
	/* cmd id start from 0x1600 */
	hqa_ext_cmds,	/* 0x1600 */
};


static HQA_CMD_TABLE HQA_CMD_TABLES[] = {
	{
		HQA_CMD_SET0,
		sizeof(HQA_CMD_SET0) / sizeof(HQA_CMD_HANDLER),
		0x1000,
	},
	{
		HQA_CMD_SET1,
		sizeof(HQA_CMD_SET1) / sizeof(HQA_CMD_HANDLER),
		0x1100,
	},
	{
		HQA_CMD_SET2,
		sizeof(HQA_CMD_SET2) / sizeof(HQA_CMD_HANDLER),
		0x1200,
	},
	{
		HQA_CMD_SET3,
		sizeof(HQA_CMD_SET3) / sizeof(HQA_CMD_HANDLER),
		0x1300,
	},
	{
		HQA_CMD_SET4,
		sizeof(HQA_CMD_SET4) / sizeof(HQA_CMD_HANDLER),
		0x1400,
	},
	{
		HQA_CMD_SET5,
		sizeof(HQA_CMD_SET5) / sizeof(HQA_CMD_HANDLER),
		0x1500,
	},
	{
		HQA_CMD_SET6,
		sizeof(HQA_CMD_SET6) / sizeof(HQA_CMD_HANDLER),
		0x1600,
	},

};


static INT32 HQA_CMDHandler(
	PRTMP_ADAPTER pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Status = NDIS_STATUS_SUCCESS;
	USHORT CmdId;
	UINT32 TableIndex = 0;

	CmdId = OS_NTOHS(HqaCmdFrame->Id);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,("\n%s: Command_Id = 0x%04x !\n", __FUNCTION__, CmdId));

	while (TableIndex < (sizeof(HQA_CMD_TABLES) / sizeof(HQA_CMD_TABLE)))
	{
 		int CmdIndex = 0;
 		CmdIndex = CmdId - HQA_CMD_TABLES[TableIndex].CmdOffset;
 		if ((CmdIndex >= 0) && (CmdIndex < HQA_CMD_TABLES[TableIndex].CmdSetSize))
 		{
			HQA_CMD_HANDLER *pCmdSet;

			pCmdSet = HQA_CMD_TABLES[TableIndex].CmdSet;

			if (pCmdSet[CmdIndex] != NULL)
				Status = (*pCmdSet[CmdIndex])(pAd, WRQ, HqaCmdFrame);
			break;
		}
		TableIndex++;
	}

	return Status;
}


INT32 RtmpDoAte(
	PRTMP_ADAPTER	pAd,
	RTMP_IOCTL_INPUT_STRUCT *WRQ,
	RTMP_STRING *wrq_name)
{
	INT32 Status = NDIS_STATUS_SUCCESS;
	HQA_CMD_FRAME *HqaCmdFrame;
	UINT32 ATEMagicNum;

	os_alloc_mem(pAd, (UCHAR **)&HqaCmdFrame, sizeof(*HqaCmdFrame));

	if (!HqaCmdFrame)
	{
		Status = -ENOMEM;
		goto ERROR0;
	}

	NdisZeroMemory(HqaCmdFrame, sizeof(*HqaCmdFrame));

	Status = copy_from_user((PUCHAR)HqaCmdFrame, WRQ->u.data.pointer,
													WRQ->u.data.length);

	if (Status)
	{
		Status = -EFAULT;
		goto ERROR1;
	}

	ATEMagicNum = OS_NTOHL(HqaCmdFrame->MagicNo);

	switch(ATEMagicNum)
	{
		case HQA_CMD_MAGIC_NO:
			Status = HQA_CMDHandler(pAd, WRQ, HqaCmdFrame);
			break;
		default:
			Status = NDIS_STATUS_FAILURE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Unknown magic number of HQA command = %x\n", ATEMagicNum));
			break;
	}

 ERROR1:
	os_free_mem(NULL, HqaCmdFrame);
 ERROR0:
	return Status;
}
