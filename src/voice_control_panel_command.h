#ifndef __VOICE_CONTROL_PANEL_COMMAND_H
#define __VOICE_CONTROL_PANEL_COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

#define	NUM_COMMAND_DEPTH	2

#define NUM_COMMAND_1ST		24
#define NUM_COMMAND_2ND		8

enum {
    FRONT_DOOR = 1,
    BACK_DOOR,
    SWITCH_A,
    SWITCH_B
};

extern char* g_command_1st[NUM_COMMAND_1ST];
extern char* g_command_2nd[NUM_COMMAND_1ST][NUM_COMMAND_2ND];

extern char* g_hint_1st[NUM_COMMAND_1ST];
extern char* g_hint_2nd[NUM_COMMAND_1ST][NUM_COMMAND_2ND];

int vcp_cmd_set(int type, int cmd);
int vcp_cmd_reserve(int type, int cmd, double seconds);
int vcp_cmd_cancel(int type);

#ifdef __cplusplus
}
#endif

#endif /* __VOICE_CONTROL_PANEL_VIEW_H */

/*
vi:ts=4:ai:nowrap:expandtab
*/
