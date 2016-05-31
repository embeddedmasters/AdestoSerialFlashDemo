/******************************************************************************
 * @file spiflash.c
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

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "em_emu.h"

#include "low_power.h"
#include "spi.h"
#include "spiflash.h"

/***************************************************************************//**
 * @addtogroup Adesto_FlashDrivers
 * @{
 ******************************************************************************/

#define LED_DEBUG
#ifdef LED_DEBUG
#include "led.h"
#include "delay.h"
int i;
int cycle_count = 0;
#endif

/* Adesto Command Hex Values */
#define CMD_READ_ARRAY                  0x0b
#define CMD_READ_ARRAY_SLOW             0x03  /* no dummy byte, but limited to lower clock rate */
#define CMD_READ_ARRAY_DUAL             0x3b  /* not used, as we don't have dual-SPI hardware */
#define CMD_PAGE_ERASE                  0x81
#define CMD_RM25C_PAGE_ERASE            0x42  /* RM26C256DS only */
#define CMD_BLOCK_ERASE                 0x20
#define CMD_BLOCK_ERASE_LARGE           0x52
#define CMD_BLOCK_ERASE_LARGER          0xd8
#define CMD_CHIP_ERASE                  0x60
#define CMD_CHIP_ERASE2                 0xc7  /* not used, exactly the same as CMD_CHIP_ERASE */
#define CMD_BYTE_PAGE_PROGRAM           0x02
#define CMD_SEQUENTIAL_PROGRAM          0xad  /* not used, page program is more efficient */
#define CMD_SEQUENTIAL_PROGRAM2         0xaf  /* not used, page program is more efficient */
#define CMD_BYTE_PAGE_PROGRAM_DUAL      0xa2  /* not used, as we don't have dual-SPI hardware */
#define CMD_WRITE_ENABLE                0x06
#define CMD_WRITE_DISABLE               0x04
#define CMD_PROTECT_SECTOR              0x36
#define CMD_UNPROTECT_SECTOR            0x39
#define CMD_READ_SECTOR_PROTECTION      0x3c
#define CMD_PROGRAM_OTP                 0x9b
#define CMD_READ_OTP                    0x77
#define CMD_READ_STATUS                 0x05
#define CMD_DATAFLASH_READ_STATUS       0xd7
#define CMD_ACTIVE_STATUS_INTERRUPT     0x25
#define CMD_WRITE_STATUS_REG_BYTE_1     0x01
#define CMD_WRITE_STATUS_REG_BYTE_2     0x31
#define CMD_RESET                       0xf0
#define CMD_READ_ID                     0x9f
#define CMD_DEEP_POWER_DOWN             0xb9
#define CMD_RESUME_FROM_DEEP_POWER_DOWN 0xab
#define CMD_ULTRA_DEEP_POWER_DOWN       0x79

#define ARG_RESET                       0xd0  /* second byte required for reset command */

// commands only for DataFlash, e.g., AT45DB081E and AT45DB641E
#define CMD_DATAFLASH_BLOCK_ERASE       0x50  /* 8 pages, 2KB */
#define CMD_DATAFLASH_SECTOR_ERASE      0x7c

#define CMD_DATAFLASH_RMW_BUF1          0x58
#define CMD_DATAFLASH_RMW_BUF2          0x59


static const uint8_t at25xe021a_id_bytes[] = { 0x1f, 0x43, 0x01, 0x00 };
static const size_t at25xe021a_protection_sector_sizes[] = { 65536, 65536, 65536, 65536 };

static const uint8_t at25xe041b_id_bytes[] = { 0x1f, 0x44, 0x02, 0x00 };
static const size_t at25xe041b_protection_sector_sizes[] = { 65536, 65536, 65536, 65536, 65536, 65536, 65536, 32768, 8192, 8192, 16384 };

static const uint8_t dataflash_cmd_set_256b_page [] = { 0x3d, 0x2a, 0x80, 0xa6 };
static const uint8_t dataflash_cmd_set_264b_page [] = { 0x3d, 0x2a, 0x80, 0xa7 };

static const uint8_t dataflash_cmd_chip_erase [] = { 0xc7, 0x94, 0x80, 0x9a };
static const uint8_t dataflash_cmd_disable_sector_protection [] = { 0x3d, 0x2a, 0x7f, 0x9a };

