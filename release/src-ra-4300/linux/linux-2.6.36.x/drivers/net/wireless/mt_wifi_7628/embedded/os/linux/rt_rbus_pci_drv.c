/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************/



#ifdef RTMP_MAC_PCI
#include	"rt_config.h"

#ifdef BB_SOC
__IMEM void rx_done_tasklet(unsigned long data);
#else
static void rx_done_tasklet(unsigned long data);
#endif

#ifdef CONFIG_ANDES_SUPPORT
static void rx1_done_tasklet(unsigned long data);
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
static void bcn_dma_done_tasklet(unsigned long data);
static void bmc_dma_done_tasklet(unsigned long data);
#endif /* MT_MAC */

static void mgmt_dma_done_tasklet(unsigned long data);
static void ac0_dma_done_tasklet(unsigned long data);
#ifdef CONFIG_ATE
static void ate_ac0_dma_done_tasklet(unsigned long data);
#endif /* CONFIG_ATE */
static void ac1_dma_done_tasklet(unsigned long data);
static void ac2_dma_done_tasklet(unsigned long data);
static void ac3_dma_done_tasklet(unsigned long data);
static void hcca_dma_done_tasklet(unsigned long data);
static void fifo_statistic_full_tasklet(unsigned long data);

#ifdef UAPSD_SUPPORT
static void uapsd_eosp_sent_tasklet(unsigned long data);
#endif /* UAPSD_SUPPORT */


/*---------------------------------------------------------------------*/
/* Symbol & Macro Definitions                                          */
/*---------------------------------------------------------------------*/
NDIS_STATUS RtmpNetTaskInit(RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;


	RTMP_OS_TASKLET_INIT(pAd, &pObj->rx_done_task, rx_done_tasklet, (unsigned long)pAd);

#ifdef CONFIG_ANDES_SUPPORT
	RTMP_OS_TASKLET_INIT(pAd, &pObj->rx1_done_task, rx1_done_tasklet, (unsigned long)pAd);
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
	RTMP_OS_TASKLET_INIT(pAd, &pObj->bcn_dma_done_task, bcn_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->mt_mac_int_0_task, mt_mac_int_0_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->mt_mac_int_1_task, mt_mac_int_1_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->mt_mac_int_2_task, mt_mac_int_2_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->mt_mac_int_3_task, mt_mac_int_3_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->mt_mac_int_4_task, mt_mac_int_4_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->bmc_dma_done_task, bmc_dma_done_tasklet, (unsigned long)pAd);
#endif /* MT_MAC */

	RTMP_OS_TASKLET_INIT(pAd, &pObj->mgmt_dma_done_task, mgmt_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac0_dma_done_task, ac0_dma_done_tasklet, (unsigned long)pAd);

#ifdef CONFIG_ATE
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ate_ac0_dma_done_task, ate_ac0_dma_done_tasklet, (unsigned long)pAd);
#endif /* CONFIG_ATE */

	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac1_dma_done_task, ac1_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac2_dma_done_task, ac2_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->ac3_dma_done_task, ac3_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->hcca_dma_done_task, hcca_dma_done_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->tbtt_task, tbtt_tasklet, (unsigned long)pAd);
	RTMP_OS_TASKLET_INIT(pAd, &pObj->fifo_statistic_full_task, fifo_statistic_full_tasklet, (unsigned long)pAd);
#ifdef UAPSD_SUPPORT
		RTMP_OS_TASKLET_INIT(pAd, &pObj->uapsd_eosp_sent_task, uapsd_eosp_sent_tasklet, (unsigned long)pAd);
#endif /* UAPSD_SUPPORT */
#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef DFS_SUPPORT
		RTMP_OS_TASKLET_INIT(pAd, &pObj->dfs_task, dfs_tasklet, (unsigned long)pAd);
#endif /* DFS_SUPPORT */
	}
#endif /* CONFIG_AP_SUPPORT */

	return NDIS_STATUS_SUCCESS;
}


void RtmpNetTaskExit(RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_OS_TASKLET_KILL(&pObj->rx_done_task);
#ifdef CONFIG_ANDES_SUPPORT
	RTMP_OS_TASKLET_KILL(&pObj->rx1_done_task);
#endif /* CONFIG_ANDES_SUPPORT */

#ifdef MT_MAC
	RTMP_OS_TASKLET_KILL(&pObj->bcn_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->mt_mac_int_0_task);
	RTMP_OS_TASKLET_KILL(&pObj->mt_mac_int_1_task);
	RTMP_OS_TASKLET_KILL(&pObj->mt_mac_int_2_task);
	RTMP_OS_TASKLET_KILL(&pObj->mt_mac_int_3_task);
	RTMP_OS_TASKLET_KILL(&pObj->mt_mac_int_4_task);
	RTMP_OS_TASKLET_KILL(&pObj->bmc_dma_done_task);
#endif /* MT_MAC */

	RTMP_OS_TASKLET_KILL(&pObj->mgmt_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->ac0_dma_done_task);
#ifdef CONFIG_ATE
	RTMP_OS_TASKLET_KILL(&pObj->ate_ac0_dma_done_task);
#endif /* CONFIG_ATE */
	RTMP_OS_TASKLET_KILL(&pObj->ac1_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->ac2_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->ac3_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->hcca_dma_done_task);
	RTMP_OS_TASKLET_KILL(&pObj->tbtt_task);
	RTMP_OS_TASKLET_KILL(&pObj->fifo_statistic_full_task);
#ifdef UAPSD_SUPPORT
	RTMP_OS_TASKLET_KILL(&pObj->uapsd_eosp_sent_task);
#endif /* UAPSD_SUPPORT */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef DFS_SUPPORT
		RTMP_OS_TASKLET_KILL(&pObj->dfs_task);
#endif /* DFS_SUPPORT */

	}
#endif /* CONFIG_AP_SUPPORT */
}


NDIS_STATUS RtmpMgmtTaskInit(RTMP_ADAPTER *pAd)
{
	RTMP_OS_TASK *pTask;
	NDIS_STATUS status;

	/* Creat Command Thread */
	pTask = &pAd->cmdQTask;
	RTMP_OS_TASK_INIT(pTask, "RtmpCmdQTask", pAd);
	status = RtmpOSTaskAttach(pTask, RTPCICmdThread, (ULONG)pTask);
	if (status == NDIS_STATUS_FAILURE)
	{
		printk ("Unable to start RTPCICmdThread!\n");
		return NDIS_STATUS_FAILURE;
	}

#ifdef WSC_INCLUDED
	/* start the crediential write task first. */
	WscThreadInit(pAd);
#endif /* WSC_INCLUDED */

	return NDIS_STATUS_SUCCESS;
}


/*
========================================================================
Routine Description:
    Close kernel threads.

Arguments:
	*pAd				the raxx interface data pointer

Return Value:
    NONE

Note:
========================================================================
*/
VOID RtmpMgmtTaskExit(RTMP_ADAPTER *pAd)
{
	INT ret;

	/* Terminate cmdQ thread */
	RTMP_OS_TASK_LEGALITY(&pAd->cmdQTask)
	{
		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_STOPED;
		NdisReleaseSpinLock(&pAd->CmdQLock);

		/*RTUSBCMDUp(&pAd->cmdQTask); */
		ret = RtmpOSTaskKill(&pAd->cmdQTask);
		if (ret == NDIS_STATUS_FAILURE)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Kill command task fail!\n"));
		}
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_UNKNOWN;
	}

#ifdef WSC_INCLUDED
	WscThreadExit(pAd);
#endif /* WSC_INCLUDED */

	return;
}


static inline void rt2860_int_enable(RTMP_ADAPTER *pAd, unsigned int mode)
{
	UINT32 regValue;

	// TODO: shiang-7603

	pAd->int_disable_mask &= ~(mode);
	regValue = pAd->int_enable_reg & ~(pAd->int_disable_mask);

#if defined(RTMP_MAC) || defined(RLT_MAC)
	RTMP_IO_WRITE32(pAd, INT_MASK_CSR, regValue);
#ifdef RTMP_MAC_PCI
	RTMP_IO_READ32(pAd, INT_MASK_CSR, &regValue); /* Push write command to take effect quickly (i.e. flush the write data) */
#endif /* RTMP_MAC_PCI */
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
	RTMP_IO_WRITE32(pAd, MT_INT_MASK_CSR, regValue);
#ifdef RTMP_MAC_PCI
	RTMP_IO_READ32(pAd, MT_INT_MASK_CSR, &regValue); /* Push write command to take effect quickly (i.e. flush the write data) */
#endif /* RTMP_MAC_PCI */
#endif /* MT_MAC */

	if (regValue != 0) {
		RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
	}
}


static inline void rt2860_int_disable(RTMP_ADAPTER *pAd, unsigned int mode)
{
	UINT32 regValue;
	// TODO: shiang-7603



	pAd->int_disable_mask |= mode;
	regValue = pAd->int_enable_reg & ~(pAd->int_disable_mask);

#if defined(RTMP_MAC) || defined(RLT_MAC)
	RTMP_IO_WRITE32(pAd, INT_MASK_CSR, regValue);
#ifdef RTMP_MAC_PCI
	RTMP_IO_READ32(pAd, INT_MASK_CSR, &regValue); /* Push write command to take effect quickly (i.e. flush the write data) */
#endif /* RTMP_MAC_PCI */
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
	RTMP_IO_WRITE32(pAd, MT_INT_MASK_CSR, regValue);
#ifdef RTMP_MAC_PCI
	RTMP_IO_READ32(pAd, MT_INT_MASK_CSR, &regValue); /* Push write command to take effect quickly (i.e. flush the write data) */
#endif /* RTMP_MAC_PCI */
#endif /* MT_MAC */
	if (regValue == 0) {
		RTMP_CLEAR_FLAG(pAd, fRTMP_ADAPTER_INTERRUPT_ACTIVE);
	}
}


/***************************************************************************
  *
  *	tasklet related procedures.
  *
  **************************************************************************/
