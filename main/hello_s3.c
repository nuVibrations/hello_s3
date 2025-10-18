#include "led_strip.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_GPIO 48  // built-in WS2812 RGB LED on DevKitC-1-N8R8

void app_main(void)
{
    led_strip_handle_t led_strip;

    led_strip_config_t strip_config = {
        .strip_gpio_num = LED_GPIO,
        .max_leds = 1,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB, // LED_STRIP_COLOR_COMPONENT_FORMAT_GRB,
    };

    led_strip_rmt_config_t rmt_config = {
        .resolution_hz = 10 * 1000 * 1000,  // 10 MHz
    };

    // create LED driver instance
    led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
    led_strip_clear(led_strip);

    while (1) {
        // Red
        led_strip_set_pixel(led_strip, 0, 64, 0, 0);
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(500));

        // Green
        led_strip_set_pixel(led_strip, 0, 0, 64, 0);
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(500));

        // Blue
        led_strip_set_pixel(led_strip, 0, 0, 0, 64);
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(500));

        // Off
        led_strip_clear(led_strip);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}
