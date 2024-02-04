/*-----------------HEADER FILE---------------*/
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <string.h>
#include <stdlib.h>
#include <rom/rtc.h> 
#include <HTTPClient.h>
#include <ArduinoJson.h>
//OTA UPDATE HEADER FILE
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
//MEMORY CARD PRE-PROCESSOR DIRECTIVE
#include "SPI.h"
#include "time.h"
#include <DS323x.h>
#include <MFRC522.h>
#define SS_PIN 5
#define RST_PIN 2
#define RELAY_PIN 21 //change to correct pin no according to circuit
#define RELAY_PIN 22//change to correct pin no according to Circuit


//WiFi Creadientials
const char* ssid = "PLANT_BK";
const char* password = "Best@#123#";
IPAddress local_IP(192, 168, 2, 15);
IPAddress gateway(192, 168, 3, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 3, 7);

const char* serverURL = "https://rfid/api";

// WiFi Function
void wifi_reconnect()
{
  if(WiFi.status() != WL_CONNECTED) {
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  delay(500);
  Serial.print(".");
  }
}

/*--------------------RTC Library----------------------*/
String device_id="RFID_DEVICE_001";
WebServer Server;
DS323x rtc;
String c_date,c_time,c_timestamp;
int rtc_update_flag;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec =19800;
const int   daylightOffset_sec = 0;
struct tm timeinfo;


// PRINT LOCAL TIME
void printLocalTime()
{
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    rtc_update_flag=0;

  }
  else
  {
    rtc_update_flag=1;
    Serial.print("TIME NOW:");
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  }
return;
}
//GET Date and time timestamp
void get_date_time_timestamp()
{                     
int month1,date1;
String month2,date2;
month1=rtc.month();
date1=rtc.day();
if(month1<10)
 {
   month2="0"+(String)month1;
 }
 else
 {
  month2=rtc.month();
 }
if(date1<10)
 {
   date2="0"+(String)date1;
 }
 else
 {
  date2=rtc.day();
 }
c_date="20"+(String)rtc.year()+""+(String)month2+""+(String)date2;
c_time=(String)rtc.hour()+""+(String)rtc.minute()+""+(String)rtc.second()+",";
c_timestamp=c_date+" "+c_time;
//Serial.println(c_date);
//Serial.println(c_time);
//Serial.println(c_timestamp);
delay(100);
}

/**
 		Helper routine to dump a byte array as hex values to Serial.
*/
void printHex(byte *buffer, byte bufferSize) {
 	for (byte i = 0; i < bufferSize; i++) {
 			Serial.print(buffer[i] < 0x10 ? " 0" : " ");
 			Serial.print(buffer[i], HEX);
 	}
}
/**
 		Helper routine to dump a byte array as dec values to Serial.
*/
void printDec(byte *buffer, byte bufferSize) {
 	for (byte i = 0; i < bufferSize; i++) {
 			Serial.print(buffer[i] < 0x10 ? "0" : "");
 			Serial.print(buffer[i], DEC);
 	}
}
//Parameters
const int ipaddress[4] = {103, 97, 67, 25};
//Variables
byte nuidPICC[4] = {0, 0, 0, 0};
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);

//-------------------READ RFID----------------//
void readRFID(void ) { /* function readRFID */
 	////Read RFID card
 	for (byte i = 0; i < 6; i++) {
 			key.keyByte[i] = 0xFF;
 	}
 	// Look for new 1 cards
 	if ( ! rfid.PICC_IsNewCardPresent())
 			return;
 	// Verify if the NUID has been readed
 	if ( 	!rfid.PICC_ReadCardSerial())
 			return;
 	// Store NUID into nuidPICC array
 	for (byte i = 0; i < 4; i++) {
 			nuidPICC[i] = rfid.uid.uidByte[i];
 	}
   get_date_time_timestamp();
  // Serial.print(F("timestamp "));
  // Serial.print(F(c_timestamp.c_str()));
 	// Serial.print(F("rfid_tag "));
 	printDec(rfid.uid.uidByte, rfid.uid.size);
 	Serial.println();
 	// Halt PICC
 	rfid.PICC_HaltA();
 	// Stop encryption on PCD
 	rfid.PCD_StopCrypto1();
}

void upload_data(const String& rfid_tag, const String& timestamp) {
  // Prepare the data payload
  DynamicJsonDocument payload(256);
  payload["rfid_tag"] = rfid_tag;
  payload["timestamp"] = timestamp;

  // Serialize the JSON payload
  String payloadString;
  serializeJson(payload, payloadString);

  // Make a POST request to the API endpoint to upload the data
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverURL);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(payloadString);

  if (httpResponseCode == 200) {
    Serial.println("Data uploaded successfully!");
  } else {
    Serial.print("Failed to upload data. Error code: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}

/*-----------------------------------SETUP---------------------------------------*/
void setup(){
  Serial.begin(115200);
  Wire.begin();
  SPI.begin();
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.print("Connected to WiFi With IP Address:");
  Serial.println(WiFi.localIP());

//Set nad update RTC Date Time from NTP Server
rtc.attach(Wire);
printLocalTime(); 
delay(1000);
if(rtc_update_flag==1)//UPDATE RTC IF NTP Server Returns OK Flag
{
int year1=timeinfo.tm_year+1900;
int month1=timeinfo.tm_mon+1;
rtc.now(DateTime(year1, month1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec)); 
}
get_date_time_timestamp();

Serial.println(F("Initialize System"));
rfid.PCD_Init();
Serial.print(F("Reader :"));
rfid.PCD_DumpVersionToSerial();

}
void loop() {
  //get_date_time_timestamp();
  	readRFID();
}