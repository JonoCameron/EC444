/*
  Function implementation for sensor functions. Ultrasonic functions modified 
  from ESP-IDF Components Library.

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Dec. 2020
*/

#include "sensor.h"
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>


// Define some parameters
#define TRIGGER_LOW_DELAY 4  // 4us delay
#define TRIGGER_HI_DELAY 10  // 10us delay for the active trigger pulse
#define PING_TIMEOUT 1000    // Timeout for measuring an echo (was 6000)
#define ROUNDTRIP 58         // Constant to multiply by the time to determine the distance traveled


double calculate_temperature(double voltage_adc, double voltage_src, double t0, double b)
{
  double invT = (double)1/t0 + (double)1/b * log( ((voltage_src / voltage_adc) - 1 ) );

  return (double)1/invT;
}



double kelvin2celsius(double temp_K)
{
  return (temp_K - 273.15);
}



double kelvin2fahrenheit(double temp_K)
{
  return ((temp_K - 273.15) * (double)9/(double)5 + 32);
}



double calculate_range_ultrasonic(double voltage_adc, double voltage_src, uint32_t adc_width)
{
  // Calculate the volts per inch (scaling)
  double vpi = voltage_src / (2^adc_width);

  // Calculate the range (inches) with the voltage scaling
  return (voltage_adc / vpi);
}



void ultrasonic_init(const ultrasonic_sensor_t* dev)
{
  // Assumes the dev object already defines the pins

  // Set the trigger pin as an output
  gpio_set_direction(dev->trigger_pin, GPIO_MODE_OUTPUT);

  // Set the echo pin as an input
  gpio_set_direction(dev->echo_pin, GPIO_MODE_INPUT);

  // Set the trigger pin to low/off
  gpio_set_level(dev->trigger_pin, 0);
}



double ultrasonic_measure_distance(const ultrasonic_sensor_t* dev, uint32_t max_distance)
{
  // Set the trigger pin low for 4us
  gpio_set_level(dev->trigger_pin, 0);
  ets_delay_us(TRIGGER_LOW_DELAY);

  // Send a 10us impulse ping
  gpio_set_level(dev->trigger_pin, 1);
  ets_delay_us(TRIGGER_HI_DELAY);

  // Set the trigger pin to low again
  gpio_set_level(dev->trigger_pin, 0);

  // Make sure the previous ping ended
  if (gpio_get_level(dev->echo_pin))
  {
    // Echo pin is already triggered, so recording a result would be inaccurate
    return -1;
  }

  // Set a timer and wait for the echo
  int64_t start = esp_timer_get_time();
  while ( !gpio_get_level(dev->echo_pin) )
  {
    // Check if we meet the timeout
    int64_t current = esp_timer_get_time();
    if ( (current - start) >= PING_TIMEOUT )
    {
      // Failed to get a response, so return a negative distance as an error
      return -2;
    }
  }

  // Began to receive an echo, so record the current time
  int64_t echo_start = esp_timer_get_time();
  int64_t echo_end = echo_start;  // Initialize the return time (will be updated later)

  // Estimate how long the echo timeout should be
  int64_t echo_timeout = echo_start + max_distance * ROUNDTRIP;

  // Wait until the echo signal becomes low again
  while ( gpio_get_level(dev->echo_pin) )
  {
    // Update the echo's end time
    echo_end = esp_timer_get_time();

    // Check if we meet the echo timeout
    if ( (echo_end - echo_start) >= echo_timeout )
    {
      // Failed to have the echo signal end on time, so return a negative distance as an error
      return -3;
    }
  }

  // The echo signal ended, and we have the return time. Return the distance.
  return ( (double)(echo_end - echo_start) / ROUNDTRIP );
}



double calculate_range_ir(double voltage_adc, double m, double b)
{
  // Uses the IR sensor's values from the "Output voltage vs Inverse number of distance" curve
  // y = mx + b
  //   y --> Output voltage
  //   x --> Inverse number of distance
  //   m --> Slope of the curve
  //   b --> y-intercept
  double inv_range = (voltage_adc - b) / m;

  // Take the inverse of inv_range to get the range in centimeters
  return (1.0 / inv_range);
}



double inches2centimeters(double distance_in)
{
  return (distance_in * 2.54);
}

