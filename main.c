#include <STC89C5xRC.H>
#include <MFRC522.h>
#include <INTRINS.H>
#include "stdio.h"

#define FOSC 12000000L

#define uint8_t unsigned char
#define uint16_t unsigned int
#define int8_t char
#define int16_t int
#define int32_t long
#define uint32_t unsigned long

#define ABS(x) ((x) > 0 ? (x) : -(x))

#define ENCODER_LEFT_A P32
#define ENCODER_LEFT_B P34
#define ENCODER_RIGHT_A P33
#define ENCODER_RIGHT_B P35

#define PWM_TICK_MAX 255
#define PWM_CH0_PIN P25
#define PWM_CH1_PIN P26
#define PWM_CH2_PIN P27
#define SERVO_MID_DUTY 18

#define LED_STATION0 P20
#define LED_STATION1 P22
#define LED_STATION2 P24
#define MUSIC_STATION0 P21
#define MUSIC_STATION1 P23
#define MUSIC_STATION2 MUSIC_STATION0 = MUSIC_STATION1

#define LMOTOR_A P01
#define LMOTOR_B P00
#define RMOTOR_A P02
#define RMOTOR_B P03

#define BAUDRATE 19200
#define TIMER0_VALUE 65470
#define TIMER1_VALUE 55535
#define TIMER2_VALUE 65535 - (FOSC / 32 / BAUDRATE)

uint16_t data uptime = 0;

uint8_t idata status;
uint8_t idata tag_type[2];
uint8_t idata serial_number[4];

uint8_t data pwm_tick = 0;
uint8_t data pwm_duty_ch0 = 7;
uint8_t data pwm_duty_ch1 = 7;
uint8_t data pwm_duty_ch2 = 7;

int8_t data encoder_left_count = 0; 
int8_t data encoder_right_count = 0;
int8_t data encoder_left_speed = 0;
int8_t data encoder_right_speed = 0;
int8_t data speed_tick = 0;

int8_t data position = 0;
int8_t data last_position = 0;

int16_t idata servo_Kp = 1;
int16_t idata servo_Ki = 0;
int16_t idata servo_Kd = 6;
int16_t idata servo_setpoint = 0;
int16_t idata servo_error = 0;
int16_t idata servo_last_error = 0;
int16_t idata servo_integral = 0;
int16_t idata servo_derivative = 0;
int16_t idata servo_output = 0;

int16_t idata lmotor_Kp = 5;
int16_t idata lmotor_Ki = 15;
int16_t idata lmotor_Kd = 3;
int16_t idata LMOTOR_SETPOINT_BASE = 5;
int16_t idata lmotor_setpoint = 0;
int16_t idata lmotor_error = 0;
int16_t idata lmotor_last_error = 0;
int16_t idata lmotor_last_last_error = 0;
int16_t idata lmotor_increment = 0;
int16_t idata lmotor_derivative = 0;
int16_t idata lmotor_output = 0;

int16_t idata rmotor_Kp = 5;
int16_t idata rmotor_Ki = 15;
int16_t idata rmotor_Kd = 3;
int16_t idata RMOTOR_SETPOINT_BASE = 5;
int16_t idata rmotor_setpoint = 0;
int16_t idata rmotor_error = 0;
int16_t idata rmotor_last_error = 0;
int16_t idata rmotor_last_last_error = 0;
int16_t idata rmotor_increment = 0;
int16_t idata rmotor_derivative = 0;
int16_t idata rmotor_output = 0;

int8_t idata task_index = 0;
int8_t idata flag_stop = 0;
int8_t idata flag_end = 0;
uint16_t idata timestamp = 0;
uint16_t idata timestamp_led = 0;

bit busy;
#define NONE_PARITY     0   //None parity
#define ODD_PARITY      1   //Odd parity
#define EVEN_PARITY     2   //Even parity
#define MARK_PARITY     3   //Mark parity
#define SPACE_PARITY    4   //Space parity

