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
 * Creates a FAT16 filesystem for the phatio sdcard.
 */
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include "fat_impl.h"

#define BUF_SIZE 32000000
uint8_t buffer[BUF_SIZE];

// 128MB
// #define PARTITION_SIZE (134217728 / 512)

// 16MB
#define PARTITION_SIZE (16003072/512)

//FAT32#define FAT_SECTORS (((PARTITION_SIZE+2)*4+511)/512)
#define FAT_SECTORS (((PARTITION_SIZE+2)*2+511)/512)

#define PARTITION_START 63

#define FSINFO (1 + PARTITION_START)
#define BACKUP_BPB (6 + PARTITION_START)
#define FAT_START (PARTITION_START+1)

// 'default' 512 entries * 32 bytes each
#define ROOT_DIR_SECTORS 32

#define ROOT_DIR_START (FAT_START+FAT_SECTORS)
#define DATA_START (ROOT_DIR_START+ROOT_DIR_SECTORS)

uint16_t free_cluster;

#define BUFFER_FOR_CLUSTER(x) (buffer + (DATA_START+(x-2))*512)


uint8_t file_buf[BUF_SIZE];
char *read_file(char *name)
{
    int32_t size = 0;
    FILE *fp = fopen(name, "r");
    for (int16_t c = 0; (c = fgetc(fp)) != EOF; size++) {
        file_buf[size] = c;
    }
    file_buf[size] = 0;
    return file_buf;
}



//
// MBR
//
void create_mbr(uint32_t block, uint32_t p1_start, uint32_t p1_sectors)
{
	uint8_t *mbr = buffer+(block*512);
//    const static uint8_t MBR_PARTITION[] = {
//        0x80, // Boot Indicator
//        0xEE, 0xFF,0xFF, // CHS
//        0x0B, // FAT32
//        0xFE, 0xFF,0xFF, // CHS
//        0x01,0x00,0x00,0x00, // start block, written below
//        0x7F,0x7A,0x00,0x00 // num sectors, written, below
//    };
    const static uint8_t MBR_PARTITION[] = {
        0x00, // Boot Indicator
        0x01, 0x01,0x00, // CHS
        0x04, // FAT16
        0xFE, 0xFF,0xFF, // CHS
        0x00,0x00,0x00,0x00, // start block, written below
        0x00,0x00,0x00,0x00 // num sectors, written, below
    };

	memcpy(mbr+446, &MBR_PARTITION, 8);
	memcpy(mbr+446+8, &p1_start, sizeof(p1_start));
	memcpy(mbr+446+12, &p1_sectors, sizeof(p1_sectors));
	mbr[510] = 0x55;
	mbr[511] = 0xAA;
}

////
//// FSINFO
////
//void create_fsinfo(uint32_t block, uint32_t free_cluster)
//{
//	 uint8_t *buf = buffer+512*block;
//	 uint32_t lead_sig = 0x41615252;
//	 uint32_t struc_sig = 0x61417272;
//	 uint32_t trail_sig = 0xAA550000;
//	 uint32_t last_cluster = 0xFFFFFFFF;
//
//	 memcpy(buf, &lead_sig, 4);
//	 memcpy(buf+484, &struc_sig, 4);
//	 memcpy(buf+488, &last_cluster, 4);
//	 memcpy(buf+492, &free_cluster, 4);
//	 memcpy(buf+508, &trail_sig, 4);
//}




