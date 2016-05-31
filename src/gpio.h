/******************************************************************************
 * @file gpio.h
 * @brief Adesto Serial Flash Demo
 * @author Embedded Masters
 * @version 1.0
 ******************************************************************************
 * @section License
 * <b>(C) Copyright 2016 Embedded Masters LLC, http://www.embeddedmasters.com</b>
 *******************************************************************************
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * DISCLAIMER OF WARRANTY/LIMITATION OF REMEDIES: Embedded Masters has no
 * obligation to support this Software. Embedded Masters is providing the
 * Software "AS IS", with no express or implied warranties of any kind,
 * including, but not limited to, any implied warranties of merchantability
 * or fitness for any particular purpose or warranties against infringement
 * of any proprietary rights of a third party.
 *
 * Embedded Masters will not be liable for any consequential, incidental, or
 * special damages, or any other relief, or for any claim by any third party,
 * arising from your use of this Software.
 *
 ******************************************************************************/

#ifndef GPIO_H_
#define GPIO_H_

#include <em_gpio.h>

/***************************************************************************//**
 * @addtogroup Peripheral_Functions
 * @{
 ******************************************************************************/
/***************************************************************************//**
 * @defgroup GPIO
 * @brief STK Push-button definitions and function handlers for PB0/PB1.
 * @{
 ******************************************************************************/


typedef struct
{
	GPIO_Port_TypeDef port;
	unsigned int      pin;
	GPIO_Mode_TypeDef mode;
	unsigned int      out;  // initial value of output register bit
} gpio_init_t;

typedef void gpio_callback_t(void *ref);

void gpio_init(const gpio_init_t *table, unsigned int count);

void gpio_irq_init(void);

void gpio_irq_handler_install(GPIO_Port_TypeDef port,
		                      unsigned int      pin,
		                      gpio_callback_t   *callback,
		                      void              *ref);

void gpio_irq_handler_remove(GPIO_Port_TypeDef port,
		                     unsigned int      pin);

/** @} (end addtogroup GPIO) */
/** @} (end addtogroup Peripheral_Functions) */

#endif /* GPIO_H_ */
