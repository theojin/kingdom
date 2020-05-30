#ifndef __TOUCHEVENT_H
#define __TOUCHEVENT_H

#ifdef __cplusplus
extern "C" {
#endif

#define TOUCH_SWIPE_UP 1
#define TOUCH_SWIPE_DOWN 2
#define TOUCH_SWIPE_LEFT 3
#define TOUCH_SWIPE_RIGHT 4

int touch_set_speed(const int pps);
void touch_swipe(const int direction, void* data);
void touch_back(void* data);

#ifdef __cplusplus
}
#endif

#endif /* __TOUCHEVENT_H */
