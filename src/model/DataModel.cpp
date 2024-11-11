#include "DataModel.h"

DataModel::DataModel() {}

DataModel& DataModel::getInstance() {
    static DataModel instance;
    return instance;
}


