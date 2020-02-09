#ifndef SHT30_h
#define SHT30_h

#include "Adafruit_SHT31.h"
#include "Sensor.h"

class Sht30 : public Sensor
{
public:
  Sht30(){};
  bool measure();

protected:
  bool initialise();
  Adafruit_SHT31 sht31 = Adafruit_SHT31();
};

#endif
