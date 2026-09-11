#include"features.h"
#include "app_mpu6050.h"
#include"oled.h"
#include"fonts.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_flash.h"
#include <stdint.h>
#include <sys/_intsup.h>
#include"menu.h"
#include "main.h"
#include"math.h"


#define RAD_TO_DEG 57.295779513082f
volatile uint8_t mpu_sample_flag = 0;

/*************************************以上是定义区************************************ */


int Features_Enter(uint8_t item)
{

	switch(item)
	{
		case 1:
			Stopwatch_Enter();
            return UI_FEATURE_WATCH;
			break;
		case 2:
			Flashlight_Enter();
			return UI_FEATURE_FLASHLIGHT;
			break;
		case 3:
            MPU6050_Enter();
			return UI_FEATURE_MPU6050;
			break;
		case 4:
            GameDino_Enter();
            return UI_FEATURE_GAME;
            break;
		case 5:
            Emoji_Enter();
			return UI_FEATURE_EMOJI;
		case 6:
            LEVEL_Enter();
            return UI_FEATURE_LEVEL;
			break;
    default:
        return UI_MENU;
    }
}







/*************************************秒表的功能************************************ */

static uint32_t run_start = 0;
static uint8_t running = 0;
static uint32_t paused_ms = 0;//暂停计时之前跑了多少时间

static uint8_t last_hour = 0xFF;
static uint8_t last_minute = 0xFF;
static uint8_t last_second = 0xFF;


static uint32_t get_elapsed_ms(void)
{
    if(running)
        return  paused_ms + HAL_GetTick() - run_start;

    else
        return paused_ms;

}
static void darw_stopwatch(uint32_t elapsed_time)
{
    uint32_t total_seconds = elapsed_time / 1000U;

    uint8_t hour = (uint8_t)(total_seconds / 3600U);
    uint8_t minutes = (uint8_t)((total_seconds % 3600U) / 60U);
    uint8_t seconds = (uint8_t)(total_seconds % 60U);

    char buf[16];

    sprintf(buf, "%02u:%02u:%02u", hour, minutes, seconds);
    
    OLED_CLS();
    OLED_ShowString(3, 32, (uint8_t*)buf, TEXTSIZE_F8X16);

    if(!running)
    {
        OLED_ShowChinese_F16X16(3, 0, 19);//开
        OLED_ShowChinese_F16X16(3, 1, 20);//始
        OLED_ShowChinese_F16X16(3, 3, 21);
        OLED_ShowChinese_F16X16(3, 4, 22);
        OLED_ShowChinese_F16X16(3, 6,23 );
        OLED_ShowChinese_F16X16(3, 7, 24);
    }
    else
    {
        OLED_ShowChinese_F16X16(3, 0, 25);//暂
        OLED_ShowChinese_F16X16(3, 1, 26);//停
        OLED_ShowChinese_F16X16(3, 3, 21);
        OLED_ShowChinese_F16X16(3, 4, 22);
        OLED_ShowChinese_F16X16(3, 6,23 );
        OLED_ShowChinese_F16X16(3, 7, 24); 
    }
}

void Stopwatch_Enter(void)
{
    running = 0;
    paused_ms = 0;
    run_start = 0;
    
    last_hour = 0xFF;
    last_minute = 0xFF;
    last_second = 0xFF;

    darw_stopwatch(0);

}

void Stopwatch_Update(void)
{
    if(!running)return;

    uint32_t elapsed_time = get_elapsed_ms();
    uint32_t total_seconds = elapsed_time / 1000U;

    uint8_t hour = (uint8_t)(total_seconds / 3600U);
    uint8_t minutes = (uint8_t)((total_seconds % 3600U) / 60U);
    uint8_t seconds = (uint8_t)(total_seconds % 60U);

    if(hour != last_hour || minutes != last_minute || seconds != last_second)
    {
        darw_stopwatch(elapsed_time);
        last_hour = hour;
        last_minute = minutes;
        last_second = seconds;
    }

}

uint8_t Stopwatch_HandleKey(uint8_t key)
{
    if(key == 1)
    {
        if(running)
        {
            running = 0;
            paused_ms += HAL_GetTick() - run_start;
        }
        else
        {
            running = 1;
            run_start = HAL_GetTick();

        }

        darw_stopwatch(get_elapsed_ms());

        return 0;
    }

    if(key == 2)
    {
        /*清零*/
        running = 0;
        paused_ms = 0;
        run_start = 0;

        last_hour = 0xFF;
        last_minute = 0xFF;
        last_second = 0xFF;

        darw_stopwatch(0);

        return 0;
    }

    if(key == 3)
    {
        /*退出，返回菜单*/
        return 1;
    }
    return 0;
}

