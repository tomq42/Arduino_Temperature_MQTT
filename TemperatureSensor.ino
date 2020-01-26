#include <SPI.h>
#include <WiFiNINA.h>
#include <ArduinoMqttClient.h>
#include <DallasTemperature.h>

// Secrets.h must contain
// char ssid[] = "Wifi SSID";
// char pass[] = "Wifi password";
#include "secrets.h"

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// The address of the MQTT broker to connect to.
const char broker[] = "home-auto";

// The port number of the MQTT broker
int        port     = 1883;

// The root topic on which data will be published.
String     topicRoot  = "sensors/temperature/";

// The maximum interval in milliseconds between 
// broadcasts of the temperature values.
const unsigned long MAX_UPDATE_INTERVAL = 1*60*1000;

// The maximum temperature delta between updates; 
// if the temperature has changed by this amount since the last 
// update, a new update will be sent.
const float MAX_DELTA = 0.1;

// The interval in milliseconds between attempts to read the 
// temperature, or more accurately the delay time at the end
// of each iteration of "loop()".
const unsigned long READ_INTERVAL = 5000;

// The pin on the Arduino to which the data wire is connected.
const int ONE_WIRE_BUS = 2;

// We support a maximum (arbitrarily) of 10 devices on the wire.
const int MAX_DEVICES = 10;
 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);


class TemperatureMonitor {
private:
  DeviceAddress deviceAddress_;
  unsigned long lastSendTime_;
  float lastSentTemperature_;
  String topic_;
  
public:
  TemperatureMonitor() : lastSendTime_(-MAX_UPDATE_INTERVAL), lastSentTemperature_(0)
  {
  }

  uint8_t* deviceAddress() { return deviceAddress_; }
  void setTopic(const String& topic) { topic_ = topic; }
  
  void monitor() {
    float tempC = sensors.getTempC(deviceAddress_);
    unsigned long time = millis();
    // time overflows every 50 days or 
    // so, so we have to manage that. 
    // If it overflows, the subtraction will just produce a large positive number.
    if (time - lastSendTime_ > MAX_UPDATE_INTERVAL || abs(tempC - lastSentTemperature_) >= MAX_DELTA) {
      sendTemperature(tempC);
    }
  }

private:
  void sendTemperature(float tempC) {
    mqttClient.beginMessage(topic_.c_str());
    mqttClient.print(tempC, 2);
    mqttClient.endMessage();
    
    lastSendTime_ = millis();
    lastSentTemperature_ = tempC;
  }
};
TemperatureMonitor monitors[MAX_DEVICES];
int deviceCount=0;

void initialiseWifi() {
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
    // don't continue
    while (true);
  }
}

void waitForSerial() {
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void initialiseOneWire() {
  sensors.begin();

  // locate devices on the bus
  Serial.print("Locating devices...");
  Serial.print("Found ");
  Serial.print(sensors.getDeviceCount(), DEC);
  Serial.println(" devices.");
  
  oneWire.reset_search();
  for (deviceCount=0; deviceCount<MAX_DEVICES; deviceCount++) {
    if (!oneWire.search(monitors[deviceCount].deviceAddress())) break;
    monitors[deviceCount].setTopic(topicRoot + String(deviceCount, DEC));
  }
  Serial.print("Found ");
  Serial.print(deviceCount, DEC);
  Serial.println(" device addresses.");

  for (int i=0; i<deviceCount; i++) {
    printAddress(monitors[i].deviceAddress());
    Serial.println();
  }
}

void setup() {
  // Initialize serial
  Serial.begin(9600);
  // wait for port to open
  // Remove for production.
  waitForSerial();

  initialiseWifi();
  initialiseOneWire();
}

void loop() {
  // Ensure Wifi connected.
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Current Wifi status ");
    Serial.println(wl_status_str(WiFi.status()));
    Serial.print("Attempting to connect to WEP network, SSID: ");
    Serial.println(ssid);
    int status = WiFi.begin(ssid, pass);

    if (status != WL_CONNECTED) {
      Serial.print("Wifi connection! Error code = ");
      Serial.print(status);
      Serial.print(" ");
      Serial.println(wl_status_str(status));
      // Don't try again straight away.
      delay(5000);
      return;
    }
    
    Serial.println("Successfully connected to WiFi");
    printCurrentNet();
    return;
  }

  // Ensure connected to MQTT broker
  if (!mqttClient.connected()) {
    Serial.println("No MQTT connection.");
    
    Serial.print("Attempting to connect to the MQTT broker: ");
    Serial.println(broker);
  
    if (!mqttClient.connect(broker, port)) {
      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqttClient.connectError());
  
      // Don't try again straight away.
      delay(5000);
      return;
    }
    Serial.println("Successfully connected to MQTT broker");
  }

  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  readTemperatures();
  
  delay(READ_INTERVAL);
}

void readTemperatures() {
  sensors.requestTemperatures();
  for (int i=0; i<deviceCount; i++) {
    monitors[i].monitor();
  }
}
