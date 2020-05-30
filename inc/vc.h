#ifndef __VC_H
#define __VC_H

#ifdef __cplusplus
extern "C" {
#endif

int vc_start(void *data);
int vc_cancel(void *data);
int vc_init(void *data);
int vc_deinit(void *data);
int vc_deactivate(void *data, double delay);
int vc_activate(void *data);
int vc_finalize(void *data);

#ifdef __cplusplus
}
#endif

#endif /* __VC_H */
