#include <string.h>
#include<stdlib.h>
//MEMORY CARD PRE-PROCESSOR DIRECTIVE
#include "SPI.h"
#include "time.h"
#include <MFRC522.h>
#define SS_PIN 5
#define RST_PIN 0
/* -----------------------------RTC Library---------------------------------------*/
String device_id="RFID_DEVICE_001";
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key; 
// Init array that will store new NUID 
byte nuidPICC[4];
String rfid_key;

/* -----------------------------Timer Delays for API---------------------------------------*/
unsigned int hexToDec(String hexString) {
  
  unsigned int decValue = 0;
  int nextInt;
  
  for (int i = 0; i < hexString.length(); i++) {
    
    nextInt = int(hexString.charAt(i));
    if (nextInt >= 48 && nextInt <= 57) nextInt = map(nextInt, 48, 57, 0, 9);
    if (nextInt >= 65 && nextInt <= 70) nextInt = map(nextInt, 65, 70, 10, 15);
    if (nextInt >= 97 && nextInt <= 102) nextInt = map(nextInt, 97, 102, 10, 15);
    nextInt = constrain(nextInt, 0, 15);
    
    decValue = (decValue * 16) + nextInt;
  }
  
  return decValue;
}
// Converting from Decimal to Hex:

// NOTE: This function can handle a positive decimal value from 0 - 255, and it will pad it
//       with 0's (on the left) if it is less than the desired string length.
//       For larger/longer values, change "byte" to "unsigned int" or "long" for the decValue parameter.


String decToHex(byte decValue, byte desiredStringLength) {
  
  String hexString = String(decValue, HEX);
  while (hexString.length() < desiredStringLength) hexString = "0" + hexString;
  
  return hexString;
}


void setup() {
  rfid_key="";
  Serial.begin(115200); //START SERIAL PORT WITH BAUD RATE 115200 bps
  Serial.println();
  SPI.begin(); // Init SPI bus
  delay(1000);
  rfid.PCD_Init(); // Init MFRC522 
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);  
}

void loop() {
 
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));
            // Store NUID into nuidPICC array
      for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i]; 
      }
        for (byte i = 0; i <rfid.uid.size; i++) {
        rfid_key += String(rfid.uid.uidByte[i]);    
    }
    Serial.println(F("The NFC ID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.println("--------------PLEASE NOTE---------------------------------------");
    Serial.println("NFC/RFID KEY TO REGISTER ON WEB PORTAL:");
    Serial.println(rfid_key);
    Serial.println("----------------COPY THE KEY FROM ABOVE-------------------------");
    Serial.println("Working");
    Serial.println(rfid_key);
    Serial.println("Good");
    rfid_key="";
    // Halt PICC
    rfid.PICC_HaltA();
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();
 
}


/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

/**
 * Helper routine to dump a byte array as dec values to Serial.
 */
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}