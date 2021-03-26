# Code Readme  

Code included in quest-1 serves to provide functionality for a Fish Feeder. The 
feeder uses a countdown timer that displays the remaining time until the next 
feeding on an I2C display. When the timer reaches 0, the servo activates to 
rotate 180 degrees to dump food out of a feeding container, and then returns 
back to its original position at 0 degrees.


## Sub-Folders: 
- ### main:
    - main.c: main project file, contains all code for the functionality of the program
    - timer.c: implements the timer interrupt functions for use in main.c
    - servo.c: implements the servo control functions for use in main.c
    - i2c_display.c: implements the I2C initialization and control functions for use in main.c
    - servo.h, timer.h, i2c_display.h: exports all necessary functions and variables from their corresponding .c files
    - CMakeLists.txt: sets the main project file and build files/requirements


## Files:
- All files in the /code main directory are used to make sure the program is able to properly build and flash to the ESP32 board (auto-configured from the template)  

For more information on the esp-idf template, please refer to the official [esp-idf-template](https://github.com/espressif/esp-idf-template) 
project that follows the [Espressif IoT Development Framework](https://github.com/espressif/esp-idf).

