#include"menu_carousel.h"
#include "oled.h"
#include "fonts.h"
#include <stdint.h>
#include <string.h>
#include <sys/select.h>

#define MENU_ITEM_COUNT 7

//这是一个“图标的指针数组”，比如menu_icons[0]就指向picture0_32x32（这个也是一个数组）
static const uint8_t *menu_icons[MENU_ITEM_COUNT] = {
    picture0_32x32,
    picture1_32x32,
    picture2_32x32,
    picture3_32x32,
    picture4_32x32,
    picture5_32x32,
    picture6_32x32,
};

static uint8_t selected = 0; //当前选中的图标

//图标超过oled屏幕时，图标就不显示了
static void draw_icon(int16_t x, int16_t y, const uint8_t *icon)
{
    if (x < 0 || x + 32 > 128)
    {
        return;
    }

    OLED_DrawBitmap((uint8_t)x, (uint8_t)y, 32, 32, icon);
}

// //显示带边框的图标
// //static void draw_icon_with_frame(int16_t x, int16_t y, const uint8_t *icon)
// {
// //     if (x < 0 || x + 32 > 128)
// //     {
// //         return;
// //     }

// //     uint8_t bitmap[128] = {0};

// //     for (uint8_t page = 0; page < 4; page++)
// //     {
// //         memcpy(&bitmap[page * 32], &icon[page * 32], 32);
// //     }

// //     for (uint8_t page = 0; page < 4; page++)
// //     {
// //         for (uint8_t col = 0; col < 32; col++)
// //         {
// //             bitmap[page * 32 + col] |= Frame[page * 32 + col];
// //         }
// //     }

// //     for (uint8_t page = 0; page < 4; page++)
// //     {
// //         OLED_SetPos((uint8_t)(y / 8 + page), (uint8_t)x);
// //         OLED_WriteBuffer(OLED_WR_DATA, &bitmap[page * 32], 32);
// //     }
// // }
// }

//在屏幕上显示三个图标
static void draw_three(uint8_t center_id)
{
    //让图标可以移动
    uint8_t left_id = (uint8_t)((center_id + MENU_ITEM_COUNT - 1) % MENU_ITEM_COUNT);
    uint8_t right_id = (uint8_t)((center_id + 1) % MENU_ITEM_COUNT);

    draw_icon(0,16,menu_icons[left_id]);
    OLED_DrawBitmap(42,8,44,44,Frame);
    draw_icon(48,16,menu_icons[center_id]);
    draw_icon(96,16,menu_icons[right_id]);

    //draw_icon_with_frame(48, 16, menu_icons[center_id]);
}


//进入菜单页时初始化并显示第一帧。
void MenuCarousel_Enter(void)
{
    selected = 0;

    OLED_CLS();
    draw_three(selected);
}


//管理不同按键执行的功能。
uint8_t MenuCarousel_HandleKey(uint8_t key)
{
    //按键1向左移动
    if (key == 1)
    {
        selected = (selected + MENU_ITEM_COUNT - 1) % MENU_ITEM_COUNT;
        OLED_CLS();
        draw_three(selected);
        return 0;

    }

    //按键2向右移动
    if (key == 2)
    {
        selected = (selected + 1) % MENU_ITEM_COUNT;
        OLED_CLS();
        draw_three(selected);
        return 0;
    }

    if (key == 3)
    {
        return 1;
    }
    return 0;
}



uint8_t MenuCarousel_GetSelected(void)
{
    return selected;
}

