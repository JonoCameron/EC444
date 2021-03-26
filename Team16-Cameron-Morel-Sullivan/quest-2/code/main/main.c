/*
  Main file for the Tactile Internet (Quest 2) project. Modified from the esp-idf adc1
  example project. Displays sensor data from thermistor, IR range finder, and 
  ultrasonic sensors over UART.

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Oct. 2020
*/


#include <stdio.h>
#include <math.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "adc1.h"
#include "timer.h"
#include "sensor.h"


// Parameters for UART communication
#define TXD_GPIO_PIN 1
#define RXD_GPIO_PIN 3
#define RTS_GPIO_PIN (UART_PIN_NO_CHANGE)
#define CTS_GPIO_PIN (UART_PIN_NO_CHANGE)
#define UART_PORT_NUM 0
#define UART_BAUD_RATE 115200

#define TASK_STACK_SIZE 2048
#define BUF_SIZE 1024
#define MAX_COUNT 90  // Support up to 3 minutes of relative data collection

// Parameters for Ultrasonic sensor
#define TRIGGER_GPIO_PIN 25   // A1 is pin 25
#define ECHO_GPIO_PIN    26   // A0 is pin 26
#define MAX_RANGE_CM     400  // Sensor can read between 2cm to 400cm according to specs


// Global Variables and Constants
uint32_t count = 0;                                    // Tracks how many times data has been collected
bool update = false;                                   // Flag for reading the sensors (done every 2s)
static esp_adc_cal_characteristics_t *adc_chars;       // ADC characteristics pointer
static const adc_channel_t channel_therm = ADC_CHANNEL_6;  // ADC1 GPIO34 (A2)
static const adc_channel_t channel_infra = ADC_CHANNEL_0;  // ADC1 GPIO36 (A4)
static adc_bits_width_t width_bit = ADC_WIDTH_BIT_12;  // ADC width (number of bits to represent raw reading)
static const adc_atten_t atten = ADC_ATTEN_DB_11;      // ADC attenuation (scaling)
static const adc_unit_t unit = ADC_UNIT_1;             // Specifies which ADC unit to use (1 or 2)
static ultrasonic_sensor_t dev;                        // Structure containing ultrasonic trigger & echo pins
uint8_t* data;                                         // Pointer to data to send over UART



static void init(void)
{
  // Initialize timer-based interrupts
  initialize_TimerQueue();

  // Configure parameters of UART driver
  uart_config_t uart_config = {.baud_rate = UART_BAUD_RATE,
                               .data_bits = UART_DATA_8_BITS,
                               .parity    = UART_PARITY_DISABLE,
                               .stop_bits = UART_STOP_BITS_1,
                               .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
                               .source_clk = UART_SCLK_APB
                              };
  int intr_alloc_flags = 0;

  #if CONFIG_UART_ISR_IN_IRAM
    intr_alloc_flags = ESP_INTR_FLAG_IRAM;
  #endif

  // Install the UART driver
  uart_driver_install(UART_PORT_NUM, BUF_SIZE * 2, 0, 0, NULL, intr_alloc_flags);
  uart_param_config(UART_PORT_NUM, &uart_config);
  uart_set_pin(UART_PORT_NUM, TXD_GPIO_PIN, RXD_GPIO_PIN, RTS_GPIO_PIN, CTS_GPIO_PIN);

  // Allocate space for the UART data
  data = (uint8_t *) malloc(BUF_SIZE);

  // Initialize the ADC channels
  check_efuse();
  config_ADC(unit, width_bit, atten, channel_therm);  // ADC for thermistor
  config_ADC(unit, width_bit, atten, channel_infra);  // ADC for IR

  // Characterize ADC
  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width_bit, DEFAULT_VREF, adc_chars);
  print_char_val_type(val_type);

  // Initialize the ultrasonic's dev structure
  dev.trigger_pin = TRIGGER_GPIO_PIN;
  dev.echo_pin = ECHO_GPIO_PIN;

  // Initialize the ultrasonic sensor
  ultrasonic_init(&dev);
}



/**
  Writes a double into a character (uint8_t) array. Writes digits from thousands to hundredths place. Assumes positive input number.  
  \param double num --> Double number to write.
  \param uint8_t* data --> Location to write to.
  \param int data_size --> Amount of bytes allocated to the data pointer.
  \return Number of bytes written to data on success, or -1 if invalid size allocated for the data pointer.
**/
int write_double(double num, uint8_t* data, int data_size)
{
  // Initialize the return value
  int written_bytes = 0;

  // Make sure there is enough room to write 7 characters ("####.##")
  if ( data_size < 7 )
  {
    // Return an error, and don't write anything
    return -1;
  }

  // Multiply number by 100 (gets 2 decimals), and convert to an int so that all digits are accounted for
  int digits = (int)(num * 100);

  // Note: Use an offset of 2 because assume data element #1 specifies the sensor (T,I,U), and element #2 is a comma

  // Write the thoudands place digit
  data[2 + written_bytes] = '0' + ((digits / 100000) % 10);
  written_bytes++;

  // Write the hundreds place digit
  data[2 + written_bytes] = '0' + ((digits / 10000) % 10);
  written_bytes++;

  // Write the tens place digit
  data[2 + written_bytes] = '0' + ((digits / 1000) % 10);
  written_bytes++;

  // Write the ones place digit
  data[2 + written_bytes] = '0' + ((digits / 100) % 10);
  written_bytes++;

  // Write the decimal
  data[2 + written_bytes] = '.';
  written_bytes++;

  // Write the tenth place digit
  data[2 + written_bytes] = '0' + ((digits / 10) % 10);
  written_bytes++;

  // Write the hundredth place digit
  data[2 + written_bytes] = '0' + (digits % 10);
  written_bytes++;

  // Return the number of written bytes
  return written_bytes;
}