static void mgmt_dma_done_tasklet(unsigned long data)
{
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, mgmt_dma_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	unsigned long flags;
	UINT32 INT_MGMT_DLY = 0;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		INT_MGMT_DLY = RLT_INT_MGMT_DLY;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		INT_MGMT_DLY = RTMP_INT_MGMT_DLY;
#endif /* RTMP_MAC */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		INT_MGMT_DLY = MT_INT_MGMT_DLY;
#endif /* MT_MAC */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~INT_MGMT_DLY;
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~INT_MGMT_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	RTMPHandleMgmtRingDmaDoneInterrupt(pAd);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	if (pAd->int_pending & INT_MGMT_DLY)
	{
		/* double check to avoid lose of interrupts */
		pObj = (POS_COOKIE) pAd->OS_Cookie;
		RTMP_OS_TASKLET_SCHE(&pObj->mgmt_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	rt2860_int_enable(pAd, INT_MGMT_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


#ifdef BB_SOC
__IMEM void rx_done_tasklet(unsigned long data)
#else
static void rx_done_tasklet(unsigned long data)
#endif
{
	unsigned long flags;
	BOOLEAN	bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, rx_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	UINT32 INT_RX = 0;

MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("-->%s():\n", __FUNCTION__));

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		INT_RX = RLT_INT_RX_DATA;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		INT_RX = RTMP_INT_RX;
#endif /* RTMP_MAC */

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		INT_RX = MT_INT_RX_DATA;
#endif /* MT_MAC */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
	if (RTMP_TEST_FLAG(pAd,  fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(INT_RX);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}
#ifdef UAPSD_SUPPORT
	UAPSD_TIMING_RECORD(pAd, UAPSD_TIMING_RECORD_TASKLET);
#endif /* UAPSD_SUPPORT */

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~(INT_RX);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	bReschedule = rtmp_rx_done_handle(pAd);

#ifdef CONFIG_BA_REORDER_MONITOR
	if (pAd->BATable.ba_timeout_check) {
		ba_timeout_flush(pAd);	
	/*
		if (!((pAd->int_pending & INT_RX) || 
				(pAd->int_disable_mask & INT_RX)))
			return;
	*/
	}
#endif


#ifdef UAPSD_SUPPORT
	UAPSD_TIMING_RECORD_STOP();
#endif /* UAPSD_SUPPORT */

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid rotting packet  */
	if (pAd->int_pending & INT_RX || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->rx_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	rt2860_int_enable(pAd, INT_RX);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

}


#ifdef CONFIG_ANDES_SUPPORT
static void rx1_done_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN	bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, rx1_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	UINT32 int_mask = 0;

#ifdef MT_MAC
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT)
		int_mask = MT_INT_RX_CMD;
#endif /* MT_MAC */

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		int_mask = RLT_INT_RX_CMD;
#endif /* RLT_MAC */


#if defined(RLT_MAC) || defined(MT_MAC)
	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	//if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
    if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(int_mask);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}
	pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~(int_mask);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	bReschedule = RxRing1DoneInterruptHandle(pAd);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid rotting packet  */
	if ((pAd->int_pending & int_mask) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->rx1_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	/* enable Rx1INT again */
	rt2860_int_enable(pAd, int_mask);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
#endif /* defined(RLT_MAC) || defined(MT_MAC) */

}
#endif /* CONFIG_ANDES_SUPPORT */


void fifo_statistic_full_tasklet(unsigned long data)
{
	unsigned long flags;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, fifo_statistic_full_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	UINT32 FifoStaFullInt = 0;

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		FifoStaFullInt = RLT_FifoStaFullInt;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		FifoStaFullInt = RTMP_FifoStaFullInt;
#endif /* RTMP_MAC */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	 {
		  RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		  pAd->int_disable_mask &= ~(FifoStaFullInt);
		  RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
 	  }

    pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~(FifoStaFullInt);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	NICUpdateFifoStaCounters(pAd);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/*
	 * double check to avoid rotting packet
	 */
	if (pAd->int_pending & FifoStaFullInt)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->fifo_statistic_full_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	rt2860_int_enable(pAd, FifoStaFullInt);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

}


#ifdef MT_MAC
static void bcn_dma_done_tasklet(unsigned long data)
{
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, bcn_dma_done_tasklet);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	unsigned long flags;
	BOOLEAN bReschedule = 0;
	UINT32 INT_BCN_DLY = MT_INT_BCN_DLY;



	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(INT_BCN_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~INT_BCN_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	RTMPHandleBcnDmaDoneInterrupt(pAd);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->int_pending & INT_BCN_DLY) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->bcn_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	rt2860_int_enable(pAd, INT_BCN_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

#ifdef CONFIG_AP_SUPPORT
MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("<--%s():bcn_state=%d\n", __FUNCTION__, pAd->ApCfg.MBSSID[0].bcn_buf.bcn_state));
#endif /* CONFIG_AP_SUPPORT */
}
#endif /* MT_MAC */


static void hcca_dma_done_tasklet(unsigned long data)
{
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, hcca_dma_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	unsigned long flags;
	BOOLEAN bReschedule = 0;
	UINT32 INT_HCCA_DLY = 0;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		INT_HCCA_DLY = MT_INT_CMD;
#endif

#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		INT_HCCA_DLY = RLT_INT_HCCA_DLY;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		INT_HCCA_DLY = RTMP_INT_HCCA_DLY;
#endif /* RTMP_MAC */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~INT_HCCA_DLY;
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~INT_HCCA_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

#if defined(RLT_MAC) || defined(MT_MAC)
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_RLT || pAd->chipCap.hif_type == HIF_MT)
		RTMPHandleTxRing8DmaDoneInterrupt(pAd);
#endif /* RLT_MAC */

#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_HCCA_DONE);
#endif /* RTMP_MAC */

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->int_pending & INT_HCCA_DLY) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->hcca_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	rt2860_int_enable(pAd, INT_HCCA_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


static void ac3_dma_done_tasklet(unsigned long data)
{
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, ac3_dma_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	unsigned long flags;
	BOOLEAN bReschedule = 0;
	UINT32 INT_AC3_DLY = 0;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		INT_AC3_DLY = MT_INT_AC3_DLY;
#endif /* MT_MAC */
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		INT_AC3_DLY = RLT_INT_AC3_DLY;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		INT_AC3_DLY = RTMP_INT_AC3_DLY;
#endif /* RTMP_MAC */


	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(INT_AC3_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~INT_AC3_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

#ifdef MT7615
    bReschedule = RTMPHandleFwDwloCmdRingDmaDoneInterrupt(pAd);
#else
	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC3_DONE);
#endif

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->int_pending & INT_AC3_DLY) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->ac3_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	rt2860_int_enable(pAd, INT_AC3_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


static void ac2_dma_done_tasklet(unsigned long data)
{
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, ac2_dma_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	unsigned long flags;
	BOOLEAN bReschedule = 0;
	UINT32 INT_AC2_DLY = 0;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		INT_AC2_DLY = MT_INT_AC2_DLY;
#endif /* MT_MAC */
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		INT_AC2_DLY = RLT_INT_AC2_DLY;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		INT_AC2_DLY = RTMP_INT_AC2_DLY;
#endif /* RTMP_MAC */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(INT_AC2_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~INT_AC2_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC2_DONE);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->int_pending & INT_AC2_DLY) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->ac2_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	rt2860_int_enable(pAd, INT_AC2_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


static void ac1_dma_done_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, ac1_dma_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	UINT32 INT_AC1_DLY = 0;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		INT_AC1_DLY = MT_INT_AC1_DLY;
#endif /* MT_MAC */
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		INT_AC1_DLY = RLT_INT_AC1_DLY;
#endif /* RLT_MAC */
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		INT_AC1_DLY = RTMP_INT_AC1_DLY;
#endif /* RTMP_MAC */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(INT_AC1_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~INT_AC1_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC1_DONE);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->int_pending & INT_AC1_DLY) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->ac1_dma_done_task);

		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	rt2860_int_enable(pAd, INT_AC1_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


static void ac0_dma_done_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, ac0_dma_done_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
	UINT32 INT_AC0_DLY = 0;

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT)
		INT_AC0_DLY = MT_INT_AC0_DLY;
#endif /* MT_MAC */
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT)
		INT_AC0_DLY = RLT_INT_AC0_DLY;
#endif /* RLT_MAC*/
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP)
		INT_AC0_DLY = RTMP_INT_AC0_DLY;
#endif /* RTMP_MAC */

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(INT_AC0_DLY);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~INT_AC0_DLY;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC0_DONE);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->int_pending & INT_AC0_DLY) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->ac0_dma_done_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("<--%s():pAd->int_pending=0x%x, bReschedule=%d\n",
			__FUNCTION__, pAd->int_pending, bReschedule));
		return;
	}

	rt2860_int_enable(pAd, INT_AC0_DLY);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


#ifdef MT_MAC
void mt_mac_int_0_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, mt_mac_int_0_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */

MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("-->%s()\n", __FUNCTION__));
	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(WF_MAC_INT_0);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~WF_MAC_INT_0;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	//bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC0_DONE);
#ifdef RT_CFG80211_P2P_SUPPORT
	RTMPHandlePreTBTTInterrupt(pAd);
#endif	
	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->int_pending & WF_MAC_INT_0) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->mt_mac_int_0_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("<--%s():pAd->int_pending=0x%x, bReschedule=%d\n",
			__FUNCTION__, pAd->int_pending, bReschedule));
		return;
	}

	rt2860_int_enable(pAd, WF_MAC_INT_0);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


void mt_mac_int_1_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, mt_mac_int_1_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */

MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("-->%s()\n", __FUNCTION__));
	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(WF_MAC_INT_1);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~WF_MAC_INT_1;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	//bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC0_DONE);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->int_pending & WF_MAC_INT_1) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->mt_mac_int_1_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("<--%s():pAd->int_pending=0x%x, bReschedule=%d\n",
			__FUNCTION__, pAd->int_pending, bReschedule));
		return;
	}

	rt2860_int_enable(pAd, WF_MAC_INT_1);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


void mt_mac_int_2_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, mt_mac_int_2_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */

MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("-->%s()\n", __FUNCTION__));
	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(WF_MAC_INT_2);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~WF_MAC_INT_2;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	//bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC0_DONE);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->int_pending & WF_MAC_INT_2) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->mt_mac_int_2_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<--%s():pAd->int_pending=0x%x, bReschedule=%d\n",
			__FUNCTION__, pAd->int_pending, bReschedule));
		return;
	}

	rt2860_int_enable(pAd, WF_MAC_INT_2);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


void mt_mac_int_3_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, mt_mac_int_3_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
//	UINT32 stat_reg, en_reg;

	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(WF_MAC_INT_3);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("<--%s():HALT in progress(Flags=0x%lu)!\n", __FUNCTION__, pAd->Flags));
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~WF_MAC_INT_3;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

		RTMPHandlePreTBTTInterrupt(pAd);


	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->int_pending & WF_MAC_INT_3) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->mt_mac_int_3_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

//	rt2860_int_enable(pAd, WF_MAC_INT_3);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}


void mt_mac_int_4_tasklet(unsigned long data)
{
	unsigned long flags;
	BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, mt_mac_int_4_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
	POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */

MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("-->%s()\n", __FUNCTION__));
	/* Do nothing if the driver is starting halt state. */
	/* This might happen when timer already been fired before cancel timer with mlmehalt */
	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
	{
		RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
		pAd->int_disable_mask &= ~(WF_MAC_INT_4);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		return;
	}

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	pAd->int_pending &= ~WF_MAC_INT_4;
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

	//bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_AC0_DONE);

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	/* double check to avoid lose of interrupts */
	if ((pAd->int_pending & WF_MAC_INT_4) || bReschedule)
	{
		RTMP_OS_TASKLET_SCHE(&pObj->mt_mac_int_4_task);
		RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("<--%s():pAd->int_pending=0x%x, bReschedule=%d\n",
			__FUNCTION__, pAd->int_pending, bReschedule));
		return;
	}

	rt2860_int_enable(pAd, WF_MAC_INT_4);
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}

static void bmc_dma_done_tasklet(unsigned long data)
{
    unsigned long flags;
    BOOLEAN bReschedule = 0;
#ifdef WORKQUEUE_BH
    struct work_struct *work = (struct work_struct *)data;
    POS_COOKIE pObj = container_of(work, struct os_cookie, bmc_dma_done_task);
    RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
    PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
    POS_COOKIE pObj;
#endif /* WORKQUEUE_BH */
    UINT32 INT_BMC_DLY = MT_INT_BMC_DLY;

	//printk("==>bmc tasklet\n");
    /* Do nothing if the driver is starting halt state. */
    /* This might happen when timer already been fired before cancel timer with mlmehalt */
    if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS | fRTMP_ADAPTER_NIC_NOT_EXIST))
    {
        RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
        pAd->int_disable_mask &= ~(INT_BMC_DLY);
        RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
        return;
    }

	pObj = (POS_COOKIE) pAd->OS_Cookie;

    RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
    pAd->int_pending &= ~INT_BMC_DLY;
    RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

    bReschedule = RTMPHandleTxRingDmaDoneInterrupt(pAd, TX_BMC_DONE);

    RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
    /* double check to avoid lose of interrupts */
    if ((pAd->int_pending & INT_BMC_DLY) || bReschedule)
    {
        RTMP_OS_TASKLET_SCHE(&pObj->bmc_dma_done_task);
        RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
        MTWF_LOG(DBG_CAT_FPGA, DBG_SUBCAT_ALL, DBG_LVL_NOISY, ("<--%s():pAd->int_pending=0x%x, bReschedule=%d\n",
            __FUNCTION__, pAd->int_pending, bReschedule));
        return;
    }

    rt2860_int_enable(pAd, INT_BMC_DLY);
    RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
}
#endif /* MT_MAC */


#ifdef CONFIG_ATE
static void ate_ac0_dma_done_tasklet(unsigned long data)
{
	return;
}
#endif /* CONFIG_ATE */


#ifdef UAPSD_SUPPORT
/*
========================================================================
Routine Description:
    Used to send the EOSP frame.

Arguments:
    data			Pointer to our adapter

Return Value:
    None

Note:
========================================================================
*/
static void uapsd_eosp_sent_tasklet(unsigned long data)
{
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, uapsd_eosp_sent_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
#endif /* WORKQUEUE_BH */

	RTMPDeQueuePacket(pAd, FALSE, NUM_OF_TX_RING, WCID_ALL, MAX_TX_PROCESS);
}
#endif /* UAPSD_SUPPORT */


#ifdef CONFIG_AP_SUPPORT
#ifdef DFS_SUPPORT
void schedule_dfs_task(RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;
	RTMP_OS_TASKLET_SCHE(&pObj->dfs_task);
}


void dfs_tasklet(unsigned long data)
{
#ifdef WORKQUEUE_BH
	struct work_struct *work = (struct work_struct *)data;
	POS_COOKIE pObj = container_of(work, struct os_cookie, dfs_task);
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pObj->pAd_va;
#else
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER) data;
#endif /* WORKQUEUE_BH */
	PRADAR_DETECT_STRUCT pRadarDetect = &pAd->CommonCfg.RadarDetect;
	PDFS_SW_DETECT_PARAM pDfsSwParam = &pRadarDetect->DfsSwParam;

	if (pRadarDetect->DFSAPRestart == 1)
	{
		int i, j;

		pDfsSwParam->dfs_w_counter += 10;
		/* reset period table */
		for (i = 0; i < pAd->chipCap.DfsEngineNum; i++)
		{
			for (j = 0; j < NEW_DFS_MPERIOD_ENT_NUM; j++)
			{
				pDfsSwParam->DFS_T[i][j].period = 0;
				pDfsSwParam->DFS_T[i][j].idx = 0;
				pDfsSwParam->DFS_T[i][j].idx2 = 0;
			}
		}

		APStop(pAd);
		APStartUp(pAd);
		pRadarDetect->DFSAPRestart = 0;
	}
	else
	/* check radar here */
	{
		int idx;
		if (pRadarDetect->radarDeclared == 0)
		{
			for (idx = 0; idx < 3; idx++)
			{
				if (SWRadarCheck(pAd, idx) == 1)
				{
					/*find the radar signals */
					pRadarDetect->radarDeclared = 1;
					break;
				}
			}
		}
	}
}
#endif /* DFS_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */


int print_int_count = 0;
VOID RTMPHandleInterrupt(VOID *pAdSrc)
{
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pAdSrc;
	UINT32 IntSource;
//+++Add by Carter
	UINT32 StatusRegister, EnableRegister, stat_reg = 0, en_reg = 0;
	UINT32 stat_reg0 = 0,en_reg0 = 0;
//---Add by Carter
	POS_COOKIE pObj;
	unsigned long flags=0;
	UINT32 INT_RX_DATA = 0, INT_RX_CMD=0, TxCoherent = 0, RxCoherent = 0, FifoStaFullInt = 0;
	UINT32 INT_MGMT_DLY = 0, INT_HCCA_DLY = 0, INT_AC3_DLY = 0, INT_AC2_DLY = 0, INT_AC1_DLY = 0, INT_AC0_DLY = 0, INT_BMC_DLY = 0;
#if defined(RTMP_MAC) || defined(RLT_MAC)
	UINT32 PreTBTTInt = 0, TBTTInt = 0;
#endif /* defined(MT_MAC) ||  defined(RTMP_MAC) || defined(RLT_MAC) */
#ifdef DFS_SUPPORT
	UINT32 GPTimeOutInt = 0;
#endif /* DFS_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
	UINT32 RadarInt = 0;
#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
	UINT32	AutoWakeupInt = 0;
#endif /**/

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	/*
		Note 03312008: we can not return here before

		RTMP_IO_READ32(pAd, INT_SOURCE_CSR, &IntSource.word);
		RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, IntSource.word);
		Or kernel will panic after ifconfig ra0 down sometimes
	*/

	/* Inital the Interrupt source. */
	IntSource = 0x00000000L;
//+++Add by Carter
	StatusRegister = 0x00000000L;
	EnableRegister = 0x00000000L;
//---Add by Carter

	/*
		Flag fOP_STATUS_DOZE On, means ASIC put to sleep, elase means ASICK WakeUp
		And at the same time, clock maybe turned off that say there is no DMA service.
		when ASIC get to sleep.
		To prevent system hang on power saving.
		We need to check it before handle the INT_SOURCE_CSR, ASIC must be wake up.

		RT2661 => when ASIC is sleeping, MAC register cannot be read and written.
		RT2860 => when ASIC is sleeping, MAC register can be read and written.
	*/
	/* if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_DOZE)) */
#ifdef MT_MAC
	// TODO: shiang-7603
	if (pAd->chipCap.hif_type == HIF_MT) {
		RTMP_IO_READ32(pAd, MT_INT_SOURCE_CSR, &IntSource);

#ifdef RELEASEEASE_EXCLUDE
        /* 20140212, per Lawrence's comment, when get WF_INT, clear WF_MAC_INT status first then clear HIF int.*/
        //TODO: Carter, make all WF_MAC_INT clear to an API to replace below code segment.
#endif
		if (IntSource & WF_MAC_INT_3)
		{
			RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
            RTMP_IO_READ32(pAd, HWISR3, &stat_reg);
            RTMP_IO_READ32(pAd, HWIER3, &en_reg);
            /* disable the interrupt source */
            RTMP_IO_WRITE32(pAd, HWIER3, (~stat_reg & en_reg));
            /* write 1 to clear */
            RTMP_IO_WRITE32(pAd, HWISR3, stat_reg);
	       	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		}

		if (IntSource & WF_MAC_INT_0)
		{
			RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
            RTMP_IO_READ32(pAd, HWISR0, &stat_reg0);
            RTMP_IO_READ32(pAd, HWIER0, &en_reg0);
            /* disable the interrupt source */
            RTMP_IO_WRITE32(pAd, HWIER0, (~stat_reg0 & en_reg0));
            /* write 1 to clear */
            RTMP_IO_WRITE32(pAd, HWISR0, stat_reg0);
	       	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		}
		RTMP_IO_WRITE32(pAd, MT_INT_SOURCE_CSR, IntSource); /* write 1 to clear */
	}
#endif /* MT_MAC */

#if defined(RLT_MAC) || defined(RTMP_MAC)
	if (pAd->chipCap.hif_type == HIF_RLT || pAd->chipCap.hif_type == HIF_RTMP) {
		RTMP_IO_READ32(pAd, INT_SOURCE_CSR, &IntSource);
		RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, IntSource); /* write 1 to clear */
	}
