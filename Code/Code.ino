/*********
AUTOPRN V2 FOR ESP32 
*********/
/* -----------------------------Pre-Processor Directives---------------------------------------*/
//WIFI, Arduino JSON & HTML Client HEADER FILE
#include <string.h>
#include<stdlib.h>
#include<rom/rtc.h> 
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
//OTA UPDATE HEADER FILE
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
//MEMORY CARD PRE-PROCESSOR DIRECTIVE
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <ESP32_FTPClient.h>
/* -----------------------------NTP Library---------------------------------------*/
#include "time.h"
time_t start_time, end_time,up_start_time,up_end_time;
double diff_time,up_diff_time;
/* -----------------------------RTC Library---------------------------------------*/
#include <DS323x.h>
DS323x rtc;
//SPI PORTS FOR MEMORY CARD READER
#define SD_CS         13
#define SPI_MOSI      15
#define SPI_MISO      2
#define SPI_SCK       14
#define MAC_ID "Device_123456"
/* -----------------------------Logic Variables---------------------------------------*/
//Global Variable for IO Logic
unsigned long time_countdown = 0;
long idle_time=0;
long cycle_time=0;
int prod_start_flag,prod_stop_flag,data_update_flag;
long counter,ok,ng,by_pass;
long prev_ok,prev_ng,prev_by_pass;
int rtc_update_flag;
String c_date,c_time,c_timestamp,p_time_stamp;
/* -----------------------------IO Variables--------------------------------------*/
int ip1,ip2,ip3,ip4;
int op1,op2;
/* -----------------------------WiFi Credentials---------------------------------------*/
//WIFI CREDENTIAL & IP Configuartions
//const char* ssid = "BK_SHOPFLOOR";
//const char* password = "koki@best#";
const char* ssid = "PLANT_BK";
const char* password = "Best@#123#";
//const char* ssid = "WIN";
//const char* password = "00001111";
// Set your Static IP address
IPAddress local_IP(192, 168, 2, 10);
// Set your Gateway IP address
IPAddress gateway(192, 168, 2, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 3, 7);   //optional
//IPAddress secondaryDNS(8, 8, 4, 4);  //optional

char ftp_server[] = "192.168.3.19";
char ftp_user[]   = "BESTKOKI/EOL";
char ftp_pass[]   = "koki@1234";
// you can pass a FTP timeout and debbug mode on the last 2 arguments
ESP32_FTPClient ftp (ftp_server,ftp_user,ftp_pass, 5000, 2);
/* -----------------------------JSON Variable---------------------------------------*/
// JSON 
String jsonBuffer;
int httpResponseCode;
/* -----------------------------Timer Delays for API---------------------------------------*/
// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
//unsigned long lastTime = 0;
// Set timer to 10 seconds (10000)
//unsigned long timerDelay = 10000;

/* -----------------------------Dual Core Task Handler Variabale---------------------------------------*/
TaskHandle_t Task1;
TaskHandle_t Task2;

/* -----------------------------START Web Server for ELEGANT OTA ---------------------------------------*/
//Async Server Object
AsyncWebServer server(80);
/* -----------------------------CREATE INSTANCE OF DS3231 RTC Module & UPDATE FLAGS---------------------------------------*/

/* -----------------------------Set NTP Server Paramters & Define Local Time Function---------------------------------------*/
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec =19800;
const int   daylightOffset_sec = 0;
struct tm timeinfo;
/* -----------------------------File Operations---------------------------------------*/
/*FILE FILE OPERATION FUNCTION DEFINITION CODE STARTS FROM HERE*/
void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);
    int file_count=0;
    File root = fs.open(dirname);
    if(!root){
        //Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        //Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            //Serial.print("  DIR : ");
            //Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
              file_count++;
            //Serial.print("  FILE: ");
            //Serial.print(file.name());
            //Serial.print("  SIZE: ");
            //Serial.println(file.size());
        }
        file = root.openNextFile();
    }
    if(file_count>=90)
    {
      delete_SD_data_files();
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

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}

int deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
        return 1;
    } else {
        Serial.println("Delete failed");
        return -1;
    }
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


