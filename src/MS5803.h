#ifndef MS5803_H
#define MS5803_H

#include "Arduino.h"

class MS5803
{
public:
    
    MS5803(uint8_t _cs);

    enum Unit:byte{
        hPa,
        Pa,
        mBars,
        m,
        cm,
        mm
    };
    
    void init();
    
    /* Does the actual read from the sensor. */
    void read();
    
    // returns temp in degrees C.
    float get_temperature();

    // Returns pressure in unit(mBars, hPa, Pa). 
    float get_pressure(byte unit=Unit::hPa);

    // string of unit
    char* unit(byte unit=Unit::hPa);

    // base atm
    float get_base_atm();

    // depth in water  Unit(m)
    float get_depth(byte unit=Unit::m);
    
    // Resets the sensor
    void reset();

    
    
private:
    
    uint8_t cs;
    boolean interface;
    
    float temp = 0;   // temp in degrees C.
    float press = 0;  // pressure in mbars
    float base_atm = 1013.25;  // base atm in hPa

    // Reads the coeffincient data from the sensor.
    uint32_t readCoefficient(uint8_t index);

    uint32_t readADC(char cmd);
    
};

#endif /* defined(____MS5803__) */