#endif /* defined(RLT_MAC) || defined(RTMP_MAC) */
/*	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (">>>fOP_STATUS_DOZE<<<\n")); */

//MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():IntSource=0x%x\n", __FUNCTION__, IntSource));

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_START_UP))
		return;

	if (RTMP_TEST_FLAG(pAd, (fRTMP_ADAPTER_RESET_IN_PROGRESS)))
		return;

	if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS)) {
#ifdef MT_MAC
		if (pAd->chipCap.hif_type == HIF_MT)
		{
			/* Fix Rx Ring FULL lead DMA Busy, when DUT is in reset stage */
        		IntSource = IntSource & (MT_INT_CMD | MT_INT_RX | WF_MAC_INT_3);
		}

		if (!IntSource)
		    return;
#endif /* MT_MAC */ 
    }

	/*
		Handle interrupt, walk through all bits
		Should start from highest priority interrupt
		The priority can be adjust by altering processing if statement
	*/
#ifdef DBG
#endif

#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		INT_RX_DATA = MT_INT_RX_DATA;
		INT_RX_CMD = MT_INT_RX_CMD;
		INT_MGMT_DLY = MT_INT_MGMT_DLY;
		INT_HCCA_DLY = MT_INT_CMD;
		INT_AC3_DLY = MT_INT_AC3_DLY;
		INT_AC2_DLY = MT_INT_AC2_DLY;
		INT_AC1_DLY = MT_INT_AC1_DLY;
		INT_AC0_DLY = MT_INT_AC0_DLY;
		INT_BMC_DLY = MT_INT_BMC_DLY;
		TxCoherent = MT_TxCoherent;
		RxCoherent = MT_RxCoherent;

//		PreTBTTInt = MT_PreTBTTInt;
//		TBTTInt = MT_TBTTInt;
//		FifoStaFullInt = MT_FifoStaFullInt;
#ifdef DFS_SUPPORT
//		GPTimeOutInt = MT_GPTimeOutInt;
#endif /* DFS_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
		RadarInt = 0;
#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
//		AutoWakeupInt = MT_AutoWakeupInt;
#endif /* CONFIG_STA_SUPPORT */
		//McuCommand = MT_McuCommand;
	}
#endif /* MT_MAC*/
#ifdef RLT_MAC
	if (pAd->chipCap.hif_type == HIF_RLT) {
		INT_RX_DATA = RLT_INT_RX_DATA;
		INT_RX_CMD = RLT_INT_RX_CMD;
		TxCoherent = RLT_TxCoherent;
		RxCoherent = RLT_RxCoherent;
		FifoStaFullInt = RLT_FifoStaFullInt;
		INT_MGMT_DLY = RLT_INT_MGMT_DLY;
		INT_HCCA_DLY = RLT_INT_HCCA_DLY;
		INT_AC3_DLY = RLT_INT_AC3_DLY;
		INT_AC2_DLY = RLT_INT_AC2_DLY;
		INT_AC1_DLY = RLT_INT_AC1_DLY;
		INT_AC0_DLY = RLT_INT_AC0_DLY;
		PreTBTTInt = RLT_PreTBTTInt;
		TBTTInt = RLT_TBTTInt;
#ifdef DFS_SUPPORT
		GPTimeOutInt = RLT_GPTimeOutInt;
#endif /* DFS_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
		RadarInt = RLT_RadarInt;
#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		AutoWakeupInt = RLT_AutoWakeupInt;
#endif /* CONFIG_STA_SUPPORT */
		//McuCommand = RLT_McuCommand;
	}
#endif /* RLT_MAC*/
#ifdef RTMP_MAC
	if (pAd->chipCap.hif_type == HIF_RTMP) {
		INT_RX_DATA = RTMP_INT_RX;
		TxCoherent = RTMP_TxCoherent;
		RxCoherent = RTMP_RxCoherent;
		FifoStaFullInt = RTMP_FifoStaFullInt;
		INT_MGMT_DLY = RTMP_INT_MGMT_DLY;
		INT_HCCA_DLY = RTMP_INT_HCCA_DLY;
		INT_AC3_DLY = RTMP_INT_AC3_DLY;
		INT_AC2_DLY = RTMP_INT_AC2_DLY;
		INT_AC1_DLY = RTMP_INT_AC1_DLY;
		INT_AC0_DLY = RTMP_INT_AC0_DLY;
		PreTBTTInt = RTMP_PreTBTTInt;
		TBTTInt = RTMP_TBTTInt;
#ifdef DFS_SUPPORT
		GPTimeOutInt = RTMP_GPTimeOutInt;
#endif /* DFS_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
		RadarInt = RTMP_RadarInt;
#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		AutoWakeupInt = RTMP_AutoWakeupInt;
#endif /* CONFIG_STA_SUPPORT */
		//McuCommand = RTMP_McuCommand;
	}
#endif /* RTMP_MAC */

#ifdef  INF_VR9_HW_INT_WORKAROUND
redo:
#endif

	pAd->bPCIclkOff = FALSE;

	/* If required spinlock, each ISR has to acquire and release itself. */

	/* Do nothing if NIC doesn't exist */
	if (IntSource == 0xffffffff)
	{
		RTMP_SET_FLAG(pAd, (fRTMP_ADAPTER_NIC_NOT_EXIST | fRTMP_ADAPTER_HALT_IN_PROGRESS));
		return;
	}

//+++Add by Carter
#ifdef MT_MAC
	if (pAd->chipCap.hif_type == HIF_MT) {
		if (IntSource & WF_MAC_INT_0)
		{	//Check HWISR0
			RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
			if ((pAd->int_disable_mask & WF_MAC_INT_0) == 0)
			{
			UINT32   Lowpart, Highpart;
			/* disable the interrupt source */
			rt2860_int_disable(pAd, WF_MAC_INT_0);
			RTMP_IO_WRITE32(pAd, HWIER0, en_reg0);
                if (stat_reg0 & BIT23) {	
#ifdef DBG
                    AsicGetTsfTime(pAd, &Highpart, &Lowpart);
                    pAd->HandlePreInterruptTime = Lowpart;
#endif
                    //RTMPHandlePreTBTTInterrupt(pAd);
                    RTMP_OS_TASKLET_SCHE(&pObj->mt_mac_int_0_task);
                }
#ifdef DBG	
		else if (stat_reg0 & BIT20) {		
                    AsicGetTsfTime(pAd, &Highpart, &Lowpart);
                    pAd->HandleInterruptTime = Lowpart;		
                }
#endif	
			rt2860_int_enable(pAd, WF_MAC_INT_0);
			}
			pAd->int_pending |= WF_MAC_INT_0;
	       	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): "
				"StatusRegister 0x2443c = 0x%x, \n", __FUNCTION__,
				StatusRegister));
		}

		if (IntSource & WF_MAC_INT_1)
		{	//Check HWISR1
			RTMP_IO_READ32(pAd, HWISR1, &StatusRegister);
			RTMP_IO_READ32(pAd, HWIER1, &EnableRegister);
			/* disable the interrupt source */
			RTMP_IO_WRITE32(pAd, HWIER1, (~StatusRegister & EnableRegister));

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): "
				"StatusRegister 0x24444 = 0x%x, \n", __FUNCTION__,
				StatusRegister));
		}

		if (IntSource & WF_MAC_INT_2)
		{	//Check HWISR2
			RTMP_IO_READ32(pAd, HWISR2, &StatusRegister);
			RTMP_IO_READ32(pAd, HWIER2, &EnableRegister);
			/* disable the interrupt source */
			RTMP_IO_WRITE32(pAd, HWIER2, (~StatusRegister & EnableRegister));

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): "
				"StatusRegister 0x2444c = 0x%x, \n", __FUNCTION__,
				StatusRegister));
		}

		if (IntSource & WF_MAC_INT_3)
		{
			RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
			if ((pAd->int_disable_mask & WF_MAC_INT_3) == 0)
			{
				UINT32   Lowpart, Highpart;
                rt2860_int_disable(pAd, WF_MAC_INT_3);
                RTMP_IO_WRITE32(pAd, HWIER3, en_reg);
                if (stat_reg & BIT31) {
#ifdef DBG
                    AsicGetTsfTime(pAd, &Highpart, &Lowpart);
                    pAd->HandlePreInterruptTime = Lowpart;
#endif
                    //RTMPHandlePreTBTTInterrupt(pAd);
                    RTMP_OS_TASKLET_SCHE(&pObj->mt_mac_int_3_task);
                }
#ifdef DBG
				else if (stat_reg & BIT15) {
                    AsicGetTsfTime(pAd, &Highpart, &Lowpart);
                    pAd->HandleInterruptTime = Lowpart;
                }
#endif
                rt2860_int_enable(pAd, WF_MAC_INT_3);
			}
			pAd->int_pending |= WF_MAC_INT_3;
	       	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		}

		if (IntSource & WF_MAC_INT_4)
		{	//Check HWISR4
			RTMP_IO_READ32(pAd, HWISR4, &StatusRegister);
			//RTMP_IO_WRITE32(pAd, 0x60310054, StatusRegister);
			/* write 1 to clear */
			RTMP_IO_READ32(pAd, HWIER4, &EnableRegister);
			/* disable the interrupt source */
			RTMP_IO_WRITE32(pAd, HWIER4, (~StatusRegister & EnableRegister));

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): "
				"StatusRegister 0x2445c = 0x%x, \n", __FUNCTION__,
				StatusRegister));
		}

		if (IntSource & MT_INT_BCN_DLY)
		{
			RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
			pAd->TxDMACheckTimes = 0;
			if ((pAd->int_disable_mask & MT_INT_BCN_DLY) == 0)
			{
				rt2860_int_disable(pAd, MT_INT_BCN_DLY);
				RTMP_OS_TASKLET_SCHE(&pObj->bcn_dma_done_task);
			}
			pAd->int_pending |= MT_INT_BCN_DLY;
	       	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
		}

		if (IntSource & INT_BMC_DLY)
    	{
			RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
			pAd->TxDMACheckTimes = 0;
            if ((pAd->int_disable_mask & INT_BMC_DLY) == 0)
            {
                rt2860_int_disable(pAd, INT_BMC_DLY);
                RTMP_OS_TASKLET_SCHE(&pObj->bmc_dma_done_task);
            }
            pAd->int_pending |= INT_BMC_DLY;
            RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);
    	}
	}
