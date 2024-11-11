#ifndef _HEADER_ALPICOOL_CONFIG
#define _HEADER_ALPICOOL_CONFIG

// wifi SSID
#define  WIFI_SSID "ssid" 
// wifi password
#define  WIFI_PASS "pass"
// mqtt server. "192.168.0.1" also works
#define  MQTT_SERVER "mymqtt.mydomain.cz"
//mqtt user. anonynymous mqqt brokers are not supported
#define  MQTT_USER "mqtt_user"
//mqtt password
#define  MQTT_PASSWORD "mqtt_password"
//Ble adress of the fridge
#define  FRIDGE_BLE_ADDRESS  "ff:22:12:07:01:b4"

//the rest you can keep defult velues
//device_name, used as mqtt client_id
#define  DEVICE_NAME "alpicool-esp32"
//topic to send state data
#define  SENSOR_TOPIC  "tele/alpicool/sensor"
//topic to subscribe for control commnads (on, off, set temperature)
#define  COMMAND_TOPIC "tele/alpicool/cmd"
//topic to publish debug messages
#define  DEBUG_TOPIC  "tele/alpicool/debug"

//how often are state data send to mqtt
#define  DATA_PUBLISH_FREQ_MILLIS 1000
#endif
