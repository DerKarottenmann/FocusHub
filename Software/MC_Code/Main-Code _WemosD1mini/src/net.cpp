#include <FS.h>
#include <LittleFS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>

const char* ap_ssid = "MyDeviceSetup";
const char* ap_password = "12345678";

ESP8266WebServer server(80);
DNSServer dnsServer;

IPAddress apIP(192, 168, 4, 1);
const byte DNS_PORT = 53;

//* HTML für unsere Hotelwlan seite 
//!W-Kommentar mit Hotelwlan
const char* htmlForm = R"rawliteral(
<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>WiFi Setup</title>
    <style>
      body { font-family: Arial; background-color: #1a1a1a; color: #ddd; text-align: center; }
      input, button { margin: 10px; padding: 8px; border-radius: 6px; border: none; }
      button { background-color: #444; color: white; cursor: pointer; }
      button:hover { background-color: #666; }
    </style>
  </head>
  <body>
    <h2>Verbinde dein Gerät mit deinem WLAN</h2>
    <form action="/save" method="POST">
      <input type="text" name="ssid" placeholder="SSID" required><br>
      <input type="password" name="password" placeholder="Passwort" required><br>
      <button type="submit">Verbinden</button>
    </form>
  </body>
</html>
)rawliteral";


void readWiFiCredentials(String &ssid, String &password) {
  EEPROM.begin(64);
  char ssidBuf[32];
  char passBuf[32];
  for (int i = 0; i < 32; ++i) {
    ssidBuf[i] = EEPROM.read(i);
    passBuf[i] = EEPROM.read(32 + i);
  }
  ssidBuf[31] = 0;
  passBuf[31] = 0;
  ssid = String(ssidBuf);
  password = String(passBuf);
}

//* Setup 
void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  delay(500);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP: ");
  Serial.println(IP);

  // leitet alles auf unser Gerät
  dnsServer.start(DNS_PORT, "*", apIP);

  server.on("/", []() {
    server.send(200, "text/html", htmlForm);
  });

  server.on("/save", HTTP_POST, []() {
    String ssid = server.arg("ssid"); // vars aus html holen
    String password = server.arg("password");
    
    Serial.println("SSID: " + ssid);
    Serial.println("Password: " + password);

    // Speichern im EEPROM
    EEPROM.begin(64); //  32 SSID und auch pw
    for (int i = 0; i < ssid.length(); ++i) {
      EEPROM.write(i, ssid[i]);
    }
    EEPROM.write(ssid.length(), 0); 
    for (int i = 0; i < password.length(); ++i) {
      EEPROM.write(32 + i, password[i]);
    }
    EEPROM.write(32 + password.length(), 0);
    EEPROM.commit(); //! wichtig

    server.send(200, "text/html", "<h2>Verbindung wird hergestellt...</h2><p>Du kannst dieses Fenster schließen.</p>");

    // Versuch, sich zu verbinden
    connectToWiFi(ssid, password);
  });

  server.begin();
  Serial.println("Webserver gestartet");
}

void loop() {

    // DNS-Anfragen bearbeiten
  dnsServer.processNextRequest();
  server.handleClient();
}

//* WLAN Verbindung 
void connectToWiFi(String ssid, String password) {
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
