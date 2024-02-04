#include "my_library.h"

void setup() {
  Serial.begin(115200);
  // Call the library functions and perform tests
  String formattedMacAddress = getFormattedMacAddress();
  Serial.print("MAC address: ");
  Serial.println(formattedMacAddress);

}

void loop() {

}