#endif /* MT_MAC */
//---Add by Carter

	if (IntSource & TxCoherent)
	{
		/*
			When the interrupt occurs, it means we kick a register to send
			a packet, such as TX_MGMTCTX_IDX, but MAC finds some fields in
			the transmit buffer descriptor is not correct, ex: all zeros.
		*/
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (">>>TxCoherent<<<\n"));
		RTMPHandleRxCoherentInterrupt(pAd);
//+++Add by shiang for debug
		rt2860_int_disable(pAd, TxCoherent);
//---Add by shiang for debug
	}

	if (IntSource & RxCoherent)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (">>>RxCoherent<<<\n"));
		RTMPHandleRxCoherentInterrupt(pAd);
//+++Add by shiang for debug
		rt2860_int_disable(pAd, RxCoherent);
//---Add by shiang for debug
	}

	RTMP_INT_LOCK(&pAd->LockInterrupt, flags);
	if (IntSource & FifoStaFullInt)
	{
		if ((pAd->int_disable_mask & FifoStaFullInt) == 0)
		{
			rt2860_int_disable(pAd, FifoStaFullInt);
			RTMP_OS_TASKLET_SCHE(&pObj->fifo_statistic_full_task);
		}
		pAd->int_pending |= FifoStaFullInt;
	}

	if (IntSource & INT_MGMT_DLY)
	{
		pAd->TxDMACheckTimes = 0;
		if ((pAd->int_disable_mask & INT_MGMT_DLY) ==0)
		{
			rt2860_int_disable(pAd, INT_MGMT_DLY);
			RTMP_OS_TASKLET_SCHE(&pObj->mgmt_dma_done_task);
		}
		pAd->int_pending |= INT_MGMT_DLY ;
	}

	if (IntSource & INT_RX_DATA)
	{
		pAd->RxDMACheckTimes = 0;
		pAd->RxPseCheckTimes = 0;
		if ((pAd->int_disable_mask & INT_RX_DATA) == 0)
		{
#ifdef UAPSD_SUPPORT
			UAPSD_TIMING_RECORD_START();
			UAPSD_TIMING_RECORD(pAd, UAPSD_TIMING_RECORD_ISR);
#endif /* UAPSD_SUPPORT */

			rt2860_int_disable(pAd, INT_RX_DATA);
			RTMP_OS_TASKLET_SCHE(&pObj->rx_done_task);
		}
		pAd->int_pending |= INT_RX_DATA;
	}

#ifdef CONFIG_ANDES_SUPPORT
	if (IntSource & INT_RX_CMD)
	{
		pAd->RxDMACheckTimes = 0;
		pAd->RxPseCheckTimes = 0;
		if ((pAd->int_disable_mask & INT_RX_CMD) == 0)
		{
			/* mask INT_R1 */
			rt2860_int_disable(pAd, INT_RX_CMD);
			RTMP_OS_TASKLET_SCHE(&pObj->rx1_done_task);
		}
		pAd->int_pending |= INT_RX_CMD;
	}
#endif /* CONFIG_ANDES_SUPPORT */


	if (IntSource & INT_HCCA_DLY)
	{
		pAd->TxDMACheckTimes = 0;
		if ((pAd->int_disable_mask & INT_HCCA_DLY) == 0)
		{
			rt2860_int_disable(pAd, INT_HCCA_DLY);
			RTMP_OS_TASKLET_SCHE(&pObj->hcca_dma_done_task);
		}
		pAd->int_pending |= INT_HCCA_DLY;
	}

	if (IntSource & INT_AC3_DLY)
	{
		pAd->TxDMACheckTimes = 0;
		if ((pAd->int_disable_mask & INT_AC3_DLY) == 0)
		{
			rt2860_int_disable(pAd, INT_AC3_DLY);
			RTMP_OS_TASKLET_SCHE(&pObj->ac3_dma_done_task);
		}
		pAd->int_pending |= INT_AC3_DLY;
	}

	if (IntSource & INT_AC2_DLY)
	{
		pAd->TxDMACheckTimes = 0;
		if ((pAd->int_disable_mask & INT_AC2_DLY) == 0)
		{
			rt2860_int_disable(pAd, INT_AC2_DLY);
			RTMP_OS_TASKLET_SCHE(&pObj->ac2_dma_done_task);
		}
		pAd->int_pending |= INT_AC2_DLY;
	}

	if (IntSource & INT_AC1_DLY)
	{
		pAd->TxDMACheckTimes = 0;
		pAd->int_pending |= INT_AC1_DLY;

		if ((pAd->int_disable_mask & INT_AC1_DLY) == 0)
		{
			rt2860_int_disable(pAd, INT_AC1_DLY);
			RTMP_OS_TASKLET_SCHE(&pObj->ac1_dma_done_task);
		}
	}

	if (IntSource & INT_AC0_DLY)
	{
/*
		if (IntSource.word & 0x2) {
			UINT32 reg;
			RTMP_IO_READ32(pAd, DELAY_INT_CFG, &reg);
			printk("IntSource = %08x, DELAY_REG = %08x\n", IntSource.word, reg);
		}
*/
		pAd->TxDMACheckTimes = 0;
		pAd->int_pending |= INT_AC0_DLY;
		if ((pAd->int_disable_mask & INT_AC0_DLY) == 0)
		{
			rt2860_int_disable(pAd, INT_AC0_DLY);
			RTMP_OS_TASKLET_SCHE(&pObj->ac0_dma_done_task);
		}
	}
	RTMP_INT_UNLOCK(&pAd->LockInterrupt, flags);

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if (IntSource & PreTBTTInt)
		RTMPHandlePreTBTTInterrupt(pAd);

	if (IntSource & TBTTInt)
		RTMPHandleTBTTInterrupt(pAd);
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
#ifdef DFS_SUPPORT
		if (IntSource & GPTimeOutInt)
		      NewTimerCB_Radar(pAd);
#endif /* DFS_SUPPORT */

#ifdef CARRIER_DETECTION_SUPPORT
		if ((IntSource & RadarInt))
		{
			if (pAd->CommonCfg.CarrierDetect.Enable == TRUE)
				RTMPHandleRadarInterrupt(pAd);
		}
#endif /* CARRIER_DETECTION_SUPPORT*/

	}
#endif /* CONFIG_AP_SUPPORT */


#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		if (IntSource & AutoWakeupInt)
			RTMPHandleTwakeupInterrupt(pAd);
	}
#endif /* CONFIG_STA_SUPPORT */

#ifdef  INF_VR9_HW_INT_WORKAROUND
	/*
		We found the VR9 Demo board provide from Lantiq at 2010.3.8 will miss interrup sometime caused of Rx-Ring Full
		and our driver no longer receive any packet after the interrupt missing.
		Below patch was recommand by Lantiq for temp workaround.
		And shall be remove in next VR9 platform.
	*/
	IntSource = 0x00000000L;
	RTMP_IO_READ32(pAd, INT_SOURCE_CSR, &IntSource);
	RTMP_IO_WRITE32(pAd, INT_SOURCE_CSR, IntSource);
	if (IntSource != 0)
		goto redo;
#endif

	return;
}


