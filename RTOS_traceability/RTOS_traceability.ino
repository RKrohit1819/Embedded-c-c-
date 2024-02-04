//------------------------HEADER FILE-----------------------------//
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <string.h>
#include <stdlib.h>
#include <rom/rtc.h> 
#include <HTTPClient.h>
#include <ArduinoJson.h>

//------------------OTA UPDATE HEADER FILE------------------------//
// #include <AsyncTCP.h>
// #include <ESPAsyncWebServer.h>
// #include <AsyncElegantOTA.h>

//-----------MEMORY CARD PRE-PROCESSOR DIRECTIVE-----------------//
#include "SPI.h"
#include "time.h"
#include <DS323x.h>
#include <MFRC522.h>
#include "time.h"
// time_t start_time, end_time,up_start_time,up_end_time;
// double diff_time,up_diff_time;

//----------------SETTING UP THE PIN IN ESP----------------------// 
#define SS_PIN 5
#define RST_PIN 2
#define RELAY_PIN1 0 
#define RELAY_PIN2 4
#define GREEN_LED 14
#define RED_LED 12
#define MAC_ID "RFID_001"

//------------------------json Variable-------------------------//
String jsonBuffer;
int httpResponseCode;


//------------------------Dual Core------------------------------//
#if CONFIG_FREERTOS_MULTICORE
static const BaseType_t app_cpu = 0;
#else
static const BaseType_t app_cpu = 1;
#endif

// static TaskHandle_t task_1 = NULL;
// static TaskHandle_t task_2 = NULL;

//-----------------------server URL-----------------------------//
const char* serverURL = "http://192.168.3.19/eol/module/autoprn/API/api/rfid_api.php?rfid=12345678901011121001&device_id=device0001&product_id=YRA&c_timestamp=2023-05-25";
const char* serverPost = "http://192.168.3.19/eol/module/autoprn/API/api/rfid_api.php?";

//---------------------WiFi Creadientials-----------------------//
const char*  ssid = "PLANT_BK";
const char*  password = "Best@#123#";
IPAddress    local_IP(192, 168, 2, 11);
IPAddress    gateway(192, 168, 3, 1);
IPAddress    subnet(255, 255, 255, 0);
IPAddress    primaryDNS(192, 168, 3, 7);

//---------------------- WiFi Function----------------------------//
void wifi_reconnect()
{
  if(WiFi.status() != WL_CONNECTED) {
    if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  redLedOn();
  Serial.print(".");
  }
   Serial.println("Connected to WiFi");
   Serial.print("IP Address: ");
   redLedOff();
   Serial.println(WiFi.localIP());
  }
}


//--------------------------RTC Library---------------------------//
//String device_id="RFID_DEVICE_001";
WebServer Server;
DS323x rtc;
String c_date,c_time,c_timestamp,c_date1,c_time1,c_timestamp1;
int rtc_update_flag;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec =19800;
const int   daylightOffset_sec = 0;
struct tm timeinfo;


//--------------------- PRINT LOCAL TIME------------------------//
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

//-----------------------GET Date and time timestamp------------------------//
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
c_date="20"+(String)rtc.year()+"-"+(String)month2+"-"+(String)date2;
c_date1="20"+(String)rtc.year()+""+(String)month2+""+(String)date2;
c_time=(String)rtc.hour()+":"+(String)rtc.minute()+":"+(String)rtc.second()+",";
c_time1=(String)rtc.hour()+""+(String)rtc.minute()+""+(String)rtc.second()+"";
c_timestamp=c_date+" "+c_time;
c_timestamp1=c_date1+""+c_time1;
// Serial.println(c_date);
// Serial.println(c_time);
// Serial.println(c_timestamp);
Serial.println(c_timestamp1);
delay(100);
}

//--------------------------process command with relay---------------------------//

void processOn(){
  digitalWrite(RELAY_PIN1, HIGH);
  digitalWrite(RELAY_PIN2, HIGH);
  Serial.println("Process is ON");

}

