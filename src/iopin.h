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
#ifndef IOPIN_H_
#define IOPIN_H_

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t iopin_t;

#define NUM_IOPINS 20

extern const iopin_t iopins[];
#define get_iopin(pinid) iopins[pinid]
const iopin_t get_iopin_by_name(const uint8_t *pinname);

void set_in(const iopin_t iopin, uint8_t val);
uint8_t get_in(const iopin_t);
void set_out(const iopin_t iopin, uint8_t val);
uint8_t get_out(const iopin_t iopin);
void set_dir(const iopin_t iopin, uint8_t val);
uint8_t get_dir(const iopin_t iopin);

#define INPORT(iopin) *(&PINB + ((0xF0 & iopin)>>4))
#define DIRPORT(iopin) *(&DDRB + ((0xF0 & iopin)>>4))
#define OUTPORT(iopin) *(&PORTB + ((0xF0 & iopin)>>4))

#define IOPIN(iopin) (0x0F & iopin)
#define IOBV(iopin) _BV(0x0F & iopin)


#endif // IOPIN_H_
