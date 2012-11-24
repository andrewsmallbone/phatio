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
#include "lio_io.h"
#include "string.h"


// lio functionality for io interraction
// setdir/setpin/getpin/getadc
//
#include <avr/pgmspace.h>
#include <stdio.h>
#include "iopin.h"
#include "adc.h"
#include "pwm.h"
#include "log.h"
#include "pins.h"

// (pin_mode <pinid> IN|HIGH|OUT|PWM|ADC [value])
Item *lio_pinmode(List *expression)
{
    const uint8_t pinid = eval_as_uint8(second(expression));
    pin_mode(pinid, eval_as_string(third(expression)));
    Item *value = fourth(expression);
    if (value) {
        set_pin_value(pinid, eval_as_uint16(value));
    }
    return 0;
}

// (set_pin <pinid> <value>)
Item *lio_setpin(List *expression)
{
    const uint8_t pinid = eval_as_uint8(second(expression));
    set_pin_value(pinid, eval_as_uint16(third(expression)));
    return 0;
}

// value (get_pin <pinid>)
Item *lio_getpin(List *expression)
{
    const uint8_t pinid = eval_as_uint8(second(expression));
    return uint16_retval(get_pin_value(pinid));
}


Item *keyboard(List *expression)
{
    Item *last = 0;
    for (List *remaining = rest(expression); remaining; remaining = rest(remaining)) {
        last = first(remaining);
        key_printf("%s", eval_as_string(last));
    }
    return last;
}

Item *fmt(List *expression)
{
    char *fmt = (char *)eval_as_string(second(expression));
    uint16_t fmt_size = strlen(fmt)+1;

    uint16_t value = eval_as_uint16(third(expression));

    // move the format to the back of our fixed string buffer
    char *moved_fmt = str_buf+STRBUFSIZE-fmt_size;
    strcpy(moved_fmt, fmt);
    // and write the formatted string to the buffer
    snprintf(str_buf, STRBUFSIZE-fmt_size, moved_fmt, value);
    return string_retval();
}

Item *led(List *expression)
{
    uint8_t led = eval_as_string(second(expression))[0] == 'R' ? 0 : 1;
    Item *value = third(expression);
    led_mode(led, value ? LEDMANUAL : LEDSTATE_OFF);
    set_out(leds[led], eval_as_uint8(value));
    return 0;
}


Item *port(List *expression)
{
    uint16_t value = eval_as_uint16(second(expression));
    uint8_t i=0;
    for (List *remaining = rest(rest(expression)); remaining; remaining = rest(remaining)) {
        set_pin_value(eval_as_uint8(first(remaining)), value & (1<<i));
        i++;
    }
    return 0;
}

// 8bit value (peek address)
Item *peek(List *expression)
{
    uint16_t addr = eval_as_uint16(second(expression));
    return uint8_retval(*((uint8_t *)(addr)));
}

// 8bit value (poke address value)
Item *poke(List *expression)
{
    uint16_t addr = eval_as_uint16(second(expression));
    if (third(expression)) {
        *((uint8_t *)(addr)) = eval_as_uint8(third(expression));
    }
    return peek(expression);
}

