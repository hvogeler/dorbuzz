# Dorbuzz

This firmware makes an esp32 s3 trigger the door buzzer of an apartment building.
It subscribes to an mqtt topic. If it receives a do_buzz message, it triggers a relay by
setting a gpio high.

This relay is connected to the Elcom door intercom system via a Elcom RED114Y.
The RED114Y is configured to open the front door.

This firmware does **NOT** impose any security restrictions. Anyone who is allowed to send
the message over the mqtt broker can trigger the door open buzzer. Given that he nows the
broker address, broker credentials and the topic name. Instead this firmware relies on the mqtt
infrastructure itself to be secured!

# Disclaimer

This is a personal fun project not meant to be used outside my own apartement.
Feel free use it at your own risk.

# Configure Credentials

Dorbuzz needs credentials

- to connect to wifi
- to connect to the mqtt broker

The firmware expectes these credentials in NVS ram. This means at some point we need to flash them to NVS before the firmware reads them.

## Configure Values

Copy the nvs_template.csv to nvs_creds.csv.

```
key,type,encoding,value
config,namespace,,
wifi_ssid,data,string,YOUR_WIFI_SSID
wifi_password,data,string,YOUR_WIFI_PASSWORD
mqtt_broker,data,string,mqtt://YOUR_BROKER:1883
mqtt_user,data,string,YOUR_USER
mqtt_password,data,string,YOUR_PASSWORD
```

Replace the placeholdes with the actual credentials.

Now run `./flash_nvs.sh`. You might need to adjust the serial port.
On my MAC it is `/dev/cu.usbmodem1101`.

This flashes the credentials to the devices NVS where they will stay until the flash is cleared.
Firmware updates will not clear the credentials.
