/*
  Main file for Quest 5 - Cruise Control. Used base code from skills 30-33.

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "adc1.h"
#include "i2c_display.h"
#include "lidar.h"
#include "mathy_math.h"
#include "mcpwm_motor.h"
#include "timer.h"
#include "udp.h"
#include "ultrasonic.h"
#include "wifi_station.h"
#include "buggy_motor.h"


// Parameters for the ultrasonic sensor
#define TRIGGER_GPIO_PIN  17    // TX is pin 17
#define ECHO_GPIO_PIN     16    // RX is pin 16
#define MAX_RANGE_CM      400   // 4m is the max range listed on the spec sheet

// Parameters for PID
#define SETPOINT_RANGE    20   // Set 20cm as the range setpoint (distance target should be to stop car)
#define SETPOINT_SIDED    0.4  // Set 40cm as the side distance setpoint (how far car should be from left wall)
#define SETPOINT_SPEED    0.1  // Set 0.1m/s as the speed setpoint (want the wheel average to match this value)

// Parameters for the optical decoder sensor
// Note: we include a gap between the ranges as a "No Man's Zone" where it is too close to tell
#define MIN_BLACK_RANGE         2000  // Minimum ADC reading (in mV) for the black section
#define MAX_BLACK_RANGE         3000  // Maximum ADC reading (in mV) for the black section (likely won't go this high)
#define MIN_WHITE_RANGE         500   // Minimum ADC reading (in mV) for the white section (likely won't go this low)
#define MAX_WHITE_RANGE         1600  // Maximum ADC reading (in mV) for the white section
#define TOTAL_TICKS             12    // Total number of ticks/sections on the encoder
#define WHEEL_DIAMETER_CM       6     // Wheel's diameter in centimeters
#define WHEEL_DIAMETER_CM_BUGGY 8     // Wheel's diameter in centimeters
#define MAX_WHEEL_COUNT         5     // Wheel counter value until we check the wheel speeds (every 0.5s)

// Buffer Parameters
#define TASK_STACK_SIZE 2048
#define BUF_SIZE        16

// UDP Parameter
#define PORT 2222  // Port number for the ESP server (local port number)


enum section
{
  UNKNOWN,
  WHITE,
  BLACK
};



// Global Variables
bool isBuggy = true;            // Flag that tells if the car is the buggy (true) or the purple car (false)
bool update = false;             // Flag for running PID control
double distance = 0;             // Holds distance measured by the ultrasonic sensor
static ultrasonic_sensor_t dev;  // Structure containing ultrasonic sensor's trigger & echo pins
double distance_lidar = 0;       // Holds distance measured by the LIDAR v4 sensor
double error = 0;                // Tracks the error found from the PID control
double speed = 0;                // Overall speed of the car
double speed_L = 0;              // Speed of the purple car's left wheel
double speed_R = 0;              // Speed of the purple car's right wheel
int speed_val = 0;
int turn_val = 0;
bool blocked = false;            // Flags if an object is within 20cm of the front of the car
int wheel_count = 0;             // Counts number of 100ms timer interrupts until reach 500ms for adjusting wheel speeds
bool checkWheels = false;        // Flags if the purple car's left & right wheel speeds should be checked
bool remoteStop = false;         // Flags if a remote entity wants the car to stop (true)
char start = 0x1B;               // Start byte for UDP transmission
uint8_t* send_data;              // Buffer to hold data to be sent over UDP

// ADC Variables
static esp_adc_cal_characteristics_t *adc_chars;               // ADC characteristics pointer
static const adc_channel_t channel_decoder_L = ADC_CHANNEL_3;  // ADC1 GPI39  (A3)
static const adc_channel_t channel_decoder_R = ADC_CHANNEL_0;  // ADC1 GPI36  (A4)
static adc_bits_width_t width_bit = ADC_WIDTH_BIT_12;          // ADC width (# of bits to represent raw reading)
static const adc_atten_t atten = ADC_ATTEN_DB_11;              // ADC attenuation (scaling)
static const adc_unit_t unit = ADC_UNIT_1;                     // Specifies which ADC unit to use (1 or 2)

// Variables to record encoder countings
int prev_state_L = UNKNOWN;    // Tracks the previous state the left optical decoder was looking at
uint32_t tickCount_L = 0;      // Records # of ticks left decoder read during duration (1s from timer interrupts)
uint32_t lastTickCount_L = 0;  // Records previous # of ticks for the left wheel
int prev_state_R = UNKNOWN;    // Tracks the previous state the right optical decoder was looking at
uint32_t tickCount_R = 0;      // Records # of ticks right decoder read during duration (1s from timer interrupts)
uint32_t lastTickCount_R = 0;  // Records previous # of ticks for the right wheel


// Function that initializes the goods
static void init(void)
{
  // Initialize timer-based interrupts
  initialize_TimerQueue();

  // Initialize the MCPWM GPIO (defined in mcpwm_motor)
  if (isBuggy)
  {
    buggy_mcpwm_init();
    // mcpwm_config_t pwm_config;
    // pwm_config.frequency = 1000;                           // frequency = 1kHz,
    // pwm_config.cmpr_a = 0;                                 // duty cycle of PWMxA = 0
    // pwm_config.cmpr_b = 0;                                 // duty cycle of PWMxb = 0
    // pwm_config.counter_mode = MCPWM_UP_COUNTER;
    // pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    // mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);  // Configure PWM0A & PWM0B with above settings
    // mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);  // Configure PWM1A & PWM1B with above settings
    calibrateESC();            // uncomment if using buggy
  }
  else
  {
    mcpwm_motor_initialize();
  }

  // Allocate space for the data buffer
  send_data = (uint8_t*) malloc(BUF_SIZE);

  // Initialize the dev structure for the ultrasonic sensor
  dev.trigger_pin = TRIGGER_GPIO_PIN;
  dev.echo_pin = ECHO_GPIO_PIN;
  ultrasonic_init(&dev);

  // Initialize the ADC channels (use for Purple Car)
  check_efuse();
  config_ADC(unit, width_bit, ADC_ATTEN_DB_12, channel_decoder_L);  // ADC for left decoder
  //config_ADC(unit, width_bit, atten, channel_decoder_L);  // ADC for left decoder
  //config_ADC(unit, width_bit, atten, channel_decoder_R);  // ADC for right decoder

  // Characterize ADC
  adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(unit, atten, width_bit, DEFAULT_VREF, adc_chars);
  print_char_val_type(val_type);

  // Initialize the I2C Display
  i2c_example_master_init();
  i2c_scanner();
  if ( initialize_I2C() == ESP_OK )
  {
    printf("I2C Display successfully initialized!\n");
  }

  // Initiate the LIDAR v4 I2C controls
  //i2c_master_init_lidar();
  //i2c_scanner_lidar();

  // Initialize alarm using timer API
  alarm_init();

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



// UDP Server task. Receives data from UDP clients, and sends a response to them (server fob's ID).
static void udp_server_task(void* pvParameters)
{
  // Configure the data for the server to send
  uint8_t server_data[3];
  server_data[0] = start;
  server_data[1] = '\0';

  // Setup the server configuration object
  server_conf conf;
  conf.port = PORT;
  conf.data = server_data;
  conf.remoteStop = &remoteStop;

  // Wait and listen for clients to communicate with
  udp_server(conf, pvParameters);

  // If we get to this point (we shouldn't), then delete the task
  vTaskDelete(NULL);
}



static void sample_adc_decoders()
{
  uint32_t voltage_decoder_L;
  uint32_t voltage_decoder_R;

  while (1)
  {
    // Sample the ADC to get the left and right encoder readings (in mV)
    voltage_decoder_L = sample_ADC(unit, width_bit, channel_decoder_L, adc_chars);
    voltage_decoder_R = sample_ADC(unit, width_bit, channel_decoder_R, adc_chars);
    
    // Check the left decoder to see if its readings fit a particular color threshold
    if (voltage_decoder_L >= MIN_BLACK_RANGE && voltage_decoder_L <= MAX_BLACK_RANGE)
    {
      // Check if the previous state was white
      if (prev_state_L == WHITE)
      {
        // Increment the tick counter and change the state
        tickCount_L++;
        prev_state_L = BLACK;
      }
      else
      {
        // Record that the decoder is currently in the black region
        prev_state_L = BLACK;
      }
    }
    else if (voltage_decoder_L >= MIN_WHITE_RANGE && voltage_decoder_L <= MAX_WHITE_RANGE)
    {
      // Check if the previous state was black
      if (prev_state_L == BLACK)
      {
        // Increment the tick counter and change the state
        tickCount_L++;
        prev_state_L = WHITE;
      }
      else
      {
        // Record that the decoder is currently in the black region
        prev_state_L = WHITE;
      }
    }

    // Check the right decoder to see if its readings fit a particular color threshold
    if (voltage_decoder_R >= MIN_BLACK_RANGE && voltage_decoder_R <= MAX_BLACK_RANGE)
    {
      // Check if the previous state was white
      if (prev_state_R == WHITE)
      {
        // Increment the tick counter and change the state
        tickCount_R++;
        prev_state_R = BLACK;
      }
      else
      {
        // Record that the decoder is currently in the black region
        prev_state_R = BLACK;
      }
    }
    else if (voltage_decoder_R >= MIN_WHITE_RANGE && voltage_decoder_R <= MAX_WHITE_RANGE)
    {
      // Check if the previous state was black
      if (prev_state_R == BLACK)
      {
        // Increment the tick counter and change the state
        tickCount_R++;
        prev_state_R = WHITE;
      }
      else
      {
        // Record that the decoder is currently in the black region
        prev_state_R = WHITE;
      }
    }

    // Add a slight delay
    vTaskDelay(15 / portTICK_RATE_MS);
  }
}



/**
  Handles timer event to raise the update flag for reading temperature. Event flag raised every 100ms.
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

      // Update the wheel counter for the purple car
      // Note: Blocks checking the wheel count while the remote device orders the car to stop
      if (!isBuggy && !remoteStop)
      {
        wheel_count++;
        if (wheel_count >= MAX_WHEEL_COUNT)
        {
          checkWheels = true;  // Raise the check wheel flag
          wheel_count = 0;     // Reset the wheel_count counter
        }
      }
    }
  }
}



// Task that continuously displays the wheel speed on the I2C display.
static void i2c_display_task()
{
  // Display the current car speed
  displaySpeed(&speed);
}

// Note: can use the timer interrupt, as it triggers once every 100ms (removes the need to call vTaskDelay)
static void cruise_control_buggy(){
  double prev_error = 0;
  double integral   = 0;
  double derivative = 0;
  
  while(1){
    distance = ultrasonic_measure_distance(&dev, MAX_RANGE_CM);
    blocked = (distance <= SETPOINT_RANGE) ? true : false;
    lastTickCount_L = tickCount_L;

    // Reset the wheel tick counters
    tickCount_L = 0;

    // Purple Car speed check
    speed = calc_speed(WHEEL_DIAMETER_CM, lastTickCount_L, TOTAL_TICKS);  // Check the speed of the left wheel
    double output = pid_control(speed, SETPOINT_SPEED, &prev_error, &integral, &derivative);

    printf("buggy speed: %.2fm/s\n", speed);
    printf("WALL-E DISTANCE: %fm\n", distance);
    printf("BLOCKED: %d\n", blocked);
    if(blocked){
      brakeESC();
    } else {
      if(prev_error > 0){
        speed_val -= 20;
        forwardESC(speed_val);
      }
      if(prev_error < 0){
        speed_val += 20;
        forwardESC(speed_val);
      }
    }
    vTaskDelay(100 / portTICK_RATE_MS);
  }

// TODO: Determine what to do here based on Ryan's code
// Note: Won't need to call isBuggy, since the task will only be created if isBuggy is true
/*
  if (isBuggy)
  {
    // Buggy speed check
    speed = get_buggy_speed();

    // Calculate the error according to calculations from the proportional, integral, and differential methods
    error = SETPOINT - speed;
    integral = integral + error * dt;
    derivative = (error - previous_error) / dt;

    // Use the calculated error measurements to determine the output
    double output = KP * error + KI * integral + KD * derivative;
    previous_error = error;
  }
*/
}



