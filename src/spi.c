/****************************************************************************//**
 * @file spi.c
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
#include "em_cmu.h"
#include "em_emu.h"
#include "em_int.h"
#include "em_usart.h"

#include "gpio.h"
#include "low_power.h"
#include "spi.h"

/***************************************************************************//**
 * @addtogroup Peripheral_Functions
 * @{
 ******************************************************************************/

/***************************************************************************//**
 * @addtogroup SPI
 * @{
 ******************************************************************************/

#define PASTE_(A, B) A##B
#define PASTE(A, B) PASTE_(A, B)

#define PASTE3_(A, B, C) A##B##C
#define PASTE3(A, B, C) PASTE3_(A, B, C)

/**************************************************************************//**
 * @verbatim
 *  The defines below are intended to allow any Leopard Gecko or Giant Gecko
 *  USART number and location to be used, but only USART1 location 1 has been
 *  tested.
 *  @endverbatim
*************************************************************************/
#define SPI_PORT             PASTE(USART, USART_NUM)
#define SPI_TX_IRQn          PASTE3(USART, USART_NUM, _TX_IRQn)
#define SPI_RX_IRQn          PASTE3(USART, USART_NUM, _RX_IRQn)
#define SERIAL_TX_IRQHandler PASTE3(USART, USART_NUM, _TX_IRQHandler
#define SERIAL_RX_IRQHandler PASTE3(USART, USART_NUM, _RX_IRQHandler
#define SPI_cmuClock         PASTE(cmuClock_USART, USART_NUM)

#if USART_NUM == 0
#  if USART_LOC == 0
#    define CK_PORT gpioPortE
#    define CK_PIN  12
#    define CS_PORT gpioPortE
#    define CS_PIN  13
#    define RX_PORT gpioPortE
#    define RX_PIN  11
#    define TX_PORT gpioPortE
#    define TX_PIN  10
#  elif USART_LOC == 1
#    define CK_PORT gpioPortE
#    define CK_PIN  5
#    define CS_PORT gpioPortE
#    define CS_PIN  4
#    define RX_PORT gpioPortE
#    define RX_PIN  6
#    define TX_PORT gpioPortE
#    define TX_PIN  7
#  elif USART_LOC == 2
#    define CK_PORT gpioPortC
#    define CK_PIN  9
#    define CS_PORT gpioPortC
#    define CS_PIN  8
#    define RX_PORT gpioPortC
#    define RX_PIN  10
#    define TX_PORT gpioPortC
#    define TX_PIN  11
#  elif USART_LOC == 3
#    define CK_PORT gpioPortC
#    define CK_PIN  15
#    define CS_PORT gpioPortC
#    define CS_PIN  14
#    define RX_PORT gpioPortE
#    define RX_PIN  12
#    define TX_PORT gpioPortE
#    define TX_PIN  13
#  elif USART_LOC == 4
#    define CK_PORT gpioPortB
#    define CK_PIN  13
#    define CS_PORT gpioPortB
#    define CS_PIN  14
#    define RX_PORT gpioPortB
#    define RX_PIN  8
#    define TX_PORT gpioPortB
#    define TX_PIN  7
#  elif USART_LOC == 5
#    define CK_PORT gpioPortB
#    define CK_PIN  13
#    define CS_PORT gpioPortB
#    define CS_PIN  14
#    define RX_PORT gpioPortC
#    define RX_PIN  1
#    define TX_PORT gpioPortC
#    define TX_PIN  0
#  else
#    error "invalid USART_LOC for USART_NUM==0"
#  endif
#elif USART_NUM == 1
#  if USART_LOC == 0
#    define CK_PORT gpioPortB
#    define CK_PIN  7
#    define CS_PORT gpioPortB
#    define CS_PIN  8
#    define RX_PORT gpioPortC
#    define RX_PIN  1
#    define TX_PORT gpioPortC
#    define TX_PIN  0
#  elif USART_LOC == 1
#    define CK_PORT gpioPortD
#    define CK_PIN  2
#    define CS_PORT gpioPortD
#    define CS_PIN  3
#    define RX_PORT gpioPortD
#    define RX_PIN  1
#    define TX_PORT gpioPortD
#    define TX_PIN  0
#  elif USART_LOC == 2
#    define CK_PORT gpioPortF
#    define CK_PIN  0
#    define CS_PORT gpioPortF
#    define CS_PIN  1
#    define RX_PORT gpioPortD
#    define RX_PIN  6
#    define TX_PORT gpioPortD
#    define TX_PIN  7
#  else
#    error "invalid USART_LOC for USART_NUM==1"
#  endif
#elif USART_NUM == 2
#  if USART_LOC == 0
#    define CK_PORT gpioPortC
#    define CK_PIN  4
#    define CS_PORT gpioPortC
#    define CS_PIN  5
#    define RX_PORT gpioPortC
#    define RX_PIN  3
#    define TX_PORT gpioPortC
#    define TX_PIN  2
#  elif USART_LOC == 1
#    define CK_PORT gpioPortB
#    define CK_PIN  5
#    define CS_PORT gpioPortB
#    define CS_PIN  6
#    define RX_PORT gpioPortB
#    define RX_PIN  4
#    define TX_PORT gpioPortB
#    define TX_PIN  3
#  else
#    error "invalid USART_LOC for USART_NUM==2"
#  endif
#else
#  error "invalid USART_NUM"
#endif


