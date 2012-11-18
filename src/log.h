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
#ifndef _log_H_
#define _log_H_

#include <avr/pgmspace.h>
#include <stdarg.h>
#include <stdbool.h>
#include "iopin.h"

//
// LED 'logging'
//
void led_init(void);
extern const iopin_t leds[];

#define GREENLED 0x92   // E2
#define REDLED 0x96 // E6

#define RED 0
#define GREEN 1
#define LED_ON(i) set_out(leds[i], 1)
#define LED_OFF(i) set_out(leds[i], 0)

#define LEDSTATE_OFF 0
#define LEDON 100
#define LEDMANUAL 50
#define SD_INITERROR 1
#define SD_READWRITEERROR 2
#define DISK_ERROR 3
#define IODIR_ERROR 4
#define USB_ERROR LEDON

void led_error(uint8_t state);
void led_flash(uint8_t led);
void led_mode(uint8_t led, uint8_t mode);

void led_task(void);


#define error(fmt, ...) error_P(PSTR(fmt), ##__VA_ARGS__)
const char *error_P(const char *, ...);
void status_handler(bool write, uint8_t data, uint8_t *buf);



//
//
// log the flash resident string
void key_printf_P(const char *, ...);
#define key_printf(fmt, ...) key_printf_P(PSTR(fmt),  ##__VA_ARGS__)
void log_consume_as_keyboard_report(uint8_t* report);
const char log_consume_as_char(void);


#endif //_log_H_
