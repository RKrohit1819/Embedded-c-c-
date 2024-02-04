#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#define RELAY_PIN1 13 
#define RELAY_PIN2 12


//---------------------WiFi Creadientials-----------------------//
const char*  ssid = "edge 30";
const char*  password = "01234567";
IPAddress    local_IP(192, 168, 2, 11);
IPAddress    gateway(192, 168, 3, 1);
IPAddress    subnet(255, 255, 255, 0);
IPAddress    primaryDNS(192, 168, 3, 7);

const char* serverUrl = "http://thirdumpire-baseball.com/iot/api/actuater.php?device_id=1001";

//---------------------- WiFi Function----------------------------//
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

//---------------------LIGHT AND FAN--------------------------//
void fanOff(){
  digitalWrite(RELAY_PIN1, HIGH);
  Serial.println("Fan is OFF");
}

void fanOn(){
  digitalWrite(RELAY_PIN1, LOW);
  Serial.println("Fan is ON");
}

void lightOff(){
  digitalWrite(RELAY_PIN2, HIGH);
  Serial.println("Light is OFF");
}

void lightOn(){
  digitalWrite(RELAY_PIN2, LOW);
  Serial.println("Light is ON");
}

//--------------------------------READ DATA FROM SERVER---------------------------//
void getData() {
  HTTPClient http;
  http.begin(serverUrl);
  int httpCode = http.GET();
  if (httpCode == HTTP_CODE_OK) {
    String payload = http.getString();
    Serial.println(payload);

    StaticJsonDocument<256> doc;  // Capacity of JSON document in bytes
    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.c_str());
    } else {
      String actuatorData = doc[0]["actuator_data"].as<String>();  // Retrieve as String
      StaticJsonDocument<64> actuatorDoc;
      DeserializationError actuatorError = deserializeJson(actuatorDoc, actuatorData);

      if (actuatorError) {
        Serial.print("actuatorData deserializeJson() failed: ");
        Serial.println(actuatorError.c_str());
      } else {
        int light = actuatorDoc["light"];
        int fan = actuatorDoc["fan"];

        Serial.print("Light: ");
        Serial.println(light);
        Serial.print("Fan: ");
        Serial.println(fan);

        if (fan == 1) {
          fanOn();
        } else {
          fanOff();
        }
        if (light == 1) {
          lightOn();
        } else {
          lightOff();
        }
      }
    }
  } else {
    Serial.print("HTTP GET request failed with error code: ");
    Serial.println(httpCode);
  }

  http.end();
}



void setup() {
  Serial.begin(115200); 
  pinMode(RELAY_PIN1, OUTPUT);
  pinMode(RELAY_PIN2, OUTPUT);
  wifi_connect();
  //getData();
  // fanOn();
  // lightOn();
}



void loop() {
  delay(5000);
  wifi_connect();
  getData();
}