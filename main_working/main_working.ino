//Header File
#include <string.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <WiFi.h>
#include<rom/rtc.h>
#include "time.h"
#include <DS323x.h>
DS323x rtc;
//SPI PORTS FOR MEMORY CARD READER
#define SD_CS         13
#define SPI_MOSI      15
#define SPI_MISO      2
#define SPI_SCK       14

const char* ssid     = "PLANT_BK";
const char* password = "Best@#123#";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 19800;
const int   daylightOffset_sec = 0;
struct tm timeinfo;
int rtc_update_flag;
String c_date,c_time,c_timestamp,p_time_stamp,c_month;

String folderName;
String folder_path;
String fileName;
String file_path;
String data0 ="This is just for the purpose of testing. Hello I am Ashish Kumar working as a GET in Best Koki Automotive Pvt Ltd.";

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);
    int file_count=0;
    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : "); 
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
              file_count++;
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}    
    

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}


void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);
    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}
void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}
void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }

     file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
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
c_month = "20"+(String)rtc.year()+"-"+(String)month2;
Serial.println(c_date);
Serial.println(c_time);
Serial.println(c_timestamp);
Srrial.println(c_month);
delay(100);
}



void setup(){
  Serial.begin(115200);
  // Connect to Wifi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");

  //SET & UPDATE RTC Date Time from NTP Server
  rtc.attach(Wire);
  printLocalTime(); 
  if(rtc_update_flag==1)//UPDATE RTC IF NTP Server Returns OK Flag
{
  int year1=timeinfo.tm_year+1900;
  int month1=timeinfo.tm_mon+1;
  rtc.now(DateTime(year1, month1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec)); 
}
  get_date_time_timestamp();



  // SD card Mounting
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS);
  delay(2000);
  if(!SD.begin(SD_CS, SPI)){ //SD Card open and Mounting Check
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType(); //Get SD card type 
  if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }
    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }
    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);
    listDir(SD, "/", 0);
    
    folderName = "/IIOT"+c_date;
    createDir(SD, &folderName[0]);
    fileName = folderName+"/Best"+c_time+".csv";
    appendFile(SD,&fileName[0],&data0[0]);
    fileName = folderName+"/Koki"+c_time+".csv";
    appendFile(SD,&fileName[0],&data0[0]);
      
}
void loop(){
  //nothing
}