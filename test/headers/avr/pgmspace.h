#include <stdint.h>
#include <string.h>
#include <avr/stdlib.h>

#define PROGMEM 
#define PSTR(x) x
#define sscanf_P(x,y,z) sscanf(x,y,z)
#define strlen_P(x) strlen(x)
#define strncmp_P(x,y,z) strncmp(x,y,z)
#define strncpy_P(x,y,z) strncpy(x,y,z)
#define sprintf_P(x,y,z) sprintf(x,y,z)
#define pgm_read_byte(x) *(x)
#define pgm_read_word(x) *(x)

#define SPM_PAGESIZE 127
