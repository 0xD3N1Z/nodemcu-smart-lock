#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

#include "DeviceStatus.h"
#include "Wireless.h"
#include "Config.h"

WiFiEventHandler onConnected, onDisconnected;
ESP8266WebServer server(80);

void Wireless::init(DeviceStatus &status) {
  WiFi.mode(WIFI_STA);
  //WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  #ifdef DEBUG
    debugln("");
    debug("Wi-Fi ağı: ");
    debugln(WIFI_SSID);

    WiFi.waitForConnectResult();

    if (!WiFi.isConnected()) {
      debugln("Bağlantı kurulamadı");
    } else {
      debugln("Bağlandı");
      debugln(WiFi.localIP());
      status.connected = true;
    }
  #endif

  attachHandlers(status);

  server.begin();
}

void Wireless::handle(DeviceStatus &status) {
  if (server.client().status() == CLOSED || !status.connected) delay(1); //işlemci kullanımını azalt

  if (!status.connected) return; //bağlantı yoksa

  server.handleClient();
}

static void attachHandlers(DeviceStatus &status) {
  onConnected = WiFi.onStationModeConnected([&status](const WiFiEventStationModeConnected &e) {
    debugln("Wi-Fi Bağlandı");
    status.connected = true;
  });

  onDisconnected = WiFi.onStationModeDisconnected([&status](const WiFiEventStationModeDisconnected &e) {
    debugln("Wi-Fi Bağlantısı koptu");
    status.connected = false;
  });

  //kök, kurulum, durum, kilitleyip açma, dosya listesi, hata ayıklama, yeniden baslatma 

  //Kök
  server.on("/", []() {
    debugln("Webserver isteği alındı: /");

    server.keepAlive(false);
    server.send(200, "text/plain", "OK");
  });

  //Kurulum
  server.on("/setup", HTTP_POST, []() {
    debugln("Webserver isteği alındı: /setup");

    server.keepAlive(false);
    server.send(200, "text/plain", "OK");
  });

  //Durum
  server.on("/status", HTTP_GET, [&status]() {
    debugln("Webserver isteği alındı: /status");

    JsonDocument doc;

    doc["lock"] = status.lock;
    doc["signalStrength"] = WiFi.RSSI();

    char output[64];
    serializeJson(doc, output);

    server.keepAlive(false);
    server.send(200, "application/json", output);
  });

  //Kilitleme
  server.on("/lock", HTTP_GET, [&status]() {
    debugln("Webserver isteği alındı: /lock");

    status.trigger(TriggerSource::WIFI);
    
    server.keepAlive(false);
    server.sendHeader("x-status-lock", !status.lock ? "1" : "0");
    server.send(200, "text/plain", "OK");
  });

  //Dosya listesi
  server.on("/fs", HTTP_GET, []() {
    debugln("Webserver isteği alındı: /fs");

    //Formatla
    if (server.hasArg("format")) {
      LittleFS.format();
      server.keepAlive(false);
      server.send(200, "text/plain", "OK");
      return;
    }

    //Dosyaları listele
    if (!server.hasArg("file")) {
      Dir dir = LittleFS.openDir("/");
      String output = "[";

      while(dir.next()) {
        File entry = dir.openFile("r");
        if (output != "[") output += ", ";

        output += String(entry.name());
        entry.close();
      }
      output += "]";
      server.keepAlive(false);
      server.send(200, "text/plain", output);
      return;
    }

    //Dosya var mı
    if (!LittleFS.exists(server.arg("file"))) {
      server.keepAlive(false);
      server.send(404, "text/plain", "NOT FOUND");
      return;
    }

    File file = LittleFS.open(server.arg("file"), "r");
    if (!file) {
      server.keepAlive(false);
      server.send(404, "text/plain", "NOT FOUND");
      return;
    }

    String content = file.readString();
    file.close();

    server.keepAlive(false);
    server.send(200, "text/json", content);
  });

  //Hata ayıklama
  server.on("/debug", [&status]() {
    debugln("Webserver isteği alındı: /debug");

    JsonDocument doc;

    doc["wireless"]["ssid"] = WiFi.SSID();
    doc["wireless"]["rssi"] = WiFi.RSSI();
    doc["wireless"]["status"] = WiFi.status();

    doc["system"]["runtime"] = status.debugRunTime;
    doc["system"]["resetReason"] = ESP.getResetReason();
    doc["system"]["cpuFrequency"] = ESP.getCpuFreqMHz();
    doc["system"]["flashCRC"] = ESP.checkFlashCRC();
    doc["system"]["sketchSize"] = ESP.getSketchSize();
    doc["system"]["freeSketchSpace"] = ESP.getFreeSketchSpace();
    doc["system"]["heapFragmentation"] = ESP.getHeapFragmentation();
    doc["system"]["freeHeap"] = ESP.getFreeHeap();
    
    char output[256];
    serializeJson(doc, output);

    server.keepAlive(false);
    server.send(200, "application/json", output);
  });

  server.on("/reset", [&status]() {
    debugln("Webserver isteği alındı: /reset");

    server.send(200, "text/plain", "OK");
    delay(100);
    status.reset();
  });
}