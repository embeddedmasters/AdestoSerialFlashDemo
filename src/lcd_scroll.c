/******************************************************************************
 * @file lcd_scrolls.c
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

#include "rtcdriver.h"
#include "segmentlcd.h"

#include "lcdtest.h"
#include "lcd_scroll.h"

/***************************************************************************//**
 * @addtogroup LCD_Functions
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup LCD_Scroll
 * @{
 ******************************************************************************/

static int lcd_scroll_length;
static int lcd_scroll_position;
static const char *lcd_scroll_message;

/***************************************************************************//**
* @brief
*	Start LCD Scroll of Character String
*
* @param[in] *s
* 		Pointer to Character String
*
******************************************************************************/
void lcd_scroll_start(const char *s)
{
	lcd_scroll_length = strlen(s);

	if (lcd_scroll_length < 7)
	{
		SegmentLCD_Write((char *) s);
		lcd_scroll_length = 0;
	}
	else
	{
		lcd_scroll_message = s;
		lcd_scroll_position = 0;
		lcd_scroll_update();
	}
}

/***************************************************************************//**
* @brief
*	Update LCD Display
*
******************************************************************************/
void lcd_scroll_update(void)
{
	if (! lcd_scroll_length)
		return;

	SegmentLCD_Write((char *) (lcd_scroll_message + lcd_scroll_position));
	lcd_scroll_position++;
	if (lcd_scroll_position >= lcd_scroll_length)
		lcd_scroll_position = 0;
}

/***************************************************************************//**
* @brief
*	Stop LCD Scroll
*
******************************************************************************/
void lcd_scroll_stop(void)
{
	lcd_scroll_length = 0;
}
