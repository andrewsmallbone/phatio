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
#ifndef LIO_H_
#define LIO_H_

#include <stdint.h>
#include <stdbool.h>
#include "util.h"
#include "log.h"
#include "char_source.h"

#define START_FLASH 0x6800 // 26K
#define is_in_flash(ptr) ((void *)ptr>=(void*)START_FLASH)

#define STRBUFSIZE 48
extern char *str_buf;
extern bool lio_stopped;

typedef uint8_t lio_type;
#define LIO_LIST 1
#define LIO_ATOM 2
#define LIO_FUNCTION 3
#define LIO_VARIABLE 4
#define LIO_STRING 5
#define LIO_UINT8 6
#define LIO_UINT16 7
#define LIO_UINT32 8

#define lio_error(fmt, ...) lio_stop(error_P(PSTR(fmt), ##__VA_ARGS__))

typedef struct Item {
    uint8_t type;
    uint8_t value;
} Item;

typedef struct {
    uint8_t type;
    uint8_t value;
} UInt8_Item;

typedef struct {
    uint8_t type;
    uint16_t value;
} UInt16_Item;

typedef struct {
    uint8_t type;
    char value[STRBUFSIZE];
} String_Item;

typedef struct {
    uint8_t rest;
    Item first;
} List;


#define is_list(item) (item_type(item) == LIO_LIST)
#define ITEM_SIZE (sizeof(uint8_t))
#define LISTITEM_SIZE (sizeof(uint8_t)+ITEM_SIZE)

#define value(item_p) ((uint8_t *)item_p+ITEM_SIZE)
void *read_pointer(Item *item);

uint16_t item_size(Item *item);
uint16_t list_size(List *list);

uint8_t item_type(Item *ptr);
Item *first(List *litem);
List *rest(List *litem);
Item *second(List *expression);
Item *third(List *expression);
Item *fourth(List *expression);
Item *fifth(List *expression);

List *new_item(List *current, uint8_t itype, uint16_t ilength);
List *next_expression(char_source *source, uint8_t *buffer, uint16_t buffer_size);
bool next_token(char_source *source, char *buf, uint16_t tmp_len);

#define as_list(item_p) ((List *)value(item_p))

Item *eval_expressions(List *expression);
Item *eval(List *expression);
const char *eval_as_string(Item *item);
uint8_t eval_as_uint8(Item *item);
uint16_t eval_as_uint16(Item *item);

Item *uint16_retval(uint16_t value);
Item *uint8_retval(uint8_t value);
Item *string_retval(void);


#endif /* LIO_H_ */
