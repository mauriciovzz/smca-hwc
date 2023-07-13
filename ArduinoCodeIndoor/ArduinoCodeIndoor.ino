#include <SoftwareSerial.h>
#include <Adafruit_AHTX0.h>

// Serial Communication --------------------------------------------------------------
const byte rxPin = 10;
const byte txPin = 11;
SoftwareSerial Arduino_Serial (rxPin, txPin);

void sendSerialData(const char *type, float value){
  char finalBuffer[32] = ""; 
  
  static char temporalValue[7];
  dtostrf(value, 6, 2, temporalValue);

  strcat(finalBuffer,"<");  
  strcat(finalBuffer,type);  
  strcat(finalBuffer,",");
  strcat(finalBuffer,temporalValue);
  strcat(finalBuffer,">");

  Serial.println(finalBuffer);
  Arduino_Serial.print(finalBuffer);
}

// DHT20 -----------------------------------------------------------------------------
Adafruit_AHTX0 aht;

void dht20_Reading(){
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  sendSerialData("temp",temp.temperature);
  sendSerialData("hum" ,humidity.relative_humidity);
  
  delay(1000); 
}

// -----------------------------------------------------------------------------------
void setup() {
  Serial.begin(57600);
  Arduino_Serial.begin(9600);

  aht.begin();

  Serial.println("<Arduino is ready>");
}

void loop() {
  dht20_Reading();
}