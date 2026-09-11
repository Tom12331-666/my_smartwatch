#include "app_mpu6050.h"
#include "i2c.h"        /* 使用工程已配置的硬件 I2C1(hi2c1) */

#define RAD_TO_DEG 57.295779513082f // 弧度转角度的系数： π/180
#define FILTER_GAIN    0.98f // 滤波系数,取值范围 0~1,值越大,滤波效果越明显

/* ============ 底层:I2C 总线读写,整个驱动只有这里直接碰硬件 ============ */

/**
  * @brief  写 MPU6050 的单个寄存器
  * @param  reg : 寄存器地址,如 0x6B(PWR_MGMT_1)
  * @param  data: 要写入的值
  * @retval HAL_OK 成功,其他为失败
  */
static HAL_StatusTypeDef MPU6050_WriteReg(uint8_t reg, uint8_t data)
{
    return HAL_I2C_Mem_Write(&hi2c2, MPU6050_ADDR_WRITE, reg,
                             I2C_MEMADD_SIZE_8BIT, &data, 1, MPU6050_IIC_TIMEOUT);
}

/**
  * @brief  从 MPU6050 的 reg 寄存器开始,连续读 len 个字节
  * @param  reg : 起始寄存器地址,如 0x3B(加速度数据起始)
  * @param  buf : 存放读回数据的数组指针
  * @param  len : 要读的字节数,如 14(加速度6 + 温度2 + 角速度6)
  * @retval HAL_OK 成功,其他为失败
  */
static HAL_StatusTypeDef MPU6050_ReadRegs(uint8_t reg, uint8_t *buf, uint16_t len)
{
    return HAL_I2C_Mem_Read(&hi2c2, MPU6050_ADDR_WRITE, reg,
                            I2C_MEMADD_SIZE_8BIT, buf, len, MPU6050_IIC_TIMEOUT);
}



/* ============ 中层:芯片初始化和数据读取 ============ */

/**
  * @brief  MPU6050 初始化
  * @retval HAL_OK 成功;HAL_ERROR 器件不存在或 I2C 通信失败
  */
HAL_StatusTypeDef MPU6050_Init(void)
{
    uint8_t who_am_i = 0;

    /* 1. 读 WHO_AM_I:确认 I2C 通路正常(寄存器地址 0x75)
          兼容片(如 MPU6500)的 ID 是 0x70 而非 0x68,故只验证通信、不再比对值 */
    if (MPU6050_ReadRegs(MPU6050_REG_WHO_AM_I, &who_am_i, 1) != HAL_OK)
        return HAL_ERROR;

    /* 2. 电源管理:退出休眠,时钟源选 PLL(X 轴陀螺),比内部 8MHz 更稳 */
    if (MPU6050_WriteReg(MPU6050_REG_PWR_MGMT_1, 0x01) != HAL_OK)
        return HAL_ERROR;

    /* 3. 采样率 = 1kHz / (1+4) = 200Hz */
    MPU6050_WriteReg(MPU6050_REG_SMPLRT_DIV, 0x04);

    /* 4. DLPF 数字低通滤波带宽 44Hz,抑制高频噪声 */
    MPU6050_WriteReg(MPU6050_REG_CONFIG, 0x03);

    /* 5. 陀螺仪量程 ±250°/s(灵敏度 131 LSB/(°/s),对应 GYRO_LSB_PER_DPS) */
    MPU6050_WriteReg(MPU6050_REG_GYRO_CONFIG, 0x00);

    /* 6. 加速度计量程 ±2g(灵敏度 16384 LSB/g,对应 ACCEL_LSB_PER_G) */
    MPU6050_WriteReg(MPU6050_REG_ACCEL_CONFIG, 0x00);

    return HAL_OK;
}

/**
  * @brief  读取加速度、温度、角速度并换算为物理量
  * @param  data: 输出结构体指针,由调用者创建
  * @retval HAL_OK 成功;HAL_ERROR I2C 通信失败
  */
