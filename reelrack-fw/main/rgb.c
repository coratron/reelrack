#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "esp_log.h"
#include "esp_err.h"
#include "rgb.h"
#include <sys/time.h>

static const char *TAG = "rgb";

led_strip_handle_t led_strip;
led_strip_config_t led_strip_config;
struct timeval sysTime;
bool inverLedDirection = false;
uint32_t nReelsPerRow;
uint8_t nRows;

esp_err_t configure_led(uint32_t numReelsPerRow, uint8_t numRows, bool ledDirection)
{
    // LED strip general initialization, according to your led board design
    led_strip_config.strip_gpio_num = LED_STRIP_GPIO;               // The GPIO that connected to the LED strip's data line
    led_strip_config.max_leds = numReelsPerRow * (uint32_t)numRows; // The number of LEDs in the strip
    led_strip_config.led_pixel_format = LED_PIXEL_FORMAT;           // Pixel format of your LED strip
    led_strip_config.led_model = LED_TYPE;                          // LED strip model
    led_strip_config.flags.invert_out = false;                      // whether to invert the output signal

    inverLedDirection = ledDirection;
    nReelsPerRow = numReelsPerRow;
    nRows = numRows;

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
    // turn all LEDs off
    ESP_ERROR_CHECK(led_strip_clear(led_strip));

    // Calculate the row and column of the LED
    uint32_t row = ledIndex / nReelsPerRow;
    uint32_t col = ledIndex % nReelsPerRow;

    // Check if the row is odd
    bool isOddRow = row % 2 == 1;

    // Calculate the adjusted LED index based on the zigzag pattern
    uint32_t adjustedIndex;
    if (isOddRow)
    {
        adjustedIndex = (row * nReelsPerRow) + (nReelsPerRow - col - 1);
    }
    else
    {
        adjustedIndex = (row * nReelsPerRow) + col;
    }

    // consider the direction of the LED strip
    if (inverLedDirection)
    {
        adjustedIndex = (nReelsPerRow * nRows) - adjustedIndex - 1;
    }

    // Set the target LED using the adjusted index
    ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, adjustedIndex, red, green, blue));

    if (red == 0 && green == 0 && blue == 0)
    {
        ESP_LOGI(TAG, "LED OFF!");
        ESP_ERROR_CHECK(led_strip_clear(led_strip));
    }
    else
    {
        ESP_LOGI(TAG, "LED %d ON with colour %d %d %d!", ledIndex, red, green, blue);
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
    }

    // record the time of the last update
    gettimeofday(&sysTime, NULL);
}

void boot_sequence(uint8_t red, uint8_t green, uint8_t blue)
{
    // just draw a nice loop, one LED at a time
    for (int i = 0; i < led_strip_config.max_leds; i++)
    {
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, red, green, blue));
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));
        vTaskDelay(pdMS_TO_TICKS(10));
        ESP_ERROR_CHECK(led_strip_clear(led_strip));
    }
}

void turn_off_leds_on_timeout(long timeout)
{
    static struct timeval now;
    gettimeofday(&now, NULL);

    // when the timer has expired (timer must be set in show_led)
    if (now.tv_sec > sysTime.tv_sec + timeout / 1000)
    {
        ESP_LOGI(TAG, "LED timeout");
        // turn all LEDs off
        ESP_ERROR_CHECK(led_strip_clear(led_strip));
    }
}

void show_error_led(uint8_t red, uint8_t green, uint8_t blue)
{
    // turn all LEDs off
    ESP_ERROR_CHECK(led_strip_clear(led_strip));

    // set the target LED, and flip the colours so it's clear
    ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, 0, 255 - red, 255 - green, 255 - blue));
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));

    // record the time of the last update
    gettimeofday(&sysTime, NULL);
}