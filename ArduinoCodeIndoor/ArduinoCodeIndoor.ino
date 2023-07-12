#include <SoftwareSerial.h>
#include <DHT.h> 

// Serial Communication --------------------------------------------------------------
const byte rxPin = 10;
const byte txPin = 11;
SoftwareSerial Arduino_Serial (rxPin, txPin);

void sendSerialData(const char *message){
  Serial.println(message);
  Arduino_Serial.print(message);
}

// -----------------------------------------------------------------------------------
void setup() {
  Serial.begin(57600);
  Arduino_Serial.begin(9600);

  Serial.println("<Arduino is ready>");

  sendSerialData("<Mensaje desde arduino>");
}

void loop() {
}