


#include "i2c.h"
#include "oled.h"
#include <stdio.h>
#include <string.h>
#include "fonts.h"

HAL_StatusTypeDef OLED_CheckDevice(uint8_t slave_addr)
{
    uint8_t dummy = 0;
    
    //检测通信
    if (HAL_I2C_Master_Transmit(&hi2c1, (slave_addr<<1), &dummy, 1, IIC_TIMEOUT) == HAL_OK)
    {
        return HAL_OK;
    }
    return HAL_ERROR;
}

//写入数据到OLED（可以是命令或数据）
HAL_StatusTypeDef OLED_WriteByte(uint8_t cmd, uint8_t data)
{
    uint8_t buffer[2];
    //先放控制字节，再放数据字节，形成2字节的传输数据包
    buffer[0] = cmd; 
    buffer[1] = data; 

    //通过I2C主机发送数据包到OLED设备，地址左移一位，因为I2C地址是7位，最低位是读写位，发送两个字节，超时时间为IIC_TIMEOUT
    if (HAL_I2C_Master_Transmit(&hi2c1, (OLED_ADDRESS<<1), buffer, 2, IIC_TIMEOUT) == HAL_OK)
    {
        return HAL_OK;
    }

    //如果发送失败，打印错误信息并返回HAL_ERROR
    printf("OLED_WriteByte 写入失败: cmd=0x%02X, data=0x%02X\n", cmd, data);
    return HAL_ERROR;
}

/**
  * @brief  向 OLED 写入多个字节（包括命令字节和数据字节）
  * @param  cmd: 写入模式，取值为 OLED_WR_CMD（写命令）或 OLED_WR_DATA（写数据）
  * @param  buffer: 指向需要写入的数据缓冲区指针
  * @param  num: 要写入的数据字节数
  * @retval HAL_OK 表示写入成功，HAL_ERROR 表示写入失败
  */
HAL_StatusTypeDef OLED_WriteBuffer(uint8_t cmd, uint8_t *buffer, uint32_t num)
{
    // 最大256字节，可根据实际OLED限制调整
    uint8_t buf[256];
    if(num > sizeof(buf) - 1)
    {
        printf("OLED_WriteBuffer 数据过长(%lu)\r\n", (unsigned long)num);
        return HAL_ERROR;
    }

    buf[0] = cmd;                 // 第一个字节为命令或数据标识
    memcpy(&buf[1], buffer, num);  // 将实际数据复制到buf数组，从第二个字节开始存放

    // 通过I2C发送数据给OLED，发送长度为num+1字节
    HAL_StatusTypeDef status = HAL_I2C_Master_Transmit(&hi2c1, OLED_ADDRESS<< 1, buf, num + 1, IIC_TIMEOUT);

    if (status == HAL_OK)
        return HAL_OK;          // 发送成功返回成功状态

    // 发送失败，打印调试信息
    printf("OLED_WriteBuffer 写入失败cmd=0x%02X len=%lu\r\n", cmd, (unsigned long)num);
    return HAL_ERROR;                // 返回失败状态
}

/**
  * @brief  设置 OLED 显示的光标位置
  * @param  y：页地址，范围 0~7，对应 OLED 的行（每页8像素高）
  * @param  x：列地址，范围 0~127，对应 OLED 的列位置
  * @retval 无
  */
void OLED_SetPos(uint8_t y, uint8_t x)
{
    // 构造三个命令字节设置 OLED 显示起始地址
    uint8_t pos_buf[3] = {
        0xB0 + y,               	// 设置页地址命令，0xB0 是页地址起始，后加页码
        ((x & 0xF0) >> 4) | 0x10, // 设置列地址高4位 + 0x10 (列地址高4位起始命令)
        (x & 0x0F)              	// 设置列地址低4位
    };

    // 通过写命令方式将位置命令发送给 OLED，3字节
    OLED_WriteBuffer(OLED_WR_CMD, pos_buf, sizeof(pos_buf));
}

/**
  * @brief  填充整个屏幕
  * @param  fill_data: 要填充的数据（0x00 全黑，0xFF 全亮等）
  * @retval 无
  */
