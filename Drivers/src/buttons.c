#include "Drivers/inc/buttons.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/interrupt.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/tm4c123gh6pm.h"

// Originally had 2 buttons to enable/disable the boat.
// Actually implementing the boat, was changed to just use one button to toggle it.
void init_buttons_interrupt() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
    GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_4);

    //GPIOPinTypeGPIOInput(GPIO_PORTC_BASE, GPIO_PIN_5);

    //GPIO_PORTC_PUR_R |= (GPIO_PIN_4 | GPIO_PIN_5);
    //GPIO_PORTC_ICR_R |= (GPIO_PIN_4 | GPIO_PIN_5);

    GPIO_PORTC_PUR_R |= (GPIO_PIN_4);
    GPIO_PORTC_ICR_R |= (GPIO_PIN_4);

    // Initialize PC4, PC5 interrupt on falling edge
    GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE);
    GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_4);

    //GPIOIntTypeSet(GPIO_PORTC_BASE, GPIO_PIN_5, GPIO_FALLING_EDGE);
    //GPIOIntEnable(GPIO_PORTC_BASE, GPIO_INT_PIN_5);

    IntEnable(INT_GPIOC);

    return;
}
