/*
 * WS2812B.h
 *
 * Created: 03.05.2018 20:28:20
 *  Author: Quenon
 */

#ifndef WS2812B_H_
#define WS2812B_H_

#include "furi.h"
#include <stdint.h>
#include <stdbool.h>
#include <input/input.h>

typedef struct {
    uint8_t version;
    uint8_t display_color_index;
    float display_brightness;
    bool settings_is_loaded;
} RGBBacklightSettings;

#define LED_PIN &gpio_ext_pa7
#define WS2812B_LEDS 3

void rgb_backlight_save_settings(void);

void rgb_backlight_update(uint8_t backlight);

void rgb_backlight_set_color(uint8_t color_index);
void rgb_backlight_set_color(uint8_t color_index);
void rgb_backlight_set_brightness(float brightness);

RGBBacklightSettings* rgb_backlight_get_settings(void);
uint8_t rgb_backlight_get_color_count(void);
const char* rgb_backlight_get_color_text(uint8_t index);

#endif /* WS2812B_H_ */