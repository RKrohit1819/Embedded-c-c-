#include <ModbusMaster.h>

// Define the Modbus slave ID and the baud rate
#define SLAVE_ID 1
#define BAUD_RATE 9600

// Define the coil address and the number of coils to read
#define COIL_ADDRESS 0
#define NUM_COILS 16

// Create a ModbusMaster object
ModbusMaster modbus;

void setup() {
  // Initialize the serial port
  Serial.begin(9600);

  // Initialize the Modbus communication
  modbus.begin(SLAVE_ID, Serial);
  modbus.setBaudRate(BAUD_RATE);
}

void loop() {
  // Read the coils and store the results in an array
  uint8_t coils[NUM_COILS];
  modbus.readCoils(COIL_ADDRESS, NUM_COILS, coils);

  // Process the coil values as needed
  for (int i = 0; i < NUM_COILS; i++) {
    Serial.print("Coil ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(coils[i]);
  }

  // Delay before reading again
  delay(1000);
}