void OLED_Fill(uint8_t fill_data)
{
    uint8_t data_buffer_temp[128] = {0};
    // 将数组的128个字节全部设置为 fill_data，准备写入整行数据
    memset(data_buffer_temp, fill_data, 128);

    // OLED共有8页，每页128列，循环逐页写入数据
    for(uint8_t m = 0; m < 8; m++)
    {
        OLED_SetPos(m, 0); // 设置光标位置到第m页第0列
        OLED_WriteBuffer(OLED_WR_DATA, data_buffer_temp, OLED_ARRAY_SIZE(data_buffer_temp)); // 写入整页数据
    }
}

/**
  * @brief  清屏函数
  * @param  无
  * @retval 无
  * @note   调用 OLED_Fill(0x00) 将屏幕所有像素全部关闭，达到清屏效果
  */
void OLED_CLS(void)
{
    OLED_Fill(0x00);  // 填充 0x00，所有像素点灭，清空屏幕显示内容
}

/**
  * @brief  OLED全屏点亮函数
  * @param  无
  * @retval 无
  * @note   调用 OLED_Fill(0xFF) 将屏幕所有像素全部点亮，显示全白
  */
void OLED_FillFull(void)
{
    OLED_Fill(0xFF);  // 填充 0xFF，所有像素点亮，屏幕亮满
}

/**
  * @brief  OLED 初始化
  * @param  无
  * @retval 无
  */
void OLED_Init(void)
{
        HAL_Delay(100);  // 延时保证电源稳定

        while (OLED_CheckDevice(OLED_ADDRESS) != HAL_OK){}

		/* 控制显示 */
		OLED_WriteByte(OLED_WR_CMD,0xAE);//设置显示打开/关闭(AFh/AEh)

		/* 控制内存寻址模式 */
		OLED_WriteByte(OLED_WR_CMD,0x20);//设置内存寻址模式(20h)
		OLED_WriteByte(OLED_WR_CMD,0x02);//00b，水平寻址模式;01b，垂直寻址模式;10b，页面寻址模式(RESET);11，无效

		/* 页起始地址 */ 
		OLED_WriteByte(OLED_WR_CMD,0xB0);//设置页面寻址模式的页面起始地址，0-7(B0-B7)(PAGE0-PAGE7)

		/* COM输出扫描方向 */
		OLED_WriteByte(OLED_WR_CMD,0xA1);//设置左右方向，0xA1正常 0xA0左右反置
		OLED_WriteByte(OLED_WR_CMD,0xC8);//设置上下方向，0xC8正常 0xC0上下反置

		/* 页内列起始地址 */
		OLED_WriteByte(OLED_WR_CMD,0x00);//设置列地址低位0-7
		OLED_WriteByte(OLED_WR_CMD,0x10);//设置列地址高位0-F(10h-1Fh) 列序号=列地址低位*列地址高位(最高位不参与乘积)

		/* 页内行起始地址 */
		OLED_WriteByte(OLED_WR_CMD,0x40);//设置起始行地址

		/* 设置对比度 */
		OLED_WriteByte(OLED_WR_CMD,0x81);//设置对比度控制寄存器(81h)
		OLED_WriteByte(OLED_WR_CMD,0x7f);//0x00~0xff

		/* 设置显示方向 */
		OLED_WriteByte(OLED_WR_CMD,0xA1);//将列地址0映射到SEG0(A0h)/将列地址127映射到SEG0(A1h)
		OLED_WriteByte(OLED_WR_CMD,0xA6);//设置正常显示(A6h)/倒转显示(A7h)

		/* 设置多路复用率 */
		OLED_WriteByte(OLED_WR_CMD,0xA8);//多路复用率(A8h)
		OLED_WriteByte(OLED_WR_CMD,0x3F);//(1 ~ 64)

		/* 全屏显示 */
		OLED_WriteByte(OLED_WR_CMD,0xA4);//设置整个显示打开/关闭(A4恢复到RAM内容显示,输出遵循RAM内容/A5全屏显示,输出忽略RAM内容)

		/*设置显示偏移量*/
		OLED_WriteByte(OLED_WR_CMD,0xd3);//设置显示偏移量(D3h)
		OLED_WriteByte(OLED_WR_CMD,0x00);

		/* 设置显示时钟分频比/振荡器频率 */
		OLED_WriteByte(OLED_WR_CMD,0xD5);//设置显示时钟分频比/振荡器频率
		OLED_WriteByte(OLED_WR_CMD,0xf0);//设定分割比

		/* 设置预充期 */
		OLED_WriteByte(OLED_WR_CMD,0xD9);//设置预充期
		OLED_WriteByte(OLED_WR_CMD,0x22);

		/* 设置com引脚硬件配置 */
		OLED_WriteByte(OLED_WR_CMD,0xDA);//设置com引脚硬件配置
		OLED_WriteByte(OLED_WR_CMD,0x12);

		/* 设置VCOMH取消选择级别 */
		OLED_WriteByte(OLED_WR_CMD,0xDB);//设置VCOMH取消选择级别
		OLED_WriteByte(OLED_WR_CMD,0x20);//0x20,0.77xVcc

		OLED_WriteByte(OLED_WR_CMD,0x8D);//设置DC-DC使能
		OLED_WriteByte(OLED_WR_CMD,0x14);

		/* 显示打开 */
		OLED_WriteByte(OLED_WR_CMD,0xAF);//设置显示打开/关闭(AFh/AEh)

		OLED_CLS();     // 初始化完成后清屏，确保屏幕全黑，清除残留显示内容
		
		//OLED_FillFull(); // 全屏点亮，便于观察OLED显示是否正常工作

}


