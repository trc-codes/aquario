// Aquario
// DS3231 Library Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
// web: http://www.RinkyDinkElectronics.com/

// Init libraries
#include <DallasTemperature.h>
#include <OneWire.h>
#include <DS3231.h>

// Define OneWire pin
#define ONE_WIRE_BUS 8

// Define Relay Digital I/O pin number and names
#define RELAY_ON 1
#define RELAY_OFF 0
#define relay_1  2  // SSR
#define relay_1_name = "Heater"
#define relay_2  3  // SSR
#define relay_2_name = "Lighting"
#define relay_3  4  // SSR
#define relay_3_name = "CO2"
#define relay_4  5  // 
#define relay_4_name = "Pump1"
#define relay_5  6  // 
#define relay_5_name = "Pump2"
#define relay_6  7  // 
#define relay_6_name = "Pump3"
#define relay_7  8  // 
#define relay_7_name = "Pump4"


// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);

// Init OneWire
OneWire oneWire(ONE_WIRE_BUS);

// Pass OneWire to DallasTemp
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress insideThermometer, outsideThermometer;

// Init a Time-data structure
Time  t;

// set water temp
float temp = 30;

// set upper water temp variance
float upperTempVariance = 2;

// set lower water temp variance
float lowerTempVariance = 5;

void tempSensorsSetup() {
  // Start up the DallasTemp library
  sensors.begin();

  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  if (!sensors.getAddress(outsideThermometer, 1)) Serial.println("Unable to find address for Device 1");
}

float getSetTemp() {
  return temp;
}

//void setSetTemp(float temp) {
//  temp = temp;
//}

float getUpperTempVariance() {
  return upperTempVariance;
}

//void setUpperTempVariance(float upperTempVariance) {
//  upperTempVariance = upperTempVariance;
//}

float getLowerTempVariance() {
  return lowerTempVariance;
}

//void setLowerTempVariance(float lowerTempVariance) {
//  lowerTempVariance = lowerTempVariance;
//}

// declare upperTempLimit and lowerTempLimit
float upperTempLimit = getSetTemp() + getUpperTempVariance();
float lowerTempLimit = getSetTemp() - getLowerTempVariance();

void setup() {
  // Setup Serial connection
  Serial.begin(9600);

  // Initialize the rtc object
  rtc.begin();
  setCurrentTime(10, 0, 0);     // Set the time to 12:00:00 (24hr format)

  //set digital pins as outputs
  pinMode(relay_1, OUTPUT);  
  pinMode(relay_2, OUTPUT);  
  pinMode(relay_3, OUTPUT);  
  pinMode(relay_4, OUTPUT);
  
  //-------( Initialize Pins so relays are inactive at reset)----
  digitalWrite(relay_1, RELAY_OFF);
  digitalWrite(relay_2, RELAY_OFF);
  digitalWrite(relay_3, RELAY_OFF);
  digitalWrite(relay_4, RELAY_OFF);  
      
  delay(4000); //Check that all relays are inactive at 

  // temp setup
  tempSensorsSetup();  
}

void loop() {

  delay (2000);
  turnRelayOn(2, "relay");
  // Wait one second before repeating :)
  delay (1000);
  turnRelayOff(2, "relay");
  delay (2000);

//  t = rtc.getTimeStr();

  Serial.println(rtc.getTimeStr());

//  checkTemp(insideThermometer);
}

void checkTemp(DeviceAddress deviceAddress) {
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println(sensors.getTempCByIndex(0));
  if (sensors.getTempCByIndex(0) >= upperTempLimit) {
    Serial.print("ALARM!! turning heater off.");
    turnRelayOff(relay_1, relay_1_name);
  } else if (sensors.getTempCByIndex(0) <= lowerTempLimit) {
    Serial.print("ALARM!! turning heater on.");
    turnRelayOn(relay_1, relay_1_name);
  }
}

void turnRelayOn(int relay_num, String relay_name) {
  Serial.print(relay_name + " ON");
  Serial.println("");
  digitalWrite(relay_num, HIGH);// set the Relay ON
}

void turnRelayOff(int relay_num, String relay_name) {
  digitalWrite(relay_num, LOW);// set the Relay OFF
  Serial.print(relay_name + " OFF");
  Serial.println("");
}

Time getCurrentTime() {
  // Need the internet connection for this - from the Raspberry Pi?
}

void setCurrentTime(int hours, int minutes, int seconds) {
  rtc.setTime(hours, minutes, seconds);
}

