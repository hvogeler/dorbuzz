#pragma once
#include <cstdlib>
#include <stdexcept>
#include "iot_button.h"
#include "button_gpio.h"

using CallBackFunc = void(void *, void *);

class IdfButton
{
    button_handle_t handle;

public:
    IdfButton(int32_t gpio_num, uint8_t active_level, bool enable_power_save, bool disable_pull)
    {
        button_config_t config = {
            .long_press_time = 0,
            .short_press_time = 0,
        };
        button_gpio_config_t gpio_config = {
            .gpio_num = gpio_num,
            .active_level = active_level,
            .enable_power_save = enable_power_save,
            .disable_pull = disable_pull};

        esp_err_t ret = iot_button_new_gpio_device(&config, &gpio_config, &handle);
        if (ret != ESP_OK || handle == nullptr)
        {
            throw std::runtime_error("Button could not be initialized on this GPIO");
        }
    }

    ~IdfButton()
    {
        if (handle != nullptr)
        {
            iot_button_delete(handle);
        }
    }

    // Delete copy constructor and assignment operator
    IdfButton(const IdfButton &) = delete;
    IdfButton &operator=(const IdfButton &) = delete;

    // Delete move constructor and assignment operator (button handle shouldn't be moved)
    IdfButton(IdfButton &&) = delete;
    IdfButton &operator=(IdfButton &&) = delete;

    void on_single_click(CallBackFunc f, void *user_data = nullptr)
    {
        register_handler(BUTTON_SINGLE_CLICK, nullptr, f, user_data);
    }

    void on_double_click(CallBackFunc f, void *user_data = nullptr)
    {
        register_handler(BUTTON_DOUBLE_CLICK, nullptr, f, user_data);
    }

    void register_handler(button_event_t event, button_event_args_t *args, CallBackFunc f, void *user_data = nullptr)
    {
        esp_err_t ret = iot_button_register_cb(handle, event, args, f, user_data);
        if (ret != ESP_OK)
        {
            throw std::runtime_error("Button handler not registered");
        }
    }
};
