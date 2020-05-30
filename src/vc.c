#include "vc.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <dlog.h>
#include <Elementary.h>

#include <tts.h>
#include <glib.h>
#include <json-glib/json-glib.h>
#include <efl_util.h>

#include <voice_control_common.h>
#include <voice_control_manager.h>

#include "action.h"
#include "command.h"
#include "main.h"
#include "touchevent.h"
#include "view.h"
#include "log.h"

/* INTERNAL */
#define VC_CMD_FORMAT_FIXED 0

static vc_cmd_list_h g_cmd_list_1st;
static vc_cmd_list_h g_cmd_list_2nd[NUM_COMMAND_1ST];
static Ecore_Timer *g_deactive_timer = NULL;

static tts_h g_tts;
static char *g_tts_lang = NULL;
static bool g_dialog_continuous = false;
static bool g_dialog_process = false;
static int g_num_candidate = 0;

static vc_cmd_list_h g_candidate_cmd_list;

static JsonParser *g_json_parser;

static bool __current_command_cb(vc_cmd_h vc_command, void* user_data)
{
	int type;
	if (0 != vc_cmd_get_type(vc_command, &type)) {
		_E("[ERROR] Fail to get type");
		return false;
	}

	return true;
}

static int __vc_get_current_commands(void *data)
{
	appdata *ad = (appdata *)data;

	_D("==== Get current commands ====");
	if (0 < g_list_length(ad->cmd_list)) {
		GList *iter = NULL;
		iter = g_list_first(ad->cmd_list);

		while (NULL != iter) {
			char *cmd = iter->data;
			if (NULL != cmd) {
				free(cmd);
				cmd = NULL;
			}

			ad->cmd_list = g_list_remove_link(ad->cmd_list, iter);
			iter = g_list_first(ad->cmd_list);
		}
	}

	vc_cmd_list_h vc_cmd_list;
	if (0 != vc_mgr_get_current_commands(&vc_cmd_list)) {
		_E("[ERROR] Fail to get current commands");

		if (0 != vc_cmd_list_destroy(vc_cmd_list, true)) {
			_E("[WARNING] Fail to cmd list destroy");
		}
		return -1;
	}

	if (0 != vc_cmd_list_foreach_commands(vc_cmd_list, __current_command_cb, ad)) {
		_E("[ERROR] Fail to get current commands");

		if (0 != vc_cmd_list_destroy(vc_cmd_list, true)) {
			_E("[WARNING] Fail to cmd list destroy");
		}
		return -1;
	}

	if (0 != vc_cmd_list_destroy(vc_cmd_list, true)) {
		_E("[WARNING] Fail to cmd list destroy");
	}

	return 0;
}

static void __vc_destroy_command_list()
{
	_D("==== Destroy Command List ====");

	if (0 != vc_cmd_list_destroy(g_cmd_list_1st, true)) {
		_E("[WARNING] Fail to destroy list");
	}

	int i;
	for (i = 0; i < NUM_COMMAND_1ST; i++) {
		if (0 != vc_cmd_list_destroy(g_cmd_list_2nd[i], true)) {
			_E("[WARNING] Fail to destroy list");
		}
	}

	_D("====");
	_D(" ");
}

