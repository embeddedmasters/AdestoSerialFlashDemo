/******************************************************************************
 * @file main.c
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

/**************************************************************************//**
 * @verbatim SPI Flash Demo Pin Connections
 * Pin Configurations for EFM32 STK to EMSESNSR-WSP with EMMEM
 * Breakout Board.
 *
 *   EFM32     EFM32    STK32
 *   port      port     exp      EMMEM    EMMEM
 *   function  pin      pin      pin      function
 *   --------  -------  -------  -------  --------
 *   Vmcu               2        P1-1     Vdd
 *   GND                1,19     P1-2     GND
 *                      n/c      P1-3     n/c
 *                      n/c      P1-4     /WP     EMMEM has jumper to Vdd
 *   GPIO      PB11     11       P1-5     /HOLD   EMMEM has jumper to Vdd
 *
 *   US1_TX    PD0      4        P2-1     SDI
 *   US1_CLK   PD2      8        P2-2     SCLK
 *   US1_RX    PD1      6        P2-3     SDO
 *   US1_CS    PD3      10       P2-4     nCS
 *                      n/c      P2-5     n/c
 *
 *
 * Connections from Silicon Labs starter kit to host via LEUART.
 *
 *   EFM32     EFM32    STL32    232R      232R
 *   port      port     exp      cable     cable
 *   function  pin      pin      pin       function
 *   --------  -------  -------  --------  --------
 *   GND                1,19     1         GND
 *   LEU0_TX0  PD4      12       5         RXD
 *   LEU0_RX0  PD5      14       4         TxD
 *
 *   @endverbatim
 */

#include <inttypes.h>
#include <stdbool.h>
#include <string.h>

#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_emu.h"
#include "em_gpio.h"

#include "caplesense.h"
#include "rtcdriver.h"
#include "segmentlcd.h"

#include "button.h"
#include "delay.h"
#include "demo_serial.h"
#include "fatal.h"
#include "gpio.h"
#include "lcdtest.h"
#include "lcd_scroll.h"
#include "led.h"
#include "low_power.h"
#include "main.h"
#include "oneshot.h"
#include "serial.h"
#include "spiflash.h"

/***************************************************************************//**
 * @addtogroup MAIN
 * @{
 ******************************************************************************/


CMU_Select_TypeDef main_osc;  // cmuSelect_HFRCO or cmuSelect_HFXO
uint32_t spi_freq;  // in Hz

spiflash_id_t part;

/****************************************************************************//**
 *
 * @brief  Initialize SPI Module and read Device ID
 *
 *****************************************************************************/

void spiflash_setup(void)
{
	part = spiflash_init(spi_freq);
	if (part == PART_UNKNOWN)
		fatal("unrecognized flash");
}

static bool use_so;
static bool erase_size_choices_initialized;
static uint32_t erase_size;
static bool do_verify;

int duration;

uint32_t data_written_byte_count;


uint8_t buf1[BUFFER_SIZE];
uint8_t buf2[BUFFER_SIZE];
uint8_t buf3[BUFFER_SIZE];


#define SLIDER_MAX 48

#define MAX_NUMERIC_CHOICES 8


typedef void sm_fn_t(void);

typedef enum
{
	state_init,
	state_id,
	state_config,
	state_erase,
	state_write,
	state_read,
	state_rmw,
	state_appl,
	state_powerdn,
	state_serial,

	state_conf_so,
	state_conf_erase_size,
	state_conf_verify,
	state_conf_spi_clk,

	state_message,

	STATE_MAX
} state_t;

typedef struct
{
	const char *name;
	sm_fn_t *enter_fn;
	sm_fn_t *leave_fn;
	sm_fn_t *idle_fn;
	sm_fn_t *button0_fn;
	state_t next;
	sm_fn_t *button1_fn;
	sm_fn_t *preflight_fn;
	sm_fn_t *run_fn;
	sm_fn_t *numeric_choices_init_fn;
	const int numeric_choices_fixed_count;
	const int32_t numeric_choices_fixed [MAX_NUMERIC_CHOICES];
} state_info_t;

sm_fn_t enter_default;
sm_fn_t idle_default;
sm_fn_t button0_default;
sm_fn_t button1_default;

sm_fn_t enter_init;

sm_fn_t enter_id;

sm_fn_t button1_config;

sm_fn_t enter_erase;
sm_fn_t run_erase;

sm_fn_t run_read;

sm_fn_t preflight_program;
sm_fn_t run_program;

sm_fn_t preflight_rmw;
sm_fn_t run_rmw;

sm_fn_t run_appl;

sm_fn_t run_powerdn;

sm_fn_t run_serial;

sm_fn_t enter_conf_so;
sm_fn_t button1_conf_so;

sm_fn_t numeric_choices_init_conf_erase_size;
sm_fn_t leave_conf_erase_size;

sm_fn_t enter_conf_verify;
sm_fn_t button1_conf_verify;

sm_fn_t leave_conf_spi_clk;

sm_fn_t enter_message;
sm_fn_t button1_message;

char *message_text;
int message_number;

