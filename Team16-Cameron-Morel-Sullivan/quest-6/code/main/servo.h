/*
  Header file for servo functions. Adapted from Espressif mcpwm_servo_control example project.  
  --> https://github.com/espressif/esp-idf/tree/master/examples/peripherals/mcpwm/mcpwm_servo_control  

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Dec. 2020
*/
#ifndef SERVO_H
#define SERVO_H


#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"


// You can get these value from the datasheet of servo you use.
// In general, pulse width varies between 1000 to 2000 microseconds.
// Note: You likely need to adjust the min and max pulsewidth to match your servo.
#define SERVO_MIN_PULSEWIDTH 400   // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 2600  // Maximum pulse width in microsecond
#define SERVO_MAX_DEGREE     180   // Maximum angle in degrees in which the servo can rotate


/**
  Use this function to calcute pulse width for per degree rotation.
  \param uint32_t degree_of_rotation --> Angle in degrees to which servo has to rotate.
  \return Calculated pulse width.
**/
uint32_t servo_per_degree_init(uint32_t degree_of_rotation);


/**
  Initializes servo MCPWM GPIO.
  \param uint32_t servo_gpio --> GPIO pin to set as PWM0A for sending data to the servo.
  \return None
**/
void initialize_servo(uint32_t servo_gpio);


/**
  Configures MCPWM module, and rotates servo to target degree. Requires an initialize_servo() call beforehand.
  \param uint32_t degree --> What degree position to move the servo to.
  \param uint32_t delay_ms --> Delay for the rotation in milliseconds (recommended 100ms per 60degrees at 5V).
  \return 0 on success, or -1 if invalid degree argument.
**/
int servo_rotateTo(uint32_t degree, uint32_t delay_ms);


#endif