/*
========================================================================
Routine Description:
    PCI command kernel thread.

Arguments:
	*Context			the pAd, driver control block pointer

Return Value:
    0					close the thread

Note:
========================================================================
*/
INT RTPCICmdThread(
	IN ULONG Context)
{
	RTMP_ADAPTER *pAd;
	RTMP_OS_TASK *pTask;
	int status;
	status = 0;

	pTask = (RTMP_OS_TASK *)Context;
	pAd = (PRTMP_ADAPTER)RTMP_OS_TASK_DATA_GET(pTask);

	RtmpOSTaskCustomize(pTask);

	NdisAcquireSpinLock(&pAd->CmdQLock);
	pAd->CmdQ.CmdQState = RTMP_TASK_STAT_RUNNING;
	NdisReleaseSpinLock(&pAd->CmdQLock);

	while (pAd->CmdQ.CmdQState == RTMP_TASK_STAT_RUNNING)
	{
		if (RtmpOSTaskWait(pAd, pTask, &status) == FALSE)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_HALT_IN_PROGRESS);
			break;
		}

		if (pAd->CmdQ.CmdQState == RTMP_TASK_STAT_STOPED)
			break;

		if (!pAd->PM_FlgSuspend)
			CMDHandler(pAd);
	}

	if (!pAd->PM_FlgSuspend)
	{	/* Clear the CmdQElements. */
		CmdQElmt	*pCmdQElmt = NULL;

		NdisAcquireSpinLock(&pAd->CmdQLock);
		pAd->CmdQ.CmdQState = RTMP_TASK_STAT_STOPED;
		while(pAd->CmdQ.size)
		{
			RTThreadDequeueCmd(&pAd->CmdQ, &pCmdQElmt);
			if (pCmdQElmt)
			{
				if (pCmdQElmt->CmdFromNdis == TRUE)
				{
					if (pCmdQElmt->buffer != NULL)
						os_free_mem(pAd, pCmdQElmt->buffer);
					os_free_mem(pAd, (PUCHAR)pCmdQElmt);
				}
				else
				{
					if ((pCmdQElmt->buffer != NULL) && (pCmdQElmt->bufferlength != 0))
						os_free_mem(pAd, pCmdQElmt->buffer);
					os_free_mem(pAd, (PUCHAR)pCmdQElmt);
				}
			}
		}

		NdisReleaseSpinLock(&pAd->CmdQLock);
	}
	/* notify the exit routine that we're actually exiting now
	 *
	 * complete()/wait_for_completion() is similar to up()/down(),
	 * except that complete() is safe in the case where the structure
	 * is getting deleted in a parallel mode of execution (i.e. just
	 * after the down() -- that's necessary for the thread-shutdown
	 * case.
	 *
	 * complete_and_exit() goes even further than this -- it is safe in
	 * the case that the thread of the caller is going away (not just
	 * the structure) -- this is necessary for the module-remove case.
	 * This is important in preemption kernels, which transfer the flow
	 * of execution immediately upon a complete().
	 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,( "<---RTPCICmdThread\n"));

	RtmpOSTaskNotifyToExit(pTask);
	return 0;

}


#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
/*
	========================================================================

	Routine Description:

	Arguments:
		Level = RESTORE_HALT : Restore PCI host and Ralink PCIe Link Control field to its default value.
		Level = Other Value : Restore from dot11 power save or radio off status. And force PCI host Link Control fields to 0x1

	========================================================================
*/
VOID RTMPPCIeLinkCtrlValueRestore(RTMP_ADAPTER *pAd, UCHAR Level)
{
	USHORT  PCIePowerSaveLevel, reg16;
	USHORT	Configuration;
	POS_COOKIE 	pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
		return;

	/* Check PSControl Configuration */
	if (pAd->StaCfg.PSControl.field.EnableNewPS == FALSE)
		return;

	/*3090 will not execute the following codes. */
    	/* Check interface : If not PCIe interface, return. */


	if (RT3593_DEVICE_ID_CHECK(pObj->DeviceID))
		return;


#ifdef RT3590
	if ((pObj->DeviceID == NIC3590_PCIe_DEVICE_ID)
		||(pObj->DeviceID == NIC3591_PCIe_DEVICE_ID)
		||(pObj->DeviceID == NIC3592_PCIe_DEVICE_ID))
		return;
#endif /* RT3390 */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s.===>\n", __FUNCTION__));
	PCIePowerSaveLevel = pAd->PCIePowerSaveLevel;
	if ((PCIePowerSaveLevel&0xff) == 0xff)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("return  \n"));
		return;
	}

	if (pObj->parent_pci_dev && (pAd->HostLnkCtrlOffset != 0))
    {
        PCI_REG_READ_WORD(pObj->parent_pci_dev, pAd->HostLnkCtrlOffset, Configuration);
        if ((Configuration != 0) &&
            (Configuration != 0xFFFF))
        {
    		Configuration &= 0xfefc;
    		/* If call from interface down, restore to orginial setting. */
    		if (Level == RESTORE_CLOSE)
    		{
    			Configuration |= pAd->HostLnkCtrlConfiguration;
    		}
    		else
    			Configuration |= 0x0;
            PCI_REG_WIRTE_WORD(pObj->parent_pci_dev, pAd->HostLnkCtrlOffset, Configuration);
    		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Restore PCI host : offset 0x%x = 0x%x\n", pAd->HostLnkCtrlOffset, Configuration));
        }
        else
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Restore PCI host : PCI_REG_READ_WORD failed (Configuration = 0x%x)\n", Configuration));
    }

    if (pObj->pci_dev && (pAd->RLnkCtrlOffset != 0))
    {
        PCI_REG_READ_WORD(pObj->pci_dev, pAd->RLnkCtrlOffset, Configuration);
        if ((Configuration != 0) &&
            (Configuration != 0xFFFF))
        {
    		Configuration &= 0xfefc;
			/* If call from interface down, restore to orginial setting. */
			if (Level == RESTORE_CLOSE)
            	Configuration |= pAd->RLnkCtrlConfiguration;
			else
				Configuration |= 0x0;
            PCI_REG_WIRTE_WORD(pObj->pci_dev, pAd->RLnkCtrlOffset, Configuration);
    		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Restore Ralink : offset 0x%x = 0x%x\n", pAd->RLnkCtrlOffset, Configuration));
        }
        else
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Restore Ralink : PCI_REG_READ_WORD failed (Configuration = 0x%x)\n", Configuration));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s <===\n", __FUNCTION__));
}

/*
	========================================================================

	Routine Description:

	Arguments:
		Max : limit Host PCI and Ralink PCIe device's LINK CONTROL field's value.
		Because now frequently set our device to mode 1 or mode 3 will cause problem.

	========================================================================
*/
VOID RTMPPCIeLinkCtrlSetting(RTMP_ADAPTER *pAd, USHORT Max)
{
	USHORT  PCIePowerSaveLevel, reg16;
	USHORT	Configuration;
	POS_COOKIE 	pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
		return;

	/* Check PSControl Configuration */
	if (pAd->StaCfg.PSControl.field.EnableNewPS == FALSE)
		return;

	/* Check interface : If not PCIe interface, return. */
	/*Block 3090 to enter the following function */


	if (RT3593_DEVICE_ID_CHECK(pObj->DeviceID))
		return;

	if (!RTMP_TEST_PSFLAG(pAd, fRTMP_PS_CAN_GO_SLEEP))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("RTMPPCIePowerLinkCtrl return on fRTMP_PS_CAN_GO_SLEEP flag\n"));
		return;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s===>\n", __FUNCTION__));
	PCIePowerSaveLevel = pAd->PCIePowerSaveLevel;
	if ((PCIePowerSaveLevel&0xff) == 0xff)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("return  \n"));
		return;
	}
	PCIePowerSaveLevel = PCIePowerSaveLevel>>6;

    /* Skip non-exist deice right away */
	if (pObj->parent_pci_dev && (pAd->HostLnkCtrlOffset != 0))
	{
        PCI_REG_READ_WORD(pObj->parent_pci_dev, pAd->HostLnkCtrlOffset, Configuration);
		switch (PCIePowerSaveLevel)
		{
			case 0:
				/* Set b0 and b1 of LinkControl (both 2892 and PCIe bridge) to 00 */
				Configuration &= 0xfefc;
				break;
			case 1:
				/* Set b0 and b1 of LinkControl (both 2892 and PCIe bridge) to 01 */
				Configuration &= 0xfefc;
				Configuration |= 0x1;
				break;
			case 2:
				/*  Set b0 and b1 of LinkControl (both 2892 and PCIe bridge) to 11 */
				Configuration &= 0xfefc;
				Configuration |= 0x3;
				break;
			case 3:
				/* Set b0 and b1 of LinkControl (both 2892 and PCIe bridge) to 11 and bit 8 of LinkControl of 2892 to 1 */
				Configuration &= 0xfefc;
				Configuration |= 0x103;
				break;
		}
        PCI_REG_WIRTE_WORD(pObj->parent_pci_dev, pAd->HostLnkCtrlOffset, Configuration);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Write PCI host offset 0x%x = 0x%x\n", pAd->HostLnkCtrlOffset, Configuration));
	}

	if (pObj->pci_dev && (pAd->RLnkCtrlOffset != 0))
	{
		/* first 2892 chip not allow to frequently set mode 3. will cause hang problem. */
		if (PCIePowerSaveLevel > Max)
			PCIePowerSaveLevel = Max;

        PCI_REG_READ_WORD(pObj->pci_dev, pAd->RLnkCtrlOffset, Configuration);
		Configuration |= 0x100;
        PCI_REG_WIRTE_WORD(pObj->pci_dev, pAd->RLnkCtrlOffset, Configuration);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Write Ralink device : offset 0x%x = 0x%x\n", pAd->RLnkCtrlOffset, Configuration));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RTMPPCIePowerLinkCtrl <==============\n"));
}
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */


/***************************************************************************
 *
 *	PCIe device initialization related procedures.
 *
 ***************************************************************************/
VOID RTMPInitPCIeDevice(RT_CMD_PCIE_INIT *pConfig, VOID *pAdSrc)
{
	PRTMP_ADAPTER pAd = (PRTMP_ADAPTER)pAdSrc;
	VOID *pci_dev = pConfig->pPciDev;
	USHORT  device_id = 0;
	POS_COOKIE pObj;


	pObj = (POS_COOKIE) pAd->OS_Cookie;
	pci_read_config_word(pci_dev, pConfig->ConfigDeviceID, &device_id);
#ifndef RT_BIG_ENDIAN
	device_id = le2cpu16(device_id);
#endif /* RT_BIG_ENDIAN */
	pObj->DeviceID = device_id;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("device_id =0x%x\n", device_id));



	OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE);

#if defined(RTMP_MAC) || defined(RLT_MAC)
	if ((device_id == NIC2860_PCIe_DEVICE_ID) ||
		(device_id == NIC2790_PCIe_DEVICE_ID) ||
		(device_id == VEN_AWT_PCIe_DEVICE_ID) ||
		(device_id == NIC3090_PCIe_DEVICE_ID) ||
		(device_id == NIC3091_PCIe_DEVICE_ID) ||
		(device_id == NIC3092_PCIe_DEVICE_ID) ||
		(device_id == NIC3390_PCIe_DEVICE_ID) ||
		(device_id == NIC3593_PCI_OR_PCIe_DEVICE_ID)||
		(device_id == NIC3592_PCIe_DEVICE_ID) ||
		(device_id ==  NIC5390_PCIe_DEVICE_ID) ||
		(device_id ==  NIC539F_PCIe_DEVICE_ID) ||
		(device_id ==  NIC5392_PCIe_DEVICE_ID) ||
		(device_id ==  NIC5360_PCI_DEVICE_ID) ||
		(device_id ==  NIC5362_PCI_DEVICE_ID) ||
		(device_id ==  NIC5392_PCIe_DEVICE_ID) ||
		(device_id ==  NIC5362_PCI_DEVICE_ID) ||
		(device_id ==  NIC5592_PCIe_DEVICE_ID))
	{
		UINT32 MacCsr0 = 0;
		WaitForAsicReady(pAd);
		RTMP_IO_READ32(pAd, MAC_CSR0, &MacCsr0);

#ifdef CONFIG_STA_SUPPORT
		pAd->chipCap.HW_PCIE_PS_SUPPORT=FALSE;

		if  ((device_id == NIC3090_PCIe_DEVICE_ID) ||
			(device_id == NIC3091_PCIe_DEVICE_ID) ||
			(device_id == NIC3092_PCIe_DEVICE_ID) ||
			(device_id ==  NIC3390_PCIe_DEVICE_ID)||
			RT3593_DEVICE_ID_CHECK(device_id))
		{
			/*Support HW new PCIe power-saving. */
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("RTMPInitPCIeDevice::STA Support HW PCIe Power Saving\n"));
			pAd->chipCap.HW_PCIE_PS_SUPPORT=TRUE;
		}
