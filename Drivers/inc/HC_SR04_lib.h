/*************************************
 * Created by: Ruo Chen
 * Created on: 11/04/2022
 * Updated on: 11/04/2022
 *
 * This library is for the TM4C123GH6PM microcontroller on the Tiva Launchpad,
 * for it to interface with the HC-SR04 ultrasonic proximity sensor.
*************************************/
#include <stdint.h>
#include <stdbool.h>

#ifndef HC_SR04_LIB_H_
#define HC_SR04_LIB_H_

void init_sonar(void);

void timer_sonar_trigger_init(void);

void timer_sonar_trigger_start(void);

void timer_sonar_read_init(void);

#endif /* HC_SR04_LIB_H_ */