const spiflash_info_t spiflash_info_table[] =
{
	[AT25SF041] = {
		.name					 = "AT25SF041",
		.id_size                 = 3,
		.id_bytes                = { 0x1f, 0x84, 0x01 },
	    .device_size             = (4 << 20) / 8,
	    .address_bytes           = 3,
	    .program_page_size       = 256,
	    .erase_info_count        = 4,
	    .erase_info              = {{ 4096,   CMD_BLOCK_ERASE,        true },
	    		                    { 32768,  CMD_BLOCK_ERASE_LARGE,  true },
	    		                    { 65536,  CMD_BLOCK_ERASE_LARGER, true },
	    		                    { (4 << 20) / 8, CMD_CHIP_ERASE,  false }},
	    .protection_sector_sizes = at25xe021a_protection_sector_sizes,
	    .protection_sector_count = sizeof(at25xe021a_protection_sector_sizes) / sizeof(size_t),
	    .read_status_cmd         = CMD_READ_STATUS,
	    .status_busy_mask        = 0x01,
	    .status_busy_level       = 0x01,
	    .has_so_irq              = false,
	    .dataflash               = false,
	},
	[AT25XE021A] = {
		.name                    = "AT25XE021A",
	    .id_size                 = 4,
	    .id_bytes                = { 0x1f, 0x43, 0x01, 0x00 },
	    .device_size             = (2 << 20) / 8,
	    .address_bytes           = 3,
	    .program_page_size       = 256,
	    .erase_info_count        = 5,
	    .erase_info              = {{ 256,    CMD_PAGE_ERASE,         true },
	                                { 4096,   CMD_BLOCK_ERASE,        true },
	    		                    { 32768,  CMD_BLOCK_ERASE_LARGE,  true },
	    		                    { 65536,  CMD_BLOCK_ERASE_LARGER, true },
	    		                    { (2 << 20) / 8, CMD_CHIP_ERASE,  false }},
	    .protection_sector_sizes = at25xe021a_protection_sector_sizes,
	    .protection_sector_count = sizeof(at25xe021a_protection_sector_sizes) / sizeof(size_t),
	    .read_status_cmd         = CMD_READ_STATUS,
	    .status_busy_mask        = 0x01,
	    .status_busy_level       = 0x01,
	    .has_so_irq              = true,
	    .so_done_level           = 0,
	    .dataflash               = false,
	},
	[AT25XE041B] = {
	    .name                    = "AT25XE041B",
	    .id_size                 = 4,
	    .id_bytes                = { 0x1f, 0x44, 0x02, 0x00 },
	    .device_size             = (4 << 20) / 8,
	    .address_bytes           = 3,
	    .program_page_size       = 256,
	    .erase_info_count        = 5,
	    .erase_info              = {{ 256,    CMD_PAGE_ERASE,         true },
	                                { 4096,   CMD_BLOCK_ERASE,        true },
	    		                    { 32768,  CMD_BLOCK_ERASE_LARGE,  true },
	    		                    { 65536,  CMD_BLOCK_ERASE_LARGER, true },
	    		                    { (4 << 20) / 8, CMD_CHIP_ERASE,  false }},
	    .protection_sector_sizes = at25xe041b_protection_sector_sizes,
	    .protection_sector_count = sizeof(at25xe041b_protection_sector_sizes) / sizeof(size_t),
	    .read_status_cmd         = CMD_READ_STATUS,
	    .status_busy_mask        = 0x01,
	    .status_busy_level       = 0x01,
	    .has_so_irq              = true,
	    .so_done_level           = 0,
	    .dataflash               = false,
	},
	[AT45DB081E] = {
	    .name                    = "AT45DB081E",
	    .id_size                 = 5,
	    .id_bytes                = { 0x1f, 0x25, 0x00, 0x01, 0x00 },
	    .device_size             = (8 << 20) / 8,
	    .address_bytes           = 3,
	    .program_page_size       = 256,
	    .erase_info_count        = 3,
	    .erase_info              = {{ 256,    CMD_PAGE_ERASE,            true },
	                                { 2048,   CMD_DATAFLASH_BLOCK_ERASE, true },
	    		                    { (8 << 20) / 8, 0,                  false }},
	    .protection_sector_sizes = 0,
	    .protection_sector_count = 0,
	    .read_status_cmd         = CMD_DATAFLASH_READ_STATUS,
	    .status_busy_mask        = 0x80,
	    .status_busy_level       = 0x00,
	    .has_so_irq              = false,
	    .dataflash               = true,
	},
	[AT45DB641E] = {
	    .name                    = "AT45DB641E",
	    .id_size                 = 3,
	    .id_bytes                = { 0x1f, 0x28, 0x00 },
	    .device_size             = (64 << 20) / 8,
	    .address_bytes           = 3,
	    .program_page_size       = 256,
	    .erase_info_count        = 3,
	    .erase_info              = {{ 256,    CMD_PAGE_ERASE,             true },
	                                { 2048,   CMD_DATAFLASH_BLOCK_ERASE,  true },
	    		                    { (64 << 20) / 8, 0,                  false }},
	    .protection_sector_sizes = 0,
	    .protection_sector_count = 0,
	    .read_status_cmd         = CMD_DATAFLASH_READ_STATUS,
	    .status_busy_mask        = 0x80,
	    .status_busy_level       = 0x00,
	    .has_so_irq              = false,
	    .dataflash               = true,
	},
	[RM25C256DS] = {
		.name                    = "RM25C256DS",
		.id_size                 = 3,
		.id_bytes                = { 0x7f, 0x7f, 0x7f },
		.device_size             = (256 << 10) / 8,
	    .address_bytes           = 2,
		.program_page_size       = 64,
#define RM25C256DS_ALLOW_ERASE_64B 1
#if RM25C256DS_ALLOW_ERASE_64B
		.erase_info_count        = 2,
		.erase_info              = {{ 64,              CMD_RM25C_PAGE_ERASE, true },    // 0x42 command
				                    { (256 << 10) / 8, CMD_CHIP_ERASE,       false }},  // 0x60 or 0xc7
#else
	    .erase_info_count        = 1,
	    .erase_info              = {{ (256 << 10) / 8, CMD_CHIP_ERASE,       false }},  // 0x60 or 0xc7
#endif
  	    .protection_sector_sizes = 0,
  	    .protection_sector_count = 0,
  	    .read_status_cmd         = CMD_READ_STATUS,
  	    .status_busy_mask        = 0x01,
  	    .status_busy_level       = 0x01,
  	    .has_so_irq              = false,
  	    .dataflash               = false,
  	    .read_slow               = true,  // RM25C256DS seems to acutally support the 0x0b READ ARRAY command, but
  	                                      // it's not documented, so we shouldn't use it.
	},
};

#define SPIFLASH_INFO_TABLE_SIZE (sizeof(spiflash_info_table) / sizeof(spiflash_info_t))



static const spiflash_info_t *spiflash_info;
static bool spiflash_use_so_irq;
static bool spiflash_busy;
static spiflash_completion_fn_t *spiflash_completion;
static void *spiflash_completion_ref;

static uint8_t spiflash_scratch_buf [257];


/***************************************************************************//**
 * @brief
 *   Checks if address is aligned on a power of 2
 * @note
 * 		Determines if address is aligned to a multiple of alignment_size which
 * 		is a power of 2.  Alignment_size is read from Device table in spiflash.c
 * @param[in] addr
 * 		Address to check Power of 2 alignment
 * @param[in] alignment_size
 * 		Alignment_size used to validate if Power of 2
 * @return
 * 		TRUE if aligned on Power of 2, False if not.
 *
 ******************************************************************************/
static bool addr_aligned(uint32_t addr, size_t alignment_size)
{
	return (addr & (alignment_size-1)) == 0;
}

/***************************************************************************//**
 * @brief
 *   Simple spi simple completion routine
 * @note
 *
 * @param[in] *ref
 //TODO What is *ref ???
 *
 ******************************************************************************/
