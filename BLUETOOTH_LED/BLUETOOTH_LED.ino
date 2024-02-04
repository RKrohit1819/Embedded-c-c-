#include "BluetoothSerial.h"
#define LED_BUILTIN 2

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;

String message = "";
char incomingChar;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  SerialBT.begin("ESP32test"); 
  Serial.println("The device started, now you can pair it with bluetooth!");
}

void loop() {
  if(SerialBT.available()){
    char incomingChar = SerialBT.read();
    if (incomingChar != '\n'){
      message += String(incomingChar);
    }
    else{
      message = "";
    }
Serial.write(incomingChar);    
      }

  if(message == "led_on"){
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if(message == "led_off"){
    digitalWrite(LED_BUILTIN, LOW);
  }
  delay(20);
}