static const state_info_t state_info[STATE_MAX] =
{
	// start of main menu
	[state_id]           = { .name         = "ID",
						     .enter_fn     = enter_id },

						     [state_config]       = { .name         = "CONFIG",
						     .button1_fn   = button1_config },
	[state_erase]        = { .name         = "ERA",
						     .enter_fn     = enter_erase,
						     .run_fn       = run_erase,
						     .numeric_choices_fixed_count  = 5,
						     .numeric_choices_fixed      = { 16, 32, 64, 128, 256 }},
	[state_write]        = { .name         = "WRITE",
						     .preflight_fn = preflight_program,
						     .run_fn       = run_program,
						     .numeric_choices_fixed_count  = 5,
						     .numeric_choices_fixed      = { 16, 32, 64, 128, 256 }},
	[state_read]         = { .name         = "READ",
						     .run_fn       = run_read,
						     .numeric_choices_fixed_count  = 5,
						     .numeric_choices_fixed      = { 16, 32, 64, 128, 256 }},
	[state_rmw]          = { .name         = "RMW",
							 .preflight_fn = preflight_rmw,
			                 .run_fn       = run_rmw,
                             .numeric_choices_fixed_count = 4,
	                         .numeric_choices_fixed      = { 100, 200, 400, 800 }},
	[state_appl]         = { .name         = "APPL",
					    	 .run_fn       = run_appl,
					    	 .numeric_choices_fixed_count  = 5,
					    	 .numeric_choices_fixed      = { 16, 32, 64, 128, 256 }},
	[state_powerdn]      = { .name         = "POWERDN",
					    	 .run_fn       = run_powerdn,
							 .numeric_choices_fixed_count = 2,
							 .numeric_choices_fixed = {0, 1}},
	[state_serial]       = { .name         = "SERIAL",
			                 .run_fn       = run_serial,
						     .next         = state_id },
	// end of main menu

 	// start of config menu
   	[state_conf_so]         = { .name         = "SO",
   						        .enter_fn     = enter_conf_so,
   						        .button1_fn   = button1_conf_so },
   	[state_conf_erase_size] = { .name         = "ERA SZ",
	        				    .leave_fn     = leave_conf_erase_size,
	        				    .numeric_choices_init_fn = numeric_choices_init_conf_erase_size },
    [state_conf_verify]     = { .name         = "VFY",
   						        .enter_fn     = enter_conf_verify,
   					   	        .button1_fn   = button1_conf_verify },
   	[state_conf_spi_clk]    = { .name         = "SPI CLK",
            			        .leave_fn     = leave_conf_spi_clk,
            			        .next         = state_config,
            			        .numeric_choices_fixed_count  = 2,
   			                    .numeric_choices_fixed      = { 2, 12 }},
   	// end of config menu

	[state_message]      = { .name         = "MESSAGE",
						     .enter_fn     = enter_message,
						     .button0_fn   = button1_message,
						     .button1_fn   = button1_message },
};

static int32_t state_slider_position[STATE_MAX];

static int32_t numeric_choices_count[STATE_MAX];
static int32_t numeric_choices[STATE_MAX][MAX_NUMERIC_CHOICES];


typedef struct
{
	bool state;
	bool pressed;
	bool released;
} button_info_t;

static button_info_t button_info[2];


/***************************************************************************//**
 * @brief
 *   Initialize Buffer with data to be used with Demo
 *
 * @param[in] buffer_offset
 * 		Offset into Buffer
 * @param[in] len
 * 		Length of Buffer
 * @param[in] seed
 * 		Seed to help randomize data
 *
 ******************************************************************************/
void init_buffer(uint32_t buffer_offset, uint32_t len, uint32_t seed)
{
	int i;
	uint8_t initial;

	initial = (seed >> 24) ^ (seed >> 16) ^ (seed >> 8) ^ seed;
	for (i = 0; i < sizeof(buf1); i++)
		buf1[buffer_offset + i] = initial + i;
}

int slider_num_choices;
const int32_t *slider_choices;
int32_t slider_threshold[MAX_NUMERIC_CHOICES];

/***************************************************************************//**
 * @brief
 *   Initialize Thresholds for Slider on STK
 *
 * @param[in] num_choices
 * 		Number of Choices available with slider
 * @param[in] choices
 * 		Available Choices with Slider
 *
 ******************************************************************************/
void slider_init_thresholds(int num_choices, const int32_t *choices)
{
	int i;

	slider_num_choices = num_choices;
	slider_choices = choices;
	for (i = 0; i < num_choices; i++)
		slider_threshold[i] = ((i + 1) * SLIDER_MAX) / num_choices;
}

/***************************************************************************//**
 * @brief
 *   Map value passed in from Slider Library to SPI Flash Demo values
 *
 * @param[in] slider_position
 * 		Position of Finger on Slider
 *
 ******************************************************************************/
int32_t slider_get_choice(int32_t slider_position)		//Map value passed in from Slider Library to SPI Flash Demo values
{
	int i;
	for (i = 0; i < (slider_num_choices - 1); i++)
		if (slider_position < slider_threshold[i])
			return slider_choices[i];
	return slider_choices[i];
}

