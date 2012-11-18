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
#ifndef _util_H_
#define _util_H_

#include <stdint.h>
#include <stdbool.h>


#define DATASIZE 300
void *data_alloc(uint16_t size);
void data_free(uint16_t size);
void store_next(void **first, void *to_store);
extern uint8_t data[];
extern uint8_t free_data;


#define SETUPDEBUGA() {DDRF |= _BV(0);}
#define DEBUGA(val) {if (val) {PORTF |= _BV(0); } else { PORTF &= ~_BV(0); }}
#define TOGGLEDEBUGA() {PORTF ^= _BV(0); }

//
// parsing
//
bool parse_number(const uint8_t *buf, uint16_t *value);
uint16_t print_number(uint8_t *buf, const uint16_t value);


// given a 0 separated progmem map  will return the index of the passed
//  name or -1 if not found, map should be double 0 terminated
//    find_index("first", "first\0second\0third\0\0") == 0
//    find_index("second", "first\0second\0third\0\0") == 1
//    find_index("third", "first\0second\0third\0\0") == 2
//    find_index("asdf", "first\0second\0third\0\0") == -1
int16_t find_index(const char *name, const char map[]);


//
// time related
//
extern volatile uint32_t _timeSinceStart;

// milliseconds since start
#define millis() _timeSinceStart
// delay for specified milliseconds (busy loop)
void delay_ms(uint16_t duration);

#endif // _util_H_
