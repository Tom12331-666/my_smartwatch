#ifndef _MENU_H
#define _MENU_H

#include "main.h"
#include "stdio.h"
#include "oled.h"
#include "string.h"
#include "fonts.h"
#include "SetTime.h"
#include "rtc.h"
#include "menu_carousel.h"
#include"features.h"


void Show_Clock_UI(void);
void Menu_UI_Update(void);
void Show_Setting_clock(void);
void Show_SettingPage_UI(void);
void Menu_HandleKey(uint8_t key);

/***************智能手表状态******************/
typedef enum {
    UI_CLOCK,
    UI_SETTING,
    UI_DATETIME_SET,
    UI_MENU,
    UI_FEATURE_WATCH,
    UI_FEATURE_FLASHLIGHT,
    UI_FEATURE_MPU6050,
    UI_FEATURE_GAME,
    UI_FEATURE_EMOJI,
    UI_FEATURE_LEVEL,
} ui_state_t;



#endif /* _MENU_H */