/*************************************秒表的功能end************************************ */



/*************************************手电的功能begin************************************ */

static int LED_flag = 0;

static void darw_flashlight(void)
{
    OLED_CLS();
    OLED_DrawBitmap(0,0,16,16,Return_16x16);
    OLED_ShowString_F8X16(1, 3, (uint8_t*)"ON");
    OLED_ShowString_F8X16(1,10, (uint8_t*)"OFF");

    if (LED_flag == 1)
    {
        OLED_ShowStringInvert(2, 24,(uint8_t*)"ON",TEXTSIZE_F8X16 );
        LED_flag = 0;
    }
    else if (LED_flag == 2)
    {
        OLED_ShowStringInvert(2, 80,(uint8_t*)"OFF",TEXTSIZE_F8X16 );
        LED_flag = 0;
    }
}


void Flashlight_Enter(void)
{
    LED_flag = 0;
    darw_flashlight();
}


uint8_t Flashlight_HandleKey(uint8_t key)
{
    if(key == 1)
    {
        LED_flag = 1;
        HAL_GPIO_WritePin(FlashLight_GPIO_Port, FlashLight_Pin, GPIO_PIN_SET);
        darw_flashlight();
        return 0;
    }
    if(key == 2)
    {
        LED_flag = 2;
        HAL_GPIO_WritePin(FlashLight_GPIO_Port, FlashLight_Pin, GPIO_PIN_RESET);
        darw_flashlight();
        return 0;
    }
    if(key == 3)
    {
        return 1;
    }
    return 0;
}

/*************************************手电的功能end************************************ */







/*************************************mpu6050的功能begin************************************ */

static MPU6050_Angle mpu_angle = {0.0f, 0.0f, 0.0f};
static uint8_t mpu_start = 0;//判断mpu6050是否初始化过
static uint32_t mpu_last_display_time = 0;//记录上一次显示的时间


static void mpu_format_angle(float angle, char *buf)
{
    int tenths = (int)(angle * 10.0f);
    char sign = '+';

    if (tenths < 0)
    {
        sign = '-';
        tenths = -tenths;
    }

    sprintf(buf, "%c%03d.%d", sign, tenths / 10, tenths % 10);
}


void MPU6050_Enter(void)
{

    mpu_sample_flag = 0;

    mpu_angle.roll = 0.0f;
    mpu_angle.pitch = 0.0f;
    mpu_angle.yaw = 0.0f;
    
    mpu_last_display_time = HAL_GetTick();

    mpu_start = 0;

    if(MPU6050_Init() != HAL_OK)
    {
        OLED_CLS();
        OLED_ShowString(0, 0, (uint8_t*)"MPU6050 fail", TEXTSIZE_F8X16);
        return;
    }

    OLED_CLS();
    OLED_DrawBitmapInvert(0,0,16,16,Return_16x16);
    OLED_ShowString_F8X16(1, 0, (uint8_t*)"Roll:");
    OLED_ShowString_F8X16(1,6,(uint8_t*)"+000.0");
    OLED_ShowString_F8X16(2,0,(uint8_t*)"Pitch:");
    OLED_ShowString_F8X16(2,6,(uint8_t*)"+000.0");
    OLED_ShowString_F8X16(3,0,(uint8_t*)"Yaw:");
    OLED_ShowString_F8X16(3,6,(uint8_t*)"+000.0");
}


void MPU6050_Update(void)
{
    MPU6050_Data data;
    float dt = 0.005f; // 采样周期，单位秒
    char buf[16]; // 用于存储角度值的字符串

    if(mpu_sample_flag == 0)
        return;
    else
        mpu_sample_flag = 0;


    if(MPU6050_ReadData(&data) != HAL_OK)
        return;

    if(mpu_start == 0)
    {
        mpu_start = 1;

        mpu_angle.roll = atan2f(data.accel_g[1], data.accel_g[2]) * RAD_TO_DEG;
        mpu_angle.pitch = atan2f(-data.accel_g[0], sqrtf(data.accel_g[1] * data.accel_g[1] + data.accel_g[2] * data.accel_g[2])) * RAD_TO_DEG;
        mpu_angle.yaw = 0.0f;
    }
    else 
    {
        MPU6050_ComplementaryUpdate(&data, &mpu_angle, dt); // 使用互补滤波器更新角度
    }


    if(HAL_GetTick() - mpu_last_display_time < 200)
        return;
    else
    {
        mpu_last_display_time = HAL_GetTick();

        mpu_format_angle(mpu_angle.roll, buf);
        OLED_ShowString_F8X16(1, 6, (uint8_t*)buf);

        mpu_format_angle(mpu_angle.pitch, buf);
        OLED_ShowString_F8X16(2, 6, (uint8_t*)buf);

        mpu_format_angle(mpu_angle.yaw, buf);
        OLED_ShowString_F8X16(3, 6, (uint8_t*)buf);
    }
}