/**
  * @brief  显示16x16点阵的汉字
  * @param  y: 页地址（0~7），OLED以8像素为一页
  * @param  x: 列地址（0~127）
  * @param  n: 汉字索引号，在汉字库中的序号
  * @param  data_cn: 指向汉字点阵数据的指针（每个汉字32字节）
  * @retval 无
  */
void OLED_ShowChinese(uint8_t y, uint8_t x, uint8_t n, const uint8_t data_cn[][16])
{
    OLED_SetPos(y, x);  // 设置到第一页位置
    for (uint8_t i = 0; i < 16; i++)  // 写入上半部分 16 字节
    {
        OLED_WriteByte(OLED_WR_DATA, data_cn[n * 2][i]);
    }

    OLED_SetPos(y + 1, x);  // 设置到第二页位置
    for (uint8_t i = 0; i < 16; i++)  // 写入下半部分 16 字节
    {
        OLED_WriteByte(OLED_WR_DATA, data_cn[n * 2 + 1][i]);
    }
}



/**
  * @brief  显示16x16点阵的汉字，接口参数为行/列索引
  * @param  line: 0~3，纵向文本行数（每行16像素高，2页）
  * @param  offset: 0~7，横向汉字偏移（每个汉字16像素宽）
  * @param  n: 汉字索引号
  * @retval 无
  */
void OLED_ShowChinese_F16X16(uint8_t line, uint8_t offset, uint8_t n)
{
    OLED_ShowChinese(line * 2, offset * 16, n, Chinese_library_16x16);
}


/**
  * @brief  OLED 显示单个 ASCII 字符
  * @param  y: OLED 页地址（0~7），每页高度为8像素
  * @param  x: OLED 列地址（0~127），字符显示的起始列
  * @param  char_data: 要显示的 ASCII 字符，范围 32~127（可见字符）
  * @param  textsize: 字体大小，支持两种尺寸：
  *                   TEXTSIZE_F6X8 （6x8 点阵）
  *                   TEXTSIZE_F8X16（8x16 点阵）
  * @retval 无
  *
  * @note   字符点阵存储在 ascll_code_6x8 和 ascll_code_8x16 两个数组中，
  *         通过 char_data - 32 计算对应字符的索引位置。
  *         6x8 字体一行显示即可，8x16 字体需要占用两页显示（上下两部分）。
  */
