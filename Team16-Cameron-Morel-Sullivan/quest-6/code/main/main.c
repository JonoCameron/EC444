/*
  Main file for Quest 6 - LIDAR Radar.

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Dec. 2020
*/

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/i2c.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include <lwip/netdb.h>

#include "adc1.h"
#include "button_intr.h"
#include "i2c.h"
#include "sensor.h"
#include "servo.h"
#include "timer.h"
#include "udp.h"
#include "wifi_station.h"

/*
// UDP Parameters (Defined from menuconfig)
#if defined(CONFIG_EXAMPLE_IPV4)
  #define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
#elif defined(CONFIG_EXAMPLE_IPV6)
  #define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
#else
  #define HOST_IP_ADDR ""
#endif
*/

#define RADR_PORT  6969 
//#define RADR_IP    "192.168.1.241"  // Jonathan's dish ESP
#define RADR_IP    "192.168.1.22"   // DJ's dish ESP

#define RC_PORT    4200
//#define RC_IP      "192.168.1.178"  // Jonathan's RC ESP
#define RC_IP      "192.168.1.23"   // DJ's RC ESP

// ESP Type
// Identifies whether the flashed ESP should function as a dish (1) or as a motor controller (2)
#define ESP_TYPE         CONFIG_ESP_TYPE

// UART Parameters
#define UART_NUM        UART_NUM_0
#define UART_TXD        (UART_PIN_NO_CHANGE)
#define UART_RXD        (UART_PIN_NO_CHANGE)
#define UART_RTS        (UART_PIN_NO_CHANGE)
#define UART_CTS        (UART_PIN_NO_CHANGE)
#define UART_BAUD_RATE  115200

// Servo Parameter
#define SERVO_GPIO      18  // Set pin 18 (MO) as PWM0A for servo connection

// Buffer Parameters
#define TASK_STACK_SIZE 2048
#define BUF_SIZE 1024

// ESP Controller Parameters
#define BUTTON_GPIO     4   // GPIO pin A5 - Button to toggle radar dish
#define LED_GPIO        33  // GPIO pin 33 - LED to indicate whether or not the radar dish is active
#define PENDING_LED     27  // GPIO pin 27 - LED to indicate that state change to dish is pending

// Dish ESP distance error checking parameters
#define MAX_READINGS    10  // How many valid (non-negative) distance readings should be recorded
#define ERROR_BOUNDS    10  // What is the error bound for those distance readings for each other (precision)


// Global Variables and Constants
bool update_leds = false;                                  // Flag for updating the LEDs
double distance = 0;                                       // Holds distance read from the LIDAR
static esp_adc_cal_characteristics_t *adc_chars;           // ADC characteristics pointer
static const adc_channel_t channel_therm = ADC_CHANNEL_6;  // ADC1 GPIO34 (A2)
static adc_bits_width_t width_bit = ADC_WIDTH_BIT_12;      // ADC width (number of bits to represent raw reading)
static const adc_atten_t atten = ADC_ATTEN_DB_11;          // ADC attenuation (scaling)
static const adc_unit_t unit = ADC_UNIT_1;                 // Specifies which ADC unit to use (1 or 2)
uint8_t* sensor_data;                                      // Buffer to hold sensor data to be sent over UDP
static const char *UDP_TAG = "ESP UDP Client";             // UDP Message Tag
uint8_t count = 0;                                         // A variable to act as a timestamp
bool dish_active = true;                                   // Flag that tells if dish ESP's servo is active or not
uint8_t* send_data;                                        // Data sent from ESP_RC to ESP_RADR over UDP
uint8_t* recv_data;                                        // Data received from ESP_RADR by ESP_RC over UDP... Not sure what, apart from ACK signals
char start = 0x1B;                                         // Start byte for UDP transmission
bool pending = false;                                      // Light LED to say that RC sent

// Mutex (for resources)
SemaphoreHandle_t mux = NULL;

