#include "test.h"
#include "fat_impl.h"


void test_FATBLOCK()
{

	assert_equals(0, FATBLOCK(0));
	assert_equals(0, FATBLOCK(1));
	assert_equals(0, FATBLOCK(255));
	assert_equals(1, FATBLOCK(256));

	assert_equals(9872/(512/2), FATBLOCK(9872));
}

void test_FATOFFSET()
{
	assert_equals(0, FATOFFSET(0));
	assert_equals(2, FATOFFSET(1));
    assert_equals(4, FATOFFSET(2));
	assert_equals(510, FATOFFSET(255));
    assert_equals(0, FATOFFSET(256));
    assert_equals(2, FATOFFSET(257));

	assert_equals((8382 % (512/2)) * 2, FATOFFSET(8382));
}

void test_fat_sfn()
{
    assert_string_equals("FSEVEN~1   ", fat_sfn(".fseventsd"));
	assert_string_equals("PORTD      ", fat_sfn("PORTD"));
	assert_string_equals("TEST    ME ", fat_sfn("test.me"));
	assert_string_equals("TEST    MEA", fat_sfn("test.measdf"));
	assert_string_equals("TEST       ", fat_sfn("test."));
	assert_string_equals("TEST       ", fat_sfn("test...."));
	assert_string_equals("TEST    XAF", fat_sfn("test...xafc."));
	assert_string_equals("TEST    SAD", fat_sfn("test.sad"));
	assert_string_equals("TESTABCDSAD", fat_sfn("testaBcD.sad"));
	assert_string_equals("TESTAB~1SAD", fat_sfn("testaBcDe.sad"));
	assert_string_equals("TEST_3~1SAD", fat_sfn("test .,3$ $x.sad"));
	assert_string_equals("$%'-_@~1XXX", fat_sfn("$%'-_@~!().xxx"));
	assert_string_equals("$%'-_@~1   ", fat_sfn("$%'-_@~!()"));
	assert_string_equals("THISIS~1ABC", fat_sfn("this.is.a long file.abcdef"));
}

void test_fat_impl(void)
{
	test_FATBLOCK();
	test_FATOFFSET();
	test_fat_sfn();
}
