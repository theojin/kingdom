#include <dbus/dbus.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>

#include <app.h>
#include <aul.h>
#include <dlog.h>
#include <vconf.h>

#include "voice_control_panel_main.h"
#include "voice_control_panel_action.h"
#include "voice_control_panel_command.h"
#include "voice_control_panel_view.h"
#include "voice_control_panel_touchevent.h"

extern struct _current_information current_information;

#if 0 /* NOT USED - if needed, reactivate*/
static void __vc_panel_action_send_key_event(void *data, Ecore_Thread *thread)
{
	LOGD("==== Send Key Event ====");

	int keynum = (int)data;
	LOGD("Key - %d", keynum);

	int fd;
	fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
	if (0 > fd) {
		LOGE("[ERROR] Fail to open dev");
		return;
	}

	int ret;
	struct uinput_user_dev uidev;
	memset(&uidev, 0, sizeof(uidev));
	snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "vc-keyevent");
	uidev.id.bustype = BUS_USB;
	uidev.id.vendor = 1;
	uidev.id.product = 1;
	uidev.id.version = 1;

	ret = write(fd, &uidev, sizeof(uidev));
	if (sizeof(uidev) != ret) {
		LOGE("[ERROR] Fail to write info");
		close(fd);
		return;
	}

	ret = ioctl(fd, UI_SET_EVBIT, EV_KEY);
	if (0 != ret) {
		LOGE("[ERROR] Fail to ioctl");
		close(fd);
		return;
	}

	ret = ioctl(fd, UI_SET_EVBIT, EV_SYN);
	if (0 != ret) {
		LOGE("[ERROR] Fail to ioctl");
		close(fd);
		return;
	}

	ret = ioctl(fd, UI_SET_KEYBIT, keynum);
	if (0 != ret) {
		LOGE("[ERROR] Fail to register key");
		close(fd);
		return;
	}

	ret = ioctl(fd, UI_DEV_CREATE);
	if (0 != ret) {
		LOGE("[ERROR] Fail to create");
		close(fd);
		return;
	}

	usleep(1000000);

	struct input_event ev;
	memset(&ev, 0, sizeof(ev));
	ev.type = EV_KEY;
	ev.code = keynum;
	ev.value = 1;

	ret = write(fd, &ev, sizeof(ev));
	if (0 > ret) {
		LOGE("[ERROR] Fail to send key event");
		close(fd);
		return;
	}

	memset(&ev, 0, sizeof(ev));
	ev.type = EV_SYN;
	ev.code = 0;
	ev.value = 0;

	ret = write(fd, &ev, sizeof(ev));
	if (0 > ret) {
		LOGE("[ERROR] Fail to send sync event");
		close(fd);
		return;
	}

	usleep(1000000);

	ret = ioctl(fd, UI_DEV_DESTROY);
	if (0 != ret) {
		LOGE("[ERROR] Fail to destroy");
		close(fd);
		return;
	}

	LOGD("====");
	LOGD(" ");

	close(fd);
	return;
}

static void __vc_panel_action_launch_app(const char* app_id)
{
	LOGD("==== Launch app(%s) ====", app_id);
	app_control_h app_control;
	app_control_create(&app_control);
	app_control_set_app_id(app_control, app_id);
	app_control_send_launch_request(app_control, NULL, NULL);
	app_control_destroy(app_control);
	LOGD("====");
	LOGD("");
}
#endif

void __launch_poweroff_popup()
{
	LOGD("--");
	DBusError err;
	dbus_error_init(&err);

	DBusConnection *conn = NULL;
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (dbus_error_is_set(&err)) {
		dbus_error_free(&err);
	}

	DBusMessage* msg;
	msg = dbus_message_new_method_call(
		"org.tizen.system.popup",
		"/Org/Tizen/System/Popup/Powerkey",
		"org.tizen.system.popup.Powerkey",
		"PopupLaunch");

	const char *tmp1 = "_SYSPOPUP_CONTENT_";
	const char *tmp2 = "powerkey";
	dbus_message_append_args(msg,
		DBUS_TYPE_STRING, &tmp1,
		DBUS_TYPE_STRING, &tmp2,
		DBUS_TYPE_INVALID);

	dbus_connection_send(conn, msg, NULL);
	dbus_message_unref(msg);

	LOGD("++");

	return;
}