//
// BPB
//
void create_FAT32_root_block(uint32_t block, uint16_t reserved_sectors, uint32_t hidden_sectors, uint32_t total_sectors,
		uint32_t sectors_per_fat, uint16_t flags, uint16_t fs_info, uint16_t backup)
{
	uint8_t *bpb = buffer+(block*512);

	// offsets from USBMS page 182, Table 8-3
	#define BPB_LEN 90
	const static uint8_t BPB[]  = {
			0xEB, 0x3C,0x90,    // Bootable - seems to be neccessary.
			'B', 'S', 'D', ' ', ' ', '4', '.', '4', // OS
			0x00,0x02,			// bytes per sector 512
			0x01,				// sectors per cluster
			0x00,0x00,			// reserved sectors from here to FAT, see below
			0x01,				// num FATs
			0x00,0x00,			// unused
			0x00,0x00,			// unused
			0xF0,				// media type: removable
			0x00,0x00,			// unused
			0x00,0x00, 			// not used
			0x00,0x00,			// not used
			0x00,0x00,0x00,0x00, // num hidden sectors, see below
			0x00,0x00,0x00,0x00, // total sectors, see below
			0x00,0x00,0x00,0x00, // sectors - per FAT, see below
			0x00,0x00, // flags
			0x00,0x00, // version

			0x02,0x00,0x00,0x00, // first cluster of root directory, see below
			0x01,0x00, //FSInfo
			0x06,0x00, // backup boot record
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // reserved
			0x80, // drive number
			0x00,//reserved
			0x29, //ebs
			0xFC,0x17,0xFF,0x78, // volume serial number
			'P','H','A','T', 'I', 'O', ' ', ' ', ' ', ' ', ' ',  // label
			'F', 'A', 'T', '3', '2', ' ', ' ', ' ' // fs type
	};


	memcpy(bpb, BPB, BPB_LEN);
	memcpy(bpb+14, &reserved_sectors, sizeof(reserved_sectors));
	memcpy(bpb+28, &hidden_sectors, sizeof(hidden_sectors));
	memcpy(bpb+32, &total_sectors, sizeof(total_sectors));
	memcpy(bpb+36, &sectors_per_fat, sizeof(sectors_per_fat));
	memcpy(bpb+40, &flags, sizeof(flags));
	memcpy(bpb+48, &fs_info, sizeof(fs_info));
	memcpy(bpb+50, &backup, sizeof(backup));

	bpb[510] = 0x55;
	bpb[511] = 0xAA;
}

void create_FAT16_root_block(uint32_t block, uint32_t hidden_sectors, uint16_t total_sectors,
        uint16_t sectors_per_fat, uint16_t flags, uint16_t fs_info, uint16_t backup)
{
    uint8_t *bpb = buffer+(block*512);

    // offsets from USBMS page 178, Table 8-1
    const static uint8_t BPB[]  = {
            0xEB, 0x3C,0x90,    // Bootable - seems to be neccessary.
            'B', 'S', 'D', ' ', ' ', '4', '.', '4', // OS
            0x00,0x02,          // bytes per sector 512
            0x01,               // sectors per cluster
            0x01,0x00,          // 1
            0x01,               // num FATs
            0x00,0x02,          // max entries in root directory (512)
            0x00,0x00,          // total sectors (below)
            0xF0,               // media type: removable
            0x00,0x00,          // sectors per FAT (below)
            0x00,0x00,          // not used in LBA
            0x00,0x00,          // not used in LBA
            0x00,0x00,0x00,0x00, // num hidden sectors, see below
            0x00,0x00,0x00,0x00, // total sectors, see below
            0x80, 0x00, 0x29,     //
            0x00,0x00,0x00,0x00, // serial number (below)
            'P','H','A','T', 'I', 'O', ' ', ' ', ' ', ' ', ' ',  // label
            'F', 'A', 'T', '1', '6', ' ', ' ', ' ' // fs type
    };


    memcpy(bpb, BPB, 62);
    memcpy(bpb+19, &total_sectors, 2);
    memcpy(bpb+22, &sectors_per_fat, 2);
    memcpy(bpb+28, &hidden_sectors, 4);

    uint32_t serial_number = time(NULL);
    memcpy(bpb+39, &serial_number, 4);

    bpb[510] = 0x55;
    bpb[511] = 0xAA;
}


//
// FAT
//
void create_fat32_header(uint8_t buf[])
{
	buf[0] = 0xF0;
	buf[1] = 0xFF;
	buf[2] = 0xFF;
	buf[3] = 0xFF;

	buf[4] = 0xFF;
	buf[5] = 0xFF;
	buf[6] = 0xFF;
	buf[7] = 0x0F;
}

void create_fat16_header(uint8_t buf[])
{
    buf[0] = 0xF0;
    buf[1] = 0xFF;

    buf[2] = 0xFF;
    buf[3] = 0x0F;
}


void fat32_entry(uint32_t cluster, uint32_t value)
{
    memcpy(buffer+FAT_START*512 + (cluster)*4, &value, sizeof(value));
}

void fat_entry(uint16_t cluster, uint16_t value)
{
    memcpy(buffer+FAT_START*512 + (cluster)*2, &value, sizeof(value));
}


//
// LFN creation
//
#define WRITE_TERMINATED_LFN_STRING(buf,i,len) if (i<len) {*(buf) = lfn[i]; } else if (i>len) { *(buf) = 0xFF; *(buf+1) = 0xFF;}

