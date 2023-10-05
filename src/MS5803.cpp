#include "MS5803.h"
#include <SPI.h>

// Sensor constants:
#define SENSOR_CMD_RESET      0x1E
#define SENSOR_CMD_ADC_READ   0x00
#define SENSOR_CMD_ADC_CONV   0x40
#define SENSOR_CMD_ADC_D1     0x00
#define SENSOR_CMD_ADC_D2     0x10
#define SENSOR_CMD_ADC_256    0x00
#define SENSOR_CMD_ADC_512    0x02
#define SENSOR_CMD_ADC_1024   0x04
#define SENSOR_CMD_ADC_2048   0x06
#define SENSOR_CMD_ADC_4096   0x08

static uint32_t    sensorCoefficients[8];         // calibration coefficients
static uint32_t    D1                       = 0;  // pressure value
static uint32_t    D2                       = 0;  // temperature value
static float       deltaTemp                = 0;
static float       sensorOffset             = 0;
static float       sensitivity              = 0;
static float       TEMP                     = 0;
static float       P                        = 0;


// Constructor when using SPI.
MS5803::MS5803(uint8_t _cs) {
    cs   = _cs;
}

void MS5803::init(){
    pinMode(cs, OUTPUT );
  	digitalWrite(cs, HIGH );

    SPI.begin();
    SPI.setDataMode(SPI_MODE3);
   	SPI.setBitOrder(MSBFIRST);
   	SPI.setClockDivider(SPI_CLOCK_DIV32);

    // resetting the sensor
    reset(); 
	
	// Read sensor coefficients --used to convert sensor data (pressure and temp)
    for (int i = 0; i < 8; i++ ){
        sensorCoefficients[i] = readCoefficient(i);  // read coefficients
        Serial.printf("Coefficient%d = ", i);
        Serial.println(sensorCoefficients[i]);
    }

    delay(100);
    float sum_base_atm = 0;
    for (int i = 0; i < 100; i++){
        read();
        sum_base_atm += get_pressure(hPa);
    }
    base_atm = sum_base_atm / 100;
    Serial.printf("base atm = %f hPa\n", base_atm);
}


void MS5803::reset() {
    	SPI.setDataMode(SPI_MODE3);
   	 	digitalWrite(cs, LOW);
   		SPI.transfer(SENSOR_CMD_RESET);
    	delay(5);
    	digitalWrite(cs, HIGH);
}


uint32_t MS5803::readCoefficient(uint8_t index) {
    uint32_t result = 0;
    
	SPI.setDataMode(SPI_MODE3);
	digitalWrite(cs, LOW);

	SPI.transfer(0xA0 + (index * 2 ));

	result = SPI.transfer(0x00);
	result = result << 8;
	result |= SPI.transfer(0x00);

	digitalWrite(cs, HIGH);

    return(result);
}


void MS5803::read() {

	D1 = readADC(SENSOR_CMD_ADC_D1 + SENSOR_CMD_ADC_4096);    // read pressure
    D2 = readADC(SENSOR_CMD_ADC_D2 + SENSOR_CMD_ADC_4096);    // read temperature
    
    // calculate temperature
    deltaTemp = D2 - sensorCoefficients[5] * pow(2, 8);
    TEMP = 2000 + (deltaTemp * sensorCoefficients[6]) / pow(2, 23); 

    // calculate pressure  (20 > temp)
    sensorOffset = sensorCoefficients[2] * pow(2, 17) + (deltaTemp * sensorCoefficients[4]) / pow(2, 6);
    sensitivity = sensorCoefficients[1] * pow(2, 16) + (deltaTemp * sensorCoefficients[3]) / pow(2, 7);
    P = ((D1 * sensitivity) / pow(2, 21) - sensorOffset) / pow(2, 15);
    
    temp = TEMP /100;
    press = P / 100;
}


uint32_t MS5803::readADC(char cmd) {

    uint32_t result = 0;
    uint32_t returnedData = 0;
    
    SPI.setDataMode(SPI_MODE3);
    digitalWrite(cs, LOW);

    SPI.transfer(cmd + SENSOR_CMD_ADC_CONV);
    
    switch (cmd & 0x0f)
    {
        case SENSOR_CMD_ADC_256 :
            delay(1);
            break;
        case SENSOR_CMD_ADC_512 :
            delay(3);
            break;
        case SENSOR_CMD_ADC_1024:
            delay(4);
            break;
        case SENSOR_CMD_ADC_2048:
            delay(6);
            break;
        case SENSOR_CMD_ADC_4096:
            delay(10);
            break;
    }
    
    digitalWrite(cs, HIGH);
    delayMicroseconds(200);

    digitalWrite(cs, LOW);

	SPI.transfer(SENSOR_CMD_ADC_READ);
	
	returnedData = SPI.transfer( 0x00 );
	result = 65536 * returnedData;
	returnedData = SPI.transfer( 0x00 );
	result = result + 256 * returnedData;
	returnedData = SPI.transfer( 0x00 );
	result = result + returnedData;

	digitalWrite(cs, HIGH);
    
    return result;
}


float MS5803::get_temperature(){
    return temp;
} 


float MS5803::get_pressure(byte unit=Unit::hPa){
    switch (unit)
    {
    case Unit::hPa:
        return press;
        break;
    case Unit::Pa:
        return press * 100;
        break;
    case Unit::mBars:
        return press;
        break;
    default:
        break;
    }
    return -1;
}


char* MS5803::unit(byte unit=Unit::hPa){
    switch (unit)
    {
    case Unit::hPa:
        return " hPa";
        break;
    case Unit::Pa:
        return " Pa";
        break;
    case Unit::mBars:
        return " mBar";
        break;
    case Unit::m:
        return " m";
        break;
    case Unit::cm:
        return " cm";
        break;
    case Unit::mm:
        return " mm";
        break;
    default:
        break;
    }
    return "";
}


float MS5803::get_base_atm(){
    return base_atm;
}


float MS5803::get_depth(byte unit=Unit::m){
    float depth = abs((press - base_atm) * 100 * 0.0001);
    switch (unit)
    {
    case Unit::m:
        return depth;
        break;
    case Unit::cm:
        return depth * 100;
        break;
    case Unit::mm:
        return depth * 1000;
        break;
    default:
        break;
    }
    return -1;
}