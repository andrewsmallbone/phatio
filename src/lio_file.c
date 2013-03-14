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
#include "lio_file.h"

#include <stdio.h>
#include <avr/pgmspace.h>
#include "disk.h"
#include "sd.h"
#include "log.h"
#include "lio_run.h"

Item *read_hexbyte(List *expression)
{
    uint8_t read = eval_as_uint8(second(expression));
    uint8_t value = 0;
    sscanf_P(sd->buf+read, PSTR("%2X"), &value);
    return uint8_retval(value);
}

Item *read_byte(List *expression)
{
    uint8_t character = eval_as_uint8(second(expression));
    return uint8_retval(sd->buf[character]);
}

Item *driver(List *expression)
{
    attach_device_handler(eval_as_string(second(expression)), store_expression(rest(rest(expression))));
    return 0;
}

extern int16_t buf_index;
extern uint16_t file_cluster;

Item *import(List *expression)
{
	const uint8_t *filename = eval_as_string(second(expression));

	// remember existing file
	int16_t old_index = buf_index;
	uint16_t old_cluster = file_cluster;

	// load and eval new file
	uint16_t lib = find_lfn_file(find_lfn_file(-1, "io", true, sd), "lib", true, sd);

	uint16_t file = find_lfn_file(lib, filename, false, sd);
    eval_text(file_source(file));

    // resurrect existing file
	sd_read_block(cluster_to_block(sd, old_cluster), sd);
	buf_index = old_index;
	file_cluster = old_cluster;
	return 0;
}