/***************************************************************************//**
 * @brief
 *   Callback function for Button Press detection on STK for Demo Menu
 *
 * @param[in] button
 * 		Button Number
 * @param[in] pressed
 * 		 Is button Pressed or not
 *
 ******************************************************************************/
void spiflash_button_callback(int button, bool pressed)
{
	button_info_t *info = & button_info[button];
	if (pressed == info->state)
		return;

	if (pressed)
		info->pressed = true;
	else
		info->released = true;
}

state_t state;
state_t prev_state;
state_t message_return_state;

/***************************************************************************//**
 * @brief
 *   Determines Demo State based on button press
 * @note
 * 		This is the primary State Machine for the Serial Flash Demo
 *
 ******************************************************************************/
void spiflash_demo(void)		//State machine main loop
{
	prev_state = STATE_MAX;
	state = state_id;

	while (true)
	{
		if (state != prev_state)
		{
			if ((prev_state < STATE_MAX) && state_info[prev_state].leave_fn)
				state_info[prev_state].leave_fn();
			prev_state = state;
			if (state_info[state].enter_fn)
				state_info[state].enter_fn();
			else
				enter_default();
		}
		else if (button_info[0].pressed)
		{
			button_info[0].pressed = false;
			if (state_info[state].button0_fn)
				state_info[state].button0_fn();
			else
				button0_default();
		}
		else if (button_info[1].pressed)
		{
			button_info[1].pressed = false;
			if (state_info[state].button1_fn)
				state_info[state].button1_fn();
			else
				button1_default();
		}
		else
		{
			if (state_info[state].idle_fn)
				state_info[state].idle_fn();
			else
				idle_default();
		}
	}
}

uint8_t ts_status_buf_1 [2];

/***************************************************************************//**
 * @brief
 *   Starts the Demo creates an initial energy pulse by tuning ON/OFF the LED
 *
 ******************************************************************************/
void test_start(void)
{

	// make sure SPI flash is in ultra deep power down
//	spiflash_ultra_deep_power_down(true, NULL, NULL);

	// disable LCD
	SegmentLCD_AllOff();
	SegmentLCD_Disable();


	CAPLESENSE_Init(true); // Disable slider sensing

	// pulse LED on 50ms
	led_set(0, 1);
	delay(50);
	led_set(0, 0);

	// delay 50ms
	delay(50);
	// wake part up
	spiflash_ultra_deep_power_down(false, NULL, NULL);		//XXX verify if we need both of these...?
	spiflash_ultra_deep_power_down(false, NULL, NULL);

	// Necessary for erase and write commands.
	// XXX could add a bool field to state_info to control whether this is done
	spiflash_set_write_enable(true, NULL, NULL);
	spiflash_set_global_protect(false, NULL, NULL);

	spiflash_read_status(2, ts_status_buf_1, NULL, NULL);
}


/***************************************************************************//**
 * @brief
 *   Called after chosen demo has completed.  Creates final energy pulse by toggling LED
 *
 ******************************************************************************/
void test_stop(void)
{
	// put part in ultra deep power down
	spiflash_ultra_deep_power_down(true, NULL, NULL);

	delay(50); // delay 50ms

	// pulse LED on 50ms
	led_set(0, 1);
	delay(50);
	led_set(0, 0);

	// enable LCD
	SegmentLCD_Init(false); // Enable LCD without voltage boost

	// force display update
	prev_state = STATE_MAX;
}

/***************************************************************************//**
 * @brief
 *   Determines numeric choices available on STK slider based on Demo/State
 *
 ******************************************************************************/
void numeric_choices_init(void)
{
	if (state_info[state].numeric_choices_init_fn)
		state_info[state].numeric_choices_init_fn();
	else
	{
		numeric_choices_count[state] = state_info[state].numeric_choices_fixed_count;
		memcpy(numeric_choices[state], state_info[state].numeric_choices_fixed, sizeof(numeric_choices[state]));
	}

	if (numeric_choices_count[state])
	{
		  CAPLESENSE_Init(false); // Init slider sensing
		  slider_init_thresholds(numeric_choices_count[state],
				  	  	  	  	 numeric_choices[state]);
	}
	else
	{
		SegmentLCD_NumberOff();
		CAPLESENSE_Init(true); // Not slider sensing
	}
}

/***************************************************************************//**
 * @brief
 *   If State Table has NULL entry these values get plugged in
 *
 ******************************************************************************/
void enter_default(void)
{
	SegmentLCD_Write((char *) state_info[state].name);
	// cast because SegmentLCD_Write doesn't have const qualifier
	numeric_choices_init();
}

/***************************************************************************//**
 * @brief
 *   Used to manage SLIDER and LCD Scroll
 *
 ******************************************************************************/
void idle_default(void)
{
	if (numeric_choices_count[state])
	{
		int32_t val;
		val = CAPLESENSE_getSliderPosition();
		if (val >= 0)
			state_slider_position[state] = val;
		SegmentLCD_Number(slider_get_choice(state_slider_position[state]));
	}
	else
	{
	    lcd_scroll_update();
	}
    EM2Sleep(325);
}

