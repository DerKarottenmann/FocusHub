#ifndef EEPROM_MANAGEMENT_H
#define EEPROM_MANAGEMENT_H


String readSSID();
String readPassword();

void writeWiFiCredentials(const String &ssid, const String &password);


#endif