void OLED_ShowChar(uint8_t y, uint8_t x, uint8_t char_data, uint8_t textsize)
{
    uint32_t addr = char_data - 32;  // 计算字符在点阵数组中的索引

    switch(textsize)
    {
        case TEXTSIZE_F6X8:   // 6x8点阵字体，宽6像素，高8像素
            OLED_SetPos(y, x);  // 设置光标位置
            for(uint8_t i = 0; i < 6; i++)  // 依次写入6字节点阵数据
            {
                OLED_WriteByte(OLED_WR_DATA, (uint8_t)ascii_code_6x8[addr][i]);
            }
            break;

        case TEXTSIZE_F8X16:  // 8x16点阵字体，宽8像素，高16像素
            OLED_SetPos(y, x);  // 设置光标位置显示上半部分
            for(uint8_t i = 0; i < 8; i++)  // 写入8字节点阵数据（上半部分）
            {
                OLED_WriteByte(OLED_WR_DATA, (uint8_t)ascii_code_8x16[addr][i]);
            }
            OLED_SetPos(y + 1, x);  // 设置下一页光标显示下半部分
            for(uint8_t i = 0; i < 8; i++)  // 写入剩余8字节点阵数据（下半部分）
            {
                OLED_WriteByte(OLED_WR_DATA, (uint8_t)ascii_code_8x16[addr][i + 8]);
            }
            break;

        default:
            break;
    }
}

/**
  * @brief  显示字符串
  * @param  y: OLED页地址（0~7）
  * @param  x: OLED列地址（0~127）
  * @param  string_data: 指向字符串的指针（以'\0'结束）
  * @param  textsize: 字体大小（6x8或8x16）
  * @retval 无
  */
void OLED_ShowString(uint8_t y, uint8_t x, uint8_t *string_data, uint8_t textsize)
{
    for(uint8_t i = 0; *string_data != '\0'; i++)
    {
				// 逐字符调用 OLED_ShowChar，x坐标根据字符宽度递增，实现横向连续显示
        OLED_ShowChar(y, x + i * textsize, *string_data++, textsize);
    }
}


/**
  * @brief  显示字符串，8x16字体版
  * @param  line: 行号（0~3），每行16像素高（2页）
  * @param  offset: 列偏移（0~15）
  * @param  string_data: 指向字符串的指针
  * @retval 无
  */
void OLED_ShowString_F8X16(uint8_t line, uint8_t offset, uint8_t *string_data)
{
    OLED_ShowString(line * 2, offset * 8, string_data, TEXTSIZE_F8X16);
}

void OLED_ShowStringInvert(uint8_t y, uint8_t x, uint8_t *string_data, uint8_t textsize)
{
    for(uint8_t i = 0; *string_data != '\0'; i++)
    {
        uint32_t addr = *string_data - 32;

        if (textsize == TEXTSIZE_F8X16)
        {
            OLED_SetPos(y, x + i * 8U);
            for (uint8_t j = 0; j < 8U; j++)
            {
                OLED_WriteByte(OLED_WR_DATA,
                               (uint8_t)~ascii_code_8x16[addr][j]);
            }

            OLED_SetPos(y + 1U, x + i * 8U);
            for (uint8_t j = 8U; j < 16U; j++)
            {
                OLED_WriteByte(OLED_WR_DATA,
                               (uint8_t)~ascii_code_8x16[addr][j]);
            }
        }
        else if (textsize == TEXTSIZE_F6X8)
        {
            OLED_SetPos(y, x + i * 6U);
            for (uint8_t j = 0; j < 6U; j++)
            {
                OLED_WriteByte(OLED_WR_DATA,
                               (uint8_t)~ascii_code_6x8[addr][j]);
            }
        }

        string_data++;
    }
}

/**
  * @brief  OLED 显示位图（支持自动对齐与裁剪）
  * @param  x: 起始列坐标（0~127）
  * @param  y: 起始像素行坐标（0~63）
  * @param  width: 图像宽度（像素）
  * @param  height: 图像高度（像素）
  * @param  bitmap: 图像数据指针（每列每页排布）
  * @retval 无
  */