/***************************************************************************//**
 * @brief
 *   Used to Advance to next state
 *
 ******************************************************************************/
void button0_default(void)
{
    lcd_scroll_stop();
    if (state_info[state].next)
    	state = state_info[state].next;
    else
	    state += 1;
}

/***************************************************************************//**
 * @brief
 *   Normally Starts Test/Demo
 *
 ******************************************************************************/
void button1_default(void)
{
	if (! state_info[state].run_fn)
		return;

	// wait for button 1 release
	while (button_info[1].state)
		enter_low_power_state();

	if (state_info[state].preflight_fn)
		state_info[state].preflight_fn();

	test_start();

	state_info[state].run_fn();

	test_stop();
}


/***************************************************************************//**
 * @brief
 *   Increment State/Default
 *
 ******************************************************************************/
void enter_init(void)
{
	// For now, nothing needs to be done here.
	state++;
}

/***************************************************************************//**
 * @brief
 *   Displays Flash ID on LCD
 *
 ******************************************************************************/
void enter_id(void)
{
	lcd_scroll_start(spiflash_info_table[part].name);
	SegmentLCD_NumberOff();
}

/***************************************************************************//**
 * @brief
 *   If in CONFIG MENU if Button1 is pressed Enters SO INTERRUPT Configuration
 *
 ******************************************************************************/
void button1_config(void)
{
	state = state_conf_so;
}

static char erase_msg[7];

/***************************************************************************//**
 * @brief
 *   Demo Menu:  Called when entering Erase Menu, determines appropriate
 *   Erase values and Displays on LCD
 *
 ******************************************************************************/
void enter_erase(void)
{
	uint32_t device_size = spiflash_info_table[part].device_size;

	memcpy(erase_msg, "ER     ", 7);
	if (erase_size == device_size)
		memcpy(erase_msg + 3, "Chip", 4);
	else switch (erase_size)
	{
	case 64:
		memcpy(erase_msg + 3, "64b", 3);
		break;
	case 256:
		memcpy(erase_msg + 3, "256b", 4);
		break;
	case 2048:
		memcpy(erase_msg + 3, "2K", 2);
		break;
	case 4096:
		memcpy(erase_msg + 3, "4K", 2);
		break;
	case 32768:
		memcpy(erase_msg + 3, "32K", 3);
		break;
	case 65536:
		memcpy(erase_msg + 3, "64K", 3);
		break;
	default:
		fatal("unknown erase size");
	}
	SegmentLCD_Write((char *) erase_msg);
	numeric_choices_init();
}

/***************************************************************************//**
 * @brief
 *   	Demo Menu: Runs Erase Demo, Gets Erase size from user selected value on
 *   	slider.
 * @note
 * 		Determines total size of device from Device Table in spiflash.c
 *
 ******************************************************************************/
void run_erase(void)
{
	uint32_t slider = slider_get_choice(state_slider_position[state]);
	uint32_t size = 1024 * slider;
	int32_t count = size;
	data_written_byte_count = 0;

	uint32_t device_size = spiflash_info_table[part].device_size;

	uint32_t addr = 0;

	while (count > 0)
	{
		if (! spiflash_erase(addr, erase_size, erase_size, use_so, NULL, NULL))
			fatal("erase error");
		addr += erase_size;
		if (addr >= device_size)
			addr = 0;
		count -= erase_size;
	}

	message_text = "ER done";
	message_number = slider;
	message_return_state = state;
	state = state_message;
}

/***************************************************************************//**
 * @brief
 *   Fills in Buffe with Data to be programmed into Flash
 *
 ******************************************************************************/
void preflight_program(void)
{
	init_buffer(0, BUFFER_SIZE, 0xdeadbeef);
}

/***************************************************************************//**
 * @brief
 *   Demo Menu: Runs Flash Write Demo  Programs Flash, Gets Program Size from user
 *   selected value on Slider
 * @note
 * 	Determines Device Minimum Page size and Total Size of Device from Device Table
 *  in spiflash.c
 *
 ******************************************************************************/
void run_program(void)
{
	uint32_t slider = slider_get_choice(state_slider_position[state]);
	uint32_t size = 1024 * slider;
	int32_t count = size;
	data_written_byte_count = 0;

	uint32_t program_page_size = spiflash_info_table[part].program_page_size;
	uint32_t device_size = spiflash_info_table[part].device_size;

	uint32_t addr = 0;
	uint32_t buffer_offset = 0;

	while (count > 0)
	{
		if (do_verify)
			init_buffer(buffer_offset, program_page_size, addr);
		spiflash_set_write_enable(true, NULL, NULL);
		spiflash_write(addr, program_page_size, & buf1[buffer_offset], use_so, NULL, NULL);

		addr += program_page_size;
		if (addr >= device_size)
			addr = 0;
		buffer_offset += program_page_size;
		if (buffer_offset >= sizeof(buf1))
			buffer_offset = 0;
		count -= program_page_size;
	}

	data_written_byte_count = addr;

	message_text = "Wr done";
	message_number = slider;
	message_return_state = state;
	state = state_message;
}

