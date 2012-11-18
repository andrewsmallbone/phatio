
#include <stdio.h>
#include "test.h"

void test_lio(void);
void test_keyboard(void);
void test_util(void);
void test_fat_impl(void);
void test_intel_hex(void);
void test_sdihex_bootloader(void);



int main(void)
{
	printf("keyboard: "); test_keyboard();	printf("\n");
	printf("util: "); test_util();	printf("\n");
	printf("fat_impl: "); test_fat_impl();	printf("\n");
    printf("lio: "); test_lio();  printf("\n");
}
