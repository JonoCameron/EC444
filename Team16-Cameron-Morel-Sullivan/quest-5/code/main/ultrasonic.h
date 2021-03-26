/*
  Header file to support Sparkfun Ultrasonic Range Finder sensor. Code adpated 
  from the ESP-IDF Components Library.  
  --> https://github.com/UncleRus/esp-idf-lib/tree/master/components/ultrasonic  

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
  Ruslan V. Uss, 2018
*/
#ifndef ULTRASONIC_H
#define ULTRASONIC_H


#include <driver/gpio.h>


typedef struct
{
  gpio_num_t trigger_pin;
  gpio_num_t echo_pin;
} ultrasonic_sensor_t;


/**
  Initializes the ultrasonic range module.
  \param ultrasonic_sensor_t* dev --> Pointer to the device descriptor.
  \return None
**/
void ultrasonic_init(const ultrasonic_sensor_t* dev);


/**
  Measures distance an object is from the Ultrasonic Range Finder sensor in centimeters.
  \param ultrasonic_sensor_t* dev --> Pointer to the device descriptor.
  \param uint32_t max_distance --> Maximum distance the sensor can measure (in cm).
  \return Distance from the sensor (cm) on success, -1 on busy echo, -2 on trigger timeout, or -3 on echo timeout.
**/
double ultrasonic_measure_distance(const ultrasonic_sensor_t* dev, uint32_t max_distance);


#endif

