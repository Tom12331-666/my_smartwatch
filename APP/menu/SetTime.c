#include"SetTime.h"
#include "rtc.h"
#include "oled.h"
#include "fonts.h"
#include "stm32f1xx_hal_rtc_ex.h"
#include <stdio.h>

//表示当前修改的字段，默认为年
typedef enum {
    FIELD_YEAR,
    FIELD_MONTH,
    FIELD_DAY,
    FIELD_HOUR,
    FIELD_MIN,
    FIELD_SEC
} edit_field_t;

static edit_field_t current_field = FIELD_YEAR;

static uint16_t edit_year = 2026;
static uint8_t edit_month = 8;
static uint8_t edit_day = 15;
static uint8_t edit_hour = 15;
static uint8_t edit_min = 30;
static uint8_t edit_sec = 0;





//判断是否为闰年
static uint8_t is_leap_year(uint16_t year)
{
    return ((year % 4U) == 0U && (year % 100U) != 0U) ||
           ((year % 400U) == 0U);
}

static uint8_t days_in_month(uint16_t year, uint8_t month)
{
    static const uint8_t days[] = {
        31U, 28U, 31U, 30U, 31U, 30U,
        31U, 31U, 30U, 31U, 30U, 31U
    };

    if (month == 2U && is_leap_year(year))
    {
        return 29U;
    }

    return days[month - 1U];
}

static uint8_t calc_weekday(uint16_t year, uint8_t month, uint8_t day)
{
    static const uint8_t t[] = {
        0U, 3U, 2U, 5U, 0U, 3U,
        5U, 1U, 4U, 6U, 2U, 4U
    };
    uint16_t y = year;

    if (month < 3U)
    {
        y--;
    }

    return (uint8_t)((y + y / 4U - y / 100U + y / 400U +
                      t[month - 1U] + day) % 7U);
}


static void draw_highlighted_field(void)
{
    char tmp[8] = {0};

    switch (current_field)
    {
        case FIELD_YEAR:
            sprintf(tmp, "%04d", edit_year);
            OLED_ShowStringInvert(0, 0, (uint8_t*)tmp, TEXTSIZE_F8X16);
            break;

        case FIELD_MONTH:
            sprintf(tmp, "%02d", edit_month);
            OLED_ShowStringInvert(0, 5U * 8U, (uint8_t*)tmp, TEXTSIZE_F8X16);
            break;

        case FIELD_DAY:
            sprintf(tmp, "%02d", edit_day);
            OLED_ShowStringInvert(0, 8U * 8U, (uint8_t*)tmp, TEXTSIZE_F8X16);
            break;

        case FIELD_HOUR:
            sprintf(tmp, "%02d", edit_hour);
            OLED_ShowStringInvert(2, 0, (uint8_t*)tmp, TEXTSIZE_F8X16);
            break;

        case FIELD_MIN:
            sprintf(tmp, "%02d", edit_min);
            OLED_ShowStringInvert(2, 3U * 8U, (uint8_t*)tmp, TEXTSIZE_F8X16);
            break;

        case FIELD_SEC:
            sprintf(tmp, "%02d", edit_sec);
            OLED_ShowStringInvert(2, 6U * 8U, (uint8_t*)tmp, TEXTSIZE_F8X16);
            break;

        default:
            break;
    }
}

static void draw_settime_ui(void)
{
    //跟随当前选中的日期或时间是年，还是月，还是日，还是时，还是分，还是秒，来绘制不同的UI
    static const char *field_name[] = {
        "YEAR", "MONTH", "DAY", "HOUR", "MIN", "SEC"
    };
    char buf[24];

    OLED_CLS();

    //显示日期
    sprintf(buf, "%04d-%02d-%02d", edit_year, edit_month, edit_day);
    OLED_ShowString(0, 0, (uint8_t*)buf, TEXTSIZE_F8X16);

    //显示时间
    sprintf(buf, "%02d:%02d:%02d", edit_hour, edit_min, edit_sec);
    OLED_ShowString(2, 0, (uint8_t*)buf, TEXTSIZE_F8X16);

    //显示当前修改的字段
    sprintf(buf, "> %s", field_name[current_field]);
    OLED_ShowString(4, 0, (uint8_t*)buf, TEXTSIZE_F8X16);

    //显示按键说明
    OLED_ShowString(6, 0, (uint8_t*)"K1- K2+ K3 NEXT", TEXTSIZE_F8X16);

    draw_highlighted_field();
}

