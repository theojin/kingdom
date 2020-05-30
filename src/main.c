#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "vc.h"
#include "view.h"
#include "log.h"

static bool app_create(void *data)
{
	_D("");


	appdata *ad = (appdata *)data;

	/* Create View */
	if (0 != view_create(ad)) {
		_E("[ERROR] Fail to create view");
		return -1;
	}

	/* Initialize Voice Control */
	if (0 != vc_init(ad)) {
		_E("[ERROR] Fail to vc init");
		return -1;
	}

	return true;
}

static void app_control(app_control_h app_control, void *data)
{
	_D("");

	appdata *ad = (appdata *)data;
	_D("state - %d", ad->app_state);

	if (0 != ad->app_state) {
		ui_app_exit();
		return;
	}

	if (ad->win) {
		elm_win_activate(ad->win);
	}
	ad->app_state = PANEL_STATE_INIT;
}

static void app_pause(void *data)
{
	_D("");

	appdata *ad = (appdata *)data;
	ad->app_state = PANEL_STATE_PAUSE;

	vc_deactivate(data, 0.5);
}

static void app_resume(void *data)
{
	_D("");

	appdata *ad = (appdata *)data;
	if (ad->app_state == PANEL_STATE_PAUSE) {
		if (ad->win) {
			elm_win_activate(ad->win);
		}
	}

	ad->app_state = PANEL_STATE_SERVICE;
}

static void app_terminate(void *data)
{
	_D("");

	appdata *ad = (appdata *)data;
	ad->app_state = PANEL_STATE_TERMINATE;

	vc_cancel(ad);

	view_destroy(ad);

	if (0 != vc_deinit(data)) {
		_E("[ERROR] Fail to vc deinit");
	}
}

static void ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	_D("");
}

static void ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	_D("");
}

static void ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	_D("");
}

static void ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	_D("");
}

static void ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	_D("");
}

int main(int argc, char *argv[])
{
	appdata ad = {0,};
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	event_callback.create = app_create;
	event_callback.terminate = app_terminate;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		_W("ui_app_main failed, Err=%d\n", ret);
	}

	return ret;
}
