/*
  Header file for configuring a WiFi station. Code adapted from the official 
  esp-idf Espressif WiFi station example.
  --> https://github.com/espressif/esp-idf/tree/master/examples/wifi/getting_started/station

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Oct. 2020
*/
#ifndef WIFISTATION_H
#define WIFISTATION_H
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"


/* The examples use WiFi configuration that you can set via project configuration menu
   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_MAXIMUM_RETRY  CONFIG_ESP_MAXIMUM_RETRY


/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1


// FreeRTOS event group to signal when we are connected
static EventGroupHandle_t s_wifi_event_group;

// Tracks the number of times we've attempted to establish the wifi connection
static int s_retry_num = 0;

// Tag for ESP_LOGI
static const char *TAG = "wifi station";
//static const char* TAG = "wifi_station";


// Handles a FreeRTOS event
void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

/**
  Initializes WiFi station. Configures and calls event_handler() according to FreeRTOS events.
  \param None
  \return None
**/
void wifi_init_sta(void);


#endif
