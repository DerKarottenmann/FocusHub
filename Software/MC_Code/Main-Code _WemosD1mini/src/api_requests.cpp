#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include "api_requests.h"


// use auto [a, b, c, d] = getStrings(); to obtain the values
std::tuple<std::string, std::string, std::string, std::string> getWeather(double lat, double lng, const String& key) {
  HTTPClient http;
  String Luftfeuchtigkeit;
  String Temperatur;
  String Wind;
  String Zustand;
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
  return std::make_tuple(Temperatur.c_str(), Luftfeuchtigkeit.c_str(), Wind.c_str(), Zustand.c_str());
}

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