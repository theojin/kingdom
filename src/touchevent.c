#include "touchevent.h"

#include <dlog.h>
#include <efl_util.h>
#include <system_info.h>

#include "action.h"
#include "command.h"
#include "main.h"
#include "view.h"
#include "log.h"

#define SWIPE_DELAY 10000000
#define MAX_PPS 1000

static int g_move_speed = 0;
static int g_screen_w = 0;
static int g_screen_h = 0;

int touch_set_speed(const int pps)
{
	if (MAX_PPS < pps) {
		g_move_speed = MAX_PPS;
	} else {
		g_move_speed = pps;
	}

	return 0;
}

void touch_back(void* data)
{
	appdata* ad = data;
	efl_util_inputgen_h back_key = ad->back_key;

	efl_util_input_generate_key(back_key, "XF86Back", 1);
	efl_util_input_generate_key(back_key, "XF86Back", 0);
}

void touch_swipe(const int direction, void* data)
{
	appdata* ad = data;
	efl_util_inputgen_h touch = ad->touch;
	int i;
	int ret = 0;
	int delay = 0;
	int center_x, center_y;
	int dest;

	_D("=== Touch swipe(%d)", direction);

	system_info_get_platform_int("http://tizen.org/feature/screen.width", &g_screen_w);
	system_info_get_platform_int("http://tizen.org/feature/screen.height", &g_screen_h);

	center_x = g_screen_w / 2;
	center_y = g_screen_h / 2;

	ret = efl_util_input_generate_touch(touch, 0, EFL_UTIL_INPUT_TOUCH_BEGIN, center_x, center_y);
	if (0 != ret) {
		_E("[ERROR] event generation failed");
		return;
	}

	delay = SWIPE_DELAY / g_move_speed;

	// scroll down
	if (TOUCH_SWIPE_UP == direction) {
		dest = g_screen_h * 0.1;
		for (i = center_y; dest < i; i -= 10) {
			ret = efl_util_input_generate_touch(touch, 0, EFL_UTIL_INPUT_TOUCH_UPDATE, center_x, i);
			if (0 != ret) {
				_E("[ERROR] event generation failed");
				return;
			}
			usleep(delay);
		}

		ret = efl_util_input_generate_touch(touch, 0, EFL_UTIL_INPUT_TOUCH_END, center_x, i);
		if (0 != ret) {
			_E("[ERROR] event generation failed");
			return;
		}
	// scroll up
	} else if (TOUCH_SWIPE_DOWN == direction) {
		dest = g_screen_h * 0.9;
		for (i = center_y; dest > i; i += 10) {
			ret = efl_util_input_generate_touch(touch, 0, EFL_UTIL_INPUT_TOUCH_UPDATE, center_x, i);
			if (0 != ret) {
				_E("[ERROR] event generation failed");
				return;
			}
			usleep(delay);
		}

		ret = efl_util_input_generate_touch(touch, 0, EFL_UTIL_INPUT_TOUCH_END, center_x, i);
		if (0 != ret) {
			_E("[ERROR] event generation failed");
			return;
		}
	// swipe left
	} else if (TOUCH_SWIPE_LEFT == direction) {
		dest = g_screen_w * 0.1;
		for (i = center_x; dest < i; i -= 10) {
			ret = efl_util_input_generate_touch(touch, 0, EFL_UTIL_INPUT_TOUCH_UPDATE, i, center_y);
			if (0 != ret) {
				_E("[ERROR] event generation failed");
				return;
			}
			usleep(delay);
		}

		ret = efl_util_input_generate_touch(touch, 0, EFL_UTIL_INPUT_TOUCH_END, i, center_y);
		if (0 != ret) {
			_E("[ERROR] event generation failed");
			return;
		}
	// swipe right
	} else if (TOUCH_SWIPE_RIGHT == direction) {
		dest = g_screen_w * 0.9;
		for (i = center_x; dest > i; i += 10) {
			ret = efl_util_input_generate_touch(touch, 0, EFL_UTIL_INPUT_TOUCH_UPDATE, i, center_y);
			if (0 != ret) {
				_E("[ERROR] event generation failed");
				return;
			}
			usleep(delay);
		}

		ret = efl_util_input_generate_touch(touch, 0, EFL_UTIL_INPUT_TOUCH_END, i, center_y);
		if (0 != ret) {
			_E("[ERROR] event generation failed");
			return;
		}
	}

	_D("==== end swipe");
}
