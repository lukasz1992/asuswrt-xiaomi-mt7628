/*******************************************************************************
* Copyright (c) 2014 MediaTek Inc.
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
/*******************************************************************************
*				C O M P I L E R   F L A G S
********************************************************************************
*/
/*******************************************************************************
*			E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/
#include "rt_config.h"
#include "mcu/andes_mt.h"
#include "mcu/hal.h"
#include "mcu/led.h"
#include "rtmp.h"
#include "rtmp_timer.h"

#ifdef LED_CONTROL_SUPPORT
/*******************************************************************************
*				  P R I V A T E   D A T A
********************************************************************************
*/
static RALINK_TIMER_STRUCT		 rLedTimer;
static BOOLEAN		 s_fgTxBlinkPolarity, s_fgFixBlinkPolarity, s_fgTxOverFix;
static LED_EVENT led_event;
/*******************************************************************************
*					 F U N C T I O N S
********************************************************************************
*/
/*----------------------------------------------------------------------------*/
/*!
* @brief LED init: initialize LED related GPIO/CR
* Refer to emulLedPreConfig, this is for MT7603 setting.
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID ledCrInit(VOID)
{
	/* MT7603 RFB LED use GPIO5 */
	/*DBGPRINT(( "before PIN_MUX1=0x%08x\n", HAL_REG_32(TOP_GPIO_PINMUX_SEL1)));*/
	/*HAL_REG_32(TOP_GPIO_PINMUX_SEL1) &= ~(TOP_GPIO_5_SEL_MASK);*/
	/*HAL_REG_32(TOP_GPIO_PINMUX_SEL1) |= (0x3<<TOP_GPIO_5_SEL_OFFSET);*/
	/* 20140715: Alexcc suggestion: set to 0: WLAN_LED (w/) open-drain, to make E2 RFB LED work */
	/*HAL_REG_32(TOP_GPIO_PINMUX_SEL1) |= (0x0<<TOP_GPIO_5_SEL_OFFSET);*/
	/*DBGPRINT(( "PIN_MUX1=0x%08x\n", HAL_REG_32(TOP_GPIO_PINMUX_SEL1)));*/
}

/*----------------------------------------------------------------------------*/
/*!
* @brief LED basic: Solid On
*
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID ledSolidOn(
	IN PRTMP_ADAPTER pAd,
	ULONG u4LedNo, BOOLEAN fgPolarity)
{
	UCHAR ucS0OffTime, ucS0OnTime, ucS1OffTime, ucS1OnTime;
	USHORT u2S0Lasting = 0, u2S1Lasting = 0;
	BOOLEAN fgTwoState, fgReplay;

	fgTwoState = FALSE;
	fgReplay   = FALSE;

	ucS0OffTime = 0;
	ucS0OnTime  = 2;
	u2S0Lasting = 2;

	ucS1OffTime = 0;
	ucS1OnTime  = 0;
	u2S1Lasting = 0;

	halLedFixBlinking(pAd, u4LedNo, fgTwoState, fgReplay, ucS0OffTime, ucS0OnTime,
		u2S0Lasting, ucS1OffTime, ucS1OnTime, u2S1Lasting, fgPolarity);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief LED basic: Solid Off
*
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID ledSolidOff(
	IN PRTMP_ADAPTER pAd,
	ULONG u4LedNo, BOOLEAN fgPolarity)
{
	UCHAR ucS0OffTime, ucS0OnTime, ucS1OffTime, ucS1OnTime;
	USHORT u2S0Lasting = 0, u2S1Lasting = 0;
	BOOLEAN fgTwoState, fgReplay;

	fgTwoState = FALSE;
	fgReplay   = FALSE;

	ucS0OffTime = 2;
	ucS0OnTime  = 0;
	u2S0Lasting = 2;

	ucS1OffTime = 0;
	ucS1OnTime  = 0;
	u2S1Lasting = 0;

	halLedFixBlinking(pAd, u4LedNo, fgTwoState, fgReplay, ucS0OffTime, ucS0OnTime,
		u2S0Lasting, ucS1OffTime, ucS1OnTime, u2S1Lasting, fgPolarity);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief LED Tx blinking: Fast blinking, 30ms off, 70ms on
* polarity = 0 (High), solid on when blinking stop
* polarity = 1 (Low), solid off when blinking stop
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID ledTxBlink(
	IN PRTMP_ADAPTER pAd,
	ULONG u4LedNo, BOOLEAN fgPolarity, BOOLEAN fgTxOverFix)
{
	UCHAR ucTxOffTime, ucTxOnTime;

	ucTxOffTime = 3;
	ucTxOnTime = 7;
	/* 20141201: we boundle it to high, but we should use input parameter */

	halLedTxBlinking(pAd, u4LedNo, ucTxOffTime, ucTxOnTime, fgPolarity, fgTxOverFix);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief LED Fix blinking: Slow blinking, one blink/sec
