/*******************************************************************************
* Copyright (c) 2007 MediaTek Inc.
*
* All rights reserved. Copying, compilation, modification, distribution
* or any other use whatsoever of this material is strictly prohibited
* except in accordance with a Software License Agreement with
* MediaTek Inc.
********************************************************************************
*/
/*******************************************************************************
* LEGAL DISCLAIMER
*
* BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND
* AGREES THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK
* SOFTWARE") RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE
* PROVIDED TO BUYER ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY
* DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT
* LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
* PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE
* ANY WARRANTY WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY
* WHICH MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK
* SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY
* WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE
* FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION OR TO
* CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
* BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
* LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL
* BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT
* ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY
* BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
* WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT
* OF LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING
* THEREOF AND RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN
* FRANCISCO, CA, UNDER THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE
* (ICC).
********************************************************************************
*/
#include "mcu/hal.h"

#ifdef LED_CONTROL_SUPPORT

UINT32 halLedFixBlinking(
	IN PRTMP_ADAPTER pAd,
	IN ENUM_LED_NNUMBER_T ucLedNo, IN BOOLEAN fgTwoState, IN BOOLEAN fgReplay,
	IN UCHAR ucS0OffTime, IN UCHAR ucS0OnTime, IN USHORT u2S0Lasting,
	IN UCHAR ucS1OffTime, IN UCHAR ucS1OnTime, IN USHORT u2S1Lasting,
	IN BOOLEAN fgPolarity
	)
{
	UINT32 rHalStatus = HAL_STATUS_SUCCESS;
	ULONG value = 0, u4LedCrS0, u4LedCrS1;
	UINT32 result = 0, reg = 0;

	/* check value range */
	if (ucLedNo >= LED_NUMBER_MAX)
		return HAL_STATUS_INVALID_PARMS;

	/* check value range */
	if (((ucS0OffTime > 0xff) || ucS0OnTime > 0xff) ||  (u2S0Lasting > 0xffff))
		return HAL_STATUS_INVALID_PARMS;

	if (((ucS1OffTime > 0xff) || ucS1OnTime > 0xff) ||  (u2S1Lasting > 0xffff))
		return HAL_STATUS_INVALID_PARMS;

	RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_1, &reg);
	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, MCU_PCIE_REMAP_1_MAP);

	/* get LED S0/S1 address */

	u4LedCrS0 = LED_0_PARAM_S0 + (WF_LED_PARAM_OFFSET * ucLedNo);
	u4LedCrS1 = LED_0_PARAM_S1 + (WF_LED_PARAM_OFFSET * ucLedNo);

	/* S0/S1 replay mode */
	if (fgTwoState == TRUE) {
		RTMP_IO_WRITE32(pAd, u4LedCrS0, ((ucS0OffTime << WF_LED0_S0_OFF_OFSET) |
			(ucS0OnTime << WF_LED0_S0_ON_OFSET)  |
			(u2S0Lasting << WF_LED0_S0_LASTING_OFFSET)));
		RTMP_IO_WRITE32(pAd, u4LedCrS1, ((ucS1OffTime << WF_LED0_S1_OFF_OFSET) |
			(ucS1OnTime << WF_LED0_S1_ON_OFSET)  |
			(u2S1Lasting << WF_LED0_S1_LASTING_OFFSET)));

		if (fgPolarity == WF_LED0_CTRL_POLARITY_ACTIVE_LOW)
		value |= WF_LED_CTRL_POLARITY;

		if (fgReplay == TRUE)
			value |= WF_LED_CTRL_REPLAY;

		/* clear LED setting */
		RTMP_IO_READ32(pAd, LED_CTRL, &result);
		RTMP_IO_WRITE32(pAd, LED_CTRL, (result & (~((
			WF_LED_CTRL_TX_OVER_BLINK | WF_LED_CTRL_MAUAL_TX_BLINK | WF_LED_CTRL_TX_BLINK_MODE |
			WF_LED_CTRL_POLARITY | WF_LED_CTRL_REPLAY) << (ucLedNo * 8)))));

		RTMP_IO_READ32(pAd, LED_CTRL, &result);
		RTMP_IO_WRITE32(pAd, LED_CTRL, (result | ((WF_LED_CTRL_KICK | value) << (ucLedNo * 8))));
		/* write KICK bit as last step */
	} else {/* S0 only */
		result = (ucS0OffTime << WF_LED0_S0_OFF_OFSET) |
					(ucS0OnTime << WF_LED0_S0_ON_OFSET) |
					(u2S0Lasting << WF_LED0_S0_LASTING_OFFSET);
		RTMP_IO_WRITE32(pAd, u4LedCrS0, result);
		RTMP_IO_WRITE32(pAd, u4LedCrS1, 0);

		if (fgPolarity == WF_LED0_CTRL_POLARITY_ACTIVE_LOW)
			value |= WF_LED_CTRL_POLARITY;

		/* clear LED setting */
		RTMP_IO_READ32(pAd, LED_CTRL, &result);
		result = result & (~((WF_LED_CTRL_TX_OVER_BLINK | WF_LED_CTRL_MAUAL_TX_BLINK |
			WF_LED_CTRL_TX_BLINK_MODE | WF_LED_CTRL_POLARITY | WF_LED_CTRL_REPLAY) << (ucLedNo * 8)));
		RTMP_IO_WRITE32(pAd, LED_CTRL, result);

		/* write KICK bit as last step */
		result = result | ((WF_LED_CTRL_KICK | value) << (ucLedNo * 8));
		RTMP_IO_WRITE32(pAd, LED_CTRL, result);
	}

	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, reg);

	return rHalStatus;
}

