
#ifndef __DISP_LAYER_H__
#define __DISP_LAYER_H__

#include "disp_private.h"

s32 disp_init_lyr(__disp_bsp_init_para * para);
s32 disp_lyr_shadow_protect(struct disp_layer *lyr, bool protect);

#endif