//Read data from memory card current data and set DATE,TIME,OK,NG,BYPASS,TOC etc.
void update_data_from_mmc(fs::FS &fs, const char * path)
{
   char *pch;
   String first_line;
   int stc=0;
   File file = fs.open(path);
   if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }
  while (file.available()) {
    first_line = file.readStringUntil('\n');
    Serial.print(first_line);
    break;
  }
 file.close();
 pch = strtok(&first_line[0],",");
 stc++;
 while (pch != NULL)
  {
    pch = strtok (NULL, ",");
    stc++;
    if(stc==3)
    {
      Serial.println (pch);
      ok=atoi(pch);
    }
    else if(stc==4)
    {
      Serial.println (pch);
      ng=atoi(pch);
    }
    else if(stc==5)
    {
      Serial.println (pch);
      by_pass=atoi(pch);
    }
    else if(stc==8)
    {
      Serial.println (pch);
      //p_time_stamp=pch;
    }
    else{
    Serial.println ("No need!");
    }
  }
 /*pch = strtok(NULL, ",");
 pch = strtok(NULL, ",");
 Serial.println(pch);
 //ok=(long)atoi(pch);
 pch = strtok(NULL, ",");
 Serial.println(pch);
 //ng=(long)atoi(pch);
 pch = strtok(NULL, ",");
 Serial.println(pch);
 //by_pass=(long)atoi(pch);
 pch = strtok(NULL, ",");
 pch = strtok(NULL, ",");
 pch = strtok(NULL, ",");
 //Serial.println(pch);
 //p_time_stamp=pch;*/

}


// ADD data to hisotrian log in memory card for each Part
void append_data_historian(long ok1,long ng1, long by_pass1,String date1,String time1, String time_stamp1,int cycle_time1, int idle_time1,int sync1)
{
String data_string;
String file_path;
file_path="/historian_data_"+c_date+".csv";
//Serial.println(file_path);
data_string=date1+","+time1+","+(String)ok1+","+(String)ng1+","+(String)by_pass1+","+(String)cycle_time1+","+(String)idle_time1+","+time_stamp1+","+(String)sync1+"\n";
//Serial.println(data_string);
appendFile(SD,&file_path[0],&data_string[0]);  
}

//Add data to production log datewise
void add_data_to_production_log(long ok1,long ng1, long by_pass1,String date1,String time1, String time_stamp1,int cycle_time1, int idle_time1,int sync1)
{
String data_string;
String file_path;
file_path="/production_data_"+c_date+".csv";
//Serial.println(file_path);
data_string=date1+","+time1+","+(String)ok1+","+(String)ng1+","+(String)by_pass1+","+(String)cycle_time1+","+(String)idle_time1+","+time_stamp1+","+(String)sync1+"\n";
//Serial.println(data_string);
writeFile(SD,&file_path[0],&data_string[0]);  
}

//Update Current Data for each production