static int __vc_create_command_list()
{
	_D("==== Create Command List ====");

	/* 1st depth */
	if (0 != vc_cmd_list_create(&g_cmd_list_1st)) {
		_E("[ERROR] Fail to cmd list create");
		return -1;
	}

	vc_cmd_h cmd;
	int i;
	for (i = 0; i < NUM_COMMAND_1ST; i++) {
		if (0 != vc_cmd_create(&cmd)) {
			_E("[ERROR] Fail to cmd create");
			return -1;
		}
		if (0 != vc_cmd_set_command(cmd, g_command_1st[i])) {
			_E("[ERROR] Fail to set command");
			vc_cmd_destroy(cmd);
			return -1;
		}
		// FIXME : System?
		if (0 != vc_cmd_set_type(cmd, VC_COMMAND_TYPE_SYSTEM)) {
			_E("[ERROR] Fail to set type");
			vc_cmd_destroy(cmd);
			return -1;
		}
		if (0 != vc_cmd_set_format(cmd, VC_CMD_FORMAT_FIXED)) {
			_E("[ERROR] Fail to set format");
			vc_cmd_destroy(cmd);
			return -1;
		}
		if (0 != vc_cmd_list_add(g_cmd_list_1st, cmd)) {
			_E("[ERROR] Fail to list add");
			vc_cmd_destroy(cmd);
			return -1;
		}
	}

	/* 2nd depth */
	for (i = 0; i < NUM_COMMAND_1ST; i++) {
		if (0 != vc_cmd_list_create(&g_cmd_list_2nd[i])) {
			_E("[ERROR] Fail to list create");
			return -1;
		}

		int j;
		for (j = 0; j < NUM_COMMAND_2ND; j++) {
			if (NULL != g_command_2nd[i][j]) {
				if (0 != vc_cmd_create(&cmd)) {
					_E("[ERROR] Fail to cmd create");
					return -1;
				}
				if (0 != vc_cmd_set_command(cmd, _(g_command_2nd[i][j]))) {
					_E("[ERROR] Fail to set command");
					vc_cmd_destroy(cmd);
					return -1;
				}
				if (0 != vc_cmd_set_type(cmd, VC_COMMAND_TYPE_SYSTEM)) {
					_E("[ERROR] Fail to set type");
					vc_cmd_destroy(cmd);
					return -1;
				}
				if (0 != vc_cmd_set_format(cmd, VC_CMD_FORMAT_FIXED)) {
					_E("[ERROR] Fail to set format");
					vc_cmd_destroy(cmd);
					return -1;
				}
				if (0 != vc_cmd_list_add(g_cmd_list_2nd[i], cmd)) {
					_E("[ERROR] Fail to list add");
					vc_cmd_destroy(cmd);
					return -1;
				}
			}
		}
	}

	return 0;
}

static Eina_Bool __start_cb(void *data)
{
	appdata *ad = (appdata *)data;
	ad->act_state = 0;

	vc_service_state_e state;
	if (0 != vc_mgr_get_service_state(&state)) {
		_E("[ERROR] Fail to get service state");
		return EINA_FALSE;
	}

	if (VC_SERVICE_STATE_READY != state) {
		_D("[WARNING] Wait for service state ready");
		return EINA_TRUE;
	}

	/*
	 * VC_RECOGNITION_MODE_STOP_BY_SILENCE			Default mode
	 * VC_RECOGNITION_MODE_RESTART_AFTER_REJECT		Restart recognition after rejected result
	 * VC_RECOGNITION_MODE_RESTART_CONTINUOUSLY		Continuously restart recognition - not support yet
	 * VC_RECOGNITION_MODE_MANUAL					Start and stop manually without silence
	 */
	if (0 != vc_mgr_set_recognition_mode(VC_RECOGNITION_MODE_STOP_BY_SILENCE)) {
		_E("[ERROR] Fail to set recognition mode");
	}

	if (0 != vc_mgr_set_command_list(g_cmd_list_1st)) {
		_E("[ERROR] Fail to set command list");
	}

	if (0 != __vc_get_current_commands(ad)) {
		_E("[ERROR] Fail to get current commands");
	}

	/* start recording & recognizing */
	if (0 != vc_mgr_start(false)) {
		_E("[ERROR] Fail to start");
	}

	return EINA_FALSE;
}

static void __vc_state_changed_cb(vc_state_e previous, vc_state_e current, void *user_data)
{
	_D("==== State is changed ====");
	_D("Previous(%d) -> Current(%d)", previous, current);

	appdata *ad = (appdata*)user_data;

	/* After vc_mgr_prepare() */
	/* VC_STATE_NONE		'None' state
	 * VC_STATE_INITIALIZED	'Initialized' state
	 * VC_STATE_READY		'Ready' state
	 */
	if (VC_STATE_INITIALIZED == previous && VC_STATE_READY == current) {
		vc_start(ad);
	}
}

static void __vc_error_cb(vc_error_e reason, void *user_data)
{
	_D("==== Error handling ====");
	char *err_msg = NULL;
	char disp_text[256];

	if (0 != vc_mgr_get_error_message(&err_msg)) {
		_E("[ERROR] Fail to get error message");
		view_show_result(user_data, "Unknow error is occurred!!");

		return;
	}

	if (NULL != err_msg) {
		snprintf(disp_text, 256, "Error is occurred!! (%s)", err_msg);
		view_show_result(user_data, disp_text);
		free(err_msg);
		err_msg = NULL;
	}

	_D("====");
	_D(" ");
}

