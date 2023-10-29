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

double liquidC;
double liquidF;
double gasC;
double gasF;

uint32_t msLastMetric;
uint32_t msLastSample;

String clientId = "ESP8266Client-";
PubSubClient client(espClient);
char msg[128];

#define ONE_WIRE_BUSA  2
#define ONE_WIRE_BUSB  3

OneWire oneWireA(ONE_WIRE_BUSA);
OneWire oneWireB(ONE_WIRE_BUSB);

DS18B20 liquidSensor(&oneWireA);
DS18B20 gasSensor(&oneWireB);

void setupWifi();
void reconnectMqtt();
void readDS();

void setup() {
  delay(3000);

  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("DS18B20 Library version: ");
  Serial.println(DS18B20_LIB_VERSION);

  setupWifi();

  client.setServer(mqtt_server, 1883);  

  liquidSensor.begin();
  gasSensor.begin();

  liquidSensor.requestTemperatures();
  gasSensor.requestTemperatures();
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

    if (liquidF && gasF) {
      sprintf(msg, "weather,deviceid=%s tempf_01=%.2f,tempf_02=%.2f", deviceid, liquidF, gasF);
    }
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish("event", msg);
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
  if (liquidSensor.isConversionComplete()) {
    Serial.print("liquidSensor:\t");
    Serial.println(liquidSensor.getTempC(),1);
    liquidSensor.requestTemperatures();
    liquidC = liquidSensor.getTempC();
    liquidF = liquidC * 9 / 5 + 32;
    Serial.print("liquid C: "); Serial.print(liquidC); Serial.print(" F: "); Serial.println(liquidF);
  }

  if (gasSensor.isConversionComplete()) {
    Serial.print("gasSensor:\t\t");
    Serial.println(gasSensor.getTempC(),1);
    gasSensor.requestTemperatures();
    gasC = gasSensor.getTempC();
    gasF = gasC * 9 / 5 + 32;
    Serial.print("gas C: "); Serial.print(gasC); Serial.print(" F: "); Serial.println(gasF);
  }
}
