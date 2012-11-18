/****************************************************************************

    Copyright (c) 2012, Andrew Smallbone <andrew@phatio.com>
    
   Developed as part of the phatIO system.  Support and information 
     available at www.phatio.com
    
   Please support development of this and other great open source projects
     by purchasing products from phatIO.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met: 

 1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer. 
 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution. 

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

****************************************************************************/
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "config.h"
#include "log.h"
#include "keyboard.h"
#include "util.h"
#include "iopin.h"

#define BUFFER_LEN 100
volatile static uint8_t buffer[BUFFER_LEN] = "";

volatile uint8_t write_position = 0;
volatile uint8_t read_position = 0;

void key_printf_P(const char *fmt, ...)
{
    uint16_t len = 0;
	va_list args;
	va_start(args, fmt);
	for (int16_t i=0; i<2; i++) {
	    uint16_t buffer_remaining = BUFFER_LEN - write_position;
	    len = vsnprintf_P((char *)buffer+write_position, buffer_remaining, fmt, args);
	    if (len< buffer_remaining) {
	        break;
	    }
	    write_position = 0;
	    read_position = 0;
	}
	va_end(args);
	write_position += len;
}

const char log_consume_as_char(void)
{
    char c = 0;
    cli();
	if (write_position != 0 && read_position < write_position) {
		c = buffer[read_position++];
	}
	sei();
	return c;
}

void log_consume_as_keyboard_report(uint8_t* report)
{
    cli();
	if (write_position != 0 && read_position < write_position) {
		read_position += map_keys(report, (uint8_t *)(buffer+read_position), 1);
	}
	sei();
}


#define STATUS_LEN 100
volatile static const char status[STATUS_LEN] = "All systems functional.\n";

const char *error_P(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
    vsnprintf_P((char *)status, STATUS_LEN, fmt, args);
	va_end(args);
	led_error(LEDON);
	return (const char *)status;
}

const char const status_header[] PROGMEM = "phatIO version XXXX (bootloader XXXX)\n"
        "See www.phatio.com for documentation, help, and ideas.\n"\
        "Last status/error message:\n\n";

void status_handler(bool write, uint8_t data, uint8_t *buf)
{
    memset(buf, ' ', 510);
    buf[511] = '\n'; // fill with zeros until implement dynamic size
    if (!write) {
        strncpy_P(buf, status_header, 510);
        eeprom_read_block(buf+15, PHATIO_VERSION_ADDR, 4);
        eeprom_read_block(buf+32, BOOTLOADER_VERSION_ADDR, 4);
		strcpy(buf+122, (char *)status);
	}
}



/*

*/







//
// LED flash/error code
//

#define FLASH_ON_DURATION 0xF // 15ms
#define FLASH_CYCLE 0x3F  // 63ms


#define FLASH -1
#define FLASH_ON -2

#define NUM_LEDS 2
static volatile int8_t led_state[NUM_LEDS] = {LEDSTATE_OFF,LEDSTATE_OFF};
const iopin_t leds[NUM_LEDS] = {REDLED, GREENLED};

void led_init()
{
    set_dir(leds[0], 1);
    set_dir(leds[1], 1);
}


// set the error LED (red) to the given state unless we're in manual mode
void led_error(uint8_t state)
{
    if (led_state[RED] != LEDMANUAL) {
        led_state[RED] = state;
	}
}

// flash the LED (if not already flashing)
void led_flash(uint8_t led)
{
	if (led_state[led] == LEDSTATE_OFF) {
		led_state[led] = FLASH;
	}
}

// set the LED mode
void led_mode(uint8_t led, uint8_t mode)
{
    led_state[led] = mode;
}


void led_task(void)
{
    if (get_bool_config(LIO_NO_LED)) {
        return;
    }
    uint32_t time = millis();

	for (uint8_t i=0; i<NUM_LEDS; i++) {
		switch (led_state[i]) {
        case LEDMANUAL:
            break;
		case LEDON:
			LED_ON(i);
			break;
		case LEDSTATE_OFF:
			LED_OFF(i);
			break;
		case FLASH:
			if ((time & FLASH_CYCLE) <= FLASH_ON_DURATION) {
				LED_ON(i);
				led_state[i] = FLASH_ON;
			}
			break;
		case FLASH_ON:
			if ((time & FLASH_CYCLE) > FLASH_ON_DURATION) {
				LED_OFF(i);
				led_state[i] = LEDSTATE_OFF;
			}
			break;
		default:
			if ((time % 200) < 100) {
				LED_OFF(i);
			} else if (time % 1000 < (led_state[i]*200)) {
				LED_ON(i);
			}
			break;
		}
	}
}
