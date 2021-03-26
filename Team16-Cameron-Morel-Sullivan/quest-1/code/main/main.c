#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "i2c_display.h"
#include "driver/gpio.h"
#include "timer.h"
#include "servo.h"



// Define the port number for the button input
#define BUTTON_GPIO 27  // A10 is pin 27
#define SERVO_GPIO  18  // Set GPIO 18 as PWM0A for servo connection

// Define the highest number we can count up to
// Note: The offical fish feeder feeds fish every 12 hours (43200 seconds) and uses displayCount_HoursMinutes().
//       For short demos, the fish feeder feeds fish every 1 minute 23 seconds (83 seconds) and uses displayCount_MinutesSeconds().
#define TIMER_MAX_COUNT 43200 // Fish are fed every 12 hours (43200 seconds). For short demos, use 83 seconds.



// Global Variables
unsigned int count = 0;  // Set count to an arbitrary value (will become 0 when start/stop button is pressed)
bool en_count = false;   // Disable counting by default, so start/stop button required to start the feeder
bool en_servo = false;   // Flag for activating the servo



/**
  ISR handler for the button. Note that pressing the button starts and stops the feeder.
  \param None
  \return None
**/
static void IRAM_ATTR gpio_isr_handler(void *arg)
{
  // Flip the enable count flag, and reset count
  en_count = !en_count;
  count = 0;
}



/**
  Calls initialization functions for the binary counter and I2C display.
  \param None
  \return None
**/
void init()
{
  // Initialize timer-based interrupts
  initialize_TimerQueue();

  // Initialize the button GPIO pin as a hardware interrupt
  gpio_reset_pin(BUTTON_GPIO);                                // Reset GPIO state (enable pullup & disable IO)
  gpio_set_direction(BUTTON_GPIO, GPIO_MODE_INPUT);           // Set button GPIO pin as input
  gpio_set_intr_type(BUTTON_GPIO, GPIO_INTR_POSEDGE);         // Setup GPIO to interrupt on pos edge
  gpio_intr_enable(BUTTON_GPIO);                              // Enable interrupt on IO pin
  gpio_install_isr_service(ESP_INTR_FLAG_LEVEL3);            // Install ISR
  gpio_isr_handler_add(BUTTON_GPIO, gpio_isr_handler, NULL);  // Hook ISR to GPIO pin
  printf("Initialized button for hardware interrupts\n");

  // Initialize the I2C display
  i2c_example_master_init();
  i2c_scanner();
  if ( initialize_I2C() == ESP_OK )
  {
    printf("I2C Display successfully initialized!\n");
  }

  // Initialize servo GPIO
  initialize_servo(SERVO_GPIO);
  printf("Servo GPIO initialized!\n");
}



/**
  Updates the current count variable.
  \param None
  \return None
**/
static void update_count()
{
  if (count > 0)
  {
    count--;
  }
  else
  {
    // Reached or exceeded the MAX_COUNT, so wrap back to 0
    count = TIMER_MAX_COUNT;
  }

}



/**
  Handles timer events such as updating the count and activating the servo enable flag.
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
    if (evt.flag == 1 && en_count)
    {
      // Update the count
      update_count();

      // Activate the servo flag if it is time to feed
      if (count == 0)
      {
        en_servo = true;
      }
    }
  }
}



/**
  Displays the current countdown timer.
  \param None
  \return None
**/
static void i2c_display_count()
{
  // Display the current count value
  displayCount_HoursMinutes(&count);  // For Fish Feeder product
  //displayCount_MinutesSeconds(&count);  // For quick demo
}



/**
  Moves the servo one full rotation to dump the fish food.
  \param None
  \return None
**/
static void servo_control()
{
  // Loop forever to check if the servo needs to rotate
  while (1)
  {
    // Check if the flag to feed the fish was raised
    if (en_servo)
    {
      uint32_t currentDegree = 0;
      uint32_t nextDegree = 0;

      // Move the servo to the default location (just in case)
      servo_rotateTo(0, 300);

      // Shake the food container 3 times
      // Note that the recommended delay at 5V is 100ms per 60degrees
      for (unsigned int i = 0; i < 3; i++)
      {
        // Rotate counter-clockwise
	nextDegree = SERVO_MAX_DEGREE/2 + 45;
        servo_rotateTo( nextDegree, (nextDegree - currentDegree) * 100/60 );
	currentDegree = nextDegree;

        // Rotate clockwise
	nextDegree = SERVO_MAX_DEGREE/2 - 45;
	servo_rotateTo( nextDegree, (currentDegree - nextDegree) * 100/60 );
	currentDegree = nextDegree;
      }

      // Move the servo back to the default location
      nextDegree = 0;
      servo_rotateTo( nextDegree, (currentDegree - nextDegree) * 100/60 );

      // Disable the flag to mark that the task has been completed
      en_servo = false;
    }

    // Wait some time to reduce power and keep the Watchdog happy. (Happy watchdog noises)
    vTaskDelay(100);
  }
}



void app_main(void)
{
  // Initialize GPIO pins and configures the button hardware interrupt
  init();

  // Create task to handle timer-based events
  xTaskCreate(timer_event_task, "timer_event_task", 2048, NULL, configMAX_PRIORITIES, NULL);

  // Create I2C display task
  xTaskCreate(i2c_display_count, "i2c_display_count", 2048, NULL, configMAX_PRIORITIES - 1, NULL);

  // Create the servo control task
  xTaskCreate(servo_control, "servo_control", 2048, NULL, configMAX_PRIORITIES - 2, NULL);

  // Initialize alarm using timer API
  alarm_init();
}

