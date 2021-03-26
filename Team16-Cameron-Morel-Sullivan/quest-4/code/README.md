# Code Readme  
Code used for Quest 4 combines functionality from Skills 25, 26, and 28.  

Code adapted from Skill 28 provides support for multiple fobs to conduct a leader 
election wirelessly over UDP. This skill also adopted code from Skill 25, and 
adapted the esp-idf [udp_client](https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_client) 
and [udp_server](https://github.com/espressif/esp-idf/tree/master/examples/protocols/sockets/udp_server) 
example projects for the UDP communication.  

Code adapted from Skill 26 was modified from various W3School tutorials for topics 
such as Node.js and MongoDB in order to create a server that handles MongoDB 
queries.  

All code is included in the src/ directory, which contains 3 subdirectories: 
library/, main/, and res/. These subdirectories function as how they are named. 
The main/ subdirectory contains the main code for the project, which is to 
setup an express server that handles requests and MongoDB queries. The library/ 
subdirectory contains any necessary library calls for the project, which is 
canvasjs.min.js, but isn't actually used in the current skill. The res/ 
subdirectory contains resource files, which includes data files (TSV) as well 
as HTML files for client interaction.  


## Sub-Folders: 
* ### src/library/
    * canvasjs.min.js: JavaScript file that defines the functionality for CanvasJS. Not currently used.  
* ### src/main/
    * express_server.js: Node.js module that configures an express server that handles client web requests and MongoDB queries.  
    * mongodb-functions.js: Custom Node.js module that defines MongoDB functions (read TSV into database and query sensor ID results).  
* ### src/main/test/  
    * server_test.js: JavaScript file to test the server.  
    * udp_test.js: JavaScript file to test UDP connections.  
* ### src/res/  
    * candidate_disp.html: HTML file that handles client searches (sensor queries) and displays results.  
* ### main/  
  * button_intr.h: Header file for hardware (button) interrupts. Not currently used in project.  
  * button_intr.c: Implements hardware (button) interrupt functions. Note currently used in project.  
  * ir_tx_rx.h: Header file for IR transmitter/transceiver functions.  
  * ir_tx_rx.c: Implements IR transmitter and transceiver functions.  
  * timer.h: Header file for timer timer interrupt functions.  
  * timer.c: Implements the timer interrupt functions.  
  * udp.h: Header file for UDP client and server functions.  
  * udp.c: Implements UDP client and server functions.  
  * wifi_station.h: Header file for functions that establish the ESP as a wifi station.  
  * wifi_station.c: Implements functions that establish the ESP as a wifi station.  
  * main.c: Main file for the IR TX-RX fob project.  
  * CMakeLists.txt: Connects the source files to the project to meet the build requirements.  
  * Kconfig.projbuild: Defines menuconfig parameters for the WAP credentials and the fob's ID.  


## Files:  
The remaining files in the code/ directory are used to ensure the project 
properly builds and flashes to the ESP32.  

For more information on the esp-idf template, please refer to the official 
[esp-idf-template](https://github.com/espressif/esp-idf-template) project that 
follows the [Espressif IoT Development Framework](https://github.com/espressif/esp-idf).  