bool vc_panel_action(const char* result, void *data)
{
	if (NULL == result)
		return false;

	LOGD("==== Action - %s ====", result);

	appdata *ad = (appdata *)data;

	if (1 == ad->current_depth) {
		do {
			if ((0 == strstr(result, "정") - result)
			|| (0 == strstr(result, "창") - result)) {
				LOGD("Enter : 정문");

				if (strstr(result, "뒤")) { // 예약
					LOGD("Enter : 정문 -> 예약");

					if (!strstr(result, "초")) {
						vc_panel_vc_play_text("단위를 확인할 수가 없습니다.");
						break;
					}

					double sec = 0.0f;
					if (strstr(result, "오")) {
						LOGD("Enter : 정문 -> 예약 -> 오초");
						sec = 5.0f;
					} else if (strstr(result, "십") || strstr(result, "시") || strstr(result, "식") || strstr(result, "수")) {
						LOGD("Enter : 정문 -> 예약 -> 십초");
						sec = 15.0f;
					} else {
						vc_panel_vc_play_text("이해할 수 없는 숫자입니다.");
						break;
					}

					/* 최종 판단 코드 */
					if (strstr(result, "열")) {
						LOGD("Enter : 정문 -> 예약 -> 열어");

						vcp_cmd_reserve(FRONT_DOOR, 1, sec); // 잠금 -> 열림
					} else if (strstr(result, "잠")) {
						LOGD("Enter : 정문 -> 예약 -> 잠궈");
						vcp_cmd_reserve(FRONT_DOOR, 0, sec); // 열림 -> 잠금
					} else {
						vc_panel_vc_play_text("명령을 이해하지 못했습니다.");
					}
					return true;
				} else if (strstr(result, "취")) { // 예약해제
					LOGD("Enter : 정문 -> 예약해제");

					/* 최종 판단 코드 */
					LOGD("SUCCESS : %s", _("IDS_CANCEL_FRONTOPEN"));
					vcp_cmd_cancel(FRONT_DOOR);
					return true;
				} else { // 일반
					LOGD("Enter : 정문 -> 일반");
					LOGD("String length : %s(%d)", result, strlen(result));
					if (strlen(result) >= 15) {
						vc_panel_vc_play_text("명령을 이해하지 못했습니다.");
						return true;
					}

					/* 최종 판단 코드 */
					if (strstr(result, "열")) {
						LOGD("Enter : 정문 -> 열어");
						vcp_cmd_set(FRONT_DOOR, 1); // 잠금 -> 열림
					} else if (strstr(result, "잠")) {
						LOGD("Enter : 정문 -> 잠궈");
						vcp_cmd_set(FRONT_DOOR, 0); // 열림 -> 잠금
					} else {
						vc_panel_vc_play_text("명령을 이해하지 못했습니다.");
					}
					return true;
				}
			} else if ((0 == strstr(result, "남") - result)
			|| (0 == strstr(result, "낭") - result)
			|| (0 == strstr(result, "날") - result)
			|| (0 == strstr(result, "멜론") - result)) {
				LOGD("Enter : 남문");

				if (strstr(result, "뒤")) { // 예약
					LOGD("Enter : 남문 -> 예약");

					if (!strstr(result, "초")) {
						vc_panel_vc_play_text("단위를 확인할 수가 없습니다.");
						break;
					}

					double sec = 0.0f;
					if (strstr(result, "오")) {
						LOGD("Enter : 남문 -> 예약 -> 오초");
						sec = 5.0f;
					} else if (strstr(result, "십") || strstr(result, "시") || strstr(result, "식") || strstr(result, "수")) {
						LOGD("Enter : 남문 -> 예약 -> 십초");
						sec = 15.0f;
					} else {
						vc_panel_vc_play_text("이해할 수 없는 숫자입니다.");
						break;
					}

					/* 최종 판단 코드 */
					if (strstr(result, "열")) {
						LOGD("Enter : 남문 -> 예약 -> 열어");
						vcp_cmd_reserve(BACK_DOOR, 1, sec); // 잠금 -> 열림
					} else if (strstr(result, "잠")) {
						LOGD("Enter : 남문 -> 예약 -> 잠궈");
						vcp_cmd_reserve(BACK_DOOR, 0, sec); // 열림 -> 잠금
					} else {
						vc_panel_vc_play_text("명령을 이해하지 못했습니다.");
					}
					return true;
				} else if (strstr(result, "취")) { // 예약해제
					LOGD("Enter : 남문 -> 예약해제");

					/* 최종 판단 코드 */
					LOGD("SUCCESS : %s", _("IDS_CANCEL_BACKOPEN"));
					vcp_cmd_cancel(BACK_DOOR);
					return true;
				} else { // 일반
					LOGD("Enter : 남문 -> 일반");
					LOGD("String length : %s(%d)", result, strlen(result));

					if (strlen(result) >= 15) {
						vc_panel_vc_play_text("명령을 이해하지 못했습니다.");
						return true;
					}

					/* 최종 판단 코드 */
					if (strstr(result, "열")) {
						LOGD("Enter : 남문 -> 열어");
						vcp_cmd_set(BACK_DOOR, 1); // 잠금 -> 열림
					} else if (strstr(result, "잠")) {
						LOGD("Enter : 남문 -> 잠궈");
						vcp_cmd_set(BACK_DOOR, 0); // 열림 -> 잠금
					} else {
						vc_panel_vc_play_text("명령을 이해하지 못했습니다.");
					}
					return true;
				}
			} else if (0 == strstr(result, "일") - result) {
				LOGD("Enter : 일번기기");

				if (strstr(result, "뒤")) { // 예약
					LOGD("Enter : 일번기기 -> 예약");

					if (!strstr(result, "초")) {
						vc_panel_vc_play_text("단위를 확인할 수가 없습니다.");
						break;
					}

					double sec = 0.0f;
					if (strstr(result, "오")) {
						LOGD("Enter : 일번기기 -> 예약 -> 오초");
						sec = 5.0f;
					} else if (strstr(result, "십") || strstr(result, "시") || strstr(result, "식") || strstr(result, "수")) {
						LOGD("Enter : 일번기기 -> 예약 -> 십초");
						sec = 15.0f;
					} else {
						vc_panel_vc_play_text("이해할 수 없는 숫자입니다.");
						break;
					}

					/* 최종 판단 코드 */
					if (strstr(result, "동")) {
						LOGD("Enter : 일번기기 -> 예약 -> 동작");
						vcp_cmd_reserve(SWITCH_A, 1, sec); // 중지 -> 동작
					} else if (strstr(result, "중")) {
						LOGD("Enter : 일번기기 -> 예약 -> 중지");
						vcp_cmd_reserve(SWITCH_A, 0, sec); // 동작 -> 중지
					} else {
						vc_panel_vc_play_text("명령을 이해하지 못했습니다.");
					}
					return true;
				} else if (strstr(result, "취")) { // 예약해제
					LOGD("Enter : 일번기기 -> 예약해제");

					/* 최종 판단 코드 */
					LOGD("SUCCESS : %s", _("IDS_CANCEL_A_ON"));
					vcp_cmd_cancel(SWITCH_A);
					return true;
				} else { // 일반
					LOGD("Enter : 일번기기 -> 일반");
					LOGD("String length : %s(%d)", result, strlen(result));

					if (strlen(result) >= 22) {
						vc_panel_vc_play_text("명령을 이해하지 못했습니다.");
						return true;
					}

					/* 최종 판단 코드 */
					if (strstr(result, "동")) {
						LOGD("Enter : 일번기기 -> 동작");
						vcp_cmd_set(SWITCH_A, 1); // 중지 -> 동작
					} else if (strstr(result, "중")) {
						LOGD("Enter : 일번기기 -> 중지");
						vcp_cmd_set(SWITCH_A, 0); // 동작 -> 중지
					} else {
						vc_panel_vc_play_text("명령을 이해하지 못했습니다.");
					}
					return true;
				}
			} else if (0 == strstr(result, "이") - result) {
				LOGD("Enter : 이번기기");

				if (strstr(result, "뒤")) { // 예약
					LOGD("Enter : 이번기기 -> 예약");

					if (!strstr(result, "초")) {
						vc_panel_vc_play_text("단위를 확인할 수가 없습니다.");
						break;
					}

					double sec = 0.0f;
					if (strstr(result, "오")) {
						LOGD("Enter : 이번기기 -> 예약 -> 오초");
						sec = 5.0f;
					} else if (strstr(result, "십") || strstr(result, "시") || strstr(result, "식") || strstr(result, "수")) {
						LOGD("Enter : 이번기기 -> 예약 -> 십초");
						sec = 15.0f;
					} else {
						vc_panel_vc_play_text("이해할 수 없는 숫자입니다.");
						break;
					}

					/* 최종 판단 코드 */
					if (strstr(result, "동")) {
						LOGD("Enter : 이번기기 -> 예약 -> 동작");
						vcp_cmd_reserve(SWITCH_B, 1, sec); // 중지 -> 동작
					} else if (strstr(result, "중")) {
						LOGD("Enter : 이번기기 -> 예약 -> 중지");
						vcp_cmd_reserve(SWITCH_B, 0, sec); // 동작 -> 중지
					} else {
						vc_panel_vc_play_text("명령을 이해하지 못했습니다.");
					}
					return true;
				} else if (strstr(result, "취")) { // 예약해제
					LOGD("Enter : 이번기기 -> 예약해제");

					/* 최종 판단 코드 */
					LOGD("SUCCESS : %s", _("IDS_CANCEL_B_ON"));
					vcp_cmd_cancel(SWITCH_B);
					return true;
				} else { // 일반
					LOGD("Enter : 이번기기 -> 일반");
					LOGD("String length : %s(%d)", result, strlen(result));

					if (strlen(result) >= 22) {
						vc_panel_vc_play_text("명령을 이해하지 못했습니다.");
						return true;
					}

					/* 최종 판단 코드 */
					if (strstr(result, "동")) {
						LOGD("Enter : 이번기기 -> 동작");
						vcp_cmd_set(SWITCH_B, 1); // 중지 -> 동작
					} else if (strstr(result, "중")) {
						LOGD("Enter : 이번기기 -> 중지");
						vcp_cmd_set(SWITCH_B, 0); // 동작 -> 중지
					} else {
						vc_panel_vc_play_text("명령을 이해하지 못했습니다.");
					}
					return true;
				}
			} else {
				vc_panel_vc_play_text("다시 말씀해주세요.");
			}
		} while (0);
#if 0
		if (!strcasecmp(result, _("IDS_FRONTOPEN"))) {
			LOGD("SUCCESS : %s", _("IDS_FRONTOPEN"));
			vcp_cmd_set(FRONT_DOOR, 1);
			return true;
		} else if (!strcasecmp(result, _("IDS_FRONTCLOSE"))) {
			LOGD("SUCCESS : %s", _("IDS_FRONTCLOSE"));
			vcp_cmd_set(FRONT_DOOR, 0);
			return true;
		} else if (!strcasecmp(result, _("IDS_BACKOPEN"))) {
			LOGD("SUCCESS : %s", _("IDS_BACKOPEN"));
			vcp_cmd_set(BACK_DOOR, 1);
			return true;
		} else if (!strcasecmp(result, _("IDS_BACKCLOSE"))) {
			LOGD("SUCCESS : %s", _("IDS_BACKCLOSE"));
			vcp_cmd_set(BACK_DOOR, 0);
			return true;
		} else if (!strcasecmp(result, _("IDS_A_ON"))) {
			LOGD("SUCCESS : %s", _("IDS_A_ON"));
			vcp_cmd_set(SWITCH_A, 1);
			return true;
		} else if (!strcasecmp(result, _("IDS_A_OFF"))) {
			LOGD("SUCCESS : %s", _("IDS_A_OFF"));
			vcp_cmd_set(SWITCH_A, 0);
			return true;
		} else if (!strcasecmp(result, _("IDS_B_ON"))) {
			LOGD("SUCCESS : %s", _("IDS_B_ON"));
			vcp_cmd_set(SWITCH_B, 1);
			return true;
		} else if (!strcasecmp(result, _("IDS_B_OFF"))) {
			LOGD("SUCCESS : %s", _("IDS_B_OFF"));
			vcp_cmd_set(SWITCH_B, 0);
			return true;
		} else if (!strcasecmp(result, _("IDS_RESERVE_FRONTOPEN"))) {
			LOGD("SUCCESS : %s", _("IDS_RESERVE_FRONTOPEN"));
			vcp_cmd_reserve(FRONT_DOOR, 1, 5.0f);
			return true;
		} else if (!strcasecmp(result, _("IDS_RESERVE_FRONTCLOSE"))) {
			LOGD("SUCCESS : %s", _("IDS_RESERVE_FRONTCLOSE"));
			vcp_cmd_reserve(FRONT_DOOR, 0, 5.0f);
			return true;
		} else if (!strcasecmp(result, _("IDS_RESERVE_BACKOPEN"))) {
			LOGD("SUCCESS : %s", _("IDS_RESERVE_BACKOPEN"));
			vcp_cmd_reserve(BACK_DOOR, 1, 5.0f);
			return true;
		} else if (!strcasecmp(result, _("IDS_RESERVE_BACKCLOSE"))) {
			LOGD("SUCCESS : %s", _("IDS_RESERVE_BACKCLOSE"));
			vcp_cmd_reserve(BACK_DOOR, 0, 5.0f);
			return true;
		} else if (!strcasecmp(result, _("IDS_RESERVE_A_ON"))) {
			LOGD("SUCCESS : %s", _("IDS_RESERVE_A_ON"));
			vcp_cmd_reserve(SWITCH_A, 1, 5.0f);
			return true;
		} else if (!strcasecmp(result, _("IDS_RESERVE_A_OFF"))) {
			LOGD("SUCCESS : %s", _("IDS_RESERVE_A_OFF"));
			vcp_cmd_reserve(SWITCH_A, 0, 5.0f);
			return true;
		} else if (!strcasecmp(result, _("IDS_RESERVE_B_ON"))) {
			LOGD("SUCCESS : %s", _("IDS_RESERVE_B_ON"));
			vcp_cmd_reserve(SWITCH_B, 1, 5.0f);
			return true;
		} else if (!strcasecmp(result, _("IDS_RESERVE_B_OFF"))) {
			LOGD("SUCCESS : %s", _("IDS_RESERVE_B_OFF"));
			vcp_cmd_reserve(SWITCH_B, 0, 5.0f);
			return true;
		} else if (!strcasecmp(result, _("IDS_CANCEL_FRONTOPEN"))) {
			LOGD("SUCCESS : %s", _("IDS_CANCEL_FRONTOPEN"));
			vcp_cmd_cancel(FRONT_DOOR);
			return true;
		} else if (!strcasecmp(result, _("IDS_CANCEL_FRONTCLOSE"))) {
			LOGD("SUCCESS : %s", _("IDS_CANCEL_FRONTCLOSE"));
			vcp_cmd_cancel(FRONT_DOOR);
			return true;
		} else if (!strcasecmp(result, _("IDS_CANCEL_BACKOPEN"))) {
			LOGD("SUCCESS : %s", _("IDS_CANCEL_BACKOPEN"));
			vcp_cmd_cancel(BACK_DOOR);
			return true;
		} else if (!strcasecmp(result, _("IDS_CANCEL_BACKCLOSE"))) {
			LOGD("SUCCESS : %s", _("IDS_CANCEL_BACKCLOSE"));
			vcp_cmd_cancel(BACK_DOOR);
			return true;
		} else if (!strcasecmp(result, _("IDS_CANCEL_A_ON"))) {
			LOGD("SUCCESS : %s", _("IDS_CANCEL_A_ON"));
			vcp_cmd_cancel(SWITCH_A);
			return true;
		} else if (!strcasecmp(result, _("IDS_CANCEL_A_OFF"))) {
			LOGD("SUCCESS : %s", _("IDS_CANCEL_A_OFF"));
			vcp_cmd_cancel(SWITCH_A);
			return true;
		} else if (!strcasecmp(result, _("IDS_CANCEL_B_ON"))) {
			LOGD("SUCCESS : %s", _("IDS_CANCEL_B_ON"));
			vcp_cmd_cancel(SWITCH_B);
			return true;
		} else if (!strcasecmp(result, _("IDS_CANCEL_B_OFF"))) {
			LOGD("SUCCESS : %s", _("IDS_CANCEL_B_OFF"));
			vcp_cmd_cancel(SWITCH_B);
			return true;
		}
	#endif

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

	LOGD("====");
	LOGD(" ");

	return true;
}

/*
vi:ts=4:ai:nowrap:expandtab
*/
