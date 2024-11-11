#include "ble_scanner.h"
#include "config/config.h"

// fridge ff:22:12:07:01:b4

static const char *MODULE = "BLE_SCANNER";

class AdvertisedDeviceCallbacks : public NimBLEAdvertisedDeviceCallbacks
{
private:
  Fridge *fridge;
  AdvertisedDeviceCallbacks() {}

public:
  AdvertisedDeviceCallbacks(Fridge *pfridge) : NimBLEAdvertisedDeviceCallbacks()
  {
    fridge = pfridge;
  }
  ~AdvertisedDeviceCallbacks() {}

  void onResult(NimBLEAdvertisedDevice *advertisedDevice)
  {
    bool fridgefound = !fridge->getActive() || fridge->isConnected();

    if (!fridgefound && advertisedDevice->getAddress().toString() == FRIDGE_BLE_ADDRESS )
    {
      ESP_LOGI(MODULE, "Found Fridge service");
      fridge->setServer(advertisedDevice);
      fridge->setDoConnect(true);
      fridgefound = true;
    }

    if (fridgefound)
    {
      BLEDevice::getScan()->stop();
    }
  }
};

Blescanner::Blescanner(Fridge *pfridge)
{
  fridge = pfridge;
  last_scan = 0;

  ESP_LOGI(MODULE, "BLE setup started oon core: ");
  ESP_LOGI(MODULE, "Starting BLE client...");
  BLEDevice::init("apicool-ble");

  BLEScan *scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  scan->setInterval(1000); // in Msec
  scan->setWindow(500);
  scan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks(fridge));
  loop();
}

void Blescanner::loop()
{
  if (millis() - last_scan > 6000)
  {
    ESP_LOGI(MODULE, "Searching fridge");
    last_scan = millis();
    bool scanFridge = fridge->getActive() && !fridge->isConnected();
    if (scanFridge)
    {
      BLEDevice::getScan()->start(2);
    }
  }
}
