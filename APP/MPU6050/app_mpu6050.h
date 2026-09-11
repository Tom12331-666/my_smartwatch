#ifndef APP_MPU6050_H
#define APP_MPU6050_H

#include "main.h"
#include"math.h"


/* MPU6050 I2C 从机地址(AD0 接地时为 0x68,接高电平为 0x69) */
#define MPU6050_ADDR         0x68
#define MPU6050_ADDR_WRITE   (MPU6050_ADDR << 1)   /* HAL 需要 8 位地址,低 1 位为读写位 */

#define MPU6050_IIC_TIMEOUT  1000

/* 寄存器地址 */
#define MPU6050_REG_SMPLRT_DIV    0x19   /* 采样率分频 */
#define MPU6050_REG_CONFIG        0x1A   /* DLPF 数字低通滤波 */
#define MPU6050_REG_GYRO_CONFIG   0x1B   /* 陀螺仪量程 */
#define MPU6050_REG_ACCEL_CONFIG  0x1C   /* 加速度计量程 */
#define MPU6050_REG_ACCEL_XOUT_H  0x3B   /* 加速度数据起始(连续 6 字节) */
#define MPU6050_REG_TEMP_OUT_H    0x41   /* 温度(2 字节) */
#define MPU6050_REG_GYRO_XOUT_H   0x43   /* 角速度数据起始(连续 6 字节) */
#define MPU6050_REG_PWR_MGMT_1    0x6B   /* 电源管理 */
#define MPU6050_REG_WHO_AM_I      0x75   /* 器件 ID,应回读 0x68 */
#define MPU6050_REG_INT_PIN_CFG    0x37   /* 中断引脚配置:高有效/推挽/非锁存 */
#define MPU6050_REG_INT_ENABLE     0x38   /* 中断使能:bit0 = 数据就绪(DATA_RDY) */
#define MPU6050_REG_INT_STATUS     0x3A   /* 中断状态:读它可确认并清除中断源 */

/* 灵敏度换算系数(默认量程 ±2g / ±250°/s) */
#define ACCEL_LSB_PER_G     16384.0f   /* ±2g 时 1g = 16384 LSB */
#define GYRO_LSB_PER_DPS    131.0f     /* ±250°/s 时 1°/s = 131 LSB */
#define TEMP_SLOPE          340.0f     /* 温度:每 1°C = 340 LSB */
#define TEMP_OFFSET         36.53f     /* 温度公式常数 */

typedef struct {
    int16_t accel_raw[3];   /* 原始加速度 X/Y/Z */
    int16_t gyro_raw[3];    /* 原始角速度 X/Y/Z */
    int16_t temp_raw;       /* 原始温度 */
    float   accel_g[3];     /* 换算后加速度,单位 g */
    float   gyro_dps[3];    /* 换算后角速度,单位 °/s */
    float   temp_c;         /* 换算后温度,单位 °C */
} MPU6050_Data;


typedef struct {
    float roll;   /* 横滚角，单位度 */
    float pitch;  /* 俯仰角，单位度 */
    float yaw;    /* 偏航角，单位度 */
} MPU6050_Angle;

void MPU6050_ComplementaryUpdate(MPU6050_Data *data,MPU6050_Angle *angle, float dt);
HAL_StatusTypeDef MPU6050_Init(void);
HAL_StatusTypeDef MPU6050_ReadData(MPU6050_Data *data);
HAL_StatusTypeDef MPU6050_EnableDataReadyInt(void);

#endif // APP_MPU6050_H
