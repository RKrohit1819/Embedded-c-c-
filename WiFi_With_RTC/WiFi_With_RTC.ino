/*-----------------HEADER FILE---------------*/
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
#define RST_PIN 0
#define RELAY_PIN 00 //change to correct pin no according to circuit
#define RELAY_PIN 00 //change to correct pin no according to Circuit

//WiFi Creadientials
const char* ssid = "PLANT_BK";
const char* password = "Best@#123#";
IPAddress local_IP(192, 168, 2, 11);
IPAddress gateway(192, 168, 3, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 3, 7);

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
c_date="20"+(String)rtc.year()+"-"+(String)month2+"-"+(String)date2;
c_time=(String)rtc.hour()+":"+(String)rtc.minute()+":"+(String)rtc.second();
c_timestamp=c_date+" "+c_time;
Serial.println(c_date);
Serial.println(c_time);
Serial.println(c_timestamp);
delay(100);
}

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
     
}
void loop() {
  //Nothing in Loop
}