#ifndef ZMK_MQTT_H
#define ZMK_MQTT_H

#include <PubSubClient.h>
#include "WiFi.h"

char * get_json_data(char * ret_buf);

class MqttHandler {
public:
    MqttHandler(void (*c)(char*, uint8_t*, unsigned int));
    void loop();
    void publishData();
    void publish(const char * topic, const char * message);
private:
    unsigned long last_connection_attempt;
    unsigned long last_publish;
    unsigned long last_loop;
    WiFiClient *espClient;
    PubSubClient * client;
    void reconnectMqtt();
    
    void (*callback)(char*, uint8_t*, unsigned int);
};

#endif

