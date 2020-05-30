#ifndef __VIEW_H
#define __VIEW_H

#ifdef __cplusplus
extern "C" {
#endif

int view_create(void *data);
void view_destroy(void *data);
int view_show(void *data);
int view_hide(void *data);
int view_add_app(int pid, int *count, void *data);
int view_show_app_list(void *data);
int view_show_help(void *data);
int view_hide_help(void *data);
int view_show_result(void *data, const char *result);

#ifdef __cplusplus
}
#endif

#endif /* __VIEW_H */
