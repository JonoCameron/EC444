/* ADC1 Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   Edited Jonathan Cameron, November 2020
*/
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling

int count = 0;
unsigned int aveVolt = 0;

static esp_adc_cal_characteristics_t *adc_charsl;
static esp_adc_cal_characteristics_t *adc_charsr;
#if CONFIG_IDF_TARGET_ESP32
static const adc_channel_t channell = ADC_CHANNEL_6;     //GPIO34 if ADC1 left, GPIO14 if ADC2
static const adc_channel_t channelr = ADC_CHANNEL_3;
static const adc_bits_width_t width = ADC_WIDTH_BIT_12;
#elif CONFIG_IDF_TARGET_ESP32S2
static const adc_channel_t channel = ADC_CHANNEL_6;     // GPIO7 if ADC1, GPIO17 if ADC2
static const adc_bits_width_t width = ADC_WIDTH_BIT_13;
#endif
static const adc_atten_t atten = ADC_ATTEN_DB_11;
static const adc_unit_t unitl = ADC_UNIT_1;
static const adc_unit_t unitr = ADC_UNIT_1;

bool thresholdl = false;
bool thresholdr = true;

int counterl = 0;
int counterr = 0;

#define TOTAL_TICKS 20
#define CIRCUMFERENCE 0.1869

void init()
{ 
  // Initialize the I2C display
  // i2c_example_master_init();
  // i2c_scanner();
  // if ( initialize_I2C() == ESP_OK )
  // {
  //   printf("I2C Display successfully initialized!\n");
  // }
}


static void check_efuse(void)
{
#if CONFIG_IDF_TARGET_ESP32
    //Check if TP is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("eFuse Two Point: NOT supported\n");
    }
    //Check Vref is burned into eFuse
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK) {
        printf("eFuse Vref: Supported\n");
    } else {
        printf("eFuse Vref: NOT supported\n");
    }
#elif CONFIG_IDF_TARGET_ESP32S2
    if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK) {
        printf("eFuse Two Point: Supported\n");
    } else {
        printf("Cannot retrieve eFuse Two Point calibration values. Default calibration values will be used.\n");
    }
#else
#error "This example is configured for ESP32/ESP32S2."
#endif
}


static void print_char_val_type(esp_adc_cal_value_t val_type)
{
    if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP) {
        printf("Characterized using Two Point Value\n");
    } else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF) {
        printf("Characterized using eFuse Vref\n");
    } else {
        printf("Characterized using Default Vref\n");
    }
}

static void i2c_display_aveVolt()
{
  //displayAveVolt(&aveVolt);
  while(1){
    vTaskDelay(500 / portTICK_RATE_MS);
  }
}

void app_main(void)
{
    //Check if Two Point or Vref are burned into eFuse
    init();

    check_efuse();

    //Running sum for average.
    int oneSecond = 0;
    float tpsl = 0; // ticks per second left
    float speedl = 0;

    float tpsr = 0;
    float speedr = 0;

    //Configure ADC
    if (unitl == ADC_UNIT_1) {
        adc1_config_width(width);
        adc1_config_channel_atten(channell, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channell, atten);
    }

    if (unitr == ADC_UNIT_1) {
        adc1_config_width(width);
        adc1_config_channel_atten(channelr, atten);
    } else {
        adc2_config_channel_atten((adc2_channel_t)channelr, atten);
    }

    //Characterize ADC
    adc_charsl = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_typel = esp_adc_cal_characterize(unitl, atten, width, DEFAULT_VREF, adc_charsl);
    print_char_val_type(val_typel);

    adc_charsr = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_value_t val_typer = esp_adc_cal_characterize(unitl, atten, width, DEFAULT_VREF, adc_charsr);
    print_char_val_type(val_typer);


    //Continuously sample ADC1
    while (1) {
        uint32_t adc_readingl = 0;
        uint32_t adc_readingr = 0;
        //Multisampling
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            if (unitl == ADC_UNIT_1) {
                adc_readingl += adc1_get_raw((adc1_channel_t)channell);
                adc_readingr += adc1_get_raw((adc1_channel_t)channelr);
            } else {
                int raw;
                adc2_get_raw((adc2_channel_t)channell, width, &raw);
                adc_readingl += raw;
            }
        }
        adc_readingl /= NO_OF_SAMPLES;
        adc_readingr /= NO_OF_SAMPLES;
        //Convert adc_reading to voltage in mV
        uint32_t voltagel = esp_adc_cal_raw_to_voltage(adc_readingl, adc_charsl);
        uint32_t voltager = esp_adc_cal_raw_to_voltage(adc_readingr, adc_charsr);
        

        // If voltage indicates a solid bit in slotted plate and the threshold flag is false, count a tick
        if( (voltagel > 2850) && thresholdl == false ){
            counterl++;
            thresholdl = true;
        }
        else if( voltagel < 2800 ){
            thresholdl = false;
        }

        if ( (voltager > 2800) && thresholdr == false ){
            counterr++;
            thresholdr = true;
        }
        else if( voltager < 2800 ){
            thresholdr = false;
        }
        
        // Count 1 second, when one second is up, reset counter and the second timer.
        if( oneSecond < 100 ){
            oneSecond++;
        }
        else{
            tpsl = counterl;
            tpsr = counterr;
            oneSecond = 0;
            counterl = 0;
            counterr = 0;
        }

        // Calculate speed from the fraction of a revolution and multiply this fraction by the distance of one revolution.
        speedl = (float) tpsl / TOTAL_TICKS * CIRCUMFERENCE;
        speedr = (float) tpsr / TOTAL_TICKS * CIRCUMFERENCE;

        printf("Voltage LEFT: %dmV\tVoltage RIGHT: %dmV\tSpeed LEFT: %.2fm/s\tSpeed RIGHT: %.2fm/s\n",
                 voltagel, voltager, speedl, speedr);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
