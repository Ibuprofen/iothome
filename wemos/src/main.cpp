#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DS18B20.h>
#include <WEMOS_SHT3X.h>

const char *ssid = "";
const char *password = "";
const char *mqtt_server = "";
WiFiClient espClient;

const char *deviceid = "w003";

#define ONE_WIRE_BUS D4

OneWire oneWire(ONE_WIRE_BUS);
DS18B20 DSsensor(&oneWire);

SHT3X sht30(0x45);

const int MAXRETRY = 4;
const uint32_t msSAMPLE_INTERVAL = 2500;
const uint32_t msMETRIC_PUBLISH = 30000;

double DScelsius;
double DSfahrenheit;
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
  Serial.println(ssid);

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
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
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

void readDS() {
  DSsensor.requestTemperatures();
  //Serial.println(DSsensor.getTempC());
  DScelsius = DSsensor.getTempC();
  DSfahrenheit = DScelsius * 9 / 5 + 32;
  Serial.print("DS  C: "); Serial.print(DScelsius); Serial.print(" F: "); Serial.println(DSfahrenheit);
}

void setup(void) {
  delay(3000);

  // start serial port
  Serial.begin(9600);

  setupWifi();

  client.setServer(mqtt_server, 1883);

  DSsensor.begin();

  Serial.println("");
  Serial.print("Device ID: "); Serial.println(deviceid);
}

void loop(void) {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - msLastSample >= msSAMPLE_INTERVAL) {
    msLastSample = millis();
    readDS();
    readSHT();
    Serial.println();
  }

  if (millis() - msLastMetric >= msMETRIC_PUBLISH) {
    msLastMetric = millis();

    if (!SHTfahrenheit || !SHThumidity) {
      sprintf(msg, "weather,deviceid=%s tempf_02=%.2f", deviceid, DSfahrenheit);
    } else {
      sprintf(msg, "weather,deviceid=%s tempf_01=%.2f,tempf_02=%.2f,hum_01=%.2f", deviceid, SHTfahrenheit, DSfahrenheit, SHThumidity);
    }
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("event", msg);
  }
}
