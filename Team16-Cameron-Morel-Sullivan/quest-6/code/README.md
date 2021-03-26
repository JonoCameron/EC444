# Code Readme

Code for Quest 6 combines some of the features and libraries used in previous 
quests in order to build our LIDAR Radar.  

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
    * Chart.min.js: JavaScript file that defines the functionality for ChartsJS. Not currently used.  
* ### src/main/
    * express_server.js: Node.js module that configures an express server that handles client web requests.  
* ### src/res/  
    * radar.html: HTML file that displays radar data and enables user remote control over the dish ESP. 
* ### main/  
    * adc1.h: Header file for ADC1 functions.  
    * adc1.c: Implements ADC1 functions.  
    * ADXL343.h: Header file that contains information for ADXL343 accelerometer I2C configurations.  
    * button_intr.h: Header file for hardware (button) interrupts.  
    * button_intr.c: Implements hardware (button) interrupt functions.  
    * i2c.h: Header file that defines all I2C functions (accelerometer, LIDAR, accelerometer).  
    * i2c.c: Implements the I2C functions (accelerometer, LIDAR, accelerometer).  
    * sensor.h: Header file for various sensor functions (i.e. temperature, ultrasonic, IR, etc).  
    * sensor.c: Implements the various sensor functions. Mainly handles ADC voltage to unit conversions.  
    * servo.h: Header file for servo functions.  
    * servo.c: Implements servo functions.  
    * timer.h: Header file for timer timer interrupt functions.  
    * timer.c: Implements the timer interrupt functions.  
    * udp.h: Header file for UDP client and server functions.  
    * udp.c: Implements UDP client and server functions.  
    * wifi_station.h: Header file for functions that establish the ESP as a wifi station.  
    * wifi_station.c: Implements functions that establish the ESP as a wifi station.  
    * main.c: Main file for the LIDAR radar project.  
    * CMakeLists.txt: Connects the source files to the project to meet the build requirements.  
    * Kconfig.projbuild: Defines menuconfig parameters for the WAP credentials.  
* ### main/i2c_test/  
    * Code in this folder was used to test the I2C functions. It enabled us to build an I2C library to control multiple I2C devices on the same SDA and SCL lines.  


## Files:  
The remaining files in the code/ directory are used to ensure the project 
properly builds and flashes to the ESP32.  

For more information on the esp-idf template, please refer to the official 
[esp-idf-template](https://github.com/espressif/esp-idf-template) project that 
follows the [Espressif IoT Development Framework](https://github.com/espressif/esp-idf).  


