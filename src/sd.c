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
/*
https://www.sdcard.org/downloads/pls/simplified_specs/Part_1_Physical_Layer_Simplified_Specification_Ver_3.01_Final_100518.pdf
http://elm-chan.org/docs/mmc/mmc_e.html
*/

#include "sd.h"
#include <util/delay.h>

#ifdef USE_BOOTLOADER_FUNCTIONS
uint8_t (*sd_init)(sd_disk *fs) = SHARED_SD_INIT;
uint8_t (*wait_for)(uint8_t required, uint32_t timeout) = SHARED_WAIT_FOR;
uint8_t (*send_cmd_crc)(uint8_t command, uint32_t arg, uint8_t crc) = SHARED_SEND_CMD_CRC;
uint8_t (*sd_read_start)(uint32_t address) = SHARED_SD_READ_START;
void (*sd_read_finished)(void) = SHARED_SD_READ_FINISHED;
uint8_t (*sd_read_block)(uint32_t address, sd_disk *fs) = SHARED_SD_READ_BLOCK;
uint8_t (*sd_write_start)(uint32_t address, sd_disk *fs) = SHARED_SD_WRITE_START;
uint8_t (*sd_write_finished)(void) = SHARED_SD_WRITE_FINISHED;
uint8_t (*sd_write_block)(uint32_t address, sd_disk *fs) = SHARED_SD_WRITE_BLOCK;

#else

// send 0xFF to the card until we receive required or (more than) timeout ms occurs.
uint8_t wait_for(uint8_t required, uint32_t timeout)
{
    uint32_t timeout_steps = timeout * 10;
    uint8_t returned = 0;

    do {
        for (uint8_t i=0; ((returned = send_spi(0xFF)) != required) && i < 0xFF; i++);
        if (returned == required) {
            break;
        }
        _delay_us(100);
        timeout_steps--;

    } while (returned != required && timeout_steps > 0);
    return returned;
}

uint8_t send_cmd_crc(uint8_t command, uint32_t arg, uint8_t crc)
{
    SELECT;
    send_spi(0xFF);

    send_spi(command);
    send_spi(arg >> 24);
    send_spi(arg >> 16);
    send_spi(arg >> 8);
    send_spi(arg);
    send_spi(crc);

    // wait for NON RESPONSE_BUSY
    uint8_t status;
    for (uint8_t i = 0; ((status = send_spi(0xFF)) & RESPONSE_BUSY) && i < 0xFF; i++);
    return status;
}

uint8_t sd_init(sd_disk *sd)
{
    // sd select on E2
    DDRE |= _BV(2);

    uint8_t status = 0;

    DESELECT;
    _delay_ms(20);

    // initialize SPI/PINS
    setup_spi(SPI_MODE_0, SPI_MSB, 0, SPI_MSTR_CLK64);
    //SPCR = 0x52; // SPSR = 0;

    // put into SPI mode
    _delay_ms(2);
    DESELECT;
    PORT_SPI |= SPI_MOSI_PIN;
    for (int i = 0; i < 10; i++) {
        send_spi(0xFF);
    }

    // CMD0
    status = send_cmd_crc(CMD0_40, 0, 0x95);
    DESELECT;
    if (status != R1_IN_IDLE_STATE) {
        return 1;
    }

    // CMD8
    status = send_cmd_crc(CMD8_48, 0x01AA, 0x87);
    if ((status & R1_ILLEGAL_COMMAND) || (status == 0xFF)) {
        sd->sd_type = SD_BYTE_ADDRESSABLE | 0x01;
    } else {
        send_spi(0xFF);
        send_spi(0xFF);
        // need to get 0x01AA bounced back or non sd2
        if (send_spi(0xFF) != 0x01 && send_spi(0xFF) != 0xAA) {
            return 2;
        }
        sd->sd_type = 0x02;
    }
    DESELECT;

    // ADCM41
    for (int i = 0; i < 1000; i++) {
        send_cmd(CMD55_77, 0x00);
        if (sd_version(sd) == 2) {
            status = send_cmd(ACMD41_69, 0X40000000);
        } else {
            status = send_cmd_crc(ACMD41_69, 0, 0xD5);
        }
        if (status == R1_READY) {
            break;
        }
        _delay_ms(1);
    }

    if (status != R1_READY) {
        if (sd_version(sd) != 2) {
            for (int i = 0; i < 1000; i++) {
                status = send_cmd(CMD1_41, 0x00);
                if (status == R1_READY) {
                    sd->sd_type &= ~SD_VERSION;//type 0
                    break;
                }
                _delay_ms(1);
            }
        } else {
            return 3;
        }
    }

    if (sd_version(sd) == 2) {

        // CMD58 check for block addressable
        status = send_cmd(CMD58_7A, 0x00);
        if (status != 0x00) { // bad r1
            return 4;
        }
        status = send_spi(0xFF);
        send_spi(0xFF);
        send_spi(0xFF);
        send_spi(0xFF);
        DESELECT;
        if ((status & 0xC0) != 0xC0) { // if card byte addressable remember
            sd->sd_type |= SD_BYTE_ADDRESSABLE;
        }
    }

    // byte addressable try to turn off
// version 1 atleast seem to ignore this although returning ready
//      if (sd_byte_addressable) {
//          if (send_cmd(CMD16_50, 0x00000200) == R1_READY) {
//              //sd_byte_addressable = false;
//          }
//          break;
//      }

    DESELECT;

    // set to fast clock
    // setup_spi(SPI_MODE_0, SPI_MSB, 0, SPI_MSTR_CLK2);
    SPCR = 0x50;
    SPSR = 0x01;

    // get sizes
    send_spi(0xFF);
    status = send_cmd(CMD9_49, 0x00);
    if (status) {
        return 0xA0;
    }
    for (uint8_t i=0; ((sd->buf[0] = send_spi(0xFF)) & RESPONSE_BUSY) && i < 0xFF; i++);

    for (uint8_t i=1; i<16; i++) {
        sd->buf[i] = send_spi(0xFF);
    }
    // ignore crc
    send_spi(0xFF);
    send_spi(0xFF);

    if ((sd->buf[0] & 0xC0) == 0) {
        sd->sd_type |= 0x01;
        sd->sd_type &= ~0x02;
    }

    DESELECT;
    if (sd_version(sd) == 2) {
        // bits 48-69, bytes = (size+1)* 512K, blocks = (size+1)<<10
        //                   64-69                    56-63           48-55
        sd->sd_size = (((uint32_t)(sd->buf[7] & 0x3F)<<16 | (sd->buf[8]<<8) | (sd->buf[9])) + 1) << 10;
    } else {
        // bits 62-73
        //                  72-73                            64-71               62-63
        uint32_t size = ((uint32_t)(sd->buf[6] & 0x03 )<<10) |((uint32_t)sd->buf[7]<<2) | ((sd->buf[8] & 0xC0) >> 6);
        uint8_t mult = (((sd->buf[9] & 0x03) << 1) | ((sd->buf[10] & 0x80) >> 7)) + 2;
        sd->sd_size = size << mult;
    }
    return 0;
}

