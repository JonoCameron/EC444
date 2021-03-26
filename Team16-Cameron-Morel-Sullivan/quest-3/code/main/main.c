/*
  Main file for the Hurricane Box (Quest 3) project. Modified from the esp-idf adc1
  example project.

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Oct. 2020
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
#include "i2c_accel.h"
#include "pwm_led.h"
#include "sensor.h"
#include "timer.h"
#include "wifi_station.h"



// UDP Parameters (Defined from menuconfig)
#if defined(CONFIG_EXAMPLE_IPV4)
  #define HOST_IP_ADDR CONFIG_EXAMPLE_IPV4_ADDR
#elif defined(CONFIG_EXAMPLE_IPV6)
  #define HOST_IP_ADDR CONFIG_EXAMPLE_IPV6_ADDR
#else
  #define HOST_IP_ADDR ""
#endif
#define PORT CONFIG_EXAMPLE_PORT

// UART Parameters
#define UART_NUM        UART_NUM_0
#define UART_TXD        (UART_PIN_NO_CHANGE)
#define UART_RXD        (UART_PIN_NO_CHANGE)
#define UART_RTS        (UART_PIN_NO_CHANGE)
#define UART_CTS        (UART_PIN_NO_CHANGE)
#define UART_BAUD_RATE 115200

// LEDC Parameters
#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_GPIO       4                    // Pin A5
#define LEDC_CHANNEL    LEDC_CHANNEL_0
#define LEDC_DUTY       4000                 // Max duty (intensity level 10)
#define LEDC_FADE_TIME  3000
#define STEP_TIME       250                  // Set intensity level steps to 250ms

// Buffer Parameters
#define TASK_STACK_SIZE 2048
#define BUF_SIZE 1024



// Global Variables and Constants
bool update_reading = false;                               // Flag for reading the sensors (done every 2s)
bool send_data = false;                                    // Flag for communicating with UDP server
static esp_adc_cal_characteristics_t *adc_chars;           // ADC characteristics pointer
static const adc_channel_t channel_therm = ADC_CHANNEL_6;  // ADC1 GPIO34 (A2)
static adc_bits_width_t width_bit = ADC_WIDTH_BIT_12;      // ADC width (number of bits to represent raw reading)
static const adc_atten_t atten = ADC_ATTEN_DB_11;          // ADC attenuation (scaling)
static const adc_unit_t unit = ADC_UNIT_1;                 // Specifies which ADC unit to use (1 or 2)
ledc_timer_config_t* ledc_timer;                           // LEDC timer pointer
ledc_channel_config_t* ledc_channel;                       // LEDC channel pointer
uint8_t* sensor_data;                                      // Buffer to hold sensor data to be sent over UDP
static const char *UDP_TAG = "ESP UDP Client";             // UDP Message Tag
uint8_t count = 0;                                         // A variable to act as a timestamp/



// Initializes all tasks
static void init(void)
{
  // Initialize timer-based interrupts
  initialize_TimerQueue();

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

  // Allocate space for the LED timer and channel pointers
  ledc_timer = malloc(sizeof(*ledc_timer));
  ledc_channel = malloc(sizeof(*ledc_channel));

  // Initialize LEDC
  initialize_ledc(ledc_timer, ledc_channel, LEDC_MODE, LEDC_TIMER, LEDC_CHANNEL, LEDC_GPIO);

  // Set up the I2C accelerometer
  initialize_accelerometer();

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
static void read_sensors()
{
  while(1)
  {
    // Only perform actions when the timer reached 100ms
    if (update_reading)
    {
      uint32_t offset = 0;  // Tracks how many total bytes have been written to the sensor_data buffer
      uint32_t len = 0;     // Tracks how many bytes have been written to buffer from one write call

      // Sample the ADC to get the temperature
      uint32_t voltage = sample_ADC(unit, width_bit, channel_therm, adc_chars);
      double temp_K = calculate_temperature((double)voltage, 3300, 298.15, 3435);

      // Read accelerometer data
      float xVal, yVal, zVal;
      float roll, pitch;
      getAccel(&xVal, &yVal, &zVal);
      calcRP(xVal, yVal, zVal, &roll, &pitch);

      // Send sensor data over Wi-Fi
      len = data_to_buffer(sensor_data, BUF_SIZE, offset, 't', count);
      offset += len;
      sensor_data[offset] = ',';
      offset++;

      len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'T', temp_K);
      offset += len;
      sensor_data[offset] = ',';
      offset++;
      len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'X', (double) xVal);
      offset += len;
      sensor_data[offset] = ',';
      offset++;
      len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'Y', (double) yVal);
      offset += len;
      sensor_data[offset] = ',';
      offset++;
      len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'Z', (double) zVal);
      offset += len;
      sensor_data[offset] = ',';
      offset++;
      len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'R', (double) roll);
      offset += len;
      sensor_data[offset] = ',';
      offset++;
      len = data_to_buffer(sensor_data, BUF_SIZE, offset, 'P', (double) pitch);
      offset += len;
      sensor_data[offset] = '\0';  // End sensor data will null character
      offset++;

      // DEBUGGING
      //uart_write_bytes(UART_NUM, (const char*) sensor_data, offset);

      // Turn off the update flag since we just handled reading all of the sensor data
      update_reading = false;

      count++; //Increment count.

      // Turn on flag to transmit data
      send_data = true;
    }
    // Add delay
    vTaskDelay(100);
  }
}



// UDP Client task. Sends data (sensor readings) to and receives data (LED intensity) from UDP server.
static void udp_client_task(void *pvParameters)
{
  char rx_buffer[128];
  char host_ip[] = HOST_IP_ADDR;
  int addr_family = 0;
  int ip_protocol = 0;

  while (1)
  {
    #if defined(CONFIG_EXAMPLE_IPV4)
      struct sockaddr_in dest_addr;
      dest_addr.sin_addr.s_addr = inet_addr(HOST_IP_ADDR);
      dest_addr.sin_family = AF_INET;
      dest_addr.sin_port = htons(PORT);
      addr_family = AF_INET;
      ip_protocol = IPPROTO_IP;
    #elif defined(CONFIG_EXAMPLE_IPV6)
      struct sockaddr_in6 dest_addr = { 0 };
      inet6_aton(HOST_IP_ADDR, &dest_addr.sin6_addr);
      dest_addr.sin6_family = AF_INET6;
      dest_addr.sin6_port = htons(PORT);
      dest_addr.sin6_scope_id = esp_netif_get_netif_impl_index(EXAMPLE_INTERFACE);
      addr_family = AF_INET6;
      ip_protocol = IPPROTO_IPV6;
    #elif defined(CONFIG_EXAMPLE_SOCKET_IP_INPUT_STDIN)
      struct sockaddr_in6 dest_addr = { 0 };
      ESP_ERROR_CHECK(get_addr_from_stdin(PORT, SOCK_DGRAM, &ip_protocol, &addr_family, &dest_addr));
    #endif

    int sock = socket(addr_family, SOCK_DGRAM, ip_protocol);
    if (sock < 0)
    {
      ESP_LOGE(UDP_TAG, "Unable to create socket: errno %d", errno);
      break;
    }
    ESP_LOGI(UDP_TAG, "Socket created, sending to %s:%d", HOST_IP_ADDR, PORT);

    // While loop for UDP client-server communication
    while (1)
    {
      // Wait until we have data to send
      if (send_data)
      {
        // Send data to the server
        int err = sendto(sock, (const char*) sensor_data, strlen((const char*) sensor_data),
                         0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
        if (err < 0)
        {
          ESP_LOGE(UDP_TAG, "Error occurred during sending: errno %d", errno);
          break;
        }
        ESP_LOGI(UDP_TAG, "Message sent");

        // Record that we sent data to the UDP server
        send_data = false;

        // Receive data from the server
        struct sockaddr_in source_addr; // Large enough for both IPv4 or IPv6
        socklen_t socklen = sizeof(source_addr);
        int len = recvfrom(sock, rx_buffer, sizeof(rx_buffer) - 1, 0, (struct sockaddr *)&source_addr, &socklen);

        // Error occurred during receiving
        if (len < 0)
        {
          ESP_LOGE(UDP_TAG, "recvfrom failed: errno %d", errno);
          break;
        }
        // Data received from the server so handle it
        else
        {
          rx_buffer[len] = 0; // Null-terminate whatever we received and treat like a string
          ESP_LOGI(UDP_TAG, "Received %d bytes from %s:", len, host_ip);
          ESP_LOGI(UDP_TAG, "%s", rx_buffer);

          // Handle LED intensity based on server feedback
          if ( (strncmp(rx_buffer, "OK: Set LED ", 12) == 0) && isdigit(rx_buffer[12]) )
          {
            // Update LED intensity to the specified level
            uint32_t intensity_level = (uint32_t) (rx_buffer[12] - '0');
            set_duty(ledc_channel, (intensity_level * LEDC_DUTY / 10) );
          }
          else if (strncmp(rx_buffer, "OK: ", 4) == 0)
          {
            ESP_LOGI(UDP_TAG, "Received expected message, reconnecting");
            break;
          } 
        }
      }

      // Wait before checking if we're ready to send more data to the server
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }

    if (sock != -1)
    {
      ESP_LOGE(UDP_TAG, "Shutting down socket and restarting...");
      shutdown(sock, 0);
      close(sock);
    }
  }
  // DEBUGGING
  printf("Uh oh, the UDP task has been deleted for some reason...\n");

  vTaskDelete(NULL);
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
      update_reading = true;
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

  // Create task to read the sensors (thermistor and accelerometer)
  xTaskCreate(read_sensors, "read_sensors", TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);

  // Create task for UDP communication (sends sensor data and receives LED intensity)
  xTaskCreate(udp_client_task, "udp_client_task", 4096, NULL, configMAX_PRIORITIES - 2, NULL);

  // Initialize alarm using timer API
  alarm_init();
}


