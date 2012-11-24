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
#include <avr/wdt.h>
#include "Drivers/USB/USB.h"

#include "disk.h"
#include "sd.h"
#include "util.h"
#include "lio_run.h"
#include "pins.h"

// all 'special' directories are assumed to have 4 consecutive dir entry clusters to save space
#define NUM_DIR_CLUSTERS 4


// following contains structures to store the 'watched' and 'handled' files and directories.
// this could do with a restructure but currently all are slightly different so the adhoc
// approach is a little more efficient.
//

// signature of handler functions
typedef void (handler_function)(bool write, uint8_t data, uint8_t *buf);

static uint16_t iodir_start_cluster;

// runfile name, modification time and current cluster
static uint8_t *RUNFILE = "RUN     LIO";
static uint32_t last_write = 0;
static uint32_t runfile_mod = 0;
static uint16_t runfile_cluster = 0;
static uint8_t runfile_triggered;


// data for the iopin files (direction, in, out, pwm, and adc)
// referenced by integer id
typedef struct iopin_file {
    uint8_t id;
    uint16_t cluster;
} iopin_file;


// watched directory - monitored for movement of files
typedef struct watched_dir {
    char *name;
    uint16_t start_cluster;
    uint8_t size;
    iopin_file *data;
    handler_function *handler;
} watched_dir;

static iopin_file pin_values[20];
static iopin_file pin_modes[20];
#define NUM_IODIRS 2
watched_dir iodirs[NUM_IODIRS] = {
        {.name = "mode", .size = 20, .data = pin_modes, .handler = pin_mode_handler},
        {.name = "pins", .size = 20, .data = pin_values, .handler = pin_value_handler}
};

// handled file - currently only the status file
typedef struct handled_file {
    char *name;
    uint16_t cluster;
    handler_function *handler;
} handled_file;

#define NUM_HANDLEDFILES 1
static handled_file handled_files[NUM_HANDLEDFILES] = {
        {.name = "status", .handler = &status_handler}
};

// devices referenced by SFN name
static uint16_t devdir_start_cluster;
typedef struct device_file
{
    struct device_file *next;
    char name[11];
    uint16_t cluster;
    void *handler;
} device_file;
device_file *devices;

sd_disk *sd;

//
// Checks (and setups) for changes in the iodir directories (in, out, dir, pwm, adc)
// which may indicate a file cluster has moved, will record the change and trigger the
// handler.
//
void changed_iodir_block(sd_disk *fs, uint16_t cluster, watched_dir *dir)
{
    iopin_file *pins = (iopin_file *)dir->data;
    uint8_t *buf = fs->buf;
    bool reload = true;

    for (int16_t i = 0; i < 512; i += 32) {
        if (reload) {
            sd_read_block(cluster_to_block(fs, cluster), fs);
            reload = false;
        }
        if ((buf[i + 11] & 0xDF) == 0) {
            uint8_t id = 0;
            uint8_t c = 0;
            for (c = 0; c < 2 && buf[i+c] >= '0' && buf[i+c] <= '9'; c++) {
                id = id*10+buf[i+c]-'0';
            }
            if (c > 0 && buf[i+c] == ' ') {
                uint16_t p = 0;
                for (; (p<dir->size) && (pins[p].id != id) && (pins[p].id != 0xFF); p++);
                if (p<dir->size) {
                    uint16_t member_cluster = fatdirent_get_cluster(buf+i);
                    // check if file cluster has moved
                    if (member_cluster != 0 && member_cluster != pins[p].cluster) {
                        if (pins[p].id == id) {
                            if (dir->handler) {
                                sd_read_block(cluster_to_block(fs, member_cluster), fs);
                                dir->handler(true, id, fs->buf);
                                if (i != (512-32)) { // reload iodir block if haven't just processed last entry
                                    reload = true;
                                }
                            }
                        } else { // if called during init
                            pins[p].id = id;
                        }
                    }
                    pins[p].cluster = member_cluster;
                } else {
                    error("unexpected file in IO dir, %s", buf+i);
                    return;
                }
            }
        }
    }
}

