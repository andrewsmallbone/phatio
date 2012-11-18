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
#include <unistd.h>
#include <string.h>
#include <stdint.h>

#define BUF_SIZE 32000000
uint8_t buffer[BUF_SIZE];
int32_t offset;

uint8_t next_char(void) { offset++; return buffer[offset-1];}
uint8_t hex_char(uint8_t c) {return (c >= 'a') ? (c - 'W') : ((c >= 'A') ? c - '7' : c - '0'); }
uint8_t next_byte(void) { return ((hex_char(next_char()) << 4) | hex_char(next_char())); }
uint16_t next_word(void) { return ((next_byte() << 8) | next_byte()); }


#define START_CODE ':'

//http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html
uint16_t crc16_update(uint16_t crc, uint8_t a)
{
    int i;

    crc ^= a;
    for (i = 0; i < 8; ++i)
    {
        if (crc & 1)
            crc = (crc >> 1) ^ 0xA001;
        else
            crc = (crc >> 1);
    }

    return crc;
}

int main(void)
{
    int32_t size = 0;
    uint16_t prog_crc = 0;

    for (int16_t c = 0; (c = getchar()) != EOF; size++) {
        buffer[size] = c;
    }
    int lines=0;
    int bytes=0;
    for (offset = 0; offset < size;) {
        if (next_char() == ':') {
            uint8_t byte_count = next_byte();
            uint16_t address = next_word();
            uint8_t type = next_byte();
            for (uint8_t i = 0; i < byte_count; i++) {
                uint8_t byte = next_byte();
                prog_crc = crc16_update(prog_crc, byte);
//                printf("%02X", byte);
//                if ((bytes+i) % 16 == 0) printf("\n");
            }
            bytes += byte_count;
            uint8_t checksum = next_byte();
            lines++;
        }
    }
    printf("%04X", prog_crc);
    //printf("%d\n", bytes);
}
