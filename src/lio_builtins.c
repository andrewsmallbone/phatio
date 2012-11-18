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
#include "lio_builtins.h"

#include "lio_references.h"
#include "lio_io.h"
#include "lio_twi.h"
#include "lio_spi.h"
#include "lio_basics.h"
#include "config.h"
#include "lio_conditions.h"
#include "lio_file.h"
#include "scheduler.h"

//
// array of null separated function names and corresponding function pointers.
//
const char builtin_names[] PROGMEM =
        LIO_REFERENCE_HANDLER_NAMES
        LIO_IO_HANDLER_NAMES
        LIO_TWI_HANDLER_NAMES
        LIO_SPI_HANDLER_NAMES
        LIO_BASIC_HANDLER_NAMES
        LIO_CONFIG_HANDLER_NAMES
        LIO_CONDITION_HANDLER_NAMES
        LIO_FILE_HANDLER_NAMES
        LIO_SCHEDULER_HANDLER_NAMES
        "\0";

builtin_handler * const builtin_handlers[] PROGMEM = {
        LIO_REFERENCE_HANDLERS,
        LIO_IO_HANDLERS,
        LIO_TWI_HANDLERS,
        LIO_SPI_HANDLERS,
        LIO_BASIC_HANDLERS,
        LIO_CONFIG_HANDLERS,
        LIO_CONDITION_HANDLERS,
        LIO_FILE_HANDLERS,
        LIO_SCHEDULER_HANDLERS,
};