/*
void update_data_current_data(long ok1,long ng1, long by_pass1,String date1,String time1, String time_stamp1,int cycle_time1, int idle_time1,int sync1)
{

String data_string;
String file_path;
file_path="/historian_data_"+c_date+".csv";
//Serial.println(file_path);
data_string=date1+","+time1+","+(String)ok1+","+(String)ng1+","+(String)by_pass1+","+(String)cycle_time1+","+(String)idle_time1+","+time_stamp1+","+(String)sync1+"\n";
//Serial.println(data_string);
appendFile(SD,&file_path[0],&data_string[0]);  
}
*/
/*FILE FILE OPERATION FUNCTION DEFINITION CODE ENDS HERE*/

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
//Serial.println(c_date);
//Serial.println(c_time);
//Serial.println(c_timestamp);
delay(100);
}
/* -----------------------------WIFI RECONNECT SET UP FUCNTION FOR ESP32---------------------------------------*/
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
  }
  http.end();
  return payload;
}
void edge_data_sync_req(long ok_1,long ng_1,long by_pass_1)
{
   time(&end_time);
   int sync1;
   long ok1,ng1,by_pass1;
   ok1=ok_1;
   ng1=ng_1; 
   by_pass1=by_pass_1;
  diff_time = difftime(end_time, start_time);+
  String prev_date="";
  prev_date=c_date;
  get_date_time_timestamp();
  if(strcmp(&prev_date[0],&c_date[0])!=0)
  {
   Serial.println("DATE DID NOT MATCHED!");
   //This will Execute when Date is not same or New Day started
   ok1=ok1-prev_ok;
   ng1=ng1-prev_ng; 
   by_pass1=by_pass1-prev_by_pass;
   prev_date=c_date;
   ok=ok1;
   ng=ng1;
   by_pass=by_pass1;
  }
  else
  {
     //Serial.println("DATE MATCHED!");
     //This will Execute when Date is same but value resets...
     if(prev_ok>ok1)
     {
        String file_path2;
        file_path2="/production_data_"+c_date+".csv";
        if(SD.exists(&file_path2[0]))
        {
          update_data_from_mmc(SD,&file_path2[0]);
          ok1=ok + ok_1;
          ng1=ng + ng_1;
          by_pass1=by_pass +  by_pass_1;
          ok=ok1;
          ng=ng1;
          by_pass=by_pass1;  
        }
     else
      {
          Serial.println("File does not Exist and Failed to open file for reading!");      
          ok1=prev_ok + ok1;
          ng1=prev_ng + ng1;
          by_pass1=prev_by_pass +  by_pass1;
          ok=ok1;
          ng=ng1;
          by_pass=by_pass1; 
      }
     }
  }
  if( data_update_flag==1){
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://192.168.3.19/eol/module/autoprn/API/api/data.php?ok=" + (String)ok1 +"&ng=" + (String)ng1+ "&by_pass=" + (String)by_pass1+ "&mac_id=" + (String)MAC_ID+"&actual_cycle_time="+(String)diff_time+"&device_date="+(String)c_date+"&device_time="+(String)c_time;
      //Serial.println(serverPath);
      jsonBuffer = httpGETRequest(serverPath.c_str());
      //Serial.println(jsonBuffer);
    }
     else {
      Serial.println("NETWORK FAILURE!");
    }
    /* if(WiFi.status()== WL_CONNECTED){
      String serverPath2 = "http://192.168.3.19/eol/module/autoprn/API/api/time_data.php?cycle_time=" + (String)diff_time +"&idle_time=" + (String)idle_time+ "&mac_id=" + (String)MAC_ID;
      Serial.println(serverPath2);
      jsonBuffer = httpGETRequest(serverPath2.c_str());
      Serial.println(jsonBuffer);
    }
    else {
      Serial.println("NO Data Update Command at this time....");
    }*/
   data_update_flag=0;
   prev_ok=ok1;
   prev_ng=ng1;
   prev_by_pass=by_pass1;
  }
  else{
     Serial.println("Data Update Flag is Off at this time...");
  }
  if(httpResponseCode==200){sync1=1;} 
   else{sync1=0;}
  //db_sync(ok, ng,by_pass,diff_time,sync1);
  append_data_historian(ok1,ng1,by_pass1,c_date,c_time,c_timestamp,diff_time,100,sync1);
  add_data_to_production_log(ok1,ng1,by_pass1,c_date,c_time,c_timestamp,diff_time,100,sync1);
   }

 
 int ftp_data_file_upload(fs::FS &fs,String input_date)
 {
  // Create the file new and write a string into it
  String data_file,historian_file,device_data_file;
  char lineBuffer[64];
  data_file="/production_data_"+input_date+".csv";
  historian_file="/historian_data_"+input_date+".csv";
  device_data_file="/device_data_"+input_date+".csv";
  File file = fs.open(historian_file);
  if(!file || file.isDirectory()){
       Serial.println("− failed to open file for reading");
       return -1;
   }
   else
   {
  ftp.OpenConnection();
  ftp.InitFile("Type A");
  ftp.NewFile(&historian_file[0]);
  while (file.available()) {
  int length = file.readBytesUntil('\n', lineBuffer, sizeof(lineBuffer) - 1);
  if (length > 0 && lineBuffer[length - 1] == '\r') {
    length--; // to remove \r if it is there
  }
  lineBuffer[length] = 0; // terminate the string
  //Serial.println();
  ftp.Write(lineBuffer);
  ftp.Write("\n");
}

  ftp.CloseFile();
  ftp.CloseConnection();
 }
  file = fs.open(data_file);
  if(!file || file.isDirectory()){
       Serial.println("− failed to open file for reading");
       return -1;
   }
   else
   {
  ftp.OpenConnection();
  ftp.InitFile("Type A");
  ftp.NewFile(&data_file[0]);
  while (file.available()) {
  int length = file.readBytesUntil('\n', lineBuffer, sizeof(lineBuffer) - 1);
  if (length > 0 && lineBuffer[length - 1] == '\r') {
    length--; // to remove \r if it is there
  }
  lineBuffer[length] = 0; // terminate the string
  //Serial.println();
  ftp.Write(lineBuffer);
  ftp.Write("\n");
}

  ftp.CloseFile();
  ftp.CloseConnection();
 }
  file = fs.open(device_data_file);
  if(!file || file.isDirectory()){
       Serial.println("− failed to open file for reading");
       return -1;
   }
   else
   {
  ftp.OpenConnection();
  ftp.InitFile("Type A");
  ftp.NewFile(&device_data_file[0]);
  while (file.available()) {
  int length = file.readBytesUntil('\n', lineBuffer, sizeof(lineBuffer) - 1);
  if (length > 0 && lineBuffer[length - 1] == '\r') {
    length--; // to remove \r if it is there
  }
  lineBuffer[length] = 0; // terminate the string
  //Serial.println();
  ftp.Write(lineBuffer);
  ftp.Write("\n");
}

  ftp.CloseFile();
  ftp.CloseConnection();
 }
 return 1;
 }
 //Device Boot Info to Server and Memory Card
