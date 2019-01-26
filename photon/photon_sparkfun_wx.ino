// This #include statement was automatically added by the Particle IDE.
#include <SparkFun_Photon_Weather_Shield_Library.h>

bool ledState = false;

float humidity = 0;
float tempf = 0;
float tempc = 0;
float pascals = 0;
float inhg = 0;
float baroTemp = 0;
float altf = 0;

char wxString[64];

//Create Instance of HTU21D or SI7021 temp and humidity sensor and MPL3115A2 barrometric sensor
Weather sensor;

float readBaro() {
  sensor.setModeBarometer();
  return sensor.readBaroTempF();
}

float readAltitude() {
  sensor.setModeAltimeter();
  return sensor.readAltitudeFt();
}

// Measure Pressure from the MPL3115A2
// When device first boots it may take a hundred or so
// iterations before the value is > 0
float readPressure() {
  float pa = sensor.readPressure();
  // try again
  if (pa == 0.00) {
    return readPressure();
  }
  return pa;
}

void getWeather() {

  // Measure Relative Humidity from the HTU21D or Si7021
  humidity = sensor.getRH();

  // Measure Temperature from the HTU21D or Si7021
  tempf = sensor.getTempF();
  tempc = sensor.getTemp();
  // Temperature is measured every time RH is requested.
  // It is faster, therefore, to read it from previous RH
  // measurement with getTemp() instead with readTemp()
  pascals = readPressure();

  //Measure the Barometer temperature in F from the MPL3115A2
  baroTemp = readBaro();

  //If in altitude mode, you can get a reading in feet  with this line:
  //altf = readAltitude();

  // "json"
  //sprintf(wxString, "{\"temp\": %.2f, \"hum\": %.2f, \"pa\": %.2f}", tempf, humidity, pascals);

  sprintf(wxString, "tempf=%.2f,tempc=%.2f,hum=%.2f,pa=%.2f", tempf, tempc, humidity, pascals);
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

  //Initialize the I2C sensors and ping them
  sensor.begin();

  sensor.setModeBarometer();
  sensor.setOversampleRate(7);
  sensor.enableEventFlags();

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