static bool spi_busy;
static spi_completion_fn_t *spi_completion;
static void *spi_completion_ref;

static size_t spi_tx_len;
static const uint8_t *spi_tx_data;
static size_t spi_tx2_len;
static const uint8_t *spi_tx2_data;
static size_t spi_tx_post_padding_len;

static size_t spi_rx_pre_padding_len;
static size_t spi_rx_len;
static uint8_t *spi_rx_data;
static size_t spi_rx_post_padding_len;

static bool spi_hold_cs_active;

/***************************************************************************//**
 * @brief
 *   Called to determine if SPI bus is active
 * @note
 *		Typically used to determine if Low Power/EM Modes can be entered.
 * @return spi_busy
 * 		TRUE/FALSE
 *
 ******************************************************************************/
bool spi_active(void)
{
	return spi_busy;
}


/***************************************************************************//**
 * @brief
 *   USART/SPI1 RX Interrupt Handler
 * @note
 * 		Copies Received data in spi_rx_data buffer.
 *
 ******************************************************************************/
void USART1_RX_IRQHandler(void)
{
	uint8_t dummy;
	if (SPI_PORT->IF & USART_IF_RXDATAV)
	{
		if (spi_rx_pre_padding_len)
		{
			dummy = SPI_PORT->RXDATA;
			(void) dummy;
			spi_rx_pre_padding_len--;
		}
		else if (spi_rx_len)
		{
			*spi_rx_data++ = SPI_PORT->RXDATA;
			spi_rx_len--;
		}
		else if (spi_rx_post_padding_len)
		{
			dummy = SPI_PORT->RXDATA;
			(void) dummy;
			spi_rx_post_padding_len--;
		}
		if ((spi_rx_pre_padding_len == 0) && (spi_rx_len == 0) && (spi_rx_post_padding_len == 0))
		{
			// copy completion fn ptr and ref arg, to avoid race condition
			// if completion fn starts another SPI xfer
			spi_completion_fn_t *completion = spi_completion;
			void *completion_ref = spi_completion_ref;

			SPI_PORT->IEN &= ~ USART_IEN_RXDATAV;  // disable rx interrupt
			if (! spi_hold_cs_active)
				GPIO_PinOutSet(CS_PORT, CS_PIN);  // deassert CS
			spi_busy = false;
			if (completion)
				completion(completion_ref);
		}
	}
}

/***************************************************************************//**
 * @brief
 *   USART/SPI1 RX Interrupt Handler
 * @note
 * 		Copies TX data into spi_tx_data buffer.
 ******************************************************************************/
void USART1_TX_IRQHandler(void)
{
	if (SPI_PORT->IF & USART_IF_TXBL)
	{
		if (spi_tx_len)
		{
			SPI_PORT->TXDATA = *spi_tx_data++;
			spi_tx_len--;
		}
		else if (spi_tx2_len)
		{
			SPI_PORT->TXDATA = *spi_tx2_data++;
			spi_tx2_len--;
		}
		else if (spi_tx_post_padding_len)
		{
			SPI_PORT->TXDATA = 0x00;  // still more to rx, so tx dummy byte
			spi_tx_post_padding_len--;
		}
		if((spi_tx_len == 0) && (spi_tx2_len == 0) && (spi_tx_post_padding_len == 0))
			SPI_PORT->IEN &= ~ USART_IEN_TXBL;  // disable tx interrupt
	}
}

