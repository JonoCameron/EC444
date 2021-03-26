#ifndef PID_H
#define PID_H

#include <stdio.h>
#include <driver/gpio.h>

int deltaOne;
int deltaTwo;
int D1;
int D2;
int takeD2;
int pidReady;

int PID(int D1, int D2);

void gpio_init();

void statusLEDs(int error);

#endif