// Initializes all tasks
static void init(void)
{
  // Initialize timer-based interrupts
  initialize_TimerQueue();

  send_data = (uint8_t*) malloc(BUF_SIZE);
  recv_data = (uint8_t*) malloc(BUF_SIZE);

  // Mutex for current values when sending
  mux = xSemaphoreCreateMutex();

  // Set and config parameters for the UART driver, then install the driver
  uart_config_t uart_config = {.baud_rate = UART_BAUD_RATE,
                               .data_bits = UART_DATA_8_BITS,
                               .parity    = UART_PARITY_DISABLE,
                               .stop_bits = UART_STOP_BITS_1,
                               .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                               .source_clk = UART_SCLK_APB
                              };
  uart_param_config(UART_NUM, &uart_config);
  uart_set_pin(UART_NUM, UART_TXD, UART_RXD, UART_RTS, UART_CTS);
  uart_driver_install(UART_NUM, BUF_SIZE * 2, 0, 0, NULL, 0);

  // Allocate space for the sensor data
  sensor_data = (uint8_t*) malloc(BUF_SIZE);

  // Initialize the ADC channels
  check_efuse();
  config_ADC(unit, width_bit, atten, channel_therm);  // ADC for thermistor

  // Characterize ADC
  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width_bit, DEFAULT_VREF, adc_chars);
  print_char_val_type(val_type);

  if (ESP_TYPE == 1)
  {
    printf("Initializing ESP dish configurations...\n");

    // Initialize the servo
    initialize_servo(SERVO_GPIO);

    // Initialize the I2C components (accelerometer, LIDAR, I2C display)
    i2c_master_init();
    i2c_scanner();

    // Check for ADXL
    uint8_t deviceID = 0;
    getDeviceID(&deviceID, 0x53);  // 0x62
    printf("devID: %d\n", deviceID);
    if (deviceID == 0xE5)
    {
      printf("\n>> Found ADXL\n");
    }

    // Initialize the alphanumeric I2C display
    if ( initialize_I2C() == ESP_OK )
    {
      printf("I2C Display successfully initialized!\n");
    }
  }
  else
  {
    printf("Initializing ESP remote controller configurations...\n");

    // Initialize the LED
    gpio_pad_select_gpio(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    gpio_reset_pin(PENDING_LED);
    gpio_set_direction(PENDING_LED, GPIO_MODE_OUTPUT);

    // Initialize the button for hardware interrupts
    gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);  // Install gpio isr service
    button_queue_init(BUTTON_GPIO);
  }

  //Initialize NVS
  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
  {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  ESP_ERROR_CHECK(ret);

  // Initialization for UDP client
  ESP_ERROR_CHECK(esp_netif_init());

  // Initialize the WiFi station (no need to call example_connect() for UDP client)
  ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
  wifi_init_sta();

  // Initialize alarm using timer API
  alarm_init();
}



// UDP Server task. Receives data from UDP clients, and sends a response to them (server fob's ID).
static void udp_server_task(void* pvParameters)
{
  
  uint8_t server_data[3];
  server_data[0] = start;
  server_data[1] = '\0';

  // Setup the server configuration object
  server_conf conf;
  if(ESP_TYPE == 1){
    conf.port = RADR_PORT;
  }
  else if(ESP_TYPE == 2){
    conf.port = RC_PORT;
  }
  conf.data = server_data;
  conf.dish_active = &dish_active;
  conf.pending = &pending;

  // Wait and listen for clients to communicate with
  udp_server(conf, pvParameters);

  // If we get to this point (we shouldn't), then delete the task  
  vTaskDelete(NULL);
}

/**
  Formats and writes a double data into a buffer. Writes the data type, sign, and data (###.##) into buffer.
  \param uint8_t* buffer --> Buffer to write to.
  \param uint32_t size --> Size of the buffer in bytes.
  \param uint32_t offset --> Offset in the buffer to write the formatted data.
  \param char datatype --> Type of data (i.e. 'T' for temperature, 'X' for x-axis acceleration, etc).
  \param double data --> Data to write.
  \param Number of bytes written to the buffer (Should expect 8 on success, 0 on failure).
**/
uint32_t data_to_buffer(uint8_t* buffer, uint32_t size, uint32_t offset, char datatype, double data)
{
  // Variables
  uint32_t written_bytes = 0;  // Holds the number of written bytes to the buffer
  int digits = 0;              // Holds the data's positive digits (thousands to hundredths place)

  // Check that writing the expected data format won't exceed the buffer's bounds
  if ( (offset + 8) >= size )
  {
    // Terminate the buffer write to avoid a buffer overflow
    return -1;
  }

  // Write the data type to the buffer
  buffer[written_bytes + offset] = datatype;
  written_bytes++;

  // Write the sign to the buffer, and set up digits to be positive
  if (data < 0)
  {
    buffer[written_bytes + offset] = '-';  // Indicate negative data value
    digits = (int)(data * -100);
  }
  else
  {
    buffer[written_bytes + offset] = '+';  // Indicate positive data value
    digits = (int)(data * 100);
  }
  written_bytes++;

  // Write the digits (thousands to hundredths) to the buffer
  buffer[written_bytes + offset] = '0' + ((digits / 100000) % 10);  // Thousands place digit
  written_bytes++;
  buffer[written_bytes + offset] = '0' + ((digits / 10000) % 10);   // Hundreds place digit
  written_bytes++;
  buffer[written_bytes + offset] = '0' + ((digits / 1000) % 10);    // Tens place digit
  written_bytes++;
  buffer[written_bytes + offset] = '0' + ((digits / 100) % 10);     // Ones place digit
  written_bytes++;
  buffer[written_bytes + offset] = '.';                             // Decimal
  written_bytes++;
  buffer[written_bytes + offset] = '0' + ((digits / 10) % 10);      // Tenth place digit
  written_bytes++;
  buffer[written_bytes + offset] = '0' + (digits % 10);             // Hundredth place digit
  written_bytes++;

  // Return the number of written bytes
  return written_bytes;
}



