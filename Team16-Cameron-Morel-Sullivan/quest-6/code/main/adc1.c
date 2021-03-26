/*
  ADC function implementations. Adapted from the esp-idf adc1 example project.
  --> https://github.com/espressif/esp-idf/tree/master/examples/peripherals/adc

  Jonathan Cameron, DJ Morel, Ryan Sullivan, Dec. 2020
*/


#include "adc1.h"



void check_efuse(void)
{
  // Check TP is burned into eFuse
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_TP) == ESP_OK)
  {
    printf("eFuse Two Point: Supported\n");
  }
  else
  {
    printf("eFuse Two Point: NOT supported\n");
  }

  // Check Vref is burned into eFuse
  if (esp_adc_cal_check_efuse(ESP_ADC_CAL_VAL_EFUSE_VREF) == ESP_OK)
  {
    printf("eFuse Vref: Supported\n");
  }
  else
  {
    printf("eFuse Vref: NOT supported\n");
  }
}



void print_char_val_type(esp_adc_cal_value_t val_type)
{
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
  {
    printf("Characterized using Two Point Value\n");
  }
  else if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
  {
    printf("Characterized using eFuse Vref\n");
  }
  else
  {
    printf("Characterized using Default Vref\n");
  }
}



void config_ADC(adc_unit_t unit, adc_bits_width_t width_bit, adc_atten_t atten, adc_channel_t channel)
{
  // Configure ADC
  if (unit == ADC_UNIT_1)
  {
    adc1_config_width(width_bit);
    adc1_config_channel_atten(channel, atten);
  }
  else
  {
    adc2_config_channel_atten((adc2_channel_t)channel, atten);
  }
}



uint32_t sample_ADC(adc_unit_t unit, adc_bits_width_t width_bit, adc_channel_t channel,
                    esp_adc_cal_characteristics_t* adc_chars)
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
      adc2_get_raw((adc2_channel_t)channel, width_bit, &raw);
      adc_reading += raw;
    }
  }
  adc_reading /= NO_OF_SAMPLES;

  // Convert adc_reading to voltage in mV
  uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);

  return voltage;
}

