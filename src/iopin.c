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

#include <string.h>
#include "iopin.h"
#include "log.h"
#include "util.h"

// old mappings
//static const uint8_t *pin_names[NUM_PINS] = {"sclk","mosi","miso" ,"12","11","10","9","8","7","13","14","15","16","17","18", "19","20","1" ,"2" ,"3" ,"4" ,"5" ,"6"};
//static const uint8_t *pin_names[NUM_PINS] = {"b1","b2","b3","b4","b5","b6","b7","c6","c7","d0","d1","d2","d3","d4","d5", "d6","d7","f0","f1","f4","f5","f6","f7"};
//static const iopin_t iopins[NUM_PINS] =     {0x01, 0x02,0x03,0x04,0x05,0x06,0x07,0x37,0x37,0x60,0x61,0x62,0x63,0x64,0x65,0x66, 0x67,0xC0,0xC1,0xC4,0xC5,0xC6,0xC7};

// PINS                                        F0   F1   F4   F5   F6   F7     C7   C6     B7   B6    B5   B4     D0   D1   D2   D3   D4   D5    D6   D7
// PORTB 0x0 PORTC 0x3 PORTD 0x6 PORTE 0x9 PORTF 0xC

static const uint8_t *pin_names[NUM_IOPINS] = {"0", "1" ,"2" ,"3" ,"4" ,"5" ,  "6","7",    "8", "9", "10", "11",  "12", "13","14","15","16","17","18","19"};
const iopin_t iopins[NUM_IOPINS] =     {0xC0,0xC1,0xC4,0xC5,0xC6,0xC7,  0x37,0x36,  0x07, 0x06,0x05,0x04,  0x60, 0x61,0x62,0x63,0x64,0x65,0x66,0x67};


const iopin_t get_iopin_by_name(const uint8_t *pinname)
{
	for (uint8_t i=0; i<NUM_IOPINS; i++) {
		if (strcmp(pinname, pin_names[i]) == 0) {
			return iopins[i];
		}
	}
	return 0;
}

void set_in(const iopin_t iopin, uint8_t val)
{
    if (val) {
        INPORT(iopin) |= IOBV(iopin);
    } else {
        INPORT(iopin) &= ~IOBV(iopin);
    }
}

uint8_t get_in(const iopin_t iopin)
{
    return (INPORT(iopin) & IOBV(iopin)) ? 1 : 0;
}

void set_out(const iopin_t iopin, uint8_t val)
{
    if (val) {
        OUTPORT(iopin) |= IOBV(iopin);
    } else {
        OUTPORT(iopin) &= ~IOBV(iopin);
    }
}

uint8_t get_out(const iopin_t iopin)
{
    return (OUTPORT(iopin) & IOBV(iopin)) ? 1 : 0;
}

void set_dir(const iopin_t iopin, uint8_t val)
{
    if (val) {
       DIRPORT(iopin) |= IOBV(iopin);
    } else {
        DIRPORT(iopin) &= ~IOBV(iopin);
    }
}

uint8_t get_dir(const iopin_t iopin)
{
    return (DIRPORT(iopin) & IOBV(iopin)) ? 1 : 0;
}