// Task to read sensors and send update the sensor_data buffer with the collected sensor data
static void read_sensors(uint32_t degree)
{
  uint32_t offset = 0;  // Tracks how many total bytes have been written to the sensor_data buffer
  uint32_t len = 0;     // Tracks how many bytes have been written to buffer from one write call

  // Sample the ADC to get the temperature
  uint32_t voltage = sample_ADC(unit, width_bit, channel_therm, adc_chars);
  double temp_K = calculate_temperature((double)voltage, 3300, 298.15, 3435);

  // Sample the LIDAR multiple times to improve distance accuracy
  unsigned int data_points = 0;
  double distance_data[MAX_READINGS];
  while (data_points < MAX_READINGS)
  {
    // Continuously sample the LIDAR to get distance readings
    double temp_distance = (double) distanceLLv4();

    // Only record the sample if the reading is valid
    if (temp_distance > 0)
    {
      distance_data[data_points] = temp_distance;
      data_points++;
    }

    // Add a slight delay between readings
    vTaskDelay(50 / portTICK_RATE_MS);
  }

  // Now that the distance data is collected, search through array & categorize distances by relative value
  // Essentially, we will compare values to see if they are +-10cm of element 0, and assign "votes" accordingly
  double rep1 = distance_data[0];
  double sum1 = distance_data[0];
  double votes1 = 1;
  double rep2 = -1;
  double sum2 = 0;
  double votes2 = 0;
  double rep3 = -1;
  double sum3 = 0;
  double votes3 = 0;
  double diff1, diff2, diff3;

  // Iterate through the distance_data array and update the sums and votes
  for (unsigned int i = 1; i < MAX_READINGS; i++)
  {
    // Find the difference between the current data point and the valid representative
    diff1 = distance_data[i] - rep1;
    if (rep2 > 0)
    {
      diff2 = distance_data[i] - rep2;
    }
    if (rep3 > 0)
    {
      diff3 = distance_data[i] - rep3;
    }

    // Check if the difference is within +-10cm rep1
    if (diff1 <= ERROR_BOUNDS && diff1 >= -ERROR_BOUNDS)
    {
      // Value is within the range of rep1, so note the situation
      sum1 += distance_data[i];
      votes1++;
    }
    else if (rep2 > 0)
    {
      // There is a second representative, so check if the difference is within +-10cm of this rep
      if (diff2 <= ERROR_BOUNDS && diff2 >= -ERROR_BOUNDS)
      {
        // Value is within the range of rep2, so note the situation
        sum2 += distance_data[i];
        votes2++;
      }
      else if (rep3 > 0)
      {
        // There is a third representative, so check if the difference is within +-10cm of this rep
        if (diff3 <= ERROR_BOUNDS && diff3 >= -ERROR_BOUNDS)
        {
          // Value is within the range of rep3, so note the situation
          sum3 += distance_data[i];
          votes3++;
        }
        // If we don't make the above condition, just ignore the current data point since it will
        // not be represented. Hopefully it won't rebel against us...
      }
      else
      {
        // Make the distance the third representative
        rep3 = distance_data[i];
        sum3 = distance_data[i];
        votes3 = 1;
      }
    }
    else
    {
      // Make the distance the second representative
      rep2 = distance_data[i];
      sum2 = distance_data[i];
      votes2 = 1;
    }
  }

  // Now that the votes have been tallied, determine which representative is better to reflect the distance
  if (votes3 >= votes2 && votes3 >= votes1)
  {
    distance = sum3 / votes3;
  }
  else if (votes2 >= votes1 && votes2 >= votes3)
  {
    distance = sum2 / votes2;
  }
  else
  {
    // Default to reflecting representative 1
    distance = sum1 / votes1;
  }

  // DEBUGGING
  // Was only used to see the differences between the MAX_READINGS distances
  /*
  len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'D', distance);
  offset += len;
  for (unsigned int i = 0; i < MAX_READINGS; i++)
  {
    sensor_data[offset] = ',';
    offset++;
    len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'd', distance_data[i]);
    offset += len;
  }
  sensor_data[offset] = '\n';  // End sensor data will null character
  offset++;

  // Send data over UART
  uart_write_bytes(UART_NUM, (const char*) sensor_data, offset);
  */

  // Read accelerometer data
  float xVal, yVal, zVal;
  float roll, pitch;
  getAccel(&xVal, &yVal, &zVal);
  calcRP(xVal, yVal, zVal, &roll, &pitch);

  // Send sensor data over Wi-Fi
  // Count variable
  len = data_to_buffer(sensor_data, BUF_SIZE, offset, 't', count);
  offset += len;
  sensor_data[offset] = ',';
  offset++;
  // Angle (in degrees)
  len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'A', degree);
  offset += len;
  sensor_data[offset] = ',';
  offset++;
  // Temperature
  len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'T', temp_K);
  offset += len;
  sensor_data[offset] = ',';
  offset++;
  // Distance
  len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'D', distance);
  offset += len;
  sensor_data[offset] = ',';
  offset++;
  // x-acceleration
  len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'X', (double) xVal);
  offset += len;
  sensor_data[offset] = ',';
  offset++;
  // y-acceleration
  len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'Y', (double) yVal);
  offset += len;
  sensor_data[offset] = ',';
  offset++;
  // z-acceleration
  len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'Z', (double) zVal);
  offset += len;
  sensor_data[offset] = ',';
  offset++;
  // Roll
  len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'R', (double) roll);
  offset += len;
  sensor_data[offset] = ',';
  offset++;
  // Pitch
  len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'P', (double) pitch);
  offset += len;
  sensor_data[offset] = ',';
  offset++;
  // Check the current status of dish_active for the next UART piece (s for status)
  if (dish_active)
  {
    // Dish ESP has no plans of stopping at the moment
    len = data_to_buffer(sensor_data, BUF_SIZE, offset, 's', (double) 1111);
  }
  else
  {
    // Dish ESP is planning to stop once it finishes its cycle
    len = data_to_buffer(sensor_data, BUF_SIZE, offset, 's', (double) 2222);
  }
  offset += len;
  sensor_data[offset] = '\n';  // End sensor data will null character
  offset++;

  // Send data over UART
  uart_write_bytes(UART_NUM, (const char*) sensor_data, offset);

  // Increment count
  count++;
}



