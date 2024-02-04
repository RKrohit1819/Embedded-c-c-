#include <WiFi.h>
// #include <HTTPClient.h>
// #include <ArduinoJson.h>


//---------------------WiFi Creadientials-----------------------//
const char*  ssid = "edge 30";
const char*  password = "01234567";
IPAddress    local_IP(192, 168, 211, 11);
IPAddress    gateway(192, 168, 3, 1);
IPAddress    subnet(255, 255, 255, 0);
IPAddress    primaryDNS(192, 168, 3, 7);

//---------------------- WiFi Function----------------------------//
void wifi_connect()
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
Serial.print("Connected to WiFi With IP Address:");
Serial.println(WiFi.localIP());
}

void setup(){
   Serial.begin(115200);
   wifi_connect();


}

void loop(){

}