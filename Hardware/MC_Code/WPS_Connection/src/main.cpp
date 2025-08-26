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
// #include <ESP8266WiFiWPS.h>

#define RESET_BUTTON_PIN D3  // Passe den Pin ggf. an
#define CLEAR_BUTTON_PIN D1
#define LED_PIN D4          // Onboard LED (D4 für Wemos D1 Mini)

void blinkLED() {
  while (true) {
    digitalWrite(LED_PIN, LOW); // LED an (LOW, da Onboard LED gegen GND geschaltet ist)
    delay(500);
    digitalWrite(LED_PIN, HIGH); // LED aus
    delay(500);
  }
}

bool checkAndResetWifi() {
  pinMode(CLEAR_BUTTON_PIN, INPUT_PULLUP); // Taster gegen GND
  unsigned long pressedTime = 0;
  const unsigned long requiredHold = 5000; // 5 Sekunden

  // Prüfe beim Booten, ob der Taster gedrückt ist
  if (digitalRead(CLEAR_BUTTON_PIN) == LOW) {
    Serial.println("Clear-Taster erkannt. Bitte gedrückt halten...");
    pressedTime = millis();
    while (digitalRead(CLEAR_BUTTON_PIN) == LOW) {
      if (millis() - pressedTime >= requiredHold) {
        Serial.println("Taster 5 Sekunden gehalten. WLAN-Daten werden gelöscht...");
        WiFi.disconnect(true); // Löscht gespeicherte WLAN-Daten
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
          }
          else {
            Serial.println("WPS fehlgeschlagen.");
          }
          delay(500);
        }
        return true; // Reset durchgeführt        
        break;
      }
      delay(10);
    }
    if (millis() - pressedTime < requiredHold) {
      Serial.println("Taster nicht lange genug gehalten. Kein Reset.");
      return false; // Kein Reset durchgeführt
    }
  }
  return false; // Taster nicht gedrückt, kein Reset
}

void setup() {
  Serial.begin(115200);
  Serial.println("Setup complete.");

  if (!checkAndResetWifi())
    Serial.println("WLAN-Zugangsdaten NICHT gelöscht");
  else
    Serial.println("WLAN-Zugangsdaten gelöscht");

  WiFi.mode(WIFI_STA); // Station Mode
  Serial.println("Verbinde mit gespeichertem WLAN...");
  WiFi.begin(); // Verbindet mit gespeicherten Zugangsdaten
  delay(10000); // Warte 5 Sekunden auf Verbindung
  
  if (WiFi.status() != WL_CONNECTED) {    
    Serial.println("setup(): Starte WPS...");
    WiFi.beginWPSConfig();
    int timeout = 20; // 20 Sekunden Timeout
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
  }
  else {
    Serial.println("\nNicht verbunden.");
    Serial.println("Starte LED-Blinken als Hinweis...");
    blinkLED();
  }
}

void loop() {
  Serial.println("Verbunden...");
  delay(1000);

  // if (Serial.available() > 0) {
  //   String input = Serial.readStringUntil('\n');
  //   input.trim();
  //   Serial.print("Eingegeben: ");
  //   Serial.println(input);
  //   if (input == "WPS") {
  //     Serial.println("Starte WPS...");
  //     if (WiFi.beginWPSConfig()) {
  //       Serial.println("WPS erfolgreich!");
  //       Serial.print("Verbunden mit: ");
  //       Serial.println(WiFi.SSID());
  //       Serial.print("IP-Adresse: ");
  //       Serial.println(WiFi.localIP());
  //     } else {
  //       Serial.println("WPS fehlgeschlagen.");
  //     }
  //   } else {
  //     Serial.println("Unknown");
  //   }
  // }
}

