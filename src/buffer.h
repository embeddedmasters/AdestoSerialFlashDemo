/****************************************************************************//**
 * @file buffer.h
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
#ifndef BUFFER_H_
#define BUFFER_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

/*****************************************************************************/
/** @defgroup AppManagement AppManagement
 */
/*****************************************************************************/

/***************************************************************************//**
 * @addtogroup AppManagement
 * @{
 ******************************************************************************/

/**************************************************************************//**
 * @defgroup Buffer
 * @{
 *****************************************************************************/

typedef struct buf buf_t;

buf_t *init_buf(uint8_t *raw_buf, size_t size);

bool buf_full(buf_t *buf);

bool buf_empty(buf_t *buf);

bool buf_write_byte(buf_t *buf, const uint8_t data);

bool buf_read_byte(buf_t *buf, uint8_t *data);

bool buf_write_char(buf_t *buf, const char c);

bool buf_read_char(buf_t *buf, char *c);

/** @} (end addtogroup Buffer) */
/** @} (end addtogroup AppManagement) */
#endif /* BUFFER_H_ */