/***************************************************************************//**
 * @brief
 *   Demo Menu: Runs Flash Read Demo  Reads Flash, Gets Read Size retrieved from
 *   user selection on Slider
 * @note
 * 		Determines Device Minimum Page size and Total Size of Device from Device Table
 * 		in spiflash.c
 *
 ******************************************************************************/
void run_read(void)
{
	uint32_t slider = slider_get_choice(state_slider_position[state]);
	uint32_t size = 1024 * slider;
	int32_t count = size;

	uint32_t program_page_size = spiflash_info_table[part].program_page_size;
	uint32_t device_size = spiflash_info_table[part].device_size;

	uint32_t addr = 0;
	uint32_t buffer_offset = 0;

	while (count > 0)
	{
		if (do_verify)
			init_buffer(buffer_offset, program_page_size, addr);
		spiflash_read(addr, program_page_size, & buf2[buffer_offset], NULL, NULL);

		addr += program_page_size;

		if (do_verify && (addr <= data_written_byte_count))
		{
			if (memcmp (buf1 + buffer_offset, buf2 + buffer_offset, program_page_size) != 0)
			{
				message_text = "DataErr";
				message_number = addr >> 10;
				if (message_number > 9999)
					message_number = 9999;
				message_return_state = state;
				state = state_message;
				return;
			}
		}

		if (addr >= device_size)
			addr = 0;
		buffer_offset += program_page_size;
		if (buffer_offset >= sizeof(buf2))
			buffer_offset = 0;
		count -= program_page_size;
	}

	message_text = "READ dn";
	message_number = slider;
	message_return_state = state;
	state = state_message;
}


/***************************************************************************//**
 * @brief
 * 		Fills Buffer for RMW Demo to be programmed into Data Flash
 * @note
 *   	RMW command only needs this for dataflash, not for normal SPI flash
 *
 ******************************************************************************/
void preflight_rmw(void)
{
	init_buffer(0, BUFFER_SIZE, 0xdeadbeef);
}

#define RMW_UPDATE_BYTE_COUNT 20

/***************************************************************************//**
 * @brief
 * 	Demo Menu: Runs DataFlash RMW Demo from Main Menu
 * @note
 * 	Gets choice from slider value,  Device Minimum Erase Size, and Total Size
 * 	of device is read from Device Table in spiflash.c
 *
 ******************************************************************************/
void run_rmw_dataflash(void)
{
	uint32_t slider = slider_get_choice(state_slider_position[state]);
	uint32_t size = slider;
	int32_t count = size;

	uint32_t program_page_size = spiflash_info_table[part].program_page_size;
	uint32_t device_size = spiflash_info_table[part].device_size;

	uint32_t addr = 0;
	uint32_t offset = 0;  // offset within block to update

	while (count > 0)
	{
		dataflash_rmw(addr + offset, RMW_UPDATE_BYTE_COUNT, buf1 + offset, NULL, NULL);

		addr += program_page_size;
		if (addr > device_size)
			addr = 0;

		offset += RMW_UPDATE_BYTE_COUNT;
		if ((offset + RMW_UPDATE_BYTE_COUNT) > program_page_size)
			offset = 0;

		count --;
	}

	message_text = "RMW dn";
	message_number = slider;
	message_return_state = state;
	state = state_message;
}

/***************************************************************************//**
 * @brief
 * 	Demo Menu: Runs RMW Demo for 25XE Series from Main Menu
 * @note
 * 	Gets choice from slider value,  Device Minimum Erase Size, and Total Size
 * 	of device is read from Device Table in spiflash.c
 *
 ******************************************************************************/
void run_rmw_normal(void)
{
	uint32_t slider = slider_get_choice(state_slider_position[state]);
	uint32_t size = slider;
	int32_t count = size;

	uint32_t erase_size = spiflash_info_table[part].erase_info[0].size;
	uint32_t device_size = spiflash_info_table[part].device_size;

	uint32_t addr = 0;
	uint32_t offset = 0;  // offset within block to update

	while (count > 0)
	{
		int i;

		spiflash_read(addr, erase_size, buf2, NULL, NULL);

		// modify data
		for (i = 0; i < RMW_UPDATE_BYTE_COUNT; i++)
			buf2[offset + i]++;

		// erase and write back
		spiflash_erase(addr, erase_size, erase_size, use_so, NULL, NULL);
		spiflash_write(addr, erase_size, buf2, use_so, NULL, NULL);

		addr += erase_size;
		if (addr > device_size)
			addr = 0;

		offset += RMW_UPDATE_BYTE_COUNT;
		if ((offset + RMW_UPDATE_BYTE_COUNT) > erase_size)
			offset = 0;

		count --;
	}

	message_text = "RMW dn";
	message_number = slider;
	message_return_state = state;
	state = state_message;
}

/***************************************************************************//**
 * @brief
 * 	Demo Menu: Chooses whether to run RMW for Dataflash or 25XE Series
 *
 ******************************************************************************/
