#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include <Servo.h>

#include "DeviceStatus.h"
#include "Config.h"

Servo servo;
unsigned long prevMillis = 0;

void DeviceStatus::handle() {
  unsigned long currentMillis = millis();
  
  if (currentMillis - prevMillis >= 1000) {
    prevMillis = currentMillis;
    debugRunTime++;
  }

  if (_reset) {
    debugln("Yeniden başlatılıyor...");

    ESP.restart();

    return;
  }

  if (_trigger) {
    debugln("Kilit durumu değiştirme isteği alındı");

    _trigger = false;

    lock = !lock;

    servo.write(lock ? SERVO_ANG_LOCKED : SERVO_ANG_UNLOCKED);

    debug("Yeni kilit durumu: ");
    debugln(lock ? "KİLİTLİ" : "KİLİT AÇIK");

    if (_triggerSource == TriggerSource::FLASH) return;

    save();
  }
}

void DeviceStatus::save() {
  File statusFile = LittleFS.open("status.json", "w");
  if (!statusFile) return;

  JsonDocument doc;

  doc["lock"] = lock;

  serializeJson(doc, statusFile);

  statusFile.close();

  debugln("Mevcut çalışma durumu dosya sistemine kaydedildi.");
}

void DeviceStatus::reload() {
  LittleFS.begin();

  servo.attach(SERVO_PIN, 500, 2400);

  File statusFile = LittleFS.open("status.json", "r");
  if (!statusFile) return;

  JsonDocument doc;

  DeserializationError error = deserializeJson(doc, statusFile);
  statusFile.close();

  if (!error) {
    lock = doc["lock"] | false;
  }

  servo.write(lock ? SERVO_ANG_LOCKED : SERVO_ANG_UNLOCKED);

  debugln("Önceki çalışma durumu dosya sisteminden okundu.");
}

void DeviceStatus::trigger(TriggerSource source) {
  _trigger = true;
  _triggerSource = source;
}

void DeviceStatus::reset() {
  _reset = true;
}
