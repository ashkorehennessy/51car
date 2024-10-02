#include <STC89C5xRC.H>
#include <uart.h>
#include <timer.h>
#include <nvic.h>
#include <MFRC522.h>
#include "stdio.h"
#include "system.h"
#include "timer.h"

#define ENCODER_LEFT_A P32
#define ENCODER_LEFT_B P12
#define ENCODER_RIGHT_A P33
#define ENCODER_RIGHT_B P13

#define PWM_TICK_MAX 100
#define PWM_CH0_PIN P10
#define PWM_CH1_PIN P11
#define PWM_CH2_PIN P12
#define PWM_CH3_PIN P13

#define TIMER0_VALUE 65363

uint8_t status;
uint8_t tag_type[2];
uint8_t serial_number[4];
uint8_t pwm_duty_ch0 = 17;
uint8_t pwm_duty_ch1 = 17;
uint8_t pwm_duty_ch2 = 0;
uint8_t pwm_duty_ch3 = 0;

int8_t encoder_left_count = 0;
int8_t encoder_right_count = 0;
int8_t encoder_left_speed = 0;
int8_t encoder_right_speed = 0;


void encoder_left_init()
{
    EXTI_CONFIG exti_config;
    exti_config.trigger = EXTI_Trigger_Falling;
    exti_config.priority = NVIC_Priority_1;
    exti_init(EXTI_0, &exti_config);  //P32 P12
    exti_cmd(EXTI_0, true);
}

void encoder_right_init()
{
    EXTI_CONFIG exti_config;
    exti_config.trigger = EXTI_Trigger_Falling;
    exti_config.priority = NVIC_Priority_1;
    exti_init(EXTI_1, &exti_config);  //P33 P13
    exti_cmd(EXTI_1, true);
}

void main(){
    // timer declare
    TIM_MODE mode = TIM_Mode_1;
    TIM_FUNCTION function = TIM_Function_Timer;
    NVIC_PRIORITY tim_priority = NVIC_Priority_1;
    TIM_CONFIG timer;

    // uart declare
    TIM_NUM uart_tim = TIM_1;
    NVIC_PRIORITY uart_priority = NVIC_Priority_0;
    UART_CONFIG uart;
    
    // timer config
    timer.mode = mode;
    timer.function = function;
    timer.enable_int = true;
    timer.priority = tim_priority;
    timer.value = TIMER0_VALUE;
    timer_init(TIM_0, &timer);
    timer_cmd(TIM_0, true);

    // uart config
    uart.baudrate = 4800;
    uart.baud_generator = uart_tim;
    uart.parity = UART_Parity_None;
    uart.priority = uart_priority;
    uart.callback = NULL;
    uart_init(&uart);
    EA = 1;
    printf("uart init\r\n");
    PcdReset();
    PcdAntennaOff();
    PcdAntennaOn();
    printf("MFRC init\r\n");
    encoder_left_init();
    encoder_right_init();
    printf("encoder init\r\n");
    while(1){        
        printf(":%d,%d\r\n",(int16_t)encoder_left_speed,(int16_t)encoder_right_speed);
        status = PcdRequest(PICC_REQALL, tag_type);//寻卡
        if (status != MI_OK){
            PcdReset();
            PcdAntennaOff();
            PcdAntennaOn();
            continue;
        }

        printf(":%04X",tag_type[0]);

        status = PcdAnticoll(serial_number);//防冲撞

        if (status != MI_OK){
            continue;    
        }

        printf(",%02X,%02X,%02X,%02X\r\n",(uint16_t)serial_number[0],(uint16_t)serial_number[1],(uint16_t)serial_number[2],(uint16_t)serial_number[3]);
    }
}

void timer0(void) interrupt 1{
    static uint8_t pwm_tick = 0;
    TH0 = TIMER0_VALUE >> 8;
    TL0 = TIMER0_VALUE;
    if(pwm_tick < pwm_duty_ch0){
        PWM_CH0_PIN = 1;
    }else{
        PWM_CH0_PIN = 0;
    }
    if(pwm_tick < pwm_duty_ch1){
        PWM_CH1_PIN = 1;
    }else{
        PWM_CH1_PIN = 0;
    }
    pwm_tick++;
    if(pwm_tick == PWM_TICK_MAX){
        pwm_tick = 0;
        encoder_left_speed = encoder_left_count;
        encoder_right_speed = encoder_right_count;
        encoder_left_count = 0;
        encoder_right_count = 0;
    }
}

void exint0(void) interrupt 0{
    if(ENCODER_LEFT_B == 0)
    {
        encoder_left_count++;
    }
    else
    {
        encoder_left_count--;
    }
}

void exint1(void) interrupt 2
{
    if(ENCODER_RIGHT_B == 0)
    {
        encoder_right_count++;
    }
    else
    {
        encoder_right_count--;
    }
}