static void spiflash_spi_simple_completion(void *ref)
{
	(void) ref;

	// copy completion fn ptr and ref arg, to avoid race condition
	// if completion fn starts another spiflash command
	spiflash_completion_fn_t *completion = spiflash_completion;
	void *spiflash_ref = spiflash_completion_ref;

	spiflash_completion = NULL;

	spiflash_busy = false;
	if (completion)
		completion(spiflash_ref);
}

/***************************************************************************//**
 * @brief
 *   SPI Flash Single Byte Command
 * @note
 * 		spiflash_single_byte_command() can be used to issue any single-byte
 *		command. The reading of the response will begin after the command
 * 		byte is written. If no response is to be read, pass rx_len == 0.
 * @param[in] cmd
 *		Command to send
 * @param[in] rx_len
 * 		Expected response length, set to 0 if no response is expected.
 * @param[in] *rx_buf
 * 		Pointer to Rx buffer where response is placed.
 * @param[in] *completion
 * 		Completion function for Completion State_Machine
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void spiflash_single_byte_command(uint8_t cmd,
		                          size_t rx_len,
		                          uint8_t *rx_buf,
                                  spiflash_completion_fn_t *completion,
                                  void *completion_ref)
{
	spiflash_scratch_buf[0] = cmd;

	spiflash_completion = completion;
	spiflash_completion_ref = completion_ref;

	spiflash_busy = true;

	spi_xfer(1, spiflash_scratch_buf,  // tx1
			 0, NULL,                  // tx2
			 true,                     // half duplex
			 rx_len, rx_buf,           // rx
			 false,                    // hold cs active
			 spiflash_spi_simple_completion,
			 NULL);

	if (! completion)
		while (spiflash_busy)
			enter_low_power_state();
}

/***************************************************************************//**
 * @brief
 *   SPI Flash Multi-Byte Command
 * @note
 * 		spiflash_multiple_byte_command() can be used to issue commands of any
 *		length. The reading of the response will begin after the last command
 *		byte is written. If no response is to be read, pass rx_len == 0.
 * @param[in] tx_len
 *		Length of command to send in bytes
 * @param[in] *tx_buf
 * 		Pointer to command buffer to send
 * @param[in] rx_len
 * 		Expected response length, set to 0 if no response is expected.
 * @param[in] *rx_buf
 * 		Pointer to Rx buffer where response is placed.
 * @param[in] hold_cs_active
 * 		Hold Chip Select High: True/False
 * @param[in] *completion
 * 		Completion function for Completion State_Machine
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void spiflash_multiple_byte_command(size_t tx_len,
		                            const uint8_t *tx_buf,
		                            size_t rx_len,
		                            uint8_t *rx_buf,
		                            bool hold_cs_active,
                                    spiflash_completion_fn_t *completion,
                                    void *completion_ref)
{
	spiflash_completion = completion;
	spiflash_completion_ref = completion_ref;

	spiflash_busy = true;

	spi_xfer(tx_len, tx_buf,  // tx1
			 0, NULL,         // tx2
			 true,            // half duplex
			 rx_len, rx_buf,  // rx
			 hold_cs_active,  // hold cs active
			 spiflash_spi_simple_completion,
			 NULL);

	if (! completion)
		while (spiflash_busy)
			enter_low_power_state();
}

/***************************************************************************//**
 * @brief
 *   SPI Flash Command with Address
 * @note
 * 		spiflash_multiple_command_with_adddress() can be used to issue commands
 * 		of any length. The reading of the response will begin after the last command
 *		byte is written. If no response is to be read, pass rx_len == 0.
 * @param[in] cmd
 * 		Command to send
 * @param[in] addr
 * 		Address for command to use
 * @param[in] dumm_bytes
 * 		Dummy bytes
 * @param[in] tx_len
 *		Length of command to send in bytes
 * @param[in] *tx_buf
 * 		Pointer to command buffer to send
 * @param[in] rx_len
 * 		Expected response length, set to 0 if no response is expected.
 * @param[in] *rx_buf
 * 		Pointer to Rx buffer where response is placed.
 * @param[in] *completion
 * 		Completion function for Completion State_Machine
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void spiflash_command_with_address(uint8_t cmd,
		                           uint32_t addr,
		                           unsigned int dummy_bytes,
		                           size_t tx_len,
		                           uint8_t *tx_buf,
		                           size_t rx_len,
		                           uint8_t *rx_buf,
		                           spiflash_completion_fn_t *completion,
		                           void *completion_ref)
{
	int i = 0;

	spiflash_scratch_buf[i++] = cmd;
	if (spiflash_info->address_bytes >= 3)
		spiflash_scratch_buf[i++] = addr >> 16;
	if (spiflash_info->address_bytes >= 2)
		spiflash_scratch_buf[i++] = (addr >> 8) & 0xff;
	if (spiflash_info->address_bytes >= 1)
		spiflash_scratch_buf[i++] = addr & 0xff;
	memset(&spiflash_scratch_buf[i], 0, dummy_bytes);
	i += dummy_bytes;

	spiflash_completion = completion;
	spiflash_completion_ref = completion_ref;

	spiflash_busy = true;

	spi_xfer(i, spiflash_scratch_buf,  // tx1
			 tx_len, tx_buf,                         // tx2
			 true,                                   // half duplex
			 rx_len, rx_buf,
			 false,                    // hold cs active
			 spiflash_spi_simple_completion,
			 NULL);

	if (! completion)
		while (spiflash_busy)
			enter_low_power_state();
}

/***************************************************************************//**
 * @brief
 *   SPI Flash Read
 * @note
 * 		Calls spiflash_command_with_address to perform Read operation
 * 		Will block if NULL passed for completion function;
 *		otherwise will call completion function passing
 *		ref argument
 *
 * @param[in] addr
 * 		Address to read from
 * @param[in] len
 * 		How many bytes to read
 * @param[in] *buffer
 * 		Buffer to place read data into.  Passed into spiflash_command_with_address function.
 * @param[in] *completion
 * 		Completion function for Completion State_Machine
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void spiflash_read(uint32_t addr,
		           size_t len,
		           uint8_t  *buffer,
		           spiflash_completion_fn_t *completion,
		           void *completion_ref)
{
	uint8_t cmd;
	int dummy_bytes;

	if (spiflash_info->read_slow)
	{
		cmd = CMD_READ_ARRAY_SLOW;
		dummy_bytes = 0;
	}
	else
	{
		cmd = CMD_READ_ARRAY;
		dummy_bytes = 1;
	}

	spiflash_command_with_address(cmd,
                                  addr,
                                  dummy_bytes,
                                  0, NULL,      // tx
			                      len, buffer,  // rx
			                      completion,
			                      completion_ref);
}


static const erase_info_t *erase_info_fixed;  // if non-NULL, always use this erase size and command
                                              // if NULL, choose erase size and command automatically
static const erase_info_t *erase_info;  // The erase size and command being used
static uint32_t erase_addr;
static size_t erase_len;
static uint8_t erase_status_buf[2];
static volatile bool spiflash_erase_busy;

static void spiflash_erase_completion1(void *ref);
static void spiflash_erase_completion2(void *ref);
static void spiflash_erase_completion3(void *ref);
static void spiflash_erase_completion4(void *ref);
static void spiflash_erase_completion5(void *ref);

/***************************************************************************//**
 * @brief
 *   SPI Erase Completion State 1(initial state)
 * @note
 * 		First state in Erase completion process.  Configures Write Enable.
 * @param[in] *ref
 * 		Completion Function Pointer
 ******************************************************************************/
