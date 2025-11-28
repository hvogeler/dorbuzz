#ifndef STATE_LED_CONTROLLER
#define STATE_LED_CONTROLLER

#include "state.hpp"
#include "leds.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "button.hpp"
#include "esp_log.h"
#include <atomic>

static const char *TAG = "StateLedController";

class StateLedController
{
    State &state_;
    Leds &leds_;
    std::atomic<bool> is_leds_on_{true};
    static constexpr const char *TAG = "StateLedController";
    TaskHandle_t task_handle_;
    std::unique_ptr<IdfButton> btn_;

public:
    StateLedController() : state_(State::getInstance()), leds_(Leds::getInstance()), task_handle_(nullptr)
    {
        btn_ = std::make_unique<IdfButton>(DORBUZZ_GPIO_LED_BUTTON, 0, false, false);
        btn_->on_single_click(btn_handler_single, this);

        xTaskCreate(state_watcher_task, "state_watcher_task", 1024 * 4, this, 1, &task_handle_);
    }

    ~StateLedController()
    {
        if (task_handle_ != nullptr)
        {
            vTaskDelete(task_handle_);
            task_handle_ = nullptr;
        }
    }

    static void state_watcher_task(void *p);
    static void btn_handler_single(void *args, void *user_data);

    Leds &leds()
    {
        return leds_;
    }

    State &state()
    {
        return state_;
    }

    void toggle_is_leds_on()
    {
        is_leds_on_ = !is_leds_on_;
    }

    bool is_leds_on()
    {
        return is_leds_on_;
    }
};

#endif