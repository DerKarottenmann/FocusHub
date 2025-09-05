#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiUdp.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include <ESP8266WiFiGeneric.h>
#include <ESP8266WiFiSTA.h>
#include <ESP8266WiFiAP.h>
#include <ESP8266WiFiScan.h>
#include <ESP8266WiFiType.h>


#define RESET_BUTTON_PIN D3
#define CLEAR_BUTTON_PIN D1
#define LED_PIN D4

void blinkLED() {
  pinMode(LED_PIN, OUTPUT);
  while (true) {
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED AN");
    delay(2500);
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED AUS");
    delay(2500);
  }
}

bool checkAndResetWifi() {
  pinMode(CLEAR_BUTTON_PIN, INPUT_PULLUP);
  unsigned long pressedTime = 0;
  const unsigned long requiredHold = 5000;

  if (digitalRead(CLEAR_BUTTON_PIN) == LOW) {
    Serial.println("Clear-Taster erkannt. Bitte gedrückt halten...");
    pressedTime = millis();
    while (digitalRead(CLEAR_BUTTON_PIN) == LOW) {
      if (millis() - pressedTime >= requiredHold) {
        Serial.println("Taster 5 Sekunden gehalten. WLAN-Daten werden gelöscht...");
        WiFi.disconnect(true);
        delay(1000);
        Serial.println("Starte WPS...");
        while (WiFi.status() != WL_CONNECTED) {
          if (WiFi.beginWPSConfig()) {
            Serial.println("WPS erfolgreich!");
            Serial.print("Verbunden mit: ");
            Serial.println(WiFi.SSID());
            Serial.println("Warte auf IP-Adresse...");
            while (WiFi.status() != WL_CONNECTED) {
              delay(1000);
              Serial.print(".");
            }
            Serial.print("IP-Adresse: ");
            Serial.println(WiFi.localIP());
          } else {
            Serial.println("WPS fehlgeschlagen.");
            blinkLED();
          }
          delay(500);
        }
        return true;
        break;
      }
      delay(10);
    }
    if (millis() - pressedTime < requiredHold) {
      Serial.println("Taster nicht lange genug gehalten. Kein Reset.");
      return false;
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  Serial.println("Setup complete.");

  if (!checkAndResetWifi())
    Serial.println("WLAN-Zugangsdaten NICHT gelöscht");
  else
    Serial.println("WLAN-Zugangsdaten gelöscht");

  WiFi.mode(WIFI_STA);
  Serial.println("Verbinde mit gespeichertem WLAN...");
  WiFi.begin();
  delay(10000);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Setup: Starte WPS...");
    WiFi.beginWPSConfig();
    int timeout = 20;
    while (WiFi.status() != WL_CONNECTED && timeout > 0) {
      delay(1000);
      Serial.print(".");
      timeout--;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nVerbunden mit: " + WiFi.SSID());
    Serial.print("IP-Adresse: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nNicht verbunden.");
    Serial.println("Starte LED-Blinken als Hinweis...");
    blinkLED();
  }
}

void loop() {
  Serial.println("Verbunden...");
  delay(5000);
}