// Task that continuously displays the wheel speed on the I2C display.
static void i2c_display_task()
{
  // Display the current distance
  displayDistance(&distance);
}



static void servo_control()
{
  // Variables to track the current servo degree
  uint32_t currentDegree = 0;
  uint32_t nextDegree = 0;

  // Move the servo to the default location (just in case)
  servo_rotateTo(0, 300);

  char ip[IP_CHAR_SIZE];
  uint16_t port;
  strcpy(ip, RC_IP);
  port = RC_PORT;

  // Loop forever to check if the servo needs to rotate
  while (1)
  {
    // Send message saying instruction no longer pending, it is being acted on.
    send_data[0] = '~';

    udp_client(ip, port, send_data, recv_data);

    // Check if the flag for enabling the servo is raised
    if (dish_active)
    {
      uint32_t currentDegree = 0;
      uint32_t nextDegree = 0;
      printf("DISH IS ACTIVE\n");
      // Move the servo to the default location (just in case)
      servo_rotateTo(0, 300);

      // Rotate from 0 to 180 degrees
      // Note that the recommended delay at 5V is 100ms per 60degrees
      for (unsigned int i = 0; i < 36; i++)
      {
        // Take a sensor measurement
        read_sensors(currentDegree);

        // Rotate the dish counter-clockwise
        nextDegree = currentDegree + 5;
        servo_rotateTo( nextDegree, 100 );
        currentDegree = nextDegree;

        // A heartbeat that sends the state of the radar. (STOP/GO)
        send_data[0] = '?';
        send_data[1] = '1';
        send_data[2] = '\0';

        udp_client(ip, port, send_data, recv_data);
      }

      // Rotate from 180 to 0 degrees
      for (unsigned int i = 0; i < 36; i++)
      {
        // Take a sensor measurement
        read_sensors(currentDegree);

        // Rotate the dish clockwise
        nextDegree = currentDegree - 5;
        servo_rotateTo( nextDegree, 100 );
        currentDegree = nextDegree;

        // A heartbeat that sends the state of the radar (1). (STOP/GO)
        send_data[0] = '?';
        send_data[1] = '1';
        send_data[2] = '\0';

        udp_client(ip, port, send_data, recv_data);
      }
      
    }
    else
    {
      // Send a udp_client message saying ACK (?) and radar state (0)
      send_data[0] = '?';
      send_data[1] = '0';
      send_data[2] = '\0';

      udp_client(ip, port, send_data, recv_data);

      printf("something somewhere told us to stop\n");
    }

    // Wait some time to reduce power and keep the Watchdog happy. (Happy watchdog noises)
    vTaskDelay(50 / portTICK_RATE_MS);
  }
}



