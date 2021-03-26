/*
  Adapted I2C example code to work with the Adafruit ADXL343 accelerometer. Ported and referenced a lot of code from the Adafruit_ADXL343 driver code.
  ----> https://www.adafruit.com/product/4097

  Emily Lam, Aug 2019 for BU EC444
  Jonathan Cameron October 2020
*/

#include "./lidar.h"
#include "./pid.h"
#include "./timer.h"

// Timer task 
static void timer_evt_task(void *arg) {
    while (1) {
        // Create dummy structure to store structure from queue
        timer_event_t evt;

        // Transfer from queue
        xQueueReceive(timer_queue, &evt, portMAX_DELAY);

        // Do something if triggered!
        if (evt.flag == 1 && D1 == 1) {
            takeD2 = 1;
            //evt.flag = 0;
        }
    }
}

void app_main() {

  // Routine
  i2c_master_init();
  i2c_scanner();
  timerQueue_init();
  alarm_init();
  gpio_init();


  // Check for LLv4, this is essentially useless because the LLv4 doesn't have an inherent ID like the accelerometer.
  uint8_t deviceID = 0;
  //getDeviceID(&deviceID);
  if (deviceID == 0x62) {
    printf("\n>> Found Garmin LiDAR v4\n");
  }

  // Create task to handle timer-based events
  xTaskCreate(timer_evt_task, "timer_evt_task", 2048, NULL, 5, NULL);

  // Create task to poll LLv4
  xTaskCreate(LLv4,"LLv4", 4096, NULL, 5, NULL);
}