void device_started(int code1,int code2)
{
  String data_string;
  String file_path;
 if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://192.168.3.19/eol/module/autoprn/API/api/device_boot.php?mac_id=" +(String)MAC_ID+"&start_type=SYSTEM_START_C1:"+(String)code1+"_C2:"+(String)code2+"&device_date="+(String)c_date+"&device_time="+(String)c_time;
      //Serial.println(serverPath);
      jsonBuffer = httpGETRequest(serverPath.c_str());
      //Serial.println(jsonBuffer);
    }
     else {
      Serial.println("WiFi Disconnected");
    }
  file_path="/device_data_"+c_date+".csv";
  //Serial.println(file_path);
  data_string=(String)c_date+","+(String)c_time+",SYSTEM STARTED!"+"\n";
  //Serial.println(data_string);
  appendFile(SD,&file_path[0],&data_string[0]);  
}
//Function to delete_data_file when file count >= 90
void delete_SD_data_files()
  {
    int month1,date1;
    int complete = 0;
    String month2,date2,file_date,file_name;
    month1=rtc.month();
    date1=rtc.day();
    date1=date1-30;
    if(date1<0){
      date1=30+date1;
      month1=month1-1;
      }
    for( int i=1;i<=15;i++){
       date1=date1+1;
       if(date1>31){
          date1=1;
          month1=month1+1;
      }
        if(month1<10){
          month2="0"+(String)month1;
        }
        else{
          month2=rtc.month();
        }
        if(date1<10){
          date2="0"+(String)date1;
        }
        else{
          date2=rtc.day();
         }
      file_date="20"+(String)rtc.year()+"-"+(String)month2+"-"+(String)date2;
      String data_file1="/production_data_"+file_date+".csv";
      String historian_file1="/historian_data_"+file_date+".csv";
      String device_data_file1="/device_data_"+file_date+".csv";
      complete+=deleteFile(SD,&data_file1[0]);
      complete+=deleteFile(SD,&historian_file1[0]);
      complete+=deleteFile(SD,&device_data_file1[0]);
      if(complete<2)
      {
        i-=1;
        complete=0;
        if(i<=0)
        {
          i=1;
        }
      }
        }
  }
 void machine_operation()
 {
  int temp=0;
  temp=hallRead();
  long total_memory,used_memory;
  total_memory=SD.totalBytes() / (1024 * 1024);
  used_memory=SD.usedBytes() / (1024 * 1024);
  String memory=(String)total_memory+"-"+(String)used_memory;
  if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://192.168.3.19/eol/module/autoprn/API/api/machine.php?mac_id=" +(String)MAC_ID+"&disk="+memory+"&temp="+(String)temp+"&memory=340kb"+"&device_date="+(String)c_date+"&device_time="+(String)c_time+"&op=device_health_status"+"&status=1";
      //Serial.println(serverPath);
      jsonBuffer = httpGETRequest(serverPath.c_str());
      //Serial.println(jsonBuffer);
    }
     else {
      Serial.println("WiFi Disconnected");
    }
