#include <Wire.h>
#include <RTClib.h>
#include <DHT.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <time.h>
#include <WiFiManager.h> // <- tambahan: WiFiManager

// ==========================================================
// KONFIGURASI PERANGKAT
// ==========================================================
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES   4
#define CLK_PIN       14  // D5
#define DATA_PIN      13  // D7
#define CS_PIN        15  // D8

#define DHTPIN  D4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

RTC_DS3231 rtc;
MD_Parola display = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
ESP8266WebServer server(80);

// ==========================================================
// KONFIGURASI JARINGAN
// ==========================================================
const long gmtOffset_sec      = 7 * 3600; // GMT+7 (WIB)
const int daylightOffset_sec  = 0;

bool wifiConnected = false;
bool showOnlyTime  = false;
IPAddress localAPIP; // IP untuk mode offline

// ==========================================================
// VARIABEL UTAMA
// ==========================================================
unsigned long lastUpdate       = 0;
const unsigned long updateInterval = 1000; // 1 detik
char buffer[64];

// ==========================================================
// FUNGSI TAMBAHAN
// ==========================================================
void showIpClean(const String &ipStr) {
  display.displayClear();
  display.displayReset();
  delay(100);

  display.displayText(ipStr.c_str(), PA_LEFT, 100, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  display.displayReset();

  int putaran = 0;
  const int maxPutaran = 3;
  while (putaran < maxPutaran) {
    if (display.displayAnimate()) {
      display.displayReset();
      putaran++;
    }
    yield();
  }

  //  Tambahan: pastikan buffer bersih total setelah IP tampil
  display.displayClear();
  display.displayReset();
  delay(200);
}


// ==========================================================
// SETUP UTAMA
// ==========================================================
void setup() {
  Serial.begin(115200);
  Wire.begin();
  dht.begin();
  display.begin();
  display.setIntensity(5);
  display.displayClear();

  display.displayText("Mulai..", PA_CENTER, 100, 1000, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
  while (!display.displayAnimate()) { yield(); }

  if (!rtc.begin()) {
    Serial.println("RTC tidak terdeteksi!");
    display.displayClear();
    display.displayText("KESALAHAN RTC", PA_CENTER, 100, 1000, PA_BLINDS, PA_BLINDS);
    while (true);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC kehilangan daya, menyetel ulang waktu...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // --- Koneksi WiFi menggunakan WiFiManager ---
  Serial.println("Memulai WiFiManager...");
  WiFiManager wm;
  wm.setTimeout(30);

  if (wm.autoConnect("ESP8266_AP")) {
    wifiConnected = true;
    String ipAddress = WiFi.localIP().toString();
    Serial.printf("WiFi tersambung. IP: %s\n", ipAddress.c_str());

    display.displayClear();
    display.displayText("WIFI", PA_CENTER, 100, 1000, PA_GROW_UP, PA_GROW_DOWN);
    while (!display.displayAnimate()) { yield(); }
    delay(200);

    showIpClean(ipAddress);

    configTime(gmtOffset_sec, daylightOffset_sec, "pool.ntp.org", "time.nist.gov");
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      rtc.adjust(DateTime(
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
      ));
      Serial.println("RTC berhasil disinkron dari Internet.");
    } else {
      Serial.println("Gagal sinkronisasi waktu NTP.");
    }

  } else {
    // ======================================================
    // MODE OFFLINE: Bikin WiFi sendiri agar web tetap bisa kontrol
    // ======================================================
    wifiConnected = false;
    Serial.println("Tidak ada WiFi. Mengaktifkan mode Access Point lokal...");

    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP8266_LOCAL", "12345678"); // SSID & password (bisa ubah)
    localAPIP = WiFi.softAPIP();

    Serial.print("Access Point aktif. IP lokal: ");
    Serial.println(localAPIP);

    display.displayClear();
    display.displayText("OFFLINE", PA_CENTER, 100, 1000, PA_SCROLL_RIGHT, PA_SCROLL_LEFT);
    while (!display.displayAnimate()) { yield(); }
    delay(200);
    showIpClean(localAPIP.toString());
  }

  // ==========================================================
  // ROUTE SERVER
  // ==========================================================
  server.on("/status", []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    DateTime now = rtc.now();

    char timeStr[9];
    sprintf(timeStr, "%02d:%02d:%02d", now.hour(), now.minute(), now.second());

    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float rtcTemp = rtc.getTemperature();
    float temp = !isnan(t) ? t : rtcTemp;
    float hum  = !isnan(h) ? h : 0;

    String json = "{";
    json += "\"time\":\"" + String(timeStr) + "\"";
    if (!showOnlyTime) {
      json += ",\"temperature\":" + String(temp, 1);
      json += ",\"humidity\":" + String(hum, 1);
    }
    json += "}";
    server.send(200, "application/json", json);
  });

  server.on("/sync-ntp", []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    if (!wifiConnected) {
      server.send(400, "text/plain", "Tidak ada koneksi WiFi");
      return;
    }

    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      rtc.adjust(DateTime(
        timeinfo.tm_year + 1900,
        timeinfo.tm_mon + 1,
        timeinfo.tm_mday,
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
      ));
      display.displayClear();
      display.displayText("SET", PA_CENTER, 100, 1000, PA_GROW_UP, PA_GROW_DOWN);
      while (!display.displayAnimate()) { yield(); }
      server.send(200, "text/plain", "RTC berhasil disinkron dari Internet");
    } else {
      display.displayClear();
      display.displayText("ERR", PA_CENTER, 100, 1000, PA_FADE, PA_FADE);
      while (!display.displayAnimate()) { yield(); }
      server.send(500, "text/plain", "Gagal sinkron NTP");
    }
  });

  server.on("/set-time", []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    if (!server.hasArg("time")) {
      server.send(400, "text/plain", "Parameter 'time' wajib (HH:MM[:SS])");
      return;
    }

    String timeStr = server.arg("time");
    int jam, mnt, dtk = 0;
    if (sscanf(timeStr.c_str(), "%d:%d:%d", &jam, &mnt, &dtk) < 2) {
      server.send(400, "text/plain", "Format waktu tidak valid");
      return;
    }

    DateTime now = rtc.now();
    rtc.adjust(DateTime(now.year(), now.month(), now.day(), jam, mnt, dtk));

    showOnlyTime = false;
    display.displayClear();
    display.displayText("SAVED", PA_CENTER, 100, 1000, PA_SCROLL_UP, PA_SCROLL_DOWN);
    while (!display.displayAnimate()) { yield(); }
    server.send(200, "text/plain", "Waktu diatur ke " + timeStr);
  });

  server.on("/display-time-only", []() {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    if (!server.hasArg("mode")) {
      server.send(400, "text/plain", "Parameter 'mode' wajib (0 atau 1)");
      return;
    }

    int mode = server.arg("mode").toInt();
    showOnlyTime = (mode == 1);

    display.displayClear();
    display.displayText(showOnlyTime ? "JAM" : "AUTO", PA_CENTER, 100, 1000, PA_FADE, PA_FADE);
    while (!display.displayAnimate()) { yield(); }
    server.send(200, "text/plain", showOnlyTime ? "Mode Jam Aktif" : "Mode Otomatis Aktif");
  });

  server.begin();
  display.displayClear();
