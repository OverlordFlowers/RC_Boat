/*************************************
 * Created by: Ruo Chen
 * Created on: 11/08/2022
 * Updated on: 11/08/2022
 *
 * This library is for the TM4C123GH6PM microcontroller on the Tiva Launchpad,
 * for it to interface with servo motors.
*************************************/

#ifndef MOTOR_LIB_H_
#define MOTOR_LIB_H_

// #define max_motor_pwm_range 0.214 (forward)
// #define min_motor_pwm_range 0.152 (backwards)

#include <stdint.h>
#include <stdbool.h>

#define max_motor_pwm_range 0.21394
#define min_motor_pwm_range 0.15180

#define forward_motor_pwm 0.195
#define stall_motor_pwm 0.18
#define reverse_motor_pwm 0.16

static uint32_t motor_period = 0;

// Initializes the motor, sets the pins needed to be used.
void init_motor(void);

// Sets the period for the PWM in ms.
void motor_PWM_SetPeriod(uint32_t period_ms);

// Sets the duty cycle of the PWM as a percentage.
void motor_PWM_SetDuty(float duty_cycle);

// Enables the PWM.
void motor_PWM_Enable(void);

// Stops the motor.
void motor_stop(void);

#endif /* MOTOR_LIB_H_ */
