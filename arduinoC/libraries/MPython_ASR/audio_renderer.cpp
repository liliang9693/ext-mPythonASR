#include <stdbool.h>
#include <string.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "freertos/FreeRTOS.h"

#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/i2s.h"
#include "audio_renderer.h"
#define TAG "renderer"

#define I2S_USE_NUM I2S_NUM_0
#define USE_I2S_FORMAT        I2S_CHANNEL_FMT_ONLY_RIGHT 
#define USE_I2S_CHANNEL_NUM   ((USE_I2S_FORMAT < I2S_CHANNEL_FMT_ONLY_RIGHT) ? (2) : (1))

renderer_config_t *renderer_instance = NULL;

static void init_i2s(renderer_config_t *config)
{
    i2s_config_t i2s_config = {
        .mode = i2s_mode_t((I2S_MODE_MASTER | I2S_MODE_RX| I2S_MODE_ADC_BUILT_IN)),
        .sample_rate = config->sample_rate,
        .bits_per_sample = config->bit_depth,
        .channel_format = config->channel_format,     
        .communication_format = I2S_COMM_FORMAT_I2S_MSB,
        .intr_alloc_flags =ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 16,
        .dma_buf_len = 64,
        .use_apll = config->use_apll,
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    i2s_set_adc_mode(ADC_UNIT_1, ADC1_CHANNEL_2);

    
}

void renderer_zero_dma_buffer()
{
    i2s_zero_dma_buffer(I2S_USE_NUM);
}

renderer_config_t *renderer_get()
{
    return renderer_instance;
}

void renderer_init(renderer_config_t *config)
{
    if (renderer_instance == NULL)
        renderer_instance = (renderer_config_t*)calloc(1, sizeof(renderer_config_t));
    renderer_instance->bit_depth  = config->bit_depth;
    renderer_instance->sample_rate = config->sample_rate;
    renderer_instance->mode = config->mode; 
    renderer_instance->use_apll = config->use_apll; 
    renderer_instance->channel_format = config->channel_format;
    renderer_instance->i2s_channal_nums = (renderer_instance->channel_format < I2S_CHANNEL_FMT_ONLY_RIGHT) ? (2) : (1);
    renderer_instance->adc1_channel = config->adc1_channel;
    init_i2s(config);
}

void renderer_start()
{
    i2s_start(I2S_USE_NUM);
    i2s_zero_dma_buffer(I2S_USE_NUM);
}

void renderer_stop()
{
    i2s_stop(I2S_USE_NUM);
    i2s_zero_dma_buffer(I2S_USE_NUM);
}

void renderer_deinit()
{
    if(renderer_instance != NULL)
    {
        i2s_stop(I2S_USE_NUM);
        i2s_driver_uninstall(I2S_USE_NUM);
        free(renderer_instance);
        renderer_instance = NULL;
    }
}

void renderer_adc_enable()
{
    esp_err_t flag;
    flag =  i2s_adc_enable(I2S_USE_NUM);
    if(flag == ESP_ERR_INVALID_ARG){
        printf("ESP_ERR_INVALID_ARG \n");
    }
    if(flag == ESP_ERR_INVALID_STATE){
        printf("ESP_ERR_INVALID_STATE \n");
    }
}

void renderer_adc_disable()
{
    i2s_adc_disable(I2S_USE_NUM);
}

void renderer_set_clk(uint32_t rate, i2s_bits_per_sample_t bits, i2s_channel_t ch)
{
    i2s_set_clk(I2S_USE_NUM, rate, bits, ch);
}

void renderer_read_raw(uint8_t *buff, uint32_t len)
{
    size_t bytes_read;
    i2s_read(I2S_USE_NUM, (void *)buff, len, &bytes_read, portMAX_DELAY);
}

