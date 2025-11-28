#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>
#include <esp_log.h>
#include <esp_timer.h>
#include <math.h>
#include <esp_task_wdt.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iot_button.h"
#include "button_gpio.h"
#include "driver/gpio.h"
#include "esp_timer_cxx.hpp"
#include "state.hpp"
#include "wifi.hpp"
#include "button.hpp"
#include "globals.hpp"
#include "leds.hpp"
#include "mqtt.hpp"
#include "state_led_controller.hpp"

// TODO: CPU to 80MHZ

#define TAG "dbz-main"
static constexpr uint8_t BRIGHT_DURATION_SECS = CONFIG_DORBUZZ_DISPLAY_ON_SECONDS;

using namespace std;
using namespace idf;
using namespace idf::esp_timer;

extern "C" void app_main(void)
{
    // Initialize NVS. It will be used in various parts of the firmware
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGW(TAG, "Erasing nvs partition");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set("dbz-mqtt", ESP_LOG_VERBOSE);
    esp_log_level_set("dbz-main", ESP_LOG_VERBOSE);

    StateLedController state_led_ctrl;

    auto &wifi = Wifi::getInstance();
    ESP_ERROR_CHECK(wifi.wifi_connect());

    ret = wifi.time_sync();
    if (ret != ESP_OK)
    {
        ESP_LOGW(TAG, "Could not sync time with time server");
    }

    auto &mqtt = Mqtt::getInstance();
    mqtt.connect();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));

        // heap_trace_dump();
        ESP_LOGD(TAG, "Free heap: %lu bytes", esp_get_free_heap_size());
        // stack size dump
        BaseType_t remaining_stack = uxTaskGetStackHighWaterMark(NULL);
        ESP_LOGD(TAG, "Free stack: %lu bytes", remaining_stack * sizeof(StackType_t));
    }
}