void OLED_DrawBitmap(uint8_t x, uint8_t y, uint8_t width, uint8_t height, const uint8_t *bitmap)
{
    uint8_t page_start, page_count;
    uint8_t x_end;
    uint32_t raw_size;
    uint32_t j = 0;

    // 起始页处理（每页8行，向上对齐）
    if ((y % 8) != 0)
    {
        y = (y / 8 + 1) * 8;
    }
    page_start = y / 8;

    // 计算图像页数，如果高度不是8的倍数，则代表需要额外的一页来显示剩余的像素（如10个像素高，（10+7）/8=2,需要两页）
    page_count = (height + 7) / 8;

    // 列范围裁剪
    if (x >= 128) return;
    if (x + width > 128) width = 128 - x;

    // 行范围裁剪
    if (page_start >= 8) return;
    if (page_start + page_count > 8) page_count = 8 - page_start;

    // 数据总量限制
    raw_size = width * page_count;
    if (raw_size > 1024) raw_size = 1024;

    x_end = x + width;// x_end = 画图的终点列（不包含）

    // 分页写入图像数据
    for (uint8_t page = 0; page < page_count; page++)
    {
        OLED_SetPos(page_start + page, x);  // 设置页+列起始位置

        for (uint8_t col = x; col < x_end; col++)
        {
            OLED_WriteByte(OLED_WR_DATA, bitmap[j++]);
            if (j >= raw_size)
                return;  // 数据发送完毕
        }
    }
}


/*把在oled显示的图片反相*/
void OLED_DrawBitmapInvert(uint8_t x,uint8_t y,uint8_t width,uint8_t height,const uint8_t *bitmap)
{
    uint8_t page_start, page_count;
    uint8_t x_end;
    uint32_t raw_size;
    uint32_t j = 0;

    if ((y % 8) != 0)
    {
        y = (y / 8 + 1) * 8;
    }
    page_start = y / 8;

    page_count = (height + 7) / 8;

    if (x >= 128) return;
    if (x + width > 128) width = 128 - x;

    if (page_start >= 8) return;
    if (page_start + page_count > 8) page_count = 8 - page_start;

    raw_size = width * page_count;
    if (raw_size > 1024) raw_size = 1024;

    x_end = x + width;

    for (uint8_t page = 0; page < page_count; page++)
    {
        OLED_SetPos(page_start + page, x);

        for (uint8_t col = x; col < x_end; col++)
        {
            OLED_WriteByte(OLED_WR_DATA, (uint8_t)~bitmap[j++]);
            if (j >= raw_size)
                return;
        }
    }
}










void OLED_ShowChineseHighlight_F16X16(uint8_t line, uint8_t offset, uint8_t n)
{
    uint8_t white_bg[16];
    uint8_t y = line * 2;
    uint8_t x = offset * 16;

    /* 1. 先把这个 16x16 区域填成白底 */
    memset(white_bg, 0xFF, sizeof(white_bg));

    OLED_SetPos(y, x);// 设置到第一页位置
    OLED_WriteBuffer(OLED_WR_DATA, white_bg, sizeof(white_bg));

    OLED_SetPos(y + 1, x);// 设置到第二页位置
    OLED_WriteBuffer(OLED_WR_DATA, white_bg, sizeof(white_bg));

    /* 2. 再写入反相字形：白底黑字 */
    //
    OLED_SetPos(y, x);// 设置到第一页位置
    for (uint8_t i = 0; i < 16; i++)
    {
        OLED_WriteByte(OLED_WR_DATA, (uint8_t)~Chinese_library_16x16[n * 2][i]);
    }

    OLED_SetPos(y + 1, x);// 设置到第二页位置
    for (uint8_t i = 0; i < 16; i++)
    {
        OLED_WriteByte(OLED_WR_DATA, (uint8_t)~Chinese_library_16x16[n * 2 + 1][i]);
    }
}




/*********************************************END OF FILE*********************************************/

/***************************************New Function************************************************ */

/* ==================== 画圆功能：显存与基础函数 ==================== */

/* 软件显存，画圆时先画到这一块缓冲区，最后一次性发给 OLED */
static uint8_t OLED_GraphicBuffer[8][128];

/**
 * @brief 清空画圆显存
 * @note  画新的一帧前调用
 */
void OLED_Graphic_Clear(void)
{
    memset(OLED_GraphicBuffer, 0, sizeof(OLED_GraphicBuffer));
}

/**
 * @brief 将画圆显存发送到 OLED
 * @note  画完圆之后调用，屏幕才会显示
 */
void OLED_Graphic_Flush(void)
{
    for (uint8_t page = 0; page < 8; page++)
    {
        OLED_SetPos(page, 0);
        OLED_WriteBuffer(OLED_WR_DATA, OLED_GraphicBuffer[page], 128);
    }
}

