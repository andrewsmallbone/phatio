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
#include "char_source.h"
#include "sd.h"
#include "disk.h"
#include "lio_run.h"


//
// File source implementation
//
static char file_reader(void);
void file_putbacker(const char c);
static int16_t buf_index;
static uint16_t file_cluster;
static char_source fsource = {.read = file_reader, .putback = file_putbacker};

char_source *file_source(uint16_t cluster)
{
    file_cluster = cluster;
    if (!cluster || !sd_read_block(cluster_to_block(sd, cluster), sd)) {
        lio_stop("failed to read run.lio");
        buf_index = -1;
    } else {
        buf_index = 0;
    }
    return &fsource;
}

char file_reader(void)
{
    if (buf_index == 512) {
        file_source(get_next_cluster(file_cluster, sd));
    }
    if (buf_index > -1 && buf_index<512) {
        char retVal = sd->buf[buf_index++];
        if (retVal == 0) {
            buf_index = -1;
        }
        return retVal;
    }
    return 0;
}

void file_putbacker(const char c)
{
    buf_index--;
}

