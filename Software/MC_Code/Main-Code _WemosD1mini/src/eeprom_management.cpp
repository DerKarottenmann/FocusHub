#include <FS.h>
#include <LittleFS.h>
#include <EEPROM.h>

String readSSID() {
  String ssid;
  char ssidBuf[32];
  for (int i = 0; i < 32; ++i) {
    ssidBuf[i] = EEPROM.read(i);
  }
  ssidBuf[31] = 0;
  ssid = String(ssidBuf);
  return ssid;
}

String readPassword() {
  String password;
  char passBuf[32];
  for (int i = 0; i < 32; ++i) {
    passBuf[i] = EEPROM.read(32 + i);
  }
  passBuf[31] = 0;
  password = String(passBuf);
  return password;
}

void writeWiFiCredentials(const String &ssid, const String &password) {
  for (int i = 0; i < ssid.length(); ++i) {
    EEPROM.write(i, ssid[i]);
  }
  EEPROM.write(ssid.length(), 0); 
  for (int i = 0; i < password.length(); ++i) {
    EEPROM.write(32 + i, password[i]);
  }
  EEPROM.write(32 + password.length(), 0);
  EEPROM.commit(); //! wichtig
}