/**
 * @brief 点亮画圆显存中的一个点
 * @param X 横坐标，范围 0~127
 * @param Y 纵坐标，范围 0~63
 */
static void OLED_Graphic_DrawPoint(int16_t X, int16_t Y)
{
    if (X < 0 || X > 127 || Y < 0 || Y > 63)
    {
        return;
    }

    OLED_GraphicBuffer[Y / 8][X] |= (uint8_t)(0x01U << (Y % 8));
}

/**
 * @brief 画圆
 * @param CX       圆心横坐标
 * @param CY       圆心纵坐标
 * @param Radius   半径
 * @param IsFilled 0 表示空心，1 表示实心
 * @note  画完圆后调用 OLED_Graphic_Flush() 显示
 */
void OLED_DrawCircle(int16_t CX,
                     int16_t CY,
                     uint8_t Radius,
                     uint8_t IsFilled)
{
    int16_t x = 0;
    int16_t y = Radius;
    int16_t d = 1 - Radius;
    int16_t i;

    while (x <= y)
    {
        /* 画圆边界上的八个对称点 */
        OLED_Graphic_DrawPoint(CX + x, CY + y);
        OLED_Graphic_DrawPoint(CX - x, CY + y);
        OLED_Graphic_DrawPoint(CX + x, CY - y);
        OLED_Graphic_DrawPoint(CX - x, CY - y);

        OLED_Graphic_DrawPoint(CX + y, CY + x);
        OLED_Graphic_DrawPoint(CX - y, CY + x);
        OLED_Graphic_DrawPoint(CX + y, CY - x);
        OLED_Graphic_DrawPoint(CX - y, CY - x);

        if (IsFilled)
        {
            /* 实心圆：填充圆内的水平线 */
            for (i = -x; i <= x; i++)
            {
                OLED_Graphic_DrawPoint(CX + i, CY + y);
                OLED_Graphic_DrawPoint(CX + i, CY - y);
            }

            for (i = -y; i <= y; i++)
            {
                OLED_Graphic_DrawPoint(CX + i, CY + x);
                OLED_Graphic_DrawPoint(CX + i, CY - x);
            }
        }

        /* 中点画圆算法的下一步 */
        if (d < 0)
        {
            d += 2 * x + 3;
        }
        else
        {
            d += 2 * (x - y) + 5;
            y--;
        }

        x++;
    }
}

/* ==================== 画圆功能结束 ==================== */

/* ==================== 画椭圆功能开始 ==================== */

/**
 * @brief 画椭圆
 * @param X       椭圆圆心横坐标
 * @param Y       椭圆圆心纵坐标
 * @param A       横向半轴长度
 * @param B       纵向半轴长度
 * @param IsFilled 0 表示空心，1 表示实心
 * @note 画完后调用 OLED_Graphic_Flush() 显示
 */
