/*************************************
 * Created by: Ruo Chen
 * Created on: 11/08/2022
 * Updated on: 11/08/2022
 *
 * This library is for the TM4C123GH6PM microcontroller on the Tiva Launchpad,
 * for it to interface with servo motors.
*************************************/

#ifndef SERVO_LIB_H_
#define SERVO_LIB_H_

#include <stdint.h>
#include <stdbool.h>

// #define max_servo_pwm_range 0.231
// #define min_servo_pwm_range 0.142
#define max_servo_pwm_range 0.22758
#define min_servo_pwm_range 0.15180
#define avg_servo_pwm   ((max_servo_pwm_range + min_servo_pwm_range) / 2.0)

static uint32_t servo_period = 0;

void init_servo(void);

void servo_PWM_SetPeriod(uint32_t period_ms);

void servo_PWM_SetDuty(float duty_cycle);

void servo_PWM_Enable(void);

void servo_neutral(void);

#endif /* SERVO_LIB_H_ */
