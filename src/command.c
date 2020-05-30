#include "command.h"

#include <stdio.h>


char* g_command_1st[NUM_COMMAND_1ST] = {
	"정문 열어", "정문 잠궈",
	"후문 열어", "후문 잠궈",
	"스위치 에이 켜", "스위치 에이 꺼",
	"스위치 비 켜", "스위치 비 꺼",
	"오초 후에 정문 열어", "오초 후에 후문 열어",
	"정문 알람 취소", "후문 알람 취소"
};
char *g_hint_1st[NUM_COMMAND_1ST] = {
	"정문 열어", "정문 잠궈",
	"후문 열어", "후문 잠궈",
	"스위치 에이 켜", "스위치 에이 꺼",
	"스위치 비 켜", "스위치 비 꺼",
	"오초 후에 정문 열어", "오초 후에 후문 열어",
	"정문 알람 취소", "후문 알람 취소"
};

char* g_command_2nd[NUM_COMMAND_1ST][NUM_COMMAND_2ND] = {
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
};

char* g_hint_2nd[NUM_COMMAND_1ST][NUM_COMMAND_2ND] = {
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
	{NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL},
};
