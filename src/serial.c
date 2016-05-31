
/****************************************************************************//**
 * @file serial.c
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

#include <stdio.h>

#include "em_cmu.h"
#include "em_emu.h"

// NOTE headers em_leuart.h, em_uart.h, em_usart.h conditionally included later

#include "buffer.h"
#include "gpio.h"
#include "serial.h"

/***************************************************************************//**
 * @addtogroup Peripheral_Functions
 * @{
 ******************************************************************************/
/***************************************************************************//**
 * @addtogroup LEUART
 * @brief UART definitions and functions.
 * @{
 ******************************************************************************/

#if (defined(LEUART_NUM) + defined(UART_NUM) + defined(USART_NUM)) != 1
#  error "In serial.h, exactly one of LEUART_NUM, UART_NUM, USART_NUM must be defined."
#endif

#define PASTE_(A, B) A##B
#define PASTE(A, B) PASTE_(A, B)

#define PASTE3_(A, B, C) A##B##C
#define PASTE3(A, B, C) PASTE3_(A, B, C)

/**************************************************************************//**
 * @verbatim
 * 	The defines below are intended to allow any Leopard Gecko or Giant Gecko
 *	LEUART, UART, or USART number and location to be used, but only LEUART0 location 0
 *	has been tested.
 *
 *	Define only one of LEUART_NUM, UART_NUM, or USART_NUM, depending on
 *	which kind of hardware port is to be used, with a numeric value
 *	corresponding to which instance of that kind of hardware port is
 *	to be used.
 *
 *	SERIAL_LOC specifies which set of GPIO pins the serial port should
 *  use, of the choices available for the selected hardware port.
 * @endverbatim
*************************************************************************/

