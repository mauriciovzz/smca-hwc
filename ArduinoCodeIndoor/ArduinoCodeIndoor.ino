#include <SoftwareSerial.h>
#include <Adafruit_AHTX0.h>
#include <RTClib.h>

#define NODE_ID "I001"

// Serial Communication --------------------------------------------------------------
const byte rxPin = 10;
const byte txPin = 11;
SoftwareSerial Arduino_Serial (rxPin, txPin);

void sendSerialData(const char *type, float value){
  char finalBuffer[64] = ""; 
  
  strcat(finalBuffer,"<");

  strcat(finalBuffer,NODE_ID);  
  strcat(finalBuffer,",");

  char* dateTime = ds1307_reading();
  strcat(finalBuffer,dateTime);  
  strcat(finalBuffer,",");

  strcat(finalBuffer,type);  
  strcat(finalBuffer,",");

  static char strValue[7];
  dtostrf(value, 8, 2, strValue);
  strcat(finalBuffer,strValue);

  strcat(finalBuffer,">");

  Serial.println(finalBuffer);
  //Arduino_Serial.print(finalBuffer);
  delay(1000);
}

// DS1307 -----------------------------------------------------------------------------
RTC_DS1307 rtc;

const char* ds1307_reading(){  
  char buffer[] = "DD/MM/YY,hh:mm:ss";
  DateTime now = rtc.now();
  char *data = now.toString(buffer); 
  return data;
}

// DHT20 -----------------------------------------------------------------------------
Adafruit_AHTX0 aht;

void dht20_reading(){
  sensors_event_t hum, temp;
  aht.getEvent(&hum, &temp);

  sendSerialData("TEMP", temp.temperature);

  sendSerialData("HUM", hum.relative_humidity);
}

// GP2Y1010AU0F -----------------------------------------------------------------------------
#define DUST_PIN A0 
#define LED_PIN   7   

#define ANALOG_VOLTAGE 5.0 // analog top of range

void gp2y1010au0f_reading(){
  float output_voltage, dust_density;

  // power on the LED
  digitalWrite(LED_PIN, LOW); 
  // Wait 0.28ms according to DS                               
  delayMicroseconds(280);                                   
  // take analog reading
  output_voltage = analogRead(DUST_PIN); 
  delay(1);
  // turn the LED off
  digitalWrite(LED_PIN, HIGH); 

  output_voltage = (output_voltage / 1023) * ANALOG_VOLTAGE;
  dust_density = (0.18 * output_voltage) - 0.1;  

  sendSerialData("DUST", dust_density);
}

// -----------------------------------------------------------------------------------
void setup() {
  Serial.begin(57600);
  Arduino_Serial.begin(9600);

  // DHT20 ---
  aht.begin();

  // GP2Y1010AU0F ---
  pinMode(LED_PIN, OUTPUT);

  // DS1207 ---
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // or for January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  
  // start
  delay(1000);
  Serial.println("<Arduino is ready>");
}

void loop() {
  dht20_reading();
  gp2y1010au0f_reading();
}