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
#include <util/crc16.h>
#include <avr/eeprom.h>
#include <avr/boot.h>
#include <alloca.h>
#include <string.h>

#include "config.h"
#include "sd.h"
#include "fat_impl.h"
#include "bootloader_leds.h"


#define START_CODE ':'
sd_disk *fs;
uint8_t *buf;

void load_page(uint16_t address, uint8_t *buf)
{
    static uint8_t f=0;
    if (++f % 10 == 0) {REDON();GREENOFF();}
    else if (f % 10 == 5) {REDOFF();GREENON();}

    eeprom_busy_wait();
    boot_page_erase(address);
    boot_spm_busy_wait(); // Wait until the memory is erased.

    for (uint8_t i = 0; i < SPM_PAGESIZE; i += 2) {
      uint16_t w = *buf++;
      w += (*buf++) << 8;
      boot_page_fill(address + i, w);
    }

    boot_page_write(address); // Store buffer in flash page.
    boot_spm_busy_wait(); // Wait until the memory is written.

    boot_rww_enable();
}

static uint16_t offset = 0;
static uint32_t cluster = 0;

char next_char(void)
{
    if (offset == FS_BLOCK_SIZE) {
        cluster = get_next_cluster(cluster, fs);
        if (cluster == 0) {
            return 0;
        }
        sd_read_block(cluster_to_block(fs, cluster), fs);
        offset = 0;
    }
    return buf[offset++];
}

uint8_t hex_char(uint8_t c)
{
    return (c >= 'a') ? (c - 'W') : ((c >= 'A') ? c - '7' : c - '0');
}

#define hex_byte(high,low) (hex_char(high) << 4 | hex_char(low))

uint8_t next_byte(void)
{
    return hex_byte(next_char(), next_char());
}

uint16_t next_word(void)
{
    return (next_byte() << 8) | next_byte();
}

uint16_t load_bootfile(uint32_t start, uint32_t size, bool program)
{
    REDOFF();
    uint8_t prog_data[SPM_PAGESIZE];
    uint16_t prog_offset = 0;
    int32_t prog_address = 0;
    uint16_t prog_crc = 0;

    cluster = start;
    offset = 0;

    sd_read_block(cluster_to_block(fs, cluster), fs);
    for (uint32_t byte = 0; byte < size && cluster != 0; byte++) {

        if (next_char() == START_CODE) {
            uint8_t byte_count = next_byte();
            uint16_t address = next_word();

            uint8_t type = next_byte();

            for (uint8_t i = 0; i < byte_count; i++) {
                prog_data[prog_offset] = next_byte();
                prog_crc = _crc16_update(prog_crc, prog_data[prog_offset]);
                prog_offset++;
                if (prog_offset == SPM_PAGESIZE) {
                    if (program) {
                        load_page(prog_address, prog_data);
                        memset(prog_data, 0xFF, SPM_PAGESIZE);
                    } else {
                        static uint8_t f = 0;
                        if (++f % 5 == 0) {GREENTOGGLE();}
                    }
                    prog_offset = 0;
                    prog_address += SPM_PAGESIZE;
                }
            }
            uint8_t checksum = next_byte();
            byte += ((byte_count + 5) * 2);
            if (type == 1 && prog_offset > 0) {
                if (program ) {
                    load_page(prog_address, prog_data);
                }
            }
        }
    }
    return prog_crc;
}

bool check_bootfile(uint8_t *name, uint32_t size, uint16_t start)
{
    uint8_t version[4];
    uint16_t crc = hex_byte(name[4], name[5]) << 8 | hex_byte(name[6], name[7]);

    eeprom_read_block((void *)&version, PHATIO_VERSION_ADDR, 4);
    if (version[0] == 'X' || strncmp(name, version, 4) != 0) {
        uint16_t actual_crc = load_bootfile(start, size, false);
        if (actual_crc == crc) {
            if (crc == load_bootfile(start, size, true)) {
                eeprom_update_block((void *)name, PHATIO_VERSION_ADDR, 4);
                return true;
            }
        }
        return false;
    }
    return true;
}

bool find_and_load_bootfile(sd_disk *fsinfo)
{
    char name[8];
    fs = fsinfo;
    buf = fs->buf;
    uint16_t boot_dir = find_sfn_file(find_sfn_file(-1, "IO         ", true, fs),
            "BOOT       ", true, fs);
    if (boot_dir == 0) {
        return true;
    }
    sd_read_block(cluster_to_block(fs, boot_dir), fs);

    for (uint16_t i = 0; i < 512; i += 32) {
        if (buf[i] != 0 && ((buf[i + 11] & 0xD8) == 0)) { // 00X0 0XXX
            uint8_t c;
            // filename must be all alpha numeric
            for (c = 0; ((buf[i+c] >= 'A' && buf[i+c] <= 'X') || (buf[i+c] >= '0' && buf[i+c] <='9')) && c < 8; c++);
            if (c == 8 && buf[i + 8] == 'H' && buf[i + 9] == 'E' && buf[i + 10] == 'X') {
                uint32_t size = *((uint32_t *) (buf + i + 28));
                memcpy(&name, &(buf[i]), 8);
                if (size > 0) {
                    return check_bootfile(name, size, fatdirent_get_cluster(buf + i));
                }
            }
        }
    }
    return true;
}