if(strcmp(&jsonBuffer[0],"1111")==0)
{
  digitalWrite(0,LOW);
  digitalWrite(4,LOW);
  Serial.println("MACHINE ON COMMAND!");
  String serverPath = "http://192.168.3.19/eol/module/autoprn/API/api/machine.php?mac_id=" + (String)MAC_ID+"&op=machine_ON_command&status=1";
  //Serial.println(serverPath);
  jsonBuffer = httpGETRequest(serverPath.c_str());
}
else if(strcmp(&jsonBuffer[0],"0000")==0)
{
  digitalWrite(0,HIGH);
  digitalWrite(4,HIGH);
  Serial.println("MACHINE OFF COMMAND!");
  String serverPath = "http://192.168.3.19/eol/module/autoprn/API/api/machine.php?mac_id=" + (String)MAC_ID+"&op=machine_OFF_command&status=1";
  //Serial.println(serverPath);
  jsonBuffer = httpGETRequest(serverPath.c_str());
}
else if(strcmp(&jsonBuffer[0],"2222")==0)
{
  if(WiFi.status()== WL_CONNECTED){
      String ftp_data_date;
      String serverPath2 = "http://192.168.3.19/eol/module/autoprn/API/api/ftp_data.php?mac_id="+(String)MAC_ID+"&op=ftp_file_upload&status=init";
      //Serial.println(serverPath2);
      ftp_data_date = httpGETRequest(serverPath2.c_str());
      //Serial.println(ftp_data_date);
      if(strcmp(&ftp_data_date[0],"0000")!=0){
      int complete=0;
      complete=ftp_data_file_upload(SD,&ftp_data_date[0]);
      if(complete==1)
      {
      serverPath2 = "http://192.168.3.19/eol/module/autoprn/API/api/ftp_data.php?mac_id="+(String)MAC_ID+"&op=ftp_file_upload&status=1";
      //Serial.println(serverPath2);
      ftp_data_date = httpGETRequest(serverPath2.c_str());
      }
      else if(complete==-1)
      {
      serverPath2 = "http://192.168.3.19/eol/module/autoprn/API/api/ftp_data.php?mac_id=" +(String)MAC_ID+"&op=ftp_file_upload&status=-1";
      //Serial.println(serverPath2);
      ftp_data_date = httpGETRequest(serverPath2.c_str());
      }
      else
      {
      serverPath2 = "http://192.168.3.19/eol/module/autoprn/API/api/ftp_data.php?mac_id=" +(String)MAC_ID+"&op=ftp_file_upload&status=0";
      Serial.println(serverPath2);
      ftp_data_date = httpGETRequest(serverPath2.c_str());
      }
      }
      }
}
else if(strcmp(&jsonBuffer[0],"3333")==0)
{
  up_diff_time=0;
  ESP.restart();
  jsonBuffer="";
}
else if(strcmp(&jsonBuffer[0],"4444")==0)
{
      String ftp_data_date;
      String serverPath2 = "http://192.168.3.19/eol/module/autoprn/API/api/ftp_data.php?mac_id=" +(String)MAC_ID+"&op=file_delete&status=init";
      //Serial.println(serverPath2);
      ftp_data_date = httpGETRequest(serverPath2.c_str());
      //Serial.println(ftp_data_date);
      if(strcmp(&ftp_data_date[0],"0000")!=0){
      int complete=0;
      String data_file1="/production_data_"+ftp_data_date+".csv";
      String historian_file1="/historian_data_"+ftp_data_date+".csv";
      String device_data_file1="/device_data_"+ftp_data_date+".csv";
      complete+=deleteFile(SD,&data_file1[0]);
      complete+=deleteFile(SD,&historian_file1[0]);
      complete+=deleteFile(SD,&device_data_file1[0]);
      if(complete==3)
        {
        serverPath2 = "http://192.168.3.19/eol/module/autoprn/API/api/ftp_data.php?mac_id=" +(String)MAC_ID+"&op=file_delete&status=3";
        Serial.println(serverPath2);
        ftp_data_date = httpGETRequest(serverPath2.c_str());
        complete=0;
        }
      else if(complete==2)
        {
        serverPath2 = "http://192.168.3.19/eol/module/autoprn/API/api/ftp_data.php?mac_id=" +(String)MAC_ID+"&op=file_delete&status=2";
        Serial.println(serverPath2);
        ftp_data_date = httpGETRequest(serverPath2.c_str());
        complete=0;
        }
        else if(complete==1)
        {
        serverPath2 = "http://192.168.3.19/eol/module/autoprn/API/api/ftp_data.php?mac_id=" +(String)MAC_ID+"&op=file_delete&status=1";
        //Serial.println(serverPath2);
        ftp_data_date = httpGETRequest(serverPath2.c_str());
        complete=0;
        }
      else
        {
        serverPath2 = "http://192.168.3.19/eol/module/autoprn/API/api/ftp_data.php?mac_id=" +(String)MAC_ID+"&op=file_delete&status=-1";
        //Serial.println(serverPath2);
        ftp_data_date = httpGETRequest(serverPath2.c_str());
        complete=0;
        }
      }
}
else
{
  Serial.println("NO COMMANDS YET!");
}
time(&up_start_time);
 
 }