static void spiflash_erase_completion1(void *ref)
{
	if (! erase_len)
	{
		spiflash_erase_busy = false;
		spiflash_spi_simple_completion(ref);
		return;
	}

	if (spiflash_info->dataflash)
		spiflash_erase_completion2(NULL);
	else
		spiflash_single_byte_command(CMD_WRITE_ENABLE,
									 0, NULL,  // rx
									 spiflash_erase_completion2,
									 NULL);
}

/***************************************************************************//**
 * @brief
 *   SPI Erase Completion State 2
 * @note
 * 	 Issues ERASE Commands and checks what type of Erase is available
 * 	 on Device, checks address alignment
 * @param[in] *ref
 * 		Completion Function Pointer
 ******************************************************************************/
static void spiflash_erase_completion2(void *ref)
{
	int i;

	if (erase_info_fixed)
		erase_info = erase_info_fixed;
	else
	{
		for (i = spiflash_info->erase_info_count - 1; i >= 0; i--)
		{
			erase_info = & spiflash_info->erase_info [i];
			if ((erase_len >= erase_info->size) &&
			    addr_aligned(erase_addr, erase_info->size))
				break;
		}
		if (i < 0)
		{
			// no suitable erase command found!
			// should never happen
			erase_info = NULL;
			while (true)
				;
		}
	}
	if (erase_info->addr_needed)
	{
		spiflash_command_with_address(erase_info->cmd,
	                                  erase_addr,
	                                  0,        // dummy bytes
	                                  0, NULL,  // tx
				                      0, NULL,  // rx
				                      spiflash_erase_completion3,
				                      NULL);
	}
	else if	(spiflash_info->dataflash)
	{
		// chip erase for dataflash doesn't need an address, but is a multibyte command
		spiflash_multiple_byte_command(sizeof(dataflash_cmd_chip_erase), dataflash_cmd_chip_erase,
									   0, NULL, // rx
									   false, // hold_cs_active
									   spiflash_erase_completion3,
									   NULL);
	}
	else
	{
		// chip erase doesn't need an address
		spiflash_single_byte_command(erase_info->cmd,
				                     0, NULL,  // rx
				                     spiflash_erase_completion3,
				                     NULL);
	}
}

/***************************************************************************//**
 * @brief
 *   SPI Erase Completion State 3
 * @note
 * 	 	An erase command has been issued. Advance the address and decrease the
 * 	 	length in preparation for the next iteration.  If erase_len drops to
 * 	 	zero, that will be handled when it gets back to Completion 1.
 * 	 	Determines whether or not to use Active SO or not.  If Active SO is used
 * 	 	go to Read completion 4 if not skip to Read completion 5.
 * @param[in] *ref
 * 		Completion Function Pointer
 ******************************************************************************/
static void spiflash_erase_completion3(void *ref)
{
	erase_addr += erase_info->size;
	erase_len -= erase_info->size;

	if (spiflash_use_so_irq)
	{
		// use of SO is enabled, so issue ASI command
		spiflash_scratch_buf[0] = CMD_ACTIVE_STATUS_INTERRUPT;
		spiflash_scratch_buf[1] = 0x00;

		spiflash_multiple_byte_command(1, spiflash_scratch_buf,  // tx
									   0, NULL,                  // rx
									   true,                     // hold CS active
									   spiflash_erase_completion4,
									   NULL);
	}
	else
	{
		// use of SO is disabled, so start polling status register
		erase_status_buf[0] = spiflash_info->status_busy_level;  // prime pump w/ BUSY
		spiflash_erase_completion5(NULL);
	}
}

/***************************************************************************//**
 * @brief
 *   SPI Erase Completion State 4
 * @note
 * 		This Erase Completion State is ONLY used if Active SO is enabled.  Returns to
 * 		Completion 1 when completed.
 * @param[in] *ref
 * 		Completion Function Pointer
 ******************************************************************************/
static void spiflash_erase_completion4(void *ref)
{
	spi_wait_so(spiflash_info->so_done_level,
					spiflash_erase_completion1,
					NULL);
}

/***************************************************************************//**
 * @brief
 *   SPI Erase Completion State 5
 * @note
 * 		This Read Completion State is ONLY used if Active SO is NOT used.  Returns to
 * 		Completion 1 when completed.
 * @param[in] *ref
 * 		Completion Function Pointer
 ******************************************************************************/
static void spiflash_erase_completion5(void *ref)
{
	if ((erase_status_buf[0] & spiflash_info->status_busy_mask) == spiflash_info->status_busy_level)
	{
        // BUSY
		spiflash_single_byte_command(spiflash_info->read_status_cmd,
				                     1,
				                     erase_status_buf,
				                     spiflash_erase_completion5,
				                     NULL);
	}
	else
	{
		// DONE
		spiflash_erase_completion1(NULL);
	}
}

