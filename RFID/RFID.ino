#include <WiFi.h>          // Replace with WiFi.h for ESP32
#include <WebServer.h>     // Replace with WebServer.h for ESP32
#include <AutoConnect.h>
#include <string.h>
#include<stdlib.h>
#include<rom/rtc.h> 
#include <HTTPClient.h>
#include <Arduino_JSON.h>
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
#define RST_PIN 0
/* -----------------------------RTC Library---------------------------------------*/

String device_id="RFID_DEVICE_004";
WebServer Server;          // Replace with WebServer for ESP32
AutoConnect  Portal(Server);
AutoConnectConfig Config;
DS323x rtc;
String c_date,c_time,c_timestamp,p_time_stamp;
int rtc_update_flag;
// JSON 
String jsonBuffer;
int httpResponseCode;
int prod_start_flag,prod_stop_flag,data_update_flag;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec =19800;
const int   daylightOffset_sec = 0;
struct tm timeinfo;
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

//GET Date time timestamp
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
c_time=(String)rtc.hour()+":"+(String)rtc.minute()+":"+(String)rtc.second();
c_timestamp=c_date+" "+c_time;
Serial.println(c_date);
Serial.println(c_time);
Serial.println(c_timestamp);
delay(100);
}

/* -----------------------------HTTP Request---------------------------------------*/
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
    Serial.println("Data Send Failure");
  }
  http.end();
  return payload;
}

void send_data(String data){
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://thirdumpire-baseball.com/api/data1.php?rfid_key=" + (String)data +"&device_id=" + (String)device_id+ "&current_date=" + (String)c_date+ "&current_time=" + (String)c_time;
      Serial.println(serverPath);
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
       if(httpResponseCode!=200)
        {
          Serial.println("Data Send to Server Failure");
        }
        else
        {
          Serial.println("Data Send to Server Success");
          data_update_flag=0; 
          rfid_key="";
            for (byte i = 0; i < 4; i++) {
              nuidPICC[i]='\0'; 
  }
                
        }
    }
     else {
      Serial.println("NETWORK FAILURE!");
    }
}
/* -----------------------------Dual Core Task Handler Variabale---------------------------------------*/
TaskHandle_t Task1;
TaskHandle_t Task2;

void rootPage() {
  String content = c_timestamp;
  Server.send(200, "text/plain", content);
}

void setup() {
  rfid_key="";
  Serial.begin(115200); //START SERIAL PORT WITH BAUD RATE 115200 bps
  Wire.begin();  //START I2C FOR TRC Module
  delay(1000); // Request the time correction on the Serial
  Config.autoReconnect = true;    // Attempt automatic reconnection.
  Config.reconnectInterval = 1;   // Seek interval time is 180[s].
  Portal.config(Config);
  Portal.begin();
  Server.on("/", rootPage);
  if (Portal.begin()) {
    Serial.println("WiFi connected: " + WiFi.localIP().toString());
  }

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
  /* -----------------------------SET & UPDATE RTC Date Time from NTP Server---------------------------------------*/
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
/* -----------------------------DEFINE TASK FOR CORE 1---------------------------------------*/
//CORE 1 TASK 1
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");
  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 
/* -----------------------------DEFINE TASK FOR CORE 2---------------------------------------*/
//CORE 2 TASK 2
  //create a task that will be executed in the Task2code() function, with priority 1 and executed on core 1
  xTaskCreatePinnedToCore(
                    Task2code,   /* Task function. */
                    "Task2",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task2,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 1 */
    delay(500); 
}
/* -----------------------------Task 1 Definition---------------------------------------*/
//Task1code: blinks an LED every 1000 ms
void Task1code( void * pvParameters ){
  Serial.print("Task1 running on core ");
  Serial.println(xPortGetCoreID());
  for(;;){
    //HERE WRITE CODE FOR Processor Core 1 Task
      Portal.handleClient();
      //Serial.print("Connected to WiFi network with IP Address: ");
      //Serial.println(WiFi.localIP());
  }
  delay(100);
}
/* -----------------------------Task 2 Definition---------------------------------------*/
//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    //HERE WRITE CODE
    
    if(rfid_key!=""){
      get_date_time_timestamp();      
      Serial.println("New Data!"); 
      data_update_flag=1;  
      if(data_update_flag==1){
       send_data(rfid_key);  
      }
    }
    else
    {
      data_update_flag=0;
      Serial.println("No New Data!"); 
      }
delay(100);
   }
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

  if (rfid.uid.uidByte[0] != nuidPICC[0] || 
    rfid.uid.uidByte[1] != nuidPICC[1] || 
    rfid.uid.uidByte[2] != nuidPICC[2] || 
    rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));
    
    if(rfid_key=="")    
      {
            // Store NUID into nuidPICC array
      for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i]; 
      }
        for (byte i = 0; i <rfid.uid.size; i++) {
        rfid_key += String(rfid.uid.uidByte[i]);    
    }
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.println(rfid_key);
      }   
    }
  else{ Serial.println(F("Card read previously."));}

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