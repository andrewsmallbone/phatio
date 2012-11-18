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

#include <util/twi.h>
#include "lio_twi.h"
#include "log.h"


void twi_init(uint16_t speed)
{
    TWSR = 0x00;
    TWBR = 0x0C;
    TWCR = (1<<TWEN);
}

uint8_t twi_get_status(void)
{
    return TWSR & 0xF8;
}

// write a byte
// returns write status if written
// of 2 if timedout
uint8_t twi_write_byte(const uint8_t data)
{
    uint16_t waits = 0;
    TWDR = data;
    TWCR = (1<<TWINT) | (1<<TWEN);
    do {
        if (TWCR & (1<<TWINT)) {
            return twi_get_status();
        }
    } while (waits++ != 0xFFFF);
    return 0x02;
}

// send start message and address
// returns 0x01 on start timeout 0x02 on address write timeout
// returns 0x18 on success
// returns value of status register on error
uint8_t twi_send_start(uint8_t address)
{
    uint16_t waits = 0;
    TWCR = (1<<TWINT)  | (1<<TWSTA) | (1<<TWEN);
    do {
        if (TWCR & (1<<TWINT)) {
            uint8_t status = twi_get_status();
            if (status == 0x08) {
                return twi_write_byte(address);
            } else {
                return status;
            }
        }
    } while (waits++ != 0xFFFF);
    return 0x01;
}


uint8_t twi_send_stop(void)
{
    TWCR = (1<<TWINT) | (1<<TWSTO) | (1<<TWEN);
    return twi_get_status();
}

Item *return_error(uint8_t status)
{
    twi_send_stop();
    return uint8_retval(status);
}


//
// public functions
//


bool wrote_from_twi_data;


Item *twi_start(List *expression)
{
    Item *address = second(expression);
    twi_init(0);
    uint8_t status = twi_send_start(eval_as_uint8(address));
    if (status != 0x18) {
        return return_error(status);
    }
    return 0;
}


Item *twi_stop(List *expression)
{
    return uint8_retval(twi_send_stop());
}

Item *twi_write(List *expression)
{
    wrote_from_twi_data = true;
    for (List *data = rest(expression); data; data = rest(data)) {
        uint8_t status = twi_write_byte(eval_as_uint8(first(data)));
        if (status != 0x28) {
            return return_error(status);
        }
    }
    return uint8_retval(0);

}

Item *twi(List *expression)
{
    Item *retval = twi_start(expression);
    if (retval) {
        return retval;
    }
    wrote_from_twi_data = false;
    for (List *data = rest(rest(expression)); data; data = rest(data)) {
        uint8_t val = eval_as_uint8(first(data));
        if (!wrote_from_twi_data) {
            uint8_t status = twi_write_byte(val);
            if (status != 0x28) {
                return return_error(status);
            }
        }
    }
    twi_send_stop();
    return 0;
}


