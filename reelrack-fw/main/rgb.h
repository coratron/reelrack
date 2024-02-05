#ifndef RGB_H
#define RGB_H

#define LED_STRIP_GPIO 2
#define LED_STRIP_RMT_RES_HZ (10 * 1000 * 1000) // 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define LED_TYPE LED_MODEL_SK6812
#define LED_PIXEL_FORMAT LED_PIXEL_FORMAT_GRBW
#define LED_TIMEOUT (30 * 1000000) // in us

esp_err_t configure_led(uint32_t numLeds);
void show_led(uint32_t ledIndex, uint8_t red, uint8_t green, uint8_t blue);
void boot_sequence(uint8_t red, uint8_t green, uint8_t blue);
void turn_off_leds_on_timeout(long timeout);
void show_error_led(uint8_t red, uint8_t green, uint8_t blue);

#endif // RGB_H