uint8_t sd_read_start(uint32_t address)
{
    SPCR = 0x50;
    SPSR = 0x01;

    send_spi(0xFF);
    uint8_t status = send_cmd(CMD17_51, address);
    if (!status) {
        if (wait_for(DATA_START_BLOCK, SD_READ_TIMEOUT) == DATA_START_BLOCK) {
            set_error(0x00,0x00,"");
            return 1;
        } else {
            DESELECT;
            set_error(0xB0, 0x00, "SD card timeout starting block read");
            return 0;
        }
    }
    DESELECT;
    set_error(0xB0, status, "SD card bad response from starting block read");
    return 0;
}

void sd_read_finished(void)
{
    send_spi(0xFF);
    send_spi(0xFF);
    DESELECT;
}

uint8_t sd_read_block(uint32_t address, sd_disk *fs)
{
    SPCR = 0x50;
    SPSR = 0x01;

    bool succeeded = false;

    if (sd_byte_addressable(fs)) {
        address <<= 9;
    }
    if (sd_read_start(address)) {
        for (int i=0; i<512; i++) {
            fs->buf[i] = sd_read_byte();
        }
        sd_read_finished();
        succeeded = true;
    }
    RELEASE;
    return succeeded;
}

uint8_t sd_write_start(uint32_t address, sd_disk *fs)
{
     if (sd_byte_addressable(fs)) {
          address <<= 9;
     }

     uint8_t status = send_cmd(CMD24_58, address);
     if (!status) {
          send_spi(DATA_START_BLOCK);
          set_error(0x00,0x00,"");
          return 1;
     } else {
          set_error(0xC0, status, "SD card timeout starting block write");
          DESELECT;
          return 0;
     }
}

uint8_t sd_write_finished(void)
{
     send_spi(0xFF);
     send_spi(0xFF);

     uint8_t status = send_spi(0xFF);

     if ((status & DATA_RES_MASK) != DATA_RES_ACCEPTED) {
          set_error(0xD0, status, "SD card write block not accepted");
          DESELECT;
          return 0;
     } else if ((wait_for(0xFF, SD_WRITE_TIMEOUT) != 0xFF)  // wait for card to finish
                    || (send_cmd(CMD13_4D, 0) != 0 || send_spi(0xFF) != 0)) { // and for status to be 0
          set_error(0xD1, status, "SD card timeout ending block write");
          DESELECT;
          return 0;
     }
     set_error(0x00,0x00,"");
     DESELECT;
     return 1;
}

uint8_t sd_write_block(uint32_t address, sd_disk *fs)
{

     bool succeeded = false;
     if (sd_write_start(address, fs)) {
          for (int i=0; i<512; i++) {
               sd_write_byte(fs->buf[i]);
          }
          succeeded = sd_write_finished();
     }
    RELEASE;
     return succeeded;
}
#endif