#if defined(LEUART_NUM)
#  include "em_leuart.h"
#  define SPORT                       PASTE(LEUART, LEUART_NUM)
#  define SERIAL_ROUTE_TXPEN          LEUART_ROUTE_TXPEN
#  define SERIAL_ROUTE_RXPEN          LEUART_ROUTE_RXPEN
#  define SERIAL_ROUTE_LOCATION_SHIFT _LEUART_ROUTE_LOCATION_SHIFT
#  define SERIAL_TX_IRQn              PASTE3(LEUART, LEUART_NUM, _IRQn)
#  define SERIAL_RX_IRQn              PASTE3(LEUART, LEUART_NUM, _IRQn)
#  define SERIAL_TX_IRQHandler        PASTE3(LEUART, LEUART_NUM, _TX_IRQHandler)
#  define SERIAL_RX_IRQHandler        PASTE3(LEUART, LEUART_NUM, _RX_IRQHandler)
#  define SERIAL_IRQHandler           PASTE3(LEUART, LEUART_NUM, _IRQHandler)
#  define serial_cmuclock1            cmuClock_CORELE
#  define serial_cmuclock2            PASTE(cmuClock_LEUART, LEUART_NUM)
#  define serial_reset                LEUART_Reset
#  define serial_initasync            LEUART_Init
#  define serial_baudrateset          LEUART_BaudrateSet
#  define serial_intclear             LEUART_IntClear
#  define serial_intenable            LEUART_IntEnable
#  define STATUS_TXC  LEUART_STATUS_TXC
#  define IF_RXDATAV  LEUART_IF_RXDATAV
#  define IF_TXBL     LEUART_IF_TXBL
#  define IEN_RXDATAV LEUART_IEN_RXDATAV
#  define IEN_TXBL    LEUART_IEN_TXBL
#  define INIT_STRUCT LEUART_Init_Typedef
#  if LEUART_NUM == 0
#    if SERIAL_LOC == 0
#      define RX_PORT gpioPortD
#      define RX_PIN  5
#      define TX_PORT gpioPortD
#      define TX_PIN  4
/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
#    elif SERIAL_LOC == 1
#      define RX_PORT gpioPortB
#      define RX_PIN  14
#      define TX_PORT gpioPortB
#      define TX_PIN  13
#    elif SERIAL_LOC == 2
#      define RX_PORT gpioPortE
#      define RX_PIN  15
#      define TX_PORT gpioPortE
#      define TX_PIN  14
#    elif SERIAL_LOC == 3
#      define RX_PORT gpioPortF
#      define RX_PIN  1
#      define TX_PORT gpioPortF
#      define TX_PIN  0
#    elif SERIAL_LOC == 4
#      define RX_PORT gpioPortA
#      define RX_PIN  0
#      define TX_PORT gpioPortF
#      define TX_PIN  2
#    else
#      error "invalid LEUART_LOC for LEUART_NUM==0"
#    endif
#  elif LEUART_NUM == 1
#    if SERIAL_LOC == 0
#      define RX_PORT gpioPortC
#      define RX_PIN  7
#      define TX_PORT gpioPortC
#      define TX_PIN  6
#    elif SERIAL_LOC == 1
#      define RX_PORT gpioPortA
#      define RX_PIN  6
#      define TX_PORT gpioPortA
#      define TX_PIN  5
#    else
#      error "invalid LEUART_LOC for LEUART_NUM==1"
#    endif
#  else
#    error "invalid LEUART_NUM"
#  endif
/** @endcond */
#endif

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
#if defined(UART_NUM)
#  include "em_usart.h"
#  define SPORT                       PASTE(UART, UART_NUM)
#  define SERIAL_ROUTE_TXPEN          UART_ROUTE_TXPEN
#  define SERIAL_ROUTE_RXPEN          UART_ROUTE_RXPEN
#  define SERIAL_ROUTE_LOCATION_SHIFT _UART_ROUTE_LOCATION_SHIFT
#  define SERIAL_TX_IRQn              PASTE3(UART, UART_NUM, _TX_IRQn)
#  define SERIAL_RX_IRQn              PASTE3(UART, UART_NUM, _RX_IRQn)
#  define SERIAL_TX_IRQHandler        PASTE3(UART, UART_NUM, _TX_IRQHandler)
#  define SERIAL_RX_IRQHandler        PASTE3(UART, UART_NUM, _RX_IRQHandler)
#  define serial_cmuclock2            PASTE(cmuClock_UART, UART_NUM)
#  define serial_reset                USART_Reset
#  define serial_initasync            USART_InitAsync
#  define serial_baudrateset          USART_BaudrateAsyncSet
#  define serial_intclear             USART_IntClear
#  define serial_intenable            USART_IntEnable
#  define STATUS_TXC  UART_STATUS_TXC
#  define IF_RXDATAV  UART_IF_RXDATAV
#  define IF_TXBL     UART_IF_TXBL
#  define IEN_RXDATAV UART_IEN_RXDATAV
#  define IEN_TXBL    UART_IEN_TXBL
#  if UART_NUM == 0
#    if SERIAL_LOC == 0
#      define RX_PORT gpioPortF
#      define RX_PIN  7
#      define TX_PORT gpioPortF
#      define TX_PIN  6
#    elif SERIAL_LOC == 1
#      define RX_PORT gpioPortE
#      define RX_PIN  1
#      define TX_PORT gpioPortE
#      define TX_PIN  0
#    elif SERIAL_LOC == 2
#      define RX_PORT gpioPortA
#      define RX_PIN  4
#      define TX_PORT gpioPortA
#      define TX_PIN  3
#    elif SERIAL_LOC == 3
#      define RX_PORT gpioPortC
#      define RX_PIN  15
#      define TX_PORT gpioPortC
#      define TX_PIN  14
#    else
#      error "invalid UART_LOC for UART_NUM==0"
#    endif
#  elif UART_NUM == 1
#    if SERIAL_LOC == 0
#      define RX_PORT gpioPortC
#      define RX_PIN  13
#      define TX_PORT gpioPortC
#      define TX_PIN  12
#    elif SERIAL_LOC == 1
#      define RX_PORT gpioPortF
#      define RX_PIN  11
#      define TX_PORT gpioPortF
#      define TX_PIN  10
#    elif SERIAL_LOC == 2
#      define RX_PORT gpioPortB
#      define RX_PIN  10
#      define TX_PORT gpioPortB
#      define TX_PIN  9
#    elif SERIAL_LOC == 3
#      define RX_PORT gpioPortE
#      define RX_PIN  3
#      define TX_PORT gpioPortE
#      define TX_PIN  2
#    else
#      error "invalid UART_LOC for UART_NUM==1"
#    endif
#  else
#    error "invalid LEUART_NUM"
#  endif
#endif