static void __vc_reset(void *data)
{
	appdata *ad = (appdata *)data;

	ad->current_depth = 1;
	ad->current_path[0] = -1;
	ad->current_path[1] = -1;

	if (0 != vc_mgr_set_command_list(g_cmd_list_1st)) {
		_E("[ERROR] Fail to set command list");
	}
}

static Eina_Bool __vc_finalize(void *data)
{
	_D("=== Finalize ===");

	ui_app_exit();
	return EINA_FALSE;
}

static Eina_Bool __vc_deactivate(void *data)
{
	_D("=== Deactivate ===");

	tts_state_e state;

	tts_get_state(g_tts, &state);
	if (TTS_STATE_PLAYING == state) {
		_D("[WARNING] Processing Dialog");
		return EINA_TRUE;
	}

	if (0 != vc_cancel(data)) {
		_E("[ERROR] Fail to deactivate");
		return EINA_FALSE;
	}

	view_show_result(data, _("IDS_RESTART"));

	return EINA_FALSE;
}

static Eina_Bool __vc_restart(void *data)
{
	_D("==== Restart ====");

	appdata *ad = (appdata *)data;
	ad->act_state = 0;

	vc_service_state_e state;
	if (0 != vc_mgr_get_service_state(&state)) {
	    _E("[ERROR] Fail to get service state");
	    return EINA_FALSE;
	}

	if (VC_SERVICE_STATE_READY != state) {
	    _D("[WARNING] Wait for service state ready");
	    return EINA_FALSE;
	}

	/* set current command list */
	if (1 == ad->current_depth) {
		if (0 != vc_mgr_set_command_list(g_cmd_list_1st)) {
			_E("[ERROR] Fail to set command list");
		}

	} else if (2 == ad->current_depth) {
		if (0 != vc_mgr_set_command_list(g_cmd_list_2nd[ad->current_path[0]])) {
			_E("[ERROR] Fail to set command list");
		}
	}

	view_show(ad);

	if (0 != __vc_get_current_commands(ad)) {
		_E("[ERROR] Fail to get current commands");
	}

	/* start recording & recognizing */
	if (0 != vc_mgr_start(false)) {
		_E("[ERROR] Fail to start");
	}

	_D("====");
	_D(" ");

	return EINA_FALSE;
}

static void __vc_service_state_changed_cb(vc_service_state_e previous, vc_service_state_e current, void *user_data)
{
	_D("==== Service state is changed ====");
	_D("Previous(%d) -> Current(%d)", previous, current);

	appdata *ad = (appdata *)user_data;

	/* VC_SERVICE_STATE_NONE		'None' state
	 * VC_SERVICE_STATE_READY		'Ready' state
	 * VC_SERVICE_STATE_RECORDING	'Recording' state
	 * VC_SERVICE_STATE_PROCESSING	'Processing' state
	 **/
	if (VC_SERVICE_STATE_READY == previous && VC_SERVICE_STATE_RECORDING == current) {
		_D("==== Show by recording ====");
		view_show(ad);

	} else if ((VC_SERVICE_STATE_RECORDING == previous || VC_SERVICE_STATE_PROCESSING == previous) && VC_SERVICE_STATE_READY == current) {
		if (PANEL_STATE_SERVICE == ad->app_state) {
			_D("==== Process finish ====");
		} else {
			_D("==== Hide ====");
			__vc_reset(ad);
		}
	} else if (VC_SERVICE_STATE_RECORDING == previous && VC_SERVICE_STATE_PROCESSING == current) {
		_D("==== Processing ====");
		view_show_result(ad, "Processing...");
	}
}

static bool __search_cmd_cb(vc_cmd_h vc_command, void* user_data)
{
	_D("==== Search the selected command cb ====");

	#if 0 // Internal
	vc_cmd_list_h vc_cmd_list;
	intptr_t p_selected_pid = (intptr_t)user_data;
	int selected_pid = (int)p_selected_pid;
	int pid;

	if (0 != vc_cmd_get_pid(vc_command, &pid)) {
		_E("[ERROR] fail to get pid");
		return true;
	}

	if (selected_pid == pid) {
		if (0 != vc_cmd_list_create(&vc_cmd_list)) {
			_E("[ERROR] Fail to create the command list");
			return false;
		}
		if (0 != vc_cmd_list_add(vc_cmd_list, vc_command)) {
			_E("[ERROR] Fail to add the command");

			if (0 != vc_cmd_list_destroy(vc_cmd_list, false)) {
				_E("[ERROR] Fail to destroy the command list");
			}

			return false;
		}

		if (0 != vc_mgr_set_selected_results(vc_cmd_list)) {
			_E("[ERROR] Fail to select the app");

			if (0 != vc_cmd_list_destroy(vc_cmd_list, false)) {
				_E("[ERROR] Fail to destroy the command list");
			}

			return false;
		}

		if (0 != vc_cmd_list_destroy(vc_cmd_list, false)) {
			_E("[ERROR] Fail to destroy the command list");
		}

		return false;
	}
#endif

	return true;
}

