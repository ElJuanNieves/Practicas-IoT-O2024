/****************************************
 * Include Libraries
 ****************************************/
#include <WiFi.h>
#include <PubSubClient.h> //Install library "PubSubClient" by Nick O'Leary
#include "DHT.h"

/****************************************
 * Define Constants
 ****************************************/
#define WIFISSID "YOUR_WIFI_NAME_HERE" // Put your WifiSSID here
#define PASSWORD "YOUR_WIFI_PASSWORD_HERE" // Put your wifi password here
#define TOKEN "YOUR_UBIDOTS_TOKEN_HERE" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "YOUR_MQTT_CLIENT_NAME_HERE" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
                                           //it should be a random and unique ascii string and different from all other devices

#define VARIABLE_LABEL_temp "temperatura" // Assing the variable label
#define VARIABLE_LABEL_hum "humedad" // Assing the variable label
#define DEVICE_LABEL "deviceForMobileApp" // Assig the device label

#define SENSOR 13 // Set the GPIO13 as SENSOR

char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[200];
char topic[150];
// Space to store values to send
char str_temp[10];
char str_hum[10];

#define pin1 13
DHT dht1(pin1, DHT11);    //El azul.

/****************************************
 * Auxiliar Functions
 ****************************************/
WiFiClient ubidots;
PubSubClient client(ubidots);

void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}

/****************************************
 * Main Functions
 ****************************************/
void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFISSID, PASSWORD);
  // Assign the pin as INPUT 
  pinMode(SENSOR, INPUT);

  Serial.println();
  Serial.print("Wait for WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);  

  dht1.begin();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL_temp); // Adds the variable label
  float t1 = dht1.readTemperature();

  Serial.println(t1);

  /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
  dtostrf(t1, 4, 2, str_temp);
   
  
  sprintf(payload, "%s {\"value\": %s", payload, str_temp); // Adds the value
  sprintf(payload, "%s } }", payload); // Closes the dictionary brackets
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);


  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", VARIABLE_LABEL_hum); // Adds the variable label
  float h1 = dht1.readHumidity();
  Serial.println(h1);

  /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
  dtostrf(h1, 4, 2, str_hum);
   
  sprintf(payload, "%s {\"value\": %s", payload, str_hum); // Adds the value
  sprintf(payload, "%s } }", payload); // Closes the dictionary brackets
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);
  
  client.loop();
  delay(15000);
}