uint8_t Mpu6050_HandleKey(uint8_t key)
{
    if(key == 3)
    {
        return 1;
    }
    return 0;
}




/*************************************mpu6050的功能end************************************ */










/*************************************动态表情包的功能begin************************************ */
static uint32_t emoji_display_time = 0;
static uint8_t emoji_frame = 0;         /* 当前眉毛帧 */


static void draw_emoji_face(uint8_t frame)
{
    OLED_Graphic_Clear();

    OLED_DrawImageToBuffer(30, 10, 16, 16, Eyebrow[0]);
    OLED_DrawImageToBuffer(82, 10, 16, 16, Eyebrow[1]);

    OLED_DrawEllipse(40, 32, 6, 6, 1);
    OLED_DrawEllipse(88, 32, 6, 6, 1);

    OLED_DrawImageToBuffer(54, 40, 20, 20, Mouth);

    OLED_Graphic_Flush();
}




void Emoji_Enter(void)
{
    emoji_display_time = HAL_GetTick();

    OLED_Graphic_Clear();
    draw_emoji_face(emoji_frame);



}

void Emoji_Update(void)
{


	/*±ÕÑÛ*/
	for(uint8_t i=0;i<3;i++)
	{
		OLED_Graphic_Clear();
		OLED_DrawImageToBuffer(30, 10+i, 16, 16, Eyebrow[0]);
        OLED_DrawImageToBuffer(82, 10+i, 16, 16, Eyebrow[1]);
		OLED_DrawEllipse(40,32,6,6-i,1);//×óÑÛ
		OLED_DrawEllipse(88,32,6,6-i,1);//ÓÒÑÛ
		OLED_DrawImageToBuffer(54, 40, 20, 20, Mouth);
		OLED_Graphic_Flush();
		    
	}
	
	/*ÕöÑÛ*/
	for(uint8_t i=0;i<3;i++)
	{
		OLED_Graphic_Clear();
		OLED_DrawImageToBuffer(30, 12-i, 16, 16, Eyebrow[0]);//×óÃ¼Ã«
		OLED_DrawImageToBuffer(82, 12-i, 16, 16, Eyebrow[1]);//ÓÒÃ¼Ã«
		OLED_DrawEllipse(40,32,6,4+i,1);//×óÑÛ
		OLED_DrawEllipse(88,32,6,4+i,1);//ÓÒÑÛ
		OLED_DrawImageToBuffer(54, 40, 20, 20, Mouth);
		OLED_Graphic_Flush();
        HAL_Delay(100);
    }
	
	HAL_Delay(500);
	

}

uint8_t Emoji_HandleKey(uint8_t key)
{
    if(key == 3)
    {
        return 1;
    }
    return 0;
}






/*************************************动态表情包的功能end************************************ */





/*************************************水平仪的功能begin************************************ */
#define LEVEL_CX 64
#define LEVEL_CY 32
#define LEVEL_RADIUS 28
#define LEVEL_DOT_RADIUS 3
#define LEVEL_SCALE 2.0f // 每度对应的像素数
#define LEVEL_OK_TOLERANCE 1.0f // 判定水平仪水平的容差，单位度

static uint8_t level_start = 0;
static uint32_t level_last_display_time = 0;
static MPU6050_Angle mpu_level_angle = {0.0f, 0.0f, 0.0f};