static void __item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	_D("==== Command list item selected cb ====");

	appdata *ad = (appdata*)data;
	Elm_Object_Item *it = (Elm_Object_Item*)event_info;
	intptr_t pid = (intptr_t)elm_object_item_data_get(it);

	if (0 != vc_cmd_list_foreach_commands(g_candidate_cmd_list, __search_cmd_cb, (void*)pid)) {
		_E("[ERROR] Fail to search the command list");
	}

	if (0 != vc_cmd_list_destroy(g_candidate_cmd_list, true)) {
		_E("[ERROR] Fail to destroy the command list");
	}

	if (0 != vc_cmd_list_create(&g_candidate_cmd_list)) {
		_E("[ERROR] Fail to create the command list");
	}

	view_hide_help(ad);

	_D("====");
	_D("");
}

static bool __vc_all_result_cb(vc_result_event_e event, vc_cmd_list_h vc_cmd_list, const char* result, const char* msg, void *user_data)
{
	appdata *ad = (appdata*)user_data;
	_D("==== All result cb ====");

	if (VC_RESULT_EVENT_REJECTED == event) {
		_D("Rejected event");
		view_show_result(ad, "Sorry.");
	} else {
		if (NULL != result) {
			_D("Result Text - %s", result);
			view_show_result(ad, result);

			view_hide_help(ad);

			if (NULL != ad->help_genlist) {
				elm_genlist_clear(ad->help_genlist);
			}

			g_num_candidate = 0;

			_D("g_num_candidate : %d", g_num_candidate);

			if (0 != vc_cmd_list_destroy(g_candidate_cmd_list, true)) {
				_E("[ERROR] Fail to destroy the command list");
			}

			if (0 != vc_cmd_list_create(&g_candidate_cmd_list)) {
				_E("[ERROR] Fail to create the command list");
			}
		}
	}

	_D("====");
	_D(" ");

	return true;
}

static void __vc_pre_result_cb(vc_pre_result_event_e event, const char* pre_result, void *user_data)
{
	_D("==== Pre result cb ====");
	_D("Result Text - %s", pre_result);
}

static void __vc_result_cb(vc_result_event_e event, vc_cmd_list_h vc_cmd_list, const char* result, void *user_data)
{
	_D("==== Result cb ====");
	appdata* ad = user_data;

	if (NULL != result) {
		_D("Result Text - %s", result);

		vc_cmd_list_first(vc_cmd_list);
		vc_cmd_h result_cmd;
		vc_cmd_list_get_current(vc_cmd_list, &result_cmd);

		char *displayText = NULL;
		vc_cmd_get_command(result_cmd, &displayText);
		if (displayText) view_show_result(user_data, displayText);
		else _E("No display text");

		if (0 == ad->act_state) {
			bool ret;
			ret = action(result, user_data);
			ad->act_state = 1;

			if (true != ret) {
				vc_finalize(user_data);
			}
		}
	}
}

static void __vc_speech_detected_cb(void *user_data)
{
	_D("==== Speech detected ====");
}

static void __vc_language_changed_cb(const char* previous, const char* current, void* user_data)
{
	_D("Language is changed (%s) to (%s)", previous, current);

	char loc[64] = {'\0', };
	snprintf(loc, 64, "%s.UTF-8", current);

	setlocale(LC_ALL, loc);

	__vc_destroy_command_list();
	if (0 != __vc_create_command_list()) {
		_E("[ERROR] Fail to create command list");
	}

	if (NULL != g_tts_lang) {
		free(g_tts_lang);
	}

	g_tts_lang = strdup(current);
}