void OLED_DrawEllipse(int16_t X,
                      int16_t Y,
                      uint8_t A,
                      uint8_t B,
                      uint8_t IsFilled)
{
    int16_t x = 0;
    int16_t y = B;
    int16_t j;
    int16_t a = A;
    int16_t b = B;
    float d1;
    float d2;

    d1 = (float)(b * b) + (float)(a * a) * ((float)-b + 0.5f);

    if (IsFilled)
    {
        for (j = -y; j < y; j++)
        {
            OLED_Graphic_DrawPoint(X, Y + j);
        }
    }

    OLED_Graphic_DrawPoint(X + x, Y + y);
    OLED_Graphic_DrawPoint(X - x, Y - y);
    OLED_Graphic_DrawPoint(X - x, Y + y);
    OLED_Graphic_DrawPoint(X + x, Y - y);

    while ((float)(b * b) * (x + 1) < (float)(a * a) * (y - 0.5f))
    {
        if (d1 <= 0.0f)
        {
            d1 += (float)(b * b) * (2 * x + 3);
        }
        else
        {
            d1 += (float)(b * b) * (2 * x + 3)
                + (float)(a * a) * (-2 * y + 2);
            y--;
        }

        x++;

        if (IsFilled)
        {
            for (j = -y; j < y; j++)
            {
                OLED_Graphic_DrawPoint(X + x, Y + j);
                OLED_Graphic_DrawPoint(X - x, Y + j);
            }
        }

        OLED_Graphic_DrawPoint(X + x, Y + y);
        OLED_Graphic_DrawPoint(X - x, Y - y);
        OLED_Graphic_DrawPoint(X - x, Y + y);
        OLED_Graphic_DrawPoint(X + x, Y - y);
    }

    d2 = (float)(b * b) * (x + 0.5f) * (x + 0.5f)
       + (float)(a * a) * (y - 1) * (y - 1)
       - (float)(a * a) * (b * b);

    while (y > 0)
    {
        if (d2 <= 0.0f)
        {
            d2 += (float)(b * b) * (2 * x + 2)
                + (float)(a * a) * (-2 * y + 3);
            x++;
        }
        else
        {
            d2 += (float)(a * a) * (-2 * y + 3);
        }

        y--;

        if (IsFilled)
        {
            for (j = -y; j < y; j++)
            {
                OLED_Graphic_DrawPoint(X + x, Y + j);
                OLED_Graphic_DrawPoint(X - x, Y + j);
            }
        }

        OLED_Graphic_DrawPoint(X + x, Y + y);
        OLED_Graphic_DrawPoint(X - x, Y - y);
        OLED_Graphic_DrawPoint(X - x, Y + y);
        OLED_Graphic_DrawPoint(X + x, Y - y);
    }
}

/* ==================== 画椭圆功能结束 ==================== */

/* ==================== 图像绘制功能开始 ==================== */

/**
 * @brief 清空软件显存中的指定区域
 * @param X      区域左上角横坐标
 * @param Y      区域左上角纵坐标
 * @param Width  区域宽度
 * @param Height 区域高度
 */
static void OLED_Graphic_ClearArea(int16_t X,
                                   int16_t Y,
                                   uint8_t Width,
                                   uint8_t Height)
{
    int16_t i;
    int16_t j;

    for (j = Y; j < Y + Height; j++)
    {
        for (i = X; i < X + Width; i++)
        {
            if (i >= 0 && i <= 127 && j >= 0 && j <= 63)
            {
                OLED_GraphicBuffer[j / 8][i] &= (uint8_t)~(0x01U << (j % 8));
            }
        }
    }
}

/**
 * @brief 将图像画到软件显存中
 * @param X      图像左上角横坐标
 * @param Y      图像左上角纵坐标
 * @param Width  图像宽度
 * @param Height 图像高度
 * @param Image  图像数据
 * @note 画完后调用 OLED_Graphic_Flush() 显示
 */
void OLED_DrawImageToBuffer(int16_t X,
                            int16_t Y,
                            uint8_t Width,
                            uint8_t Height,
                            const uint8_t *Image)
{
    uint8_t i;
    uint8_t j;
    int16_t Page;
    int16_t Shift;
    int16_t Col;

    if (Width == 0 || Height == 0)
    {
        return;
    }

    /* 先把图像区域清空 */
    OLED_Graphic_ClearArea(X, Y, Width, Height);

    /* 遍历图像涉及的页 */
    for (j = 0; j < (Height - 1) / 8 + 1; j++)
    {
        /* 遍历图像涉及的列 */
        for (i = 0; i < Width; i++)
        {
            Col = X + i;

            if (Col >= 0 && Col <= 127)
            {
                Page = Y / 8;
                Shift = Y % 8;

                if (Y < 0)
                {
                    Page -= 1;
                    Shift += 8;
                }

                if (Page + j >= 0 && Page + j <= 7)
                {
                    /* 图像在当前页的部分 */
                    OLED_GraphicBuffer[Page + j][Col] |=
                        (uint8_t)(Image[j * Width + i] << Shift);
                }

                if (Page + j + 1 >= 0 && Page + j + 1 <= 7)
                {
                    /* 图像跨越到下一页的部分 */
                    OLED_GraphicBuffer[Page + j + 1][Col] |=
                        (uint8_t)(Image[j * Width + i] >> (8 - Shift));
                }
            }
        }
    }
}

/* ==================== 图像绘制功能结束 ==================== */



