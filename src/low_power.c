/****************************************************************************//**
 * @file low_power.c
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

#include "em_emu.h"
#include "gpio.h"

#include "low_power.h"
#include "spi.h"

/***************************************************************************//**
 * @addtogroup Peripheral_Functions
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @defgroup Low_Power
 * @{
 ******************************************************************************/

// STK exp connecter pin 14
#define EM1_DEBUG_PORT gpioPortD
#define EM1_DEBUG_PIN  5

// STK exp connecter pin 16
#define EM2_DEBUG_PORT gpioPortD
#define EM2_DEBUG_PIN  6


static bool force_em0;

volatile uint32_t wait_em1_count;
volatile uint32_t wait_em2_count;

/***************************************************************************//**
 * @brief
 *   Force to EM0/Run Mode
 *
 * @param[in] val
 * 		1 Sets to EM0 0 Clears forcing to EM0.
 *
 ******************************************************************************/
void set_force_em0(bool val)
{
	force_em0 = val;
}

/***************************************************************************//**
 * @brief
 *   Decides whether to put EFM into EM0, EM1, or EM2 based on Activity
 *
 ******************************************************************************/
void enter_low_power_state(void)
{
	if (force_em0)
		__ASM volatile ("wfi");
	else if (spi_active())
	{
//		wait_em1_count++;
		GPIO_PinOutSet(EM1_DEBUG_PORT, EM1_DEBUG_PIN);
		EMU_EnterEM1();
		GPIO_PinOutClear(EM1_DEBUG_PORT, EM1_DEBUG_PIN);
	}
	else
	{
//		wait_em2_count++;
		GPIO_PinOutSet(EM2_DEBUG_PORT, EM2_DEBUG_PIN);
		EMU_EnterEM2(true);
		GPIO_PinOutClear(EM2_DEBUG_PORT, EM2_DEBUG_PIN);
	}
}

static const gpio_init_t lp_debug_pins[] =
{
  { EM1_DEBUG_PORT, EM1_DEBUG_PIN, gpioModePushPull,  0 },
  { EM2_DEBUG_PORT, EM2_DEBUG_PIN, gpioModePushPull,  0 },
};

/***************************************************************************//**
 * @brief
 *   This is for debugging the low power states to determine how many times
 *   the various low power states were entered.
 *
 ******************************************************************************/
void low_power_init(void)
{
	force_em0 = 0;
	wait_em1_count = 0;
	wait_em2_count = 0;
	gpio_init(lp_debug_pins, sizeof(lp_debug_pins)/sizeof(gpio_init_t));
}

/** @} (end addtogroup Peripheral Functions) */
/** @} (end addtogroup Low_Power) */

