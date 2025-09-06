#include "Drivers/inc/motor.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"

// Sets the pins used to control the motor.
void init_motor(void) {
    // init pwm
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_7);
    GPIOPinConfigure(GPIO_PB7_M0PWM1);

    // Sets clock divider to pwm to 8
    SysCtlPWMClockSet(SYSCTL_PWMDIV_8);

    PWMGenConfigure(PWM0_BASE, PWM_GEN_1, PWM_GEN_MODE_DOWN);
}

void motor_PWM_SetPeriod(uint32_t period_ms) {
    uint32_t clk_freq = SysCtlClockGet();
    //uint32_t pwm_prescaler = SysCtlPWMClockGet();
    uint32_t pwm_prescaler = 8;

    motor_period = ((clk_freq/pwm_prescaler * (period_ms)/1000));

    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, motor_period);
}

void motor_PWM_SetDuty(float duty_cycle) {
    uint32_t duty = motor_period * duty_cycle;
    if (duty == 0) {
        duty = 1;
    }

    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, duty);
}

void motor_PWM_Enable(void) {
    PWMGenEnable(PWM0_BASE, PWM_GEN_1);
    PWMOutputState(PWM0_BASE, PWM_OUT_1_BIT, true);
}

void motor_stop(void) {
    motor_PWM_SetDuty(stall_motor_pwm);
}
