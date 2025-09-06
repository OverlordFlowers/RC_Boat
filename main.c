/****************************INCLUDES**********************/
#include "Drivers/inc/servo.h"
#include "Drivers/inc/motor.h"
#include "Drivers/inc/HC_SR04_lib.h"
#include "Drivers/inc/buttons.h"

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

#include "threads.h"
#include "G8RTOS/G8RTOS.h"
#include "driverlib/uart.h"

/*************************END OF INCLUDES******************/

/****************************DEFINES***********************/
/*************************END OF DEFINES*******************/

/**
 * main.c
 */
int main(void)
{
    G8RTOS_Init();
    init_servo();
    // set as 8 ms period
    servo_PWM_SetPeriod(8);
    // neutral position
    servo_PWM_SetDuty(avg_servo_pwm);
    servo_PWM_Enable();

    init_motor();
    // set as 8 ms period
    motor_PWM_SetPeriod(8);
    // stop
    motor_PWM_SetDuty(stall_motor_pwm);
    motor_PWM_Enable();

    init_buttons_interrupt();

    G8RTOS_AddAPeriodicEvent(set_trig_low, 6, 35);
    G8RTOS_AddAPeriodicEvent(echo_read, 5, 16);
    // This interrupt has the highest priority.
    G8RTOS_AddAPeriodicEvent(master_enable, 4, 18);

    G8RTOS_AddThread(idle_thread, 255, "idle\0");

    G8RTOS_InitFIFO(FIFO_DIST);

    // Unused in current state
    G8RTOS_InitFIFO(FIFO_SERVO_PWM_DUTY);
    G8RTOS_InitFIFO(FIFO_MOTOR_PWM_DUTY);

    init_sonar();
    timer_sonar_trigger_init();
    timer_sonar_read_init();
    //sleepClockEnables();
    //InitConsole();
    G8RTOS_Launch();

    while(1);
}

void InitConsole(void)
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); // 1

    // Step 1a: Configure RX function of GPIO pin PA0 3
    GPIOPinConfigure(GPIO_PA0_U0RX);

    // Step 1b: Configure TX function of GPIO pin PA1 3
    GPIOPinConfigure(GPIO_PA1_U0TX);

    // Step 2: Enable peripheral (SYSCTL_PERIPH_UART0)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC); // clock set 2
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1); // following step 1 5
    UARTStdioConfig(0, 115200, 16000000); // console config 4
}

void sleepClockEnables() {
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_TIMER0);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOA);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_GPIOB);
    SysCtlPeripheralSleepEnable(SYSCTL_PERIPH_PWM0);
}