#if defined(USART_NUM)
#  include "em_usart.h"
#  define SPORT                       PASTE(USART, USART_NUM)
#  define SERIAL_ROUTE_TXPEN          USART_ROUTE_TXPEN
#  define SERIAL_ROUTE_RXPEN          USART_ROUTE_RXPEN
#  define SERIAL_ROUTE_LOCATION_SHIFT _USART_ROUTE_LOCATION_SHIFT
#  define SERIAL_TX_IRQn              PASTE3(USART, USART_NUM, _TX_IRQn)
#  define SERIAL_RX_IRQn              PASTE3(USART, USART_NUM, _RX_IRQn)
#  define SERIAL_TX_IRQHandler        PASTE3(USART, USART_NUM, _TX_IRQHandler)
#  define SERIAL_RX_IRQHandler        PASTE3(USART, USART_NUM, _RX_IRQHandler)
#  define serial_cmuclock2            PASTE(cmuClock_USART, USART_NUM)
#  define serial_reset                USART_Reset
#  define serial_initasync            USART_InitAsync
#  define serial_baudrateset          USART_BaudrateAsyncSet
#  define serial_intclear             USART_IntClear
#  define serial_intenable            USART_IntEnable
#  define STATUS_TXC  USART_STATUS_TXC
#  define IF_RXDATAV  USART_IF_RXDATAV
#  define IF_TXBL     USART_IF_TXBL
#  define IEN_RXDATAV USART_IEN_RXDATAV
#  define IEN_TXBL    USART_IEN_TXBL
#  if USART_NUM == 0
#    define serial_cmuClock cmuClock_USART0
#    if SERIAL_LOC == 0
#      define RX_PORT gpioPortE
#      define RX_PIN  11
#      define TX_PORT gpioPortE
#      define TX_PIN  10
#    elif SERIAL_LOC == 1
#      define RX_PORT gpioPortE
#      define RX_PIN  6
#      define TX_PORT gpioPortE
#      define TX_PIN  7
#    elif SERIAL_LOC == 2
#      define RX_PORT gpioPortC
#      define RX_PIN  10
#      define TX_PORT gpioPortC
#      define TX_PIN  11
#    elif SERIAL_LOC == 3
#      define RX_PORT gpioPortE
#      define RX_PIN  12
#      define TX_PORT gpioPortE
#      define TX_PIN  13
#    elif SERIAL_LOC == 4
#      define RX_PORT gpioPortB
#      define RX_PIN  8
#      define TX_PORT gpioPortB
#      define TX_PIN  7
#    elif SERIAL_LOC == 5
#      define RX_PORT gpioPortC
#      define RX_PIN  1
#      define TX_PORT gpioPortC
#      define TX_PIN  0
#    else
#      error "invalid USART_LOC for USART_NUM==0"
#    endif
#  elif USART_NUM == 1
#    define serial_cmuClock cmuClock_USART1
#    if SERIAL_LOC == 0
#      define RX_PORT gpioPortC
#      define RX_PIN  1
#      define TX_PORT gpioPortC
#      define TX_PIN  0
#    elif SERIAL_LOC == 1
#      define RX_PORT gpioPortD
#      define RX_PIN  1
#      define TX_PORT gpioPortD
#      define TX_PIN  0
#    elif SERIAL_LOC == 2
#      define RX_PORT gpioPortD
#      define RX_PIN  6
#      define TX_PORT gpioPortD
#      define TX_PIN  7
#    else
#      error "invalid USART_LOC for USART_NUM==1"
#    endif
#  elif USART_NUM == 2
#    define serial_cmuClock cmuClock_USART2
#    if SERIAL_LOC == 0
#      define RX_PORT gpioPortC
#      define RX_PIN  3
#      define TX_PORT gpioPortC
#      define TX_PIN  2
#    elif SERIAL_LOC == 1
#      define RX_PORT gpioPortB
#      define RX_PIN  4
#      define TX_PORT gpioPortB
#      define TX_PIN  3
#    else
#      error "invalid USART_LOC for USART_NUM==2"
#    endif
#  else
#    error "invalid USART_NUM"
#  endif
#endif
/** @endcond */

