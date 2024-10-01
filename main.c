#include <STC89C5xRC.H>
#include <uart.h>
#include <timer.h>
#include <nvic.h>
#include <MFRC522.h>
#include "stdio.h"
#include "encoder.h"
#include "system.h"
#include "timer.h"
uint8_t tag_type[2];
uint8_t serial_number[4];

extern int16_t encoder_left_speed;
extern int16_t encoder_right_speed;

void mfrc_func(){
    uint8_t status;
    status = PcdRequest(PICC_REQALL, tag_type);//寻卡
    if (status != MI_OK){
        PcdReset();
        PcdAntennaOff();
        PcdAntennaOn();
        return;
    }

    printf("card type:%04X",tag_type[0]);

    status = PcdAnticoll(serial_number);//防冲撞

    if (status != MI_OK){
        return;    
    }

    printf("   serial:%02X %02X %02X %02X\r\n",(uint16_t)serial_number[0],(uint16_t)serial_number[1],(uint16_t)serial_number[2],(uint16_t)serial_number[3]);
}
void main(){
    TIM_MODE mode = TIM_Mode_1;
    TIM_FUNCTION function = TIM_Function_Timer;
    NVIC_PRIORITY tim_priority = NVIC_Priority_1;
    TIM_CONFIG timer;
    TIM_NUM uart_tim = TIM_1;
    NVIC_PRIORITY uart_priority = NVIC_Priority_0;
    UART_CONFIG uart;
    
    // timer config
    timer.mode = mode;
    timer.function = function;
    timer.enable_int = true;
    timer.priority = tim_priority;
    timer.value = 45536;
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
    // PcdReset();
    // PcdAntennaOff();
    // PcdAntennaOn();
    // printf("MFRC init\r\n");
    encoder_left_init();
    encoder_right_init();
    printf("encoder init\r\n");
    while(1){
        printf(":%d,%d\r\n",encoder_left_speed,encoder_right_speed);
    }
}

TIMER0{
    timer_set_value(TIM_0, 45536);
    encoder_read_left_speed();
    encoder_read_right_speed();
}
