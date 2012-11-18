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

#include <alloca.h>

#include "usb.h"
#include "config.h"
#include "util.h"
#include "sd.h"
#include "scheduler.h"
#include "adc.h"
#include "pwm.h"
#include "pins.h"
#include "disk.h"
#include "lio.h"
#include "lio_run.h"

#include "lio_twi.h"


int main(void)
{
    sd_disk disk;

    config_init();
     led_init();
     pin_init();

    scheduler_init();
    bool sd_present_on_startup = true;

     // wait until sd card readable
    while (sd_init(&disk)) {
        sd_present_on_startup = false;
        LED_ON(RED);
         delay_ms(200);
         LED_OFF(RED);
         delay_ms(800);
     }

     if (!disk_init(&disk)) {
         led_error(DISK_ERROR);
     } else if (!iodir_init(&disk)) {
         led_error(IODIR_ERROR);
     }

     if (sd_present_on_startup) {
         LED_ON(RED);
         execute_runfile();
     }

     usb_setup(&disk);
    pwm_init();

    for (;;) {
        usb_task();
        check_runfile_changed();
        perform_triggered_task();
     }
}
