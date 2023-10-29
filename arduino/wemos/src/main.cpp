#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DS18B20.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

const char *ssid = "";
const char *password = "";
const char *mqtt_server = "";
WiFiClient espClient;

const char *deviceid = "w003";

#define ONE_WIRE_BUS D4

OneWire oneWire(ONE_WIRE_BUS);
DS18B20 DSsensor(&oneWire);

const int MAXRETRY = 4;
const uint32_t msSAMPLE_INTERVAL = 2500;
const uint32_t msMETRIC_PUBLISH = 30000;

double DHTcelsius;
double DScelsius;
double DHTfahrenheit;
double DSfahrenheit;
double DHThumidity;
uint32_t msLastMetric;
uint32_t msLastSample;

PubSubClient client(espClient);
char msg[128];

#define DHTPIN D3     // Digital pin connected to the DHT sensor 

// Uncomment the type of sensor in use:
//#define DHTTYPE    DHT11     // DHT 11
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
//#define DHTTYPE    DHT21     // DHT 21 (AM2301)

DHT_Unified dht(DHTPIN, DHTTYPE);

void setupWifi() {
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

void setupDHT() {
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor"));
  // Print temperature sensor details.
  sensor_t DHTsensor;
  dht.temperature().getSensor(&DHTsensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(DHTsensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(DHTsensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(DHTsensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(DHTsensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(DHTsensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(DHTsensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&DHTsensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(DHTsensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(DHTsensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(DHTsensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(DHTsensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(DHTsensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(DHTsensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));
}

void readDHT() {
  // Get temperature event and print its value.
  sensors_event_t DHTevent;
  dht.temperature().getEvent(&DHTevent);
  if (isnan(DHTevent.temperature)) {
    Serial.println(F("Error reading temperature!"));
  } else {
    DHTcelsius = DHTevent.temperature;
    DHTfahrenheit = DHTcelsius * 9 / 5 + 32;
    Serial.print("DHT C: "); Serial.print(DHTcelsius); Serial.print(" F: "); Serial.print(DHTfahrenheit);
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&DHTevent);
  if (isnan(DHTevent.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  } else {
    Serial.print(" Hum: ");
    DHThumidity = DHTevent.relative_humidity; 
    Serial.print(DHThumidity);
    Serial.println("%");
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

  setupDHT();
}

void loop(void) {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if (millis() - msLastSample >= msSAMPLE_INTERVAL) {
    msLastSample = millis();
    readDS();
    readDHT();
  }

  if (millis() - msLastMetric >= msMETRIC_PUBLISH) {
    msLastMetric = millis();

    if (!DHTfahrenheit || !DHThumidity) {
      sprintf(msg, "weather,deviceid=%s tempf_02=%.2f", deviceid, DSfahrenheit);
    } else {
      sprintf(msg, "weather,deviceid=%s tempf_01=%.2f,tempf_02=%.2f,hum_01=%.2f", deviceid, DHTfahrenheit, DSfahrenheit, DHThumidity);
    }
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("event", msg);
  }
}
