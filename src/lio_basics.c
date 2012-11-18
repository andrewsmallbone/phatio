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

#include "lio_basics.h"
#include "disk.h"
#include "lio_run.h"
#include "lio_references.h"


static uint16_t second_arg;
static uint16_t third_arg;

void parse_args(List *expression)
{
    uint16_t two = eval_as_uint16(second(expression));
    uint16_t three = eval_as_uint16(third(expression));
    second_arg = two;
    third_arg = three;
}

Item *add(List *expression)
{
    parse_args(expression);
    return uint16_retval(second_arg + third_arg);
}

Item *subtract(List *expression)
{
    parse_args(expression);
    return uint16_retval(second_arg - third_arg);
}

Item *multiply(List *expression)
{
    parse_args(expression);
    return uint16_retval(second_arg * third_arg);
}

Item *divide(List *expression)
{
    parse_args(expression);
    return uint16_retval(second_arg / third_arg);
}

Item *modulus(List *expression)
{
    parse_args(expression);
    return uint16_retval(second_arg % third_arg);
}

Item *inverse(List *expression)
{
    return uint16_retval(~eval_as_uint16(second(expression)));
}

Item *bitwise_or(List *expression)
{
    parse_args(expression);
    return uint16_retval(second_arg | third_arg);
}

Item *bitwise_and(List *expression)
{
    parse_args(expression);
    return uint16_retval(second_arg & third_arg);
}

Item *lshift(List *expression)
{
    parse_args(expression);
    return uint16_retval(second_arg << third_arg);
}

Item *rshift(List *expression)
{
    parse_args(expression);
    return uint16_retval(second_arg >> third_arg);
}

Item *if_handler(List *expression)
{
    Item *item = eval_as_uint8(second(expression)) ? third(expression) : fourth(expression);
    if (item_type(item) == LIO_LIST) {
        return eval(as_list(item));
    } else {
        return item;
    }
}


Item *eq_handler(List *expression)
{
    parse_args(expression);
    return uint8_retval(second_arg == third_arg ? 1 : 0);
}


Item *lt_handler(List *expression)
{
    parse_args(expression);
    return uint8_retval(second_arg < third_arg ? 1 : 0);
}

Item *gt_handler(List *expression)
{
    parse_args(expression);
    return uint8_retval(second_arg > third_arg ? 1 : 0);
}

Item *and_handler(List *expression)
{
    for (List *args = rest(expression); args; args = rest(args)) {
        if (!eval_as_uint8(first(args))) {
            return uint8_retval(0);
        }
    }
    return uint8_retval(1);
}

Item *or_handler(List *expression)
{
    for (List *args = rest(expression); args; args = rest(args)) {
        if (eval_as_uint8(first(args))) {
            return uint8_retval(1);
        }
    }
    return uint8_retval(0);
}

Item *not_handler(List *expression)
{
    return uint8_retval(eval_as_uint8(second(expression)) ? 0 : 1);

}

Item *increment_handler(List *expression)
{
    reference *ref = get_variable_ref(second(expression));
    if (ref) {
        uint16_t value = eval_as_uint16(third(expression));

        *((uint16_t *)ref->data) += value;
        return uint16_retval(*((uint16_t *)ref->data));
    }
    lio_stop("unknown reference");
    return 0;

}

//// (for (start expression) (test expression) (loop action expression).... code....)
//// (for reference (list) code...
//Item *for_handler(List *expression)
//{
//    eval(as_list(second(expression)));
//
//    if (is_list(third(expression))) {
//        List *test = as_list(third(expression));
//        List *action = as_list(fourth(expression));
//        List *code = rest(rest(rest(rest(expression))));
//        while (eval_as_uint8(eval(test))) {
//            if (code) {
//                eval_expressions(code);
//            }
//            eval(action);
//        }
//    }
//    return 0;
//}

// (while (expression) code....)
Item *while_handler(List *expression)
{
    Item *test = second(expression);
    List *code = rest(rest(expression));
    Item *retval = 0;
    if (is_list(test) && code) {
        while (eval_as_uint8(eval(as_list(test)))) {
            retval = eval_expressions(code);
        }
    }
    return retval;
}