UINT32 halLedTxBlinking(
	IN PRTMP_ADAPTER pAd,
	IN ENUM_LED_NNUMBER_T ucLedNo, IN UCHAR ucTxOffTime, IN UCHAR ucTxOnTime,
	IN BOOLEAN fgPolarity, IN BOOLEAN fgTxOverBlink
	)
{
	UINT32 rHalStatus = HAL_STATUS_SUCCESS;
	ULONG value = 0;
	UINT32 result = 0, reg = 0;

	/* check value range */
	if (ucLedNo >= LED_NUMBER_MAX)
		return HAL_STATUS_INVALID_PARMS;

	/* check value range */
	if ((ucTxOffTime > 0xff) || (ucTxOnTime > 0xff))
		return HAL_STATUS_INVALID_PARMS;

	RTMP_IO_READ32(pAd, MCU_PCIE_REMAP_1, &reg);
	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, MCU_PCIE_REMAP_1_MAP);

	switch (ucLedNo) {
	case LED_NUMBER_0:
			RTMP_IO_READ32(pAd, LED_TX_BLINK_PARAM0, &result);
			result &= ~(0x0000FFFF);
			RTMP_IO_WRITE32(pAd, LED_TX_BLINK_PARAM0, result);
			result |= ((ucTxOffTime << WF_LED_CTRL_TX_BLINK_OFF_OFFSET) |
				(ucTxOnTime << WF_LED_CTRL_TX_BLINK_ON_OFFSET));
			RTMP_IO_WRITE32(pAd, LED_TX_BLINK_PARAM0, result);
				break;

	case LED_NUMBER_1:
			RTMP_IO_READ32(pAd, LED_TX_BLINK_PARAM0, &result);
			result &= ~(0xFFFF0000);
			RTMP_IO_WRITE32(pAd, LED_TX_BLINK_PARAM0, result);
			result |= (((ucTxOffTime << WF_LED_CTRL_TX_BLINK_OFF_OFFSET) |
				(ucTxOnTime << WF_LED_CTRL_TX_BLINK_ON_OFFSET)) <<16);
			RTMP_IO_WRITE32(pAd, LED_TX_BLINK_PARAM0, result);
				 break;

	case LED_NUMBER_2:
			RTMP_IO_READ32(pAd, LED_TX_BLINK_PARAM1, &result);
			result &= ~(0x0000FFFF);
			RTMP_IO_WRITE32(pAd, LED_TX_BLINK_PARAM1, result);
			result |= ((ucTxOffTime << WF_LED_CTRL_TX_BLINK_OFF_OFFSET) |
				(ucTxOnTime << WF_LED_CTRL_TX_BLINK_ON_OFFSET));
			RTMP_IO_WRITE32(pAd, LED_TX_BLINK_PARAM1, result);
				break;

	case LED_NUMBER_3:
			RTMP_IO_READ32(pAd, LED_TX_BLINK_PARAM1, &result);
			result &= ~(0xFFFF0000);
			RTMP_IO_WRITE32(pAd, LED_TX_BLINK_PARAM1, result); 
			result |= (((ucTxOffTime << WF_LED_CTRL_TX_BLINK_OFF_OFFSET) |
				(ucTxOnTime << WF_LED_CTRL_TX_BLINK_ON_OFFSET)) << 16);
			RTMP_IO_WRITE32(pAd, LED_TX_BLINK_PARAM1, result);
			break;

	default:
			break;
	}


	/* polarity */
	if (fgPolarity == WF_LED0_CTRL_POLARITY_ACTIVE_LOW)
	value |= WF_LED_CTRL_POLARITY;

	/* tx-over-blink */
	if (fgTxOverBlink == TRUE) {
		value |= WF_LED_CTRL_TX_OVER_BLINK;
		value &= ~(WF_LED_CTRL_TX_BLINK_MODE);
	} else {/* normal tx blink */
		value |= (WF_LED_CTRL_TX_BLINK_MODE);
		value &= ~(WF_LED_CTRL_TX_OVER_BLINK);
	}

	/* clear LED setting */
	RTMP_IO_READ32(pAd, LED_CTRL, &result);
	result = result & (~((WF_LED_CTRL_TX_OVER_BLINK | WF_LED_CTRL_MAUAL_TX_BLINK | WF_LED_CTRL_TX_BLINK_MODE |
					WF_LED_CTRL_POLARITY | WF_LED_CTRL_REPLAY) << (ucLedNo * 8)));
	RTMP_IO_WRITE32(pAd, LED_CTRL, result);
	/* write KICK bit as last step */
	result = result | ((WF_LED_CTRL_KICK | value) << (ucLedNo * 8));
	RTMP_IO_WRITE32(pAd, LED_CTRL, result);

	RTMP_IO_WRITE32(pAd, MCU_PCIE_REMAP_1, reg);
	return rHalStatus;
}
#endif

