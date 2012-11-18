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
#include <avr/pgmspace.h>

#include "lio_references.h"
#include "lio_run.h"
#include "lio_builtins.h"

reference *references = 0;

reference *define_variable(const char *name, lio_type type, uint16_t data_length, uint16_t data_size)
{
    char *name_buf = data_alloc(strlen(name)+1);
    reference *ref = (reference *)data_alloc(sizeof(reference)+data_length*data_size);
    if (name_buf && ref) {
        strcpy(name_buf, name);
        ref->name = name_buf;
        ref->type = type;
        ref->length = data_length;
        ref->data = (ref+1); // data is after ref
        ref->next = 0;
    }
    return ref;
}

#define uint8p(x) ((uint8_t *)(x))
#define uint16p(x) ((uint16_t *)(x))

// (defvar name value...)
Item *defvar(List *expression)
{
    uint8_t data_size = 1;
    uint8_t data_length = 0;
    List *data = rest(rest(expression));
    lio_type type = item_type(first(data));

    if (type == LIO_UINT8 | type == LIO_UINT16) {
        data_length = 1;
        data_size = 2;
        for (List *value=data; value; value = rest(value), data_length++);
    } else if (type == LIO_STRING) {
        data_length = strlen(eval_as_string(first(data))) + 1;
    }
    reference *ref = define_variable(eval_as_string(second(expression)), type, data_length, data_size);

    if (ref) {
        // copy data
        if (type == LIO_LIST) {
            ref->data = (uint16_t *)store_expression(data);
        } else if (type == LIO_STRING) {
            strcpy(uint8p(ref->data), eval_as_string(first(data)));
        } else {
            uint16_t i=0;
            uint16_t *u16data = uint16p(ref->data);
            for (List *val=rest(rest(expression)); val; val = rest(val)) {
                u16data[i] = eval_as_uint16(first(val));
                i++;
            }
        }
        store_next((void **)&references, ref);
    }
    return 0;
}



Item *get_variable(reference *ref, uint8_t has_index, int16_t index)
{
    if (ref) {
        if (has_index && (index < 0 || index > ref->length-1)) {
            return 0;
        }
        if (ref->type == LIO_LIST) {
            return eval_expressions((List *)ref->data);
        } else if (ref->type == LIO_STRING) {
            if (has_index) {
                return uint16_retval(uint8p(ref->data)[index]);
            } else {
                strncpy(str_buf, uint8p(ref->data), ref->length);
                return string_retval();
            }
        } else {
            return uint16_retval(uint16p(ref->data)[has_index ? index : 0]);
        }
    }
    lio_stop("unknown reference");
    return 0;
}

// (setvar name [index] value)
Item *setvar(List *expression)
{
    reference *ref = get_variable_ref(second(expression));
    if (ref) {
        Item *three = third(expression);
        Item *four = fourth(expression);
        if (ref->type == LIO_STRING) {
            if (!four) {
                strncpy(uint8p(ref->data), eval_as_string(three), ref->length);
                uint8p(ref->data)[ref->length-1] = 0;
                return get_variable(ref, 0, 0);
            } else {
                uint16_t index = eval_as_uint16(three);
                uint8p(ref->data)[index] = eval_as_uint16(four);
                return get_variable(ref, 1, index);
            }
        }
        uint16_t third_arg = eval_as_uint16(three);
        if (four) {
            uint16_t value = eval_as_uint16(four);
            uint16p(ref->data)[third_arg] = value;
            return uint16_retval(value);
        } else {
            *uint16p(ref->data) = third_arg;
            return uint16_retval(third_arg);
        }
    }
    lio_stop("unknown reference");
    return 0;
}


// (getvar name [index])
Item *getvar(List *expression)
{
    Item *three = third(expression);
    int16_t index = three ? eval_as_uint8(three) : 0;
    return get_variable(get_variable_ref(second(expression)), three != 0, index);
//    if (ref) {
//        uint8_t index = eval_as_uint8(third(expression));
//        return uint16_retval(ref->data[index]);
//    }
//    lio_stop("unknown reference");
//    return 0;
}



Item *get_variable_item(Item *item)
{
    return get_variable(get_variable_ref(item), 0, 0);
//
//    reference *ref = get_variable_ref(item);
//    if (ref) {
//        if (ref->type == LIO_LIST) {
//            return eval_expressions((List *)ref->data);
//        } else {
//            return uint16_retval(ref->data[0]);
//        }
//    }
//    return 0;
}

void *get_variable_by_name(const char *name)
{
    for (reference *ref = references; ref; ref = ref->next) {
        if (strcmp(name, ref->name) == 0) {
            return ref;
        }
    }
    return 0;
}


// eval an item to a reference
void *get_variable_ref(Item *pointer_or_name)
{
    lio_type type = item_type(pointer_or_name);
    if (type == LIO_VARIABLE) {
        return read_pointer(pointer_or_name);
    } else {
        return get_variable_by_name(eval_as_string(pointer_or_name));
    }
}

// get reference (variable or c function) by name
// returning the pointer to the source and setting type
void *resolve_reference(const char *name, lio_type *type)
{

    // check 'dynamic' references - vars etc.
    reference *ref = get_variable_by_name(name);
    if (ref) {
        *type = LIO_VARIABLE;
        return ref;
    }

    // check 'static' references - builtin functions
    int16_t index = find_index(name, builtin_names);
    if (index != -1) {
        *type = LIO_FUNCTION;
        return (void *) pgm_read_word(&builtin_handlers[index]);
    }
    return 0;
}