/* -----------------------------SET UP FUCNTION FOR ESP32---------------------------------------*/

void setup() {
  Serial.begin(115200); //START SERIAL PORT WITH BAUD RATE 115200 bps
  //GET RESET CODE
  int rst_code_core1=rtc_get_reset_reason(0);
  int rst_code_core2=rtc_get_reset_reason(1);
  //START SPI
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI, SD_CS);  //START SPI for Memory Card Reader
  delay(2000); // Request the time correction on the Serial
  Wire.begin();  //START I2C FOR TRC Module
  delay(2000); // Request the time correction on the Serial
  if(!SD.begin(SD_CS, SPI)){ //SD Card open and Mounting Check
        Serial.println("Card Mount Failed");
        //return;
    }
  uint8_t cardType = SD.cardType(); //Get SD card type 
  if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        //return;
    }
 delay(2000);
/* -----------------------------IO Declaration---------------------------------------*/
  pinMode(5, INPUT);
  pinMode(18, INPUT);
  pinMode(19, INPUT);
  pinMode(17, INPUT);
  pinMode(0, OUTPUT);//For Relay 1
  pinMode(4, OUTPUT);//For Relay 2
  //OFF THE REALY FROM BOOT
  digitalWrite(0,HIGH);
  digitalWrite(4,HIGH);
/* -----------------------------Connect to WiFi---------------------------------------*/ 
if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
    Serial.println("STA Failed to configure");
  }
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  int con_count=0;
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    con_count+=1;
    if(con_count>10)
    {
      break;
    }
  }
  con_count=0;
  //Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
/* -----------------------------SET & UPDATE RTC Date Time from NTP Server---------------------------------------*/
rtc.attach(Wire);
printLocalTime(); 
if(rtc_update_flag==1)//UPDATE RTC IF NTP Server Returns OK Flag
{
int year1=timeinfo.tm_year+1900;
int month1=timeinfo.tm_mon+1;
rtc.now(DateTime(year1, month1, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec)); 
}
get_date_time_timestamp();
/* -----------------------------Read from Memory Card and Update Data---------------------------------------*/
String file_path1;
file_path1="/production_data_"+c_date+".csv";
update_data_from_mmc(SD,&file_path1[0]);
 Serial.println("OK=");
 Serial.print(ok);
 Serial.println("NG=");
 Serial.print(ng);
 Serial.println("by_pass=");
 Serial.print(by_pass);
