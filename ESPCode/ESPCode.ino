#include <SoftwareSerial.h>

// Serial Communication --------------------------------------------------------------
const byte rxPin = D1;
const byte txPin = D2;
SoftwareSerial ESP_Serial(rxPin, txPin);

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];       

char recvType[numChars] = {0};
char recvValue[numChars] = {0};

boolean newData = false;

void recvSerialData(){
  static boolean recvInProgress = false;
  static byte ndx = 0;
  char startMarker = '<';
  char endMarker = '>';
  char rc;

  while(ESP_Serial.available() > 0 && newData == false){
    rc = ESP_Serial.read();

    if(recvInProgress == true){
      if (rc != endMarker) {
        receivedChars[ndx] = rc;
        ndx++;
        if (ndx >= numChars) {
          ndx = numChars - 1;
        }
      }
      else {
        receivedChars[ndx] = '\0'; // terminate the string
        recvInProgress = false;
        ndx = 0;
        newData = true;
      }
    }
    else if (rc == startMarker) {
      recvInProgress = true;
    }
  }

  if (newData == true) {
    Serial.println(receivedChars);
    newData = false;
  }
}

// -----------------------------------------------------------------------------------
void setup() {
  Serial.begin(57600);
  ESP_Serial.begin(9600);

  Serial.println("<ESP is ready>");
}

void loop() {
  recvSerialData();
}

