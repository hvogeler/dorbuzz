#include "sdkconfig.h"
#include "leds.hpp"
#include "esp_log.h"

static const char *TAG = "Leds";

void Leds::jingle()
{
    wlan_led().turn_on();
    vTaskDelay(pdMS_TO_TICKS(500));
    server_led().turn_on();
    vTaskDelay(pdMS_TO_TICKS(500));
    buzzing_led().turn_on();
    vTaskDelay(pdMS_TO_TICKS(500));

    wlan_led().turn_off();
    server_led().turn_off();
    buzzing_led().turn_off();

    wlan_led().turn_on();
    vTaskDelay(pdMS_TO_TICKS(800));
    wlan_led().turn_off();

    server_led().turn_on();
    vTaskDelay(pdMS_TO_TICKS(800));
    server_led().turn_off();

    buzzing_led().turn_on();
    vTaskDelay(pdMS_TO_TICKS(800));
    buzzing_led().turn_off();

    vTaskDelay(pdMS_TO_TICKS(500));

    for (int i = 0; i < 3; i++)
    {
        wlan_led().turn_on();
        server_led().turn_on();
        buzzing_led().turn_on();
        vTaskDelay(pdMS_TO_TICKS(400));
        wlan_led().turn_off();
        server_led().turn_off();
        buzzing_led().turn_off();
        vTaskDelay(pdMS_TO_TICKS(400));
    }
}
