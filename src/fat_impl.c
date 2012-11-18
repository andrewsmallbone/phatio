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
#include <string.h>
#include "fat_impl.h"
#include "bootloader_shared.h"

// disk_init is only used on avr but this file is also used by the host support functions so put an
// ifdef around it so it is only built in AVR code - perhaps move to own file
#ifdef ARCH
#include "sd.h"

#ifdef USE_BOOTLOADER_FUNCTIONS
bool (*disk_init)(sd_disk *sd) = SHARED_DISK_INIT;
uint8_t *(*find_sfn_entry)(uint8_t *buf, const uint8_t *name, uint8_t attr_m, uint8_t attr) = SHARED_FIND_SFN_ENTRY;
uint16_t (*find_sfn_file)(int16_t cluster, uint8_t *name, bool directory, sd_disk *sd) = SHARED_FIND_SFN_FILE;
int16_t (*get_next_cluster)(int16_t cluster, sd_disk *sd) = SHARED_GET_NEXT_CLUSTER;

#else
#define P1_START 446
#define BLOCK_EQ(i,val) (sd->buf[i] == val)
bool disk_init(sd_disk *sd)
{
    uint32_t pstart = 0;
    uint16_t psize = 0;

    //
    // Find MBR and First partition and get fat and root directory from them
    //
    if (sd_read_block(0, sd)
            && BLOCK_EQ(510,0x55) && BLOCK_EQ(511,0xAA) && BLOCK_EQ(P1_START+4,0x04)) {
        pstart = uint32_value(sd->buf+P1_START+8);
        if (pstart != 0 && sd_read_block(pstart, sd)) {
            // fat start = pstart + number reserved sectors
            sd->fat_start = pstart + uint16_value(sd->buf+14);

            sd->p_size = uint16_value(sd->buf+19);

            // root_dir start = <fat start> + <fat size> * <number of fats>;
            sd->root_dir = sd->fat_start + uint8_value(sd->buf+16) * uint16_value(sd->buf+22);

            // data_start = <root_dir start> + <num root entries> * 32 / 512
            sd->data_start = sd->root_dir + (uint16_value(sd->buf+17) >> 4);
            return true;
        }
    }
    return false;
}

uint8_t *find_sfn_entry(uint8_t *buf, const uint8_t *name, uint8_t attr_m, uint8_t attr)
{
    for (uint16_t i = 0; i < 512; i += 32) {
        if ((buf[i + 11] & attr_m) == attr) {
            uint8_t c;
            for (c = 0; name[c] == buf[i + c] && c < 11; c++);
            if (c == 11) {
                return buf+i;
            }
        }
    }
    return 0;
}

uint16_t find_sfn_file(int16_t cluster, uint8_t *name, bool directory, sd_disk *sd)
{
    uint8_t attributes = directory ? FAT_FILE_DIR : 0;

    while (cluster) {
        sd_read_block(cluster_to_block(sd, cluster), sd);
        uint8_t *buf = find_sfn_entry(sd->buf, name, 0xD8, attributes);
        if (buf) {
            return fatdirent_get_cluster(buf);
        }
        cluster = get_next_cluster(cluster, sd);
    }
    return 0;
}

int16_t get_next_cluster(int16_t cluster, sd_disk *sd)
{
    uint16_t fat_entry;
    if (cluster < 0) {
        return cluster - 1;
    }

    sd_read_block(sd->fat_start + FATBLOCK(cluster), sd);
    memcpy(&fat_entry, sd->buf+FATOFFSET(cluster), sizeof(fat_entry));

    if (fat_entry > 2 && fat_entry < 0xFFEF) {
        return fat_entry;
    }
    return 0;
}
#endif



uint16_t find_lfn_file(int16_t cluster, uint8_t *name, bool directory, sd_disk *sd)
{
    return find_sfn_file(cluster, fat_sfn(name), directory, sd);
}


#endif //ARCH


uint16_t cluster_to_block(sd_disk *sd, int16_t cluster)
{
    return (cluster < 0) ? (sd->root_dir - cluster - 1) : (sd->data_start+cluster-2);
}

uint8_t sfn_checksum(uint8_t sfn[])
{
	uint8_t checksum = 0;
	for (uint8_t i=0; i<11; i++) {
		checksum = (((checksum & 1) << 7) | ((checksum & 0xfe) >> 1)) + sfn[i];
	}
	return checksum;
}


uint8_t fat_sfn_safe_char(uint8_t c)
{
	if (c == ' ' || c =='.') {
		return 0;
	} else if (c >= 'a' && c<= 'z') {
		return c-('a'-'A');
	} else if ((c >= 'A' && c<= 'Z') || (c >= '0' && c<= '9')) {
		return c;
	}  else {
		static char *valid = "$%'-_@~'!()";
		for (uint8_t i=0; valid[i]; i++) {
			if (c == valid[i]) {
				return c;
			}
		}

	}
	return '_';
}

uint8_t *fat_sfn(const uint8_t *lfn)
{
	static uint8_t sfn[12];
	int len = -1;
	int lastdot = -1;
	uint8_t i = 0;

	// find the last . that splits filename and extension
	for (i=0; lfn[i] && i<255; i++) {
		len++;
		if (lfn[i] == '.' && lfn[i+1] != '.' && lfn[i+1] != 0) {
			lastdot = i;
		}
	}

	// go through the long file name putting valid characters into the short file name
	uint8_t j = 0;
	for (i=0; i<(lastdot <= 0 ? len+1 : lastdot) && j<9; i++) {
		uint8_t c = fat_sfn_safe_char(lfn[i]);
		if (c != 0) {
			sfn[j++] = c;
		}
	}
	// trailing ~1
	if (j>8) { // if we went past 8 characters put "~1"
		sfn[6] = '~';
		sfn[7] = '1';
	} else { // otherwise add trailing spaces if necessary
		for (; j<8; j++) {
			sfn[j] = ' ';
		}
	}

	// add extension
	j=8;
	for (i=((lastdot <= 0) ? len+1 : lastdot+1); i<=len && j<11; i++) {
		uint8_t c = fat_sfn_safe_char(lfn[i]);
		if (c != 0) {
			sfn[j++] = c;
		}
	}
	for (; j<11; j++) {
		sfn[j] = ' ';
	}

	return sfn;
}