/**
  LED task to light LED based on the dish ESP's current servo status.
  \param None
  \return None
**/
void led_task()
{
  while (1)
  {
    if (update_leds)
    {
      if (dish_active)
      {
        gpio_set_level(LED_GPIO, 1);
      }
      else
      {
        gpio_set_level(LED_GPIO, 0);
      }

      if (pending)
      {
        gpio_set_level(PENDING_LED, 1);
      }
      else
      {
        gpio_set_level(PENDING_LED, 0);
      }

      // Reflect that the LEDs have been updated
      update_leds = false;
    }
    else
    {
      // Add a delay to keep everyone happy & reduce RC ESP power consumption
      vTaskDelay(250 / portTICK_RATE_MS);
    }
  }
}



/**
  Button task that sends a UDP message to the ESP dish to toggle its activity.
  \param None
  \return None
**/
void button_task_control()
{
  uint32_t io_num;
  while(1)
  {
    if ( xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY) )
    {
      xSemaphoreTake(mux, portMAX_DELAY);

      // DEBUGGING
      // Toggle the dish_active flag to simulate the UDP request successfully went through
      //dish_active = !dish_active;
      char ip[IP_CHAR_SIZE];
      uint16_t port;
      strcpy(ip, RADR_IP);
      port = RADR_PORT;

      // If the dish is active and there is not an instruction pending, send an instruction ('0') to stop the dish
      if(dish_active && pending == false){
        send_data[0] = '!';
        send_data[1] = '0';
        send_data[2] = '\0';

        udp_client(ip, port, send_data, recv_data);
        pending = true;
        update_leds = true;
      }
      // If the dish is inactive and there is no instruction pending, send an instruction ('1') to start the dish
      else if(dish_active == false && pending == false){
        send_data[0] = '!';
        send_data[1] = '1';
        send_data[2] = '\0';

        udp_client(ip, port, send_data, recv_data);
        pending = true;
        update_leds = true;
      }
      // Fill the send_data buffer with garbage so that we can't mistakenly send an instruction incorrectly (idk, just a precaution).
      for(int i = 0; i < 3; i++){
        send_data[i] = 'N';
      }

      xSemaphoreGive(mux); 
    }
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}



/**
  Handles timer event to raise the update flag for reading temperature. Event flag raised every 2 seconds.
  \param void* arg --> Pointer to arguments.
  \return None
**/
static void timer_event_task(void *arg)
{
  while (1)
  {
    // Create dummy structure to store structure from queue
    timer_event_t evt;

    // Transfer from queue
    xQueueReceive(timer_queue, &evt, portMAX_DELAY);

    // Do something if triggered!
    if (evt.flag == 1)
    {
      // Set the update flag
      update_leds = true;
    }
  }
}



// Runs the show by setting up all initialization and configurations for tasks
void app_main(void)
{
  // Initialize everything
  init();

  // Create task to handle timber-based events
  xTaskCreate(timer_event_task, "timer_event_task", TASK_STACK_SIZE, NULL, configMAX_PRIORITIES, NULL);

  // Run the appropriate tasks based on the ESP function
  if (ESP_TYPE == 1)  // Dish ESP
  {
    // Create task to control the servo and read the sensors (thermistor, LIDAR, & accelerometer)
    xTaskCreate(servo_control, "servo_control", TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);

    // Create I2C display task
    xTaskCreate(i2c_display_task, "i2c_display_task", TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, NULL);
  }
  else  // RC ESP
  {
    // Create hardware interrupt (button) task for sending UDP messages
    xTaskCreate(button_task_control, "button_task_control", TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);

    // Create LED task
    xTaskCreate(led_task, "led_task", TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, NULL);
  }

  // Create server task to listen for RC signal
    xTaskCreate(udp_server_task, "udp_server_task", 3072, NULL, configMAX_PRIORITIES - 3, NULL);

}




