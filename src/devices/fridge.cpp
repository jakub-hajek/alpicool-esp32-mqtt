#include "fridge.h"
#include "Arduino.h"
#include "NimBLEDevice.h"

#include "model/DataModel.h"


static const char * MODULE = "FRIDGE";


struct StatusReport last_status_report;


static const uint8_t pingMessage[] = {0xfe, 0xfe, 0x3, 0x1, 0x2, 0x0};

static const uint8_t * last_message = NULL;
static const uint16_t DELAY_BETWEEN_MESSAGES = 2000;

static void notifyCallback(BLERemoteCharacteristic *remoteCharacteristic, uint8_t *data, size_t length, bool isNotify)
{
    if (length >= sizeof(struct StatusReport)) {    
        struct StatusReport * r = (struct StatusReport *) data;
        last_status_report = *r;
        ESP_LOGI(MODULE,"alpicool real temperature %i", r->Sensors.Temp);
        ESP_LOGI(MODULE,"alpicool voltage %i", r->Sensors.InputV1); //cely volty
        ESP_LOGI(MODULE,"alpicool voltage2 %i", r->Sensors.InputV2); //desetiny voltu
        ESP_LOGI(MODULE,"alpicool set temperature %i", r->Settings.TempSet);
        ESP_LOGI(MODULE,"data leght %i", length);
        DataModel::getInstance().fridge_data.actual_temperature = 1.0f * r->Sensors.Temp;
        DataModel::getInstance().fridge_data.desired_temperature = 1.0f * r->Settings.TempSet;
        DataModel::getInstance().fridge_data.voltage = 1.0f * r->Sensors.InputV1 + 0.1f * r->Sensors.InputV2;
        DataModel::getInstance().fridge_data.eco = r->Settings.EcoMode;
        DataModel::getInstance().fridge_data.on = r->Settings.On;
    }
   
}

class FridgeClientCallbacks : public BLEClientCallbacks
{
private:
    Fridge * fridge;
    FridgeClientCallbacks() {}

public:
    FridgeClientCallbacks(Fridge *p) : BLEClientCallbacks()
    {
        fridge = p;
    }

    void onConnect(BLEClient *client)
    {
        ESP_LOGI(MODULE,"Connected");
        client->updateConnParams(120,120,0,600);
    }

    void onDisconnect(BLEClient *client)
    {
        ESP_LOGI(MODULE,"Disconnected");
        fridge->ondisconnect();
        
    }
};



Fridge::Fridge(BLEUUID pserviceUUID,  BLEUUID pwriteCharUUID, BLEUUID preadCharUUID)
{
    esp_log_level_set(MODULE, ESP_LOG_VERBOSE);
    ESP_LOGI(MODULE,"Creating Fridge..");
    doConnect = false;
    connected = false;
    active = true;
    server = nullptr;
    serviceUUID = pserviceUUID;
    writeCharUUID = pwriteCharUUID;
    readCharUUID = preadCharUUID;
    client = BLEDevice::createClient();
    client->setClientCallbacks(new FridgeClientCallbacks(this));
    client->setConnectionParams( 32,160,0,900 );
    lastLoop = 0;
    lastMessageMillis = 0;

}

void Fridge::setConnected(bool c)
{
    connected = c;
}


void Fridge::setDoConnect(bool ds)
{
    doConnect = ds;
}

bool Fridge::isConnected()
{
    return connected;
}

bool Fridge::getActive()
{
    return active;
}

void Fridge::setServer(BLEAdvertisedDevice *s)
{
    server = s;
}

bool Fridge::getDoConenct() {
    return doConnect;
}

void Fridge::disconnect()
{
    active = false;
    if (connected) client->disconnect();
}

void Fridge::connect()
{
    active = true;
}

void Fridge::ondisconnect()
{
    connected = false;
    DataModel::getInstance().fridge_data.voltage = 0;
}

void Fridge::setEcoMode(bool i) {
    ESP_LOGI(MODULE,"setting EcoMode to: %s", i ? "true" : "false");
    if (!connected) return;
    if (last_status_report.Preamble == 0) return;
    struct Settings s = last_status_report.Settings;
    s.EcoMode = i;
    struct SetStateCommand c;
    std::vector<uint8_t> com = c.NewSetStateCommand(s);
    if (!remoteWriteCharacteristic->writeValue(com.data(), com.size(), true)) {
        ESP_LOGI(MODULE,"Cannot set On/Off");
    } else {
        ESP_LOGI(MODULE,"On/Off set");
    }
}

void Fridge::setOnOff(bool i) {
    ESP_LOGI(MODULE,"setting ON /OFF to: %s", i ? "true" : "false");
    if (!connected) return;
    if (last_status_report.Preamble == 0) return;
    struct Settings s = last_status_report.Settings;
    s.On = i;
    struct SetStateCommand c;
    std::vector<uint8_t> com = c.NewSetStateCommand(s);
    if (!remoteWriteCharacteristic->writeValue(com.data(), com.size(), true)) {
        ESP_LOGI(MODULE,"Cannot set On/Off");
    } else {
        ESP_LOGI(MODULE,"On/Off set");
    }
};

void Fridge::setTemperature(int8_t i) {
    ESP_LOGI(MODULE,"setting temperature to: %i", i);
    if (!connected) return;
    int8_t temp = i;
    if (i < -20) temp = -20;
    if (i > 20) temp = 20;
    struct SetTempCommand c;
    std::vector<uint8_t> com = c.NewSetTempCommand(temp);
    if (!remoteWriteCharacteristic->writeValue(com.data(), com.size(), true)) {
        ESP_LOGI(MODULE,"Cannot set temperature");
    } else {
        ESP_LOGI(MODULE,"Temperature set");
    }
}

void Fridge::connectToServer()
{

    ESP_LOGI(MODULE,"Connecting to %s", server->getAddress().toString().c_str());

    client->connect(server);

    NimBLERemoteService *remoteService = client->getService(serviceUUID);

    if (remoteService == nullptr || remoteService == NULL)
    {
        ESP_LOGI(MODULE,"Failed to connect to the service");
        client->disconnect();
        return;
    }

    remoteReadCharacteristic = remoteService->getCharacteristic(readCharUUID);

    if (remoteReadCharacteristic == nullptr || remoteReadCharacteristic == NULL)
    {
        ESP_LOGI(MODULE,"Failed to find read characteristic");
        client->disconnect();
        return;
    }
    

    remoteWriteCharacteristic = remoteService->getCharacteristic(writeCharUUID);
    if (remoteWriteCharacteristic == nullptr || remoteWriteCharacteristic == NULL)
    {
        ESP_LOGI(MODULE,"Failed to find write characteristic");
        client->disconnect();
        return;
    }
    ESP_LOGD(MODULE, "Write Characteristics UUID: ");
    ESP_LOGD(MODULE,"s", remoteWriteCharacteristic->getUUID().toString().c_str());

    if (remoteReadCharacteristic->canNotify());
    {
        remoteReadCharacteristic->registerForNotify(notifyCallback,true, false);
        ESP_LOGI(MODULE,"Callback registered");
    }

    connected = true;
}



void Fridge::loop()
{
    lastLoop = millis();
    if (doConnect)
    {
        connectToServer();
        doConnect = false;
    }

    if (connected && millis() - lastMessageMillis > DELAY_BETWEEN_MESSAGES)
    {
        if(remoteWriteCharacteristic->writeValue(pingMessage, 6, true)) {
            lastMessageMillis = millis();         
        } else {
            disconnect();
            connect();
        }

    }
}
