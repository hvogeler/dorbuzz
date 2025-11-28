#ifndef STATE_H
#define STATE_H

#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "esp_timer_cxx.hpp"
#include <mutex>

typedef enum class StateId
{
    IDLE,
    BUZZING,
    REQUESTING,
} state_t;

/**
 * @brief Singleton class managing global application state for the door buzzer system.
 *
 * This class implements the Meyers' Singleton pattern with thread-safe initialization.
 * It manages WiFi connection status, server connection status, and buzzer state,
 * providing mutex-protected access to shared state across FreeRTOS tasks.
 *
 * @note Thread-safe singleton instance creation guaranteed by C++11.
 * @note All state modifications should be protected using lock()/unlock().
 */
class State
{
private:
    bool is_wlan_connected_ = false;   ///< WiFi connection status
    bool is_server_connected_ = false; ///< Server connection status
    state_t state_ = StateId::IDLE;    ///< Current application state
    mutable std::mutex mutex_;
    std::unique_ptr<idf::esp_timer::ESPTimer> buz_timer_;

public:
    State(const State &) = delete;
    State &operator=(const State &) = delete;
    std::mutex &getMutex() { return mutex_; }
    /**
     * @brief Get the singleton instance of State.
     * @return Reference to the singleton State instance.
     */
    static State &getInstance()
    {
        static State instance_;
        return instance_;
    }

    /**
     * @brief Activate the buzzer by creating a buzzer task.
     * @note Sets state to BUZZING if not already active.
     */
    void do_buzzer();

    /**
     * @brief Get the configured WiFi SSID.
     * @return Pointer to SSID string.
     */
    const char *get_wlan_ssid(void);

    /**
     * @brief Get the configured WiFi password.
     * @return Pointer to password string.
     */
    const char *get_wlan_password(void);

    /**
     * @brief Get the current application state.
     * @return Current state value (IDLE, BUZZING, or REQUESTING).
     */
    state_t state(void);

    /**
     * @brief Set the application state.
     * @param new_state New state value to set.
     */
    void set_state(state_t new_state);

    /**
     * @brief Get WiFi connection status.
     * @return true if connected to WiFi, false otherwise.
     */
    bool is_wlan_connected(void);

    /**
     * @brief Set WiFi connection status.
     * @param connected Connection status to set.
     */
    void set_is_wlan_connected(bool connected);

    /**
     * @brief Get server connection status.
     * @return true if connected to server, false otherwise.
     */
    bool is_server_connected(void);

    /**
     * @brief Set server connection status.
     * @param connected Connection status to set.
     */
    void set_is_server_connected(bool connected);

    /**
     * @brief Get string representation of current state.
     * @return Pointer to state string ("IDLE", "BUZZING", "REQUESTING", or "UNKNOWN").
     */
    const char *state_str();

    /**
     * @brief Acquire the state mutex with timeout.
     * @param timeout_ms Timeout in milliseconds.
     * @param loc Source location identifier for logging.
     * @return true if lock acquired, false on timeout.
     */
    // bool lock(int timeout_ms, const char *loc);

    /**
     * @brief Release the state mutex.
     */
    // void unlock();

private:
    State() {}
};

#endif
