#ifndef OLED_H
#define OLED_H

#include "main.h"


#define OLED_ADDRESS 0x3C //oled地址
#define IIC_TIMEOUT 1000 //i2c超时时间

#define OLED_WR_CMD        (0x00)    // OLED写指令时的数据标志，表示接下来写的是命令
#define OLED_WR_DATA       (0x40)    // OLED写数据时的数据标志，表示接下来写的是数据

#define OLED_ARRAY_SIZE(A) (sizeof(A)/sizeof(A[0]))  // 计算数组A的元素个数，常用宏

#define TEXTSIZE_F6X8       6  
#define TEXTSIZE_F8X16      8  
#define TEXTSIZE_F16X16     16  


/************ 函数声明 ************/
void OLED_Init(void);
HAL_StatusTypeDef OLED_CheckDevice(uint8_t slave_addr);
HAL_StatusTypeDef OLED_WriteByte(uint8_t cmd, uint8_t data);
HAL_StatusTypeDef OLED_WriteBuffer(uint8_t cmd, uint8_t *buffer, uint32_t num);

void OLED_SetPos(uint8_t y, uint8_t x);
void OLED_Fill(uint8_t fill_data);
void OLED_CLS(void);
void OLED_FillFull(void);
void OLED_ShowChinese_F16X16(uint8_t line, uint8_t offset, uint8_t n);
void OLED_ShowChinese(uint8_t y, uint8_t x, uint8_t n, const uint8_t data_cn[][16]);
void OLED_ShowChar(uint8_t y, uint8_t x, uint8_t char_data, uint8_t textsize);
void OLED_ShowString(uint8_t y, uint8_t x, uint8_t *string_data, uint8_t textsize);
void OLED_ShowString_F8X16(uint8_t line, uint8_t offset, uint8_t *string_data);
void OLED_ShowStringInvert(uint8_t y, uint8_t x, uint8_t *string_data, uint8_t textsize);
void OLED_DrawBitmap(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap);
void OLED_ShowChineseHighlight_F16X16(uint8_t line, uint8_t offset, uint8_t n);
void OLED_DrawBitmapInvert(uint8_t x,uint8_t y,uint8_t width,uint8_t height,const uint8_t *bitmap);

/* 画圆辅助函数 */
void OLED_Graphic_Clear(void);
void OLED_Graphic_Flush(void);
void OLED_DrawCircle(int16_t CX, int16_t CY, uint8_t Radius, uint8_t IsFilled);
void OLED_DrawEllipse(int16_t X, int16_t Y, uint8_t A, uint8_t B, uint8_t IsFilled);
void OLED_DrawImageToBuffer(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image);

#endif // OLED_H
