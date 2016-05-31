/****************************************************************************//**
 * @file buffer.c
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

#include <string.h>
#include "em_int.h"
#include "buffer.h"


/***************************************************************************//**
 * @addtogroup AppManagement
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup Buffer
 * @brief Buffer definitions, initializations, and handler routines
 * @{
 ******************************************************************************/

typedef struct buf
{
  size_t size;
  volatile size_t count;
  size_t write_idx;
  size_t read_idx;
  uint8_t data[0];
} buf_t;

/****************************************************************************//**
 * @brief  Buffer Initialization
 *
 * @param[in] *raw_buf
 * 		Pointer to buffer
 * @param[in] size
 * 		Size of Buffer
 * @return buf
 *
 *****************************************************************************/
buf_t *init_buf(uint8_t *raw_buf, size_t size)
{
	buf_t *buf = (buf_t *) raw_buf;
	memset(buf, 0, size);
	buf->size = size - sizeof(buf_t);
	buf->count = 0;
	buf->write_idx = 0;
	buf->read_idx = 0;
	return buf;
}

/****************************************************************************//**
 * @brief Checks if buffer is full
 *
 * @param[in] *buf
 * 		Pointer to buffer
 * @return TRUE/FALSE
 *
 *****************************************************************************/
bool buf_full(buf_t *buf)
{
	return buf->count == buf->size;
}

/****************************************************************************//**
 * @brief Checks if buffer is empty
 *
 * @param[in] *buf
 * 		Pointer to buffer
 * @return TRUE/FALSE
 *
 *****************************************************************************/
bool buf_empty(buf_t *buf)
{
	return buf->count == 0;
}

/****************************************************************************//**
 * @brief  Buffer Write Byte
 *
 * @param[in] *buf
 * 		Pointer to buffer
 * @param[in] data
 * 		Byte/data to write
 * @return true if successful
 *
 *****************************************************************************/
bool buf_write_byte(buf_t *buf, const uint8_t data)
{
	if (buf->count >= buf->size)
		return false;  // no room
	buf->data[buf->write_idx++] = data;
	if (buf->write_idx >= buf->size)
		buf->write_idx = 0;
	INT_Disable();
	buf->count++;
	INT_Enable();
	return true;
}

/****************************************************************************//**
 * @brief  Buffer Read Byte
 *
 * @param[in] *buf
 * 		Pointer to buffer
 * @param[in] *data
 * 		Pointer to data variable to write to
 * @return true if successful
 *
 *****************************************************************************/
bool buf_read_byte(buf_t *buf, uint8_t *data)
{
	if (buf->count == 0)
		return false;  // no data
	*data = buf->data[buf->read_idx++];
	if (buf->read_idx >= buf->size)
		buf->read_idx = 0;
	INT_Disable();
	buf->count--;
	INT_Enable();
	return true;
}

/****************************************************************************//**
 * @brief  Buffer Write Char
 *
 * @param[in] *buf
 * 		Pointer to buffer
 * @param[in] c
 * 		Char to write
 * @return true if successful
 *
 *****************************************************************************/
bool buf_write_char(buf_t *buf, const char c)
{
  return buf_write_byte(buf, (uint8_t) c);
}

/****************************************************************************//**
 * @brief  Buffer Read Char
 *
 * @param[in] *buf
 * 		Pointer to buffer
 * @param[in] *c
 * 		Pointer to char to store read char
 * @return true if successful
 *
 *****************************************************************************/
bool buf_read_char(buf_t *buf, char *c)
{
	return buf_read_byte(buf, (uint8_t *) c);
}

/** @} (end addtogroup Buffer) */
/** @} (end addtogroup AppManagement) */
