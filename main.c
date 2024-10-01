#include <STC89C5xRC.H>
#include <uart.h>
#include <timer.h>
#include <nvic.h>
#include <MFRC522.h>
#include "stdio.h"
unsigned char g_ucTempbuf[20];
void main(){
    unsigned char status;
    unsigned int temp;
    unsigned char i;
    TIM_NUM tim = TIM_1;
    NVIC_PRIORITY priority = NVIC_Priority_1;
    UART_CONFIG uart;
    uart.baudrate = 4800;
    uart.baud_generator = tim;
    uart.parity = UART_Parity_None;
    uart.priority = priority;
    uart.callback = NULL;
    uart_init(&uart);
    printf("uart init\r\n");
    PcdReset();
    PcdAntennaOff();
    PcdAntennaOn();
    printf("MFRC init\r\n");
    while(1){
        status = PcdRequest(PICC_REQALL, g_ucTempbuf);//寻卡
        if (status != MI_OK){
            PcdReset();
            PcdAntennaOff();
            PcdAntennaOn();
            continue;
        }
        printf("card type:");
        for(i=0;i<2;i++) {
            temp=g_ucTempbuf[i];
            printf("%02X ",temp);
        }
    
        status = PcdAnticoll(g_ucTempbuf);//防冲撞
        if (status != MI_OK){
            continue;    
        }
    

        printf("   serial:");
        for(i=0;i<4;i++)
        {
            temp=g_ucTempbuf[i];
            printf("%02X ",temp);
        }
        printf("\r\n");
    }
}
