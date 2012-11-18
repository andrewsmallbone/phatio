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
#include <stdio.h>
#include <avr/interrupt.h>

#include "lio.h"
#include "log.h"
#include "util.h"
#include "lio_references.h"
#include "lio_builtins.h"
#include "lio_run.h"
#include "bootloader_shared.h"

#define QUOTE '"'
bool lio_stopped;

Item *first(List *item)
{
    return (item == 0) ? 0 : &(item->first);
}

Item *second(List *list)
{
    return first(rest(list));
}

Item *third(List *list)
{
    return first(rest(rest(list)));
}

Item *fourth(List *list)
{
    return first(rest(rest(rest(list))));
}

Item *fifth(List *list)
{
    return first(rest(rest(rest(rest(list)))));
}

Item *uint8_retval(uint8_t value)
{
    static UInt8_Item returnHolder = { .type = LIO_UINT8 };
    returnHolder.value = value;
    return (Item *)&returnHolder;
}

Item *uint16_retval(uint16_t value)
{
    static UInt16_Item returnHolder = { .type = LIO_UINT16 };
    returnHolder.value = value;
    return (Item *)&returnHolder;
}

static String_Item stringHolder = { .type = LIO_STRING };
char *str_buf = stringHolder.value;

Item *string_retval(void)
{
    return (Item *)&stringHolder;
}

void *read_pointer(Item *item)
{
    return is_in_flash(item) ? (void **)pgm_read_word(value(item)) : *((void **)value(item));
}


List *rest(List *item)
{
    if (item == 0) {
        return (List *)0;
    } else {
        uint8_t rest = is_in_flash(item) ? (uint8_t )(pgm_read_byte(&item->rest)) : item->rest;
        return (rest == 0) ? 0 : (List *) ((void *)item+rest);
    }
}

Item *eval_expressions(List *expression)
{
    Item *retVal;
    for (List *remaining = expression; remaining; remaining = rest(remaining)) {
        Item *expr = first(remaining);
        if (LIO_LIST == item_type(expr)) {
            retVal = eval(as_list(expr));
        }
    }
    return retVal;
}


Item *eval(List *expression)
{
    Item *head = first(expression);
    lio_type type = item_type(head);
    if (type == LIO_LIST) {
        return eval_expressions(expression);
    } else if (type == LIO_FUNCTION) {
        builtin_handler *handler = read_pointer(head);
        return handler(expression);
    } else if (type == LIO_VARIABLE) {
        reference *ref = get_variable_ref(head);
        if (ref->type == LIO_LIST) {
            return eval_expressions((List *)ref->data);
        }
        // error???
    }
    return 0;
}

uint8_t item_type(Item *item)
{
    if (item == 0) {
        return 0;
    } else {
        return is_in_flash(item) ? pgm_read_byte(&item->type) : item->type;
    }
}

const char *eval_as_string(Item *item)
{
    uint8_t type = item_type(item);

    if (type == LIO_VARIABLE) {
        return eval_as_string(get_variable_item(item));
    }

    if (type == LIO_LIST) {
        return eval_as_string(eval(as_list(item)));
    }
    if (type == LIO_ATOM || type == LIO_STRING) {
        if (is_in_flash(item)) {
            strncpy_P(str_buf, value(item), STRBUFSIZE);
            return str_buf;
        } else {
            return value(item);
        }
    }
    if (type == LIO_UINT8) {
        print_number(str_buf, *((uint8_t *)value(item)));
        return str_buf;
    }
    if (type == LIO_UINT16) {
        print_number(str_buf, *((uint16_t *)value(item)));
        return str_buf;
    }

    return 0;
}

#define char_equals(to_test, required) (to_test == required || to_test == (required-'a'+'A'))

uint16_t eval_as_uint16(Item *item)
{
    uint8_t type = item_type(item);
    if (type == LIO_VARIABLE) {
        return eval_as_uint16(get_variable_item(item));
    }
    if (type == LIO_LIST) {
        return eval_as_uint16(eval(as_list(item)));
    }
    if (type == LIO_UINT16) {
        return is_in_flash(item) ? pgm_read_word(value(item)) : *((uint16_t *)value(item));
    }
    if (type == LIO_UINT8) {
        return eval_as_uint8(item);
    }
    if (type == LIO_ATOM || type == LIO_STRING) {
        uint16_t ret = -1;
        const char *val = eval_as_string(item);
        parse_number(val, &ret);
//        if (val[0] == '0' && char_equals(val[1], 'x')) {
//            sscanf(val+2, "%X", &ret);
//        } else {
//            sscanf(val, "%u", &ret);
//        }
        return ret;
    }
    return 0;
}

uint8_t eval_as_uint8(Item *item)
{
    uint8_t type = item_type(item);

    if (type == LIO_UINT8) {
        return is_in_flash(item) ? pgm_read_byte(value(item)) : *((uint8_t *)value(item));
    } else {
        return 0xFF & eval_as_uint16(item);
    }
}