void run_rmw(void)
{
	if (spiflash_is_dataflash())
		run_rmw_dataflash();
	else
		run_rmw_normal();
}

/***************************************************************************//**
 * 	@brief
 * 		Demo Menu: Performs Erase for Application Demo from Main Menu
 * 	@note
 * 		Looks up Device Size from Serial Memory Table
 * 	@param[in] len
 *  	Size of Serial Memory being used.  Looked up from Serial Memory Table
 *
 ******************************************************************************/
void appl_erase(uint32_t len)
{
	uint32_t device_size = spiflash_info_table[part].device_size;
	if (len > device_size)
		len = device_size;

	if (! spiflash_erase(0, len, 0, use_so, NULL, NULL))
		fatal("erase error");
}

/***************************************************************************//**
 * 	@brief
 * 		Demo Menu: Performs Write for Application Demo from Main Menu
 * 	@note
 * 		Looks up Program Page Size and Device Size from Serial Memory Table
 *	@param[in] len
 *		How much memory to write
 ******************************************************************************/
void appl_write(uint32_t len)
{
	uint32_t program_page_size = spiflash_info_table[part].program_page_size;
	uint32_t device_size = spiflash_info_table[part].device_size;
	uint32_t addr = 0;
	uint32_t buffer_offset = 0;
	uint32_t count = len;

	while (count)
	{
		if (addr == 0)
			appl_erase(len - addr);
		init_buffer(buffer_offset, program_page_size, addr);
		spiflash_write(addr, program_page_size, & buf1[buffer_offset], use_so, NULL, NULL);
		data_written_byte_count += program_page_size;
		addr += program_page_size;
		if (addr >= device_size)
			addr = 0;
		buffer_offset += program_page_size;
		if (buffer_offset >= sizeof(buf1))
			buffer_offset = 0;
		count -= program_page_size;
	}
}

/***************************************************************************//**
 * 	@brief
 * 		Demo Menu: Performs Read for Application Demo from Main Menu
 * 	@note
 * 		Looks up Program Page Size and Device Size from Serial Memory Table
 *	@param[in] len
 *		How much memory to read
 ******************************************************************************/
bool appl_read(uint32_t len)
{
	uint32_t program_page_size = spiflash_info_table[part].program_page_size;
	uint32_t device_size = spiflash_info_table[part].device_size;
	uint32_t addr = 0;
	uint32_t buffer_offset = 0;
	uint32_t count = len;

	while (count)
	{
		init_buffer(buffer_offset, program_page_size, addr);
		spiflash_read(addr, program_page_size, & buf2[buffer_offset], NULL, NULL);
		if (memcmp(buf1 + buffer_offset, buf2 + buffer_offset, program_page_size) != 0)
		{
			return false;
		}
		addr += program_page_size;
		if (addr >= device_size)
			addr = 0;
		buffer_offset += program_page_size;
		if (buffer_offset >= sizeof(buf2))
			buffer_offset = 0;
		count -= program_page_size;
	}
	return true;
}

/***************************************************************************//**
 * 	@brief
 * 		Demo Menu: Runs Application Demo from Main Menu
 * 	@note
 * 		Looks up Device Size from Serial Memory Table
 ******************************************************************************/
//TODO There is not an Erase that is occurring in the Application Demo??
//TODO Compare to older versions
void run_appl(void)
{
	uint32_t slider = slider_get_choice(state_slider_position[state]);
	uint32_t size = 1024 * slider;
	uint32_t addr;
	uint32_t device_size = spiflash_info_table[part].device_size;
	uint32_t bytes_per_iter;
	bool read_status;

	if (size > device_size)
		bytes_per_iter = device_size;
	else
		bytes_per_iter = size;

	addr = 0;
	while (size)
	{
		data_written_byte_count = 0;
		appl_write(bytes_per_iter);
		read_status = appl_read(bytes_per_iter);
		if (! read_status)
		{
			message_number = addr / spiflash_info_table[part].program_page_size;
			if (message_number > 9999)
				message_number = 9999;
			break;
		}
		size -= bytes_per_iter;
		addr += bytes_per_iter;
	}

	if (read_status)
	{
		message_text = "Appl dn";
		message_number = slider;
	}
	else
	{
		message_text = "ReadErr";
	}

	message_return_state = state;
	state = state_message;
}

/***************************************************************************//**
 * 	@brief
 * 		Demo Menu: Runs Serial Flash Powerdown Demo from Main Menu
 * 	@note
 * 		Slider position determines whether device is put into DEEP SLEEP or ULTRA
 * 		DEEP SLEEP
 ******************************************************************************/
void run_powerdn(void)
{

	uint32_t slider = slider_get_choice(state_slider_position[state]);
	// test deep power down
	if(slider == 0){
		spiflash_deep_power_down(true, NULL, NULL);
		oneshot_start_s(1);
		while( ! oneshot_done() );
		EMU_EnterEM3(true);
		message_text = "DPD dn";
		spiflash_deep_power_down(false, NULL, NULL);
	}
	else{
		spiflash_ultra_deep_power_down(true, NULL, NULL);
		oneshot_start_s(1);
		while( ! oneshot_done() );
		EMU_EnterEM3(true);
		message_text = "UDPD dn";
		spiflash_ultra_deep_power_down(false, NULL, NULL);
	}
	button_info[1].pressed = false;
	message_number = slider;
	message_return_state = state;
	state = state_message;
}

