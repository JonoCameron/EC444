#include "./pid.h"

#define BLUE 12
#define GREEN 27
#define RED 33

int deltaOne = 0;
int deltaTwo = 0;
int D1 = 0;
int D2 = 0;
int takeD2 = 0;
int pidReady = 0;

int PID(int deltaOne, int deltaTwo){
    D1 = 0;
    D2 = 0;
    int delta = (deltaOne - deltaTwo);
    return delta;
}

void gpio_init(){
    gpio_reset_pin(BLUE);
    gpio_reset_pin(GREEN);
    gpio_reset_pin(RED);
    gpio_set_direction(BLUE, GPIO_MODE_OUTPUT);
    gpio_set_direction(GREEN, GPIO_MODE_OUTPUT);
    gpio_set_direction(RED, GPIO_MODE_OUTPUT);
}

void statusLEDs(int error){
    if(error > 1){
        // blue
        gpio_set_level(BLUE, 1);
        gpio_set_level(GREEN, 0);
        gpio_set_level(RED, 0);
    }
    else if(error <= 1 && error >= -1){
        // green
        gpio_set_level(BLUE, 0);
        gpio_set_level(GREEN, 1);
        gpio_set_level(RED, 0);
    }
    else if(error < -1){
        // red
        gpio_set_level(BLUE, 0);
        gpio_set_level(GREEN, 0);
        gpio_set_level(RED, 1);
    }
}