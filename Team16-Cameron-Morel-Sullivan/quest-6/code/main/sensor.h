/*
  Header file for sensor functions (i.e. temperature, ultrasonic, and IR) that 
  rely on ADC. Sparkfun Ultrasonic Range Finder functions adapted from the 
  ESP-IDF Components Library (specifically ultrasonic). 

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Dec. 2020
*/
#ifndef SENSOR_H
#define SENSOR_H


#include <stdio.h>
#include <math.h>
#include <driver/gpio.h>


// Data structure to track a sensor's trigger and echo pins.
typedef struct
{
  gpio_num_t trigger_pin;
  gpio_num_t echo_pin;
} ultrasonic_sensor_t;


/**
  Calculates the temperature of a thermistor from its voltage and default specs.
  \param double voltage_adc --> ADC voltage reading of the voltage divider after the thermistor (in mV).
  \param double voltage_src --> Input voltage to the voltage divider (in mV).
  \param double t0 --> Base temperature value (room temperature) used in the thermistor specs (in K).
  \param double b --> Beta constant for the thermistor.
  \return Current temperature in Kelvin (K) as a double.
**/
double calculate_temperature(double voltage_adc, double voltage_src, double t0, double b);


/**
  Converts a temperature in Kelvin to Celsius.
  \param double temp_K --> Temperature in Kelvin.
  \return Temperature in Celsius.
**/
double kelvin2celsius(double temp_K);


/**
  Converts a temperature in Kelvin to Fahrenheit.
  \param double temp_K --> Temperature in Kelvin.
  \return Temperature in Fahrenheit.
**/
double kelvin2fahrenheit(double temp_K);


/**
  Calculates the range of an ultrasonic sensor (LV-MaxSonar-EZ) from its ADC voltage and specs.
  \param double voltage_adc --> ADC voltage reading from the ultrasonic sensor's AN pin (in mV).
  \param double voltage_src --> Input voltage to the ultrasonic sensor's 5V pin (in mV).
  \param uint32_t adc_width --> Bit width of the ADC.
  \return Distance (inches) away from the back of the sensor's PCB to the target.
**/
double calculate_range_ultrasonic(double voltage_adc, double voltage_src, uint32_t adc_width);


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


/**
  Calculates the range of an IR sensor based on its ADC voltage and specs. Note: our sensor works between 20-150cm.
  \param double voltage_adc --> ADC voltage reading from the IR sensor's voltage output pin (in mV).
  \param double m --> Slope of the sensor's "Output voltage (mV) vs Inverse number of distance (1/cm)" curve.
  \param double b --> y-intercept of the sensor's "Output voltage (mV) vs Inverse number of distance (1/cm)" curve.
  \return Distance (cm) away an object is from the IR sensor.
**/
double calculate_range_ir(double voltage_adc, double m, double b);


/**
  Converts a distance in inches to centimeters.
  \param double distance_in --> Distance in inches.
  \return Distance in centimeters.
**/
double inches2centimeters(double distance_in);


#endif