uint16_t item_size(Item *item)
{
    if (item == 0) {
        return 0;
    }
    switch (item_type(item)) {
    case LIO_LIST:
        return 1+list_size(as_list(item));
	case LIO_FUNCTION:
	case LIO_VARIABLE:
	    return 3;
    case LIO_ATOM:
    case LIO_STRING:
        return 2+ (is_in_flash(item) ? strlen_P(value(item)) : strlen(value(item)));
    case LIO_UINT8:
        return 2;
    case LIO_UINT16:
        return 3;
    default:
        return 0;
    }
}

uint16_t list_size(List *list)
{
    uint16_t size = 0;
    while (list) {
        size += 1+item_size(first(list));
        list = rest(list);
    }
    return size;
}

bool set_current(List *current, uint16_t size, lio_type type, uint8_t *end)
{
    current->rest = size + LISTITEM_SIZE;
    current->first.type = type;
    if ((uint8_t *)current + current->rest >= end) {
        return false;
    }
    return true;
}

bool is_number(const uint8_t *buf, uint16_t *value)
{
    if (buf[0] == '\'' && buf[2] == '\'') {
        *value = buf[1];
        return true;
    } else {
        return parse_number(buf, value);
    }
}


List *next_expression(char_source *source, uint8_t *buffer, uint16_t buffer_size)
{
    uint16_t tmp_integer;
    List *current = (List* )buffer;
    uint8_t *end = buffer + buffer_size - 2; // end of buffer - space for last element rest fields

    #define MAXDEPTH 10
    List *enclosing[MAXDEPTH];
    List *last[MAXDEPTH];

    int8_t depth = 0;
    next_token(source, str_buf, STRBUFSIZE);
    if (lio_stopped || !str_buf[0]) {
        return 0;
    } else if (str_buf[0] != '(') {
        lio_stop("start of expression not found - check braces");
        return 0;
    }
    bool first_in_list = true;
    while ((uint8_t *)current < end && next_token(source, str_buf, STRBUFSIZE)) {
        if (str_buf[0] == '(') {

            if (depth == MAXDEPTH) {
                lio_stop("To many nested expressions");
                return 0;
            }
            if (!set_current(current, 0, LIO_LIST, end)) break;
            for (int i=depth-1; i>=0; i--) {
                enclosing[i]->rest += current->rest;
            }

            enclosing[depth] = current;
            last[depth] = current;
            current = rest(current);
            current->rest = 0;
            first_in_list = true;
            depth++;
            last[depth] = 0;
        } else if (str_buf[0] == ')') {
            if (last[depth]) {
                last[depth]->rest = 0;
            }

            if (depth == 0) {
                return (List *)buffer;
            }
            depth--;
            first_in_list = false;

        } else {
            lio_type type;
            void * ref = resolve_reference(str_buf, &type);
            if (ref) {
                if (set_current(current, 2, type, end)) {
                    *((void **)value(&(current->first))) = ref;
                }
            } else if (first_in_list) {
                lio_stop("unexpected function");
                return 0;
            } else if (str_buf[0] == QUOTE) {
                str_buf[strlen(str_buf)-1] = 0;
                if (set_current(current, strlen(str_buf+1)+1, LIO_STRING, end)) {
                    strcpy(&(current->first.value), str_buf+1);
                }
            } else if (is_number(str_buf, &tmp_integer)) {
                if (tmp_integer <= 0xFF) {
                    if (set_current(current, 1, LIO_UINT8, end)) {
                        *((uint8_t *)value(&current->first)) = (uint8_t)tmp_integer;
                    }
                } else {
                    if (set_current(current, 2, LIO_UINT16, end)) {
                        *((uint16_t *)value(&current->first)) = (uint16_t)tmp_integer;
                    }
                }
            } else {
                if (set_current(current, strlen(str_buf)+1, LIO_ATOM, end)) {
                    strcpy(value(&current->first), str_buf);
                }
            }
            last[depth] = current;
            for (int i=depth-1; i>=0; i--) {
                enclosing[i]->rest += current->rest;
            }
            current = rest(current);
            first_in_list = false;
        }
    }
    if ((uint8_t *)current > end) {
        lio_stop("expression to large");
    } else {
        lio_stop("unexpected end of file");
    }
    return 0;
}


//bool next_token(uint8_t *buf, uint16_t length, int16_t *start, int16_t *end)
bool next_token(char_source *source, char *buf, uint16_t tmp_len)
{
    bool started_token = false;
    while(!lio_stopped && (*buf = source->read()) != 0) {
        switch (*buf) {
            case '#':
            case ';':
                while (*buf != '\n' && *buf != 0) {
                    *buf = source->read(); // eat to end of line or buffers
                }
                break;
            case ' ':
            case '\n':
            case '\t':
                if (started_token) {
                    *buf = 0; // terminate
                    return true;
                }
                break;
            case ')':
            case '(':
                if (started_token) {
                    source->putback(*buf);
                    *buf = 0; // send back previous token
                } else {
                    *(++buf) = 0; // send back this token.
                }
                return true;

                break;
            case QUOTE:
                if (started_token) {
                    source->putback(*buf);
                    *buf = 0;
                } else {
                    buf++;
                    for (char last = 0; !((*buf = source->read()) == 0 || (*buf == QUOTE && last != '\\')); buf++) {
                        last = *buf;
                    }
                    *(buf+1) = 0;
                }
                return true;
                break;
            default:
                started_token = true;
                buf++;
                break;
        }
    }
    return false;
}
