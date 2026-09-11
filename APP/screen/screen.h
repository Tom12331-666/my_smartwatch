#ifndef __SCREEN_H
#define __SCREEN_H

#include "main.h"
#include <stdint.h>

typedef enum {
    SCREEN_MAIN = 0,   /* 屏1：标题页 */
    SCREEN_name1,        /* 屏2：ADC1 电压（PA0） */
    SCREEN_name2,    /* 屏3：运行计数器 */
    SCREEN_COUNT       /* 总屏数，用于取模循环 */
} screen_id_t;

void Screen_Init(void);
void Screen_Switch(void);
void Screen_Update(void);

#endif /* __SCREEN_H */