static void draw_level_ui(void)
{
    //1.小圆的圆心坐标
    float dot_x = LEVEL_CX + mpu_level_angle.roll * LEVEL_SCALE;
    float dot_y = LEVEL_CY + mpu_level_angle.pitch * LEVEL_SCALE;

    //2.小圆点的最大偏移量
    float max_offset = LEVEL_RADIUS - LEVEL_DOT_RADIUS; // 28 - 3;

    //3.小圆点的实际偏移量
    float dx = dot_x - LEVEL_CX;
    float dy = dot_y - LEVEL_CY;

    //4.实际直线距离
    float dist = sqrtf(dx*dx + dy*dy);

    if(dist >max_offset)
    {
        dot_x = LEVEL_CX + dx /dist * max_offset;
        dot_y = LEVEL_CY + dy /dist * max_offset;
    }

    /* 清空画圆显存 */
    OLED_Graphic_Clear();

    /* 画外面的大圆 */
    OLED_DrawCircle(LEVEL_CX, LEVEL_CY, LEVEL_RADIUS, 0);

    /* 画中间的小圆点 */
    OLED_DrawCircle((int16_t)dot_x, (int16_t)dot_y,LEVEL_DOT_RADIUS, 1);

    OLED_Graphic_Flush();

    /* 水平时显示提示 */
    if (fabsf(mpu_level_angle.roll) < LEVEL_OK_TOLERANCE &&
        fabsf(mpu_level_angle.pitch) < LEVEL_OK_TOLERANCE)
    {
        OLED_ShowString(7, 44,(uint8_t*)"LEVEL OK",TEXTSIZE_F6X8);
    }
}









void LEVEL_Enter(void)
{
    mpu_sample_flag = 0;

    mpu_level_angle.roll = 0.0f;
    mpu_level_angle.pitch = 0.0f;
    mpu_level_angle.yaw = 0.0f;

    level_start = 0;
    level_last_display_time = HAL_GetTick();
    if(MPU6050_Init() != HAL_OK)
    {
        OLED_CLS();
        OLED_ShowString(0, 0, (uint8_t*)"MPU6050 fail", TEXTSIZE_F8X16);
        return;
    }


    //先画一个大圆
    OLED_Graphic_Clear();
    OLED_DrawCircle(64, 32, 28, 0);
    OLED_Graphic_Flush();
    
    //再画一个小圆
    OLED_DrawCircle(64, 32, 3, 1);
    OLED_Graphic_Flush();


}


//对LEVEL界面进行刷新,并在小圆点上显示水平仪的状态
void LEVEL_Update(void)
{

    MPU6050_Data data;
    float dt = 0.005f; // 采样周期，单位秒


    if(mpu_sample_flag == 0)
        return;
    else
        mpu_sample_flag = 0;


    if(MPU6050_ReadData(&data) != HAL_OK)
        return;


    if(level_start == 0)
    {
        level_start = 1;

        mpu_level_angle.roll = atan2f(data.accel_g[1], data.accel_g[2]) * RAD_TO_DEG;
        mpu_level_angle.pitch = atan2f(-data.accel_g[0], sqrtf(data.accel_g[1] * data.accel_g[1] + data.accel_g[2] * data.accel_g[2])) * RAD_TO_DEG;
        mpu_level_angle.yaw = 0.0f;
    }
    else 
    {
        MPU6050_ComplementaryUpdate(&data, &mpu_level_angle, dt); // 使用互补滤波器更新角度
    }


    if(HAL_GetTick() - level_last_display_time < 100)
        return;
    else
    {
        level_last_display_time = HAL_GetTick();
        draw_level_ui();
    }

}

uint8_t LEVEL_HandleKey(uint8_t key)
{
    if (key == 3)
    {
        return 1;
    }

    return 0;
}




/*************************************水平仪的功能end************************************ */





/************************************* Dino Game Begin ************************************/



/* 恐龙游戏的屏幕布局
y=0   ┌────────────────┐
      │                │
      │   天空/游戏区    │
      │                │
y=56  ├────────────────┤ <-- 地面顶边
      │  地面 8 像素高   │
y=63  └────────────────┘ <-- 屏幕最底部*/


#define GAME_DINO_W        16   /* 恐龙图片宽度 */
#define GAME_DINO_H        24   /* 恐龙图片高度 */
#define GAME_BARRIER_W     16   /* 障碍物图片宽度 */
#define GAME_BARRIER_H     24   /* 障碍物图片高度 */
#define GAME_DINO_X        20   /* 恐龙固定在屏幕左侧的横坐标 */
#define GAME_GROUND_Y      56   /* 地面所在的纵坐标 */
#define GAME_DINO_GROUND_Y (GAME_GROUND_Y - GAME_DINO_H)  /* 恐龙贴地时的 y */
#define GAME_FRAME_MS      30   /* 游戏每帧间隔时间 */
#define GAME_ANIM_MS       100  /* 恐龙跑步动画切换时间 */
#define GAME_SPEED         3    /* 地面和障碍物的移动速度 */
#define GAME_JUMP_VY       -9   /* 恐龙跳跃初速度，负值表示向上 */
#define GAME_GRAVITY       1    /* 恐龙下落重力 */

