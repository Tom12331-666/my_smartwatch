//
#include "menu.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_rtc_ex.h"
#include <stdint.h>

//




static ui_state_t current_ui = UI_CLOCK;
static ui_state_t last_key = 0;



/*************************************************主界面*************************************************/

static uint8_t last_second = 0xFF;
static uint8_t last_date = 0xFF;
static uint8_t first_draw = 1;
static uint8_t last_time = 0;

static void read_clock(RTC_TimeTypeDef *time, RTC_DateTypeDef *date)
{
    HAL_RTC_GetTime(&hrtc, time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, date, RTC_FORMAT_BIN);
}

void Show_Clock_UI(void)
{
    RTC_TimeTypeDef Menu_time;
    RTC_DateTypeDef Menu_date;
    char buf[20];

    read_clock(&Menu_time, &Menu_date);

    OLED_CLS();

    sprintf(buf, "%04d-%02d-%02d", 2000+Menu_date.Year, Menu_date.Month, Menu_date.Date);
    OLED_ShowString(0, 0, (uint8_t*)buf, TEXTSIZE_F8X16);

    sprintf(buf, "%02d:%02d:%02d", Menu_time.Hours, Menu_time.Minutes, Menu_time.Seconds);
    OLED_ShowString(3, 32, (uint8_t*)buf, TEXTSIZE_F8X16);

    OLED_ShowChinese_F16X16(3,0,7);
    OLED_ShowChinese_F16X16(3,1,8);
    OLED_ShowChinese_F16X16(3,6,9);
    OLED_ShowChinese_F16X16(3,7,10);
}

//如果需要页面刷新可以把代码放到这里
void Menu_UI_Update(void)
{

    last_time = HAL_GetTick();

    //这个是主页面刷新
    if(current_ui == UI_CLOCK)
    {
        RTC_TimeTypeDef now_time;
        RTC_DateTypeDef now_date;
        char buf[20];

        read_clock(&now_time, &now_date);

        if(first_draw || last_second != now_time.Seconds)
        {
            last_second = now_time.Seconds;
            sprintf(buf, "%02d:%02d:%02d", now_time.Hours, now_time.Minutes, now_time.Seconds);
            OLED_ShowString(3, 32, (uint8_t*)buf, TEXTSIZE_F8X16);
        }

        if(first_draw || last_date != now_date.Date)
        {
            last_date = now_date.Date;
            sprintf(buf, "%04d-%02d-%02d", 2000+now_date.Year, now_date.Month, now_date.Date);
            OLED_ShowString(0, 0, (uint8_t*)buf, TEXTSIZE_F8X16);
        }

        first_draw = 0;
    }

    //这个是秒表计时页面刷新
    if(current_ui == UI_FEATURE_WATCH)
    {
        Stopwatch_Update();
    }


    //这个是MPU6050页面刷新
    if(current_ui == UI_FEATURE_MPU6050)
    {
        MPU6050_Update();
    }

    //这个是动态表情页面刷新
    if(current_ui == UI_FEATURE_EMOJI)
    {
        Emoji_Update();
    }

    //这个是水平仪页面刷新
    if(current_ui == UI_FEATURE_LEVEL)
    {
        LEVEL_Update();
    }

    /* 恐龙小游戏页面刷新 */
    if(current_ui == UI_FEATURE_GAME && (HAL_GetTick() - last_time >= 200))
    {
        GameDino_Update();
        last_time = HAL_GetTick();
    }
}

void Show_Setting_clock(void)
{
    RTC_TimeTypeDef Menu_time;
    RTC_DateTypeDef Menu_date;

    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == 0x5A5AU)//检查作为“RTC是否已配置”的标记。
    {
        /*读取之前保存的年、月、日、星期*/
        Menu_date.Year = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);
        Menu_date.Month = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);
        Menu_date.Date = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR4);
        Menu_date.WeekDay = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR5);

        //把之前保存的年、月、日、星期写入RTC
        HAL_RTC_SetDate(&hrtc, &Menu_date, RTC_FORMAT_BIN);
        return;
    }

    /*当RTC的寄存器从未备份过东西时*/
    Menu_date.Year = 26;
    Menu_date.Month = 8;
    Menu_date.Date = 15;
    Menu_date.WeekDay = RTC_WEEKDAY_SATURDAY;

    Menu_time.Hours = 15;
    Menu_time.Minutes = 30;
    Menu_time.Seconds = 0;

    HAL_RTC_SetTime(&hrtc, &Menu_time, RTC_FORMAT_BIN);
    HAL_RTC_SetDate(&hrtc, &Menu_date, RTC_FORMAT_BIN);

    /*保存年、月、日、星期到备份寄存器中*/
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x5A5AU);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, Menu_date.Year);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, Menu_date.Month);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, Menu_date.Date);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, Menu_date.WeekDay);
}