//ftp_data_file_upload(SD,"2022-6-18");
delay(100);
/* -----------------------------OTA SERVER START---------------------------------------*/  
//OTA SERVER Init
 server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "AutoPRN Firmware Version 2.1, BEST KOKI AUTOMOTIVE PVT. LTD., IMS");
  });
  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");
  //Start Time to Count Cycle Time & Idle Time
  time(&start_time);
  time(&up_start_time);
  up_end_time=up_start_time;
  device_started(rst_code_core1,rst_code_core2);
  listDir(SD, "/", 0);
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
    time(&up_end_time);
    up_diff_time = difftime(up_end_time, up_start_time);
    if(up_diff_time>=50)
    {
      wifi_reconnect();
      machine_operation();
      up_diff_time=0;
      Serial.println("Interval Reached! Task Done!");
    }
   else
    {
      
     //Serial.print("Waiting for Interval to Start. (Interval>250:True?Flase) Interval=:");
     Serial.println(up_diff_time);
    }

  }
  delay(1000);
}
/* -----------------------------Task 2 Definition---------------------------------------*/
//Task2code: blinks an LED every 700 ms
void Task2code( void * pvParameters ){
  Serial.print("Task2 running on core ");
  Serial.println(xPortGetCoreID());

  for(;;){
    //HERE WRITE CODE
    if(data_update_flag==1)
    {
      Serial.println("New Data!"); 
       edge_data_sync_req(ok,ng,by_pass);
       time(&start_time);
       data_update_flag=0;
    }
    else
    {
      Serial.println("No New Data!"); 
    }
/*
Serial.print("IP1=");
Serial.println(ip1);
Serial.print("IP2=");
Serial.println(ip2);
Serial.print("IP3=");
Serial.println(ip3);
Serial.print("IP4=");
Serial.println(ip4);*/
delay(100);
   }
}

/* -----------------------------Main Loop---------------------------------------*/
void loop() {
      //Read All 4 Digital IOs
      ip1=digitalRead(23);    // For Start Button
      ip2=digitalRead(5);    // For OK
      ip3=digitalRead(18);  //For Value Reset Signal
      ip4=digitalRead(19); //For NG
/* -----------------------------Signal Data Logic Starts From Here---------------------------------------*/
      if(ip1==HIGH)
      {
        if(prod_start_flag==0){
            //idle_time=millis()-time_countdown;
            //idle_time=idle_time/1000;
            //time_countdown=0;
            //time_countdown=millis();
            prod_start_flag=1;
        }
      
      }
      else
      {
      
        if(prod_start_flag==1)
        {
        prod_start_flag=0;
        prod_stop_flag=1; 
        }
      }
 if(prod_stop_flag==1)
 { 
  //cycle_time=millis()-time_countdown;
  //cycle_time=cycle_time/1000;
  prod_stop_flag=0;
  time_countdown=0;
  
 }

    if(ip2==1 && ip4==0)
    {
      ok++;
      data_update_flag=1;
      delay(2000);
      ip2=0;
    }
   else if(ip2==0 && ip4==1)
    {
      ng++;
      data_update_flag=1;
      delay(2000);
      ip4=0;
    }
    else if(ip4==1 && ip2==1)
    {
      by_pass++;
      delay(2000);
      data_update_flag=1;
    }
    else{
      Serial.print("Machine is in IDLE Condtion!");
    }
//if(prod_stop_flag==0 && prod_start_flag==0 && time_countdown<=0){
//time_countdown=0;
//time_countdown=millis();  
//}
if(ip3==1)
{
ok=0;
ng=0;
by_pass=0;
}
/*
Serial.print("Start_Flag=");
Serial.println(prod_start_flag);
Serial.print("Stop_Flag=");
Serial.println(prod_stop_flag);
Serial.print("Time Count=");
Serial.println(time_countdown);
Serial.print("Cycle Time=");
Serial.println(cycle_time);
Serial.print("Idle Time=");
Serial.println(idle_time);
Serial.print("IP1=");
Serial.println(ip1);
Serial.print("IP2=");
Serial.println(ip2);
Serial.print("IP3=");
Serial.println(ip3);
Serial.print("IP4=");
Serial.println(ip4);
Serial.print("OK=");
Serial.println(ok);
Serial.print("NG=");
Serial.println(ng);   
Serial.print("TOC=");
Serial.println(by_pass);
Serial.printf("Current counter value: %ld\n", counter);
*/
/* -----------------------------Signal Data Logic Ends Here---------------------------------------*/
 delay(10);
}

