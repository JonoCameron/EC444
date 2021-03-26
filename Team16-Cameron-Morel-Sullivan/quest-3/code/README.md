# Code Readme

Code used for Quest 3 uses 2 different file organizations. The first is the 
classic esp-idf format used to build and flash projects to an ESP32. This 
format is located under code/main/. The second organization is for the Node.js 
modules. That particular organization exists under code/src/ and then splits 
into 3 subdirectories depending on the purpose of the given file. In general, 
libraries are saved into code/src/lib/, resource files (HTML and CSV) are 
stored in code/src/res/, and Node.js modules to run are stored in 
code/src/main/.  


## Sub-Folders:  
* ### main/  
    * main.c: The main project file that calls functions from other headers as RTOS tasks.  
    * ADXL343.h: Header file that contains information for ADXL343 accelerometer I2C configurations.  
    * adc1.h: Header file for ADC1 functions.  
    * adc1.c: Implements the ADC1 functions.  
    * i2c_accel.h: Header file for ADXL343 accelerometer I2C functions.  
    * i2c_accel.c: Implements ADXL343 accelerometer I2C functions.  
    * pwm_led.h: Header file for PWM LED functions.  
    * pwm_led.c: Implements PWM LED functions.  
    * sensor.h: Header file for various sensor functions (i.e. temperature, ultrasonic, IR, etc).  
    * sensor.c: Implements the various sensor functions. Mainly handles ADC voltage to unit conversions.  
    * timer.h: Header file for timer timer interrupt functions.  
    * timer.c: Implements the timer interrupt functions.  
    * wifi_station.h: Header file for configuring a WiFi station.  
    * wifi_station.c: Code to enable Wi-Fi support on the ESP32.  
    * CMakeLists.txt: Connects the source files to the project to meet the build requirements.  

* ### src/library/  
    * canvasjs.min.js: JavaScript file that defines the functionality for CanvasJS.  

* ### src/main/  
    * UDP_Server.js: Node.js module that serves URL requests, establishes a UDP server to collect data from the ESP, and send LED PWM levels to the ESP over UDP OK acknowledgements.  

* ### src/res/  
    * sensor_data.csv: CSV file that contains sensor data (thermistor & accelerometer) recorded at different intervals.  
    * sensor_data.html: HTML file that loads CanvasJS graphs from sensor data recorded in sensor_data.csv, Raspberry Pi livestream in an iFrame, and LED PWM slider.  


## Files:  
The remaining files in the code/ directory are used to ensure the project 
properly builds and flashes to the ESP32.  

For more information on the esp-idf template, please refer to the official 
[esp-idf-template](https://github.com/espressif/esp-idf-template) project that 
follows the [Espressif IoT Development Framework](https://github.com/espressif/esp-idf).