/***************************************************************************//**
 * @brief
 *   SPI Flash Erase
 * @note
 * 		Top Level SPI Erase function, steps through Erase completion states.
 * 		Determines if Active SO is to be used or not.  Checks address alignment.
 * 		Enters Low Power State during Erase.
 * 		The erase commands will be ignored by the flash chip unless it has
 *		been previously write-enabled. Will return false, and never call
 *		completion function, if address is not erase page aligned or len is not
 *		a multiple of erase page size.
 * @param[in] addr
 * 		Address to start erase
 * @param[in] len
 * 		How many bytes to erase
 * @param[in] cmd_size
 * 		Size of Command in Bytes
 * @param[in] use_so_irq
 * 		Determines whether or not Active SO is used or not.
 * @param[in] *completion
 * 		Completion Function pointer.
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 * @return true
 * 		If successful
 ******************************************************************************/
bool spiflash_erase(uint32_t addr,
			        size_t len,
			        uint32_t cmd_size,  // bytes per erase command, 0 for auto
		            bool use_so_irq,
		            spiflash_completion_fn_t *completion,
		            void *completion_ref)
{
	int i;
	bool found = false;

	spiflash_use_so_irq = use_so_irq && spiflash_info->has_so_irq;

	if (cmd_size)
	{
		// The desired erase size per command was specified.
		// Find the matching erase command, and ensure the address is
		// suitably aligned and the erase size is a multiple of the
		// command size.
		if (len % cmd_size)
			return false;
		for (i = spiflash_info->erase_info_count - 1; i >= 0; i--)
		{
			erase_info_fixed = & spiflash_info->erase_info [i];
			if (cmd_size == erase_info_fixed->size)
			{
				found = true;
				break;
			}
		}
		if (! found)
		{
			erase_info_fixed = NULL;
			return false;
		}
	}
	else
	{
		// No erase size per command was specified.
		// Ensure that there is a suitable choice.
		for (i = spiflash_info->erase_info_count - 1; i >= 0; i--)
		{
			erase_info = & spiflash_info->erase_info [i];
			if (len % erase_info->size)
				continue;
			if (! addr_aligned(addr, erase_info->size))
				continue;
			found = true;
			break;
		}
		erase_info = NULL;  // will be chosen in spiflash_erase_completion2;
		if (! found)
			return false;
	}

	erase_addr = addr;
	erase_len = len;
	spiflash_completion = completion;
	spiflash_completion_ref = completion_ref;
	spiflash_erase_busy = true;

	if (spiflash_info->dataflash)
		spiflash_multiple_byte_command(sizeof(dataflash_cmd_disable_sector_protection), dataflash_cmd_disable_sector_protection,
									   0, NULL, // rx
									   false, // hold_cs_active
									   spiflash_erase_completion1,
									   NULL);

	else
		spiflash_erase_completion1(NULL);

	// if called synchronously, wait for entire write sequence to complete
	if (! completion)
	{
		while (spiflash_erase_busy)
		{
			enter_low_power_state();
		}
	}
	return true;
}


static uint8_t *write_data;
static uint32_t write_addr;
static size_t write_len;
static size_t write_size;
static uint8_t write_status_buf[2];
static volatile bool spiflash_write_busy;

static void spiflash_write_completion1(void *ref);
static void spiflash_write_completion2(void *ref);
static void spiflash_write_completion3(void *ref);
static void spiflash_write_completion4(void *ref);
static void spiflash_write_completion5(void *ref);

/***************************************************************************//**
 * @brief
 *   SPI Write Completion State 1(initial state)
 * @note
 * 		First State in Write completion process.  Configures Write Enable.
 * @param[in] *ref
 * 		Completion Function Pointer
 ******************************************************************************/
static void spiflash_write_completion1(void *ref)
{
	if (! write_len)
	{
		spiflash_write_busy = false;
		spiflash_spi_simple_completion(ref);
		return;
	}

	if (spiflash_info->dataflash)
		spiflash_write_completion2(NULL);
	else
		spiflash_single_byte_command(CMD_WRITE_ENABLE,
			                     	 0, NULL,  // rx
			                     	 spiflash_write_completion2,
			                     	 NULL);
}

/***************************************************************************//**
 * @brief
 *   SPI Write Completion State 2
 * @note
 * 	 Issues Write Command with Address and check Program Page size
 * @param[in] *ref
 * 		Completion Function Pointer
 ******************************************************************************/
static void spiflash_write_completion2(void *ref)
{
	write_size = write_len;
	if ((write_addr & ~(spiflash_info->program_page_size-1)) !=
		((write_addr + write_size - 1) & ~(spiflash_info->program_page_size-1)))
		write_size = spiflash_info->program_page_size - (write_addr & (spiflash_info->program_page_size-1));

	spiflash_command_with_address(CMD_BYTE_PAGE_PROGRAM,
	                              write_addr,
	                              0, // dummy bytes
				                  write_size, write_data,  // tx
				                  0, NULL,                 // rx
				                  spiflash_write_completion3,
				                  NULL);

}

/***************************************************************************//**
 * @brief
 *   SPI Write Completion State 3
 * @note
 * 	 	A write command has been issued. Advance the address and decrease the
 * 	 	length in preparation for the next iteration.  If erase_len drops to
 * 	 	zero, that will be handled when it gets back to Completion 1.
 * 	 	Determines whether or not to use Active SO or not.  If Active SO is used
 * 	 	go to Write completion 4 if not skip to Write completion 5.
 * @param[in] *ref
 * 		Completion Function Pointer
 ******************************************************************************/
static void spiflash_write_completion3(void *ref)
{
	// Write is in progress, but advance the address
	// and decrease the length in preparation for next
	// iteration.  If write_len drops to zero, that will
	// be handled when we get to
	// spiflash_write_completion1().
	write_data += write_size;
	write_addr += write_size;
	write_len -= write_size;

	if (spiflash_use_so_irq == 1)
	{
		// use of SO is enabled, so issue ASI command
		spiflash_scratch_buf[0] = CMD_ACTIVE_STATUS_INTERRUPT;
		spiflash_scratch_buf[1] = 0x00;

		spiflash_multiple_byte_command(2, spiflash_scratch_buf,  // tx
									   0, NULL,                  // rx
									   true,                     // hold CS active
									   spiflash_write_completion4,
									   NULL);
	}
	else
	{
		// use of SO is disabled, so start polling status register
		write_status_buf[0] = spiflash_info->status_busy_level;  // prime pump w/ BUSY
		spiflash_write_completion5(NULL);
	}
}

