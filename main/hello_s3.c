#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "cubic_curve.h"
#include "db_utils.h"
#include <math.h>

static const char *TAG = "HELLO_S3";

#define LED_GPIO            (48)
#define SAMPLE_RATE         (48000)
#define FADE_MILLISECONDS   (6)
#define FADE_SAMPLES        (SAMPLE_RATE * FADE_MILLISECONDS / 1000)
#define I2S_BCLK            GPIO_NUM_17
#define I2S_LRCLK           GPIO_NUM_18
#define I2S_DOUT            GPIO_NUM_16

static void play_tone(float freqHz, int ms, float gainDb, float fadeTimeMs)
{

    ESP_LOGI(TAG, "play_tone()");
    CubicCurve cc;

    setCubicCurve( &cc, 0.0f, 1.0f, 0, FADE_SAMPLES);
    
    const float freq = freqHz;

    const float gainRatio = dbToRatio(gainDb, -120.0f);
    const int samples = SAMPLE_RATE * ms / 1000;
    int16_t buffer[256];

    i2s_chan_handle_t tx_chan;
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_new_channel(&chan_cfg, &tx_chan, NULL);

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCLK,
            .ws   = I2S_LRCLK,
            .dout = I2S_DOUT,
            .din  = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    i2s_channel_init_std_mode(tx_chan, &std_cfg);
    i2s_channel_enable(tx_chan);

    float c = 0.0f;
    int n = sizeof(buffer) / sizeof(buffer[0]);
    for (int i = 0; i < samples; ) {
        
        if (i + n > samples) n = samples - i;
        for (int j = 0; j < n; j++) {
            c = nextCubicCurveValue(&cc);

            //ESP_LOGI(TAG, "curve value: %f", c);

            buffer[j] = (int16_t)(gainRatio * sin(2 * M_PI * freq * (i + j) / SAMPLE_RATE) * 32767) * c;

            if (i+j == FADE_SAMPLES)
            {
                ESP_LOGI(TAG, "setCubicCurve");
                setCubicCurve(&cc, 1.0f, 0.0f, samples - FADE_SAMPLES - FADE_SAMPLES, FADE_SAMPLES);
            }
        }
        size_t written;
        i2s_channel_write(tx_chan, buffer, n * sizeof(int16_t), &written, portMAX_DELAY);
        i += n;
    }

    for (int i=0; i< 10; i++)
    {
        // Write a silence buffer at the end to eliminate pops on close.
        for (int j = 0; j < n; j++) {
            buffer[j] = 0;
        }
        size_t written;
        i2s_channel_write(tx_chan, buffer, n * sizeof(int16_t), &written, portMAX_DELAY);
    }

    ESP_LOGI(TAG, "Final curve value: %f", c);


    i2s_channel_disable(tx_chan);
    i2s_del_channel(tx_chan);
}

void app_main(void)
{
    // LED setup
    led_strip_handle_t strip;
    led_strip_config_t strip_cfg = {
        .strip_gpio_num = LED_GPIO,
        .max_leds = 1,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
    };
    led_strip_rmt_config_t rmt_cfg = { .resolution_hz = 10 * 1000 * 1000 };
    led_strip_new_rmt_device(&strip_cfg, &rmt_cfg, &strip);
    led_strip_clear(strip);

    while (1) {
        // Red
        led_strip_set_pixel(strip, 0, 64, 0, 0);
        led_strip_refresh(strip);
        play_tone(700, 200, -40.0f, 50);
        vTaskDelay(pdMS_TO_TICKS(500));

        // Green
        led_strip_set_pixel(strip, 0, 0, 64, 0);
        led_strip_refresh(strip);
        play_tone(700, 200, -40.0f, 50);
        vTaskDelay(pdMS_TO_TICKS(500));

        // Blue
        led_strip_set_pixel(strip, 0, 0, 0, 64);
        led_strip_refresh(strip);
        play_tone(700, 200, -40.0f, 50);
        vTaskDelay(pdMS_TO_TICKS(500));

        // Off
        led_strip_clear(strip);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
