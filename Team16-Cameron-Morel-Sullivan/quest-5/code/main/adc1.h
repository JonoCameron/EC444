/*
  Header file for ADC1 functions. Adapted from the esp-idf adc1 example project.
  --> https://github.com/espressif/esp-idf/tree/master/examples/peripherals/adc

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Nov. 2020
*/

#ifndef ADC1_H
#define ADC1_H


#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#define DEFAULT_VREF    1100        //Use adc2_vref_to_gpio() to obtain a better estimate
#define NO_OF_SAMPLES   64          //Multisampling


/**
  Checks if TP or Vref is burned into eFuse.
  \param None
  \return None
**/
void check_efuse(void);


/**
  Checks what value type is used for characterization (eFuse TP, eFuse Vref, or default).
  \param esp_adc_cal_value_t val_type --> Value type to check.
  \return None
**/
void print_char_val_type(esp_adc_cal_value_t val_type);


/**
  Configures ADC based on select parameters.
  \param adc_unit_t unit --> ADC Unit (1 or 2). Note that ADC2 is connected to Wi-Fi.
  \param adc_bits_width_t width_bit --> Bit width for the ADC.
  \param adc_atten_t atten --> Attenuation (scale) for the ADC.
  \param adc_channel_t channel --> Channel to use for ADC communication (see GPIO pins).
  \return None
**/
void config_ADC(adc_unit_t unit, adc_bits_width_t width_bit, adc_atten_t atten, adc_channel_t channel);


/**
  Samples a particular ADC channel for its voltage. Assumes the global "unit" variable applies to all ADC channels.
  \param adc_unit_t unit --> ADC Unit (1 or 2). Note that ADC2 is connected to Wi-Fi.
  \param adc_bits_width_t width_bit --> Bit width for the ADC.
  \param adc_channel_t channel --> ADC channel to sample.
  \param esp_adc_cal_characteristics_t* adc_chars --> ADC characteristics pointer.
  \return ADC's voltage reading (mV) as a uint32_t value.
**/
uint32_t sample_ADC(adc_unit_t unit, adc_bits_width_t width_bit, adc_channel_t channel,
                    esp_adc_cal_characteristics_t* adc_chars);


#endif
