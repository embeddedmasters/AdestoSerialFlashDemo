/**************************************************************************//**
 * @file
 * @brief Energy Mode enabled LCD Controller test and demo for EFM32TG_STK3300
 * @version 4.1.0
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2014 Silicon Labs, http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup LCD_Functions
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup LCD_Test
 * @{
 ******************************************************************************/

#ifndef LCDTEST_H_
#define LCDTEST_H_

#include "rtcdriver.h"

/** Timer used for bringing the system back to EM0. */
extern RTCDRV_TimerID_t xTimerForWakeUp;

/**************************************************************************//**
 * @brief Initialize GPIO interrupt on PC14
 *****************************************************************************/
void GPIO_IRQInit(void);

/**************************************************************************//**
 * @brief Sleeps in EM2 in given time unless some other IRQ sources has been
 *        enabled
 * @param msec Time in milliseconds
 *****************************************************************************/
void EM2Sleep(uint32_t msec);

/** @} (end addtogroup LCD_Test) */
/** @} (end addtogroup LCD_Functions) */

#endif /* LCDTEST_H_ */