static buf_t *serial_tx_buf;
static buf_t *serial_rx_buf;

/***************************************************************************//**
 * @brief
 *   Serial RX IRQ Handler
 *
 ******************************************************************************/
void SERIAL_RX_IRQHandler(void)
{
	if (SPORT->IF & IF_RXDATAV)
	{
		uint8_t data = SPORT->RXDATA;
		buf_write_byte(serial_rx_buf, data);
		if (buf_full(serial_rx_buf))
			SPORT->IEN &= ~ IEN_RXDATAV;  // disable rx interrupt
	}
}

/***************************************************************************//**
 * @brief
 *   Serial TX IRQ Handler
 *
 ******************************************************************************/
void SERIAL_TX_IRQHandler(void)
{
	if (SPORT->IF & IF_TXBL)
	{
		uint8_t data;
		if (buf_read_byte(serial_tx_buf, & data))
			SPORT->TXDATA = data;
		else
			SPORT->IEN &= ~ IEN_TXBL;  // disable tx interrupt
	}
}

#if defined(SERIAL_IRQHandler)
void SERIAL_IRQHandler(void)
{
	SERIAL_RX_IRQHandler();
	SERIAL_TX_IRQHandler();
}
#endif

/***************************************************************************//**
 * @brief
 *   Write Character to Serial Port - Blocking
 * @param[in] c
 * 		Character to write
 *
 ******************************************************************************/
void serial_blocking_write_char(char c)
{
	while (! buf_write_char(serial_tx_buf, c))
	{
		;
	}
	SPORT->IEN |= IEN_TXBL;  // enable tx if not already going
}

/***************************************************************************//**
 * @brief
 *   Write String to Serial Port - Blocking
 * @param[in] *p
 * 		Pointer to String to write
 *
 ******************************************************************************/
void serial_blocking_write_str(const char *p)
{
	while (*p)
		serial_blocking_write_char(*p++);
}

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
#if 0
int serial_printf(const char *fmt, ...)
{
	va_list ap;
	int n;
	char spbuf[300];

	va_start(ap, fmt);
	n = vsnprintf(spbuf, sizeof(spbuf), fmt, ap);
	va_end(ap);

	serial_blocking_write_str(spbuf);
	return n;
}
#endif
/** @endcond */

const gpio_init_t serial_pins[] =
{
	{ TX_PORT, TX_PIN, gpioModePushPull,  1 },
	{ RX_PORT, RX_PIN, gpioModeInputPull, 1 },
};

#if defined(LEUART_NUM)
LEUART_Init_TypeDef sportInit =
{
  .enable       = leuartEnableRx | leuartEnableTx,
  .refFreq      = 0,
  .baudrate     = 9600,
};
#endif

