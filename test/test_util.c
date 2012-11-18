#include "test.h"
#include "util.h"
#include "char_source.h"
#include "string.h"

void test_string_char_source()
{
    const char *test = "this is a test";
    char temp[64];
    char_source *src = string_source(test);
    for (int i=0; (temp[i] = src->read()) != 0; i++);
    assert_string_equals(temp, test);
}


void test_find_index()
{
    const char *test = "setdir\0setpin\0get\0getpin\0<\0+\0set\0\0";
    assert_equals(0, find_index("setdir", test));
    assert_equals(1, find_index("setpin", test));
    assert_equals(2, find_index("get", test));
    assert_equals(3, find_index("getpin", test));
    assert_equals(4, find_index("<", test));
    assert_equals(5, find_index("+", test));
    assert_equals(6, find_index("set", test));
    assert_equals(-1, find_index("xxx", test));

}

void test_parse(char *str, uint16_t value)
{
    uint16_t parsed_value = 0;
    assert(parse_number(str, &parsed_value));
    assert_equals(value, parsed_value);

}

void test_parse_number()
{
//    uint16_t value;
//    assert(!parse_number("asdf", &value));
//    assert(!parse_number("", &value));
//    assert(!parse_number("  ", &value));
//
//    test_parse("212", 212);
//    test_parse("  22", 22);
//    test_parse("0xFFDD", 0xFFDD);
//    test_parse("0xFFDDEEFF", 0xFFDDEEFF);
}

void test_util(void)
{
    test_string_char_source();
    test_find_index();
	test_parse_number();
}
