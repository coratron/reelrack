#ifndef RGB_H
#define RGB_H

esp_err_t configure_led(uint32_t numLeds);
void show_led(uint32_t ledIndex, uint8_t red, uint8_t green, uint8_t blue);
void boot_sequence(uint8_t red, uint8_t green, uint8_t blue);

#endif // RGB_H
