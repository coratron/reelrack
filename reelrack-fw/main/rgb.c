#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "esp_log.h"
#include "esp_err.h"

// GPIO assignment
#define LED_STRIP_BLINK_GPIO 2
// 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define LED_STRIP_RMT_RES_HZ (10 * 1000 * 1000)

#define LED_TYPE LED_MODEL_SK6812

static const char *TAG = "example";

led_strip_handle_t led_strip;
led_strip_config_t led_strip_config;

esp_err_t configure_led(uint32_t numLeds)
{
    // LED strip general initialization, according to your led board design
    led_strip_config.strip_gpio_num = LED_STRIP_BLINK_GPIO;   // The GPIO that connected to the LED strip's data line
    led_strip_config.max_leds = numLeds;                      // The number of LEDs in the strip
    led_strip_config.led_pixel_format = LED_PIXEL_FORMAT_GRB; // Pixel format of your LED strip
    led_strip_config.led_model = LED_TYPE;                    // LED strip model
    led_strip_config.flags.invert_out = false;                // whether to invert the output signal

    // LED strip backend configuration: RMT
    led_strip_rmt_config_t rmt_config = {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
        .rmt_channel = 0,
#else
        .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
        .resolution_hz = LED_STRIP_RMT_RES_HZ, // RMT counter clock frequency
        .flags.with_dma = false,               // DMA feature is available on ESP target like ESP32-S3
#endif
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&led_strip_config, &rmt_config, &led_strip));
    ESP_LOGI(TAG, "Created LED strip object with RMT backend");

    // REVIEW:
    return ESP_OK;
}

void show_led(uint32_t ledIndex, uint8_t red, uint8_t green, uint8_t blue)
{
    ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, ledIndex, red, green, blue));

    if (red == 0 && green == 0 && blue == 0)
    {
        ESP_LOGI(TAG, "LED OFF!");
        ESP_ERROR_CHECK(led_strip_clear(led_strip));
    }
    else
    {
        ESP_LOGI(TAG, "LED ON!");
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
    }
}

void boot_sequence(uint8_t red, uint8_t green, uint8_t blue)
{
    // just draw a nice loop, one LED at a time
    for (int i = 0; i < led_strip_config.max_leds; i++)
    {
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, red, green, blue));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(100));
        ESP_ERROR_CHECK(led_strip_clear(led_strip));
    }
}