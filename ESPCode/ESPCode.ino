#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>

// WIFI ------------------------------------------------------------------------------
const char* ssid = "iPhone";
const char* password = "12345678";

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

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
    strcpy(tempChars, receivedChars);
    parseData();
    showParsedData();
    //sendMqttData(recvType, recvValue);
    newData = false;
  }
}

void parseData() {     
  char* strtokIndx; 

  strtokIndx = strtok(tempChars,",");     
  strcpy(recvType, strtokIndx); 

  strtokIndx = strtok(NULL, ",");
  strcpy(recvValue, strtokIndx); 
}

void showParsedData() {
  Serial.print("Type: ");
  Serial.print(recvType); 
  Serial.print(" Value: ");
  Serial.println(recvValue);
}

// -----------------------------------------------------------------------------------
void setup() {
  Serial.begin(57600);
  ESP_Serial.begin(9600);

  setup_wifi();
  //client.setServer(mqtt_server, mqtt_port);

  Serial.println("<ESP is ready>");
}

void loop() {
  //if (!client.connected()) {
  //  reconnect();
  //}  
  //client.loop();

  recvSerialData();
}

