#include "Arduino.h"
#include "NimBLEDevice.h"
#include <ArduinoJson.h>
#include "model/DataModel.h"
#include "devices/fridge.h"
#include "devices/ble_scanner.h"
#include "config/config.h"
#include "utils/wifi_util.h"
#include "utils/mqtt.h"
#include "WiFi.h"
#include "esp_coexist.h"



//#define CONFIG_SW_COEXIST_ENABLE 1
//#define CONFIG_ESP32_WIFI_SW_COEXIST_ENABLE 1

Blescanner * blescanner;
MqttHandler * mqtt = NULL;
Fridge * fridge;
uint32_t lastWifi = 0;



void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  char buf[length+1];
  mempcpy(buf,payload,length);
  buf[length] = '\0';
  ESP_LOGI("MQTT_CALLBACK", "message received %s", buf );
  StaticJsonDocument<1000> doc;
  DeserializationError error = deserializeJson(doc, buf);

  // Test if parsing succeeds.
  if (error) {
    ESP_LOGI("MQTT_CALLBACK","deserializeJson() failed: ");
    return;
  }
  if (doc.containsKey("fridge_on")) {
    fridge->setOnOff(doc["fridge_on"]);
  }
  if (doc.containsKey("fridge_eco")) {
    fridge->setEcoMode(doc["fridge_eco"]);
  }
  if (doc.containsKey("fridge_temperature")) {
    fridge->setTemperature(doc["fridge_temperature"]);
  }
  if (doc.containsKey("restart")) {
    bool res = doc["restart"];
    if (res) {
      ESP.restart();
    }
  }
}

void connect_to_wifi_int() {
    connect_wifi(WIFI_SSID, WIFI_PASS);
}

int my_logging(const char * s, va_list args) {
  //static std::mutex m;
  char buf[500];
  char * x = buf;
  int ret = vsnprintf(buf, 500, s, args);

  Serial.print(buf);
  if (WiFi.status() == WL_CONNECTED) {
    if (mqtt != NULL) mqtt->publish(DEBUG_TOPIC, buf);
  }
  return ret;
}

void setup_internal(void *p)
{

  esp_log_set_vprintf(&my_logging);
  //esp_log_set_vprintf(&vprintf);
  esp_log_level_set("*", ESP_LOG_INFO);

  connect_to_wifi_int();
  fridge = new Fridge(BLEUUID("00001234-0000-1000-8000-00805f9b34fb"),
    BLEUUID("00001235-0000-1000-8000-00805f9b34fb"),
    BLEUUID("00001236-0000-1000-8000-00805f9b34fb"));

  blescanner = new Blescanner(fridge);
  mqtt = new MqttHandler(mqtt_callback);


  ESP_LOGI("","Setup done");
}


void loop_internal()
{
  delay(50);
  esp_coex_preference_set(ESP_COEX_PREFER_BT);
  esp_log_level_set("*", ESP_LOG_INFO);

  blescanner->loop();
  mqtt->loop();
  fridge->loop();



  if (millis() - lastWifi > 10000)
  {
    const char *x = ("[APP] Free memory: " + String(esp_get_free_heap_size()) + " bytes").c_str();
    ESP_LOGI("", "%s", x);
    connect_to_wifi_int();
    lastWifi = millis();
  }


}



void setup()
{
  Serial.begin(9600);
  setup_internal(NULL);

}

void loop()
{
  loop_internal();

}