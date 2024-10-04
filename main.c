#include <STC89C5xRC.H>
#include <uart.h>
#include <timer.h>
#include <nvic.h>
#include <MFRC522.h>
#include "stdio.h"
#include "system.h"
#include "timer.h"

#define ABS(x) ((x) > 0 ? (x) : -(x))

#define ENCODER_LEFT_A P32
#define ENCODER_LEFT_B P34
#define ENCODER_RIGHT_A P33
#define ENCODER_RIGHT_B P35

#define PWM_TICK_MAX 255
#define PWM_CH0_PIN P25
#define PWM_CH1_PIN P26
#define PWM_CH2_PIN P27
#define SERVO_MID_DUTY 19

#define LMOTOR_A P01
#define LMOTOR_B P00
#define RMOTOR_A P02
#define RMOTOR_B P03

#define TIMER0_VALUE 65475
#define TIMER1_VALUE 55536

uint8_t pdata status;
uint8_t pdata tag_type[2];
uint8_t pdata serial_number[4];

uint8_t data pwm_tick = 0;
uint8_t data pwm_duty_ch0 = 7;
uint8_t data pwm_duty_ch1 = 7;
uint8_t data pwm_duty_ch2 = 7;

int8_t data encoder_left_count = 0; 
int8_t data encoder_right_count = 0;
int8_t pdata encoder_left_speed = 0;
int8_t pdata encoder_right_speed = 0;
int8_t pdata speed_setpoint = 10;
int8_t pdata speed_tick = 0;

int8_t pdata position = 0;
int8_t pdata last_position = 0;

int16_t pdata servo_Kp = 1;
int16_t pdata servo_Ki = 0;
int16_t pdata servo_Kd = 0;
int16_t pdata servo_setpoint = 0;
int16_t pdata servo_error = 0;
int16_t pdata servo_last_error = 0;
int16_t pdata servo_integral = 0;
int16_t pdata servo_derivative = 0;
int16_t pdata servo_output = 0;

int16_t pdata lmotor_Kp = 10;
int16_t pdata lmotor_Ki = 0;
int16_t pdata lmotor_Kd = 3;
#define LMOTOR_SETPOINT_BASE 20
int16_t pdata lmotor_setpoint = 0;
int16_t pdata lmotor_error = 0;
int16_t pdata lmotor_last_error = 0;
int16_t pdata lmotor_integral = 0;
int16_t pdata lmotor_derivative = 0;
int16_t pdata lmotor_output = 0;

int16_t pdata rmotor_Kp = 10;
int16_t pdata rmotor_Ki = 0;
int16_t pdata rmotor_Kd = 3;
#define RMOTOR_SETPOINT_BASE 20
int16_t pdata rmotor_setpoint = 0;
int16_t pdata rmotor_error = 0;
int16_t pdata rmotor_last_error = 0;
int16_t pdata rmotor_integral = 0;
int16_t pdata rmotor_derivative = 0;
int16_t pdata rmotor_output = 0;

static inline void Delay1ms(void)	//@12.000MHz
{
	unsigned char data i, j;

	i = 2;
	j = 239;
	do
	{
		while (--j);
	} while (--i);
}


static inline void encoder_left_init()
{
    EXTI_CONFIG exti_config;
    exti_config.trigger = EXTI_Trigger_Falling;
    exti_config.priority = NVIC_Priority_1;
    exti_init(EXTI_0, &exti_config);  //P32 P12
    exti_cmd(EXTI_0, true);
}

static inline void encoder_right_init()
{
    EXTI_CONFIG exti_config;
    exti_config.trigger = EXTI_Trigger_Falling;
    exti_config.priority = NVIC_Priority_1;
    exti_init(EXTI_1, &exti_config);  //P33 P13
    exti_cmd(EXTI_1, true);
}

void main(){
    // timer declare
    TIM_CONFIG pdata timer;

    // uart declare
    UART_CONFIG pdata uart;
    
    // timer0 config
    timer.mode = TIM_Mode_1;
    timer.function = TIM_Function_Timer;
    timer.enable_int = true;
    timer.priority = NVIC_Priority_1;
    timer.value = TIMER0_VALUE;
    timer_init(TIM_0, &timer);
    timer_cmd(TIM_0, true);
    // timer1 config
    timer.mode = TIM_Mode_1;
    timer.function = TIM_Function_Timer;
    timer.enable_int = true;
    timer.priority = NVIC_Priority_0;
    timer.value = TIMER1_VALUE;
    timer_init(TIM_1, &timer);
    timer_cmd(TIM_1, true);

    // uart config
    uart.baudrate = 19200;
    uart.baud_generator = TIM_2;
    uart.parity = UART_Parity_None;
    uart.priority = NVIC_Priority_0;
    uart.callback = NULL;
    uart_init(&uart);
    EA = 1;
    printf("uart init\r\n");
    PcdReset();
    PcdAntennaOff();
    Delay1ms();
    PcdAntennaOn();
    printf("MFRC init\r\n");
    encoder_left_init();
    encoder_right_init();
    printf("encoder init\r\n");
    while(1){        
        // printf(":%d,%d,%d,%d\r\n",(int16_t)position,(int16_t)servo_output,(int16_t)encoder_left_speed,(int16_t)encoder_right_speed); 
        // status = PcdRequest(PICC_REQALL, tag_type);
        // if (status != MI_OK){
        //     PcdAntennaOff();
        //     Delay1ms();
        //     PcdAntennaOn();
        //     continue;
        // }

        // printf(":%04X",tag_type[0]);

        // status = PcdAnticoll(serial_number);
        // if (status != MI_OK){
        //     continue;    
        // }

        // printf(",%02X,%02X,%02X,%02X\r\n",(uint16_t)serial_number[0],(uint16_t)serial_number[1],(uint16_t)serial_number[2],(uint16_t)serial_number[3]);
        printf(":%d,%d,%d,%d,%d,%d\r\n",(int16_t)lmotor_output,(int16_t)rmotor_output,(int16_t)encoder_left_speed,(int16_t)encoder_right_speed,lmotor_setpoint,rmotor_setpoint);
    }
}

