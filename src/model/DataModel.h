#ifndef _HEADER_ALPICOOL_DATAMODEL
#define _HEADER_ALPICOOL_DATAMODEL

struct fridge_data {
    float actual_temperature = 0;
    float desired_temperature = 0;
    float voltage = 0;
    bool on;
    bool eco;
};

class DataModel {
    public:
        struct fridge_data fridge_data;
        static DataModel& getInstance();
    private:
        DataModel(); 
    public:
        DataModel(DataModel const&) = delete;
        void operator=(DataModel const&) = delete;
};



#endif