static int16_t game_dino_y = GAME_DINO_GROUND_Y;      /* 恐龙当前 y 坐标 */
static int16_t game_dino_vy = 0;                      /* 恐龙垂直速度 */
static int16_t game_obstacle_x = 128;                 /* 障碍物当前 x 坐标 */
static uint16_t game_ground_offset = 0;               /* 地面纹理滚动偏移 */
static int16_t game_cloud_x = 80;                     /* 云朵当前 x 坐标 */
static uint8_t game_barrier_type = 0;                 /* 当前障碍物类型 */
static uint8_t game_over = 0;                         /* 是否游戏结束 */
static uint32_t game_score = 0;                       /* 游戏分数 */
static uint32_t game_last_update = 0;                 /* 上一帧更新时间 */
static uint32_t game_last_anim = 0;                   /* 上一帧动画时间 */
static uint8_t game_anim_index = 0;                   /* 恐龙动画序列索引 */
static uint8_t game_dino_frame = 0;                   /* 当前恐龙图片帧 */
static uint32_t game_last_score_time = 0;             /* 上一次计分时间 */



static const uint8_t game_dino_frames[] = {0, 1, 2, 1}; /* 恐龙跑步帧顺序 */

/* 绘制连续滚动的地面 */
static void game_draw_ground(void)
{
    uint16_t offset = game_ground_offset % 256U;
    uint16_t remain = 256U - offset;
    uint16_t width = (remain < 128U) ? remain : 128U;

    if (width > 0)
    {
        OLED_DrawImageToBuffer(0,
                               GAME_GROUND_Y,
                               (uint8_t)width,
                               8,
                               &Ground[offset]);
    }

    if (width < 128U)
    {
        OLED_DrawImageToBuffer((int16_t)width,
                               GAME_GROUND_Y,
                               (uint8_t)(128U - width),
                               8,
                               &Ground[0]);
    }
}

/* 在屏幕左上角显示当前分数 */
static void game_draw_score(void)
{
    char buf[20];

    sprintf(buf, "SCORE %04lu", (unsigned long)game_score);
    OLED_ShowString_F8X16(0, 0, (uint8_t*)buf);
}

/* 绘制游戏结束画面 */
static void game_draw_over_screen(void)
{
    char buf[20];

    OLED_CLS();

    OLED_ShowString_F8X16(1, 3, (uint8_t*)"GAME OVER");
    sprintf(buf, "SCORE %04lu", (unsigned long)game_score);
    OLED_ShowString_F8X16(2, 3, (uint8_t*)buf);
    OLED_ShowString_F8X16(3, 2, (uint8_t*)"K2 RST K3 EXIT");
}

/* 绘制游戏当前一帧画面 */
static void game_draw_frame(void)
{
    /* 清空画图显存 */
    OLED_Graphic_Clear();

    /* 画滚动地面 */
    game_draw_ground();

    /* 画云朵 */
    OLED_DrawImageToBuffer(game_cloud_x, 16, 16, 8, Cloud);

    /* 画当前恐龙跑步帧 */
    OLED_DrawImageToBuffer(GAME_DINO_X,
                           game_dino_y,
                           GAME_DINO_W,
                           GAME_DINO_H,
                           Dino[game_dino_frame]);

    /* 画当前障碍物 */
    OLED_DrawImageToBuffer(game_obstacle_x,
                           GAME_GROUND_Y - GAME_BARRIER_H,
                           GAME_BARRIER_W,
                           GAME_BARRIER_H,
                           Barrier[game_barrier_type]);

    /* 将显存发送到 OLED */
    OLED_Graphic_Flush();

    /* 直接刷新分数文字 */
    game_draw_score();

    /* 如果游戏结束，再绘制游戏结束提示 */
    if (game_over)
    {
        game_draw_over_screen();
    }
}

