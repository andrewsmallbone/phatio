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

#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/eeprom.h>

#include "config.h"
#include "sd.h"
#include "fat_impl.h"
#include "bootloader_sdihex.h"
#include "bootloader_leds.h"


int main(void)
{
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

	// led
    SETUPRED();
    SETUPGREEN();
    GREENON();
    REDON();

    uint8_t bootloader_version[5] = "b002";
    eeprom_update_block((void *)&bootloader_version, BOOTLOADER_VERSION_ADDR, 4);

    sd_disk sd;
    _delay_ms(250);

	uint16_t flash = 0;
	if (!sd_init(&sd) && disk_init(&sd)) {
	    if (!find_and_load_bootfile(&sd)) {
	        flash = 4*3;
	    }
	}
	GREENOFF();
	REDOFF();

	for (int i=0; i<flash; i++) {
	    REDTOGGLE();
        _delay_ms(250);
 	}
    REDOFF();
	__asm__("jmp 0000");
	sd_write_block(0,0);
}
