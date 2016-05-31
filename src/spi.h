/****************************************************************************//**
 * @file spi.h
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

#ifndef SPI_H_
#define SPI_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

/***************************************************************************//**
 * @addtogroup Peripheral_Functions
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @defgroup SPI SPI
 * @{
 ******************************************************************************/

#define USART_NUM 1
#define USART_LOC 1


bool spi_active(void);

// Some serial flash parts support an Active Status Interrupt on their
// SPI data output signal. Define USE_SO_IRQ to 1 to enable using that
// feature, or 0 otherwise.
#define USE_SO_IRQ 1

typedef void spi_completion_fn_t(void *ref);

void spi_xfer(size_t tx_len,
			  const uint8_t *tx_data,
			  size_t tx2_len,
			  const uint8_t *tx2_data,
			  bool half_duplex, // if true, doesn't read data returned when transmitting
			  size_t rx_len,
			  uint8_t *rx_data,
			  bool hold_cs_active,
			  spi_completion_fn_t *completion,  // completion callback fn
			  void *completion_ref);  // argument to be passed to completion callback


#ifdef USE_SO_IRQ
void spi_wait_so(uint8_t level,
		         spi_completion_fn_t *completion,  // completion callback fn
		         void *completion_ref);  // argument to be passed to completion callback
#endif


void spi_init(int bit_rate);

/** @} (end addtogroup Peripheral Functions) */
/** @} (end addtogroup SPI) */


#endif /* SPI_H_ */