HAL_StatusTypeDef MPU6050_ReadData(MPU6050_Data *data)
{
    uint8_t buf[14];
    int16_t raw[7];
    int i;

    /* 从 0x3B 一次连读 14 字节:加速度6 + 温度2 + 角速度6 */
    if (MPU6050_ReadRegs(MPU6050_REG_ACCEL_XOUT_H, buf, 14) != HAL_OK)
        return HAL_ERROR;

    /* 大端字节序:高字节在前,拼成 16 位有符号数 */
    for (i = 0; i < 7; i++)
        raw[i] = (int16_t)((buf[i * 2] << 8) | buf[i * 2 + 1]);

    /* 按顺序填进结构体(加速度X/Y/Z → 温度 → 角速度X/Y/Z) */
    data->accel_raw[0] = raw[0];
    data->accel_raw[1] = raw[1];
    data->accel_raw[2] = raw[2];
    data->temp_raw     = raw[3];
    data->gyro_raw[0]  = raw[4];
    data->gyro_raw[1]  = raw[5];
    data->gyro_raw[2]  = raw[6];

    /* 换算为物理量 */
    for (i = 0; i < 3; i++) {
        data->accel_g[i]  = data->accel_raw[i] / ACCEL_LSB_PER_G;
        data->gyro_dps[i] = data->gyro_raw[i] / GYRO_LSB_PER_DPS;
    }
    data->temp_c = data->temp_raw / TEMP_SLOPE + TEMP_OFFSET;

    return HAL_OK;
}

/**
  * @brief  使能"数据就绪"中断:每采好一帧新数据,INT 引脚发一个 50us 高脉冲
  * @note   INT 引脚需接到 MCU 的一个 EXTI 引脚(本工程为 PA1,上升沿触发)
  * @retval HAL_OK 成功
  */
HAL_StatusTypeDef MPU6050_EnableDataReadyInt(void)
{
    /* 1. INT 引脚保持默认配置:推挽输出、高电平有效、非锁存(50us 脉冲) */
    MPU6050_WriteReg(MPU6050_REG_INT_PIN_CFG, 0x00);

    /* 2. 打开"数据就绪"中断源:INT_ENABLE(0x38) 写 0x01 = DATA_RDY_EN
          (InvenSense 官方 eMPL 驱动定义) */
    return MPU6050_WriteReg(MPU6050_REG_INT_ENABLE, 0x01);
}




void MPU6050_ComplementaryUpdate(MPU6050_Data *data,MPU6050_Angle *angle, float dt)
{

    float ax = data->accel_g[0];
    float ay = data->accel_g[1];
    float az = data->accel_g[2];

    float gx = data->gyro_dps[0];
    float gy = data->gyro_dps[1];
    float gz = data->gyro_dps[2];

    /* 1. 由加速度计计算角度，单位度 */
    float acc_roll  = atan2f(ay, az) * RAD_TO_DEG;
    float acc_pitch = atan2f(-ax,sqrtf(ay * ay + az * az)) * RAD_TO_DEG;

    /* 2. 由陀螺仪积分角度 */
    float gyro_roll  = angle->roll  + gx * dt;
    float gyro_pitch = angle->pitch + gy * dt;
    float gyro_yaw  = angle->yaw  + (float)gz * dt;

    /* 3. 互补融合 */
    angle->roll  = FILTER_GAIN * gyro_roll  + (1.0f - FILTER_GAIN) * acc_roll;
    angle->pitch = FILTER_GAIN * gyro_pitch + (1.0f - FILTER_GAIN) * acc_pitch;
    angle->yaw   = gyro_yaw;

    if (angle->roll > 180.0f)  angle->roll  -= 360.0f;
    if (angle->roll < -180.0f) angle->roll  += 360.0f;
    if (angle->pitch > 90.0f)  angle->pitch  = 90.0f;
    if (angle->pitch < -90.0f) angle->pitch  = -90.0f;
}