void timer0(void) interrupt 1{
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
    if(pwm_tick < pwm_duty_ch2){
        PWM_CH2_PIN = 1;
    }else{
        PWM_CH2_PIN = 0;
    }
    pwm_tick++;
}

void timer1(void) interrupt 3{
    TH1 = TIMER1_VALUE >> 8;
    TL1 = TIMER1_VALUE;
    // calculate position
    switch (P1)
    {
        // 0000 0000
        case 0x00:
            position = last_position;
            break;
        // 0001 1000
        case 0x18:
            position = 0;
            break;
        // 0001 0000
        case 0x10:
            position = -6;
            break;
        // 0000 1000
        case 0x08:
            position = 6;
            break;
        // 0010 0000
        case 0x20:
            position = -24;
            break;
        // 0000 0100
        case 0x04:
            position = 24;
            break;
        // 0110 0000
        case 0x60:
            position = -26;
            break;
        // 0000 0110
        case 0x06:
            position = 26;
            break;
        // 0100 0000
        case 0x40:
            position = -29;
            break;
        // 0000 0010
        case 0x02:
            position = 29;
            break;
        // 1000 0000
        case 0x80:
            position = -48;
            break;
        // 0000 0001
        case 0x01:
            position = 48;
            break;
        default:
            position = last_position;
            break;
    }
    last_position = position;
    // calculate servo pid
    servo_error = servo_setpoint - position;
    servo_integral += servo_error;
    servo_derivative = servo_error - servo_last_error;
    servo_output = servo_Kp * servo_error + servo_Ki * servo_integral + servo_Kd * servo_derivative;
    servo_last_error = servo_error;
    // set servo pwm duty
    servo_output = servo_output >> 3;
    if(servo_output > 6)servo_output = 6;
    if(servo_output < -6)servo_output = -6;
    pwm_duty_ch0 = SERVO_MID_DUTY - servo_output;
    
    // read speed
    speed_tick++;
    if(speed_tick == 5){
        speed_tick = 0;
        encoder_left_speed = encoder_left_count;
        encoder_right_speed = encoder_right_count;
        encoder_left_count = 0;
        encoder_right_count = 0;
        
        if(servo_output > 0){
            lmotor_setpoint = LMOTOR_SETPOINT_BASE - servo_output * 2;
            rmotor_setpoint = RMOTOR_SETPOINT_BASE;
        } else {
            lmotor_setpoint = LMOTOR_SETPOINT_BASE;
            rmotor_setpoint = RMOTOR_SETPOINT_BASE + servo_output * 2;
        }
        // calculate lmotor pid
        lmotor_error = lmotor_setpoint - encoder_left_speed;
        lmotor_integral += lmotor_error;
        lmotor_derivative = lmotor_error - lmotor_last_error;
        lmotor_output = lmotor_Kp * lmotor_error + lmotor_Ki * lmotor_integral + lmotor_Kd * lmotor_derivative;
        lmotor_last_error = lmotor_error;
        // calculate rmotor pid
        rmotor_error = rmotor_setpoint - encoder_right_speed;
        rmotor_integral += rmotor_error;
        rmotor_derivative = rmotor_error - rmotor_last_error;
        rmotor_output = rmotor_Kp * rmotor_error + rmotor_Ki * rmotor_integral + rmotor_Kd * rmotor_derivative;
        rmotor_last_error = rmotor_error;
        if(lmotor_output > 0){
            LMOTOR_A = 1;
            LMOTOR_B = 0;
        } else {
            LMOTOR_A = 0;
            LMOTOR_B = 1;
        }
        if(lmotor_output > 250)lmotor_output = 250;
        if(lmotor_output < -250)lmotor_output = -250;
        pwm_duty_ch1 = ABS(lmotor_output);
        if(rmotor_output > 0){
            RMOTOR_A = 1;
            RMOTOR_B = 0;
        } else {
            RMOTOR_A = 0;
            RMOTOR_B = 1;
        }
        if(rmotor_output > 250)rmotor_output = 250;
        if(rmotor_output < -250)rmotor_output = -250;
        pwm_duty_ch2 = ABS(rmotor_output);

    }

}

void exint0(void) interrupt 0{
    if(ENCODER_LEFT_B == 1)
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
    if(ENCODER_RIGHT_B == 1)
    {
        encoder_right_count++;
    }
    else
    {
        encoder_right_count--;
    }
}
