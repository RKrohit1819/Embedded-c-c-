#include "my_library.h"

// String MyLibrary::getFormattedMacAddress() {
String getFormattedMacAddress() {
  WiFi.mode(WIFI_MODE_STA);
  String macAddress = WiFi.macAddress();
  macAddress.replace(":", "");
  return macAddress;
}
