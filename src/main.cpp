#include <Arduino.h>
#include "Guineapig.WiFiConfig.h"
#include "web-assets.h"

#pragma region Async Web Server ***************************
#include <ESPAsyncWebServer.h>
#ifdef ESP32
#include <WiFi.h>
#define LED_ON_VAL HIGH
#define LED_OFF_VAL LOW
#else
#include <ESP8266WiFi.h>
#define LED_ON_VAL LOW
#define LED_OFF_VAL HIGH
#endif
AsyncWebServer webServer(80);
#pragma endregion *****************************************

bool resetWifiFlag = false;
int curr_led_val = LED_ON_VAL;

String processor(const String& var) {
  if (var == "LED_ONOFF") return String(curr_led_val == LED_ON_VAL ? "on" : "off");
  return String();
}
String procBulbOn(const String& var) {
  if (var == "COLOR1") return "#fedc94";
  else if (var == "COLOR2") return "#f2ce75";
  return String();
}
String procBulbOff(const String& var) {
  if (var == "COLOR1") return "#ccc";
  else if (var == "COLOR2") return "#ccc";
  return String();
}

void setup()
{
  Serial.begin(115200);
  //嘗試連上無線網路，若成功傳回 true 繼續作業；若失敗則啟用 AP 模式讓使用者連上來設定網路
  if (WiFiConfig.connectWiFi())
  {
    pinMode(LED_BUILTIN, OUTPUT);
    //建立一個小網站讓使用者開關LED燈及清除網路設定(方便實驗觀察用，一般應用時不太需要)
    webServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "text/html", index_html, processor);
    });
    webServer.on("/bulb-on.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "image/svg+xml", bulb_svg, procBulbOn);
    });
    webServer.on("/bulb-off.svg", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send_P(200, "image/svg+xml", bulb_svg, procBulbOff);
    });
    //點燈
    webServer.on("/led/on", HTTP_GET, [](AsyncWebServerRequest *request) {
      digitalWrite(LED_BUILTIN, LED_ON_VAL);
      curr_led_val = LED_ON_VAL;
      request->send(200, "text/plain", "OK");
    });
    //關燈
    webServer.on("/led/off", HTTP_GET, [](AsyncWebServerRequest *request) {
      digitalWrite(LED_BUILTIN, LED_OFF_VAL);
      curr_led_val = LED_OFF_VAL;
      request->send(200, "text/plain", "OK");
    });
    //重設網路
    webServer.on("/reset-wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
      resetWifiFlag = true;
      request->send(200, "text/plain", "Reset and rebooting...");
    });
    webServer.begin();
    digitalWrite(LED_BUILTIN, curr_led_val);
  }
}

void loop()
{
  if (resetWifiFlag)
  {
    delay(3000);
    WiFiConfig.clearWiFiConfig();
  }
  delay(500);
}