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

#include "lio_conditions.h"
#include "lio_run.h"
#include "iopin.h"
#include "adc.h"
#include "scheduler.h"
#include "log.h"

#define TOGGLE 2

typedef struct digital_condition {
    struct digital_condition *next;
    iopin_t pin;
    List *action;
    unsigned value :1;
    unsigned trigger_value :2;
} digital_condition;


typedef struct adc_condition {
    struct adc_condition *next;
    uint8_t pinid;
    uint16_t trigger_value;
    uint16_t limit;
    List *action;
    unsigned triggered_before :1;
    unsigned above :1;
} adc_condition;

static adc_condition *adc_conditions;
static digital_condition *digital_conditions;


//
// simple digital conditions
//
void check_digital_condition(digital_condition *condition)
{
    const uint8_t new_val = get_in(condition->pin);

    if (new_val != condition->value) {
        condition->value = new_val;
        if (condition->trigger_value == TOGGLE || new_val == condition->trigger_value) {
            add_triggered_task(condition->action);
        }
    }
}

void new_digital_condition(List *expression, uint8_t trigger)
{
    digital_condition *condition = data_alloc(sizeof(digital_condition));
    if (condition) {
        store_next((void **)&digital_conditions, condition);

        condition->pin = get_iopin(eval_as_uint8(second(expression)));
        condition->value = get_in(condition->pin);
        condition->trigger_value = trigger;
        condition->action = store_expression(rest(rest(expression)));
    }
}

Item *pin_goes_high(List *expression)
{
    new_digital_condition(expression, 1);
}

Item *pin_goes_low(List *expression)
{
    new_digital_condition(expression, 0);

}

Item *pin_changes(List *expression)
{
    new_digital_condition(expression, TOGGLE);
}


void check_adc_condition(adc_condition *condition)
{
    const int16_t current_value = read_adc(condition->pinid);

    if ((condition->above && (current_value > condition->trigger_value))
            || (!condition->above && (current_value < condition->trigger_value))) {
        if (!(condition->triggered_before)) {
            condition->triggered_before = 1;
            add_triggered_task(condition->action);
        }
    } else {
        if ((condition->above && (current_value < (condition->limit)))
                || (!condition->above && (current_value > condition->limit))) {
            condition->triggered_before = 0;
        }
    }
}

adc_condition new_adc_condition(List *expression, bool above)
{
    adc_condition *condition = data_alloc(sizeof(adc_condition));
    if (condition) {
        store_next((void **)&adc_conditions, condition);
        condition->pinid = eval_as_uint8(second(expression));
        condition->trigger_value = eval_as_uint16(third(expression));

        const bool has_limit = (item_type(fourth(expression)) != LIO_LIST);
        condition->limit = has_limit ? eval_as_uint16(fourth(expression)) : condition->trigger_value;
        condition->action = store_expression(rest(rest(rest(has_limit ? rest(expression) : expression))));
        condition->triggered_before = 0;
        condition->above = above;
    }
}

Item *adc_goes_above(List *expression)
{
    new_adc_condition(expression, true);
}

Item *adc_goes_below(List *expression)
{
    new_adc_condition(expression, false);
}


// clear and check conditions
void conditions_clear(void)
{
    digital_conditions = 0;
    adc_conditions = 0;
}

void conditions_check(void)
{
    for (digital_condition *condition = digital_conditions; condition; condition = condition->next) {
        check_digital_condition(condition);
    }
    for (adc_condition *condition = adc_conditions; condition; condition = condition->next) {
        check_adc_condition(condition);
    }
}


