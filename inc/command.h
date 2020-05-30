#ifndef __COMMAND_H
#define __COMMAND_H

#ifdef __cplusplus
extern "C" {
#endif

#define	NUM_COMMAND_DEPTH	2

#define NUM_COMMAND_1ST		12
#define NUM_COMMAND_2ND		8

extern char* g_command_1st[NUM_COMMAND_1ST];
extern char* g_command_2nd[NUM_COMMAND_1ST][NUM_COMMAND_2ND];

extern char* g_hint_1st[NUM_COMMAND_1ST];
extern char* g_hint_2nd[NUM_COMMAND_1ST][NUM_COMMAND_2ND];

#ifdef __cplusplus
}
#endif

#endif /* __COMMAND_H */