/***************************************************************************//**
 * @brief
 *   SPI transfer function
 *
 * @details
 * 		Performs one SPI transaction. Fundamentally SPI does a write and read of the
 * 		same length (byte count) at the same time. Often only a few bytes are to be
 *		written, and the data returned while those bytes are written is to be
 *		discarded. This can be done using the "half_duplex" flag.  The data to be
 *		written may optionally be split into two separate buffers in order to handle
 *		discontiguous data. Set tx_len or tx_len2 to 0 if the corresponding buffer
 *		is not used.  Will block if NULL passed for completion function; otherwise
 *		will call completion function passing ref argument.  Note that completion
 *		function may be called either before or after the function returns.
 *
 * @param[in] tx_len
 * 		Number of bytes to transfer from tx buffer
 * @param[in] *tx_data
 * 		Pointer to data to be transferred/TX'd
 * @param[in] tx2_len
 * 		Number of bytes to transfer from tx2 buffer
 * @param[in] *tx2_data
 * 		Pointer to data to be transferred from tx2 buffer.
 * @param[in] hold_cs_active
 * 		TRUE/FALSE, Determines whether to hold CS active
 * @param[in] *completion
 * 		Completion state machine callback function
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback
 *
 ******************************************************************************/
void spi_xfer(size_t tx_len,
			  const uint8_t *tx_data,
			  size_t tx2_len,
			  const uint8_t *tx2_data,
			  bool half_duplex,
			  size_t rx_len,
			  uint8_t *rx_data,
			  bool hold_cs_active,
			  spi_completion_fn_t *completion,
			  void *completion_ref)
{
	spi_rx_len = rx_len;
	spi_rx_data = rx_data;

	spi_tx_len = tx_len;
	spi_tx_data = tx_data;
	spi_tx2_len = tx2_len;
	spi_tx2_data = tx2_data;

	spi_tx_post_padding_len = 0;
	spi_rx_pre_padding_len = 0;
	spi_rx_post_padding_len = 0;

	spi_hold_cs_active = hold_cs_active;

	if (half_duplex)
	{
		spi_rx_pre_padding_len = tx_len + tx2_len;
		spi_tx_post_padding_len = spi_rx_len;
	}
	else if ((tx_len + tx2_len) < rx_len)
		spi_tx_post_padding_len = rx_len - (tx_len + tx2_len);
	else
		spi_rx_post_padding_len = (tx_len + tx2_len) - rx_len;

	//printf("tx: %d, tx2: %d, tx_post: %d\r\n", tx_len, tx2_len, spi_tx_post_padding_len);
	//printf("rx_pre: %d, rx: %d, rx_post: %d\r\n", spi_rx_pre_padding_len, rx_len, spi_rx_post_padding_len);

	spi_completion = completion;
	spi_completion_ref = completion_ref;

	spi_busy = true;

	GPIO_PinOutClear(CS_PORT, CS_PIN);  // assert CS

	// enable interrupts to start transfer
	SPI_PORT->IEN |= (USART_IEN_TXBL | USART_IEN_RXDATAV);

	if (! completion)
		while (spi_busy)
			EMU_EnterEM1();
}


static bool so_busy;
static spi_completion_fn_t *so_completion;
static void *so_completion_ref;

volatile uint32_t so_irq_count;
volatile uint32_t so_spurious_irq_count;
volatile uint32_t so_irq_fake_count;

/***************************************************************************//**
 * @brief
 *   Active SO Interrupt handler
 * @param[in] *ref
 *
 ******************************************************************************/
void SO_IRQ(void *ref)
{
	(void) ref;

	int p = GPIO_PinInGet(RX_PORT, RX_PIN);

	if (p != 0)
	{
		// should never happen - we should only get the interrupt when the pin goes low
		GPIO->IFC = 1 << RX_PIN;
		so_spurious_irq_count++;
		return;
	}

	// disable IRQ
	GPIO_IntConfig(RX_PORT, RX_PIN,
			       false,   // risingEdge
			       false,   // fallingEdge
			       false);  // enable

	so_irq_count++;

	if (! so_busy)
	{
		// should never happen
		while (1)
			;
	}

	GPIO_PinOutSet(CS_PORT, CS_PIN);  // deassert CS

	// copy completion fn ptr and ref arg, to avoid race condition
	// if completion fn starts another SPI xfer
	spi_completion_fn_t *completion = so_completion;
	void *completion_ref = so_completion_ref;

	so_busy = false;
	if (completion)
		completion(completion_ref);
}

volatile unsigned p1;

