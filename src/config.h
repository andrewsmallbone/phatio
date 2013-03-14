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
#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <stdbool.h>
#include <stdint.h>
#include "lio.h"

extern uint8_t bit_config;
extern uint8_t byte_config[];


#define BOOTLOADER_VERSION_ADDR (void *)0
#define PHATIO_VERSION_ADDR (void *)4
#define CONFIG_START_ADDR (void *)8
#define BYTE_CONFIG_START_ADDR (CONFIG_START_ADDR+sizeof(bit_config))


#define NUM_BOOLCONFIG 8
#define NUM_BYTECONFIG 1

#define LIO_CONFIG_INITIALIZED 0
#define LIO_PRINT_ERRORS 1
#define LIO_NO_LED 2
#define LIO_KEYBOARD_DELAY 8

void config_init(void);

#define get_bool_config(offset) (bit_config & (1<<offset))
#define get_byte_config(offset) (byte_config[offset-NUM_BOOLCONFIG])

void set_bool_config(uint8_t offset, bool value, bool permanent);
Item *config(List *expression);

#define LIO_CONFIG_HANDLER_NAMES "config\0"
#define LIO_CONFIG_HANDLERS &config

#endif // _CONFIG_H_
