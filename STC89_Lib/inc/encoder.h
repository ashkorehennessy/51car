#ifndef __ENCODER_H
#define __ENCODER_H

#define ENCODER_LEFT_A P32
#define ENCODER_LEFT_B P12
#define ENCODER_RIGHT_A P33
#define ENCODER_RIGHT_B P13


void encoder_left_init();

void encoder_right_init();

void encoder_read_left_speed();

void encoder_read_right_speed();

#endif // __ENCODER_H
