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

#include "keyboard.h"
#include <avr/pgmspace.h>
#include "util.h"


#define KEYBOARD_SHIFT (1<<7)

static const uint8_t  ascii_from32[] PROGMEM = {
  /* ascii 32 = ' ' */ 0x2C,
  /* ascii 33 = '!' */ 0x1E | KEYBOARD_SHIFT,
  /* ascii 34 = '"' */ 0x34 | KEYBOARD_SHIFT,
  /* ascii 35 = '#' */ 0x20 | KEYBOARD_SHIFT,
  /* ascii 36 = '$' */ 0x21 | KEYBOARD_SHIFT,
  /* ascii 37 = '%' */ 0x22 | KEYBOARD_SHIFT,
  /* ascii 38 = '&' */ 0x24 | KEYBOARD_SHIFT,
  /* ascii 39 = ''' */ 0x34,
  /* ascii 40 = '(' */ 0x26 | KEYBOARD_SHIFT,
  /* ascii 41 = ')' */ 0x27 | KEYBOARD_SHIFT,
  /* ascii 42 = '*' */ 0x25 | KEYBOARD_SHIFT,
  /* ascii 43 = '+' */ 0x2E | KEYBOARD_SHIFT,
  /* ascii 44 = ',' */ 0x36,
  /* ascii 45 = '-' */ 0x2D,
  /* ascii 46 = '.' */ 0x37,
  /* ascii 47 = '/' */ 0x38,
  /* ascii 48 = '0' */ 0x27,
  /* ascii 49 = '1' */ 0x1E,
  /* ascii 50 = '2' */ 0x1F,
  /* ascii 51 = '3' */ 0x20,
  /* ascii 52 = '4' */ 0x21,
  /* ascii 53 = '5' */ 0x22,
  /* ascii 54 = '6' */ 0x23,
  /* ascii 55 = '7' */ 0x24,
  /* ascii 56 = '8' */ 0x25,
  /* ascii 57 = '9' */ 0x26,
  /* ascii 58 = ':' */ 0x33| KEYBOARD_SHIFT,
  /* ascii 59 = ';' */ 0x33,
  /* ascii 60 = '<' */ 0x36 | KEYBOARD_SHIFT,
  /* ascii 61 = '=' */ 0x2E,
  /* ascii 62 = '>' */ 0x37 | KEYBOARD_SHIFT,
  /* ascii 63 = '?' */ 0x38 | KEYBOARD_SHIFT,
  /* ascii 64 = '@' */ 0x1F | KEYBOARD_SHIFT
};

static const uint8_t ascii_from91[] PROGMEM = {
  /* ascii 91 = '[' */ 0x2F,
  /* ascii 92 = '\' */ 0x31,
  /* ascii 93 = ']' */ 0x30,
  /* ascii 94 = '^' */ 0x23 | KEYBOARD_SHIFT,
  /* ascii 95 = '_' */ 0x2D | KEYBOARD_SHIFT,
  /* ascii 96 = '`' */ 0x34
};

static const uint8_t ascii_from123[] PROGMEM = {
  /* ascii 123 = '{' */ 0x2F | KEYBOARD_SHIFT,
  /* ascii 124 = '|' */ 0x31 | KEYBOARD_SHIFT,
  /* ascii 125 = '}' */ 0x30 | KEYBOARD_SHIFT,
  /* ascii 126 = '~' */ 0x38 | KEYBOARD_SHIFT,
  /* ascii 127 = '\b' */ 0x2A
};


#define modifier(x) x[0]
#define add_keycode(report, key) { for (int k=0; k<6; k++) { if (report[2+k] == 0) {report[2+k] = key; break; }}}



char const lc[] PROGMEM = "LeftControl";
char const ls[] PROGMEM = "LeftShift";
char const la[] PROGMEM = "LeftAlt";
char const lg[] PROGMEM = "LeftGUI";
char const rc[] PROGMEM = "RightControl";
char const rs[] PROGMEM = "RightShift";
char const ra[] PROGMEM = "RightAlt";
char const rg[] PROGMEM = "RightGUI";

#define NUM_MODIFIERS 8
const char PROGMEM * const modifiers[NUM_MODIFIERS] PROGMEM = { lc, ls, la, lg, rc, rs, ra, rg};



void map_ascii_key(uint8_t *report, uint8_t key)
{
     add_keycode(report, key & ~(KEYBOARD_SHIFT));
     if ((key & KEYBOARD_SHIFT) == KEYBOARD_SHIFT) {
          modifier(report) = (uint8_t)KEYBOARD_MODIFIER_LEFTSHIFT;
     }
}

#define NUM_ESCAPED_CHARS 3
const char escaped_chars[NUM_ESCAPED_CHARS] = {'n', 't', 'b'};
const uint8_t PROGMEM const escaped_values[NUM_ESCAPED_CHARS] PROGMEM = { 0x28, 0x2B, 0x2A};