/***************************************************************************//**
 * @brief
 *   SPI Write Completion State 4
 * @note
 * 		This Write completion state is ONLY used if Active SO is enabled.  Returns to
 * 		Write completion 1 when completed.
 * @param[in] *ref
 * 		Completion Function Pointer
 ******************************************************************************/
static void spiflash_write_completion4(void *ref)
{
	// wait for flash chip to be done with command,
	// then invoke completion1
	spi_wait_so(spiflash_info->so_done_level,
				spiflash_write_completion1,
		    	NULL);
}

/***************************************************************************//**
 * @brief
 *   SPI Write Completion State 5
 * @note
 * 		This Write completion state is ONLY used if Active SO is NOT used.  Returns to
 * 		Write completion 1 when completed.
 * @param[in] *ref
 * 		Completion Function Pointer
 ******************************************************************************/
static void spiflash_write_completion5(void *ref)
{
	if ((write_status_buf[0] & spiflash_info->status_busy_mask) == spiflash_info->status_busy_level)
	{
        // BUSY
		spiflash_single_byte_command(spiflash_info->read_status_cmd,
				                     1,
				                     write_status_buf,
				                     spiflash_write_completion5,
				                     NULL);
	}
	else
	{
		// DONE
		spiflash_write_completion1(NULL);
	}
}

/***************************************************************************//**
 * @brief
 *   SPI Flash Write
 * @note
 * 		Top Level SPI Write function, steps through Write completion states.
 * 		Determines if Active SO is to be used or not. Enters Low Power State
 * 		during Write.
 * @param[in] addr
 * 		Address to write to
 * @param[in] len
 * 		How many bytes to write
 * @param[in] *buffer
 * 		Pointer to data buffer to use for write.
 * @param[in] use_so_irq
 * 		Determines whether or not Active SO is used or not.
 * @param[in] *completion
 * 		Completion Function pointer.
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void spiflash_write(uint32_t addr,
		            size_t len,
		            uint8_t  *buffer,
		            bool use_so_irq,
		            spiflash_completion_fn_t *completion,
		            void *completion_ref)
{
	spiflash_use_so_irq = use_so_irq && spiflash_info->has_so_irq;

	write_data = buffer;
	write_addr = addr;
	write_len = len;
	spiflash_completion = completion;
	spiflash_completion_ref = completion_ref;
	spiflash_write_busy = true;

	if (spiflash_info->dataflash)
		spiflash_multiple_byte_command(sizeof(dataflash_cmd_disable_sector_protection), dataflash_cmd_disable_sector_protection,
									   0, NULL, // rx
									   false, // hold_cs_active
									   spiflash_write_completion1,
									   NULL);

	else
		spiflash_write_completion1(NULL);

	// if called synchronously, wait for entire write sequence to complete
	if (! completion)
		while (spiflash_write_busy)
			enter_low_power_state();
}


static void dataflash_rmw_completion1(void *ref);
static void dataflash_rmw_completion2(void *ref);
static void dataflash_rmw_completion3(void *ref);

/***************************************************************************//**
 * @brief
 *   DataFlash Read-Modify-Write Completion State 1(initial state)
 * @note
 * 		First State in Write completion process. Sends RMW command
 * @param[in] *ref
 * 		Completion Function Pointer
 ******************************************************************************/
static void dataflash_rmw_completion1(void *ref)
{
	spiflash_command_with_address(CMD_DATAFLASH_RMW_BUF1,
	                              write_addr,
	                              0, // dummy bytes
	                              write_size, write_data,  // tx
	                              0, NULL,                 // rx
	                              dataflash_rmw_completion2,
	                              NULL);
}

/***************************************************************************//**
 * @brief
 *   DataFlash Read-Modify-Write Completion State 2
 * @note
 * 		Second State in DataFlash Read-Modify-Write completion process.  Starts
 * 		polling Status Register.
 * @param[in] *ref
 * 		Completion Function Pointer
 ******************************************************************************/
static void dataflash_rmw_completion2(void *ref)
{
	// start polling status register
	write_status_buf[0] = spiflash_info->status_busy_level;  // prime pump w/ BUSY
	dataflash_rmw_completion3(NULL);
}

/***************************************************************************//**
 * @brief
 *   DataFlash Read-Modify-Write Completion State 3
 * @note
 * 		Third(last) State in DataFlash Read-Modify-Write completion process.
 * 		Continues polling Status Register.
 * @param[in] *ref
 * 		Completion Function Pointer
 ******************************************************************************/
static void dataflash_rmw_completion3(void *ref)
{
	// poll the status register
	// until the flash indicates that the RMW is done.
	if ((write_status_buf[0] & spiflash_info->status_busy_mask) == spiflash_info->status_busy_level)
	{
		// BUSY
		spiflash_single_byte_command(spiflash_info->read_status_cmd,
					                 1,
					                 write_status_buf,
					                 dataflash_rmw_completion3,
					                 NULL);
	}
	else
	{
		// DONE
		spiflash_write_busy = false;
		spiflash_spi_simple_completion(ref);
	}
}

