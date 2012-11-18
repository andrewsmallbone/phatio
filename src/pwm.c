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

#include <stdio.h>
#include "iopin.h"
#include "pwm.h"

/*
#define NUM_HANDLERS 4

static PWMDetails pwm[NUM_HANDLERS] = {
		{ .name = "PWMB5", .ddr = &DDRB, .ddrbit = _BV(5), .ocr = &OCR1AL, .tccr = &TCCR1A, .tcrbit = _BV(COM1A1)},
		{ .name = "PWMB6", .ddr = &DDRB, .ddrbit = _BV(6), .ocr = &OCR1BL, .tccr = &TCCR1B, .tcrbit = _BV(COM1B1)},
		{ .name = "PWMB7", .ddr = &DDRB, .ddrbit = _BV(7), .ocr = &OCR1CL, .tccr = &TCCR1C, .tcrbit = _BV(COM1C1)},
		{ .name = "PWMC6", .ddr = &DDRC, .ddrbit = _BV(6), .ocr = &OCR3AL, .tccr = &TCCR3A, .tcrbit = _BV(COM3A1)}
		{ .name = "PWMC7", .ddr = &DDRC, .ddrbit = _BV(7), .ocr = &OCR4A, .tccr = &TCCR4A, .tcrbit = _BV(COM4A1)},
		{ .name = "PWMD7", .ddr = &DDRD, .ddrbit = _BV(7), .ocr = &OCR4D, .tccr = &TCCR4D, .tcrbit = _BV(COM4D1)}
};
*/

uint8_t pwm(uint8_t pinid, int16_t value)
{
	volatile uint8_t *tccr;
	uint8_t tccrbit;
	volatile uint8_t *ocr;

	switch (pinid) {
	case 10: // b5 OC1A
		ocr = &OCR1AL;
		tccr = &TCCR1A;
		tccrbit = _BV(COM1A1);
		break;
	case 9: // b6 OC1B
		ocr = &OCR1BL;
		tccr = &TCCR1A;
		tccrbit = _BV(COM1B1);
		break;
	case 8: // b7 OC1C
		ocr = &OCR1CL;
		tccr = &TCCR1A;
		tccrbit = _BV(COM1C1);
		break;
	case 7: // c6 OC3A
		ocr = &OCR3AL;
		tccr = &TCCR3A;
		tccrbit = _BV(COM3A1);
		break;
	case 6: // c7 OC4A
		ocr = &OCR4A;
		tccr = &TCCR4A;
		tccrbit = _BV(COM4A1);
		break;
	default:
		// report error
		return 0;
	}


    if (value == 0) {
        *tccr &= ~tccrbit;
        set_out(get_iopin(pinid), 0);
    } else if (value > 0) {
        set_dir(get_iopin(pinid), 1);
        //*ddr |= ddrbit;
        *ocr = (value > 0xFF) ? 0xFF : value;
        *tccr |= tccrbit;
    }
    return *ocr;
}

void pwm_init(void)
{
	// Timer 1 PB5 (OC1A), PB6 (OC1B), PB7 (OC1C)
	// clock/1 (Page 133 table 14-6)
	// PWM, Phase Correct, 8-bit (Page 131 table 14-5)
	TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10)| (0 << WGM13) | (0 << WGM12);
	// oc registers off for now (Page 131 table 14-4)
	TCCR1A |= (0 << WGM11) | (1 << WGM10) | (0 << COM1A1) | (0 << COM1A0) | (0 << COM1B1) | (0 << COM1B0) | (0 << COM1C1) | (0 << COM1C0);

    // Timer 3 PC6 (OC3A)
	TCCR3B |= (0 << CS32) | (0 << CS31) | (1 << CS30)| (0 << WGM33) | (0 << WGM32);
	TCCR3A |= (0 << WGM31) | (1 << WGM30) | (0 << COM3A1) | (0 << COM3A0);

	// Timer 4 PC7 (OC4A)
    TCCR4A |= (0 << COM4A1) | (0 << COM4A0) | (1 << PWM4A);
    TCCR4B |= (0 << CS43) | (0 << CS42) | (0 << CS41) | (1 << CS40);

}