uint8_t map_keys(uint8_t* report, uint8_t *input, uint8_t  extensions)
{
     uint8_t current_key = *input;
     uint8_t charsRead = 1;

     if (current_key < 128) {
          if ((input[0] == '%') && (input[1] == '{') && extensions) {
               return map_extension_block(report, input);
          }
          if (current_key == '\\') {
              for (uint16_t i=0; i<NUM_ESCAPED_CHARS; i++) {
                  if (input[1] == escaped_chars[i]) {
                      add_keycode(report, pgm_read_byte(&escaped_values[i]));
                      charsRead++;
                      break;
                  }
              }
          } else if (current_key > 31) {
               if (current_key < 65) {
                    map_ascii_key(report, pgm_read_byte(&ascii_from32[current_key-32]));
               } else if (current_key < 91) {// A..Z
                    add_keycode(report, current_key-(65-4));
                    modifier(report) |= KEYBOARD_MODIFIER_LEFTSHIFT;
               } else if (current_key < 97) {
                    map_ascii_key(report, pgm_read_byte(&ascii_from91[current_key-91]));
               } else if (current_key<123) {// a..z
                    add_keycode(report,current_key-(97-4));
               } else {
                    map_ascii_key(report, pgm_read_byte(&ascii_from123[current_key-123]));
               }
          } else {
              if (current_key == '\t') {
                  add_keycode(report, 0x2B); // tab
// the following arent' possible from within phatIO runfile
//         } else if (current_key == '\n' || current_key == 13) {// new line or carriage return = enter
//                    add_keycode(report, 0x28); // ENTER
//               } else if (current_key == 27) {
//                    add_keycode(report, 0x29); // ESCAPE
//               } else if (current_key == 8) {
//                    add_keycode(report, 0x2A); // backspace
               }
          }
     }
     return charsRead;
}

uint8_t map_extension_block(uint8_t* report, uint8_t *input)
{
     int consumed = 2;


     while ((input[consumed] != 0) && (input[consumed] != '}')) {
          uint8_t found = 0;

          // one of the modifiers
          for (int modifier=0; !found && modifier<NUM_MODIFIERS; modifier++) {
               char *m = (char *)pgm_read_word(&(modifiers[modifier]));
               uint8_t l = strlen_P(m);
               if (strncmp_P(input+consumed, m, l) == 0) {
                    modifier(report) |= (1<<modifier);
                    found = 1;
                    consumed += l;
                    break;
               }
          }

          // hex
          if (!found && input[consumed] == '0' && input[consumed+1] == 'x') {
               uint16_t hex = 0;
               found = parse_number(&input[consumed], &hex);
               if (found) {
                    consumed += 4;
                    add_keycode(report, hex);
               }
          }

          // if we haven't found anything and there's only a single character test if an ascii
          if ((!found && input[consumed+1] == '+') || input[consumed+1] == '}') {
               found = map_keys(report, input+consumed, 0);
               consumed += found;
          }

          // if we didn't find anything we can use we'll reprocess the whole block as plain text
          if (!found) {
               return map_keys(report, input, 0);
          }


          // consume +
          if (input[consumed] == '+') {
               consumed++;
               continue;
          }
     }
     return consumed+1;


// test handcoding for space
//          // check for modifiers
//          uint8_t start = 0;
//          if (((input[consumed] == 'L') && (input[consumed+1] == 'e') && (input[consumed+2] == 'f') && (input[consumed+3] == 't') && (start=4)) ||
//               ((input[consumed] == 'R') && (input[consumed+1] == 'i') && (input[consumed+2] == 'g') && (input[consumed+3] == 'h') && (input[consumed+4] == 't') && (start=5))) {
//               uint8_t end = 0;
//               uint8_t mod = 0;
//               if ((input[start] == 'C') && (input[start+1] == 'o') && (input[start+2] == 'n') && (input[start+3] == 't') && (input[start+4] == 'r') && (input[start+5] == 'o') && (input[start+6] == 'l') && (end=start+6)) {
//                    mod = 0;
//               } else if ((input[start] == 'S') && (input[start+1] == 'h') && (input[start+2] == 'i') && (input[start+3] == 'f') && (input[start+4] == 't') && (end=start+4)) {
//                    mod = 1;
//               } else if ((input[start] == 'A') && (input[start+1] == 'l') && (input[start+2] == 't') && (end=start+2)) {
//                    mod = 2;
//               } else if ((input[start] == 'G') && (input[start+1] == 'U') && (input[start+2] == 'I') && (end=start+4)) {
//                    mod = 3;
//               }
//               if (end > 0) {
//                    modifier(report) |= (1<<(start==5 ? 4 : 0 + mod));
//                    consumed += end+1;
//                    continue;
//               }

}