/***************************************************************************//**
 * 	@brief
 * 		Demo Menu: Runs Serial Output Demo from Main Menu
 * 	@note
 * 		Prints out data that is Wrote/Read via LPUART, can be viewed via terminal Window.
 * 		Use EMCOM-FT230 plugged into USB-UART Slot closest to 20pin Expansion Header.
 *
 ******************************************************************************/
void run_serial(void)
{
	demo_serial();
}

/***************************************************************************//**
 * 	@brief
 * 		CONFIG Menu: Displays Active SO Selection in CONFIG Menu
 * 	@note
 * 		User chooses whether Active SO Interrupt is used by Pressing PB1.
 * 		If the Serial Memory Device does not have Active SO feature this selection
 * 		will not be displayed in the CONFIG Menu.
 *
 ******************************************************************************/
void display_conf_so(void)
{
	char *s;
	if (use_so)
		s = "SO    Y";
	else
		s = "SO    N";
	SegmentLCD_Write(s);
}

/***************************************************************************//**
 * 	@brief
 * 		CONFIG Menu: Confirms whether Serial Memory Device has Active SO Feature
 * 	@note
 * 		Looks up in Serial Memory Table whether Device being used has Active SO
 * 		feature or not.  If it does it will display on LCD.
 *
 ******************************************************************************/
void enter_conf_so(void)
{
	if (! spiflash_info_table[part].has_so_irq)
	{
		state++;
		return;
	}
	display_conf_so();
}

/***************************************************************************//**
 * 	@brief
 * 		CONFIG Menu:  If PB1 is pressed when in Active SO CONFIG Menu toggles
 * 		whether Active SO is enabled or not.  Display Y or N on LCD.
 *
 ******************************************************************************/
void button1_conf_so(void)
{
	use_so = ! use_so;
	display_conf_so();
}


/***************************************************************************//**
 * 	@brief
 * 		CONFIG MENU: Displays range of values for Erase Size in upper portion of LCD and configures
 * 		Slider to allow these values to be selected.
 * 	@note
 * 		Due to display limitations (four digit numeric only), values in kilobytes are shown as the
 *		number of kilobytes, but values of 64 or 256 bytes are shown as zero
 *		(i.e., display rounded to 0 kibibytes)
 *
 ******************************************************************************/
void numeric_choices_init_conf_erase_size(void)
{
	int i;

	if (erase_size_choices_initialized)
		return;

	numeric_choices_count[state] = 0;
	for (i = 0; i < spiflash_info_table[part].erase_info_count; i++)
	{
		numeric_choices[state][i] = spiflash_info_table[part].erase_info[i].size >> 10;
		numeric_choices_count[state] += 1;
	}

	for (i = 0; i < numeric_choices_count[state]; i++)
	{
		int32_t choice = numeric_choices[state][i];
		if (((erase_size < 1024) && (choice == 0)) ||
		    ((erase_size >= 1024) && (choice == erase_size >> 10)))
		{
			state_slider_position[state] = slider_threshold[i] - 1;
			break;
		}
	}
}

//TODO Validate correct display of 64b vs. Chip Erase...
/***************************************************************************//**
 * 	@brief
 * 		CONFIG Menu: Gets user selected Erase size based on slider value
 * 	@note
 * 		Due to display limitations (four digit numeric only), values in kilobytes are shown as the
 *		number of kilobytes, but values of 64 or 256 bytes are shown as zero
 *		(i.e., display rounded to 0 kibibytes)
 *
 ******************************************************************************/
void leave_conf_erase_size(void)
{
	int choice = slider_get_choice(state_slider_position[state_conf_erase_size]);
	// due to display limitations (numeric only), values in kibibytes are shown as the
	// number of kibibytes, but values of 64 or 256 bytes are shown as zero
	// (i.e., display rounded to 0 kibibytes)
	if (choice)
		erase_size = 1024 * choice;
	else
		erase_size = spiflash_smallest_erase_size_above(0);
	SegmentLCD_NumberOff();
}

/***************************************************************************//**
 * 	@brief
 * 		CONFIG Menu: Gets user selection to verify write data or not.
 * 	@note
 * 		Verify if Enabled will read back the data that was wrote and verify against
 * 		the initial data in the buffer that was used for write.
 *
 ******************************************************************************/
void display_conf_verify(void)
{
	char *s;
	if (do_verify)
		s = "VFT   Y";
	else
		s = "VFY   N";
	SegmentLCD_Write(s);
}

/***************************************************************************//**
 * 	@brief
 * 		CONFIG Menu: Calls function to display Verify Y/N on LCD
 *
 ******************************************************************************/
void enter_conf_verify(void)
{
	display_conf_verify();
}

/***************************************************************************//**
 * 	@brief
 * 		CONFIG Menu: PB1 toggles between Verify Enable/Disable
 *
 ******************************************************************************/
