#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

#include <app.h>
#include <glib.h>
#include <Elementary.h>
#include <dlog.h>
#include <efl_util.h>

enum {
	PANEL_STATE_INIT = 1,
	PANEL_STATE_PAUSE,
	PANEL_STATE_SERVICE,
	PANEL_STATE_TERMINATE,
};

typedef struct _appdata {
	/* GUI */
	float scale_w;
	float scale_h;

	Evas_Object *win;
	Evas_Object *layout_main;
	Evas_Object *image_mic;
	Evas_Object *image_arrow;
	Evas_Object *content_box;
	Evas_Object *image_setting;
	Evas_Object *image_close;

	Evas_Object *help_win;
	Evas_Object *help_genlist;
	Evas_Object *help_layout;

	Elm_Theme *theme;

	int app_state;

	int act_state;

	int current_depth;
	int current_path[2];

	efl_util_inputgen_h touch;
	efl_util_inputgen_h back_key;

	GList *cmd_list;
} appdata;

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
