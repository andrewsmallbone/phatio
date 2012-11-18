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
#include "lio_spi.h"


#include "spi.h"
Item *spi(List *expression)
{
    uint8_t value = 0;
    for (List *remaining = rest(expression); remaining; remaining = rest(remaining)) {
        uint8_t val = eval_as_uint8(first(remaining));
        value = send_spi(val);
    }
    return uint8_retval(value);
}

// (spi_conf <mode> <data direction> <clock divisor> )
// (spi_conf  [0..4] [lsb|msb] <1|2|3|4|5|6|7> )
Item *spi_conf(List *expression)
{
    static const uint8_t divisors[] PROGMEM = { SPI_SLAVE, SPI_MSTR_CLK2, SPI_MSTR_CLK4,
            SPI_MSTR_CLK8, SPI_MSTR_CLK16, SPI_MSTR_CLK32, SPI_MSTR_CLK64, SPI_MSTR_CLK128 };

    uint8_t mode = eval_as_uint8(second(expression));
    if (mode > 3) mode = 0;

    const char *dir = eval_as_string(third(expression));

    const uint8_t data_direction = (dir == 0 || dir[0] == 'm') ? SPI_MSB : SPI_LSB;

    // don't allow slave mode for now
    const uint8_t div = eval_as_uint8(fourth(expression));
    const uint8_t divisor = pgm_read_byte(&divisors[(div <= 7 && div >0) ? div : 1]);

    setup_spi(mode, data_direction, 0, divisor);
    return 0;
}
