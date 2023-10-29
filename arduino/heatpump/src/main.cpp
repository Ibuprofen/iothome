#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DS18B20.h>
#include <Secrets.h>
#include <PubSubClient.h>

WiFiClient espClient;

const char *deviceid = "w008";

const uint32_t msSAMPLE_INTERVAL = 2500;
const uint32_t msMETRIC_PUBLISH = 30000;

double temp01C;
double temp01F;
double temp02C;
double temp02F;

uint32_t msLastMetric;
uint32_t msLastSample;

String clientId = "ESP8266Client-";
PubSubClient client(espClient);
char msg[128];

#define ONE_WIRE_BUSA  D1
#define ONE_WIRE_BUSB  D2

OneWire oneWireA(ONE_WIRE_BUSA);
OneWire oneWireB(ONE_WIRE_BUSB);

DS18B20 sensorA(&oneWireA);
DS18B20 sensorB(&oneWireB);

void setupWifi();
void reconnectMqtt();
void readDS();

void setup() {
  delay(3000);

  Serial.begin(9600);
  Serial.print("DS18B20 Library version: ");
  Serial.println(DS18B20_LIB_VERSION);

  setupWifi();

  client.setServer(mqtt_server, 1883);  

  sensorA.begin();
  sensorB.begin();

  // 9 is default
  if (!sensorA.setResolution(9)) {
    Serial.println("No sensorA found");
  };

  if (!sensorB.setResolution(9)) {
    Serial.println("No sensorB found");
  };

  sensorA.requestTemperatures();
  sensorB.requestTemperatures();
}

void loop() {
  if (!client.connected()) {
    reconnectMqtt();
  }
  client.loop();

  if (millis() - msLastSample >= msSAMPLE_INTERVAL) {
    msLastSample = millis();
    readDS();
  }

  if (millis() - msLastMetric >= msMETRIC_PUBLISH) {
    msLastMetric = millis();

    if ((temp01F > 0 && temp02F > 0) && (temp01F < 185 && temp02F < 185)) {
      sprintf(msg, "weather,deviceid=%s tempf_01=%.2f,tempf_02=%.2f", deviceid, temp01F, temp02F);
      Serial.print("Publish message: ");
      Serial.println(msg);
      client.publish("event", msg);
    } else {
      Serial.print("sensorA: ");
      Serial.print(temp01F); Serial.print(" ");
      Serial.print("sensorB: ");
      Serial.println(temp02F);
    }
  }
  
  delay(100);
}

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

void reconnectMqtt() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
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

void readDS() {
  // print the temperature when available and request a new reading
  if (sensorA.isConversionComplete()) {
    temp01C = sensorA.getTempC();
    temp01F = temp01C * 9 / 5 + 32;
    Serial.print("liquid\tC: "); Serial.print(temp01C); Serial.print(" F: "); Serial.println(temp01F);
    sensorA.requestTemperatures();
  }

  if (sensorB.isConversionComplete()) {
    temp02C = sensorB.getTempC();
    temp02F = temp02C * 9 / 5 + 32;
    Serial.print("gas\tC: "); Serial.print(temp02C); Serial.print(" F: "); Serial.println(temp02F);
    sensorB.requestTemperatures();
  }
}
