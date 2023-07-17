#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include "config.h"

// WIFI ------------------------------------------------------------------------------
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

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

// MQTT ------------------------------------------------------------------------------
const char *mqtt_server = MQTT_SERVER; 
const char *mqtt_clientid = MQTT_CLIENT_ID;
const char *mqtt_username = MQTT_USERNAME;
const char *mqtt_password = MQTT_PASSWORD;
const int   mqtt_port = MQTT_PORT;
const int   mqtt_websocket_port = MQTT_WEBSOCKET_PORT;

WiFiClientSecure espClient;
PubSubClient client(espClient);

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");   
    String clientId = "ESP8266Client-";   // Create a random client ID
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),mqtt_username,mqtt_password)) {
      Serial.println("connected");
      
      // subscribe the topics here

    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String incommingMessage = "";
  for (int i = 0; i < length; i++) incommingMessage+=(char)payload[i];

  Serial.println("Message arrived ["+String(topic)+"]"+incommingMessage);

  //--- check the incomming message

}

void publishMessage(const char* topic, String payload , boolean retained){
  if (client.publish(topic, payload.c_str(), true))
    Serial.println("Message published ["+String(topic)+"]: "+payload);
}

// Serial Communication --------------------------------------------------------------
const byte rxPin = D1;
const byte txPin = D2;
SoftwareSerial ESP_Serial(rxPin, txPin);

const byte numChars = 32;
char receivedChars[numChars];
char tempChars[numChars];       

char recvType[numChars]  = {0};
char recvTopic[numChars] = {0};
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
}

void parseRecvData() {     
  char* strtokIndx; 

  strtokIndx = strtok(tempChars,",");     
  strcpy(recvType, strtokIndx); 

  strtokIndx = strtok(NULL, ",");
  strcpy(recvTopic, strtokIndx); 

  strtokIndx = strtok(NULL, ",");
  strcpy(recvValue, strtokIndx); 
}

void showParsedData() {
  Serial.print("Type: ");
  Serial.print(recvType); 
  Serial.print(" / Topic: ");
  Serial.print(recvTopic);
  Serial.print(" / Value: ");
  Serial.println(recvValue);
}


void sendSerialData(){
  DynamicJsonDocument doc(1024);

  doc["device"] = "indoor";
  doc["type"] = recvType;
  doc["topic"] = recvTopic;
  doc["value"] = recvValue;
 
  char mqtt_message[128];
  serializeJson(doc, mqtt_message);

  publishMessage(recvTopic, mqtt_message, true);

  delay(500);
}

// -----------------------------------------------------------------------------------
void setup() {
  Serial.begin(57600);
  ESP_Serial.begin(9600);

  setup_wifi();
  
  espClient.setInsecure();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  Serial.println("<ESP is ready>");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }  
  client.loop();

  recvSerialData();
  if (newData == true) {
    //Serial.println(receivedChars);
    strcpy(tempChars, receivedChars);
    parseRecvData();
    //showParsedData();
    sendSerialData();
    newData = false;
  }
}