#endif /* CONFIG_STA_SUPPORT */

		/* Support advanced power save after 2892/2790. */
		/* MAC version at offset 0x1000 is 0x2872XXXX/0x2870XXXX(PCIe, USB, SDIO). */
		if ((MacCsr0 & 0xffff0000) != 0x28600000)
		{

#ifdef PCIE_PS_SUPPORT
			OPSTATUS_SET_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE);
#endif /* PCIE_PS_SUPPORT */
			RtmpRaDevCtrlInit(pAd, RTMP_DEV_INF_PCIE);
			return;
		}

		pAd->infType = RTMP_DEV_INF_PCIE;
	}
#endif /* defined(RTMP_MAC) || defined(RLT_MAC) */

#ifdef MT_MAC
	if (device_id == NIC7603_PCIe_DEVICE_ID 
#ifdef MT7615
		|| (device_id == NIC7615_PCIe_DEVICE_ID)
#endif /* MT7615 */
	)
	{
		UINT32 Value;
	
		RTMP_IO_READ32(pAd, TOP_HVR, &Value);
		pAd->HWVersion = Value;

		RTMP_IO_READ32(pAd, TOP_FVR, &Value);
		pAd->FWVersion = Value; 

		RTMP_IO_READ32(pAd, TOP_HCR, &Value);
		pAd->ChipID = Value;

		if (IS_MT7603(pAd))
		{
			RTMP_IO_READ32(pAd, STRAP_STA, &Value);
			pAd->AntMode = (Value >> 24) & 0x1;
		}

		pAd->chipCap.hif_type = HIF_MT;
		pAd->infType = RTMP_DEV_INF_PCIE;

#ifdef MT7615
// TODO: shiang-7615, for debug
		RTMP_IO_READ32(pAd, HIF_SYS_REV, &Value);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): device_id=0x%x, HWVersion=0x%x, FWVersion=0x%x, pAd->ChipID=0x%x, HIF_SYS_REV=0x%x\n", 
					__FUNCTION__, device_id, pAd->HWVersion, pAd->FWVersion, pAd->ChipID, Value));
//---
#endif /* MT7615 */
	}
#endif /* MT_MAC */
	RtmpRaDevCtrlInit(pAd, pAd->infType);
}


#ifdef CONFIG_STA_SUPPORT
#ifdef PCIE_PS_SUPPORT
VOID RTMPInitPCIeLinkCtrlValue(RTMP_ADAPTER *pAd)
{
    INT     pos;
    USHORT	reg16, data2, PCIePowerSaveLevel, Configuration;
	UINT32 MacValue;
    BOOLEAN	bFindIntel = FALSE;
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	if (!OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Not PCIe device.\n"));
		return;
	}

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s.===>\n", __FUNCTION__));
	/* Init EEPROM, and save settings */
	if (!(IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd) ||
		IS_RT3593(pAd) || IS_RT5390(pAd) || IS_RT5392(pAd) || IS_RT5592(pAd)))
	{
		RT28xx_EEPROM_READ16(pAd, 0x22, PCIePowerSaveLevel);
		pAd->PCIePowerSaveLevel = PCIePowerSaveLevel & 0xff;
		pAd->LnkCtrlBitMask = 0;
		if ((PCIePowerSaveLevel&0xff) == 0xff)
		{
			OPSTATUS_CLEAR_FLAG(pAd, fOP_STATUS_ADVANCE_POWER_SAVE_PCIE_DEVICE);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("====> PCIePowerSaveLevel = 0x%x.\n", PCIePowerSaveLevel));
			return;
		}
		else
		{
			PCIePowerSaveLevel &= 0x3;
			RT28xx_EEPROM_READ16(pAd, 0x24, data2);

			if( !(((data2&0xff00) == 0x9200) && ((data2&0x80) !=0)) )
			{
				if (PCIePowerSaveLevel > 1 )
					PCIePowerSaveLevel = 1;
			}

			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("====> Write 0x83 = 0x%x.\n", PCIePowerSaveLevel));
			AsicSendCommandToMcu(pAd, 0x83, 0xff, (UCHAR)PCIePowerSaveLevel, 0x00, FALSE);
			RT28xx_EEPROM_READ16(pAd, 0x22, PCIePowerSaveLevel);
			PCIePowerSaveLevel &= 0xff;
			PCIePowerSaveLevel = PCIePowerSaveLevel >> 6;
			switch(PCIePowerSaveLevel)
			{
					case 0:	/* Only support L0 */
						pAd->LnkCtrlBitMask = 0;
					break;
					case 1:	/* Only enable L0s */
						pAd->LnkCtrlBitMask = 1;
					break;
					case 2:	/* enable L1, L0s */
						pAd->LnkCtrlBitMask = 3;
					break;
					case 3:	/* sync with host clk and enable L1, L0s */
					pAd->LnkCtrlBitMask = 0x103;
					break;
			}
					RT28xx_EEPROM_READ16(pAd, 0x24, data2);
					if ((PCIePowerSaveLevel&0xff) != 0xff)
					{
						PCIePowerSaveLevel &= 0x3;

						if( !(((data2&0xff00) == 0x9200) && ((data2&0x80) !=0)) )
						{
							if (PCIePowerSaveLevel > 1 )
								PCIePowerSaveLevel = 1;
						}

						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("====> rt28xx Write 0x83 Command = 0x%x.\n", PCIePowerSaveLevel));
							       printk("\n\n\n%s:%d\n",__FUNCTION__,__LINE__);

						AsicSendCommandToMcu(pAd, 0x83, 0xff, (UCHAR)PCIePowerSaveLevel, 0x00);
					}
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("====> LnkCtrlBitMask = 0x%x.\n", pAd->LnkCtrlBitMask));
		}
		}
		else if (IS_RT3090(pAd) || IS_RT3572(pAd) || IS_RT3390(pAd) ||
				IS_RT3593(pAd) || IS_RT5390(pAd) || IS_RT5392(pAd) ||
				IS_RT5592(pAd))
		{
			UCHAR	LinkCtrlSetting = 0;

			/* Check 3090E special setting chip. */
			RT28xx_EEPROM_READ16(pAd, 0x24, data2);
			if ((data2 == 0x9280) && ((pAd->MACVersion&0xffff) == 0x0211))
			{
				pAd->b3090ESpecialChip = TRUE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("Special 3090E chip \n"));
			}

			RTMP_IO_READ32(pAd, AUX_CTRL, &MacValue);
			/*enable WAKE_PCIE function, which forces to enable PCIE clock when mpu interrupt asserting. */
			/*Force PCIE 125MHz CLK to toggle */
			MacValue |= 0x402;
			RTMP_IO_WRITE32(pAd, AUX_CTRL, MacValue);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" AUX_CTRL = 0x%32x\n", MacValue));



			/* for RT30xx F and after, PCIe infterface, and for power solution 3 */
			if ((IS_VERSION_AFTER_F(pAd))
				&& (pAd->StaCfg.PSControl.field.rt30xxPowerMode >= 2)
				&& (pAd->StaCfg.PSControl.field.rt30xxPowerMode <= 3))
			{
				RTMP_IO_READ32(pAd, AUX_CTRL, &MacValue);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" Read AUX_CTRL = 0x%x\n", MacValue));
				/* turn on bit 12. */
				/*enable 32KHz clock mode for power saving */
				MacValue |= 0x1000;
				if (MacValue != 0xffffffff)
				{
					RTMP_IO_WRITE32(pAd, AUX_CTRL, MacValue);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" Write AUX_CTRL = 0x%x\n", MacValue));
					/* 1. if use PCIePowerSetting is 2 or 3, need to program OSC_CTRL to 0x3ff11. */
					MacValue = 0x3ff11;
					RTMP_IO_WRITE32(pAd, OSC_CTRL, MacValue);
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" OSC_CTRL = 0x%x\n", MacValue));
					/* 2. Write PCI register Clk ref bit */
					RTMPrt3xSetPCIePowerLinkCtrl(pAd);
				}
				else
		{
					/* Error read Aux_Ctrl value.  Force to use solution 1 */
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" Error Value in AUX_CTRL = 0x%x\n", MacValue));
					pAd->StaCfg.PSControl.field.rt30xxPowerMode = 1;
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" Force to use power solution1 \n"));
				}
			}
			/* 1. read setting from inf file. */

			PCIePowerSaveLevel = (USHORT)pAd->StaCfg.PSControl.field.rt30xxPowerMode;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("====> rt30xx Read PowerLevelMode =  0x%x.\n", PCIePowerSaveLevel));
			/* 2. Check EnableNewPS. */
			if (pAd->StaCfg.PSControl.field.EnableNewPS == FALSE)
				PCIePowerSaveLevel = 1;

			if (IS_VERSION_BEFORE_F(pAd) && (pAd->b3090ESpecialChip == FALSE))
			{
				/* Chip Version E only allow 1, So force set 1. */
				PCIePowerSaveLevel &= 0x1;
				pAd->PCIePowerSaveLevel = (USHORT)PCIePowerSaveLevel;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("====> rt30xx E Write 0x83 Command = 0x%x.\n", PCIePowerSaveLevel));

				AsicSendCommandToMcu(pAd, 0x83, 0xff, (UCHAR)PCIePowerSaveLevel, 0x00, FALSE);
			}
			else
			{
				/* Chip Version F and after only allow 1 or 2 or 3. This might be modified after new chip version come out. */
				if (!((PCIePowerSaveLevel == 1) || (PCIePowerSaveLevel == 3)))
					PCIePowerSaveLevel = 1;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("====> rt30xx F Write 0x83 Command = 0x%x.\n", PCIePowerSaveLevel));
				pAd->PCIePowerSaveLevel = (USHORT)PCIePowerSaveLevel;
				/* for 3090F , we need to add high-byte arg for 0x83 command to indicate the link control setting in */
				/* PCI Configuration Space. Because firmware can't read PCI Configuration Space */
				if ((pAd->Rt3xxRalinkLinkCtrl & 0x2) && (pAd->Rt3xxHostLinkCtrl & 0x2))
				{
					LinkCtrlSetting = 1;
				}
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("====> rt30xxF LinkCtrlSetting = 0x%x.\n", LinkCtrlSetting));
				AsicSendCommandToMcu(pAd, 0x83, 0xff, (UCHAR)PCIePowerSaveLevel, LinkCtrlSetting, FALSE);
			}

		}

    /* Find Ralink PCIe Device's Express Capability Offset */
	pos = pci_find_capability(pObj->pci_dev, PCI_CAP_ID_EXP);

    if (pos != 0)
    {
        /* Ralink PCIe Device's Link Control Register Offset */
        pAd->RLnkCtrlOffset = pos + PCI_EXP_LNKCTL;
    	pci_read_config_word(pObj->pci_dev, pAd->RLnkCtrlOffset, &reg16);
        Configuration = le2cpu16(reg16);
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Read (Ralink PCIe Link Control Register) offset 0x%x = 0x%x\n",
                                    pAd->RLnkCtrlOffset, Configuration));
        pAd->RLnkCtrlConfiguration = (Configuration & 0x103);
        Configuration &= 0xfefc;
        Configuration |= (0x0);

        pObj->parent_pci_dev = RTMPFindHostPCIDev(pObj->pci_dev);
        if (pObj->parent_pci_dev)
        {
		USHORT  vendor_id;

		pci_read_config_word(pObj->parent_pci_dev, RTMP_OS_PCI_VENDOR_ID, &vendor_id);
		vendor_id = le2cpu16(vendor_id);
		if (vendor_id == PCIBUS_INTEL_VENDOR)
                 {
			bFindIntel = TRUE;
                        RTMP_SET_PSFLAG(pAd, fRTMP_PS_TOGGLE_L1);
                 }
		/* Find PCI-to-PCI Bridge Express Capability Offset */
		pos = pci_find_capability(pObj->parent_pci_dev, PCI_CAP_ID_EXP);

		if (pos != 0)
		{
			BOOLEAN		bChange = FALSE;
			/* PCI-to-PCI Bridge Link Control Register Offset */
			pAd->HostLnkCtrlOffset = pos + PCI_EXP_LNKCTL;
			pci_read_config_word(pObj->parent_pci_dev, pAd->HostLnkCtrlOffset, &reg16);
			Configuration = le2cpu16(reg16);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Read (Host PCI-to-PCI Bridge Link Control Register) offset 0x%x = 0x%x\n",
			                            pAd->HostLnkCtrlOffset, Configuration));
			pAd->HostLnkCtrlConfiguration = (Configuration & 0x103);
			Configuration &= 0xfefc;
			Configuration |= (0x0);

			switch (pObj->DeviceID)
			{

				default:
					break;
			}

			if (bChange)
			{
				reg16 = cpu2le16(Configuration);
				pci_write_config_word(pObj->parent_pci_dev, pAd->HostLnkCtrlOffset, reg16);
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Write (Host PCI-to-PCI Bridge Link Control Register) offset 0x%x = 0x%x\n",
						pAd->HostLnkCtrlOffset, Configuration));
			}
		}
		else
		{
			pAd->HostLnkCtrlOffset = 0;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: cannot find PCI-to-PCI Bridge PCI Express Capability!\n", __FUNCTION__));
		}
        }
    }
    else
    {
        pAd->RLnkCtrlOffset = 0;
        pAd->HostLnkCtrlOffset = 0;
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: cannot find Ralink PCIe Device's PCI Express Capability!\n", __FUNCTION__));
    }

    if (bFindIntel == FALSE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Doesn't find Intel PCI host controller. \n"));
		/* Doesn't switch L0, L1, So set PCIePowerSaveLevel to 0xff */
		pAd->PCIePowerSaveLevel = 0xff;
		/* RT3090 will no co-existence with RT3593 */
		if ((pAd->RLnkCtrlOffset != 0)&&(pAd->chipCap.HW_PCIE_PS_SUPPORT==TRUE))
		{
			pci_read_config_word(pObj->pci_dev, pAd->RLnkCtrlOffset, &reg16);
			Configuration = le2cpu16(reg16);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Read (Ralink 30xx PCIe Link Control Register) offset 0x%x = 0x%x\n",
			                        pAd->RLnkCtrlOffset, Configuration));
			pAd->RLnkCtrlConfiguration = (Configuration & 0x103);
			Configuration &= 0xfefc;
			Configuration |= (0x0);
			reg16 = cpu2le16(Configuration);
			pci_write_config_word(pObj->pci_dev, pAd->RLnkCtrlOffset, reg16);
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Write (Ralink PCIe Link Control Register)  offset 0x%x = 0x%x\n",
			                        pos + PCI_EXP_LNKCTL, Configuration));
		}
	}
}


