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

#include "secret.h"                    // Enthält API-Key + PLZ + Land

extern bool checkAndResetWifi(ESP8266WebServer& server, DNSServer& dnsServer);
extern void setupAP(ESP8266WebServer& server, DNSServer& dnsServer);
extern bool tryConnectToWiFi(String ssid, String password);


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

// Netzwerk Access Point consts
// Passwort
IPAddress apIP(192, 168, 4, 1); // IP des Wemsos im AP-Modus
const byte DNS_PORT = 53; // Port (53 wird für DNS standartmäßig genutzt)

// server starten
ESP8266WebServer server(80);
DNSServer dnsServer;


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


// Holt die Koordinaten 
std::pair<double, double> getCoordinates(const String& plz, const String& land, const String& key) {
  HTTPClient http;
  String url = "https://maps.googleapis.com/maps/api/geocode/json?address=" + plz + "," + land + "&key=" + key;
  WiFiClientSecure client;
  client.setInsecure(); // Zertifikatsprüfung deaktivieren
  http.begin(client, url);   // Verbindung starten
  delay(500);                 
  int httpCode = http.GET();   // Request ausführen
  Serial.println("HTTP-Code: " + String(httpCode)); // Debug-Ausgabe
  delay(500);

  double lat = 0.0, lng = 0.0;

  if (httpCode == 200) {               // 200 = Erfolg
    String payload = http.getString(); // Antwort als String
    DynamicJsonDocument doc(4096);     // Maximal 4KB
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("JSON-Fehler: ");
      Serial.println(error.c_str());
    } else {
      if (doc["results"].size() > 0) {
        lat = doc["results"][0]["geometry"]["location"]["lat"];
        lng = doc["results"][0]["geometry"]["location"]["lng"];
      } else {
        Serial.println("Keine gültigen Koordinaten gefunden.");
      }
    }
  } else {
    Serial.println("Fehler beim Abrufen der Koordinaten.");
  }

  http.end(); // Verbindung schließen
  return std::make_pair(lat, lng);
}

// Holt Wetterdaten 
void getWeather(double lat, double lng, const String& key) {
  HTTPClient http;
  String url = "https://weather.googleapis.com/v1/currentConditions:lookup?key=" + key +
               "&location.latitude=" + String(lat, 6) + "&location.longitude=" + String(lng, 6);
  WiFiClientSecure client;          
  client.setInsecure(); // Zertifikatsprüfung deaktivieren
  http.begin(client ,url);
  delay(500);                 
  int httpCode = http.GET();
  Serial.println("HTTP-Code: " + String(httpCode)); // Debug-Ausgabe
  delay(500);      

  if (httpCode == 200) {
    String payload = http.getString();
    delay(500); 
    DynamicJsonDocument doc(2048);     
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("JSON-Fehler: ");
      Serial.println(error.c_str());
    } else {
      // Daten extrahieren
      double temp = doc["temperature"]["degrees"];
      double humidity = doc["relativeHumidity"];
      double wind = doc["wind"]["speed"]["value"];
      String condition = doc["weatherCondition"]["description"]["text"];

      // Serial-Ausgabe
      Serial.println("Temperatur: " + String(temp) + "°C");
      Temperatur = String(temp);
      Serial.println("Luftfeuchtigkeit: " + String(humidity) + "%");
      Luftfeuchtigkeit = String(humidity);
      Serial.println("Wind: " + String(wind) + " km/h");
      Wind = String(wind);
      Serial.println("Zustand: " + condition);
      Zustand = condition;
    }
  } else {
    Serial.println("Fehler beim Abrufen der Wetterdaten.");
  }

  http.end(); 
}


void setup() {
  Serial.begin(115200); 
  LittleFS.begin();
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
      tryConnectToWiFi(ssid, password);
    } else {
      // Keine gespeicherten Daten, AP-Modus starten -> was tun??
      setupAP(server, dnsServer);
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

  server.handleClient();

}