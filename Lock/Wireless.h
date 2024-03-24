#ifndef WIRELESS_H
#define WIRELESS_H

#include <Arduino.h>
#include <ArduinoJson.h>

#include "DeviceStatus.h"

class Wireless {
public:
  void init(DeviceStatus &status);

  void handle(DeviceStatus &status);   
};

static void attachHandlers(DeviceStatus &status);

#endif