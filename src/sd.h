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

#ifndef SD_H_
#define SD_H_

#include <stdint.h>
#include <stdbool.h>

#include "spi.h"
#include "sd_info.h"
#include "bootloader_shared.h"

#ifdef USE_BOOTLOADER_FUNCTIONS
extern uint8_t (*sd_init)(sd_disk *fs);
extern uint8_t (*wait_for)(uint8_t required, uint32_t timeout);
extern uint8_t (*send_cmd_crc)(uint8_t command, uint32_t arg, uint8_t crc);
extern uint8_t (*sd_read_start)(uint32_t address);
extern void (*sd_read_finished)(void);
extern uint8_t (*sd_read_block)(uint32_t address, sd_disk *fs);
extern uint8_t (*sd_write_start)(uint32_t address, sd_disk *fs);
extern uint8_t (*sd_write_finished)(void);
extern uint8_t (*sd_write_block)(uint32_t address, sd_disk *fs);
#else
uint8_t sd_init(sd_disk *fs);
uint8_t wait_for(uint8_t required, uint32_t timeout);
uint8_t send_cmd_crc(uint8_t command, uint32_t arg, uint8_t crc);
uint8_t sd_read_start(uint32_t address);
void sd_read_finished(void);
uint8_t sd_read_block(uint32_t address, sd_disk *fs);
uint8_t sd_write_start(uint32_t address, sd_disk *fs);
uint8_t sd_write_finished(void);
uint8_t sd_write_block(uint32_t address, sd_disk *fs);

#endif

inline uint8_t sd_read_byte(void)
{
	  SPDR = 0xFF;
	  while (!(SPSR & (1<<SPIF)));
	  return SPDR;
}

#define sd_write_byte(out) {SPDR = out;while (!(SPSR & (1<<SPIF)));}

//commands
#define CMD0_40 0x40 | 0x00
#define CMD1_41 0x40 | 0x01
#define CMD8_48 0x40 | 0x08
#define CMD9_49 0x40 | 0x09
#define CMD13_4D 0x40 | 0x0D
#define CMD16_50 0x40 | 0x10
#define CMD17_51 0x40 | 0x11
#define CMD24_58 0x40 | 0x18
#define CMD55_77 0x40 | 0x37
#define CMD58_7A 0x40 | 0x3A
#define ACMD41_69 0x40 | 0x29

//response
#define RESPONSE_BUSY 0x80

#define R1_READY 0x00
#define R1_IN_IDLE_STATE 0x01
#define R1_ERASE_RESET 0x02
#define R1_ILLEGAL_COMMAND 0x04
#define R1_COM_CRC_ERROR 0x08
#define R1_ERASE_SEQUENCE_ERROR 0x10
#define R1_ADDRESS_ERROR 0x20
#define R1_PARAMETER_ERROR 0x40


#define DATA_START_BLOCK    0xFE
#define DATA_RES_MASK   0x0F
#define DATA_RES_ACCEPTED   0x05

//timeouts
#define SD_INIT_TIMEOUT 2000
#define SD_READ_TIMEOUT 100
#define SD_WRITE_TIMEOUT 250

#define send_cmd(index, arg) send_cmd_crc(index, arg, 0xFF)

#define DESELECT PORTB |= _BV(0)
#define SELECT PORTB &= ~_BV(0)
#define RELEASE send_spi(0x00)

#define set_error(stage,status,message) //{_sd_error_stage = stage; }//_sd_error_status = status; }
//extern uint16_t _sd_error_stage;
//extern bool sd_byte_addressable;
//extern uint8_t sd_version;

#endif
