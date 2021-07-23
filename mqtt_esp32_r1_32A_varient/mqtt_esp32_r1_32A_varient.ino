//For device type R6/10A
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

//Relays for switching appliances
#define Relay1  15

// Update these with values suitable for your network.

const char* ssid = "MICROTRON TECHNOLOGIES";
const char* password = "micro@440027";
const char* mqtt_server = "128.199.29.16";

#define sub1 ("/"+WiFi.macAddress()+"/r1").c_str()

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

//  WiFi.mode(WIFI_/STA);
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

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  if (strstr(topic, sub1))
  {
    for (int i = 0; i < length; i++) {
      Serial.print((char)payload[i]);
    }
    Serial.println();
    // Switch on the LED if an 1 was received as first character
    if ((char)payload[0] == '1') {
      //Logic to turn on the relay
      digitalWrite(Relay1, LOW);
    } else {
      //Logic to turn off the relay
      digitalWrite(Relay1, HIGH);
    }
  }
  else
  {
    Serial.println("unsubscribed topic");
  }

}

void setInitialState() {
  digitalWrite(Relay1, HIGH);
}

void publishSerialID() {
  StaticJsonDocument<256> doc;
  String Mac_Address = WiFi.macAddress();
  doc["mac_address"] = Mac_Address;
  doc["type"]= "R1/32A";
  doc["no_of_relays"] = "1";
  doc["amp"] = "32A";
  char device_info[128];
  serializeJson(doc, device_info);
  
  client.publish("device_serial",device_info);

  Serial.print("Device MAC Address: ");
  Serial.println(Mac_Address);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      //Once connected send an chip id and resubscribe
      publishSerialID();
      client.subscribe(sub1);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  setInitialState();
  pinMode(Relay1, OUTPUT);
  
  Serial.begin(115200);
  
  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