/***************************************************************************//**
 * @brief
 *   SPI DataFlash Read-Modify-Write Function
 * @note
 * 		Perform RMW operation on a page of dataflash, steps through RMW
 * 		completion states.  Enters Low Power State during Write.
 *
 *		Reponsibilities of caller:
 * 		ensure that check that part is dataflash
 * 		ensure that len is not greater than page length
 * 		ensure that addr + len does not cross a page boundary
 * @param[in] addr
 * 		Address to read from
 * @param[in] len
 * 		How many bytes to update/perform RMW
 * @param[in] *buffer
 * 		Pointer to data buffer to use for write.
 * @param[in] *completion
 * 		Completion Function pointer.
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void dataflash_rmw(uint32_t addr,
				   size_t len,
				   uint8_t  *buffer,
				   spiflash_completion_fn_t *completion,
				   void *completion_ref)  // argument to be passed to completion callback
{
	write_data = buffer;
	write_addr = addr;
	write_len = len;
	spiflash_completion = completion;
	spiflash_completion_ref = completion_ref;
	spiflash_write_busy = true;

	spiflash_multiple_byte_command(sizeof(dataflash_cmd_disable_sector_protection), dataflash_cmd_disable_sector_protection,
								   0, NULL, // rx
								   false, // hold_cs_active
								   dataflash_rmw_completion1,
								   NULL);

	// if called synchronously, wait for entire write sequence to complete
	if (! completion)
		while (spiflash_write_busy)
			enter_low_power_state();
}


/***************************************************************************//**
 * @brief
 * 		Send Write Enable/Disable Command
 * @param[in] *ref
 * 		Completion Function Pointer
 ******************************************************************************/
void spiflash_set_write_enable(bool enable,
					           spiflash_completion_fn_t *completion,
                               void *completion_ref)
{
	spiflash_single_byte_command(enable ? CMD_WRITE_ENABLE : CMD_WRITE_DISABLE,
			                     0, NULL,  // rx
			                     completion,
			                     completion_ref);
}

/***************************************************************************//**
 * @brief
 * 		Send Sector Protect/Unprotect Command
 * @param[in] protect
 * 		True/False
 * @param[in] addr
 * 		Address/Sector to protect
  * @param[in] *completion
 * 		Completion Function pointer.
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void spiflash_set_sector_protection(bool protect,
                                    uint32_t addr,
		                            spiflash_completion_fn_t *completion,
                                    void *completion_ref)
{
	spiflash_command_with_address(protect? CMD_PROTECT_SECTOR : CMD_UNPROTECT_SECTOR,
			                      addr,
			                      0, // dummy bytes
			                      0, NULL,  // tx
			                      0, NULL,  // rx
			                      completion,
			                      completion_ref);
}


/***************************************************************************//**
 * @brief
 * 		Send Global Protect Command
 * @param[in] protect
 * 		True/False
  * @param[in] *completion
 * 		Completion Function pointer.
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void spiflash_set_global_protect(bool protect,
								 spiflash_completion_fn_t *completion,
								 void *completion_ref)
{
	spiflash_scratch_buf[0] = CMD_WRITE_STATUS_REG_BYTE_1;
	spiflash_scratch_buf[1] = protect ? 0x3c : 0x00;

	spiflash_multiple_byte_command(2, spiflash_scratch_buf,  // tx
			                       0, NULL, // rx
			                       false,  // hold_cs_active
			                       completion,
			                       completion_ref);
}

/***************************************************************************//**
 * @brief
 * 		Send Reset Command
 * @note
 * 		Currently not being used...untested
 * 		The reset command will be ignored by the flash chip unless it has been
 *		previously enabled by writing a 1 to the RSTE bit of the
 *		status register
 *
 * @param[in] *completion
 * 		Completion Function pointer.
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void spiflash_reset(spiflash_completion_fn_t *completion,
	                void *completion_ref)
{
	spiflash_scratch_buf[0] = CMD_RESET;
	spiflash_scratch_buf[1] = ARG_RESET;

	spiflash_multiple_byte_command(2, spiflash_scratch_buf,  // tx
			                       0, NULL, // rx
			                       false,  // hold_cs_active
			                       completion,
			                       completion_ref);
}

/***************************************************************************//**
 * @brief
 * 		Send Read OTP command
 * @note
 * 		Currently not being used...untested
 * @param[in] addr
 * 		OTP Address to read
 * @param[in] len
 * 		How many OTP bytes to read
 * @param[in]
 * 		Pointer to buffer to store read bytes.
 * @param[in] *completion
 * 		Completion Function pointer.
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void spiflash_read_otp(uint32_t addr,
		               size_t   len,
		               uint8_t  *buffer,
		               spiflash_completion_fn_t *completion,
		               void *completion_ref)
{
	spiflash_command_with_address(CMD_READ_OTP,
                                  addr,
                                  2,  // dummy bytes
                                  0, NULL,      // tx
			                      len, buffer,  // rx
			                      completion,
			                      completion_ref);
}

/***************************************************************************//**
 * @brief
 * 		Send Write OTP command
 * @note
 * 		Currently not being used...untested
 * @param[in] addr
 * 		OTP Address to write
 * @param[in] len
 * 		How many OTP bytes to write
 * @param[in] *buffer
 * 		Pointer to buffer with data to write.
 * @param[in] *completion
 * 		Completion Function pointer.
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void spiflash_write_otp(uint32_t addr,
		                size_t   len,
		                uint8_t  *buffer,
		                spiflash_completion_fn_t *completion,
		                void *completion_ref)
{
	spiflash_command_with_address(CMD_PROGRAM_OTP,
	                              addr,
	                              0, // dummy bytes
				                  len, buffer,  // tx
				                  0, NULL,      // rx
				                  spiflash_write_completion1,
				                  NULL);
}

/***************************************************************************//**
 * @brief
 * 		Read Status Register
 * @note
 * 		For Adesto AT25XE021A and AT25XE041B, there are two bytes which can
 * 		be read with a single call with len == 2.
 * @param[in] len
 * 		Number bytes to read
 * @param[in] *buffer
 * 		Pointer to buffer store read bytes to.
 * @param[in] *completion
 * 		Completion Function pointer.
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void spiflash_read_status(size_t len,
		                  uint8_t *buffer,
		                  spiflash_completion_fn_t *completion,
        		          void *completion_ref)
{
	spiflash_single_byte_command(spiflash_info->read_status_cmd,
			                     len,
			                     buffer,
			                     completion,
			                     completion_ref);
}

/***************************************************************************//**
 * @brief
 * 		Write First Status Register
 * @param[in] data
 * 		Byte to write to status register
 * @param[in] *completion
 * 		Completion Function pointer.
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void spiflash_write_status(uint8_t data,
                           spiflash_completion_fn_t *completion,
	        		       void *completion_ref); // argument to be passed to completion callback

/***************************************************************************//**
 * @brief
 * 		Write Second Status Register
 * @param[in] data
 * 		Byte to write to status register
 * @param[in] *completion
 * 		Completion Function pointer.
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void spiflash_write_status2(uint8_t data,
                            spiflash_completion_fn_t *completion,
	        		        void *completion_ref); // argument to be passed to completion callback

/***************************************************************************//**
 * @brief
 * 		Read Device ID
 * @param[in] len
 * 		Byte to write to status register
 * @param[in] *buffer
 * 		Pointer to buffer to store Device ID
 * @param[in] *completion
 * 		Completion Function pointer.
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void spiflash_read_id(size_t len,
		              uint8_t *buffer,
		              spiflash_completion_fn_t *completion,
        		      void *completion_ref)
{
	spiflash_single_byte_command(CMD_READ_ID,
			                     len, buffer, // rx
			                     completion,
			                     completion_ref);
}

/***************************************************************************//**
 * @brief
 * 		Enter/Resume from Deep PowerDown
 * @param[in] power_down
 * 		True=Enter Deep PowerDown, False=Resume from Deep PowerDown
 * @param[in] *completion
 * 		Completion Function pointer.
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
void spiflash_deep_power_down(bool power_down,
		                      spiflash_completion_fn_t *completion,
	                          void *completion_ref)
{
	spiflash_single_byte_command(power_down ? CMD_DEEP_POWER_DOWN : CMD_RESUME_FROM_DEEP_POWER_DOWN,
			                     0, NULL,  // rx
			                     completion,
			                     completion_ref);
}

/***************************************************************************//**
 * @brief
 * 		Enter/Resume from Ultra Deep PowerDown
 * @param[in] power_down
 * 		True=Enter Ultra Deep PowerDown, False=Resume from Ultra Deep PowerDown
 * @param[in] *completion
 * 		Completion Function pointer.
 * @param[in] *completion_ref
 * 		Argument to be passed to completion callback function
 ******************************************************************************/
