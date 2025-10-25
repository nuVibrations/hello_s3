#include "driver/i2s_std.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>

#define LED_GPIO        48
#define SAMPLE_RATE     44100
#define I2S_BCLK        GPIO_NUM_17
#define I2S_LRCLK       GPIO_NUM_18
#define I2S_DOUT        GPIO_NUM_16

typedef struct {
	float d0;
	float d1;
	float d2;
	float d3;

	float target;
	size_t startCounter;
	size_t stopCounter;
} CubicCurve;

void setCubicCurve(CubicCurve* cc,
                   float startTarget,
                   float stopTarget,
                   int   preDelaySamples,
                   int   curveSamples)
{
    if (curveSamples <= 0) curveSamples = 1;

    const float h = 1.0f / (float)curveSamples;
    const float delta = stopTarget - startTarget;

    // Cubic coefficients in normalized domain [0,1]
    const float a = startTarget;
    const float b = 1.0f;
    const float c = 3.0f * (delta - 1.0f);
    const float d = -2.0f * (delta - 1.0f);

    // Convert to forward-difference coefficients
    cc->d0 = a;
    cc->d1 = b * h + c * h * h + d * h * h * h;
    cc->d2 = 2.0f * c * h * h + 6.0f * d * h * h * h;
    cc->d3 = 6.0f * d * h * h * h;

    cc->target       = stopTarget;
    cc->startCounter = preDelaySamples > 0 ? preDelaySamples : 0;
    cc->stopCounter  = curveSamples > 0 ? curveSamples : 1;
}

float nextCubicCurveValue( CubicCurve * cc)
{
	float d0 = cc->d0;

	if (cc->startCounter > 0)
	{
		cc->startCounter--;
	}
	else if (cc->stopCounter > 0)
	{
		cc->d0 += cc->d1;
		cc->d1 += cc->d2;
		cc->d2 += cc->d3;
		
		cc->stopCounter--;
	}

	return d0;
}


float dbToRatio(float db, float floor)
{
	if (db == floor)
	{
		return 0.0f;
	}

	float ratio = pow(10, db/20.0f);
	return ratio;
}

float ratioToDb(float ratio, float floor)
{
	if (0.0 == ratio)
	{
		return floor;
	}

	float dB = 20.0f * log(ratio);
	if (dB < floor)
	{
		dB = floor;
	}

	return dB;
}

static void play_tone(float freqHz, int ms, float gainDb, float fadeTimeMs)
{
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

    for (int i = 0; i < samples; ) {
        int n = sizeof(buffer) / sizeof(buffer[0]);
        if (i + n > samples) n = samples - i;
        for (int j = 0; j < n; j++)
            buffer[j] = (int16_t)(gainRatio * sin(2 * M_PI * freq * (i + j) / SAMPLE_RATE) * 32767);
        size_t written;
        i2s_channel_write(tx_chan, buffer, n * sizeof(int16_t), &written, portMAX_DELAY);
        i += n;
    }

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
        play_tone(700, 200, -30.0f, 50);
        vTaskDelay(pdMS_TO_TICKS(500));

        // Green
        led_strip_set_pixel(strip, 0, 0, 64, 0);
        led_strip_refresh(strip);
        play_tone(700, 200, -30.0f, 50);
        vTaskDelay(pdMS_TO_TICKS(500));

        // Blue
        led_strip_set_pixel(strip, 0, 0, 0, 64);
        led_strip_refresh(strip);
        play_tone(700, 200, -30.0f, 50);
        vTaskDelay(pdMS_TO_TICKS(500));

        // Off
        led_strip_clear(strip);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