/* FIXME : Set the utt_text */
static void __vc_mgr_dialog_request_cb(int pid, const char *disp_text, const char *utt_text, bool continuous, void *user_data)
{
	int utt_id;

	_D("==== Requested dialog processing start ====");

	if (NULL != g_deactive_timer) {
		ecore_timer_del(g_deactive_timer);
		g_deactive_timer = NULL;
	}

	if (true == g_dialog_process) {
		view_show_result(user_data, "Please, try again after moments");
	} else {
		//display the text
		view_show_result(user_data, disp_text);

		g_dialog_continuous = continuous;
		g_dialog_process = true;

		//play tts
		if (0 != tts_add_text(g_tts, utt_text, g_tts_lang, TTS_VOICE_TYPE_AUTO, TTS_SPEED_AUTO, &utt_id)) {
			_E("[WARNING] Text adding is failed");
		}

		if (0 != tts_play(g_tts)) {
			_E("[WARNING] Text playing is failed");
		}
	}
}

static void __tts_utterance_completed_cb(tts_h tts, int utt_id, void* user_data)
{
	_D("==== Utterance (%d) is completed ====", utt_id);

	if (0 != tts_stop(tts)) {
		_E("[ERROR] Fail to stop tts");
	}

	if (true == g_dialog_continuous) {
		vc_activate(user_data);
	} else {
		vc_deactivate(user_data, 0.5);
	}

	g_dialog_process = false;
}

int vc_start(void *data)
{
	_D("=== Start the voice control panel ===");

	appdata* ad = data;
	ad->act_state = 0;

	ecore_thread_main_loop_begin();
	/* set the command list and start recording */
	ecore_idler_add(__start_cb, data);
	ecore_thread_main_loop_end();

	return 0;
}

int vc_cancel(void *data)
{
	vc_service_state_e service_state;

	if (0 != vc_mgr_get_service_state(&service_state)) {
		_E("[ERROR] Fail to get service state");
		return -1;
	}

	if ((VC_SERVICE_STATE_RECORDING == service_state) || (VC_SERVICE_STATE_PROCESSING == service_state)) {
		_D("==== service state (%d)", service_state);

		if (0 != vc_mgr_cancel()) {
			_E("[ERROR] Fail to cancel");
			return -1;
		}
	}

	return 0;
}

int vc_init(void *data)
{
	_D("==== Initialize Voice control ====");
	appdata *ad = (appdata *)data;

	if (0 != vc_mgr_initialize()) {
		_E("[ERROR] Fail to initialize");
		return -1;
	}

	if (0 != vc_mgr_set_state_changed_cb(__vc_state_changed_cb, data)) {
		_E("[ERROR] Fail to set state changed cb");
		return -1;
	}

	if (0 != vc_mgr_set_service_state_changed_cb(__vc_service_state_changed_cb, data)) {
		_E("[ERROR] Fail to set service state changed cb");
		return -1;
	}

	if (0 != vc_mgr_set_all_result_cb(__vc_all_result_cb, data)) {
		_E("[ERROR] Fail to set all result cb");
		return -1;
	}

	if (0 != vc_mgr_set_pre_result_cb(__vc_pre_result_cb, data)) {
		_E("[ERROR] Fail to set pre result cb");
		return -1;
	}

	if (0 != vc_mgr_set_result_cb(__vc_result_cb, data)) {
		_E("[ERROR] Fail to set result cb");
		return -1;
	}

	if (0 != vc_mgr_set_speech_detected_cb(__vc_speech_detected_cb, data)) {
		_E("[ERROR] Fail to set speech detected cb");
		return -1;
	}

	if (0 != vc_mgr_set_current_language_changed_cb(__vc_language_changed_cb, data)) {
		_E("[ERROR] Fail to set language changed cb");
		return -1;
	}

	if (0 != vc_mgr_set_dialog_request_cb(__vc_mgr_dialog_request_cb, data)) {
		_E("[ERROR] Fail to set dialog request cb");
		return -1;
	}

	if (0 != vc_mgr_set_error_cb(__vc_error_cb, data)) {
		_E("[ERROR] Fail to set error cb");
		return -1;
	}

	/* initialized -> ready */
	if (0 != vc_mgr_prepare()) {
		_E("[ERROR] Fail to prepare");
		return -1;
	}

	if (0 != __vc_create_command_list()) {
		_E("[ERROR] Fail to create command list");
		return -1;
	}

	//candidate command list create
	if (0 != vc_cmd_list_create(&g_candidate_cmd_list)) {
		_E("[ERROR] Fail to create the command list");
		return -1;
	}

	evas_object_smart_callback_add(ad->help_genlist, "selected", __item_selected_cb, ad);

	//tts handle create & set
	if (0 != tts_create(&g_tts)) {
		_E("[ERROR] Fail to create tts handle");
		return -1;
	}

	if (0 != tts_set_utterance_completed_cb(g_tts, __tts_utterance_completed_cb, data)) {
		_E("[ERROR] Fail to set utt completed cb");
		tts_destroy(g_tts);

		return -1;
	}

	char *language = NULL;
	int voice_type;

	if (0 != tts_get_default_voice(g_tts, &language, &voice_type) || NULL == language) {
		_E("[ERROR] Fail to get default voice");
		tts_destroy(g_tts);

		return -1;
	}

	g_tts_lang = strdup(language);

	free(language);
	language = NULL;

	if (NULL == g_tts_lang) {
		_E("[ERROR] Fail to memory allocation for voice information");
		tts_destroy(g_tts);

		return -1;
	}

	if (0 != tts_prepare(g_tts)) {
		_E("[ERROR] Fail to tts prepare");
		tts_destroy(g_tts);

		return -1;
	}

#if 0
	g_json_parser = json_parser_new();

	if (NULL == g_json_parser) {
		_E("[ERROR] Fail to memory allocation for json parser");
		tts_destroy(g_tts);

		return -1;
	}
#endif

	// initialize virtual touch device
	ad->touch = efl_util_input_initialize_generator(EFL_UTIL_INPUT_DEVTYPE_TOUCHSCREEN);

	if (NULL == ad->touch) {
		_E("[ERROR] initialize efl util failed");
		tts_destroy(g_tts);
		g_object_unref(g_json_parser);

		return -1;
	}

	ad->back_key = efl_util_input_initialize_generator(EFL_UTIL_INPUT_DEVTYPE_KEYBOARD);

	if (NULL == ad->back_key) {
		_E("[ERROR] initialize efl util failed");
		tts_destroy(g_tts);
		g_object_unref(g_json_parser);
		efl_util_input_deinitialize_generator(ad->touch);

		return -1;
	}

	touch_set_speed(800);

	_D("====");
	_D(" ");

	return 0;
}

