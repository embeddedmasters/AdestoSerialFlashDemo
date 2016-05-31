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

#include <inttypes.h>

#include "em_cmu.h"
#include "em_emu.h"

#include "rtcdriver.h"

#include "oneshot.h"
#include "fatal.h"

/***************************************************************************//**
 * @addtogroup AppManagement
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup OneShot_Timer
 * @brief Buffer definitions, initializations, and handler routines
 * @{
 ******************************************************************************/

#define RTC_DIVIDER                     ( 32 )

#define RTC_CLOCK                       ( 32768U )
#define MSEC_TO_TICKS_DIVIDER           ( 1000U * RTC_DIVIDER )
#define MSEC_TO_TICKS_ROUNDING_FACTOR   ( MSEC_TO_TICKS_DIVIDER / 2 )
#define MSEC_TO_TICKS( ms )             ( ( ( (uint64_t)(ms) * RTC_CLOCK )    \
                                            + MSEC_TO_TICKS_ROUNDING_FACTOR ) \
                                          / MSEC_TO_TICKS_DIVIDER )

static RTCDRV_TimerID_t oneshot_timer;

/***************************************************************************//**
 * @brief
 *   Create OneShot/RTC Timer instance
 *
 ******************************************************************************/
void oneshot_init(void)
{
	if (ECODE_EMDRV_RTCDRV_OK != RTCDRV_AllocateTimer(& oneshot_timer))
		fatal("can't allocate oneshot timer");
}

static volatile bool oneshot_done_flag;

/***************************************************************************//**
 * @brief
 *   Callback function for OneShot/RTC timer
 * @param[in] id
 * 		Instance of RTC/Oneshot timer
 * @param[in] *user
 *
 ******************************************************************************/
static void oneshot_callback(RTCDRV_TimerID_t id, void *user)
{
	oneshot_done_flag = true;
}

/***************************************************************************//**
 * @brief
 *   Oneshot initialization and start
 * @param[in] s
 * 		32bit Time value in seconds
 *
 ******************************************************************************/
void oneshot_start_s(uint32_t s)
{
	uint32_t ticks = MSEC_TO_TICKS(1000 * s);

	oneshot_done_flag = false;

	if (ECODE_EMDRV_RTCDRV_OK != RTCDRV_StartTimer(oneshot_timer,
												   rtcdrvTimerTypeOneshot,
												   ticks,
												   oneshot_callback,
												   0))
		fatal("can't initialize oneshot timer");
}

/***************************************************************************//**
 * @brief
 *   Indicates whether OneShot/RTC timer has completed
 * @return oneshot_done_flag
 * 		Flag indicating whether OneShot/RTC timer has expired, TRUE/FALSE.
 *
 ******************************************************************************/
bool oneshot_done(void)
{
	return oneshot_done_flag;
}

/** @} (end addtogroup Peripheral Functions) */
/** @} (end addtogroup Buttons) */