static void change_current_value(int8_t delta)
{
    switch (current_field)
    {
        case FIELD_YEAR:
            if (delta > 0 && edit_year < 2099U)
            {
                edit_year++;
            }
            else if (delta < 0 && edit_year > 2000U)
            {
                edit_year--;
            }
            break;

        case FIELD_MONTH:
            if (delta > 0 && edit_month < 12U)
            {
                edit_month++;
            }
            else if (delta < 0 && edit_month > 1U)
            {
                edit_month--;
            }
            break;

        case FIELD_DAY:
            if (delta > 0 && edit_day < days_in_month(edit_year, edit_month))
            {
                edit_day++;
            }
            else if (delta < 0 && edit_day > 1U)
            {
                edit_day--;
            }
            break;

        case FIELD_HOUR:
            if (delta > 0 && edit_hour < 23U)
            {
                edit_hour++;
            }
            else if (delta < 0 && edit_hour > 0U)
            {
                edit_hour--;
            }
            break;

        case FIELD_MIN:
            if (delta > 0 && edit_min < 59U)
            {
                edit_min++;
            }
            else if (delta < 0 && edit_min > 0U)
            {
                edit_min--;
            }
            break;

        case FIELD_SEC:
            if (delta > 0 && edit_sec < 59U)
            {
                edit_sec++;
            }
            else if (delta < 0 && edit_sec > 0U)
            {
                edit_sec--;
            }
            break;

        default:
            break;
    }

    if (edit_day > days_in_month(edit_year, edit_month))
    {
        edit_day = days_in_month(edit_year, edit_month);
    }
}

static void save_and_write_rtc(void)
{
    RTC_TimeTypeDef t = {0};
    RTC_DateTypeDef d = {0};

    t.Hours = edit_hour;
    t.Minutes = edit_min;
    t.Seconds = edit_sec;

    d.Year = edit_year - 2000U;
    d.Month = edit_month;
    d.Date = edit_day;
    d.WeekDay = calc_weekday(edit_year, edit_month, edit_day);//将“年月日”实时换算成“星期几”

    HAL_RTC_SetTime(&hrtc, &t, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &d, RTC_FORMAT_BIN);

    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x5A5AU);//向RTC_BKP_DR1寄存器写入RTC已经被配置的标志
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, d.Year);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, d.Month);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, d.Date);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, d.WeekDay);
}

//得到当前的真实时间和日期
void SetTime_Enter(void)
{
    RTC_TimeTypeDef t;
    RTC_DateTypeDef d;

    HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &d, RTC_FORMAT_BIN);

    edit_year = 2000U + d.Year;
    edit_month = d.Month;
    edit_day = d.Date;
    edit_hour = t.Hours;
    edit_min = t.Minutes;
    edit_sec = t.Seconds;

    current_field = FIELD_YEAR;

    //
    draw_settime_ui();
}

//通过按键来设置时间
//按键3来选择当前要修改的字段
uint8_t SetTime_HandleKey(uint8_t key)
{
    switch (key)
    {
        case 1:
            change_current_value(-1);//当前字段的数值减一
            draw_settime_ui();//重新显示当前修改后的数值
            break;

        case 2:
            change_current_value(1);//当前字段的数值加一
            draw_settime_ui();//重新显示当前修改后的数值
            break;

        case 3:
            if (current_field == FIELD_SEC)
            {
                save_and_write_rtc();//保存当前时间到RTC
                return 1U;
            }

            current_field = (edit_field_t)(current_field + 1);//选择下一个字段
            draw_settime_ui();
            break;

        default:
            break;
    }

    return 0U;
}