static void cruise_control_pc()
{
  // Variables to hold the base left and right wheel duty cycles
  double duty_L = 90.0;
  double duty_R = 75.0;

  // PID variables for the wheel speed
  double prev_error_spd = 0;
  double integral_spd   = 0;
  double derivative_spd = 0;

  // PID variables for the side distance
  double prev_error_sided = 0;
  double integral_sided   = 0;
  double derivative_sided = 0;

  // Run the infinite while loop here to check for speed, distance, UDP commands, etc
  while (1)
  {
    // Check the distance, and adjust the block flag as necessary
    if (update)
    {
      // Sample the ultrasonic sensor
      distance = ultrasonic_measure_distance(&dev, MAX_RANGE_CM);

      if (distance <= SETPOINT_RANGE)
      {
        // Immediately raise the blocked flag
        blocked = true;
      }
      else
      {
        // Nothing is in the way, so turn off the blocked flag
        blocked = false;
      }

      update = false;
    }

    // Immediately stop the car if it is within 20cm of a target OR has been told to stop from someone remotely
    if (blocked || remoteStop)
    {
      stop();
    }
    // Check the wheel speeds, and adjust the frequency as necessary
    else if (checkWheels)
    {
      // Record the current LIDAR sensor reading
      distance_lidar = (double) distanceLLv4();

      // Record the tick counts for the period
      lastTickCount_L = tickCount_L;
      lastTickCount_R = tickCount_R;

      // Reset the wheel tick counters
      tickCount_L = 0;
      tickCount_R = 0;

      // Purple Car speed check
      speed_L = calc_speed(WHEEL_DIAMETER_CM, lastTickCount_L, TOTAL_TICKS);  // Check the speed of the left wheel
      speed_R = calc_speed(WHEEL_DIAMETER_CM, lastTickCount_R, TOTAL_TICKS);  // Check the speed of the right wheel
      speed = (speed_L + speed_R)/2.0;                                        // Check the average wheel speed

      // Run PID control on the wheel speeds
      double output_spd = pid_control(speed, SETPOINT_SPEED, &prev_error_spd, &integral_spd, &derivative_spd);
        
      // Run PID control on the side distance
      double output_sided = pid_control(distance_lidar/100, SETPOINT_SIDED, &prev_error_sided, &integral_sided, &derivative_sided);

      // Use the PID outputs to adjust the wheel duty cycles
      double proxy_L = duty_L + output_spd + output_sided;
      double proxy_R = duty_R + output_spd - output_sided;

      // Avoid going past 100% for the duty cycle
      if (proxy_L > 100)
      {
        proxy_L = 100;
      }
      else if (proxy_L < 0)
      {
        proxy_L = 0;
      }
      if (proxy_R > 100)
      {
        proxy_R = 100;
      }
      else if (proxy_R < 0)
      {
        proxy_R = 0;
      }

      // Drive the car forward
      drive_forward(proxy_L, proxy_R);

      checkWheels = false;
    }

    // Add a short delay to satisfy the Watchdog
    vTaskDelay(10 / portTICK_RATE_MS);
  }

}

