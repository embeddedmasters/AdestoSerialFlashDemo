/****************************************************************************//**
 * @file button.h
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

#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdbool.h>

/*****************************************************************************/
/** @defgroup Peripheral_Functions Peripheral_Functions
*/
/*****************************************************************************/

/***************************************************************************//**
 * @addtogroup Peripheral_Functions
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @defgroup Buttons
 * @{
 ******************************************************************************/

typedef void button_callback_fn_t(int button, bool pressed);

void button_init(button_callback_fn_t *callback);

/** @} (end addtogroup Peripheral Functions) */
/** @} (end addtogroup Buttons) */


#endif /* BUTTON_H_ */
