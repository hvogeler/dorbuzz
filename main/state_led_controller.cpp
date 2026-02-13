
#include "state_led_controller.hpp"
#include "wifi.hpp"
#include "esp_log.h"

void StateLedController::state_watcher_task(void *p)
{
    ESP_LOGI(TAG, "starting state_watcher_task");

    auto *slctrl = static_cast<StateLedController *>(p);
    auto &state = slctrl->state();
    auto &leds = slctrl->leds();
    leds.jingle();

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(200));
        if (!slctrl->is_leds_on())
        {
            std::lock_guard<std::mutex> lock_leds(leds.getMutex());
            leds.turn_leds_off();
            continue;
        }

        auto &wifi = Wifi::getInstance();
        std::lock_guard<std::mutex> lock_wifi(wifi.getMutex());
        std::lock_guard<std::mutex> lock_state(state.getMutex());

        std::lock_guard<std::mutex> lock_leds(leds.getMutex());

        if (wifi.get_is_connected() && leds.wlan_led().state() == LedState::off)
        {
            leds.wlan_led().turn_on();
        }

        if (!wifi.get_is_connected() && leds.wlan_led().state() == LedState::on)
        {
            leds.wlan_led().turn_off();
        }

        if (state.is_server_connected() && leds.server_led().state() == LedState::off)
        {
            leds.server_led().turn_on();
        }

        if (!state.is_server_connected() && leds.server_led().state() == LedState::on)
        {
            leds.server_led().turn_off();
        }

        if (state.state() == StateId::BUZZING && leds.buzzing_led().state() == LedState::off)
        {
            leds.buzzing_led().turn_on();
        }

        if (state.state() != StateId::BUZZING && leds.buzzing_led().state() == LedState::on)
        {
            leds.buzzing_led().turn_off();
        }
    }
}

void StateLedController::btn_handler_single(void *args, void *user_data)
{
    auto *slctrl = static_cast<StateLedController *>(user_data);
    slctrl->toggle_is_leds_on();
}
