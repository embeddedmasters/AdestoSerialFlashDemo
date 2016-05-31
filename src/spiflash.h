/****************************************************************************//**
 * @file spiflash.h
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

#ifndef SPIFLASH_H_
#define SPIFLASH_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>

/*****************************************************************************/
/** @defgroup Adesto_FlashDrivers      Adesto_FlashDrivers
 */
/*****************************************************************************/

/***************************************************************************//**
 * @addtogroup Adesto_FlashDrivers
 * @{
 ******************************************************************************/

typedef enum
{
	AT25SF041,
	AT25XE021A,
	AT25XE041B,
	AT45DB081E,
	AT45DB641E,
	RM25C256DS,
	PART_UNKNOWN
} spiflash_id_t;

#define MAX_FLASH_ID_LEN 8

typedef struct
{
	size_t size;
	uint8_t cmd;
	bool addr_needed;
} erase_info_t;

#define MAX_ERASE_SIZES 5

typedef struct
{
	const char *name;

	size_t id_size;
	const uint8_t id_bytes [MAX_FLASH_ID_LEN];
	size_t device_size;
	int address_bytes;  // only values supported are 2 and 3
	size_t program_page_size;

	// most parts have multiple erase commands of various sizes
	int erase_info_count;
	const erase_info_t erase_info[MAX_ERASE_SIZES];

	const size_t *protection_sector_sizes;
	unsigned int protection_sector_count;

	uint8_t read_status_cmd;
	uint8_t status_busy_mask;
	uint8_t status_busy_level;

	bool read_slow;  // if true, use READ ARRAY SLOW command with no dummy byte
	bool has_so_irq;
	bool dataflash;     // if true, part has 256 byte and 264 byte page capability
	uint8_t so_done_level;  // level expected on SO when operation done, when using active status interrupt
} spiflash_info_t;

extern const spiflash_info_t spiflash_info_table[];

typedef void spiflash_completion_fn_t(void *ref);


void spiflash_single_byte_command(uint8_t cmd,
                                  size_t rx_len,
                                  uint8_t *rx_buf,
                                  spiflash_completion_fn_t *completion,
                                  void *completion_ref);

void spiflash_multiple_byte_command(size_t tx_len,
		                            const uint8_t *tx_buf,
		                            size_t rx_len,
		                            uint8_t *rx_buf,
		                            bool hold_cs_active,
                                    spiflash_completion_fn_t *completion,
                                    void *completion_ref);

void spiflash_read(uint32_t addr,
		           size_t len,
		           uint8_t  *buffer,
		           spiflash_completion_fn_t *completion,
		           void *completion_ref);  // argument to be passed to completion callback

bool spiflash_erase(uint32_t addr,
			        size_t len,
			        uint32_t cmd_size,  // bytes per erase command, 0 for auto
			        bool use_so_irq,
		            spiflash_completion_fn_t *completion,
		            void *completion_ref);  // argument to be passed to completion callback

void spiflash_write(uint32_t addr,
		            size_t len,
		            uint8_t  *buffer,
		            bool use_so_irq,
		            spiflash_completion_fn_t *completion,
		            void *completion_ref);  // argument to be passed to completion callback

void spiflash_set_write_enable(bool enable,
				               spiflash_completion_fn_t *completion,
				               void *completion_ref);

void spiflash_set_sector_protection(bool protect,
									uint32_t addr,
                                    spiflash_completion_fn_t *completion,
                                    void *completion_ref);

void spiflash_set_global_protect(bool protect,
								 spiflash_completion_fn_t *completion,
								 void *completion_ref);

void spiflash_read_otp(uint32_t addr,
		               size_t   len,
		               uint8_t  *buffer,
		               spiflash_completion_fn_t *completion,
		               void *completion_ref);  // argument to be passed to completion callback

void spiflash_write_otp(uint32_t addr,
		                size_t   len,
		                uint8_t  *buffer,
		                spiflash_completion_fn_t *completion,
		                void *completion_ref);  // argument to be passed to completion callback

void spiflash_read_status(size_t len,
		                  uint8_t *buffer,
		                  spiflash_completion_fn_t *completion,
        		          void *completion_ref); // argument to be passed to completion callback

void spiflash_write_status(uint8_t data,
                           spiflash_completion_fn_t *completion,
	        		       void *completion_ref); // argument to be passed to completion callback

void spiflash_write_status2(uint8_t data,
                            spiflash_completion_fn_t *completion,
	        		        void *completion_ref); // argument to be passed to completion callback

// Read the manufacturer and device ID of the flash chip.
void spiflash_read_id(size_t len,
		              uint8_t *buffer,
		              spiflash_completion_fn_t *completion,
        		      void *completion_ref);

void spiflash_reset(spiflash_completion_fn_t *completion,
	                void *completion_ref);

void spiflash_deep_power_down(bool power_down,
		                      spiflash_completion_fn_t *completion,
	                          void *completion_ref);

void spiflash_ultra_deep_power_down(bool power_down,
		                            spiflash_completion_fn_t *completion,
	                                void *completion_ref);

uint32_t dataflash_get_page_size(void);

bool dataflash_set_page_size(uint32_t page_size);

spiflash_id_t spiflash_init(int bit_rate);

uint32_t spiflash_smallest_erase_size_above(uint32_t size);

bool spiflash_is_dataflash(void);

void dataflash_rmw(uint32_t addr,
				   size_t len,
				   uint8_t  *buffer,
				   spiflash_completion_fn_t *completion,
				   void *completion_ref);  // argument to be passed to completion callback

/** @} (end addtogroup Adesto_FlashDrivers) */

#endif /* SPIFLASH_H_ */
