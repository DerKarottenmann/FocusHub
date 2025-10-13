#include <Arduino.h>            
#include <DNSServer.h>    
#include <ESP8266WiFi.h>            
#include <ESP8266HTTPClient.h>     
#include <ArduinoJson.h>  
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <FS.h>
#include <LittleFS.h>
#include "network.h"      // in /include
#include "eeprom_management.h"      // in /include 
#include "api_requests.h"      // in /include

#include "secret.h"                    // Enthält API-Key + PLZ + Land

extern bool checkAndResetWifi(ESP8266WebServer& server, DNSServer& dnsServer);
extern void setupAP(ESP8266WebServer& server, DNSServer& dnsServer);
extern bool tryConnectToWiFi(String ssid, String password);
extern std::tuple<std::string, std::string, std::string, std::string> getWeather(double lat, double lng, const String& key);
extern std::pair<double, double> getCoordinates(const String& plz, const String& land, const String& key);



#define CLEAR_BUTTON_PIN D3         
#define LED_PIN D4 
//SDA-Pin D2
//SCL-Pin D1
// LCD-Display init
LiquidCrystal_I2C lcd(0x27, 16, 2);
// test
// API-Konfiguration
const char* apiKey = API_KEY;
const char* postalCode = POSTAL_CODE;         
const char* countryCode = COUNTRY_CODE; 
String Temperatur;
String Luftfeuchtigkeit;
String Wind;
String Zustand; 


// server starten
ESP8266WebServer server(80);
DNSServer dnsServer;


// was ist das?
// Fail-Safe LED
void blinkLED() {
  pinMode(LED_PIN, OUTPUT);
  while (true) {
    digitalWrite(LED_PIN, LOW);    
    delay(500);
    digitalWrite(LED_PIN, HIGH);   
    delay(500);
  }
}



void setup() {
  Serial.begin(115200); 
  if (!LittleFS.begin()) {
    Serial.println("An Error has occurred while mounting LittleFS");
  }
  Wire.begin(D2, D1);
  server.begin();
  EEPROM.begin(64); // 32 bytes für SSID + 32 für Passwort
  Serial.println("Server gestartet");
  lcd.init();
  lcd.backlight();
  lcd.print("Starte...");

  delay(5000);          


  // WLAN-Reset prüfen
  if (!checkAndResetWifi(server, dnsServer)) {
    Serial.println("Verbinde mit gespeichertem WLAN...");
    // Gespeicherte Daten abrufen
    String ssid = readSSID();
    String password = readPassword();
    if (ssid.length() > 0 and password.length() > 0) {
      // Mit gespeicherten Daten verbinden
      if (tryConnectToWiFi(ssid, password)) {
        Serial.println("WLAN Verbindung erfolgreich.");
      } else {
        // Verbindung fehlgeschlagen, AP-Modus starten
        setupAP(server, dnsServer);
        server.begin();
      }
    } else {
      // Keine gespeicherten Daten, AP-Modus starten -> was tun??
      setupAP(server, dnsServer);
      server.begin();
    }
    
  }

  // Verbindung anzeigen
  Serial.println("Verbunden mit: " + WiFi.SSID());
  Serial.println("IP: " + WiFi.localIP().toString());

  // Koordinaten abrufen
  auto coords = getCoordinates(postalCode, countryCode, apiKey);
  Serial.println("Koordinaten: " + String(coords.first, 6) + "," + String(coords.second, 6));

  // Wetterdaten abrufen
  getWeather(coords.first, coords.second, apiKey);
}


void loop() {
  delay(60000); //60 Sekunden warten
  Serial.println("\n Aktualisierte Wetterdaten ");
  auto coords = getCoordinates(postalCode, countryCode, apiKey);
  Serial.println("Koordinaten: " + String(coords.first, 6) + "," + String(coords.second, 6));
  getWeather(coords.first, coords.second, apiKey);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(Temperatur);
  lcd.write(223); // Gradzeichen ASCII
  lcd.print("C");
  lcd.setCursor(5,0);
  lcd.print(Luftfeuchtigkeit);
  lcd.write(37);
  lcd.setCursor(0,1);
  lcd.print(Wind);
  lcd.print("km/h");
  lcd.setCursor(8,1);
  lcd.print(Zustand);

  dnsServer.processNextRequest();
  server.handleClient();

}