int vc_deinit(void *data)
{
	_D("==== De-initialize Voice control ====");

	appdata *ad = (appdata *)data;

	if (0 < g_list_length(ad->cmd_list)) {
		GList *iter = NULL;
		iter = g_list_first(ad->cmd_list);

		while (NULL != iter) {
			char *cmd = iter->data;
			if (NULL != cmd) {
				free(cmd);
				cmd = NULL;
			}

			ad->cmd_list = g_list_remove_link(ad->cmd_list, iter);
			iter = g_list_first(ad->cmd_list);
		}
	}

	__vc_destroy_command_list();

	if (0 != vc_mgr_deinitialize()) {
		_E("[ERROR] Fail to deinitialize");
		return -1;
	}

	if (NULL != g_tts_lang) {
		free(g_tts_lang);
		g_tts_lang = NULL;
	}

	if (0 != tts_destroy(g_tts)) {
		_E("[ERROR] Fail to tts destroy");
		return -1;
	}

	if (0 != vc_cmd_list_destroy(g_candidate_cmd_list, true)) {
		_E("[ERROR] Fail to command list destroy");
		return -1;
	}

	g_object_unref(g_json_parser);

	if (0 != efl_util_input_deinitialize_generator(ad->touch)) {
		_E("[ERROR] Fail to deinitialize");
		return -1;
	}
	if (0 != efl_util_input_deinitialize_generator(ad->back_key)) {
		_E("[ERROR] Fail to deinitialize");
		return -1;
	}

	_D("====");
	_D(" ");

	return 0;
}

int vc_activate(void *data)
{
	if (NULL != g_deactive_timer) {
		ecore_timer_del(g_deactive_timer);
		g_deactive_timer = NULL;
	}

	ecore_idler_add(__vc_restart, data);

	return 0;
}

int vc_deactivate(void *data, double delay)
{
	if (NULL != g_deactive_timer) {
		ecore_timer_del(g_deactive_timer);
		g_deactive_timer = NULL;
	}

	g_deactive_timer = ecore_timer_add(delay, __vc_deactivate, data);

	return 0;
}

int vc_finalize(void *data)
{
	ecore_idler_add(__vc_finalize, data);

	return 0;
}
