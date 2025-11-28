#include <string>
#include "mqtt.hpp"
#include "esp_mac.h"
#include "esp_log.h"
#include "nvs.hpp"
#include "state.hpp"

static std::string TAG = "dbz_mqtt";

static void mqtt5_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

esp_err_t Mqtt::connect()
{
    // Get mqtt url and credentials
    Nvs nvs_creds;
    esp_err_t ret = nvs_creds.open_namespace("config");
    if (ret == ESP_OK)
    {
        nvs_creds.read("mqtt_broker", mqtt_broker_);
        nvs_creds.read("mqtt_user", mqtt_user_);
        nvs_creds.read("mqtt_password", mqtt_password_);
        mqtt5_cfg.broker.address.uri = mqtt_broker_.c_str();
        mqtt5_cfg.credentials.username = mqtt_user_.c_str();
        mqtt5_cfg.credentials.authentication.password = mqtt_password_.c_str();
        ESP_LOGI(TAG.c_str(), "Connecting to Mqtt broker: %s", mqtt_broker_.c_str());
    }
    else
    {
        ESP_LOGE(TAG.c_str(), "Reading Mqtt credentials from NVS failed");
    }

    ESP_LOGI(TAG.c_str(), "Connecting to Mqtt broker: %s", mqtt5_cfg.broker.address.uri);
    mqtt_client = esp_mqtt_client_init(&mqtt5_cfg);
    if (!mqtt_client)
    {
        return ESP_FAIL;
    }

    ret = esp_mqtt_client_register_event(mqtt_client, static_cast<esp_mqtt_event_id_t>(-1), mqtt5_event_handler, NULL);
    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = esp_mqtt_client_start(mqtt_client);
    if (ret != ESP_OK)
    {
        return ret;
    }

    return ESP_OK;
}

void Mqtt::publish(std::string json_doc)
{
    if (!mqtt_client)
    {
        ESP_LOGE(TAG.c_str(), "Not publishing cur_temp because mqtt client not initialized");
        return;
    }
    msg_id = esp_mqtt_client_publish(mqtt_client, buzz_topic.c_str(), json_doc.c_str(), 0, 1, 0);
}

void Mqtt::publish_log(std::string log_topic, std::string json_doc)
{
    if (!mqtt_client)
    {
        ESP_LOGE(TAG.c_str(), "Not publishing log because mqtt client not initialized");
        return;
    }
    msg_id = esp_mqtt_client_publish(mqtt_client, log_topic.c_str(), json_doc.c_str(), 0, 1, 0);
}

void Mqtt::subscribe()
{
    if (!mqtt_client)
    {
        ESP_LOGE(TAG.c_str(), "Not subscribing because mqtt client not initialized");
        return;
    }
    msg_id = esp_mqtt_client_subscribe(mqtt_client, buzz_topic.c_str(), 1);
    if (msg_id >= 0)
    {
        ESP_LOGI(TAG.c_str(), "Subscribed to topic %s", buzz_topic.c_str());
    }
    else
    {
        ESP_LOGE(TAG.c_str(), "Subscribe to topic %s failed", buzz_topic.c_str());
    }
}

static void mqtt5_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG.c_str(), "Event dispatched from event loop base=%s, event_id=%" PRIi32, base, event_id);
    esp_mqtt_event_handle_t event = static_cast<esp_mqtt_event_handle_t>(event_data);
    esp_mqtt_client_handle_t client = event->client;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
    {
        ESP_LOGI(TAG.c_str(), "MQTT_EVENT_CONNECTED");
        auto &mqtt = Mqtt::getInstance();
        mqtt.set_connect_return_code(MQTT_CONNECTION_ACCEPTED);
        mqtt.set_is_mqtt_connected(true);
        mqtt.subscribe();
        break;
    }
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG.c_str(), "MQTT_EVENT_DISCONNECTED");
        State::getInstance().set_is_server_connected(false);
        ESP_LOGI(TAG.c_str(), "Attempting to reconnect to MQTT broker...");
        esp_mqtt_client_reconnect(client);
        break;
    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG.c_str(), "MQTT_EVENT_SUBSCRIBED");
        State::getInstance().set_is_server_connected(true);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG.c_str(), "MQTT_EVENT_UNSUBSCRIBED");
        State::getInstance().set_is_server_connected(false);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG.c_str(), "MQTT_EVENT_PUBLISHED");
        break;
    case MQTT_EVENT_DATA:
    {
        ESP_LOGI(TAG.c_str(), "MQTT_EVENT_DATA");
        auto len = event->data_len;
        std::string msg(static_cast<const char *>(event->data), len);
        ESP_LOGI(TAG.c_str(), "Received Message: %s", msg.c_str());
        State::getInstance().do_buzzer();
        break;
    }
    case MQTT_EVENT_ERROR:
    {
        ESP_LOGI(TAG.c_str(), "MQTT_EVENT_ERROR");
        ESP_LOGI(TAG.c_str(), "MQTT5 return code is %d", event->error_handle->connect_return_code);
        auto &mqtt = Mqtt::getInstance();
        mqtt.set_connect_return_code(event->error_handle->connect_return_code);
        break;
    }
    default:
        ESP_LOGI(TAG.c_str(), "Other event id:%d", event->event_id);
        break;
    }
}