display.displayReset();
delay(300);
display.displayText("READY", PA_CENTER, 100, 1000, PA_FADE, PA_FADE);
while (!display.displayAnimate()) { yield(); }

  Serial.println("Server web aktif di port 80");
}

// ==========================================================
// LOOP UTAMA
// ==========================================================
void loop() {
  server.handleClient(); // Sekarang aktif baik online maupun offline

  if (display.displayAnimate()) {
    if (millis() - lastUpdate > updateInterval) {
      lastUpdate = millis();

      DateTime now = rtc.now();
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      float rtcTemp = rtc.getTemperature();

      int temp = !isnan(t) ? (int)t : (int)rtcTemp;
      int hum  = !isnan(h) ? (int)h : -1;

      if (showOnlyTime) {
        snprintf(buffer, sizeof(buffer), "%02d:%02d", now.hour(), now.minute());
        display.displayText(buffer, PA_CENTER, 100, 0, PA_FADE, PA_FADE);
      } else {
        if (hum >= 0)
          snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d T:%02dC H:%02d%%",
                   now.hour(), now.minute(), now.second(), temp, hum);
        else
          snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d T:%02dC",
                   now.hour(), now.minute(), now.second(), temp);

        display.displayText(buffer, PA_LEFT, 150, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
      }
    }
  }
  yield();
}
