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


// Two interrupts: One where the echo voltage goes high, another for when it goes low.
// 10 us trigger; use a timer to keep track?
// Can just borrow timer code from labs

// 24 cm minimum distance able to be measured
// uS / 58 = cm
// thread should just run every 100 ms, 60 ms response time from echo depending on distance
// echo going high MUST be an interrupt

// Lower clock speed for lower power consumption?

void init_sonar(void) {
    // Init GPIO port
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    // Use port A?
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_6);

    // Configure one pin to both edge interrupt

    GPIOPinTypeGPIOInput(GPIO_PORTA_BASE, GPIO_PIN_7);

    GPIO_PORTA_PUR_R |= 0x80;
    GPIO_PORTA_ICR_R |= 0x80;

    // Initialize PA7 interrupt on falling edge
    GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_7, GPIO_BOTH_EDGES);
    GPIOIntEnable(GPIO_PORTA_BASE, GPIO_INT_PIN_7);

    IntEnable(INT_GPIOA);

    return;
}

void timer_sonar_trigger_init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerDisable(TIMER0_BASE, TIMER_A);
    // Configure timer as 16-bit timer, one-shot
    TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_ONE_SHOT);

    // No prescaler needed.
    TimerPrescaleSet(TIMER0_BASE, TIMER_A, 0);

    // Clears related interrupt.
    TIMER0_ICR_R = 0x01;
    //Enables interrupts on timeout
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    IntEnable(INT_TIMER0A);

}

void timer_sonar_trigger_start(void) {
    // Requests read by setting trigger high
    GPIO_PORTA_DATA_R |= 0b01000000;
    // Minimum time needed for signal to be high in order to request data form the sonar.
    TimerLoadSet(TIMER0_BASE, TIMER_A, 1600);
    // Enable timer.
    TIMER0_CTL_R |= 0x00000001;
}

// Sets timer to count the amount of time needed for the sonar to be read
void timer_sonar_read_init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    TimerConfigure(TIMER1_BASE, TIMER_CFG_A_ONE_SHOT_UP);
    TimerPrescaleMatchSet(TIMER1_BASE, TIMER_A, 8);
}
