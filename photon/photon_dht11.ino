#include "Adafruit_DHT.h"

#define DHTPIN 2     // what pin we're connected to
#define DHTTYPE DHT11   // DHT 11 
DHT dht(DHTPIN, DHTTYPE);

bool ledState = false;

float humidity = 0;
float tempf = 0;
float tempc = 0;
float pascals = 0;
float inhg = 0;
float baroTemp = 0;
float altf = 0;

char wxString[64];

void getWeather() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  humidity = dht.getHumidity();
  // Read temperature as Celsius
  tempc = dht.getTempCelcius();
  // Read temperature as Farenheit
  tempf = dht.getTempFarenheit();
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(humidity) || humidity == 0.0) {
    return;
  }

  // Compute heat index
  // Must send in temp in Fahrenheit!
  //float hi = dht.getHeatIndex();
  //float dp = dht.getDewPoint();
  //float k = dht.getTempKelvin();

  sprintf(wxString, "tempf=%.2f,tempc=%.2f,hum=%.2f", tempf, tempc, humidity);
}

int getAndPublishWeather(String command) {
  getWeather();

  Particle.publish("weather", wxString, PRIVATE);

  return 1;
}

void ledOn() {
  digitalWrite(D7, HIGH);
  ledState = !ledState;
}
void ledOff() {
  digitalWrite(D7, LOW);
  ledState = !ledState;
}

//---------------------------------------------------------------
void setup() {
  Serial1.begin(38400);

  pinMode(D7, OUTPUT);

  dht.begin();

  getWeather();

  Particle.function("getWeather", getAndPublishWeather);

  Particle.variable("wxstr", wxString);
}

//---------------------------------------------------------------
void loop() {
  // sanity blinking
//   if (millis() % 1000 == 0 && ledState) {
//     ledOn();
//   } else if (millis() % 1000 == 0) {
//     ledOff();
//   }
  
  if (millis() % 60000 == 0) {
    ledOn();
    getAndPublishWeather("");
    ledOff();
  }
  //System.sleep(SLEEP_MODE_DEEP);
}