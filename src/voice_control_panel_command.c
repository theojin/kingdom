#include <stdio.h>
#include <Ecore.h>

#include "voice_control_panel_command.h"
#include "voice_control_panel_main.h"
#include "voice_control_panel_vc.h"
#include "resource.h"

extern struct _current_information current_information;

char* g_command_1st[NUM_COMMAND_1ST] = {
	"IDS_FRONTOPEN", "IDS_FRONTCLOSE", "IDS_BACKOPEN", "IDS_BACKCLOSE",
	"IDS_A_ON", "IDS_A_OFF", "IDS_B_ON", "IDS_B_OFF",
 
	"IDS_RESERVE_FRONTOPEN", "IDS_RESERVE_FRONTCLOSE", "IDS_RESERVE_BACKOPEN", "IDS_RESERVE_BACKCLOSE", 
	"IDS_RESERVE_A_ON", "IDS_RESERVE_A_OFF", "IDS_RESERVE_B_ON", "IDS_RESERVE_B_OFF",

	"IDS_CANCEL_FRONTOPEN", "IDS_CANCEL_FRONTCLOSE", "IDS_CANCEL_BACKOPEN", "IDS_CANCEL_BACKCLOSE", 
	"IDS_CANCEL_A_ON", "IDS_CANCEL_A_OFF", "IDS_CANCEL_B_ON", "IDS_CANCEL_B_OFF",
};
char *g_hint_1st[NUM_COMMAND_1ST] = {
	"IDS_FRONTOPEN", "IDS_FRONTCLOSE", "IDS_BACKOPEN", "IDS_BACKCLOSE",
	"IDS_A_ON", "IDS_A_OFF", "IDS_B_ON", "IDS_B_OFF",
 
	"IDS_RESERVE_FRONTOPEN", "IDS_RESERVE_FRONTCLOSE", "IDS_RESERVE_BACKOPEN", "IDS_RESERVE_BACKCLOSE", 
	"IDS_RESERVE_A_ON", "IDS_RESERVE_A_OFF", "IDS_RESERVE_B_ON", "IDS_RESERVE_B_OFF",

	"IDS_CANCEL_FRONTOPEN", "IDS_CANCEL_FRONTCLOSE", "IDS_CANCEL_BACKOPEN", "IDS_CANCEL_BACKCLOSE", 
	"IDS_CANCEL_A_ON", "IDS_CANCEL_A_OFF", "IDS_CANCEL_B_ON", "IDS_CANCEL_B_OFF",
};

char* g_command_2nd[NUM_COMMAND_1ST][NUM_COMMAND_2ND] = {
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};

char* g_hint_2nd[NUM_COMMAND_1ST][NUM_COMMAND_2ND] = {
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL}
};

