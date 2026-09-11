
#include"Key.h"



static uint8_t last_key = 0;
static uint32_t last_time = 0;
static uint8_t key_ready = 0;




uint8_t Key_Getstate(void)
{
    if(HAL_GPIO_ReadPin(KEY1_GPIO_Port,KEY1_Pin)==GPIO_PIN_RESET)
    {
        return 1;
    }
    if(HAL_GPIO_ReadPin(KEY2_GPIO_Port,KEY2_Pin)==GPIO_PIN_RESET)
    {
        return 2;
    }
    if(HAL_GPIO_ReadPin(KEY3_GPIO_Port,KEY3_Pin)==GPIO_PIN_RESET)
    {
        return 3;
    }
    return 0;
}





/**
 * @brief 按键扫描函数，用于检测按键状态并处理按键事件
 * 该函数通过比较当前按键状态与上一次的状态，并判断按键持续时间，
 * 来确定是否触发按键事件，然后调用相应的菜单处理函数
 */
void Key_Scan(void)
{
    uint8_t key_num = Key_Getstate();  // 获取当前按键状态


    // 如果当前按键状态与上一次不同
    if(key_num != last_key )
    {
        last_key = key_num;           // 更新上一次按键状态为当前状态
        last_time = HAL_GetTick();     // 记录当前时间戳
        key_ready = 0;                 // 重置按键就绪标志
    }
    // 当按键就绪标志为0，且按键持续时间超过20ms，且按键不为0（即有按键按下）
    if(!key_ready &&(HAL_GetTick()-last_time)>20  && last_key != 0)
    {
        key_ready = 1;                // 设置按键就绪标志为1
        
        Menu_HandleKey(key_num);      // 调用菜单处理函数处理当前按键
        
    }
    if(key_num == 0)
    {
        key_ready = 0;
    }
}