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
#include "adc.h"
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define NUM_ADC 12

// todo: reduce the size of this?
typedef struct
{
    uint8_t pin;
    uint8_t channel;
} pin_to_channel_map;

/*
00 0000 ADC0 PF0  0
00 00001 ADC1 PF1 1
x00 00010 ADC2 -
x00 00011 ADC3 -
00 0100 ADC4 PF4 2
00 0101 ADC5 PF5 3
00 0110 ADC6 PF6 4
00 0111 ADC7 PF7 5
10 0000 ADC8 PD4 16
10 0001 ADC9 PD6 18
10 0010 ADC10 PD7 19
10 0011 ADC11 PB4 11
10 0100 ADC12 PB5 10
10 0101 ADC13 PB6 9
*/
static const pin_to_channel_map map[NUM_ADC] PROGMEM = {{0,0},{1,1},{2,4},{3,5},{4,6},{5,7},{16,8},{18,9},{19,10},{11,11},{10,12},{9,13}};

int16_t read_adc(uint8_t pin)
{
    cli();
	uint8_t channel;

	uint8_t adc = 0xFF;
	for (int16_t i=0; i<NUM_ADC; i++) {
	    if (pgm_read_byte(&map[i].pin) == pin) {
	        adc = pgm_read_byte(&map[i].channel);
	        break;
	    }
	}

	if (adc == 0xFF) {
	    return -1;
	} else if (adc <= 7) {
		channel = adc;
		DIDR1 |= (1 << adc);
	} else {
		channel = 0x20 | (adc - 8);
		DIDR2 |= (1 << (adc-8));
	}

	// ref = AVCC (VCC)
	ADMUX = (0<<REFS1) | (1<<REFS0) | (0 << ADLAR)| (0x1F & channel);
	ADCSRB = (ADCSRB & 0xDF) | (channel & 0x20);

	ADCSRA |= (1<<ADEN) | (1<<ADSC) | (1<<ADPS2) | (1<<ADPS1) | (1<<ADPS0);
	while((ADCSRA & (1<<ADSC)));

	DIDR1 = 0;
	DIDR2 = 0;
	sei();
	return ADC;
}
