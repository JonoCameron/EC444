# Code Readme

Code used for Quest 2 uses 2 different file organizations. The first is the 
classic esp-idf format used to build and flash projects to an ESP32. This 
format is located under code/main/. The second organization is for the Node.js 
modules. That particular organization exists under code/src/ and then splits 
into 3 subdirectories depending on the purpose of the given file. In general, 
libraries are saved into code/src/lib/, resource files (HTML and CSV) are 
stored in code/src/res/, and Node.js modules to run are stored in 
code/src/main/.  

This project used the [esp-idf adc1 example](https://github.com/espressif/esp-idf/tree/39f090a4f1dee4e325f8109d880bf3627034d839/examples/peripherals/adc) 
as a starting point, and added additional functionality (I2C, hardware 
interrupts, sensor functions, etc) on on top.  


## Sub-Folders:  
* ### main/  
    * main.c: The main project file that calls functions from other headers as RTOS tasks.  
    * adc1.h: Header file for ADC1 functions.  
    * adc1.c: Implements the ADC1 functions.  
    * i2c_display.h: Header file for I2C display functions.  
    * i2c_display.c: Implements the I2C display functions.  
    * sensor.h: Header file for various sensor functions (i.e. temperature, ultrasonic, IR, etc).  
    * sensor.c: Implements the various sensor functions. Mainly handles ADC voltage to unit conversions.  
    * timer.h: Header file for timer timer interrupt functions.  
    * timer.c: Implements the timer interrupt functions.  
    * CMakeLists.txt: Connects the source files to the project to meet the build requirements.  

* ### src/library/  
    * canvasjs.min.js: JavaScript file that defines the functionality for CanvasJS.  

* ### src/main/  
    * ServeURLRequest_Module.js: Node.js module that serves GET requests. Intended for CanvasJS graph displays.  
    * UART_Read_Module.js: Node.js module that reads data from the ESP32 over UART. Parses data lines by '\n'.  

* ### src/res/  
    * sensor_data.csv: CSV file that contains sensor data (therm, IR, ultrasonic) recorded at different intervals.  
    * sensor_data.html: HTML file that loads 3 CanvasJS graphs from the data recorded in sensor_data.csv.  


## Files:  
The remaining files in the code/ directory are used to ensure the project 
properly builds and flashes to the ESP32.  

For more information on the esp-idf template, please refer to the official 
[esp-idf-template](https://github.com/espressif/esp-idf-template) project that 
follows the [Espressif IoT Development Framework](https://github.com/espressif/esp-idf).

