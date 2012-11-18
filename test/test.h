
#include <stdio.h>
#include <assert.h>
#define TEST(x) void x()
#define assert_equals(x, y) { assert(x == y); printf(".");}
#define assert_string_equals(ex, gx)\
{const char *e = ex; const char *g = gx;\
    if (g == 0 && e != 0) assert(0);\
    for (int i=0; ; i++) {\
        if (e[i] != g[i]) assert(0);\
        if (e[i] == 0 && g[i] == 0) break;\
    }\
    printf(".");\
}