void button1_conf_verify(void)
{
	do_verify = ! do_verify;
	display_conf_verify();
}

/***************************************************************************//**
 * 	@brief
 * 		CONFIG Menu: User Selection for SPI Clock Frequency.  User can select
 * 		between 2MHz or 12MHz SPI clock frequency.
 * 	@note
 * 		2MHz SPI operation runs from internal 14MHz clock.  Fast wakeup from Sleep Modes
 * 		12MHz SPI operation runs from external 48MHz XTAL.  Requires longer time to wakeup
 * 		from sleep modes to allow HFCLK to stabilize.
 *
 ******************************************************************************/
void leave_conf_spi_clk(void)
{
	CMU_Select_TypeDef new_main_osc;  // cmuSelect_HFRCO or cmuSelect_HFXO

	spi_freq = 1000000 * slider_get_choice(state_slider_position[state_conf_spi_clk]);

	SegmentLCD_NumberOff();

	if (spi_freq == 2000000)
		new_main_osc = cmuSelect_HFRCO;
	else
		new_main_osc = cmuSelect_HFXO;

	if (new_main_osc == main_osc)
		return;  // not changed

	main_osc = new_main_osc;
	CMU_ClockSelectSet(cmuClock_HF, main_osc);

	if (main_osc == cmuSelect_HFRCO)
		CMU_OscillatorEnable(cmuSelect_HFXO, false, false); // stop HFXO
	else
		CMU_OscillatorEnable(cmuSelect_HFRCO, false, false); // stop HFRCO

	spiflash_setup();
}

/***************************************************************************//**
 * 	@brief
 * 		CONFIG Menu: Final PB1 Press concludes Demo Conigruation and sets state to
 * 		initial state.
 *
 ******************************************************************************/
void button1_cfg_done(void)
{
	state = state_id;
}

/***************************************************************************//**
 * 	@brief
 * 		Displays current info on LCD until user press Push-buttons to change or
 * 		move to the next selection.
 *
 ******************************************************************************/
void enter_message(void)
{
	SegmentLCD_Write(message_text);
	SegmentLCD_Number(message_number);
}

/***************************************************************************//**
 * 	@brief
 * 		Displays current state on LCD based on PB1 Press and location in menu
 *
 ******************************************************************************/
void button1_message(void)
{
    lcd_scroll_stop();
	state = message_return_state;
}


/**************************************************************************//**
 * @brief
 * 		Main function, initializes demo
 * @details
 * 		Configures Clocks, GPIO/GPIO Interrupts for Push-buttons, Configures LEDs,
 * 		initializes Low Power States, Enables LCD, Enables Capsense/Slider, configures
 * 		SPI/spiflash setup, calls main spiflash_demo from while(1)
 *****************************************************************************/
int main(void)
{
  int i;

  CHIP_Init();  // Chip errata

  main_osc = cmuSelect_HFRCO;
  spi_freq = 2000000;

  // start HFRCO
  CMU_ClockSelectSet(cmuClock_HF, main_osc);

  CMU_ClockEnable(cmuClock_HFPER, 1);
  CMU_ClockEnable(cmuClock_GPIO, true);

  // Start LFXO, and use LFXO for low-energy modules
  CMU_ClockEnable(cmuClock_CORELE, true);
  CMU_ClockSelectSet(cmuClock_LFA, cmuSelect_LFXO);
  CMU_ClockSelectSet(cmuClock_LFB, cmuSelect_LFXO);
  CMU_ClockEnable(cmuClock_LESENSE, 1);
  CMU_ClockDivSet(cmuClock_LESENSE, cmuClkDiv_1);
  CMU_ClockEnable(cmuClock_ACMP0, 1);
  CMU_ClockEnable(cmuClock_ACMP1, 1);

  RTCDRV_Init();
  RTCDRV_AllocateTimer( &xTimerForWakeUp);
  delay_init();

  gpio_irq_init();
  led_init();
  button_init(spiflash_button_callback);	//Manages Button States
  low_power_init();

  SegmentLCD_Init(false); // Enable LCD without voltage boost

  for (i = 0; i < STATE_MAX; i++)		//Configure Slider to Default
	  state_slider_position[i] = 0;

  CAPLESENSE_Init(true); // Init slider sensing but in sleep

  spiflash_setup();		//Configures SPI Port and Reads Flash ID

  use_so = false;
  erase_size_choices_initialized = false;
  erase_size = spiflash_smallest_erase_size_above(256);
  do_verify = false;

  // if the part is a DataFlash, make sure it is set to 256b pages
  if (dataflash_get_page_size() == 264)
  {
	  if (! dataflash_set_page_size(256))
		  fatal("Can't set DataFlash page size");
  }

  spiflash_ultra_deep_power_down(true, NULL, NULL);		//Puts Flash into Ultra Deep Powerdown

  SegmentLCD_Write("Fl Demo");		//TODO possibly remove...?
  EM2Sleep(250);

  data_written_byte_count = 0;

  while (true)
	spiflash_demo();

  return 0;
}

/** @} (end addtogroup MAIN) */
