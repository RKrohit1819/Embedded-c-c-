void setup() {
  Serial.begin(9600);
  
  int myVar = 10;  // Initialize a variable.

  int num = 5;
  
  Serial.print("myVar's lvalue: ");
  Serial.println((int) &num, DEC);  // Grab myVar's lvalue
  Serial.print("myVar's rvalue: ");
  Serial.println(num, DEC);
  //Serial.println();
  
  //int *myPointer;   // Declare your pointer.
  //myPointer = &myVar; //Assign myVar's memory address to pointer.
  
  //Serial.print("myPointer's lvalue: ");
  //Serial.println((long) &myPointer, DEC);  //myPointer's lvalue
  //Serial.print("myPointer's rvalue: ");
  //Serial.println((long) myPointer, DEC);  //myPointer's rvalue
}

void loop() {
}
