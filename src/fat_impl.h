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
#ifndef _FATIMPL_H_
#define _FATIMPL_H_

#include <stdint.h>
#include <stdbool.h>
#include "sd_info.h"

#define fatdirent_get_cluster(buf) (uint16_value(buf+26))

#ifdef USE_BOOTLOADER_FUNCTIONS
extern bool (*disk_init)(sd_disk *sd);
extern uint8_t *(*find_sfn_entry)(uint8_t *buf, const uint8_t *name, uint8_t attr_m, uint8_t attr);
extern uint16_t (*find_sfn_file)(int16_t cluster, uint8_t *name, bool directory, sd_disk *sd);
extern int16_t (*get_next_cluster)(int16_t cluster, sd_disk *sd);
#else
//uint16_t fatdirent_get_cluster(uint8_t *buf);
bool disk_init(sd_disk *sd);
uint8_t *find_sfn_entry(uint8_t *buf, const uint8_t *name, uint8_t attr_m, uint8_t attr);
uint16_t find_sfn_file(int16_t cluster, uint8_t *name, bool directory, sd_disk *sd);
int16_t get_next_cluster(int16_t cluster, sd_disk *sd);
#endif


uint16_t find_lfn_file(int16_t cluster, const uint8_t *name, bool directory, sd_disk *sd);
uint8_t sfn_checksum(uint8_t *sfn);
uint8_t *fat_sfn(const uint8_t *lfn);
uint8_t fat_lfn_checksum(uint8_t *sfn);
uint16_t cluster_to_block(sd_disk *sd, int16_t cluster);


// FAT file attributes
#define FAT_FILE_READONLY 0x01
#define FAT_FILE_HIDDEN 0x02
#define FAT_FILE_SYSTEM 0x04
#define FAT_FILE_VOLUME 0x08
#define FAT_FILE_DIR 0x10
#define FAT_FILE_ARCHIVE 0x20

//FAT32#define EOC 0xFFFFFFFF
//FAT32#define RESERVED 0x00000001
#define EOC 0xFFFF
#define RESERVED 0x0001

// the block the FAT entry for cluster is in cluster/(512/2)
#define FATBLOCK(cluster) (cluster >> 8)
// offset from beginning of the clusters FAT block ((cluster % (512/2)) * 2)
#define FATOFFSET(cluster) ((cluster & 0xFF) << 1)


#define uint8_value(buf) (*((uint8_t *)(buf)))
#define uint16_value(buf) (*((uint16_t *)(buf)))
#define uint32_value(buf) (*((uint32_t *)(buf)))
#endif //_FATIMPL_H_
