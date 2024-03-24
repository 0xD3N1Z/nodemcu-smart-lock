#ifndef DEVICESTATUS_H
#define DEVICESTATUS_H

#include <Arduino.h>

enum class TriggerSource {
  WIFI, FLASH
};

class DeviceStatus {
private:
  bool _reset;
  bool _trigger;
  TriggerSource _triggerSource;

public:
  bool connected;
  bool lock;
  unsigned long debugRunTime;

  void save();

  void reload();

  void trigger(TriggerSource source);

  void reset();

  void handle();
};

#endif