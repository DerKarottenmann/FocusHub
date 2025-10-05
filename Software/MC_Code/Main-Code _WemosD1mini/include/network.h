#ifndef NETWORK_H
#define NETRK_H


bool tryConnectToWiFi(String ssid, String password);
void setupAP(ESP8266WebServer &server, DNSServer &dnsServer);
bool checkAndResetWifi();

#endif
