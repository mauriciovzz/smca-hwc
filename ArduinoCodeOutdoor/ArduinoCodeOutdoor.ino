#include <SoftwareSerial.h>
#include <Wire.h>
#include <RTClib.h>
#include <SensirionI2CSen5x.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>

#define NODE_TYPE "OUTDOOR"
#define NODE_ID "1"

int minutes = 1;
unsigned long reading_time = (minutes * 30 * 1000L);
unsigned long last_time = 0L;
char* currentTimeStamp = "";

// DS1307 -----------------------------------------------------------------------------
RTC_DS1307 rtc;

const char* ds1307_reading(){  
  char buffer[] = "YYYY/MM/DD,hh:mm:ss";
  DateTime now = rtc.now();
  char *data = now.toString(buffer); 
  return data;
}

// Serial Communication --------------------------------------------------------------
const byte rxPin = 10;
const byte txPin = 11;
SoftwareSerial Arduino_Serial (rxPin, txPin);

void sendSerialData(const char *variable_id, float value){
  char finalBuffer[100] = ""; 
  
  strcat(finalBuffer,"<");

  strcat(finalBuffer,NODE_TYPE);  
  strcat(finalBuffer,",");

  strcat(finalBuffer,NODE_ID);  
  strcat(finalBuffer,",");

  strcat(finalBuffer,currentTimeStamp);  
  strcat(finalBuffer,",");

  strcat(finalBuffer,variable_id);  
  strcat(finalBuffer,",");

  static char strValue[7];
  dtostrf(value, 1, 2, strValue);
  strcat(finalBuffer,strValue);

  strcat(finalBuffer,">");

  Serial.println(finalBuffer);
  Arduino_Serial.print(finalBuffer);
  delay(1000);
}

// SEN54 -----------------------------------------------------------------------------
SensirionI2CSen5x sen5x;

void sen54_reading(){
  uint16_t error;
  char errorMessage[256];

  // Read Measurement
  float massConcentrationPm1p0;
  float massConcentrationPm2p5;
  float massConcentrationPm4p0;
  float massConcentrationPm10p0;
  float ambientHumidity;
  float ambientTemperature;
  float vocIndex;
  float noxIndex;

  error = sen5x.readMeasuredValues(
    massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
    massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
    noxIndex);

  if (error) {
    Serial.print("Error trying to execute readMeasuredValues(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } 
  else {
    sendSerialData("5",      massConcentrationPm1p0);  
    sendSerialData("4",      massConcentrationPm2p5);  
    sendSerialData("7",        massConcentrationPm4p0);
    sendSerialData("5",       massConcentrationPm10p0); 
    sendSerialData("2",     ambientHumidity);         
    sendSerialData("1", ambientTemperature);      
    //sendSerialData("VOCI",   vocIndex);               
  }
}

// ML8511 -----------------------------------------------------------------------------
#define UVOUT   A0 	//Output from the sensor
#define REF_3V3 A1 	//3.3V power on the Arduino board

void ml8511_reading(){
  int uvLevel  = averageAnalogRead(UVOUT);
  int refLevel = averageAnalogRead(REF_3V3);

  //Use the 3.3V power pin as a reference to get a very accurate output value from sensor
  float outputVoltage = 3.3 / refLevel * uvLevel;  
  //Convert the voltage to a UV intensity level
  float uvIntensity = mapfloat(outputVoltage, 0.99, 2.8, 0.0, 15.0); 

  sendSerialData("UVI", uvIntensity); 
}

int averageAnalogRead(int pinToRead){
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 

  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;

  return(runningValue);  
}

float mapfloat(float x, float in_min, float in_max, float out_min, float out_max){
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// BMP085---------------------------------------------------------------------
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085);

void bmp085_reading(){  
  sensors_event_t event;
  bmp.getEvent(&event);
 
  /* Display the results (barometric pressure is measure in hPa) */
  if (event.pressure)
  {
    sendSerialData("3", event.pressure); 
  }
  else
  {
    Serial.println("Sensor error");
  }
}

// -----------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Arduino_Serial.begin(9600);

  Serial.println("Arduino starting.");

  // DS1307 ---
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  
  Serial.println("DS1307 ready.");

  // SEN54 ---
  Wire.begin();
  sen5x.begin(Wire);
  delay(2000);

  uint16_t error;
  char errorMessage[256];
  error = sen5x.deviceReset();
  if (error) {
    Serial.print("Error trying to execute deviceReset(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  // Start Measurement
  error = sen5x.startMeasurement();
  if (error) {
    Serial.print("Error trying to execute startMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }

  Serial.println("SEN54 ready.");

  // ML8511 ---
  pinMode(UVOUT, INPUT);
  pinMode(REF_3V3, INPUT);

  Serial.println("ML8511 ready.");
 
//   // BMP085 ---
//  if(!bmp.begin())  {
//     Serial.print("No BMP085 detected ... Check your wiring or I2C ADDR.");
//     while(1);
//   }

//   Serial.println("BMP085 ready.");

  // Start
  delay(1000);
  Serial.println("<Arduino is ready>");
}

void loop() {  
  if (millis() - last_time >= reading_time) {
    last_time += reading_time;
    
    currentTimeStamp = ds1307_reading();
 
    sen54_reading();
    // ml8511_reading(); 
    // bmp085_reading();
  }
}