/***************************************************************************//**
 * @brief
 *   Active SO Interrupt handler
 * @note
 * 		Note: may be called synchronously (completion == NULL), in which case
 * 		this function does not return until the SO interrupt occurs, or
 * 		asynchronously (completion != NULL), in which case this function merely
 * 		sets up the interrupt, returns to the caller, and the caller should do
 * 		whatever is desired until the SO interrupt occurs, at which time the
 * 		completion function is called.
 * @param[in] level
 * 		Determines Edge Level Interrupt: Rising/Falling, spiflash_info->so_done_level
 * 		is the variable that is passed into the function and is initialized in the
 * 		Device table found in spiflash.c
 * @param[in] *completion
 * 		Completion State machine callback function
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback
 *******************************************************************************/
void spi_wait_so(uint8_t level,
                 spi_completion_fn_t *completion,  // completion callback fn
		         void *completion_ref)  // argument to be passed to completion callback
{
	so_completion = completion;
	so_completion_ref = completion_ref;
	so_busy = true;

	INT_Disable();

	// enable IRQ
	// The EFM32 has edge-triggered interrupts, so we'll never get an
	// interrupt if the port pin was already at the desired level.  To
	// avoid deadlock, we check the pin state here, and if it's already
	// at the desired level, simulate an edge interrupt.
	p1 = GPIO_PinInGet(RX_PORT, RX_PIN);
	if (p1 == level)
	{
		so_irq_fake_count++;
		SO_IRQ(NULL);
	}
	else
	{
		// Note that GPIO_IntConfig will clear any pending IRQ for the pin
		GPIO_IntConfig(RX_PORT, RX_PIN,
					   level,  // risingEdge
					   !level, // fallingEdge
					   true);  // enable
	}
	INT_Enable();

	// if called synchronously, actually wait here
	if (!completion)
		while (so_busy)
			enter_low_power_state();
}

USART_InitSync_TypeDef spiInit =
{
  .enable       = usartEnableRx | usartEnableTx,
  .refFreq      = 0,
  .baudrate     = 1000000,
  .databits     = usartDatabits8,
  .master       = true,
  .msbf         = true,
  .clockMode    = usartClockMode0,
  .prsRxEnable  = false,          /* Enable USART Rx via Peripheral Reflex System */
  .prsRxCh      = usartPrsRxCh0,  /* Select PRS channel if enabled */
  .autoTx       = false,
};

static const gpio_init_t spi_pins[] =
{
  { CK_PORT, CK_PIN, gpioModePushPull,  0 },
  { CS_PORT, CS_PIN, gpioModePushPull,  1 },
  { RX_PORT, RX_PIN, gpioModeInputPull, 1 },
  { TX_PORT, TX_PIN, gpioModePushPull,  1 },
};

/***************************************************************************//**
 * @brief
 *   SPI/USART Initialization
 *
 * @param[in] bit_rate
 * 		Bit rate for SPI Bus in MHz: Either 2000000 or 12000000
 *
 ******************************************************************************/
void spi_init(int bit_rate)
{
	CMU_ClockEnable(cmuClock_GPIO, true);
	CMU_ClockEnable(SPI_cmuClock, true);

	gpio_init(spi_pins, sizeof(spi_pins)/sizeof(gpio_init_t));

	USART_Reset(SPI_PORT);
	USART_InitSync(SPI_PORT, &spiInit);

	SPI_PORT->CTRL |= USART_CTRL_TXBIL_HALFFULL;
	SPI_PORT->ROUTE = USART_ROUTE_TXPEN | USART_ROUTE_RXPEN | USART_ROUTE_CLKPEN | (USART_LOC << _USART_ROUTE_LOCATION_SHIFT);

	if (bit_rate)
		USART_BaudrateSyncSet(SPI_PORT, 0, bit_rate);

	USART_IntClear(SPI_PORT, USART_IF_RXDATAV | USART_IF_TXBL);

	NVIC_ClearPendingIRQ(SPI_TX_IRQn);
	NVIC_EnableIRQ(SPI_TX_IRQn);

	NVIC_ClearPendingIRQ(SPI_RX_IRQn);
	NVIC_EnableIRQ(SPI_RX_IRQn);

	so_busy = false;

	so_irq_count = 0;
	so_spurious_irq_count = 0;
	so_irq_fake_count = 0;

	// register (but don't enable) rx pin GPIO interrupt
	gpio_irq_handler_install(RX_PORT, RX_PIN, SO_IRQ, NULL);
	}

/** @} (end addtogroup Peripheral Functions) */
/** @} (end addtogroup spi) */
