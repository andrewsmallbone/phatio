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
#include <string.h>
#include <util/delay.h>
#include <stdio.h>

#include "util.h"
#include "lio_run.h"

uint8_t data[DATASIZE];
uint8_t free_data = 0;

void *data_alloc(uint16_t size)
{
    if (free_data+size > DATASIZE) {
        lio_stop("out of memory");
        return 0;
    }

    void *allocated = data+free_data;
    free_data += size;
    return allocated;
}
void data_free(uint16_t size)
{
    free_data -= size;
}

typedef struct nextable
{
    struct nextable *next;
} nextable;

void store_next(void **first, void *to_store)
{
    nextable **next_holder;
    for (next_holder = (nextable **)first; *next_holder; next_holder = &((*next_holder)->next));
    *next_holder = to_store;
    ((nextable *)to_store)->next = 0;
}





volatile uint32_t _timeSinceStart = 1;
volatile bool _usbCounting = false;

int16_t find_index(const char *name, const char map[])
{
    int16_t len = strlen(name);
    int8_t j=0;
    int16_t index = 0;
    for (int16_t i=0; ; i++) {
        char c = pgm_read_byte(map+i);
        if (j >= 0 && c==name[j]) {
            if (c==0) {
                return index;
            }
            j++;
        } else if (c==0) {
            index++;
            if (pgm_read_byte(map+i+1) == 0) {
                return -1;
            }
            j=0;
        } else {
            j = -1;
        }
    }
    return -1;
}


bool parse_number(const uint8_t *buf, uint16_t *value)
{
    return (sscanf_P(buf, PSTR("%i"), value) == 1);
}

uint16_t print_number(uint8_t *buf, const uint16_t value)
{
    return sprintf_P(buf, PSTR("%d"), value);
}


void delay_ms(uint16_t duration)
{
	uint32_t end = millis()+duration;
	while(duration-- > 0 && millis() < end) {
		_delay_ms(1);
	}
}



//extern uint8_t _end;
//extern uint8_t __stack;
//#define STACK_CANARY 0xc5
//
//
//void StackPaint(void) __attribute__ ((naked)) __attribute__ ((section (".init1")));
//
//void StackPaint(void)
//{
//#if 0
//    uint8_t *p = &_end;
//
//    while(p <= &__stack)
//    {
//        *p = STACK_CANARY;
//        p++;
//    }
//#else
//    __asm volatile ("    ldi r30,lo8(_end)\n"
//                    "    ldi r31,hi8(_end)\n"
//                    "    ldi r24,lo8(0xc5)\n" /* STACK_CANARY = 0xc5 */
//                    "    ldi r25,hi8(__stack)\n"
//                    "    rjmp .cmp\n"
//                    ".loop:\n"
//                    "    st Z+,r24\n"
//                    ".cmp:\n"
//                    "    cpi r30,lo8(__stack)\n"
//                    "    cpc r31,r25\n"
//                    "    brlo .loop\n"
//                    "    breq .loop"::);
//#endif
//}
//
//uint16_t StackCount(void)
//{
//    const uint8_t *p = &_end;
//    uint16_t       c = 0;
//
//    while(*p == STACK_CANARY && p <= &__stack)
//    {
//        p++;
//        c++;
//    }
//
//    return c;
//}
