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
#include <stdio.h>
#include "pins.h"
#include "iopin.h"
#include "pwm.h"
#include "adc.h"
#include "log.h"
#include "util.h"

uint8_t modes[20];

/*
   IN, HIGH, ADC, OUT, PWM
 */
#define BUILT_IN 5
static const char const mode_names[] PROGMEM = "IN\0\0ADC\0OUT\0PWM\0HIGH\0????\0";
static const char mode_first_letters[] = "IAOPH";

#define MODE_IN 'I'
#define MODE_ADC 'A'
#define MODE_OUT 'O'
#define MODE_PWM 'P'
#define MODE_HIGH 'H'

void pin_init(void)
{
    for (int i=0; i<20; i++) {
        pin_mode(i, "I");
    }
}


void pin_mode(uint8_t pin, const uint8_t *value)
{
    uint8_t mode = value[0];
    if (mode >= 'a') {
        mode = mode - 'a' + 'A';
    }

    iopin_t iopin = get_iopin(pin);

    // if was but no longer pwm turn off
    if (mode != MODE_PWM && modes[pin] == MODE_PWM) {
        pwm(pin, 0);
    }
    modes[pin] = mode;

    switch (mode) {
        case MODE_IN:
        case MODE_ADC:
            set_dir(iopin, 0);
            set_out(iopin, 0);
            break;
        case MODE_HIGH:
            set_dir(iopin, 0);
            set_out(iopin, 1);
            break;
        case MODE_OUT:
        case MODE_PWM:
            set_dir(iopin, 1);
            break;
        default:
            break;
    }
}


void set_pin_value(uint8_t pin, uint16_t value)
{
    uint8_t mode = modes[pin];

    switch (mode) {
        case MODE_OUT:
            set_out(get_iopin(pin), value);
            break;
        case MODE_PWM:
            pwm(pin, value);
            break;
        default:
            break;
    }
}

uint16_t get_pin_value(uint8_t pin)
{
    uint8_t mode = modes[pin];

    switch (mode) {
        case MODE_ADC:
            return read_adc(pin);
            break;
        case MODE_IN:
        case MODE_HIGH:
            return get_in(get_iopin(pin));
            break;
//        case MODE_OUT:
//            return get_out(get_iopin(pin));
//            break;
//        case MODE_PWM:
//            return pwm(pin, -1);
//            break;
        default:
            return 0;
            break;
    }
}

void pin_mode_handler(bool write, uint8_t pin, uint8_t *buf)
{
    if (write) {
        pin_mode(pin, buf);
    } else {
        uint8_t m = modes[pin];
        for (uint16_t i=0; i<=BUILT_IN; i++) {
            if (m == pgm_read_byte(&mode_names[i*4]) || i==BUILT_IN) {
                strncpy_P(buf, &mode_names[i*4], 5);
                return;
            }
        }
    }
}

void pin_value_handler(bool write, uint8_t pin, uint8_t *buf)
{
    if (write) {
        uint16_t decimal_value = 0;
        parse_number(buf, &decimal_value);
        set_pin_value(pin, decimal_value);
    } else {
        print_number(buf, get_pin_value(pin));
    }
}