/* 使用缩小的碰撞框判断恐龙和障碍物是否碰撞 */
static uint8_t game_check_collision(void)
{
    /* 恐龙缩小后的碰撞框 */
    int16_t dino_left  = GAME_DINO_X + 3;
    int16_t dino_right = GAME_DINO_X + 12;
    int16_t dino_top   = game_dino_y + 4;
    int16_t dino_bot   = game_dino_y + 23;

    /* 障碍物缩小后的碰撞框 */
    int16_t obs_left  = game_obstacle_x + 2;
    int16_t obs_right = game_obstacle_x + 13;
    int16_t obs_top   = GAME_GROUND_Y - GAME_BARRIER_H + 2;
    int16_t obs_bot   = GAME_GROUND_Y - 1;

    /* 横向没有重叠就不碰撞 */
    if (dino_right <= obs_left || dino_left >= obs_right)
    {
        return 0;
    }

    /* 纵向没有重叠就不碰撞 */
    if (dino_bot <= obs_top || dino_top >= obs_bot)
    {
        return 0;
    }

    /* 两个碰撞框都有重叠，说明撞到了 */
    return 1;
}

/* 进入恐龙游戏页面时初始化所有游戏变量 */
void GameDino_Enter(void)
{
    game_dino_y = GAME_DINO_GROUND_Y;
    game_dino_vy = 0;
    game_obstacle_x = 128;
    game_ground_offset = 0;
    game_cloud_x = 80;
    game_barrier_type = 0;
    game_over = 0;
    game_score = 0;
    game_anim_index = 0;
    game_dino_frame = 0;
    game_last_update = HAL_GetTick();
    game_last_anim = HAL_GetTick();
    game_last_score_time = HAL_GetTick();

    game_draw_frame();
}

/* 主循环调用的游戏更新函数 */
void GameDino_Update(void)
{
    uint32_t now;

    /* 游戏结束后不再更新游戏画面 */
    if (game_over)
    {
        return;
    }

    now = HAL_GetTick();

    /* 每 30ms 才更新一次，避免跑得太快 */
    if (now - game_last_update < GAME_FRAME_MS)
    {
        return;
    }

    game_last_update = now;

    /* 地面和云朵移动 */
    game_ground_offset = (uint16_t)((game_ground_offset + GAME_SPEED) % 256U);
    game_cloud_x -= 1;
    if (game_cloud_x < -16)
    {
        game_cloud_x = 128;
    }

    /* 障碍物从右向左移动 */
    game_obstacle_x -= GAME_SPEED;

    /* 每300ms加一次分，分数加一 */
    if (HAL_GetTick() - game_last_score_time >= 300)
    {
        game_score++;
        game_last_score_time = HAL_GetTick();
    }

    /* 障碍物完全离开屏幕后，按顺序生成下一个障碍 */
    if (game_obstacle_x < -GAME_BARRIER_W)
    {
        game_barrier_type = (uint8_t)((game_barrier_type + 1) % 3U);
        game_obstacle_x = 128;
    }

    /* 恐龙跳跃和重力 */
    if (game_dino_y < GAME_DINO_GROUND_Y || game_dino_vy != 0)
    {
        game_dino_vy += GAME_GRAVITY;
        game_dino_y += game_dino_vy;

        /* 回到地面后停止下落 */
        if (game_dino_y >= GAME_DINO_GROUND_Y)
        {
            game_dino_y = GAME_DINO_GROUND_Y;
            game_dino_vy = 0;
        }
    }

    /* 恐龙跑步动画切换 */
    if (now - game_last_anim >= GAME_ANIM_MS)
    {
        game_last_anim = now;
        game_anim_index = (uint8_t)((game_anim_index + 1) % 4U);
        game_dino_frame = game_dino_frames[game_anim_index];
    }

    /* 碰撞检测 */
    if (game_check_collision())
    {
        game_over = 1;
    }

    /* 绘制当前这一帧 */
    game_draw_frame();
}

/* 处理恐龙游戏的按键 */
uint8_t GameDino_HandleKey(uint8_t key)
{
    /* KEY1：只有在地面时才能跳跃 */
    if (key == 1)
    {
        if (!game_over &&
            game_dino_y == GAME_DINO_GROUND_Y &&
            game_dino_vy == 0)
        {
            game_dino_vy = GAME_JUMP_VY;
        }

        return 0;
    }

    /* KEY2：重新开始游戏 */
    if (key == 2)
    {
        GameDino_Enter();
        return 0;
    }

    /* KEY3：退出并返回菜单 */
    if (key == 3)
    {
        return 1;
    }

    return 0;
}

/************************************* Dino Game End ************************************/






