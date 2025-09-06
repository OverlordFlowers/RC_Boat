#include "threads.h"

#include "utils/uartstdio.h"
#include "driverlib/uart.h"

static bool ready_to_read = false;
static bool boat_enable = false;

// After timer times out, sets the trigger pin load to initiate reading.
void set_trig_low(void) {
    TimerDisable(TIMER0_BASE, TIMER_A);
    GPIO_PORTA_DATA_R &= ~(GPIO_PIN_6);
    TIMER0_ICR_R = 0x01;
}

// Reads the echo
void echo_read(void) {
    uint8_t test = GPIO_PORTA_DATA_R;
    if (GPIO_PORTA_DATA_R & GPIO_PIN_7) {
        // enable timer
        TimerEnable(TIMER1_BASE, TIMER_A);
    } else if (~GPIO_PORTA_DATA_R & GPIO_PIN_7) {
        // disable timer
        TimerDisable(TIMER1_BASE, TIMER_A);
        // raise flag to read timer
        ready_to_read = true;
    }
    GPIO_PORTA_ICR_R |= 0x80;
}

// For enable/disabling the RTOS threads
void master_enable(void) {
    // If PC4...
    // Disables take priority
    if (~GPIO_PORTC_DATA_R & GPIO_PIN_4) {
        if (boat_enable) {
            boat_enable = false;
            // kill all threads
            G8RTOS_KillAllThreads();
            // Stops boat
            BOAT_MOVE_STATE = STALL;
            BOAT_TURN_STATE = NEUTRAL;
            motor_stop();
            servo_neutral();
            // Re-add idle thread
            G8RTOS_AddThread(idle_thread, 255, "idle\0");
            IntMasterEnable();
        } else if (!boat_enable) {
            if (!boat_enable) {
                boat_enable = true;
                motor_stop();
                servo_neutral();

                // Re-add threads, re-initialize FIFOs
                G8RTOS_InitFIFO(FIFO_DIST);
                //G8RTOS_InitFIFO(FIFO_SERVO_PWM_DUTY);
                //G8RTOS_InitFIFO(FIFO_MOTOR_PWM_DUTY);
                G8RTOS_AddThread(request_distance, 246, "req_dist\0");
                G8RTOS_AddThread(handle_echo, 245, "get_dist\0");
                G8RTOS_AddThread(handle_distance, 244, "hand_dist\0");
                G8RTOS_AddThread(servo_control, 243, "servo\0");
                G8RTOS_AddThread(motor_control, 242, "motor\0");
            }
        }
    }

    // Clears interrupts
    GPIO_PORTC_ICR_R |= (GPIO_PIN_4);
}

void idle_thread(void) {
    while(1);
}

// Worst-case scenario is a 60 ms response time according to sonar sensor datasheet.
void request_distance(void) {
    while(1) {
        timer_sonar_trigger_start();
        sleep(70);
    }
}

void handle_echo(void) {
    uint32_t timer_val = 0;
    uint32_t timer_val_avg = 0;

    float us_result = 0;
    float cm_measured = 0;

    while(1) {
        if (ready_to_read) {
            // Put this into a critical section?
            timer_val = TIMER1_TAV_R;
            TIMER1_TAV_R = 0;
            TIMER1_TBV_R = 0;
            // Decaying average to prioritize more recently measured values
            timer_val_avg = ((timer_val_avg + timer_val) >> 1);

            // Calculates the microseconds it took.
            us_result = (1.0 * timer_val_avg / SysCtlClockGet()) * 1000000;
            cm_measured = us_result / 58;

            // cm can just be a int value
            // to transfer float value without the compiler attempt to typecase, this code extracts the value directly from the memory address.
            writeFIFO(FIFO_DIST, *(int*)&cm_measured);

            // insert cm_measured into FIFO
            UARTprintf("cm measured: %2d\n", (uint32_t)cm_measured);
            ready_to_read = false;
        }
        sleep(40);
    }
}

/*
 * handle_distance handles the measured distance.
 * Behavior:
 *  Move forward if dist > 50 cm
 *  turn port if dist < 50 cm
 *  keep turning port until dist > 50 cm
 *  reverse if dist < 30 cm
 */

// The reason there is a small in-between distance is so the momentum of the boat can be used to turn.

void handle_distance(void) {
    float cm_measured = 0.0;
    uint32_t read_result = 0;
    //float pwm_servo_duty_neutral = (min_servo_pwm_range + max_servo_pwm_range) / 2;
    //float pwm_servo_range = max_servo_pwm_range - min_servo_pwm_range;

    while(1) {
        read_result = readFIFO(FIFO_DIST);
        memcpy(&cm_measured, &read_result, sizeof(read_result));

        // Account for minimum distance

        if (cm_measured < 30) {
            BOAT_TURN_STATE = STARBOARD;
            BOAT_MOVE_STATE = REVERSE;

        } else if (cm_measured < 50) {
            BOAT_TURN_STATE = PORT;
            BOAT_MOVE_STATE = STALL;
        } else {
            BOAT_TURN_STATE = NEUTRAL;
            BOAT_MOVE_STATE = FORWARD;
        }

        //writeFIFO(FIFO_SERVO_PWM_DUTY, *(int*)&pwm_servo_duty_neutral);

        sleep(30);
    }

}

// Reasoning behind servo_control thread:
// There is no way for the boat to tell what direction it is turned in. Easiest way to do this was to simply have it turn to port (left of the boat bow).
// Person controlling it would have to manually place it in the correct position.
void servo_control(void) {
    //float pwm_servo_duty = 0.0;
    //uint32_t read_result = 0;
    while(1) {
        //read_result = readFIFO(FIFO_SERVO_PWM_DUTY);

        if (BOAT_TURN_STATE == PORT) {
            servo_PWM_SetDuty(max_servo_pwm_range);
        } else if (BOAT_TURN_STATE == STARBOARD) {
            servo_PWM_SetDuty(min_servo_pwm_range);
        } else if (BOAT_TURN_STATE == NEUTRAL) {
            servo_PWM_SetDuty(avg_servo_pwm);
        }
        sleep(100);
    }
}

// The reasoning behind the motor control:
// The motor for the RC boat was too powerful and did not respond well to minute changes in control. Otherwise, I would use a FIFO for the
// thread to read and control it that way.

// Since the motor is being controlled using constants there was no need to use a FIFO- using global states would have been sufficient.
// The motor is pulsed after running (it sleeps for a certain amount of time after setting the duty cycle of the PWM signal) before stalling.
// If the motor was left to constantly run it would accelerate too quickly. At the same time, short bursts were needed to allow the boat to move.
void motor_control(void) {
    //float pwm_motor_duty = 0.0;
    //uint32_t read_result = 0;
    while(1) {
        if (BOAT_MOVE_STATE == FORWARD) {
            motor_PWM_SetDuty(forward_motor_pwm);
            sleep(200);
        } else if (BOAT_MOVE_STATE == REVERSE) {
            motor_PWM_SetDuty(reverse_motor_pwm);
            sleep(500);
        } else if (BOAT_MOVE_STATE == STALL) {
            motor_PWM_SetDuty(stall_motor_pwm);
        }


        motor_PWM_SetDuty(stall_motor_pwm);

        sleep(600);
    }
}
