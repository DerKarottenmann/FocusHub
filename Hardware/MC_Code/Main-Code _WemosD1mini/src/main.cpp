#include <Arduino.h>                
#include <ESP8266WiFi.h>            
#include <ESP8266HTTPClient.h>     
#include <ArduinoJson.h>            

#define CLEAR_BUTTON_PIN D1         
#define LED_PIN D4                  

// API-Konfiguration
const char* apiKey = "AIzaSyBUWS8aT5PF0w9LRaR6tqlmZWyYCrxgXiE";
const char* postalCode = "80997";         
const char* countryCode = "DE";  

        

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

// Bei Tasterdruck WlanData gelöscht + WPS starten
bool checkAndResetWifi() {
  pinMode(CLEAR_BUTTON_PIN, INPUT_PULLUP); // Taster hat PullupWds
  unsigned long pressedTime = 0;
  const unsigned long requiredHold = 5000; 

  if (digitalRead(CLEAR_BUTTON_PIN) == LOW) {
    pressedTime = millis();
    while (digitalRead(CLEAR_BUTTON_PIN) == LOW) {
      if (millis() - pressedTime >= requiredHold) {
        WiFi.disconnect(true);     // WLAN_Daten löschen
        delay(1000);
        Serial.println("Starte WPS...");
        while (WiFi.status() != WL_CONNECTED) {
          if (WiFi.beginWPSConfig()) {
            Serial.println("WPS erfolgreich!");
            return true;
          } else {
            Serial.println("WPS fehlgeschlagen.");
            blinkLED();
          }
        }
        return true;
      }
      delay(10);
    }
  }
  return false; // Kein Reset durchgeführt
}

// Verbindet mit gespeichertem WLAN
bool connectWiFi() {
  WiFi.mode(WIFI_STA);             // Stational-Mode aktivieren
  WiFi.begin();                    // Verbindung
  int timeout = 20;                // Timeout nach 20 Sekunden (sonts keine Zeit für verbinden)

  while (WiFi.status() != WL_CONNECTED && timeout-- > 0) {
    delay(1000);
    Serial.print(".");
  }
  return WiFi.status() == WL_CONNECTED;
}

// Holt die Koordinaten 
std::pair<double, double> getCoordinates(const String& plz, const String& land, const String& key) {
  HTTPClient http;
  String url = "https://maps.googleapis.com/maps/api/geocode/json?address=" + plz + "," + land + "&key=" + key;
  WiFiClientSecure client;
  http.begin(client, url);                 // Verbindung starten
  int httpCode = http.GET();      // Request ausführen

  double lat = 0.0, lng = 0.0;

  if (httpCode == 200) {               // 200 = Erfolg
    String payload = http.getString(); // Antwort als String
    DynamicJsonDocument doc(2048);     // Maximal 2KB
    deserializeJson(doc, payload);     // JSON parsen

    if (doc["results"].size() > 0) {
      lat = doc["results"][0]["geometry"]["location"]["lat"];
      lng = doc["results"][0]["geometry"]["location"]["lng"];
    } else {
      Serial.println("Keine gültigen Koordinaten gefunden.");
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
  http.begin(client ,url);                 
  int httpCode = http.GET();      

  if (httpCode == 200) {
    String payload = http.getString(); 
    DynamicJsonDocument doc(2048);     
    deserializeJson(doc, payload);     

    // Daten extrahieren
    double temp = doc["temperature"]["degrees"];
    double humidity = doc["relativeHumidity"];
    double wind = doc["wind"]["speed"]["value"];
    String condition = doc["weatherCondition"]["description"]["text"];

    // Serial-Ausgabe
    Serial.println("Temperatur: " + String(temp) + "°C");
    Serial.println("Luftfeuchtigkeit: " + String(humidity) + "%");
    Serial.println("Wind: " + String(wind) + " km/h");
    Serial.println("Zustand: " + condition);
  } else {
    Serial.println("Fehler beim Abrufen der Wetterdaten.");
  }

  http.end(); 
}


void setup() {
  Serial.begin(115200);           
  delay(1000);

  // WLAN-Reset prüfen
  if (!checkAndResetWifi()) {
    Serial.println("Verbinde mit gespeichertem WLAN...");
    if (!connectWiFi()) {
      Serial.println("Verbindung fehlgeschlagen. Starte LED-Blinken...");
      blinkLED();
    }
  }

  // Verbindung anzeigen
  Serial.println("Verbunden mit: " + WiFi.SSID());
  Serial.println("IP: " + WiFi.localIP().toString());

  // Koordinaten abrufen
  auto coords = getCoordinates(postalCode, countryCode, apiKey);
  Serial.println("Koordinaten: " + String(coords.first, 6) + ", " + String(coords.second, 6));

  // Wetterdaten abrufen
  getWeather(coords.first, coords.second, apiKey);
}


void loop() {
  delay(60000);
}
