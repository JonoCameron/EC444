# Code Readme

Code for Quest 5 keeps the car functionality from Skills 30-33.  

Note that some of the code included in this skill comes from example Espressif 
projects. Specifically, ADC1 code adapted from the 
[Espressif ADC1 example project](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/adc). 
The ultrasonic sensor used the 
[esp-idf-lib ultrasonic component](https://github.com/UncleRus/esp-idf-lib/tree/master/components/ultrasonic) 
as a starting point, and the ultrasonic functions were modified to work for my 
sensor. The DC motor code was adapted from the 
[Espressif MCPWM Brushed DC Motor Control example project](https://github.com/espressif/esp-idf/tree/master/examples/peripherals/mcpwm/mcpwm_brushed_dc_control).  

All Node.js code is included in the src/ directory, which contains 3 
subdirectories: library/, main/, and res/. These subdirectories function as 
how they are named. The main/ subdirectory contains the main code for the 
project, which is to setup an express server that handles requests. The 
library/ subdirectory contains any necessary library calls for the project, 
which is canvasjs.min.js, but isn't actually used in the quest. The res/ 
subdirectory contains resource files such as HTML files for client interaction.  


## Sub-Folders: 
* ### src/library/
    * canvasjs.min.js: JavaScript file that defines the functionality for CanvasJS. Not currently used.  
* ### src/main/
    * express_server.js: Node.js module that configures an express server that handles client web requests.  
* ### src/res/  
    * control_car.html: HTML file that enables clients to toggle the car via a button. 
* ### main/  
  * adc1.h: Header file for ADC1 functions.  
  * adc1.c: Implements ADC1 functions.  
  * buggy_motor.h: Header file for Buggy ESC and steering control functions.
  * buggy_motor.c: Implements Buggy ESC and steering control functions.
  * i2c_display.h: Header file for I2C display functions.  
  * i2c_display.c: Implements the I2C display functions.  
  * lidar.h: Header file for LIDAR v4 functions using I2C.  
  * lidar.c: Implements the LIDAR v4 functions using I2C.  
  * mathy_math.h: Header file that defines various math functions (i.e. speed & PID).  
  * mathy_math.c: Implements math functions.  
  * mcpwm_motor.h: Header file for PWM motor control functions.  
  * mcpwm_motor.c: Implements functions to move a PWM motor in various directions.  
  * timer.h: Header file for timer timer interrupt functions.  
  * timer.c: Implements the timer interrupt functions.  
  * udp.h: Header file for UDP client and server functions.  
  * udp.c: Implements UDP client and server functions.  
  * ultrasonic.h: Header file for ultrasonic sensor functions that rely on trigger and echo signals.  
  * ultrasonic.c: Implements ultrasonic sensor functions that use trigger and echo.  
  * wifi_station.h: Header file for functions that establish the ESP as a wifi station.  
  * wifi_station.c: Implements functions that establish the ESP as a wifi station.  
  * main.c: Main file for the purple car project.  
  * CMakeLists.txt: Connects the source files to the project to meet the build requirements.  
  * Kconfig.projbuild: Defines menuconfig parameters for the WAP credentials.  


## Files:  
The remaining files in the code/ directory are used to ensure the project 
properly builds and flashes to the ESP32.  

For more information on the esp-idf template, please refer to the official 
[esp-idf-template](https://github.com/espressif/esp-idf-template) project that 
follows the [Espressif IoT Development Framework](https://github.com/espressif/esp-idf).  

