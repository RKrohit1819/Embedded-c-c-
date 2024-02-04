#include "WiFi.h"

String macAddress;
String formattedMacAddress;

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_MODE_STA);
  Serial.println(WiFi.macAddress());
  macAddress = WiFi.macAddress();
  Serial.println(macAddress);

  // Remove the colon from the MAC address
  formattedMacAddress = macAddress;
  formattedMacAddress.replace(":", "");
  Serial.println(formattedMacAddress);
}

void loop() {

}
