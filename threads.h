#ifndef THREADS_H_
#define THREADS_H_

#include <stdbool.h>

#include "Drivers/inc/servo.h"
#include "Drivers/inc/motor.h"
#include "Drivers/inc/HC_SR04_lib.h"

#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"

#include "inc/hw_memmap.h"
#include "inc/hw_memmap.h"
#include "inc/hw_timer.h"
#include "inc/hw_types.h"
#include "inc/tm4c123gh6pm.h"

#include "G8RTOS/G8RTOS.h"

#include "ai_states.h"


#define FIFO_DIST 0
#define FIFO_SERVO_PWM_DUTY 1
#define FIFO_MOTOR_PWM_DUTY 2

// aperiodic threads
void set_trig_low(void);

void echo_read(void);

void master_enable(void);

// periodic threads
void idle_thread(void);

void request_distance(void);

void handle_echo(void);

void handle_distance(void);

void servo_control(void);

void motor_control(void);

#endif /* THREADS_H_ */
