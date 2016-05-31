/******************************************************************************
 * @file gpio.c
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

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "em_gpio.h"

#include "gpio.h"

/***************************************************************************//**
 * @addtogroup Peripheral_Functions
 * @{
 ******************************************************************************/
/***************************************************************************//**
 * @addtogroup GPIO
 * @brief STK Push-button definitions and function handlers for PB0/PB1.
 * @{
 ******************************************************************************/


#define MAX_PIN 16

/***************************************************************************//**
 * @brief
 *   GPIO Initialization
 *
 * @param[in] *table
 * 		Pointer to GPIO table.
 * @param[in] count
 * 		Number of GPIO to initialize
 *
 ******************************************************************************/
void gpio_init(const gpio_init_t *table, unsigned int count)
{
	while(count--)
	{
		GPIO_PinModeSet(table->port, table->pin, table->mode, table->out);
		table++;
	}
}


static gpio_callback_t *gpio_callback[MAX_PIN];
static void *gpio_callback_ref[MAX_PIN];

/***************************************************************************//**
 * @brief
 *   Even Numbered GPIO Interrupt Handler
 *
 ******************************************************************************/
void GPIO_EVEN_IRQHandler(void)
{
	int pin;
	uint32_t int_flags = GPIO_IntGet();

	for (pin = 0; pin < MAX_PIN; pin += 2)
		if (int_flags & (1 << pin))
		{
			GPIO_IntClear(1 << pin);
			gpio_callback[pin](gpio_callback_ref[pin]);
		}
}

/***************************************************************************//**
 * @brief
 *   Odd Numbered GPIO Interrupt Handler
 *
 ******************************************************************************/
void GPIO_ODD_IRQHandler(void)
{
	int pin;
	uint32_t int_flags = GPIO_IntGet();

	for (pin = 1; pin < MAX_PIN; pin += 2)
		if (int_flags & (1 << pin))
		{
			GPIO_IntClear(1 << pin);
			gpio_callback[pin](gpio_callback_ref[pin]);
		}
}

/***************************************************************************//**
* @brief
*   GPIO Interrupt Initialize EVEN and ODD
* @note
* 	Call gpio_irq_init() before installing any interrupt handlers
*  	using gpio_irq_handler_install().
*******************************************************************************/
void gpio_irq_init(void)
{
	memset(gpio_callback, 0, sizeof(gpio_callback));
	NVIC_ClearPendingIRQ(GPIO_EVEN_IRQn);
	NVIC_EnableIRQ(GPIO_EVEN_IRQn);
    NVIC_ClearPendingIRQ(GPIO_ODD_IRQn);
    NVIC_EnableIRQ(GPIO_ODD_IRQn);
}

/***************************************************************************//**
 * @brief
 *   GPIO Interrupt Callback Configuration
 *
 * @note
 *	gpio_irq_handler_install() presently will not support
*	interrupt handlers for multiple pins with the same number,
*	i.e., PB7 and PE7.
*	gpio_irq_install() does not actually enable the interrupt;
*	to enable, call e.g.
*	GPIO_IntConfig(port, pin, risingEdge, fallingEdge, enable);
*
*
* @param[in] port
* 		Port Definition
* @param[in] pin
* 		Pin Number
* @param[in] *callback
* 		Pointer to callback function
* @param[in] *ref
* 		Referring Function
*******************************************************************************/
void gpio_irq_handler_install(GPIO_Port_TypeDef port,
		                      unsigned int      pin,
		                      gpio_callback_t   *callback,
		                      void              *ref)
{
	gpio_callback[pin] = callback;
	gpio_callback_ref[pin] = ref;
}

/***************************************************************************//**
 * @brief
 *   Remove GPIO Interrupt Handler
 *
 * @param[in] port
 * 		Port Definition
 * @param[in] pin
 * 		Pin Number
******************************************************************************/
void gpio_irq_handler_remove(GPIO_Port_TypeDef port,
		             	 	 unsigned int      pin)
{
	gpio_callback[pin] = NULL;
}

/** @} (end addtogroup GPIO) */
/** @} (end addtogroup Peripheral_Functions) */
