#ifndef GLOBALS
#define GLOBALS

#include <stdint.h>
#include "driver/gpio.h"
#include "gpio_cxx.hpp"

static constexpr uint32_t DORBUZZ_GPIO_RELAY = 4;
static constexpr uint32_t DORBUZZ_GPIO_LED_WLAN = 1; // D18
static constexpr uint32_t DORBUZZ_GPIO_LED_SERVER = 2;
static constexpr uint32_t DORBUZZ_GPIO_LED_BUZZING = 3; // D18
static constexpr uint32_t DORBUZZ_GPIO_LED_BUTTON = 9;  // D18

const idf::GPIO_Output gpio_relay{idf::GPIONum(DORBUZZ_GPIO_RELAY)};

#endif