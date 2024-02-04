#include <WiFi.h>
#include "time.h"
#include<rom/rtc.h>
#include <DS323x.h>
DS323x rtc;

const char* ssid     = "edge 30";
const char* password = "01234567";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;
struct tm timeinfo;
int rtc_update_flag;
String c_date,c_time,c_timestamp,p_time_stamp;
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

  // Connect to Wi-Fi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  
  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop(){
  delay(20000);
  printLocalTime();
}
