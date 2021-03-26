/*
  Header file that defines various math functions.

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/
#ifndef MATHYMATH_H
#define MATHYMATH_H


#include <stdint.h>


// Parameters
#define PI 3.14
#define KP 30    // Proportional scale factor
#define KI 0.5   // Integral scale factor
#define KD 0.5   // Difference scale factor
#define dt 0.5     // Set dt for 1s

/**
  Calculates a wheel's speed (m/s).
  \param double d --> Diameter of the wheel (in centimeters).
  \param uint32_t tickCount --> Number of ticks recorded in a second.
  \param uint32_t totalTicks --> Total number of ticks on the encoder.
**/
double calc_speed(double d, uint32_t tickCount, uint32_t totalTicks);

/**
  Calculates the error output according to PID. Used for adjusting the setting of speed, distance, etc.
  \param double measurement --> Current measurement to run PID on.
  \param double setpoint --> Setpoint contstant (ideal value) for the given measurement.
  \param double* prev_error --> Pointer to the previous error value.
  \param double* integral --> Pointer to the integral value.
  \param double* derivative --> Pointer to the derivative value.
  \return Output of the PID error calculation as a double.
**/
double pid_control(double measurement, double setpoint, double* prev_error, double* integral, double* derivative);



#endif


