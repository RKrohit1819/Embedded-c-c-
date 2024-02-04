#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

#define PIRPIN 13 
#define LDRPIN 34
int pirReading = 0;
int ldrReading = 0;
int data_update_flag;
String serverURL="http://thirdumpire-baseball.com/iot/api/sensor.php";
#define device_id "12345"
#define device_idPIR "123456"
#define sensorone "LDR"
#define sensortwo "PIR"

//---------------------WiFi Creadientials-----------------------//
const char*  ssid = "edge 30";
const char*  password = "01234567";
IPAddress    local_IP(192, 168, 2, 11);
IPAddress    gateway(192, 168, 3, 1);
IPAddress    subnet(255, 255, 255, 0);
IPAddress    primaryDNS(192, 168, 3, 7);

//---------------------- WiFi Function----------------------------//
// void wifi_connect()
// {
//   if(WiFi.status() != WL_CONNECTED) {
//     if (!WiFi.config(local_IP, gateway, subnet, primaryDNS)) {
//     Serial.println("STA Failed to configure");
//   }
//   WiFi.begin(ssid, password);
//   Serial.println("Connecting");
//   delay(500);
//   Serial.print(".");
//   }
// Serial.print("Connected to WiFi With IP Address:");
// Serial.println(WiFi.localIP());
// }
void wifi_connect() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println();
  }

  Serial.print("Connected to WiFi with IP Address: ");
  Serial.println(WiFi.localIP());
}

void readPIR(){
  delay(1000);
   pirReading = digitalRead(PIRPIN); 
  if (pirReading == HIGH) {           
   
    Serial.println("Motion detected");
  }
  else {
    Serial.println("Motion not detected");
  }
}

void readLDR(){
  delay(1000);
  ldrReading = analogRead(LDRPIN);
  ldrReading = map(ldrReading, 0, 1024, 0, 100);
  Serial.print("LDR Value: ");
  Serial.println(ldrReading);
}

/* -----------------------------HTTP Request---------------------------------------*/
void uploadLDRReading(){
  HTTPClient http;
  String dataURL="";
  dataURL=serverURL+"?"+"device_id="+(String)device_id +"&sensor_type="+(String)sensorone+"&ldr="+(String)ldrReading;
  Serial.println(dataURL);
  http.begin(dataURL);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println(payload);
    
  } else {
    data_update_flag=0;
    Serial.print("HTTP GET request failed with error code ");
    Serial.println(httpCode);
  }
  http.end();
}

void uploadPIRReading(){
  HTTPClient http;
  String dataURL="";
  dataURL=serverURL+"?"+"device_id="+(String)device_idPIR +"&sensor_type="+(String)sensortwo+"&pir="+(String)pirReading;
  Serial.println(dataURL);
  http.begin(dataURL);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println(payload);
    
  } else {
    data_update_flag=0;
    Serial.print("HTTP GET request failed with error code ");
    Serial.println(httpCode);
  }
  http.end();
}



void setup() {
  Serial.begin(115200);    
  pinMode(PIRPIN, INPUT);
  wifi_connect(); 
  
}
void loop() {
wifi_connect();
  readLDR();
  readPIR();
  delay(2000);
  uploadLDRReading();
  delay(2000);
  uploadPIRReading();
 
}
