/*
  Implements various math functions.

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/
#include "mathy_math.h"



double calc_speed(double d, uint32_t tickCount, uint32_t totalTicks)
{
  // Calculate the circumference (in meters) from the diameter (in centimeters)
  double c = PI * d / 100;

  // Calculate the time scale from dt
  double t_scale = 1.0 / dt;

  // Calculate the rotations per second
  double rps = ((double) tickCount) / ((double) totalTicks) * t_scale;

  // Return the speed in m/s
  return (c * rps);
}



double pid_control(double measurement, double setpoint, double* prev_error, double* integral, double* derivative)
{
  // Calculate the error according to calculations from the proportional, integral, and differential methods
  double error = setpoint - measurement;
  (*integral) = (*integral) + error * dt;
  (*derivative) = (error - (*prev_error)) / dt;

  // Record the error as prev_error
  (*prev_error) = error;

  // Use the calculated error measurements to determine the output
  double output = KP * error + KI * (*integral) + KD * (*derivative);

  return output;
}