#define PARITYBIT NONE_PARITY   //Testing even parity
void Delay1ms(void)	//@12.000MHz
{
	unsigned char data i, j;

	i = 2;
	j = 239;
	do
	{
		while (--j);
	} while (--i);
}

void uart_send_char(char c)
{
    while (busy);
    busy = 1;
    SBUF = c;
}

void uart_send(char *str)
{
    while (*str)
    {
        uart_send_char(*str++);
    }
}

char putchar(char c)
{
    if (c == '\n')
        uart_send_char('\r');
    uart_send_char(c);
    return c;
}

void Delay1000ms(void)	//@12.000MHz
{
	unsigned char data i, j, k;

	_nop_();
	i = 8;
	j = 154;
	k = 122;
	do
	{
		do
		{
			while (--k);
		} while (--j);
	} while (--i);
}

void encoder_left_init()
{
    IT0 = 1;
    EX0 = 1;
}

void encoder_right_init()
{
    IT1 = 1;
    EX1 = 1;
}

void timer0_init()
{
    TMOD |= 0x01;
    TH0 = TIMER0_VALUE >> 8;
    TL0 = TIMER0_VALUE;
    TF0 = 0;
    ET0 = 1;
    TR0 = 1;
    PT0 = 1;
}

void timer1_init()
{
    TMOD |= 0x10;
    TH1 = TIMER1_VALUE >> 8;
    TL1 = TIMER1_VALUE;
    TF1 = 0;
    ET1 = 1;
    TR1 = 1;
}

void uart_init(){
    TL2 = RCAP2L= TIMER2_VALUE;
    TH2 = RCAP2H= TIMER2_VALUE >> 8;
    SM1 = 1;
    REN = 1;
    ES = 1;

    RCLK = 1;
    TCLK = 1;
    TR2 = 1;
    ET2 = 1;
}

void main(){
    // timer declare
    // TIM_CONFIG idata timer;
    
    // // timer0 config
    // timer.mode = TIM_Mode_1;
    // timer.function = TIM_Function_Timer;
    // timer.enable_int = true;
    // timer.priority = NVIC_Priority_1;
    // timer.value = TIMER0_VALUE;
    // timer_init(TIM_0, &timer);
    // timer_cmd(TIM_0, true);
    // // timer1 config
    // timer.mode = TIM_Mode_1;
    // timer.function = TIM_Function_Timer;
    // timer.enable_int = true;
    // timer.priority = NVIC_Priority_0;
    // timer.value = TIMER1_VALUE;
    // timer_init(TIM_1, &timer);
    // timer_cmd(TIM_1, true);
    encoder_left_init();
    encoder_right_init();
    timer0_init();
    timer1_init();
    uart_init();

    EA = 1;
    printf("uart init\r\n");
    PcdReset();
    PcdAntennaOff();
    Delay1ms();
    PcdAntennaOn();
    printf("MFRC init\r\n");
    while(1){
        printf(":%d,%d,%d,%d,%d,%d\n",(int16_t)lmotor_output,(int16_t)rmotor_output,(int16_t)encoder_left_speed,(int16_t)encoder_right_speed,lmotor_setpoint,rmotor_setpoint);
        PcdAntennaOff();
        Delay1ms();
        PcdAntennaOn();
        status = PcdRequest(PICC_REQALL, tag_type);
        if (status != MI_OK){
            PcdAntennaOff();
            Delay1ms();
            PcdAntennaOn();
            continue;
        }

        printf(":%04X",tag_type[0]);

        status = PcdAnticoll(serial_number);
        if (status != MI_OK){
            continue;    
        }
        if(serial_number[0] == 0xD3 && serial_number[1] == 0x28 && serial_number[2] == 0x67 && serial_number[3] == 0x05){
            timestamp_led = uptime;
            MUSIC_STATION0 = 0;
            LED_STATION0 = 0;
            flag_stop = 1;
            Delay1000ms();
            timestamp_led = uptime;
            flag_stop = 0;
            LED_STATION0 = 0;
            Delay1000ms();
        }
        if(serial_number[0] == 0x13 && serial_number[1] == 0xE5 && serial_number[2] == 0x92 && serial_number[3] == 0x05){
            timestamp_led = uptime;
            MUSIC_STATION1 = 0;
            LED_STATION1 = 0;
            flag_stop = 1;
            Delay1000ms();
            timestamp_led = uptime;
            flag_stop = 0;
            LED_STATION1 = 0;
            Delay1000ms();
        }
        if(serial_number[0] == 0xB3 && serial_number[1] == 0x66 && serial_number[2] == 0xDA && serial_number[3] == 0xE4){
            timestamp_led = uptime;
            MUSIC_STATION2 = 0;
            LED_STATION2 = 0;
            flag_stop = 1;
            Delay1000ms();
            timestamp_led = uptime;
            flag_stop = 0;
            LED_STATION2 = 0;
            Delay1000ms();
        }

        // printf(",%02X,%02X,%02X,%02X\r\n",(uint16_t)serial_number[0],(uint16_t)serial_number[1],(uint16_t)serial_number[2],(uint16_t)serial_number[3]);
    }
}

