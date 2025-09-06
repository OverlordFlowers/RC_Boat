#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdint.h>

// This is to enable port C for button interrupts, in order to safely start/stop the RTOS.
// Pin 4 is used to enable the RTOS, while pin 5 is used to disable functions.
void init_buttons_interrupt();




#endif /* BUTTONS_H_*/
