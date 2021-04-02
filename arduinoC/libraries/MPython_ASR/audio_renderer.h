#ifndef INCLUDE_AUDIO_RENDERER_H_
#define INCLUDE_AUDIO_RENDERER_H_

#include "freertos/FreeRTOS.h"
#include "driver/i2s.h"
#include "driver/adc.h"

#define RENDERER_DEFAULT(){                     \
        .bit_depth  = I2S_BITS_PER_SAMPLE_16BIT,    \
        .sample_rate = 16000,                       \
        .mode = I2S_MODE_TX,                \
};
    
typedef struct
{
    i2s_mode_t mode;
    int sample_rate;
    i2s_bits_per_sample_t bit_depth;
    i2s_channel_fmt_t  channel_format;
    bool use_apll;
    adc1_channel_t adc1_channel;
    uint8_t i2s_channal_nums;
    i2s_pin_config_t i2s_pin;
} renderer_config_t;

typedef enum
{
    PCM_INTERLEAVED, PCM_LEFT_RIGHT
} pcm_buffer_layout_t;

typedef enum
{
    PCM_BIG_ENDIAN, PCM_LITTLE_ENDIAN
} pcm_endianness_t;

typedef struct
{
    uint32_t sample_rate;
    i2s_bits_per_sample_t bit_depth;
    uint8_t num_channels;
    pcm_buffer_layout_t buffer_format;
    pcm_endianness_t endianness;
} pcm_format_t;

void renderer_init(renderer_config_t *config);
void renderer_start();
void renderer_stop();
void renderer_deinit();

void renderer_zero_dma_buffer();
renderer_config_t *renderer_get();

void renderer_adc_enable();
void renderer_adc_disable();
void renderer_set_clk(uint32_t rate, i2s_bits_per_sample_t bits, i2s_channel_t ch);
void renderer_read_raw(uint8_t *buff, uint32_t len);

#endif
