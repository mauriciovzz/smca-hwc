#include <SoftwareSerial.h>
#include <Adafruit_AHTX0.h>

// Serial Communication --------------------------------------------------------------
const byte rxPin = 10;
const byte txPin = 11;
SoftwareSerial Arduino_Serial (rxPin, txPin);

void sendSerialData(const char *type, char *topic, float value){
  char finalBuffer[32] = ""; 
  
  static char strValue[7];
  dtostrf(value, 6, 2, strValue);

  strcat(finalBuffer,"<");  
  strcat(finalBuffer,type);  
  strcat(finalBuffer,",");
  strcat(finalBuffer,topic); 
  strcat(finalBuffer,",");
  strcat(finalBuffer,strValue);
  strcat(finalBuffer,">");

  Serial.println(finalBuffer);
  Arduino_Serial.print(finalBuffer);
  delay(2000);
}

// DHT20 -----------------------------------------------------------------------------
const char *topicTemp = "indoor/00001/temp";
const char *topicHum  = "indoor/00001/hum";
Adafruit_AHTX0 aht;

void dht20_Reading(){
  sensors_event_t hum, temp;
  aht.getEvent(&hum, &temp);

  sendSerialData("temp", topicTemp, temp.temperature);

  sendSerialData("hum" , topicHum , hum.relative_humidity);
}

// GP2Y1010AU0F -----------------------------------------------------------------------------
const char *topicDust = "indoor/00001/dust";
#define dustPin A0 // white wire
#define ledPin 7   // green wire

#define ANALOG_VOLTAGE 5.0 // analog top of range

void gp2y1010au0f_Reading(){
  float output_voltage, dust_density;

  // power on the LED
  digitalWrite(ledPin, LOW); 
  // Wait 0.28ms according to DS                               
  delayMicroseconds(280);                                   
  // take analog reading
  output_voltage = analogRead(dustPin); 
  delay(1);

  // turn the LED off
  digitalWrite(ledPin, HIGH); 

  output_voltage = (output_voltage / 1023) * ANALOG_VOLTAGE;
  dust_density = (0.18 * output_voltage) - 0.1;  

  sendSerialData("dust", topicDust, dust_density);
}

// -----------------------------------------------------------------------------------
void setup() {
  Serial.begin(57600);
  Arduino_Serial.begin(9600);

  aht.begin();

  pinMode(ledPin, OUTPUT);

  Serial.println("<Arduino is ready>");
}

void loop() {
  dht20_Reading();
  gp2y1010au0f_Reading();
}