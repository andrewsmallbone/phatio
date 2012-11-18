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
#ifndef LIOBUILTINS_H_
#define LIOBUILTINS_H_

#include "lio.h"


typedef struct reference
{
    struct reference *next;
    const char *name;
    void *data;
    uint8_t type;
    uint8_t length;
} reference;
extern reference *references;

#define references_clear() (references = 0)

void *eval_as_reference(Item *fp_or_name);

void *get_variable_ref(Item *pointer_or_name);
Item *get_variable_item(Item *reference_item);
void *resolve_reference(const char *name, lio_type *type);


Item *defvar(List *expression);
Item *setvar(List *expression);
Item *getvar(List *expression);


#define LIO_REFERENCE_HANDLERS      &defvar, &defvar, &defvar,&setvar,&setvar,&getvar
#define LIO_REFERENCE_HANDLER_NAMES "defvar\0defconst\0defun\0setvar\0=\0getvar\0"

#endif // LIOBUILTINS_H_