/** @cond DO_NOT_INCLUDE_WITH_DOXYGEN */
#if defined(UART_NUM) || defined(USART_NUM)
USART_InitAsync_TypeDef sportInit =
{
  .enable       = usartEnableRx | usartEnableTx,
  .refFreq      = 0,
  .baudrate     = 115200,
  .oversampling = usartOVS16,
  .databits     = usartDatabits8,
  .parity       = usartNoParity,
  .stopbits     = usartStopbits1,
  .mvdis        = false,          /* Disable majority voting */
  .prsRxEnable  = false,          /* Enable USART Rx via Peripheral Reflex System */
  .prsRxCh      = usartPrsRxCh0,  /* Select PRS channel if enabled */
};
#endif
/** @endcond */

/***************************************************************************//**
 * @brief
 *   Reset Serial Port
 *
 ******************************************************************************/
void serial_close(void)
{
	serial_reset(SPORT);
}

/***************************************************************************//**
 * @brief
 *   Initialize Serial Port and TX/RX Buffers
 *  @param[in] bit_rate
 *  	Baud Rate
 *  @param[in] *raw_rx_buf
 *  	Pointer to Receive Buffer
 *  @param[in] raw_rx_buf_size
 *  	Size of RX buffer
 *  @param[in] *raw_tx_buf
 *  	Pointer to Transmit Buffer
 *  @param[in] raw_tx_buf_size
 *  	Size of TX buffer
 *
 ******************************************************************************/
void serial_init(int bit_rate,
		         uint8_t *raw_rx_buf,
		         size_t raw_rx_buf_size,
		         uint8_t *raw_tx_buf,
		         size_t raw_tx_buf_size)
{
	serial_rx_buf = init_buf(raw_rx_buf, raw_rx_buf_size);
	serial_tx_buf = init_buf(raw_tx_buf, raw_tx_buf_size);

	CMU_ClockEnable(cmuClock_GPIO, true);
#ifdef serial_cmuclock1
	CMU_ClockEnable(serial_cmuclock1, true);
#endif
	CMU_ClockEnable(serial_cmuclock2, true);

	gpio_init(serial_pins, sizeof(serial_pins)/sizeof(gpio_init_t));

	serial_reset(SPORT);
	serial_initasync(SPORT, &sportInit);
	SPORT->ROUTE = SERIAL_ROUTE_TXPEN | SERIAL_ROUTE_RXPEN | (SERIAL_LOC << SERIAL_ROUTE_LOCATION_SHIFT);

	if (bit_rate)
#if defined(LEUART_NUM)
		serial_baudrateset(SPORT, 0, bit_rate);
#else
		serial_baudrateset(SPORT, 0, bit_rate, usartOVS16);
#endif

	serial_intclear(SPORT, IF_RXDATAV | IF_TXBL);
	serial_intenable(SPORT, IF_RXDATAV);

	NVIC_ClearPendingIRQ(SERIAL_RX_IRQn);
	NVIC_EnableIRQ(SERIAL_RX_IRQn);

#if SERIAL_RX_IRQn != SERIAL_TX_IRQn
	NVIC_ClearPendingIRQ(SERIAL_TX_IRQn);
	NVIC_EnableIRQ(SERIAL_TX_IRQn);
#endif
}

/***************************************************************************//**
 * @brief
 *   Clear Transmit Buffer
 *
 ******************************************************************************/
void serial_tx_flush(void)
{
	while (! buf_empty(serial_tx_buf))
		;
}

/***************************************************************************//**
 * @brief
 *   Write Character to Serial Port
 * @param[in] c
 * 		Character to write
 *
 ******************************************************************************/
int RETARGET_WriteChar(char c)
{
	serial_blocking_write_char(c);
	return 0;
}

/***************************************************************************//**
 * @brief
 *   Read Character from Serial Port
 *
 ******************************************************************************/
int RETARGET_ReadChar(void)
{
	return 0;
}

/** @} (end addtogroup LEUART) */
/** @} (end addtogroup Peripheral Functions) */
