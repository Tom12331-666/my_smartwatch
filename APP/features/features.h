#ifndef _FEATURES_H_
#define _FEATURES_H_



#include"main.h"
#include <stdint.h>
#include "app_mpu6050.h"


int Features_Enter(uint8_t item);
void Stopwatch_Enter(void);
void Stopwatch_Update(void);
uint8_t Stopwatch_HandleKey(uint8_t key);

//手电筒
uint8_t Flashlight_HandleKey(uint8_t key);
void Flashlight_Enter(void);

//MPU6050
void MPU6050_Update(void);
void MPU6050_Enter(void);
uint8_t Mpu6050_HandleKey(uint8_t key);


//动态表情
void Emoji_Enter(void);
void Emoji_Update(void);
uint8_t Emoji_HandleKey(uint8_t key);

//水平仪
void LEVEL_Enter(void);
void LEVEL_Update(void);
uint8_t LEVEL_HandleKey(uint8_t key);

/* 恐龙小游戏 */
void GameDino_Enter(void);
void GameDino_Update(void);
uint8_t GameDino_HandleKey(uint8_t key);

#endif /* _FEATURE_H_ */
