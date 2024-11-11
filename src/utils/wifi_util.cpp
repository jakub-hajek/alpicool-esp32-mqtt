#include "WiFi.h"
#include "mqtt.h"

int connect_wifi(const char *ssid, const char *pass)
{

  if (WiFi.status() == WL_CONNECTED)
    return 0;

  WiFi.begin(ssid, pass);

  if (WiFi.waitForConnectResult(5000) != WL_CONNECTED)
  {
    ESP_LOGE("WIFI", "WiFi Failed!");
    return -1;
  }
  delay(100);

  ESP_LOGI("WIFI", "WiFi connection Successful");
  ESP_LOGI("WIFI", "%s", WiFi.localIP().toString().c_str()); // Print the IP address
  ESP_LOGI("WIFI", "%s", WiFi.macAddress().c_str());
  return 0;
}