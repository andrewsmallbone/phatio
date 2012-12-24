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

#ifndef LIO_BASICS_H_
#define LIO_BASICS_H_

#include "lio.h"

Item *add(List *expression);
Item *subtract(List *expression);
Item *multiply(List *expression);
Item *divide(List *expression);
Item *modulus(List *expression);
Item *inverse(List *expression);
Item *bitwise_or(List *expression);
Item *bitwise_and(List *expression);
Item *lshift(List *expression);
Item *rshift(List *expression);
Item *if_handler(List *expression);
Item *eq_handler(List *expression);
Item *lt_handler(List *expression);
Item *gt_handler(List *expression);
Item *and_handler(List *expression);
Item *or_handler(List *expression);
Item *not_handler(List *expression);
Item *increment_handler(List *expression);

//Item *for_handler(List *expression);
Item *while_handler(List *expression);
Item *delay_ms_handler(List *expression);


#define LIO_BASIC_HANDLER_NAMES "+\0-\0*\0/\0%\0~\0|\0&\0<<\0>>\0if\0eq\0<\0>\0and\0or\0not\0+=\0while\0delay_ms\0"
#define LIO_BASIC_HANDLERS &add, &subtract, &multiply, &divide, &modulus, &inverse,&bitwise_or, &bitwise_and,\
        &lshift, &rshift, &if_handler, &eq_handler,&lt_handler,&gt_handler,&and_handler, &or_handler, &not_handler, &increment_handler,\
        &while_handler, &delay_ms_handler


#endif /* LIO_BASICS_H_ */