void create_lfn_entry(uint8_t **entry_position, uint8_t entry, uint8_t *lfn, uint8_t checksum)
{
	uint8_t *buf = *entry_position;
	*entry_position += 32;

	uint32_t len = strlen(lfn);

	buf[0] = entry;
	WRITE_TERMINATED_LFN_STRING(buf+1,0,len);
	WRITE_TERMINATED_LFN_STRING(buf+3,1,len);
	WRITE_TERMINATED_LFN_STRING(buf+5,2,len);
	WRITE_TERMINATED_LFN_STRING(buf+7,3,len);
	WRITE_TERMINATED_LFN_STRING(buf+9,4,len);
	buf[11] = 0x0F;
	buf[12] = 0x00;
	buf[13] = checksum;
	WRITE_TERMINATED_LFN_STRING(buf+14,5,len);
	WRITE_TERMINATED_LFN_STRING(buf+16,6,len);
	WRITE_TERMINATED_LFN_STRING(buf+18,7,len);
	WRITE_TERMINATED_LFN_STRING(buf+20,8,len);
	WRITE_TERMINATED_LFN_STRING(buf+22,9,len);
	WRITE_TERMINATED_LFN_STRING(buf+24,10,len);
	buf[26] = 0x00;
	buf[27] = 0x00;
	WRITE_TERMINATED_LFN_STRING(buf+28,11,len);
	WRITE_TERMINATED_LFN_STRING(buf+30,12,len);
}






void create_sfn_entry(uint8_t **buffer, uint8_t *shortName, uint8_t attributes, uint16_t cluster, uint32_t size)
{
	uint8_t *buf = *buffer;
	*buffer += 32;

	static const uint8_t times[] = {
		0x4B, 0x1D, 0x7D, 0x7B, 0x40, // creation
		0x7B, 0x40, // accessed date
		0x00, 0x00, // filled in below
		0x1D, 0x7D, 0x7B, 0x40, // accessed time
		0x2C, 0x00 // modified time
	};
	memcpy(buf, shortName, 11);
	buf[11] = attributes;
	memcpy(buf+13, times, 15);
	uint8_t *c = (uint8_t *)&cluster;
	//buf[20] = c[2]; buf[21] = c[3]; // high
	buf[26] = c[0]; buf[27] = c[1]; // low
	//fprintf(stderr, "%x %x %x %x %u %s\n", buf[20], buf[21], buf[26], buf[27], cluster, shortName);

	memcpy(buf+28, &size, sizeof(size));
}

void create_directory_entry(uint8_t **buffer, uint8_t *name, uint8_t *sfn, uint8_t attributes, uint16_t cluster, uint32_t size)
{
	sfn = fat_sfn(sfn ? sfn : name);
	uint8_t len = strlen(name);
	uint8_t checksum = sfn_checksum(sfn);

	uint8_t num_lfn_entries = len/13 + 1;
	for (uint8_t num=num_lfn_entries; num>0; num--) {
		uint8_t ordinal = num | ((num == num_lfn_entries) ? (1<<6) : 0);
		create_lfn_entry(buffer, ordinal, name+((num-1)*13), checksum);
	}
	create_sfn_entry(buffer,sfn, attributes, cluster, size);
}


void create_file(uint8_t **entry, uint8_t *name, uint8_t *sfn, uint8_t attributes, uint8_t *content, int32_t length)
{
    if (length == -1) {
        length = strlen(content);
    }
	create_directory_entry(entry, name, sfn, attributes, free_cluster, length);
	for (int i=0; i<length; i+=512) {
	    fat_entry(free_cluster, (length-i > 512) ? free_cluster+1 : EOC);
	    strncpy(BUFFER_FOR_CLUSTER(free_cluster), content+i, 512);
	    free_cluster++;
	}
}


uint8_t *create_directory(uint8_t **entry_position, uint8_t *name, uint8_t *sfn, uint16_t parent_cluster, uint8_t size)
{
	uint16_t first_cluster = free_cluster;
	for (int i=1; i<size; i++) {
		fat_entry(free_cluster, free_cluster+1);
		free_cluster++;
	}
	fat_entry(free_cluster, EOC);
	create_directory_entry(entry_position, name, sfn, FAT_FILE_DIR, first_cluster, 0);

	uint8_t *directory_start = BUFFER_FOR_CLUSTER(first_cluster);
	create_sfn_entry(&directory_start, ".          ", FAT_FILE_DIR, first_cluster, 0);
	create_sfn_entry(&directory_start, "..         ", FAT_FILE_DIR, parent_cluster, 0);
	free_cluster++;
	return directory_start;
}



