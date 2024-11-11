#ifndef _HEADER_ALPICOOL_FRIDGE
#define _HEADER_ALPICOOL_FRIDGE
#include "NimBLEDevice.h"
#include <cstring>

const uint16_t Preamble = 0xfefe;
const uint16_t ConstPreamble = 0xfefe;

struct __attribute__((packed)) Sensors {
    int8_t Temp;
    int8_t UB17;
    int8_t InputV1;
    int8_t InputV2;
};

struct __attribute__((packed)) Settings {
    bool Locked;
    bool On;
    bool EcoMode;
    int8_t HLvl;
    int8_t TempSet;
    int8_t HighestTempSettingMenuE2;
    int8_t LowestTempSettingMenuE1;
    int8_t HysteresisMenuE3;
    int8_t SoftStartDelayMinMenuE4;
    bool CelsiusFahrenheitModeMenuE5;
    int8_t TempCompGTEMinus6DegCelsiusMenuE6;
    int8_t TempCompGTEMinus12DegCelsiusLTMinus6DegCelsiusMenuE7;
    int8_t TempCompLTMinus12DegCelsiusMenuE8;
    int8_t TempCompShutdownMenuE9;
};



const uint8_t dataLenStatusReport = 0x15;
const uint8_t cmdCodeStatusReport = 0x1;

struct __attribute__((packed)) StatusReport {
    uint16_t Preamble = 0;
    uint8_t DataLen;
    uint8_t CommandCode;
    struct Settings Settings;
    struct Sensors Sensors;
    uint16_t Checksum;

    uint16_t CRC() {
        uint16_t checksum = (Preamble >> 8) + (Preamble & 0xff) + static_cast<uint8_t>(DataLen) 
            + static_cast<uint8_t>(CommandCode) + static_cast<uint8_t>(Settings.HLvl)
            + static_cast<uint8_t>(Settings.TempSet) + static_cast<uint8_t>(Settings.LowestTempSettingMenuE1)
            + static_cast<uint8_t>(Settings.HighestTempSettingMenuE2) + static_cast<uint8_t>(Settings.HysteresisMenuE3)
            + static_cast<uint8_t>(Settings.SoftStartDelayMinMenuE4) + static_cast<uint8_t>(Settings.TempCompGTEMinus6DegCelsiusMenuE6)
            + static_cast<uint8_t>(Settings.TempCompGTEMinus12DegCelsiusLTMinus6DegCelsiusMenuE7)
            + static_cast<uint8_t>(Settings.TempCompLTMinus12DegCelsiusMenuE8) + static_cast<uint8_t>(Settings.TempCompShutdownMenuE9)
            + static_cast<uint8_t>(Sensors.Temp) + static_cast<uint8_t>(Sensors.UB17) + static_cast<uint8_t>(Sensors.InputV1)
            + static_cast<uint8_t>(Sensors.InputV2);
        if (Settings.Locked) checksum++;
        if (Settings.On) checksum++;
        if (Settings.EcoMode) checksum++;
        if (Settings.CelsiusFahrenheitModeMenuE5) checksum++;
        return checksum;
    }

    bool Valid() {
        if (Preamble != ::Preamble) {
            return false;
        }
        if (CommandCode != cmdCodeStatusReport) {
            return false;
        }
        if (DataLen != dataLenStatusReport) {
            return false;
        }
        if (Checksum != CRC()) {
            return false;
        }
        return true;
    }
};

const int8_t dataLenSetTemp = 0x4;
const int8_t cmdCodeSetTemp = 0x5;

struct __attribute__((packed)) SetTempCommand {
    uint16_t Preamble;
    int8_t DataLen;
    int8_t CommandCode;
    int8_t Temp;
    uint16_t Checksum;

    static std::vector<uint8_t> NewSetTempCommand(int8_t temp) {
        SetTempCommand c = {
            .Preamble = ::Preamble,
            .DataLen = dataLenSetTemp,
            .CommandCode = cmdCodeSetTemp,
            .Temp = temp
        };

        c.Checksum = (c.Preamble >> 8) + (c.Preamble & 0xff) + static_cast<uint8_t>(c.DataLen) + static_cast<uint8_t>(c.CommandCode) + static_cast<uint8_t>(c.Temp);
        std::vector<uint8_t> bytes(sizeof(c));
        std::memcpy(bytes.data(), &c, sizeof(c));
        int size = bytes.size();
        //fix endianity for 2 byte checksum
        uint8_t lastbyte = bytes.data()[size-1];
        uint8_t beforelastbyte = bytes.data()[size-2];
        bytes.data()[size-2] = lastbyte;
        bytes.data()[size-1] = beforelastbyte;
        return bytes;
    }
};

