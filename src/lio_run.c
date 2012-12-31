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

#include <avr/interrupt.h>

#include "lio_run.h"
#include "lio_conditions.h"
#include "lio_references.h"
#include "bootloader_shared.h"
#include "scheduler.h"
#include "disk.h"
#include "char_source.h"
#include "config.h"

#define END_FLASH 0x7000   // 28K
static uint16_t current = START_FLASH;

static uint8_t (*write_to_flash)(uint16_t address, uint8_t *buf) = SHARED_LOAD_PAGE;

// Stores the given expression in flash returning the address.
// Currently uses whole chunks of SPM_PAGESIZE (128bytes)
List *store_expression(List *expr)
{
	if (is_in_flash(expr)) {
		return expr;
	}
    uint16_t flash_start = current;

    uint8_t *start = (uint8_t *)expr;
    uint16_t size = list_size(expr);

    for (uint16_t i=0; i<size && current < END_FLASH; i+=SPM_PAGESIZE, current+=SPM_PAGESIZE) {
        // only copy to flash if different from current contents. - following seems cheaper than memcmp_P()
        for (uint16_t b=0; b<SPM_PAGESIZE && (i+b)<size; b++) {
            if (*(start+i+b) != pgm_read_byte(current+b)) {
                write_to_flash(current, start+i);
                break;
            }
        }
    }
    if (current >= END_FLASH) {
        lio_stop("out of space");
    }
    //key_printf("%d", list_size(expr));
    return (List *)flash_start;
}

#define LIOBUFSIZE 255
void eval_text(char_source *src)
{
    uint8_t liobuf[LIOBUFSIZE];
    List *expression;
    while ((expression = next_expression(src, liobuf, LIOBUFSIZE)) != 0) {
        safe_eval(expression);
    }
}

void lio_stop(const char *message)
{
    cli();

    lio_stopped = true;
    current = START_FLASH;

    device_handlers_clear();
    scheduler_clear();
    conditions_clear();
    references_clear();

    if (message) {
        error("%s", message);
        if (get_bool_config(LIO_PRINT_ERRORS)) {
            key_printf("#phatIO: %s", message);
        }
    }
    sei();
}

void runfile_changed(uint16_t cluster)
{
    lio_stop(0);
    cli();
    lio_stopped = false;
    if (cluster) {
        eval_text(file_source(cluster));
        find_device_files();
        scheduler_add(5, conditions_check);
    }
    if (lio_stopped) {
        lio_stop(0); // ensure lio halted if evaluation fails
    } else {
        led_error(LEDSTATE_OFF);
    }
    sei();
}

