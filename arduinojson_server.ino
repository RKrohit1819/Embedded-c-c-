//Header File
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// WiFi Creadientials
const char* ssid = "PLANT_BK";
const char* password = "Best@#123#";
IPAddress local_IP(192, 168, 2, 11);
IPAddress gateway(192, 168, 3, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(192, 168, 3, 7);

//Domain name with URL path or IP address with path
//const char* serverUrl = "http://192.168.3.19/eol/module/autoprn/API/api/dataview.php?device_id=Device_12345";
const char* serverUrl = "http://192.168.3.88/api/api/dataview.php?device_id=Device_123456";

//WiFi Function
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

//Function to get Data from server and Store the value in Saparate variable
void getData() {
  HTTPClient http;
  http.begin(serverUrl);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println(payload);
    StaticJsonDocument<90> doc;  //Capacity of json Documents in byte
    DeserializationError error = deserializeJson(doc, payload);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
    } else {
      int ok = doc[0]["ok"];
      int ng = doc[0]["ng"];
      int by_pass = doc[0]["by_pass"];
     Serial.print("Received ok: ");
     Serial.println(ok);
     Serial.print("Received ng: ");
     Serial.println(ng);
     Serial.print("Received by_pass: ");
     Serial.println(by_pass);
    }
  } else {
    Serial.print("HTTP GET request failed with error code ");
    Serial.println(httpCode);
  }
  http.end();
}

void setup(){
  Serial.begin(115200);
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
  getData();
     

}
void loop() {
  //Nothing in Loop
}