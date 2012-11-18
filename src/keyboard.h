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
#ifndef _keyboard_H_
#define _keyboard_H_

#include <stdint.h>

#define KEYBOARD_MODIFIER_LEFTCTRL					  (1 << 0)
#define KEYBOARD_MODIFIER_LEFTSHIFT					  (1 << 1)
#define KEYBOARD_MODIFIER_LEFTALT                     (1 << 2)
#define KEYBOARD_MODIFIER_LEFTGUI                     (1 << 3)
#define KEYBOARD_MODIFIER_RIGHTCTRL                   (1 << 4)
#define KEYBOARD_MODIFIER_RIGHTSHIFT                  (1 << 5)
#define KEYBOARD_MODIFIER_RIGHTALT                    (1 << 6)
#define KEYBOARD_MODIFIER_RIGHTGUI                    (1 << 7)

uint8_t map_keys(uint8_t* report, uint8_t *input, uint8_t extensions);
uint8_t map_extension_block(uint8_t* report, uint8_t *input);

#endif //_keyboard_H_
