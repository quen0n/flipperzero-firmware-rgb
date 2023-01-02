/*
 * WS2812B.c
 *
 * Created: 03.05.2018 20:28:37
 *  Author: Quenon
 */
#include "WS2812B.h"
#include <string.h>
#include <stm32wbxx.h>
#include "furi_hal_light.h"
#include <furi.h>
#include <furi_hal.h>
#include <storage/storage.h>
#include <input/input.h>

#define TAG "RGB Backlight"
#define RGB_BACKLIGHT_SETTINGS_VERSION 3
#define RGB_BACKLIGHT_SETTINGS_FILE_NAME ".rgb_backlight.settings"
#define RGB_BACKLIGHT_SETTINGS_PATH EXT_PATH(RGB_BACKLIGHT_SETTINGS_FILE_NAME)

static uint8_t WS2812B_ledbuffer[WS2812B_LEDS][3];

static RGBBacklightSettings rgb_settings = {
    .version = RGB_BACKLIGHT_SETTINGS_VERSION,
    .display_brightness = 1.0f,
    .display_color_index = 0,
    .settings_is_loaded = false};

#define COLOR_COUNT (sizeof(color_value) / sizeof(uint32_t))
const char* color_text[] = {
    "Orange",
    "Yellow",
    "Lime",
    "Olive",
    "Green",
    "Teal",
    "Blue",
    "Aqua",
    "Fuchsia",
    "Red",
    "White"};
const uint32_t color_value[] = {
    //R G B
    0xFF4500, //Orange
    0xFFFF00, //Yellow
    0x00FF00, //Lime
    0x808000, //Olive
    0x008000, //Green
    0x008080, //Teal
    0x0000FF, //Blue
    0x00FFFF, //Aqua
    0xFF00FF, //Fuchsia
    0xFF0000, //Red
    0xFFFFE0, //White
};

void WS2812B_send(void) {
    furi_kernel_lock();
    /* Последовательная отправка цветов светодиодов */
    for(uint8_t lednumber = 0; lednumber < WS2812B_LEDS; lednumber++) {
        //Последовательная отправка цветов светодиода
        for(uint8_t color = 0; color < 3; color++) {
            //Последовательная отправка битов цвета
            for(uint8_t i = 7; i != 255; i--) {
                if(WS2812B_ledbuffer[lednumber][color] & (1 << i)) {
                    furi_hal_gpio_write(LED_PIN, true);
                    uint32_t start = DWT->CYCCNT;
                    while((DWT->CYCCNT - start) < 31) {
                    }
                    furi_hal_gpio_write(LED_PIN, false);
                    start = DWT->CYCCNT;
                    while((DWT->CYCCNT - start) < 15) {
                    }
                } else {
                    furi_hal_gpio_write(LED_PIN, true);
                    uint32_t start = DWT->CYCCNT;
                    while((DWT->CYCCNT - start) < 15) {
                    }
                    furi_hal_gpio_write(LED_PIN, false);
                    start = DWT->CYCCNT;
                    while((DWT->CYCCNT - start) < 31) {
                    }
                }
            }
        }
    }
    furi_kernel_unlock();
}

static void _port_init(void) {
    furi_hal_gpio_init(LED_PIN, GpioModeOutputPushPull, GpioPullNo, GpioSpeedVeryHigh);
}

uint8_t rgb_backlight_get_color_count(void) {
    return COLOR_COUNT;
}

const char* rgb_backlight_get_color_text(uint8_t index) {
    return color_text[index];
}

static void rgb_backlight_load_settings(void) {
    _port_init();
    RGBBacklightSettings settings;
    File* file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
    const size_t settings_size = sizeof(RGBBacklightSettings);

    FURI_LOG_I(TAG, "loading settings from \"%s\"", RGB_BACKLIGHT_SETTINGS_PATH);
    bool fs_result =
        storage_file_open(file, RGB_BACKLIGHT_SETTINGS_PATH, FSAM_READ, FSOM_OPEN_EXISTING);

    if(fs_result) {
        uint16_t bytes_count = storage_file_read(file, &settings, settings_size);

        if(bytes_count != settings_size) {
            fs_result = false;
        }
    }

    if(fs_result) {
        FURI_LOG_I(TAG, "load success");
        if(settings.version != RGB_BACKLIGHT_SETTINGS_VERSION) {
            FURI_LOG_E(
                TAG,
                "version(%d != %d) mismatch",
                settings.version,
                RGB_BACKLIGHT_SETTINGS_VERSION);
        } else {
            furi_kernel_lock();
            memcpy(&rgb_settings, &settings, settings_size);
            furi_kernel_unlock();
        }
    } else {
        FURI_LOG_E(TAG, "load failed, %s", storage_file_get_error_desc(file));
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
    rgb_settings.settings_is_loaded = true;
};

void rgb_backlight_save_settings(void) {
    RGBBacklightSettings settings;
    File* file = storage_file_alloc(furi_record_open(RECORD_STORAGE));
    const size_t settings_size = sizeof(RGBBacklightSettings);

    FURI_LOG_I(TAG, "saving settings to \"%s\"", RGB_BACKLIGHT_SETTINGS_PATH);

    furi_kernel_lock();
    memcpy(&settings, &rgb_settings, settings_size);
    furi_kernel_unlock();

    bool fs_result =
        storage_file_open(file, RGB_BACKLIGHT_SETTINGS_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS);

    if(fs_result) {
        uint16_t bytes_count = storage_file_write(file, &settings, settings_size);

        if(bytes_count != settings_size) {
            fs_result = false;
        }
    }

    if(fs_result) {
        FURI_LOG_I(TAG, "save success");
    } else {
        FURI_LOG_E(TAG, "save failed, %s", storage_file_get_error_desc(file));
    }

    storage_file_close(file);
    storage_file_free(file);
    furi_record_close(RECORD_STORAGE);
};

RGBBacklightSettings* rgb_backlight_get_settings(void) {
    if(!rgb_settings.settings_is_loaded) {
        rgb_backlight_load_settings();
    }
    return &rgb_settings;
}

void rgb_backlight_set_color(uint8_t color_index) {
    rgb_settings.display_color_index = color_index;
}
void rgb_backlight_set_brightness(float brightness) {
    rgb_settings.display_brightness = brightness;
}

void rgb_backlight_update(uint8_t backlight) {
    if(!rgb_settings.settings_is_loaded) {
        rgb_backlight_load_settings();
    }
    for(uint8_t i = 0; i < WS2812B_LEDS; i++) {
        //Green
        WS2812B_ledbuffer[i][0] =
            ((color_value[rgb_settings.display_color_index] & 0x00FF00) >> 8) *
            (backlight / 255.0f);
        //Red
        WS2812B_ledbuffer[i][1] =
            (color_value[rgb_settings.display_color_index] >> 16) * (backlight / 255.0f);
        //Blue
        WS2812B_ledbuffer[i][2] =
            (color_value[rgb_settings.display_color_index] & 0xFF) * (backlight / 255.0f);
    }

    WS2812B_send();
}
