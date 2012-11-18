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
#include "config.h"
#include <avr/eeprom.h>

static uint32_t runtime_config;

void config_init(void)
{
    runtime_config = eeprom_read_dword(CONFIG_START_ADDR);
    // on first run zero and save
    if (get_bool_config(LIO_CONFIG_INITIALIZED)) {
        runtime_config = 0;
        eeprom_update_dword(CONFIG_START_ADDR, runtime_config);
    }
}

bool get_bool_config(uint8_t offset)
{
    return (runtime_config & (1<<offset));
}

void set_bool_value(uint32_t *conf, uint8_t offset, bool value)
{
    if (value) {
        *conf |= (1<<offset);
    } else {
        *conf &= ~(1<<offset);
    }
}

bool set_bool_config(uint8_t offset, bool value, bool permanent)
{
    set_bool_value(&runtime_config, offset, value);
    if (permanent) {
        uint32_t stored_config = eeprom_read_dword(CONFIG_START_ADDR);
        set_bool_value(&stored_config, offset, value);
        eeprom_update_dword(CONFIG_START_ADDR, stored_config);
    }
}

// (config <id> <value> <persist>
Item *config(List *expression)
{
    uint8_t item = eval_as_uint8(second(expression));
    Item *value = third(expression);
    if (value) {
        set_bool_config(item, eval_as_uint8(value), eval_as_uint8(fourth(expression)));
    }
    return 0;
}