// check if one of the watched device files has 'moved'
void changed_devdir_block(sd_disk *fs)
{
    uint8_t *buf = fs->buf;

    for (device_file *device = devices; device; device = device->next) {
        uint8_t *entry = find_sfn_entry(fs->buf, device->name, 0xDF, 0);
        if (entry) {
            uint16_t member_cluster = fatdirent_get_cluster(entry);
            if (member_cluster != 0 && member_cluster != device->cluster) {
                device->cluster = member_cluster;
                sd_read_block(cluster_to_block(fs, member_cluster), fs);
                if (device->handler) {
                    safe_evalexpressions(device->handler);
                }
                return; // TODO - more than just 1
            }
        }
    }
}


//
// Called to see if block is 'handled' by code rather than just written to disk
//
bool has_handler(uint32_t block, sd_disk *fs, bool write)
{
    if (block < fs->data_start) {
        return false;
    }
    int32_t cluster = block - fs->data_start + 2;

    // check if iopin file has been modified
    for (int i=0; i<NUM_IODIRS; i++) {
        watched_dir *dir = &iodirs[i];
        for (int c=0; c<dir->size; c++) {
            if (cluster == dir->data[c].cluster) {
                if (!write) {
                    memset(fs->buf, 0, 512);
                }
                if (dir->handler) {
                    dir->handler(write, dir->data[c].id, fs->buf);
                    return true;
                }
                return false;
            }
        }
    }

    // handled files (status)
    for (uint16_t file=0; file<NUM_HANDLEDFILES; file++) {
        if (cluster == handled_files[file].cluster) {
            if (!write) {
                memset(fs->buf, 0, 512);
            }
            handled_files[file].handler(write, 0, fs->buf);
            return true;
        }
    }


    // device file
    for (device_file *device = devices; device; device = device->next) {
        if (cluster == device->cluster) {
            if (!write) {
                memset(fs->buf, 0, 512);
            }
            if (device->handler) {
                safe_evalexpressions(device->handler);
            }
            return true;
        }
    }
    // device file may have moved
    if (cluster >= devdir_start_cluster && cluster < (devdir_start_cluster+NUM_DIR_CLUSTERS)) {
        changed_devdir_block(fs);
    }


    return false;
}


//
// Called after block has been written to disk to see if a handler
// is watching the file/block
//
bool has_watcher(uint32_t block, sd_disk *fs)
{
    if (block < fs->data_start) {
        return false;
    }
    int32_t cluster = block - fs->data_start + 2;

    // check if iopin file cluster moved
    for (uint16_t i=0; i<NUM_IODIRS; i++) {
        watched_dir *dir = &iodirs[i];
        if (cluster >= dir->start_cluster && cluster < dir->start_cluster+NUM_DIR_CLUSTERS) {
            changed_iodir_block(fs, cluster, dir);
            return true;
        }
    }


    // runfile watcher - refactor when have another usage
    if (cluster >= iodir_start_cluster && cluster < iodir_start_cluster+NUM_DIR_CLUSTERS) {
        uint8_t *entry = find_sfn_entry(fs->buf, RUNFILE, 0xDF, 0);
        if (entry) {
            uint32_t new_mod = uint32_value(entry+22);
            uint16_t new_cluster = fatdirent_get_cluster(entry);
            if (new_cluster && (new_mod != runfile_mod || new_cluster != runfile_cluster)) {
                runfile_cluster = new_cluster;
                runfile_triggered = true;
                runfile_mod = new_mod;
                return true;
            }
        }
    }

    return false;
}

void check_runfile_changed(void)
{
    if (runfile_triggered && millis() > last_write+100) {
        runfile_changed(runfile_cluster);
        runfile_triggered = 0;
    }
}