// Task to move the car (buggy or purple car) according to its appropriate cruise control function
static void move_car(void *arg)
{
  // Check which cruise control task we should run for the car
  if (isBuggy)
  {
    // Call the buggy cruise control function
    cruise_control_buggy();
  }
  else
  {
    // 1. mcpwm gpio initialization
    // Note: rely on init to call mcpwm_motor_initialize()

    // 2. initial mcpwm configuration
    printf("Configuring Initial Parameters of mcpwm...\n");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000;                           // frequency = 500Hz,
    pwm_config.cmpr_a = 0;                                 // duty cycle of PWMxA = 0
    pwm_config.cmpr_b = 0;                                 // duty cycle of PWMxb = 0
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);  // Configure PWM0A & PWM0B with above settings
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);  // Configure PWM1A & PWM1B with above settings

    // Call the purple car cruise control function
    cruise_control_pc();
  }
}



void app_main(void)
{
  // Initialize everything
  init();

  // Create task to handle timber-based events
  xTaskCreate(timer_event_task, "timer_event_task", TASK_STACK_SIZE, NULL, configMAX_PRIORITIES, NULL);

  // Create UDP server task
  xTaskCreate(udp_server_task, "udp_server_task", TASK_STACK_SIZE*2, (void*)AF_INET, configMAX_PRIORITIES - 1, NULL);

  // Create I2C display task
  xTaskCreate(i2c_display_task, "i2c_display_task", TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 2, NULL);

  // Create ADC decoder sampling task
  xTaskCreate(sample_adc_decoders, "sample_adc_decoders", TASK_STACK_SIZE, NULL, configMAX_PRIORITIES - 3, NULL);

  // Create task to drive the car (the task checks if buggy or purple car)
  xTaskCreate(move_car, "move_car", TASK_STACK_SIZE*2, NULL, configMAX_PRIORITIES - 4, NULL);
}


