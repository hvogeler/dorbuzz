#ifndef LEDS_HPP
#define LEDS_HPP

#include <stdint.h>
#include <mutex>
#include "driver/gpio.h"
#include "gpio_cxx.hpp"
#include "led.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "globals.hpp"

/**
 * @brief Singleton class for managing LEDs.
 *
 * This class provides centralized LED control using the Meyers Singleton pattern.
 *
 * @note Thread-safe singleton instance creation guaranteed by C++11.
 */
class Leds
{
    Led wlan_led_;
    Led server_led_;
    Led buzzing_led_;
    mutable std::mutex mutex_; ///< Mutex for thread-safe state access

private:
    // Private constructor
    Leds() : wlan_led_(DORBUZZ_GPIO_LED_WLAN), server_led_(DORBUZZ_GPIO_LED_SERVER), buzzing_led_(DORBUZZ_GPIO_LED_BUZZING)
    {
    }

    // Destructor
    ~Leds()
    {
    }

    // Delete copy constructor and assignment operator
    Leds(const Leds &) = delete;
    Leds &operator=(const Leds &) = delete;

public:
    /**
     * @brief Get the singleton instance of Leds.
     * @return Reference to the singleton Leds instance.
     */
    static Leds &getInstance()
    {
        static Leds instance;
        return instance;
    }
    std::mutex &getMutex() { return mutex_; }
    
    // Add LED control methods here
    void jingle();

    void turn_leds_off()
    {
        if (wlan_led_.state() == LedState::on)
        {
            wlan_led_.turn_off();
        }
        if (server_led_.state() == LedState::on)
        {
            server_led_.turn_off();
        }
        if (buzzing_led_.state() == LedState::on)
        {
            buzzing_led_.turn_off();
        }
    }

    Led &wlan_led()
    {
        return wlan_led_;
    }

    Led &server_led()
    {
        return server_led_;
    }

    Led &buzzing_led()
    {
        return buzzing_led_;
    }
};

#endif // LEDS_HPP