#include "bootloader_leds.h"
//
// Setup handlers/watchers
//
bool iodir_init(sd_disk *fs)
{
    // find the io dir clusters

    iodir_start_cluster = find_lfn_file(-1, "io", true, fs);

    if (!iodir_start_cluster) {
        return false;
    }
    // iopin dirs
    for (uint16_t dir = 0; dir < NUM_IODIRS; dir++) {
        watched_dir *current = &iodirs[dir];
        uint8_t count = 0;
        // find directory and its clusters
        current->start_cluster = find_lfn_file(iodir_start_cluster, current->name, true, fs);
        for (uint16_t p=0; p<current->size; p++) {
            current->data[p].id = 0xFF;
        }

        for (uint16_t i=0; i<NUM_DIR_CLUSTERS; i++) {
            changed_iodir_block(fs, current->start_cluster+i, current);
        }
    }

    for (uint16_t file=0; file<NUM_HANDLEDFILES; file++) {
        handled_files[file].cluster = find_lfn_file(iodir_start_cluster, handled_files[file].name, false, fs);
    }

    sd = fs;

    devdir_start_cluster = find_lfn_file(iodir_start_cluster, "dev", true, fs);

    // RUNFILE
    runfile_cluster = find_sfn_file(iodir_start_cluster, RUNFILE, false, fs);

    return true;
};


void execute_runfile(void)
{
    runfile_changed(runfile_cluster);
}

void device_handlers_clear(void)
{
    devices = 0;
}

void attach_device_handler(const char *name, void *handler)
{
    device_file *device = data_alloc(sizeof(device_file));
    store_next((void **)&devices, device);

    strncpy(device->name, name, 11);
    device->handler = handler;
    device->next = 0;
}

void find_device_files()
{
    for (device_file *device = devices; device; device = device->next) {
        char *sfn = fat_sfn(device->name);
        device->cluster = find_sfn_file(devdir_start_cluster, sfn, false, sd);
        strncpy(device->name, sfn, 11);
    }
}


//
// read/write blocks called from scsi layer
//

//
// USB helpers
//
void wait_until_IN_ready(void)
{
    if (!(Endpoint_IsReadWriteAllowed()))
    {
        Endpoint_ClearIN();
        if (Endpoint_WaitUntilReady()) {
          return;
        }
    }
}
void wait_until_OUT_ready(void)
{
    if (!(Endpoint_IsReadWriteAllowed()))
    {
        Endpoint_ClearOUT();
        if (Endpoint_WaitUntilReady())
          return;
    }
}


void usb_read_block(sd_disk *fs)
{
    for (uint16_t i=0; i<512; i++) {
        if (i % 64 == 0) {
            wait_until_OUT_ready();
        }
        fs->buf[i] = Endpoint_Read_8();

    }
}

void usb_write_block(sd_disk *fs)
{
    uint16_t i=0;
    for (i=0; i<512; i++) {
        if (i % 64 == 0) {
            wait_until_IN_ready();
        }
        Endpoint_Write_8(fs->buf[i]);
    }
}

void read_blocks(uint32_t start, uint16_t count, sd_disk *fs)
{
    if (Endpoint_WaitUntilReady()) {
        return;
    }

    for (int block=0; block<count; block++) {
        led_flash(GREEN);
        uint32_t current = start+block;

        if (!has_handler(current, fs, false)) {
            if (!sd_read_block(current, fs)) {
                led_error(SD_READWRITEERROR);
            }

        }
        usb_write_block(fs);
    }

    if (!(Endpoint_IsReadWriteAllowed())) {
        Endpoint_ClearIN();
    }
}


void write_blocks(uint32_t start, uint16_t count, sd_disk *sd)
{
    last_write = millis();
    if (Endpoint_WaitUntilReady()) {
      return;
    }
    static uint32_t fs_dd_block_count;
    bool dd = false;
    for (int block=0; block<count; block++) {
        uint32_t current = start+block;
        led_flash(RED);

        if (current == 1 || fs_dd_block_count == (current-1)) {
            fs_dd_block_count = current;
            dd = true;
        } else {
            // if we've been sent more than 450 blocks from cluster 1 in a row assume
            // file system rewrite and reboot to catch changes
            if (fs_dd_block_count > 450) {
                wdt_enable(WDTO_15MS);
                for (;;);
            }
            fs_dd_block_count = 1;
        }

        usb_read_block(sd);

        if (dd || !has_handler(current, sd, true)) {
            if (!sd_write_block(current, sd)) {
                led_error(SD_READWRITEERROR);
            } else {
                if (!dd) {
                    has_watcher(current, sd);
                }
            }
        }

    }

    if (!(Endpoint_IsReadWriteAllowed())) {
        Endpoint_ClearOUT();
    }
}
