//!! alanları doldurduktan sonra dosya adından "_" kaldırılmalı

#define DEBUG

#ifdef DEBUG
#define debug(x) Serial.print(x)
#define debugln(x) Serial.println(x)
#else
#define debug(x)
#define debugln(x)
#endif

#define WIFI_SSID "WiFi adı"
#define WIFI_PASS "WiFi şifresi"

#define SERVO_PIN 4 //GPIO4, D2
#define SERVO_ANG_LOCKED 180
#define SERVO_ANG_UNLOCKED 90