void Menu_HandleKey(uint8_t key)
{
    static uint8_t count = 1;
    if(count == 1)
    {
        count = 0;
        last_key = key;
    }

    /*主页面*/
    if(current_ui == UI_CLOCK)
    {
        switch(key)
        {
            case 1:
                OLED_ShowChinese_F16X16(3, 6, 9);
                OLED_ShowChinese_F16X16(3, 7, 10);
                OLED_ShowChineseHighlight_F16X16(3, 0, 7);
                OLED_ShowChineseHighlight_F16X16(3, 1, 8);
                break;

            case 2:
                OLED_ShowChinese_F16X16(3, 0, 7);
                OLED_ShowChinese_F16X16(3, 1, 8);
                OLED_ShowChineseHighlight_F16X16(3, 6, 9);
                OLED_ShowChineseHighlight_F16X16(3, 7, 10);
                break;

            case 3:
                if(last_key == 2 && key == 3)
                {
                    OLED_CLS();
                    Show_SettingPage_UI();
                    current_ui = UI_SETTING;
                    last_key = 0;
                    return;
                }
                /*通过这个进入菜单*/
                else if(last_key == 1 && key == 3)
                {
                    current_ui = UI_MENU;
                    MenuCarousel_Enter();

                    last_key = 0;
                    return;
                }
                break;

            default:
                break;
        }
    }

    /*进入设置界面*/
    else if(current_ui == UI_SETTING)
    {
        switch(key)
        {
            case 1:
                Show_SettingPage_UI();
                OLED_DrawBitmapInvert(0, 0, 16, 16, Return_16x16);
                break;

            case 2:
                OLED_DrawBitmap(0, 0, 16, 16, Return_16x16);
                OLED_ShowChineseHighlight_F16X16(1,0,11 );
                OLED_ShowChineseHighlight_F16X16(1,1,12 );
                OLED_ShowChineseHighlight_F16X16(1,2,13 );
                OLED_ShowChineseHighlight_F16X16(1,3,14 );
                OLED_ShowChineseHighlight_F16X16(1,4,9 );
                OLED_ShowChineseHighlight_F16X16(1,5,10);
                break;

            case 3:
                if(last_key == 1 && key == 3)
                {
                    current_ui = UI_CLOCK;
                    OLED_CLS();
                    Show_Clock_UI();
                }
                else if(last_key == 2 && key == 3)
                {
                    /* 日期时间设置入口 */
                    current_ui = UI_DATETIME_SET;
                    SetTime_Enter();//得到当前的真实时间和日期
                    return;
                }
                break;

            default:
                break;
        }
    }

    /*进入修改“日期时间设置”界面*/
    else if(current_ui == UI_DATETIME_SET)
    {
        //通过按键来设置时间
        //按键3来选择当前要修改的字段
        if(SetTime_HandleKey(key))
        {
            current_ui = UI_SETTING;
            last_key = 0;
            Show_SettingPage_UI();//设置界面
        }
    }

    /*进入菜单页面*/
    else if(current_ui == UI_MENU)
    {
        if(MenuCarousel_HandleKey(key))
        {
            uint8_t item = MenuCarousel_GetSelected();
           if (item == 0)
            {        current_ui = UI_CLOCK;
                    Show_Clock_UI();
            }
            else
            {
                //current_ui = UI_FEATURE_WATCH;
                
                current_ui = Features_Enter(item);
            }
        }
    }

    /*进入菜单的各个功能*/
    /****现在是秒表功能****/
    else if(current_ui == UI_FEATURE_WATCH)
    {
        if(Stopwatch_HandleKey(key))
        {
            current_ui = UI_MENU;
            MenuCarousel_Enter();
        }
    }

    /****现在是手电筒功能****/
    else if(current_ui == UI_FEATURE_FLASHLIGHT)
    {
        if(Flashlight_HandleKey(key))
        {
            current_ui = UI_MENU;
            MenuCarousel_Enter();
        }
    }

    /****现在是MPU6050功能****/
    else if(current_ui == UI_FEATURE_MPU6050)
    {
        if(Mpu6050_HandleKey(key))
        {
            current_ui = UI_MENU;
            MenuCarousel_Enter();
        }
    }

    /****现在是动态表情功能****/
    else if(current_ui == UI_FEATURE_EMOJI)
    {
        if(Emoji_HandleKey(key))
        {
            current_ui = UI_MENU;
            MenuCarousel_Enter();
        }
    }

    /****现在是水平仪功能****/
    else if(current_ui == UI_FEATURE_LEVEL)
    {
        if(LEVEL_HandleKey(key))
        {
            current_ui = UI_MENU;
            MenuCarousel_Enter();
        }
    }

    /* 恐龙小游戏页面按键 */
    else if(current_ui == UI_FEATURE_GAME)
    {
        if(GameDino_HandleKey(key))
        {
            current_ui = UI_MENU;
            MenuCarousel_Enter();
        }
    }
    last_key = key;
}

    



/****************************************设置界面**************************************************/

void Show_SettingPage_UI(void)
{
    OLED_CLS();

    OLED_DrawBitmap(0, 0, 16, 16, Return_16x16);
    OLED_ShowChinese_F16X16(1,0,11 );
    OLED_ShowChinese_F16X16(1,1,12 );
    OLED_ShowChinese_F16X16(1,2,13 );
    OLED_ShowChinese_F16X16(1,3,14 );
    OLED_ShowChinese_F16X16(1,4,9 );
    OLED_ShowChinese_F16X16(1,5,10);
}



/****************************************菜单界面**************************************************/


