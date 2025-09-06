#include "Drivers/inc/servo.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"

void init_servo(void) {
    // init pwm
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);

    //HWREG(GPIO_PORTB_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    //HWREG(GPIO_PORTB_BASE + GPIO_O_CR) |= 0x01;

    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);
    GPIOPinConfigure(GPIO_PB6_M0PWM0);

    // Sets clock divider to pwm to 8
    SysCtlPWMClockSet(SYSCTL_PWMDIV_8);

    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);
}

void servo_PWM_SetPeriod(uint32_t period_ms) {
    uint32_t clk_freq = SysCtlClockGet();
    //uint32_t pwm_prescaler = SysCtlPWMClockGet();
    uint32_t pwm_prescaler = 8;

    // clk/prescaler * desired_time = register value
    servo_period = ((clk_freq/pwm_prescaler * (period_ms)/1000));

    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, servo_period);
}

// Sets the duty cycle.
void servo_PWM_SetDuty(float duty_cycle) {
    uint32_t duty = servo_period * duty_cycle;
    if (duty == 0) {
        duty = 1;
    }

    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, duty);

    return;
}

void servo_PWM_Enable(void) {
    PWMGenEnable(PWM0_BASE, PWM_GEN_0);
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true);

    return;
}

void servo_neutral(void) {
    //uint32_t duty = (max_servo_pwm_range + min_servo_pwm_range) / 2;
    servo_PWM_SetDuty(avg_servo_pwm);

    return;
}
