/*
  Implements steering and motor control of the Buggy.

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/

#ifndef BUGGY_MOTOR
#define BUGGY_MOTOR

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_attr.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

//You can get these value from the datasheet of servo you use, in general pulse width varies between 1000 to 2000 mocrosecond
#define SERVO_MIN_PULSEWIDTH 1290   // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH 1500   // Maximum pulse width in microsecond
#define SERVO_MED_PULSEWIDTH 1400   // Maximum pulse width in microsecond
#define SERVO_LEFT  1100            // Pulse width corresponding to left turn
#define SERVO_RIGHT 1700            // Pulse width corresponding to left turn
#define SERVO_MID   1400            // Pulse width corresponding to left turn
#define MOTOR_GPIO      32          // ESC GPIO
#define STEERING_GPIO   18          // Steering GPIO

// MCPWM init function //
/**
  \return None
**/
void buggy_mcpwm_init();


// ESC Functions //
/**
  \return None
**/
void calibrateESC();

/**
  \return None
**/
void testESC();

/**
  \param int speed --> sets speed of car in reverse
  \return None
**/
void reverseESC(int speed);

/**
  \param int speed --> sets speed of car in reverse
  \return None
**/
void forwardESC(int speed);

/**
  \return None
**/
void brakeESC();

// Steering functions //
/**
  \param int angle --> sets angle of car in reverse
  \return None
**/
void turnLeft(int angle);

/**
  \param int angle --> sets angle of car in reverse
  \return None
**/
void turnRight(int angle);

/**
  \return None
**/
void goStraight();

/**
  \return None
**/
void testSteering();

#endif