/**
  Writes an unsigned integer into a character (uint8_t) array. Writes up to 7 digits for the number.
  \param uint32_t num --> Unsigned int to write.
  \param uint8_t* data --> Location to write to.
  \param int data_size --> Amount of bytes allocated to the data pointer.
  \return Number of bytes written to data on success, or -1 if invalid size allowed for the data pointer.
**/
int write_int(uint32_t num, uint8_t* data, int data_size)
{
  // Initialize the return value
  int written_bytes = 0;

  // Make sure there is enough room to write the integer
  if ( data_size < 7 )
  {
    // Return an error, and don't write anything
    return -1;
  }

  // Write the millions place digit
  data[written_bytes] = '0' + ((num / 1000000) % 10);
  written_bytes++;

  // Write the hundred thousands place digit
  data[written_bytes] = '0' + ((num / 100000) % 10);
  written_bytes++;

  // Write the ten thousands place digit
  data[written_bytes] = '0' + ((num / 10000) % 10);
  written_bytes++;

  // Write the thousands place digit
  data[written_bytes] = '0' + ((num / 1000) % 10);
  written_bytes++;

  // Write the hundreds place digit
  data[written_bytes] = '0' + ((num / 100) % 10);
  written_bytes++;

  // Write the tens place digit
  data[written_bytes] = '0' + ((num / 10) % 10);
  written_bytes++;

  // Write the ones place digit
  data[written_bytes] = '0' + (num % 10);
  written_bytes++;


  return written_bytes;
}



/**
  Samples a particular ADC channel for its voltage. Assumes the global "unit" variable applies to all ADC channels.
  \param adc_channel_t channel --> ADC channel to sample.
  \return ADC's voltage reading as a uint32_t value.
**/
uint32_t sample_ADC(adc_channel_t channel)
{
  // Sample ADC1 when called to do so
  uint32_t adc_reading = 0;

  // Multisampling
  for (int i = 0; i < NO_OF_SAMPLES; i++)
  {
    if (unit == ADC_UNIT_1)
    {
      adc_reading += adc1_get_raw((adc1_channel_t)channel);
    }
    else
    {
      int raw;
      adc2_get_raw((adc2_channel_t)channel, ADC_WIDTH_BIT_12, &raw);
      adc_reading += raw;
    }
  }
  adc_reading /= NO_OF_SAMPLES;

  // Convert adc_reading to voltage in mV
  uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

  return voltage;
}



static void read_sensors()
{
  while(1)
  {
    // Only perform actions when the timer reached 100ms
    if (update)
    {
      // Write the time counter to UART
      int len = write_int(count, data, BUF_SIZE);
      data[len] = ',';
      len++;
      uart_write_bytes(UART_PORT_NUM, (const char *)data, len);

      // Update count (wrap around if go over the MAX_COUNT)
      if ( count < MAX_COUNT )
      {
        count++;
      }
      else
      {
        // Wrap around back to 0
        count = 0;
      }

      // Sample the ADC to get the temperature
      uint32_t voltage = sample_ADC(channel_therm);
      double temp_K = calculate_temperature((double)voltage, 3300, 298.15, 3435);
      //temp = kelvin2fahrenheit(temp_K);  // Update the global temp value (F) to show value on I2C (debugging)

      // Send the formatted temperature data over UART
      len = write_double(temp_K, data, BUF_SIZE);
      data[0] = 'T';
      data[1] = ',';
      len = len + 2;
      data[len] = ',';
      len++;
      uart_write_bytes(UART_PORT_NUM, (const char *) data, len);

      // Sample the ADC to get the IR range
      voltage = sample_ADC(channel_infra);
      double range = calculate_range_ir((double)voltage, 60150.376, 0.997);

      // Send the formatted IR range data over UART
      len = write_double(range, data, BUF_SIZE);
      data[0] = 'I';
      data[1] = ',';
      len = len + 2;
      data[len] = ',';
      len++;
      uart_write_bytes(UART_PORT_NUM, (const char *) data, len);

      // Sample the ultrasonic sensor
      double distance = ultrasonic_measure_distance(&dev, MAX_RANGE_CM);

      // Send the formatted ultrasonic distance data over UART
      len = write_double(distance, data, BUF_SIZE);
      data[0] = 'U';
      data[1] = ',';
      len = len + 2;
      data[len] = '\n';
      len++;
      uart_write_bytes(UART_PORT_NUM, (const char *) data, len);

      // Turn off the update flag since we just handled reading all of the sensor data
      update = false;
    }
    // Add delay
    vTaskDelay(100);
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
      update = true;
    }
  }
}



void app_main(void)
{
  // Initialize everything
  init();

  // Create task to handle timber-based events
  xTaskCreate(timer_event_task, "timer_event_task", TASK_STACK_SIZE, NULL, configMAX_PRIORITIES, NULL);

  // Create task to read the sensors (thermistor, IR, ultrasonic)
  xTaskCreate(read_sensors, "read_sensors", TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);

  // Initialize alarm using timer API
  alarm_init();
}


