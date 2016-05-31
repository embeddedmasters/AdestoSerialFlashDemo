/******************************************************************************
 * @file lcd_scroll.h
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

#ifndef LCD_SCROLL_H_
#define LCD_SCROLL_H_

/***************************************************************************//**
 * @addtogroup LCD_Functions
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup LCD_Scroll
 * @{
 ******************************************************************************/

void lcd_scroll_start(const char *s);

void lcd_scroll_update(void);

void lcd_scroll_stop(void);

/** @} (end addtogroup LCD_Scroll) */
/** @} (end addtogroup LCD_Functions) */


#endif /* LCD_SCROLL_H_ */