int main(int argc, char *argv[])
{
    char *bootfile = argv[1];
    fprintf(stderr, "partition start=%d size=%d \n",PARTITION_START, PARTITION_SIZE);
    fprintf(stderr, "fat start=%d sectors=%d\n",FAT_START, FAT_SECTORS);
    fprintf(stderr, "data start=%d \n",DATA_START);

	memset(buffer, 0, BUF_SIZE);
	// MBR
	create_mbr(0, PARTITION_START, PARTITION_SIZE);

	// root block
	create_FAT16_root_block(PARTITION_START, PARTITION_START, PARTITION_SIZE, FAT_SECTORS, 0, 1, 6);

	// backup root block
	//create_FAT16_root_block(BACKUP_BPB, RESERVED_SECTORS, PARTITION_START, PARTITION_SIZE, FAT_SECTORS, 0, 1, 6);

	// FAT
	create_fat16_header(buffer+(512*FAT_START));

	//fat_entry(2, EOC);
	uint8_t *root_dir = buffer+ROOT_DIR_START*512;
	create_sfn_entry(&root_dir, "PHATIO      ", FAT_FILE_VOLUME, 0, 0);

	// start io directory in its own block
	free_cluster = 2;


	// create the io directory
	uint16_t io_cluster = free_cluster;

    create_file(&root_dir, "readme.txt", 0, 0, read_file("../fs/readme.txt"), -1);

	uint8_t *io_dir = create_directory(&root_dir, "io", 0, 0, 4);
	uint8_t *s = io_dir;

    uint8_t *mode_dir = create_directory(&io_dir, "mode", 0, io_cluster, 4);
    for (int i=0; i<20; i++) {
        char name[3];
        sprintf(name, "%d", i);
        create_file(&mode_dir, name, 0, 0, "????", -1);
    }

    uint8_t *pins_dir = create_directory(&io_dir, "pins", 0, io_cluster, 4);
    for (int i=0; i<20; i++) {
        char name[3];
        sprintf(name, "%d", i);
        create_file(&pins_dir, name, 0, 0, "????", -1);
    }


	uint8_t *boot_dir = create_directory(&io_dir, "boot", 0, io_cluster, 4);
	create_file(&boot_dir, bootfile+3, 0, 0, read_file(bootfile), -1);


    create_file(&io_dir, "status", 0, 0, "If you can see this phatio isn't working correctly.\n", 512);
    create_file(&io_dir, "run.lio", 0, 0, " ", -1);

    uint8_t *dev_dir = create_directory(&io_dir, "dev", 0, io_cluster, 4);

    uint8_t *etc_dir = create_directory(&io_dir, "etc", 0, io_cluster, 4);
    create_file(&etc_dir, "config", 0, 0, read_file("../fs/io/etc/config"), -1);
    create_file(&etc_dir, "pwm_pins", 0, 0, read_file("../fs/io/etc/pwm_pins"), -1);
    create_file(&etc_dir, "adc_pins", 0, 0, read_file("../fs/io/etc/adc_pins"), -1);
    create_file(&etc_dir, "digital_pins", 0, 0, read_file("../fs/io/etc/digital_pins"), -1);
    create_file(&etc_dir, "license.txt", 0, 0, read_file("../fs/io/etc/license.txt"), -1);
    create_file(&etc_dir, "lufa-license.txt", 0, 0, read_file("../fs/io/etc/lufa-license.txt"), -1);

    // stop os x creating sysfiles to drive
    // http://hostilefork.com/2009/12/02/trashes-fseventsd-and-spotlight-v100/
    create_file(&root_dir, ".metadata_never_index", 0, 0x05, "Present to stop Mac Os X from indexing.\n", -1);
    create_file(&root_dir, ".Trashes", 0, 0x05, "Present to stop Mac Os X from create a Trash directory.\n", -1);
    uint8_t *event_dir = create_directory(&root_dir, ".fseventsd", 0, 0, 2);
    create_file(&event_dir, "no_log", 0, 0x05, "Present to stop Mac Os X from logging filesystem events.\n", -1);


    fprintf(stderr, "writing %d blocks \n", DATA_START+free_cluster);
	write(1, buffer, 512*(DATA_START+free_cluster+100));
}