int vcp_cmd_set(int type, int cmd)
{
	switch (type) {
		case FRONT_DOOR:
			if (current_information.front_door == cmd) {
				if (cmd) vc_panel_vc_play_text("정문은 이미 열려있습니다.");
				else vc_panel_vc_play_text("정문은 이미 잠겨있습니다.");
				break;
			}
			current_information.front_door = cmd;

			if (resource_write_led(GPIO_FRONT, cmd)) {
				LOGE("Cannot set FRONT as [%d]", cmd);
			}

			if (current_information.timer[0]) {
				ecore_timer_del(current_information.timer[0]);
				current_information.timer[0] = NULL;
			}

			if (cmd == 1) {
				vc_panel_vc_play_text("정문을 열겠습니다.");
			} else {
				vc_panel_vc_play_text("정문을 잠그겠습니다.");
			}
			break;
		case BACK_DOOR:
			if (current_information.back_door == cmd) {
				if (cmd) vc_panel_vc_play_text("남문은 이미 열려있습니다.");
				else vc_panel_vc_play_text("남문은 이미 잠겨있습니다.");
				break;
			}
			current_information.back_door = cmd;

			if (resource_write_led(GPIO_BACK, cmd)) {
				LOGE("Cannot set BACK as [%d]", cmd);
			}

			if (current_information.timer[1]) {
				ecore_timer_del(current_information.timer[1]);
				current_information.timer[1] = NULL;
			}

			if (cmd == 1) {
				vc_panel_vc_play_text("남문을 열겠습니다.");
			} else {
				vc_panel_vc_play_text("남문을 잠그겠습니다.");
			}
			break;
		case SWITCH_A:
			if (current_information.switch_a == cmd) {
				if (cmd) vc_panel_vc_play_text("일번기기는 이미 동작하고 있습니다.");
				else vc_panel_vc_play_text("일번기기는 이미 중지된 상태입니다.");
				break;
			}
			current_information.switch_a = cmd;

			if (resource_write_relay(GPIO_SWITCH_A, cmd)) {
				LOGE("Cannot set SWITCH A as [%d]", cmd);
			}

			if (current_information.timer[2]) {
				ecore_timer_del(current_information.timer[2]);
				current_information.timer[2] = NULL;
			}

			if (cmd == 1) {
				vc_panel_vc_play_text("일번 기기를 시작하겠습니다.");
			} else {
				vc_panel_vc_play_text("일번 기기를 중지하겠습니다.");
			}
			break;
		case SWITCH_B:
			if (current_information.switch_b == cmd) {
				if (cmd) vc_panel_vc_play_text("이번기기는 이미 동작하고 있습니다.");
				else vc_panel_vc_play_text("이번기기는 이미 중지된 상태입니다.");
				break;
			}
			current_information.switch_b = cmd;

			if (resource_write_relay(GPIO_SWITCH_B, cmd)) {
				LOGE("Cannot set SWITCH B as [%d]", cmd);
			}

			if (current_information.timer[3]) {
				ecore_timer_del(current_information.timer[3]);
				current_information.timer[3] = NULL;
			}

			if (cmd == 1) {
				vc_panel_vc_play_text("이번 기기를 시작하겠습니다.");
			} else {
				vc_panel_vc_play_text("이번 기기를 중지하겠습니다.");
			}
			break;
		default:
			LOGE("No case here");
			break;
	}
	return 0;
}

