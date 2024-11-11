#include "mqtt.h"
#include "model/DataModel.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "config/config.h"



MqttHandler::MqttHandler(void (*c)(char*, uint8_t*, unsigned int)) {
    espClient = new WiFiClient();
    client = new PubSubClient(*espClient);
    client->setBufferSize(1111);
    client->setServer(MQTT_SERVER, 1883);
    callback = c;
    client->setCallback(callback);
    last_connection_attempt = 0;
    last_publish = 0;
    last_loop = 0;
}

void MqttHandler::reconnectMqtt() {
  if (WiFi.status() != WL_CONNECTED ) { 
    return;
  }
  if (client->connected()) {
    return;
  }
  
 
        // Loop until we're reconnected
  if(!client->connected() && (millis() - last_connection_attempt > 8000) ) {
        last_connection_attempt = millis();
        ESP_LOGI("MQTT", "Attempting MQTT connection...");
        // Attempt to connect
        if (client->connect(DEVICE_NAME, MQTT_USER, MQTT_PASSWORD)) {
          ESP_LOGI("MQTT", "connected");
          client->subscribe(COMMAND_TOPIC);
          ESP_LOGI("MQTT", "callback set");
        } else {
          ESP_LOGE("MQTT", "failed, rc=%i", client->state());
          ESP_LOGE("MQTT", " try again in 5 seconds");
        }
    }
}
char * get_json_data(char * ret_buf) {
  DataModel &dm = DataModel::getInstance();
    DynamicJsonDocument doc(4000);
    if (DataModel::getInstance().fridge_data.voltage > 0 ) {
      doc["f_voltage"] = DataModel::getInstance().fridge_data.voltage;
      doc["f_eco"] = DataModel::getInstance().fridge_data.eco;
      doc["f_on"] = DataModel::getInstance().fridge_data.on;
      doc["f_actual_temperature"] = DataModel::getInstance().fridge_data.actual_temperature;
      doc["f_desired_temperature"] = DataModel::getInstance().fridge_data.desired_temperature;
    }
    char output[1000];
    serializeJson(doc, output);
    strcpy(ret_buf, output);
    return ret_buf;
}

void MqttHandler::publish(const char * topic, const char * message) {
    if (client->connected()) client->publish(topic, message);
}

void MqttHandler::publishData () {
    char buf[1000];
    client->publish(SENSOR_TOPIC, get_json_data(buf));
}

void MqttHandler::loop() {
  reconnectMqtt();
  if (WiFi.status() != WL_CONNECTED ) { 
    return;
  }
  client->loop();
  last_loop = millis();
  if (millis() - last_publish > DATA_PUBLISH_FREQ_MILLIS) {
    last_publish = millis();
    publishData();
  }

}