*
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID ledOneBlinkPersec(
	IN PRTMP_ADAPTER pAd,
	ULONG u4LedNo, BOOLEAN fgPolarity)
{
	UCHAR ucS0OffTime, ucS0OnTime, ucS1OffTime, ucS1OnTime;
	USHORT u2S0Lasting = 0, u2S1Lasting = 0;
	BOOLEAN fgTwoState, fgReplay;

	fgTwoState = FALSE;
	fgReplay   = FALSE;

	ucS0OffTime = 50;
	ucS0OnTime  = 50;
	u2S0Lasting = 0;

	ucS1OffTime = 0;
	ucS1OnTime  = 0;
	u2S1Lasting = 0;

	halLedFixBlinking(pAd, u4LedNo, fgTwoState, fgReplay, ucS0OffTime, ucS0OnTime,
		u2S0Lasting, ucS1OffTime, ucS1OnTime, u2S1Lasting, fgPolarity);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief LED Fix blinking: fast blinking, two blink/sec
*
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID ledTwoBlinkPersec(
	IN PRTMP_ADAPTER pAd,
	ULONG u4LedNo, BOOLEAN fgPolarity)
{
	UCHAR ucS0OffTime, ucS0OnTime, ucS1OffTime, ucS1OnTime;
	USHORT u2S0Lasting = 0, u2S1Lasting = 0;
	BOOLEAN fgTwoState, fgReplay;

	fgTwoState = FALSE;
	fgReplay   = FALSE;

	ucS0OffTime = 25;
	ucS0OnTime  = 25;
	u2S0Lasting = 0;

	ucS1OffTime = 0;
	ucS1OnTime  = 0;
	u2S1Lasting = 0;

	halLedFixBlinking(pAd, u4LedNo, fgTwoState, fgReplay, ucS0OffTime, ucS0OnTime,
		u2S0Lasting, ucS1OffTime, ucS1OnTime, u2S1Lasting, fgPolarity);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief LED Fix blinking: fast blinking, three blink/sec
*
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID ledThreeBlinkPersec(
	IN PRTMP_ADAPTER pAd,
	ULONG u4LedNo, BOOLEAN fgPolarity)
{
	UCHAR ucS0OffTime, ucS0OnTime, ucS1OffTime, ucS1OnTime;
	USHORT u2S0Lasting = 0, u2S1Lasting = 0;
	BOOLEAN fgTwoState, fgReplay;

	fgTwoState = FALSE;
	fgReplay   = FALSE;

	ucS0OffTime = 17;
	ucS0OnTime  = 17;
	u2S0Lasting = 0;

	ucS1OffTime = 0;
	ucS1OnTime  = 0;
	u2S1Lasting = 0;

	halLedFixBlinking(pAd, u4LedNo, fgTwoState, fgReplay, ucS0OffTime, ucS0OnTime,
		u2S0Lasting, ucS1OffTime, ucS1OnTime, u2S1Lasting, fgPolarity);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief LED Fix blinking: Fast blinking, Three blink/sec, last for 4 seconds
*
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID ledThreeBlinkPersecLastFourSec(
	IN PRTMP_ADAPTER pAd,
	ULONG u4LedNo, BOOLEAN fgPolarity)
{
	UCHAR ucS0OffTime, ucS0OnTime, ucS1OffTime, ucS1OnTime;
	USHORT u2S0Lasting = 0, u2S1Lasting = 0;
	BOOLEAN fgTwoState, fgReplay;

	fgTwoState = TRUE;
	fgReplay   = FALSE;

	ucS0OffTime = 17;
	ucS0OnTime  = 17;
	u2S0Lasting = 400;

	ucS1OffTime = 2;	
	ucS1OnTime  = 0;
	u2S1Lasting = 2;

	halLedFixBlinking(pAd, u4LedNo, fgTwoState, fgReplay, ucS0OffTime, ucS0OnTime,
		u2S0Lasting, ucS1OffTime, ucS1OnTime, u2S1Lasting, fgPolarity);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief LED Fix blinking: Fast blinking, 500ms on, 100ms off
*
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID ledBlink500msOn100msOff(
	IN PRTMP_ADAPTER pAd,
	ULONG u4LedNo, BOOLEAN fgPolarity)
{
	UCHAR ucS0OffTime, ucS0OnTime, ucS1OffTime, ucS1OnTime;
	USHORT u2S0Lasting = 0, u2S1Lasting = 0;
	BOOLEAN fgTwoState, fgReplay;

	fgTwoState = FALSE;
	fgReplay   = FALSE;

	ucS0OffTime = 10;
	ucS0OnTime  = 50;
	u2S0Lasting = 0;

	ucS1OffTime = 0;
	ucS1OnTime  = 0;
	u2S1Lasting = 0;

	halLedFixBlinking(pAd, u4LedNo, fgTwoState, fgReplay, ucS0OffTime, ucS0OnTime,
		u2S0Lasting, ucS1OffTime, ucS1OnTime, u2S1Lasting, fgPolarity);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief LED Generic Fix blinking: 
*
* Input: S0-on-time, S0-off-time
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID ledGenericFixBlinking(
	IN PRTMP_ADAPTER pAd,
	ULONG u4LedNo, BOOLEAN fgPolarity, UCHAR ucS0OffTime, UCHAR ucS0OnTime)
{
	UCHAR ucS1OffTime, ucS1OnTime;
	USHORT u2S0Lasting = 0, u2S1Lasting = 0;
	BOOLEAN fgTwoState, fgReplay;

	fgTwoState = FALSE;
	fgReplay   = FALSE;

	/* S0 on/off get from function input */
	u2S0Lasting = 0;

	/* S1 ignore */
	ucS1OffTime = 0;
	ucS1OnTime  = 0;
	u2S1Lasting = 0;

	halLedFixBlinking(pAd, u4LedNo, fgTwoState, fgReplay, ucS0OffTime, ucS0OnTime,
		u2S0Lasting, ucS1OffTime, ucS1OnTime, u2S1Lasting, fgPolarity);
}


/* WPS timer handler */
VOID ledOn5secOff3secTimeout(
	IN PVOID SystemSpecific1, 
	IN PVOID FunctionContext, 
	IN PVOID SystemSpecific2, 
	IN PVOID SystemSpecific3) 
{
	LED_EVENT *pled_event = (LED_EVENT *)FunctionContext;
	ULONG u4LedNo;
	UCHAR  ucEvent;
	BOOLEAN Cancelled = TRUE;
	PRTMP_ADAPTER pAd = pled_event->pAd;

	u4LedNo = pled_event->u4LedNo & 0xFFFF;
	ucEvent = (pled_event->u4LedNo >> 16) & 0xFF;

	switch (ucEvent) {
	case LED_TIMEOUT_WPS_SUCCESS_ON_5S_OFF_3S:
		ledSolidOff(pAd, u4LedNo, s_fgFixBlinkPolarity);

		/* we have only one timer parameter, so integrate event to u4LedNo */
		/* u4LedNo: byte0~1. event: byte2. */
		u4LedNo |= (LED_TIMEOUT_WPS_SUCCESS_ON_5S_OFF_3S_TX_BLINK << 16);
			led_event.u4LedNo = u4LedNo;
			/* timeout 3 sec off */
		if (rLedTimer.Valid)
			RTMPCancelTimer(&rLedTimer, &Cancelled);
		else
			RTMPInitTimer(pAd, &rLedTimer, GET_TIMER_FUNCTION(ledOn5secOff3secTimeout), &led_event, FALSE);

		RTMPSetTimer(&rLedTimer, 3*1000);
		break;

	/* Tx Blinking after 3 seconds off */
	case LED_TIMEOUT_WPS_SUCCESS_ON_5S_OFF_3S_TX_BLINK:
		ledTxBlink(pAd, u4LedNo, s_fgTxBlinkPolarity, s_fgTxOverFix);
		break;

		/* Solif Off after 5 seconds On */
	case LED_TIMEOUT_WPS_SUCCESS_ON_5S_OFF:
		ledSolidOff(pAd, u4LedNo, s_fgFixBlinkPolarity);
		break;

	default:
		break;
	}
}

/*----------------------------------------------------------------------------*/
/*!
* @brief LED blinking: On 5 sec, then off 3 sec. When end, switch to Tx blink
*
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID ledOn5secOff3sec(
	PRTMP_ADAPTER pAd,
	ULONG u4LedNo, BOOLEAN fgPolarity)
{
	BOOLEAN Cancelled = TRUE;

	ledSolidOn(pAd, u4LedNo, fgPolarity);
	/* we have only one timer parameter, so integrate event to u4LedNo */
	/* u4LedNo: byte0~1. event: byte2. */
	u4LedNo |= (LED_TIMEOUT_WPS_SUCCESS_ON_5S_OFF<<16);
	led_event.pAd = pAd;
	led_event.u4LedNo = u4LedNo;

	/* timeout 5 sec  */
	if (rLedTimer.Valid)
		RTMPCancelTimer(&rLedTimer, &Cancelled);
	else
		RTMPInitTimer(pAd, &rLedTimer, GET_TIMER_FUNCTION(ledOn5secOff3secTimeout), &led_event, FALSE);

	RTMPSetTimer(&rLedTimer, 5*1000);
}

/*----------------------------------------------------------------------------*/
/*!
* @brief LED blinking: On 5 sec, then off 3 sec. When end, switch to Tx blink
*
*
* @return (none)
*/
/*----------------------------------------------------------------------------*/
VOID ledOn5secOff3secTxBlink(
	PRTMP_ADAPTER pAd,
	ULONG u4LedNo, BOOLEAN fgPolarity)
{
	BOOLEAN Cancelled = TRUE;

	ledSolidOn(pAd, u4LedNo, fgPolarity);

	/* we have only one timer parameter, so integrate event to u4LedNo */
	/* u4LedNo: byte0~1. event: byte2. */
	u4LedNo |= (LED_TIMEOUT_WPS_SUCCESS_ON_5S_OFF_3S<<16);
	led_event.pAd = pAd;
	led_event.u4LedNo = u4LedNo;

	/* timeout 5 sec  */
	if (rLedTimer.Valid)
		RTMPCancelTimer(&rLedTimer, &Cancelled);
	else
		RTMPInitTimer(pAd, &rLedTimer, GET_TIMER_FUNCTION(ledOn5secOff3secTimeout), &led_event, FALSE);

	RTMPSetTimer(&rLedTimer, 5*1000);
}

/*----------------------------------------------------------------------------*/
/*!
* \brief	LED command
*
* \param[in]
*
* \return none
*/
/*----------------------------------------------------------------------------*/
INT hemExtCmdLedCtrl(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR u4LedNo,
	IN UCHAR ucOnTime,
	IN UCHAR ucOffTime,
	IN UCHAR Led_Parameter)
{
	BOOLEAN			fgTxBlinkPolarity, fgFixBlinkPolarity;
	BOOLEAN			fgPolarityReverse, fgTxOverFix;
	/* get revese bit */
	if (Led_Parameter & EXT_CMD_LED_BEHAVIOR_BIT_POLARITY_REVERSE)
		fgPolarityReverse = TRUE;
	else
		fgPolarityReverse = FALSE;

	/* get TX over fix blink bit */
	if (Led_Parameter & EXT_CMD_LED_BEHAVIOR_BIT_TX_OVER_BLINK)
		fgTxOverFix = TRUE;
	else
		fgTxOverFix = FALSE;

	fgFixBlinkPolarity = WF_LED0_CTRL_POLARITY_ACTIVE_LOW ^ fgPolarityReverse;
	fgTxBlinkPolarity  = WF_LED0_CTRL_POLARITY_ACTIVE_HIGH ^ fgPolarityReverse;
	/* save the polarity for WPS timeout case */
	s_fgFixBlinkPolarity = fgFixBlinkPolarity;
	s_fgTxBlinkPolarity  = fgTxBlinkPolarity;
	s_fgTxOverFix        = fgTxOverFix;

	switch (Led_Parameter) {
	/*** COMMON ***/
	case ENUM_LED_BEHAVIOR_SOLID_ON:
		ledSolidOn(pAd, u4LedNo, fgFixBlinkPolarity);
		break;

	case ENUM_LED_BEHAVIOR_SOLID_OFF:
		ledSolidOff(pAd, u4LedNo, fgFixBlinkPolarity);
		break;

			/*** BLINKING ***/
	case ENUM_LED_BEHAVIOR_BLINKING_TX_MAC:
		ledTxBlink(pAd, u4LedNo, fgTxBlinkPolarity, fgTxOverFix);
		break;

	case ENUM_LED_BEHAVIOR_BLINKING_1_BLINK_PER_SEC:
		ledOneBlinkPersec(pAd, u4LedNo, fgFixBlinkPolarity);
		break;

	case ENUM_LED_BEHAVIOR_BLINKING_2_BLINK_PER_SEC:
		ledTwoBlinkPersec(pAd, u4LedNo, fgFixBlinkPolarity);
		break;

	case ENUM_LED_BEHAVIOR_BLINKING_3_BLINK_PER_SEC:
		ledThreeBlinkPersec(pAd, u4LedNo, fgFixBlinkPolarity);
		break;

	case ENUM_LED_BEHAVIOR_BLINKING_500MS_ON_100MS_OFF:
		ledBlink500msOn100msOff(pAd, u4LedNo, fgFixBlinkPolarity);
		break;

	/*** WPS ***/
	case ENUM_LED_BEHAVIOR_WPS_3_BLINK_FOR_4_SEC:
		ledThreeBlinkPersecLastFourSec(pAd, u4LedNo, fgFixBlinkPolarity);
		break;

	case ENUM_LED_BEHAVIOR_WPS_5S_ON_3S_OFF_THEN_BLINKG_TX:
		ledOn5secOff3secTxBlink(pAd, u4LedNo, fgFixBlinkPolarity);
		break;

	case ENUM_LED_BEHAVIOR_WPS_5S_ON:
		ledOn5secOff3sec(pAd, u4LedNo, fgFixBlinkPolarity);
		break;

		/*** Genric FIX BLINKING ***/
	case ENUM_LED_BEHAVIOR_GENERIC_FIX_BLINKING:
		ledGenericFixBlinking(pAd, u4LedNo, fgFixBlinkPolarity, ucOffTime, ucOnTime);
		break;

	/*** Default ***/
	default:
		break;
	}
	return 0;
}

#endif /* LED_CONTROL_SUPPORT */