Eina_Bool _front_timer_cb(void *data)
{
	int cmd = (int) data;
	LOGD("정문 알람 수행[%d]", cmd);
	vcp_cmd_set(FRONT_DOOR, cmd);
	current_information.timer[0] = NULL;
	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool _back_timer_cb(void *data)
{
	int cmd = (int) data;
	LOGD("후문 알람 수행[%d]", cmd);
	vcp_cmd_set(BACK_DOOR, cmd);
	current_information.timer[1] = NULL;
	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool _switch_a_timer_cb(void *data)
{
	int cmd = (int) data;
	LOGD("스위치 A 알람 수행[%d]", cmd);
	vcp_cmd_set(SWITCH_A, cmd);
	current_information.timer[2] = NULL;
	return ECORE_CALLBACK_CANCEL;
}

Eina_Bool _switch_b_timer_cb(void *data)
{
	int cmd = (int) data;
	LOGD("스위치 B 알람 수행[%d]", cmd);
	vcp_cmd_set(SWITCH_B, cmd);
	current_information.timer[3] = NULL;
	return ECORE_CALLBACK_CANCEL;
}

int vcp_cmd_reserve(int type, int cmd, double seconds)
{
	int is_reserved = 0;
	char tts_str[1024] = {0, };

	switch (type) {
		case FRONT_DOOR:
			if (current_information.timer[0]) {
				ecore_timer_del(current_information.timer[0]);
				is_reserved = 1;
			}
			current_information.timer[0] = ecore_timer_add(seconds, _front_timer_cb, (void *) cmd);
			if (!current_information.timer[0]) LOGE("Timer error");

			if (cmd == 1) {
				if (is_reserved) vc_panel_vc_play_text("기존 알람을 해제하고 정문 열림 알람을 설정하였습니다.");
				else vc_panel_vc_play_text("정문 열림 알람을 설정하였습니다.");
			} else {
				if (is_reserved) vc_panel_vc_play_text("기존 알람을 해제하고 정문 잠금 알람을 설정하였습니다.");
				else vc_panel_vc_play_text("정문 잠금 알람이 설정되었습니다.");
			}
			break;
		case BACK_DOOR:
			if (current_information.timer[1]) {
				ecore_timer_del(current_information.timer[1]);
				is_reserved = 1;
			}
			current_information.timer[1] = ecore_timer_add(seconds, _back_timer_cb, (void *) cmd);
			if (!current_information.timer[1]) LOGE("Timer error");

			if (cmd == 1) {
				if (is_reserved) vc_panel_vc_play_text("기존 알람을 해제하고 남문 열림 알람을 설정하였습니다.");
				else vc_panel_vc_play_text("남문 열림 알람을 설정하였습니다.");
			} else {
				if (is_reserved) vc_panel_vc_play_text("기존 알람을 해제하고 남문 잠금 알람을 설정하였습니다.");
				else vc_panel_vc_play_text("남문 잠금 알람을 설정하였습니다.");
			}
			break;
		case SWITCH_A:
			if (current_information.timer[2]) {
				ecore_timer_del(current_information.timer[2]);
				is_reserved = 1;
			}
			current_information.timer[2] = ecore_timer_add(seconds, _switch_a_timer_cb, (void *) cmd);
			if (!current_information.timer[2]) LOGE("Timer error");

			if (cmd == 1) {
				if (is_reserved) vc_panel_vc_play_text("기존 알람을 해제하고 일번 기기 시작 알람을 설정하였습니다.");
				else vc_panel_vc_play_text("일번 기기 시작 알람을 설정하였습니다.");
			} else {
				if (is_reserved) vc_panel_vc_play_text("기존 알람을 해제하고 일번 기기 중지 알람을 설정하였습니다.");
				else vc_panel_vc_play_text("일번 기기 중지 알람을 설정하였습니다.");
			}
			break;
		case SWITCH_B:
			if (current_information.timer[3]) {
				ecore_timer_del(current_information.timer[3]);
				is_reserved = 1;
			}
			current_information.timer[3] = ecore_timer_add(seconds, _switch_b_timer_cb, (void *) cmd);
			if (!current_information.timer[3]) LOGE("Timer error");

			if (cmd == 1) {
				if (is_reserved) vc_panel_vc_play_text("기존 알람을 해제하고 이번 기기 시작 알람을 설정하였습니다.");
				else vc_panel_vc_play_text("이번 기기 시작 알람을 설정하였습니다.");
			} else {
				if (is_reserved) vc_panel_vc_play_text("기존 알람을 해제하고 이번 기기 중지 알람을 설정하였습니다.");
				else vc_panel_vc_play_text("이번 기기 중지 알람을 설정하였습니다.");
			}
			break;
		default:
			LOGE("No case here");
			break;
	}
	return 0;
}

int vcp_cmd_cancel(int type)
{
	switch (type) {
		case FRONT_DOOR:
			if (current_information.timer[0]) {
				ecore_timer_del(current_information.timer[0]);
				current_information.timer[0] = NULL;
			} else {
				vc_panel_vc_play_text("설정된 알람이 없습니다.");
				break;
			}

			vc_panel_vc_play_text("정문 알람이 해제되었습니다.");
			break;
		case BACK_DOOR:
			if (current_information.timer[1]) {
				ecore_timer_del(current_information.timer[1]);
				current_information.timer[1] = NULL;
			} else {
				vc_panel_vc_play_text("설정된 알람이 없습니다.");
				break;
			}

			vc_panel_vc_play_text("남문 알람이 해제되었습니다.");
			break;
		case SWITCH_A:
			if (current_information.timer[2]) {
				ecore_timer_del(current_information.timer[2]);
				current_information.timer[2] = NULL;
			} else {
				vc_panel_vc_play_text("설정된 알람이 없습니다.");
				break;
			}

			vc_panel_vc_play_text("일번 기기 알람이 해제되었습니다.");
			break;
		case SWITCH_B:
			if (current_information.timer[3]) {
				ecore_timer_del(current_information.timer[3]);
				current_information.timer[3] = NULL;
			} else {
				vc_panel_vc_play_text("설정된 알람이 없습니다.");
				break;
			}

			vc_panel_vc_play_text("이번 기기 알람이 해제되었습니다.");
			break;
		default:
			LOGE("No case here");
			break;
	}
	return 0;
}

/*
vi:ts=4:ai:nowrap:expandtab
*/
