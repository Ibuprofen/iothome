#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WEMOS_SHT3X.h>
#include "config.h"

WiFiClient espClient;

SHT3X sht30(0x45);

const int MAXRETRY = 4;
const uint32_t msSAMPLE_INTERVAL = 2500;
const uint32_t msMETRIC_PUBLISH = 30000;

float SHTcelsius;
float SHTfahrenheit;
float SHThumidity;
uint32_t msLastMetric;
uint32_t msLastSample;

PubSubClient client(espClient);
char msg[128];

void setupWifi() {
  // in case this device used to be a softAP, clear it
  WiFi.softAPdisconnect();

  delay(3000);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    WiFi.begin(SSID, PASSWORD);
    Serial.println("WiFi failed, retrying.");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Client Hostname: ");
  Serial.println(WiFi.hostname());
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
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void readSHT() {
  if (sht30.get() == 0) {
    SHTcelsius = sht30.cTemp;
    SHTfahrenheit = sht30.fTemp;
    SHThumidity = sht30.humidity;

    Serial.print("SHT C: "); Serial.print(SHTcelsius); Serial.print(" F: "); Serial.print(SHTfahrenheit);
    Serial.print(" H: "); Serial.println(sht30.humidity);
  } else {
    Serial.println("Error reading SHT sensor");
  }
}

void setup(void) {
  delay(3000);

  // start serial port
  Serial.begin(9600);

  setupWifi();

  client.setServer(MQTT_SERVER, 1883);

  Serial.println("");
  Serial.print("Device ID: "); Serial.println(DEVICEID);
}

void loop(void) {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - msLastSample >= msSAMPLE_INTERVAL) {
    msLastSample = millis();
    readSHT();
  }

  if (millis() - msLastMetric >= msMETRIC_PUBLISH) {
    msLastMetric = millis();

    if (SHTfahrenheit && SHThumidity) {
      sprintf(msg, "weather,deviceid=%s tempf_01=%.2f,hum_01=%.2f", DEVICEID, SHTfahrenheit, SHThumidity);
    }
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("event", msg);
  }
}
