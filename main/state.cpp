#include <stdbool.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "state.hpp"
#include "driver/gpio.h"
#include "gpio_cxx.hpp"
#include <chrono>
#include <thread>
#include "globals.hpp"

using namespace idf;

static const char *TAG = "state";

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1
#define WIFI_MAXIMUM_RETRY 5

// ============== Member Methods ===============
bool State::is_wlan_connected(void)
{
    return is_wlan_connected_;
}

void State::set_is_wlan_connected(bool connected)
{
    is_wlan_connected_ = connected;
}

bool State::is_server_connected(void)
{
    return is_server_connected_;
}

void State::set_is_server_connected(bool connected)
{
    is_server_connected_ = connected;
}

state_t State::state(void)
{
    return state_;
}

void State::set_state(state_t new_state)
{
    state_ = new_state;
}

void State::do_buzzer()
{
    if (state_ == StateId::BUZZING)
    {
        ESP_LOGW(TAG, "Buzzer already active, ignoring new request");
        return;
    }
    state_ = StateId::BUZZING;

    // Cancel existing timer before creating new one
    if (buz_timer_)
    {
        buz_timer_.reset(); // Stop and destroy old timer
    }

    ESP_LOGI(TAG, "Buzzing for %d seconds", CONFIG_DORBUZZ_BUZZER_SECONDS);

    gpio_relay.set_high();
    buz_timer_ = std::make_unique<idf::esp_timer::ESPTimer>([]()
                                                            {
                                                auto& state = State::getInstance();
                                                std::lock_guard<std::mutex> lock_state(state.getMutex());
                                                gpio_relay.set_low();
                                                ESP_LOGI(TAG, "Buzzing ended");
                                                state.set_state(StateId::IDLE); });
    buz_timer_->start(std::chrono::seconds(CONFIG_DORBUZZ_BUZZER_SECONDS));
}

const char *State::state_str()
{
    switch (state_)
    {
    case StateId::IDLE:
        return "IDLE";
    case StateId::BUZZING:
        return "BUZZING";
    case StateId::REQUESTING:
        return "REQUESTING";
    default:
        return "UNKNOWN";
    }
}