void processOff(){
  digitalWrite(RELAY_PIN1, LOW);
  digitalWrite(RELAY_PIN2, LOW);
  Serial.println("Process is OFF");

}
void redLedOn(){
  digitalWrite(RED_LED, HIGH);

}

void redLedOff(){
  digitalWrite(RED_LED, LOW);

}
void greenLedOn(){
  digitalWrite(GREEN_LED, HIGH);

}
void greenLedOff(){
  digitalWrite(GREEN_LED, LOW);

}
void redLedBlink(){
  digitalWrite(RED_LED, HIGH);
  delay(2000);
  digitalWrite(RED_LED, LOW);
  delay(2000);
}

void greenLedBlink(){
  digitalWrite(GREEN_LED, HIGH);
  delay(2000);
  digitalWrite(GREEN_LED, LOW);
  delay(2000);
}


// Helper routine to dump a byte array as hex values to Serial.
void printHex(byte *buffer, byte bufferSize) {
 	for (byte i = 0; i < bufferSize; i++) {
 			Serial.print(buffer[i] < 0x10 ? " 0" : " ");
 			Serial.print(buffer[i], HEX);
 	}
}


// Helper routine to dump a byte array as dec values to Serial.
void printDec(byte *buffer, byte bufferSize) {
 	for (byte i = 0; i < bufferSize; i++) {
 			Serial.print(buffer[i] < 0x10 ? "0" : "");
 			// Serial.print(buffer[i], DEC);
 	 }
}

//--------------------------HTTP Request---------------------------//
String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
  // Your Domain name with URL path or IP address with path
  http.begin(client, serverName);
  // Send HTTP POST request
  httpResponseCode = http.GET();
  String payload = "{}"; 
  if (httpResponseCode>0) {
    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  if(httpResponseCode!=200)
  {
  }
  http.end();
  return payload;
}
//Parameters
//const int ipaddress[4] = {103, 97, 67, 25};
//Variables
byte nuidPICC[4] = {0, 0, 0, 0};
String rfid_key;
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);
//----------------------------READ RFID----------------------------//
void readRFID(const String& c_timestamp,  String& rfid_key) { 
 	////Read RFID card
   if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;
    checkProductionData();

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));
            // Store NUID into nuidPICC array
    //   for (byte i = 0; i < 4; i++) {
    //   nuidPICC[i] = rfid.uid.uidByte[i]; 
    //   }
    //     for (byte i = 0; i <rfid.uid.size; i++) {
    //     rfid_key += String(rfid.uid.uidByte[i]);    
    // }
    for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i]; 
}

for (byte i = 0; i < rfid.uid.size; i++) {
    rfid_key += String(rfid.uid.uidByte[i]);
}
    get_date_time_timestamp();
    greenLedOn();
    Serial.println(F("The NFC ID tag is:"));
    // Serial.print(F("In hex: "));
    // printHex(rfid.uid.uidByte, rfid.uid.size);
    //Serial.println();
    // Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    // Serial.println();
    // Serial.println("--------------PLEASE NOTE---------------------------------------");
    // Serial.println("NFC/RFID KEY TO REGISTER ON WEB PORTAL:");
    //Serial.println(rfid_key);
    // Serial.println("----------------COPY THE KEY FROM ABOVE-------------------------");
    // Serial.println("Working");
    Serial.println(rfid_key);
    // Serial.println("Good");
    rfid_key="";
    delay(3000);
    greenLedOff();
    // Halt PICC
    rfid.PICC_HaltA();
    // Stop encryption on PCD
    rfid.PCD_StopCrypto1();
    String c_key = c_timestamp1 + rfid_key;
