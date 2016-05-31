/****************************************************************************//**
 * @file demo_serial.c
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
#include <stdio.h>

#include "em_cmu.h"

#include "demo_serial.h"
#include "hex_dump.h"
#include "main.h"
#include "serial.h"
#include "spiflash.h"

/***************************************************************************//**
 * @addtogroup Serial_Demo
 * @{
 ******************************************************************************/
/***************************************************************************//**
 * @addtogroup Serial_Demo_Functions
 * @brief Functions for Serial Output Demo.
 * @{
 ******************************************************************************/


/**************************************************************************//**
 * @brief read and print flash ID
 *****************************************************************************/
void read_id(void)
{
  uint8_t id[4];
  spiflash_read_id(sizeof(id),
		           id,
			       NULL,
	        	   NULL);
  printf("ID:");
  hex_dump(id, sizeof(id), 0);
  printf("\r\n");
}

/***************************************************************************//**
 * @brief
 *   Create a buffer of all 1's
 *
 * @param[in] *buf
 * 		Pointer to buffer.
 * @param[in] len
 * 		Size of Buffer
 * @return true if successful
 *
 ******************************************************************************/
bool all_ones(uint8_t *buf, size_t len)
{
  size_t i;
  for (i = 0; i < len; i++)
	  if (buf[i] != 0xff)
		  return false;
  return true;
}

/***************************************************************************//**
 * @brief
 *   Read Status Register of Device, Clear UART Tx Buffer
 *
 ******************************************************************************/
void get_status(void)
{
  uint8_t status[2];

  spiflash_read_status(2, status, NULL, NULL);
  printf("status = %02x %02x\r\n", status[0], status[1]);
  serial_tx_flush();
}

/***************************************************************************//**
 * @brief
 *   Set WRITE ENABLE, Clear UART Tx Buffer, Get Status
 *
 ******************************************************************************/
void write_enable(void)
{
  printf("setting write enable\r\n");
  serial_tx_flush();
  spiflash_set_write_enable(true, NULL, NULL);
  printf("write enabled\r\n");
  serial_tx_flush();

  get_status();
}


/***************************************************************************//**
* @note
* 	 	These serial I/O buffers should never be accessed directly,
*		but only through the buffer.h API. They are circular buffers, and part
*		of the allocation is carved out for buffer control.
*
 ******************************************************************************/
static uint8_t raw_serial_rx_buf[200];
static uint8_t raw_serial_tx_buf[1000];

/*  */

/***************************************************************************//**
* @brief
*   Serial Output Demo, Prints out Data Wrote and Read back from Device
*
 ******************************************************************************/
void demo_serial(void)
{
	int i;
	int hex_dump_size = 256;

	CMU_ClockEnable(cmuClock_CORELE, true);
	CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
	CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);

	serial_init(9600,
			    raw_serial_rx_buf, sizeof(raw_serial_rx_buf),
			    raw_serial_tx_buf, sizeof(raw_serial_tx_buf));
	printf("\r\nEmbedded Masters SPI Flash Demo\r\n");

	if (! spiflash_init(2000000))
	  {
		printf("SPI flash failed to initialize\r\n");
		while(true)
			;
	  }

	get_status();

	printf("reading\r\n");
	spiflash_read(0x00000, sizeof(buf1), buf1, NULL, NULL);
	printf("read done\r\n");
	serial_tx_flush();

	printf("data read");
	if (hex_dump_size != sizeof(buf1))
		printf(", first %d bytes", hex_dump_size);
	printf (":\r\n");
	hex_dump(buf1, hex_dump_size, 0);
	serial_tx_flush();

	printf("erasing\r\n");
	spiflash_erase(0x00000, sizeof(buf1), 0, true, NULL, NULL);
	printf("erase done\r\n");
	serial_tx_flush();

	get_status();

	printf("setting buffer to increment from %02x\r\n", buf1[0]+1);
	buf2[0]++;
	for (i = 1; i < sizeof(buf2); i++)
		buf2[i] = buf2[i-1] + 1;
	serial_tx_flush();

	write_enable();

	printf("setting sector unprotect\r\n");
	spiflash_set_sector_protection(false, 0x00000, NULL, NULL);

	get_status();

	printf("writing\r\n");
	serial_tx_flush();
	spiflash_write(0x00000, sizeof(buf2), buf2, true, NULL, NULL);
	printf("write done\r\n");
	serial_tx_flush();

	get_status();

	printf("reading\r\n");
	serial_tx_flush();
	spiflash_read(0x00000, sizeof(buf3), buf3, NULL, NULL);
	printf("read done\r\n");
	serial_tx_flush();

	printf("data read");
	if (hex_dump_size != sizeof(buf1))
		printf(", first %d bytes", hex_dump_size);
	printf (":\r\n");
	serial_tx_flush();
	hex_dump(buf3, hex_dump_size, 0);

	spiflash_ultra_deep_power_down(true, NULL, NULL);

	serial_tx_flush();
	serial_close();
}

/** @} (end addtogroup Serial_Demo_Functions) */
/** @} (end addtogroup Serial_Demo) */
