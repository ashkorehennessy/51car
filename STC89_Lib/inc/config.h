#ifndef __CONFIG_H__
#define __CONFIG_H__

/* 控制引脚定义，使用 I/O 来模拟 SPI 接口 */
/* 用户可根据所使用的板卡资源进行修改     */
/* 请注意板卡上的引脚是否复用到其它模块   */

sbit  MF522_NSS = P1^7;
sbit  MF522_SCK = P1^6;
sbit  MF522_SI  = P1^5;
sbit  MF522_SO  = P1^4;

/* 使用 【MFRC522-PA】 模块时，+5V 工作的 MCU 不能接复位控制引脚 */
/* 因为该模块的复位电路上有二极管 1N4148，当 MCU 的复位管件输出为高时，影响到模块的电源 */
sbit     MF522_RST  = P2^0;   

/* 指示灯控制引脚，可不使用 */
sbit     LED_GREEN = P0^0;


// 外部晶振频率, 单位是Hz
#define FOSC 11059200L
// 执行每条指令所花费的时钟周期数, 传统51单片机为12T, STC89支持12T和6T, 更新的单片机支持1T, 烧录时不要忘记在软件中修改
#define CPI 12

// 是否编译电源管理模块
#define COMPILE_POWER 1
// 是否编译看门狗模块
#define COMPILE_WDT 1
// 是否编译gpio模块
#define COMPILE_GPIO 0
// 是否编译外部中断模块
#define COMPILE_EXTI 1
// 是否编译定时器模块
#define COMPILE_TIMER 1
// 是否编译通用异步收发器模块
#define COMPILE_UART 1
// 是否编译内存扩展模块
#define COMPILE_MEM 0
// 是否编译ISP模块
#define COMPILE_ISP 0
// 是否编译软件PWM模块
#define COMPILE_SOFT_PWM 0
// 是否编译软件I2C模块
#define COMPILE_SOFT_I2C 0
// 是否编译软件SPI模块
#define COMPILE_SOFT_SPI 0

#if COMPILE_UART == 1
// 串口接收缓冲区大小
#define UART_BUFFER_SIZE 15
// 用串口发送实现putchar
#define IMPLEMENT_STDIO_PUT 1
// 用串口接收实现getchar
#define IMPLEMENT_STDIO_GET 0
#endif

// 详见soft_pwm.h
#if COMPILE_SOFT_PWM == 1
#define PWM_PIN P30
#define PWM_TIMER 0
#endif

// 详见soft_i2c.h
#if COMPILE_SOFT_I2C == 1
#define I2C_SCL P20
#define I2C_SDA P21
#endif

// 详见soft_spi.h
#if COMPILE_SOFT_SPI == 1
#define SPI_SCLK P27
#define SPI_MOSI P26
#define SPI_MISO P26
#define SPI_CS   P25
#define SPI_MODE 0
#endif

#endif // __CONFIG_H__