const uint8_t dataLenSetState = 0x11;
const uint8_t cmdCodeSetState = 0x2;

struct __attribute__((packed)) SetStateCommand {
    uint16_t Preamble;
    uint8_t DataLen;
    uint8_t CommandCode;
    struct Settings Settings;
    uint16_t Checksum;

    uint16_t CRC() {
        uint16_t checksum = (Preamble >> 8) + (Preamble & 0xff) + static_cast<uint8_t>(DataLen) + static_cast<uint8_t>(CommandCode) + static_cast<uint8_t>(Settings.HLvl) + static_cast<uint8_t>(Settings.TempSet) + static_cast<uint8_t>(Settings.LowestTempSettingMenuE1) + static_cast<uint8_t>(Settings.HighestTempSettingMenuE2) + static_cast<uint8_t>(Settings.HysteresisMenuE3) + static_cast<uint8_t>(Settings.SoftStartDelayMinMenuE4) + static_cast<uint8_t>(Settings.TempCompGTEMinus6DegCelsiusMenuE6) + static_cast<uint8_t>(Settings.TempCompGTEMinus12DegCelsiusLTMinus6DegCelsiusMenuE7) + static_cast<uint8_t>(Settings.TempCompLTMinus12DegCelsiusMenuE8) + static_cast<uint8_t>(Settings.TempCompShutdownMenuE9);
        if (Settings.Locked) checksum++;
        if (Settings.On) checksum++;
        if (Settings.EcoMode) checksum++;
        if (Settings.CelsiusFahrenheitModeMenuE5) checksum++;
        return checksum;
    }

    void updateCRC() {
        Checksum = CRC();
    }

    bool Valid() {
        if (Preamble != ::Preamble) {
            return false;
        }
        if (CommandCode != cmdCodeSetState) {
            return false;
        }
        if (DataLen != dataLenSetState) {
            return false;
        }
        if (Checksum != CRC()) {
            return false;
        }
        if (Settings.TempSet > Settings.HighestTempSettingMenuE2) {
            return false;
        }
        if (Settings.TempSet < Settings.LowestTempSettingMenuE1) {
            return false;
        }
        return true;
    }

    static std::vector<uint8_t> NewSetStateCommand(const struct Settings& s) {
        SetStateCommand c = {
            .Preamble = ::Preamble,
            .DataLen = dataLenSetState,
            .CommandCode = cmdCodeSetState,
            .Settings = s
        };
        c.updateCRC();
        if (!c.Valid()) {
            throw std::runtime_error("Invalid state command");
        }
        std::vector<uint8_t> bytes(sizeof(c));
        std::memcpy(bytes.data(), &c, sizeof(c));
        //fix endianity for 2 byte checksum
        int size = bytes.size();
        uint8_t lastbyte = bytes.data()[size-1];
        uint8_t beforelastbyte = bytes.data()[size-2];
        bytes.data()[size-2] = lastbyte;
        bytes.data()[size-1] = beforelastbyte;
        return bytes;
    }
};


class Fridge {
public:
    Fridge(BLEUUID pserviceUUID,  BLEUUID pwriteCharUUID, BLEUUID preadCharUUID);
    void connectToServer();

public:
    BLERemoteCharacteristic *remoteWriteCharacteristic;
    void setConnected(bool c);
    void setDoConnect(bool dc);
    bool getDoConenct();


    void setServer(BLEAdvertisedDevice * s);
    void ondisconnect();
    void loop();
    void disconnect();
    void connect();
    bool isConnected();
    bool getActive();
    void setEcoMode(bool i);
    void setOnOff(bool i);
    void setTemperature(int8_t i);
private:
    BLEUUID serviceUUID;
    BLEUUID writeCharUUID;
    BLEUUID readCharUUID;
    BLERemoteCharacteristic *remoteReadCharacteristic;
    BLEAdvertisedDevice *server;
    bool kunda = false;
    bool doConnect;
    bool connected;
    bool active;
    uint32_t lastLoop;
    uint32_t lastMessageMillis;
    BLEClient * client;


};


#endif