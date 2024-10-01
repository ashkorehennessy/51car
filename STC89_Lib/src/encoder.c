#include "encoder.h"
#include "exti.h"

int16_t encoder_left_count = 0;
int16_t encoder_right_count = 0;
int16_t encoder_left_speed = 0;
int16_t encoder_right_speed = 0;


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

void encoder_read_left_speed(){
    encoder_left_speed = encoder_left_count;
    encoder_left_count = 0;
}

void encoder_read_right_speed(){
    encoder_right_speed = encoder_right_count;
    encoder_right_count = 0;
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

