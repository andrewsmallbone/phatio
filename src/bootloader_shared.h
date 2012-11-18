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
// location of shared functions
#define SHAREDFPS(i) ((void *)((0x7000+(i*2))/2))

#define SHARED_SD_INIT          SHAREDFPS(1)
#define SHARED_WAIT_FOR         SHAREDFPS(2)
#define SHARED_SEND_CMD_CRC     SHAREDFPS(3)

#define SHARED_SD_READ_START    SHAREDFPS(4)
#define SHARED_SD_READ_FINISHED SHAREDFPS(5)
#define SHARED_SD_READ_BLOCK    SHAREDFPS(6)
#define SHARED_SD_WRITE_START    SHAREDFPS(7)
#define SHARED_SD_WRITE_FINISHED SHAREDFPS(8)
#define SHARED_SD_WRITE_BLOCK    SHAREDFPS(9)

#define SHARED_DISK_INIT        SHAREDFPS(10)
#define SHARED_GET_NEXT_CLUSTER SHAREDFPS(11)
#define SHARED_FIND_SFN_ENTRY   SHAREDFPS(12)
#define SHARED_FIND_SFN_FILE    SHAREDFPS(13)
#define SHARED_LOAD_PAGE        SHAREDFPS(14)
