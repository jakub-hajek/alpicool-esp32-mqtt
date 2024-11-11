#ifndef _HEADER_ALPICOOL_BLE_SCANNER
#define _HEADER_ALPICOOL_BLE_SCANNER
#include "NimBLEDevice.h"
#include "fridge.h"

class Blescanner {
public:
    Blescanner(Fridge * pfridge);
    void loop();

private:
    Fridge * fridge; 
    uint32_t last_scan;
};


#endif