void timer0_isr(void) interrupt 1{
    TL0 = TIMER0_VALUE;
    TH0 = TIMER0_VALUE >> 8;
    if(pwm_tick < pwm_duty_ch0){
        PWM_CH0_PIN = 1;
    }else{
        PWM_CH0_PIN = 0;
    }
    // if(pwm_tick < pwm_duty_ch1){
    //     PWM_CH1_PIN = 1;
    // }else{
    //     PWM_CH1_PIN = 0;
    // }
    // if(pwm_tick < pwm_duty_ch2){
    //     PWM_CH2_PIN = 1;
    // }else{
    //     PWM_CH2_PIN = 0;
    // }
    pwm_tick++;
}

void timer1_isr(void) interrupt 3{
    TH1 = TIMER1_VALUE >> 8;
    TL1 = TIMER1_VALUE;
    uptime += 10;
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
            position = -8;
            break;
        // 0000 1000
        case 0x08:
            position = 8;
            break;
        // 0010 0000
        case 0x20:
            position = -17;
            break;
        // 0000 0100
        case 0x04:
            position = 17;
            break;
        // 0110 0000
        case 0x60:
            position = -19;
            break;
        // 0000 0110
        case 0x06:
            position = 19;
            break;
        // 0100 0000
        case 0x40:
            position = -23;
            break;
        // 0000 0010
        case 0x02:
            position = 23;
            break;
        // 1000 0000
        case 0x80:
            position = -42;
            break;
        // 0000 0001
        case 0x01:
            position = 42;
            break;
        // 1111 1111
        case 0xFF:
            flag_end = 1;
            flag_stop = 1;
            break;
        // 0111 1111
        case 0x7F:
            flag_end = 1;
            flag_stop = 1;
            break;
        // 1111 1110
        case 0xFE:
            flag_end = 1;
            flag_stop = 1;
            break;
        // 0111 1110
        case 0x7E:
            flag_end = 1;
            flag_stop = 1;
            break; 
        default:
            position = last_position;
            break;
    }
    last_position = position;
    // task switch
    switch (task_index){
        case 0:
            if(flag_end == 1){
                task_index = 1;
                timestamp = uptime;
                timestamp_led = uptime;
                LED_STATION0 = 0;
                LED_STATION1 = 0;
                LED_STATION2 = 0;
            }
            break;
        case 1:
            if(uptime - timestamp > 5000){
                task_index = 2;
                flag_stop = 0;
                timestamp = uptime;
                lmotor_error = 0;
                lmotor_last_error = 0;
                lmotor_last_last_error = 0;
                lmotor_output = 0;
                rmotor_error = 0;
                rmotor_last_error = 0;
                rmotor_last_last_error = 0;
                rmotor_output = 0;
            } else {
                position = 0;
            }
            break;
        case 2:
            if(uptime - timestamp > 700){
                task_index = 3;
                timestamp = uptime;
            } else {
                flag_stop = 0;
            }
            break;
        case 3:
            if(uptime - timestamp > 1000){
                task_index = 4;
                timestamp = uptime;
            }
            break;
    }
    // led control
    if(uptime - timestamp_led > 1000){
        LED_STATION0 = 1;
        LED_STATION1 = 1;
        LED_STATION2 = 1;
        MUSIC_STATION0 = 1;
        MUSIC_STATION1 = 1;
        MUSIC_STATION2 = 1;
    }
    // calculate servo pid
    servo_error = servo_setpoint - position;
    servo_integral += servo_error;
    servo_derivative = servo_error - servo_last_error;
    servo_output = servo_Kp * servo_error + servo_Ki * servo_integral + servo_Kd * servo_derivative;
    servo_last_error = servo_error;
    // set servo pwm duty
    servo_output = servo_output >> 3;
    if(servo_output < -6)servo_output = -6;
    if(servo_output > 0)servo_output = (servo_output * 5) / 3;
    if(servo_output > 6)servo_output = 6;
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
            lmotor_setpoint = LMOTOR_SETPOINT_BASE - (servo_output >> 1);
            rmotor_setpoint = RMOTOR_SETPOINT_BASE;
        } else {
            lmotor_setpoint = LMOTOR_SETPOINT_BASE;
            rmotor_setpoint = RMOTOR_SETPOINT_BASE + (servo_output >> 1);
        }
        if(lmotor_setpoint < 0)lmotor_setpoint = 0;
        if(rmotor_setpoint < 0)rmotor_setpoint = 0;
        // calculate lmotor pid
        lmotor_last_last_error = lmotor_last_error;
        lmotor_last_error = lmotor_error;
        lmotor_error = lmotor_setpoint - encoder_left_speed;
        lmotor_derivative = (lmotor_error - 2 * lmotor_last_error + lmotor_last_last_error);
        lmotor_increment = lmotor_Kp * (lmotor_error - lmotor_last_error) + lmotor_Ki * lmotor_error + lmotor_Kd * lmotor_derivative;
        lmotor_output += lmotor_increment;
        // calculate rmotor pid
        rmotor_last_last_error = rmotor_last_error;
        rmotor_last_error = rmotor_error;
        rmotor_error = rmotor_setpoint - encoder_right_speed;
        rmotor_derivative = (rmotor_error - 2 * rmotor_last_error + rmotor_last_last_error);
        rmotor_increment = rmotor_Kp * (rmotor_error - rmotor_last_error) + rmotor_Ki * rmotor_error + rmotor_Kd * rmotor_derivative;
        rmotor_output += rmotor_increment;

        if(lmotor_output > 255)lmotor_output = 255;
        if(lmotor_output < -255)lmotor_output = -255;
        if(rmotor_output > 255)rmotor_output = 255;
        if(rmotor_output < -255)rmotor_output = -255;
        if(lmotor_output > 0){
            LMOTOR_A = 1;
            LMOTOR_B = 0;
        } else {
            LMOTOR_A = 0;
            LMOTOR_B = 1;
        }
        if(rmotor_output > 0){
            RMOTOR_A = 1;
            RMOTOR_B = 0;
        } else {
            RMOTOR_A = 0;
            RMOTOR_B = 1;
        }
        if(flag_stop == 0){
            if(flag_end == 1){
                LMOTOR_SETPOINT_BASE = 2;
                RMOTOR_SETPOINT_BASE = 2;
            } else {
                LMOTOR_SETPOINT_BASE = 5;
                RMOTOR_SETPOINT_BASE = 5;
            }
        } else {
            LMOTOR_SETPOINT_BASE = 0;
            RMOTOR_SETPOINT_BASE = 0;
        }
        pwm_duty_ch1 = ABS(lmotor_output);
        pwm_duty_ch2 = ABS(rmotor_output);
    }
}

void ex0_isr(void) interrupt 0{
    if(ENCODER_LEFT_B == 0)
    {
        encoder_left_count++;
    }
    else
    {
        encoder_left_count--;
    }
}

void ex1_isr(void) interrupt 2
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

void uart_isr(void) interrupt 4
{
    if(TI == 1){
        TI = 0;
        busy = 0;
    }
}