// http://192.168.3.19/eol/module/autoprn/API/api/rfid_api.php?rfid=12345678901001&device_id=device0001&product_id=YRA&c_timestamp=2023-05-25 14:43:00&ckey=20230525144001&process_type=END
 if(WiFi.status()== WL_CONNECTED){
String serverPath = "http://192.168.3.19/eol/module/autoprn/API/api/rfid_api.php?rfid="+rfid_key+"&device_id="+(String)MAC_ID+"&product_id=YRA&c_timestamp="+(String)c_timestamp+"&ckey="+(String)c_key+"&process_type=END";
jsonBuffer = httpGETRequest(serverPath.c_str());
 }

}

//------------------------Read the data from Server-------------------//
void checkProductionData(){
  HTTPClient http;
  http.begin(serverURL);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println(payload);
    StaticJsonDocument<256> doc;  //Capacity of json Documents in byte
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
    } else {
      //storing the deserialized Data
      int status = doc[0]["status"];
      String error = doc[0]["error"];
      Serial.print("Status: ");
      Serial.println(status);
      Serial.print("Error: ");
      Serial.println(error);
      if(status == 0){
        processOn();
        redLedOn();
        delay(5000);
        redLedOff();
        processOff();

      }
    }
  } else {
    Serial.print("HTTP GET request failed with error code ");
    Serial.println(httpCode);
  }
  http.end();

}

// void updateRFIDData(const String& c_timestamp, const String& rfid_key){
//   DynamicJsonDocument doc(256);

//   // Add the timestamp and RFID tag to the JSON document
//   doc["timestamp"] = c_timestamp;
//   doc["ckey"] = rfid_key;
//   doc["process_type"] = "START";

//   // Serialize the JSON document to a string
//   String jsonStr;
//   serializeJson(doc, jsonStr);

//   // Print the JSON string
//   Serial.println(jsonStr);
// }

//--------------------------Task 1 Definition------------------------//
void TaskOne(void *pvParameters){
 Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  for (;;)
  {
    // Here, write the code for Processor Core 1 Task
    wifi_reconnect();
    // machine_operation();
    delay(1000); // Delay between iterations
  }

}
//--------------------------Task 1 Definition-----------------------//
void TaskTwo(void *pvParameters){

}


//-------------------------------SETUP--------------------------------//
void setup(){
  Serial.begin(115200);
//------------------------- IO Declaration---------------------------//
  pinMode(RELAY_PIN1, OUTPUT);
  pinMode(RELAY_PIN2, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

//-------------------------Connect to WiFi-------------------------//
  Wire.begin();
  SPI.begin();
  wifi_reconnect();
  // if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
  //   Serial.println("STA Failed to configure");
  // }
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(1000);
  //    redLedOn();
  //   Serial.print(".");
      
  //  }
  // Serial.print("Connected to WiFi With IP Address:");
  // redLedOff();
  // Serial.println(WiFi.localIP());

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
// rfid = readRFID();
// Serial.println(rfid);
// checkProductionData();
get_date_time_timestamp();
// updateRFIDData(c_timestamp, rfid_key);

Serial.println(F("Initialize System"));
rfid.PCD_Init();
Serial.print(F("Reader :"));
rfid.PCD_DumpVersionToSerial();


//--------------------------DEFINE TASK FOR CORE 1 ---------------------------//
  xTaskCreatePinnedToCore(
                    TaskOne,     // Task function 
                    "Task1",     // name of task 
                    1024,       // Stack size of task 
                    NULL,        // parameter of the task 
                    1,           // priority of the task 
                    NULL ,    // Task handle to keep track of created task 
                    0);          // pin task to core 0                 
  delay(500); 

//--------------------------DEFINE TASK FOR CORE 2----------------------------//
// xTaskCreatePinnedToCore(
//                     TaskTwo,     // Task function
//                     "Task2",     // name of task 
//                     10000,       // Stack size of task 
//                     NULL,        // parameter of the task 
//                     6,           // priority of the task 
//                     &task_2 ,    // Task handle to keep track of created task 
//                     1);          // pin task to core 1 
//     delay(500); 
 }

//----------------------------------LOOP--------------------------------------//
void loop() {
  //get_date_time_timestamp();
  	readRFID(c_timestamp, rfid_key);
}