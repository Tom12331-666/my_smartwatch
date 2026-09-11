# 日期时间设置功能设计思路

## 1. 先想用户流程

这个功能不是一上来就写代码，而是先想用户怎么操作：

```text
主页
 -> 选择“设置”
 -> 进入设置页
 -> 选择“日期时间设置”
 -> 进入日期时间设置页
 -> 修改年月日时分秒
 -> 保存
 -> 回到设置页
```

所以第一件事是定义页面状态：

```text
UI_CLOCK        = 主页
UI_SETTING      = 设置页
UI_DATETIME_SET = 日期时间设置页
```

这样 `Menu_HandleKey()` 才知道按 KEY3 时应该进入哪个页面。

## 2. 定义需要保存的数据

进入日期时间设置页以后，需要记住用户正在编辑的值：

```c
static uint16_t edit_year = 2026;
static uint8_t edit_month = 8;
static uint8_t edit_day = 15;
static uint8_t edit_hour = 15;
static uint8_t edit_min = 30;
static uint8_t edit_sec = 0;
```

为什么不用 RTC 里的值直接改？

因为如果每按一次键就写一次 RTC，用户还没调整完，RTC 已经被改了很多次，而且也不容易撤销。所以把“正在编辑的值”和“RTC 最终值”分开：

```text
edit_year 等 = 用户正在编辑的临时值
RTC        = 保存后的最终值
```

## 3. 定义当前正在修改的字段

只有年月日时分秒还不够，还要知道当前高亮的是哪个字段：

```c
typedef enum {
    FIELD_YEAR,
    FIELD_MONTH,
    FIELD_DAY,
    FIELD_HOUR,
    FIELD_MIN,
    FIELD_SEC
} edit_field_t;
```

作用：

```text
current_field = FIELD_YEAR  -> 正在改年
current_field = FIELD_MONTH -> 正在改月
...
```

这样 KEY1/KEY2 知道给哪个变量加减，显示函数也知道该反相哪一段数字。

## 4. 按键规则

```text
KEY1：当前字段减 1
KEY2：当前字段加 1
KEY3：切换到下一个字段
最后一个字段再按 KEY3：保存
```

对应代码：

```c
uint8_t SetTime_HandleKey(uint8_t key)
{
    switch (key)
    {
        case 1:
            change_current_value(-1);
            draw_settime_ui();
            break;

        case 2:
            change_current_value(1);
            draw_settime_ui();
            break;

        case 3:
            if (current_field == FIELD_SEC)
            {
                save_and_write_rtc();
                return 1U;
            }
            current_field++;
            draw_settime_ui();
            break;
    }
}
```

思考顺序：

```text
先判断按了什么键
再判断当前改哪个字段
最后决定是“改值”还是“切字段”还是“保存”
```

## 5. 显示函数

显示函数不负责修改数据，它只负责把当前数据画到屏幕上：

```c
draw_settime_ui()
{
    清屏；
    画日期；
    画时间；
    画当前字段名；
    画按键提示；
    反相当前选中字段；
}
```

重要原则：

```text
先修改数据
再刷新显示
```

所以 KEY1/KEY2 都先调用：

```c
change_current_value()
```

然后再调用：

```c
draw_settime_ui()
```

## 6. 修改数值和边界

```text
年：2000~2099
月：1~12
日：1~当月最大天数
时：0~23
分：0~59
秒：0~59
```

对应逻辑：

```c
if (delta > 0 && edit_month < 12)
{
    edit_month++;
}
else if (delta < 0 && edit_month > 1)
{
    edit_month--;
}
```

这样可以避免用户把月份改成 0 或 13。

## 7. 保存逻辑

保存是最后一步，不在每次按键时执行：

```text
把 edit 变量转成 RTC 结构体
        ↓
写入 HAL_RTC_SetTime
        ↓
写入 HAL_RTC_SetDate
        ↓
写入 BKP 备份寄存器
```

保存函数只负责写 RTC，不修改用户看到的界面。

## 8. 整体流程

```text
进入设置页
   -> SetTime_Enter()
      -> 读取 RTC
      -> 复制到 edit 变量
      -> 画页面

按 KEY1/KEY2
   -> change_current_value()
   -> draw_settime_ui()

按 KEY3
   -> 如果没到 SEC：current_field++
   -> 如果到了 SEC：save_and_write_rtc()

保存完成
   -> 返回 1
   -> menu.c 切换回 UI_SETTING
```

## 9. 以后自己写类似功能的顺序

1. 先画用户流程。
2. 再定义页面状态。
3. 再定义要保存的数据。
4. 再定义按键规则。
5. 再写显示函数。
6. 再写数据修改函数。
7. 最后写保存函数。

不要在脑子还很乱的时候直接写代码。先用中文把流程写清楚，再把每一句中文翻译成一个函数。
