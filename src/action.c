#include "action.h"

#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include <app.h>
#include <dlog.h>

#include "command.h"
#include "main.h"
#include "touchevent.h"
#include "view.h"
#include "log.h"

/*
 	"정문 열어", "정문 잠궈",
	"후문 열어", "후문 잠궈",
	"스위치 에이 켜", "스위치 에이 꺼",
	"스위치 비 켜", "스위치 비 꺼",
	"오초 후에 정문 열어", "오초 후에 후문 열어",
	"정문 알람 취소", "후문 알람 취소"
 */
bool action(const char* result, void *data)
{
	if (NULL == result)
		return false;

	_D("==== Action - %s ====", result);

	appdata *ad = (appdata *)data;

	if (1 == ad->current_depth) {
		if (!strcasecmp(result, "정문 열어")) {
			// FIXME
			return true;
		} else if (!strcasecmp(result, "정문 잠궈")) {
			// FIXME
			return true;
		} else if (!strcasecmp(result, "후문 열어")) {
			// FIXME
			return true;
		} else if (!strcasecmp(result, "후문 잠궈")) {
			// FIXME
			return true;
		} else if (!strcasecmp(result, "스위치 에이 켜")) {
			// FIXME
			return true;
		} else if (!strcasecmp(result, "스위치 에이 꺼")) {
			// FIXME
			return true;
		} else if (!strcasecmp(result, "스위치 비 켜")) {
			// FIXME
			return true;
		} else if (!strcasecmp(result, "스위치 비 꺼")) {
			// FIXME
			return true;
		} else if (!strcasecmp(result, "오초 후에 정문 열어")) {
			// FIXME
			return true;
		} else if (!strcasecmp(result, "오초 후에 후문 열어")) {
			// FIXME
			return true;
		} else if (!strcasecmp(result, "정문 알람 취소")) {
			// FIXME
			return true;
		} else if (!strcasecmp(result, "후문 알람 취소")) {
			// FIXME
			return true;
		}

		int i;
		for (i = 0; i < NUM_COMMAND_1ST; i++) {
			if (NULL == g_command_1st[i])
				continue;

			if (!strcasecmp(result, _(g_command_1st[i]))) {
				ad->current_path[0] = i;
				ad->current_depth = 2;
			}
		}
	}

	_D("====");
	_D(" ");

	return true;
}
