/****************************************************************************//**
 * @file button.c
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

#include "em_gpio.h"

#include "gpio.h"
#include "button.h"

/***************************************************************************//**
 * @addtogroup Peripheral_Functions
 * @{
 ******************************************************************************/
/***************************************************************************//**
 * @addtogroup Buttons
 * @brief STK Push-button definitions and function handlers for PB0/PB1.
 * @{
 ******************************************************************************/

#define BUTTON0_PORT gpioPortB
#define BUTTON0_PIN  9
#define BUTTON1_PORT gpioPortB
#define BUTTON1_PIN 10


typedef struct
{
	GPIO_Port_TypeDef port;
	int pin;
} button_info_t;

static const button_info_t button_info[] =
{
	{ BUTTON0_PORT, BUTTON0_PIN },
	{ BUTTON1_PORT, BUTTON1_PIN }
};

static button_callback_fn_t *button_callback;

/***************************************************************************//**
 * @brief
 *   Button Callback function
 *
 * @param[in] *ref
 * 		Pointer to Button callback function.
 *
 ******************************************************************************/
void button_gpio_callback(void *ref)
{
	int num = (int) ref;
	int state;
	const button_info_t *info = & button_info[num];

	state = ! GPIO_PinInGet(info->port, info->pin);
	button_callback(num, state);
}


gpio_init_t button_gpio_init[] =
{
	{ BUTTON0_PORT, BUTTON0_PIN, gpioModeInputPullFilter, 1 },
	{ BUTTON1_PORT, BUTTON1_PIN, gpioModeInputPullFilter, 1 },
};

/***************************************************************************//**
 * @brief
 *   Push-button initialization
 *
 * @param[in] *callback
 * 		Pointer to Button callback function.
 *
 ******************************************************************************/
void button_init(button_callback_fn_t *callback)
{
	button_callback = callback;

	gpio_irq_handler_install(BUTTON0_PORT, BUTTON0_PIN, button_gpio_callback, (void *) 0);
	gpio_irq_handler_install(BUTTON1_PORT, BUTTON1_PIN, button_gpio_callback, (void *) 1);

	gpio_init(button_gpio_init, sizeof(button_gpio_init)/sizeof(gpio_init_t));

	// Set both falling and rising edge interrupts for both buttons
	GPIO_IntConfig(BUTTON0_PORT, BUTTON0_PIN, true, true, true);
	GPIO_IntConfig(BUTTON1_PORT, BUTTON1_PIN, true, true, true);
}

/** @} (end addtogroup Buttons) */
/** @} (end addtogroup Peripheral Functions) */
