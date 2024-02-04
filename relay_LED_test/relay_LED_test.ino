#define RELAY_PIN1 0 
#define RELAY_PIN2 4
#define GREEN_LED 14
#define RED_LED 12

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
void processOff(){
  digitalWrite(RELAY_PIN1, HIGH);
  digitalWrite(RELAY_PIN2, HIGH);
  Serial.println("Process is OFF");

}
void processOn(){
  digitalWrite(RELAY_PIN1, LOW);
  digitalWrite(RELAY_PIN2, LOW);
  Serial.println("Process is ON");

}
void setup() {
  pinMode(RELAY_PIN1, OUTPUT);
  pinMode(RELAY_PIN2, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);

  processOn();
  greenLedOn();
  redLedOn();
}

void loop() {
           
}
