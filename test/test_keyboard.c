#include "test.h"
#include "keyboard.h"


static uint8_t report[8] = {0,0,0,0,0,0,0,0};
#define clear(report) { for (int i=0; i<8; i++) report[i] = 0;}

void test_map_keys()
{
	clear(report);
	assert_equals(map_keys(report, "Aasdf", 1), 1);
	assert_equals(report[0], KEYBOARD_MODIFIER_LEFTSHIFT);
	assert_equals(report[2], 0x04);

	clear(report);
	assert_equals(map_keys(report, "zasdf", 1), 1);
	assert_equals(report[0], 0);
	assert_equals(report[2], 0x1D);

	clear(report);
	assert_equals(map_keys(report, "^asdf", 1), 1);
	assert_equals(report[0], KEYBOARD_MODIFIER_LEFTSHIFT);
	assert_equals(report[2], 0x23);

	clear(report);
	assert_equals(map_keys(report, "%{bad+B+2}", 1), 1);
	assert_equals(report[0], KEYBOARD_MODIFIER_LEFTSHIFT);
	assert_equals(report[2], 0x22);

	clear(report);
	assert_equals(map_keys(report, "%{LeftGUI+ }rocketnumbernine.com", 1), 12);
	assert_equals(report[0], KEYBOARD_MODIFIER_LEFTGUI);
	assert_equals(report[2], 0x2C);

}

void test_map_extension_block()
{
	// simple modifiers
	clear(report);
	assert_equals(map_extension_block(report, "%{LeftControl}"), 14);
	assert_equals(report[0], KEYBOARD_MODIFIER_LEFTCTRL);

	clear(report);
	assert_equals(map_extension_block(report, "%{RightGUI}"), 11);
	assert_equals(report[0], KEYBOARD_MODIFIER_RIGHTGUI);


	// modifier + key
	clear(report);
	assert_equals(map_extension_block(report, "%{LeftGUI+a}"), 12);
	assert_equals(report[0], KEYBOARD_MODIFIER_LEFTGUI);
	assert_equals(report[2], 0x04);

	// modifier + implicit shift (B = b + leftshift)
	clear(report);
	assert_equals(map_extension_block(report, "%{LeftGUI+B}"), 12);
	assert_equals(report[0], KEYBOARD_MODIFIER_LEFTGUI | KEYBOARD_MODIFIER_LEFTSHIFT);
	assert_equals(report[2], 0x05);

	// multiple
	clear(report);
	assert_equals(map_extension_block(report, "%{LeftShift+LeftControl+LeftAlt+LeftGUI+B}"), 42);
	assert_equals(report[0], KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_LEFTALT | KEYBOARD_MODIFIER_LEFTGUI);
	assert_equals(report[2], 0x05);

	// multiple characters
	clear(report);
	assert_equals(map_extension_block(report, "%{.+LeftGUI+B+2}"), 16);
	assert_equals(report[0], KEYBOARD_MODIFIER_LEFTGUI | KEYBOARD_MODIFIER_LEFTSHIFT);
	assert_equals(report[2], 0x37);
	assert_equals(report[3], 0x05);
	assert_equals(report[4], 0x1F);


	// non such token - the % will be consumed
	clear(report);
	assert_equals(map_extension_block(report, "%{bad+B+2}"), 1);
	assert_equals(report[0], KEYBOARD_MODIFIER_LEFTSHIFT);
	assert_equals(report[2], 0x22);

	// hex
	clear(report);
	assert_equals(map_extension_block(report, "%{LeftShift+0x31}"), 17);
	assert_equals(report[0], KEYBOARD_MODIFIER_LEFTSHIFT);
	assert_equals(report[2], 0x31);
}

void test_keyboard(void)
{
	test_map_keys();
	test_map_extension_block();
}
