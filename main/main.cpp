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
#include "driver/gpio.h"
#include "esp_timer_cxx.hpp"
#include "state.hpp"
#include "wifi.hpp"
#include "globals.hpp"
#include "leds.hpp"
#include "mqtt.hpp"
#include "state_led_controller.hpp"
#include "webprov.hpp"

// TODO: CPU to 80MHZ

#define TAG "dbz-main"
static constexpr uint8_t BRIGHT_DURATION_SECS = CONFIG_DORBUZZ_DISPLAY_ON_SECONDS;

static void blink_green_task(void *pvParameters)
{
    auto &leds = Leds::getInstance();
    while (true)
    {
        leds.wlan_led().blink(200);
    }
}

static void blink_yellow_task(void *pvParameters)
{
    auto &leds = Leds::getInstance();
    while (true)
    {
        leds.server_led().blink(200);
    }
}

static void force_provisioning_cb(void *args, void *user_data)
{
    ESP_LOGW(TAG, "Long press detected - starting provisioning mode");
    auto &mqtt = Mqtt::getInstance();
    mqtt.stop();

    auto &leds = Leds::getInstance();
    leds.wlan_led().turn_off();
    leds.buzzing_led().turn_off();

    xTaskCreate(blink_yellow_task, "blink_yellow", 2048, nullptr, 1, nullptr);

    auto &prov = WebProv::getInstance();
    prov.start_provisioning();
}

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

    // Check provisioning status - blocks and reboots if not provisioned
    auto &prov = WebProv::getInstance();
    prov.on_prov_start = [&state_led_ctrl](const std::string &ap_ssid)
    {
        ESP_LOGI(TAG, "Provisioning started. Connect to WiFi: %s", ap_ssid.c_str());
        auto &leds = state_led_ctrl.leds();
        std::lock_guard<std::mutex> lock(leds.getMutex());
        leds.jingle();
    };
    prov.init();

    // Register long-press on button to force re-provisioning
    state_led_ctrl.register_long_press(force_provisioning_cb);

    auto &wifi = Wifi::getInstance();
    ret = wifi.wifi_connect();
    if (ret == ESP_OK)
    {
        ret = wifi.time_sync();
        if (ret != ESP_OK)
        {
            ESP_LOGW(TAG, "Could not sync time with time server");
        }

        auto &mqtt = Mqtt::getInstance();
        mqtt.connect();
    }
    else
    {
        ESP_LOGW(TAG, "WiFi connect failed - blinking green LED. Long press button to start provisioning.");
        auto &leds = Leds::getInstance();
        leds.server_led().turn_off();
        leds.buzzing_led().turn_off();
        xTaskCreate(blink_green_task, "blink_green", 2048, nullptr, 1, nullptr);
    }
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