//Puts device IN/OUT ULTRA DEEP POWER DOWN, INPUT:  Boolean TRUE/FALSE
void spiflash_ultra_deep_power_down(bool power_down,
		                            spiflash_completion_fn_t *completion,
	                                void *completion_ref)
{
	// any command can be used to resume; device will otherwise ignore the cmd
	spiflash_single_byte_command(power_down ? CMD_ULTRA_DEEP_POWER_DOWN : CMD_RESUME_FROM_DEEP_POWER_DOWN,
			                     0, NULL,  // rx
			                     completion,
			                     completion_ref);
}

/***************************************************************************//**
 * @brief
 * 		Determines if Device is DataFlash or note
 * @return TRUE/FALSE
 * 		Looks up in Device table if device is Dataflash type: True/False
 ******************************************************************************/

bool spiflash_is_dataflash(void)
{
	return spiflash_info->dataflash;
}

/***************************************************************************//**
 * @brief
 * 		Get DataFlash Page Size
 ******************************************************************************/

uint32_t dataflash_get_page_size(void)
{
	uint8_t status_buf[2];

	if (! spiflash_info->dataflash)
		return false;

	spiflash_read_status(sizeof(status_buf), status_buf, NULL, NULL);

	if (status_buf[0] & 1)
		return 256;
	else
		return 264;
}

/***************************************************************************//**
 * @brief
 * 		Set DataFlash Page Size
 * @param[in] page_size
 * 		Page size to set for DataFlash
 ******************************************************************************/

bool dataflash_set_page_size(uint32_t page_size)
{
	const uint8_t *cmd;
	uint8_t status_buf[2];
	int cmd_len;

	if (! spiflash_info->dataflash)
		return false;
	if (page_size == 256)
	{
		cmd = dataflash_cmd_set_256b_page;
		cmd_len = sizeof(dataflash_cmd_set_256b_page);
	}
	else if (page_size == 264)
	{
		cmd = dataflash_cmd_set_264b_page;
		cmd_len = sizeof(dataflash_cmd_set_264b_page);
	}
	else
		return false;

	spiflash_multiple_byte_command(cmd_len, cmd,  // tx
			                       0, NULL, // rx
			                       false,  // hold_cs_active
			                       NULL,
			                       NULL);

	do
		spiflash_read_status(sizeof(status_buf), status_buf, NULL, NULL);
	while ((status_buf[0] & spiflash_info->status_busy_mask) == spiflash_info->status_busy_level);

	if (status_buf[0] & 1)
		return page_size == 256;
	else
		return page_size == 264;
}

/***************************************************************************//**
 * @brief
 * 		Initialize SPI Bus, Read Device ID, Determine Device properties
 * @param[in] bit_rate
 * 		Sets SPI Bit Rate
 ******************************************************************************/
//SPI Port Initialization:  Reads Device ID, Verify if it is Device we support
spiflash_id_t spiflash_init(int bit_rate)
{
	int i;
	const spiflash_info_t *p;

	spi_init(bit_rate);
	spiflash_busy = false;
	spiflash_info = NULL;

	// issue a resume from deep power down synchronously
	spiflash_deep_power_down(false, NULL, NULL);
	spiflash_deep_power_down(false, NULL, NULL);

	// issue a read ID command synchronously
	spiflash_read_id(MAX_FLASH_ID_LEN, spiflash_scratch_buf, NULL, NULL);

	for (i = 0; i < SPIFLASH_INFO_TABLE_SIZE; i++)
	{
		p = & spiflash_info_table[i];
		if (memcmp(spiflash_scratch_buf, p->id_bytes, p->id_size) == 0)
		{
			spiflash_info = p;
			return i;
		}
	}

	return PART_UNKNOWN;
}

/***************************************************************************//**
 * @brief
 * 		Determine smallest Erase Page Size
 * @param[in] size
 * 	  	Erase size
 ******************************************************************************/

uint32_t spiflash_smallest_erase_size_above(uint32_t size)
{
	int i;
	const erase_info_t *ei;

	for (i = 0; i < spiflash_info->erase_info_count; i++)
	{
		ei = & spiflash_info->erase_info [i];
		if (ei->size > size)
			return ei->size;
	}
	return 0;
}

/** @} (end addtogroup Adesto_FlashDrivers) */