/*
	========================================================================

	Routine Description:
		1. Write a PCI register for rt30xx power solution 3

	========================================================================
*/
VOID RTMPrt3xSetPCIePowerLinkCtrl(RTMP_ADAPTER *pAd)
{

	ULONG	HostConfiguration = 0;
	ULONG	Configuration;
/*
	ULONG	Vendor;
	ULONG	offset;
*/
	POS_COOKIE 	pObj;
	INT     pos;
    	USHORT	reg16;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("RTMPrt3xSetPCIePowerLinkCtrl.===> %x\n", (UINT)pAd->StaCfg.PSControl.word));

	/* Check PSControl Configuration */
	if (pAd->StaCfg.PSControl.field.EnableNewPS == FALSE)
		return;
	pObj->parent_pci_dev = RTMPFindHostPCIDev(pObj->pci_dev);
        if (pObj->parent_pci_dev)
        {

		/* Find PCI-to-PCI Bridge Express Capability Offset */
		pos = pci_find_capability(pObj->parent_pci_dev, PCI_CAP_ID_EXP);

		if (pos != 0)
		{
			pAd->HostLnkCtrlOffset = pos + PCI_EXP_LNKCTL;
		}
	/* If configurared to turn on L1. */
	HostConfiguration = 0;
		if (pAd->StaCfg.PSControl.field.rt30xxForceASPMTest == 1)
		{
						MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Enter,PSM : Force ASPM \n"));

			/* Skip non-exist deice right away */
			if ((pAd->HostLnkCtrlOffset != 0))
			{
	       		 PCI_REG_READ_WORD(pObj->parent_pci_dev, pAd->HostLnkCtrlOffset, HostConfiguration);
				/* Prepare Configuration to write to Host */
				HostConfiguration |= 0x3;
	        		PCI_REG_WIRTE_WORD(pObj->parent_pci_dev, pAd->HostLnkCtrlOffset, HostConfiguration);
				pAd->Rt3xxHostLinkCtrl = HostConfiguration;
				/* Because in rt30xxForceASPMTest Mode, Force turn on L0s, L1. */
				/* Fix HostConfiguration bit0:1 = 0x3 for later use. */
				HostConfiguration = 0x3;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PSM : Force ASPM : Host device L1/L0s Value =  0x%x\n",(UINT)HostConfiguration));
			}
		}
		else if (pAd->StaCfg.PSControl.field.rt30xxFollowHostASPM == 1)
		{

			/* Skip non-exist deice right away */
			if ((pAd->HostLnkCtrlOffset != 0))
			{
	       		 PCI_REG_READ_WORD(pObj->parent_pci_dev, pAd->HostLnkCtrlOffset, HostConfiguration);
				pAd->Rt3xxHostLinkCtrl = HostConfiguration;
				HostConfiguration &= 0x3;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PSM : Follow Host ASPM : Host device L1/L0s Value =  0x%x\n", (UINT)HostConfiguration));
			}
		}
        }
	/* Prepare to write Ralink setting. */
	/* Find Ralink PCIe Device's Express Capability Offset */
	pos = pci_find_capability(pObj->pci_dev, PCI_CAP_ID_EXP);

    if (pos != 0)
    {
        /* Ralink PCIe Device's Link Control Register Offset */
       pAd->RLnkCtrlOffset = pos + PCI_EXP_LNKCTL;
    	pci_read_config_word(pObj->pci_dev, pAd->RLnkCtrlOffset, &reg16);
        Configuration = le2cpu16(reg16);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Read (Ralink PCIe Link Control Register) offset 0x%x = 0x%x\n",
			                                    pAd->RLnkCtrlOffset, (UINT)Configuration));
		Configuration |= 0x100;
		if ((pAd->StaCfg.PSControl.field.rt30xxFollowHostASPM == 1)
			|| (pAd->StaCfg.PSControl.field.rt30xxForceASPMTest == 1))
		{
			switch(HostConfiguration)
			{
				case 0:
					Configuration &= 0xffffffc;
					break;
				case 1:
					Configuration &= 0xffffffc;
					Configuration |= 0x1;
					break;
				case 2:
					Configuration &= 0xffffffc;
					Configuration |= 0x2;
					break;
				case 3:
					Configuration |= 0x3;
					break;
			}
		}
		reg16 = cpu2le16(Configuration);
		pci_write_config_word(pObj->pci_dev, pAd->RLnkCtrlOffset, reg16);
		pAd->Rt3xxRalinkLinkCtrl = Configuration;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("PSM :Write Ralink device L1/L0s Value =  0x%x\n", (UINT)Configuration));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO,("PSM :RTMPrt3xSetPCIePowerLinkCtrl <==============\n"));

}
#endif /* PCIE_PS_SUPPORT */
#endif /* CONFIG_STA_SUPPORT */

#endif /* RTMP_MAC_PCI */

