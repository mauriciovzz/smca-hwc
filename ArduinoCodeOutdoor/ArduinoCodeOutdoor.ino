#include <SoftwareSerial.h>
#include <SensirionI2CSen5x.h>
#include <Wire.h>

// Serial Communication --------------------------------------------------------------
const byte rxPin = 10;
const byte txPin = 11;
SoftwareSerial Arduino_Serial (rxPin, txPin);

void sendSerialData(const char *type, char *topic, float value){
  char finalBuffer[64] = ""; 
  
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
  delay(1000);
}

// SEN54 -----------------------------------------------------------------------------
const char *topicTemp   = "outdoor/00001/temp";
const char *topicHum    = "outdoor/00001/hum";
const char *topicPM10   = "outdoor/00001/pm10";
const char *topicPM2P5  = "outdoor/00001/pm2p5";
const char *topicVOC    = "outdoor/00001/voc";
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
    sendSerialData("pm2p5", topicPM2P5, massConcentrationPm2p5);
    sendSerialData("pm10",  topicPM10,  massConcentrationPm10p0);
    sendSerialData("hum",   topicHum,   ambientHumidity);
    sendSerialData("temp",  topicTemp,  ambientTemperature);
    sendSerialData("voc",   topicVOC,   vocIndex);
  }
}

// -----------------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  Arduino_Serial.begin(9600);

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

  //temp offset
  float tempOffset = 0.0;
  error = sen5x.setTemperatureOffsetSimple(tempOffset);
  if (error) {
    Serial.print("Error trying to execute setTemperatureOffsetSimple(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  } else {
    Serial.print("Temperature Offset set to ");
    Serial.print(tempOffset);
    Serial.println(" deg. Celsius (SEN54/SEN55 only)");
  }

  // Start Measurement
  error = sen5x.startMeasurement();
  if (error) {
    Serial.print("Error trying to execute startMeasurement(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
  delay(1000);
  
  Serial.println("<Arduino is ready>");
}

void loop() {
  sen54_reading();
}