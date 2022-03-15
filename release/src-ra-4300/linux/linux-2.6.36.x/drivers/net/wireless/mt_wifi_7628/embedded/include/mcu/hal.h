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
#ifndef _HAL_H
#define _HAL_H

#ifdef LED_CONTROL_SUPPORT

#include "rt_config.h"

/*******************************************************************************
*			    P U B L I C   D A T A
********************************************************************************
*/

/*******************************************************************************
*			   P R I V A T E   D A T A
********************************************************************************
*/

/*******************************************************************************
*				 M A C R O S
********************************************************************************
*/

#define HAL_STATUS_SUCCESS		     ((UINT32) 0x00000000L)
#define HAL_STATUS_INVALID_PARMS	       ((UINT32) 0x00000001L)
#define HAL_STATUS_FAILURE		      ((UINT32) 0x00000002L)
#define HAL_STATUS_TIMEOUT		      ((UINT32) 0x00000004L)
#define HAL_STATUS_ABORT		      ((UINT32) 0x00000008L)
#define HAL_STATUS_BIP_ERROR		    ((UINT32) 0x00000010L)

#ifndef BIT
#define BIT(n)			  ((UINT32) 1 << (n))
#endif /* BIT */

#ifndef BITS
#define BITS(m, n)		       (~(BIT(m)-1) & ((BIT(n) - 1) | BIT(n)))
#define BITS2(m, n)		      (BIT(m) | BIT(n))
#define BITS3(m, n, o)		    (BIT(m) | BIT(n) | BIT(o))
#define BITS4(m, n, o, p)		  (BIT(m) | BIT(n) | BIT(o) | BIT(p))
#endif /* BITS */

/* WIFI LED PIN AUX for 6630 */
#define TOP_RGU_BASE	    0x81020000
#define RGU_AGPS_SYNC_AON_SEL    (TOP_RGU_BASE + 0x3014)
#define WF_LED_PIN_MUX_MASK	     BITS(2, 3)
#define WF_LED_PIN_MUX_OFFSET	   2

/* WIFI LED PIN AUX for MT7603 */
#define TOP_GPIO_BASE	   0x80023000
#define TOP_GPIO_PINMUX_SEL1      (TOP_GPIO_BASE + 0x0080)
#define TOP_GPIO_5_SEL_MASK       BITS(20, 22)
#define TOP_GPIO_5_SEL_OFFSET     20
#define TOP_GPIO_0_SEL_MASK       BITS(0, 3)
#define TOP_GPIO_0_SEL_OFFSET     0

/* LED */
#define MCU_PCIE_REMAP_1_OFFSET   0x40000
#define LED_FW_BASE	       0x80024000
#define MCU_PCIE_REMAP_1_MAP      0x80000000
#define LED_BASE		  (MCU_PCIE_REMAP_1_OFFSET + LED_FW_BASE - MCU_PCIE_REMAP_1_MAP)
#define LED_CTRL		  (LED_BASE  + 0x0000)
#define LED_TX_BLINK_PARAM0       (LED_BASE  + 0x0004)
#define LED_TX_BLINK_PARAM1       (LED_BASE  + 0x0008)
#define LED_0_PARAM_S0	    (LED_BASE  + 0x0010)
#define LED_0_PARAM_S1	    (LED_BASE  + 0x0014)

/* LED0_PR_S0 */
#define WF_LED0_S0_OFF_MASK	       BITS(24, 31)
#define WF_LED0_S0_OFF_OFSET	      24
#define WF_LED0_S0_ON_MASK		BITS(16, 23)
#define WF_LED0_S0_ON_OFSET	       16
#define WF_LED0_S0_LASTING_MASK	   BITS(0, 15)
#define WF_LED0_S0_LASTING_OFFSET	 0

/* LED0_PR_S1 */
#define WF_LED0_S1_OFF_MASK	       BITS(24, 31)
#define WF_LED0_S1_OFF_OFSET	      24
#define WF_LED0_S1_ON_MASK		BITS(16, 23)
#define WF_LED0_S1_ON_OFSET	       16
#define WF_LED0_S1_LASTING_MASK	   BITS(0, 15)
#define WF_LED0_S1_LASTING_OFFSET	 0

#define WF_LED0_CTRL_POLARITY_ACTIVE_HIGH 0
#define WF_LED0_CTRL_POLARITY_ACTIVE_LOW  1

/* WIFI LED for MT7603 */
#define WF_LED_CTRL_KICK		  BIT(7)
#define WF_LED_CTRL_TX_OVER_BLINK	 BIT(5)
#define WF_LED_CTRL_MAUAL_TX_BLINK	BIT(3)
#define WF_LED_CTRL_TX_BLINK_MODE	 BIT(2)
#define WF_LED_CTRL_POLARITY	      BIT(1)
#define WF_LED_CTRL_REPLAY		BIT(0)

#define WF_LED_CTRL_TX_BLINK_OFF_MASK     BITS(8, 15)
#define WF_LED_CTRL_TX_BLINK_OFF_OFFSET   8
#define WF_LED_CTRL_TX_BLINK_ON_MASK      BITS(0, 7)
#define WF_LED_CTRL_TX_BLINK_ON_OFFSET    0

#define EXT_CMD_LED_BEHAVIOR_BIT_POLARITY_REVERSE  BIT(30)
#define EXT_CMD_LED_BEHAVIOR_BIT_TX_OVER_BLINK     BIT(31)

#define WF_LED_PARAM_OFFSET	       0x8

typedef enum _ENUM_LED_NNUMBER_T {
	LED_NUMBER_0 = 0,
	LED_NUMBER_1 = 1,
	LED_NUMBER_2 = 2,
	LED_NUMBER_3 = 3,
	LED_NUMBER_MAX
} ENUM_LED_NNUMBER_T;

UINT32
halLedFixBlinking(
	IN PRTMP_ADAPTER pAd,
	IN ENUM_LED_NNUMBER_T ucLedNo, IN BOOLEAN fgTwoState, IN BOOLEAN fgReplay,
	IN UCHAR ucS0OffTime, IN UCHAR ucS0OnTime, IN USHORT u2S0Lasting,
	IN UCHAR ucS1OffTime, IN UCHAR ucS1OnTime, IN USHORT u2S1Lasting,
	IN BOOLEAN fgPolarity
);

UINT32
halLedTxBlinking(
	IN PRTMP_ADAPTER pAd,
	IN ENUM_LED_NNUMBER_T ucLedNo, IN UCHAR ucTxOffTime, IN UCHAR ucTxOnTime,
	IN BOOLEAN fgPolarity, IN BOOLEAN fgTxOverBlink
);

#endif /* LED_CONTROL_SUPPORT */
#endif /* _HAL_H */

