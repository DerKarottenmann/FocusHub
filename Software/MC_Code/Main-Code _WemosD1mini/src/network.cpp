#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <LittleFS.h>
#include "eeprom_management.h"


const char* ap_ssid = "FocusHub_Setup_AP";
const char* ap_password = "12345678";

//* WLAN Verbindung 
bool tryConnectToWiFi(String ssid, String password) {
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  Serial.println("Verbinde mit Heim-WLAN...");

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 20000) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Verbunden! IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Fehler - starte wieder im AP-Modus.");
    WiFi.disconnect();
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_password);
  }
}


// Access point (also Hotelwlan seite) einrichten
void setupAP(ESP8266WebServer &server, DNSServer &dnsServer, 
             int DNS_PORT = 53, IPAddress apIP = IPAddress(192, 168, 4, 1)) {
  Serial.println("Starte Access Point...");

  WiFi.softAP(ap_ssid, ap_password);
  delay(500);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("Access Point Name: ");
  Serial.println(ap_ssid);
  Serial.println("Access Point Password: ");
  Serial.println(ap_password);

  dnsServer.start(DNS_PORT, "*", apIP);


  // Routen definieren -> dann weiß der Server wann was zu tun ist
  // Root-Seite (Formular)
  server.on("/", [&server]() {
    File file = LittleFS.open("/AP_form.html", "r");
    server.streamFile(file, "text/html");
    file.close();
  });
  // Formular absenden (SSID + Passwort speichern) und Verbindung versuchen
  server.on("/save", HTTP_POST, [&server]() {
    String ssid = server.arg("ssid");
    String password = server.arg("password");

    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);

    writeWiFiCredentials(ssid, password);
    server.send(200, "text/html", "<h2>Verbindung wird hergestellt...</h2><p>Du kannst dieses Fenster schließen.</p>");
    tryConnectToWiFi(ssid, password);
  });
}


bool checkAndResetWifi(int CLEAR_BUTTON_PIN = D3, ESP8266WebServer &server, DNSServer &dnsServer) {
  pinMode(CLEAR_BUTTON_PIN, INPUT_PULLUP); // Taster hat PullupWds
  unsigned long pressedTime = 0;
  const unsigned long requiredHold = 5000; 

  if (digitalRead(CLEAR_BUTTON_PIN) == LOW) {
    pressedTime = millis();
    while (digitalRead(CLEAR_BUTTON_PIN) == LOW) {
      if (millis() - pressedTime >= requiredHold) {

        // neue WlanDaten holen -> AP starten
        setupAP(server, dnsServer);
        return true;
      }
      delay(10);
    }
  }
  return false; // Kein Reset durchgeführt
}
