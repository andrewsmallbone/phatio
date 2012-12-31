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
#include <avr/io.h>
#include <avr/interrupt.h>
#include "scheduler.h"
#include "util.h"
#include "lio_run.h"
#include "log.h"

#define MILLISECOND 250

#define MAX_SCHEDULED 16
typedef struct scheduled_handler {
 uint16_t every_ms;
 void *handler;
} schedule;
volatile schedule tasks[MAX_SCHEDULED];

#define MAX_TRIGGERED 10
void *triggered_tasks[MAX_TRIGGERED];

#include "iopin.h"
static iopin_t deb;

uint8_t servo_value = 0;

iopin_t servo_pin;
uint16_t servo_duration;
bool servo_triggered;

bool on_millisecond = true;

ISR(TIMER0_COMPA_vect)
{
   // OUTPORT(deb) |= IOBV(deb);

    if (on_millisecond) {
        OCR0A == MILLISECOND;
        _timeSinceStart++;

//        if (servo_pin) {
//            if (!servo_triggered) {
//                servo_triggered = true;
//                set_out(servo_pin, 1);
//                if (servo_duration < 250) {
//                    OCR0A = servo_duration;
//                    on_millisecond = false;
//                }
//            } else {
//                servo_duration -= 250;
//                if (servo_duration > 0 && servo_duration < 250) {
//                    OCR0A = servo_duration;
//                    on_millisecond = false;
//                } else if (servo_duration <= 0) {
//                    set_out(servo_pin, 0);
//                    servo_pin = 0;
//                    servo_triggered = 0;
//                    servo_duration = 0;
//                }
//            }
//        }

//        if (_timeSinceStart % 20 == 0) {
//            pulse_pin(get_iopin(10), servo_value);
//        }

        for (uint16_t i=0; i<MAX_SCHEDULED; i++) {
            if (tasks[i].handler != 0 && ((_timeSinceStart+i) % tasks[i].every_ms == 0)) {
                if (is_in_flash(tasks[i].handler)) {
                    add_triggered_task(tasks[i].handler);
                } else {
                    void (*fp)(void) = (void (*)(void))tasks[i].handler;
                    fp();
                }

            }
        }
    } else {
//        if (servo_pin && servo_triggered) {
//            if (servo_duration < 250) {
//                set_out(servo_pin, 0);
//                servo_pin = 0;
//                servo_triggered = 0;
//                servo_duration = 0;
//            }
//        }
        OCR0A = MILLISECOND;
        on_millisecond = true;
    }

//    OUTPORT(deb) &= ~IOBV(deb);
}


Item *servo(List *expression)
{
    uint8_t x = eval_as_uint8(second(expression));
    servo_value = (x>250) ? 250 : x;

    return 0;
}


void scheduler_clear(void)
{
//    deb = get_iopin(9);
//    set_dir(deb, 1);

    for (uint16_t i=0; i<MAX_TRIGGERED; i++) {
        triggered_tasks[i] = 0;
    }
    for (uint16_t i=0; i<MAX_SCHEDULED; i++) {
        tasks[i].handler = 0;
    }
    scheduler_add(10, led_task);
}

void pulse_pin(iopin_t pin, uint16_t duration)
{
    servo_duration = 200 + 2*duration;
    servo_triggered = false;
    servo_pin = pin;
}

uint8_t scheduler_add(uint16_t every, void *handler)
{
    for (uint8_t i=0; i<MAX_SCHEDULED; i++) {
    	if (tasks[i].handler == 0) {
	        tasks[i].handler = handler;
	        tasks[i].every_ms = every;
    		return i+1;
    	}
	}
	return 0;
}

void scheduler_remove(uint8_t scheduled)
{
    if (scheduled > 0) {
		tasks[scheduled-1].handler = 0;
		tasks[scheduled-1].every_ms = 0;
	}
}

void scheduler_init(void)
{
    scheduler_clear();
    // TIMER0_COMPA_vect interrupt every CLK/64/OCR0A = (1KHz...250KHz)
    TCCR0A |= _BV(WGM01);
    TCCR0B |=  _BV(CS00) |  _BV(CS01);
    TIMSK0 |= _BV(OCIE0A);
    OCR0A = MILLISECOND;
}

void add_triggered_task(void *handler)
{
    for (uint16_t i=0; i<MAX_TRIGGERED; i++) {
        if (!triggered_tasks[i]) {
            triggered_tasks[i] = handler;
            break;
        }
    }
}

void perform_triggered_task(void)
{
    cli();
    void *handler = 0;
    for (uint16_t i=0; i<MAX_TRIGGERED; i++) {
        if (triggered_tasks[i]) {
            handler = triggered_tasks[i];
            triggered_tasks[i] = 0;
            break;
        }
    }
    sei();
    if (handler) {
        if (is_in_flash(handler)) {
            safe_evalexpressions((List *)handler);
        } else {
            void (*fp)(void) = (void (*)(void))handler;
            fp();
        }
    }
}

Item *every(List *expression)
{
    uint8_t retval = scheduler_add(eval_as_uint16(second(expression)),store_expression(rest(rest(expression))));
    return uint8_retval(retval);
}

Item *delete_schedule(List *expression)
{
	scheduler_remove(eval_as_uint8(second(expression)));
	return 0;
}
