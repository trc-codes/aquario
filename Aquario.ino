// Aquario
// DS3231 Library Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
// web: http://www.RinkyDinkElectronics.com/

#include <DallasTemperature.h>
#include <OneWire.h>
#include <DS3231.h>

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);

// Define OneWire pin
#define ONE_WIRE_BUS 8

// Init OneWire
OneWire oneWire(ONE_WIRE_BUS);

// Pass OneWire to DallasTemp
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress insideThermometer, outsideThermometer;

// Init a Time-data structure
Time  t;

// Define Relay Digital I/O pin number
#define RELAY_ON 1
#define RELAY_OFF 0

#define Relay_1  2  
#define Relay_2  3
#define Relay_3  4
#define Relay_4  5

// set temp
float temp = 30;

// set upper temp variance
float upperTempVariance = 2;

// set lower temp variance
float lowerTempVariance = 5;

int waittime; // Delay between changes

void tempSensorsSetup() {
  // Start up the DallasTemp library
  sensors.begin();

  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  if (!sensors.getAddress(outsideThermometer, 1)) Serial.println("Unable to find address for Device 1");
}

float getSetTemp() {
  return temp;
}

void setSetTemp(float temp) {
  temp = temp;
}

float getUpperTempVariance() {
  return upperTempVariance;
}

void setUpperTempVariance(float upperTempVariance) {
  upperTempVariance = upperTempVariance;
}

float getLowerTempVariance() {
  return lowerTempVariance;
}

void setLowerTempVariance(float lowerTempVariance) {
  lowerTempVariance = lowerTempVariance;
}

// declare upperTempLimit and lowerTempLimit
float upperTempLimit = getSetTemp() + getUpperTempVariance();
float lowerTempLimit = getSetTemp() - getLowerTempVariance();

void setup() {
  // Setup Serial connection
  Serial.begin(9600);

  // Initialize the rtc object
  rtc.begin();
  
  // The following lines can be uncommented to set the date and time
  //rtc.setDOW(WEDNESDAY);     // Set Day-of-Week to SUNDAY
  rtc.setTime(10, 0, 0);     // Set the time to 12:00:00 (24hr format)
  //rtc.setDate(1, 1, 2014);   // Set the date to January 1st, 2014

  waittime = 100;
//-------( Initialize Pins so relays are inactive at reset)----
  digitalWrite(Relay_1, RELAY_OFF);
  digitalWrite(Relay_2, RELAY_OFF);
  digitalWrite(Relay_3, RELAY_OFF);
  digitalWrite(Relay_4, RELAY_OFF);  
  
  
//set digital pins as outputs
  pinMode(Relay_1, OUTPUT);   
  pinMode(Relay_2, OUTPUT);  
  pinMode(Relay_3, OUTPUT);  
  pinMode(Relay_4, OUTPUT);    
  delay(4000); //Check that all relays are inactive at 

  // temp setup
  tempSensorsSetup();
}

void loop() {
  // Get data from the DS3231
  t = rtc.getTime();
  
 /* // Send date over serial connection
  Serial.print("Today is the ");
  Serial.print(t.date, DEC);
  Serial.print(". day of ");
  Serial.print(rtc.getMonthStr());
  Serial.print(" in the year ");
  Serial.print(t.year, DEC);
  Serial.println(".");
  
  // Send Day-of-Week and time
  Serial.print("It is the ");
  Serial.print(t.dow, DEC);
  Serial.print(". day of the week (counting monday as the 1th), and it has passed ");
  Serial.print(t.hour, DEC);
  Serial.print(" hour(s), ");
  Serial.print(t.min, DEC);
  Serial.print(" minute(s) and ");
  Serial.print(t.sec, DEC);
  Serial.println(" second(s) since midnight.");

  // Send a divider for readability
  Serial.println("  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -");*/

//  // Send Day-of-Week
//  Serial.print(rtc.getDOWStr());
//  Serial.print(" ");
//  
//  // Send date
//  Serial.print(rtc.getDateStr());
//  Serial.print(" -- ");
//
//  // Send time
//  Serial.println(rtc.getTimeStr());
//  
//if(t.hour == 10 && t.min == 00 && t.sec == 15) {
//  turnRelayOnAndOff(Relay_1);
//}
//
//if(t.hour == 10 && t.min == 00 && t.sec == 30) {
//  turnRelayOnAndOff(Relay_2);
//}
//
//if(t.hour == 10 && t.min == 00 && t.sec == 45) {
//  turnRelayOnAndOff(Relay_3);
//}
//
//if(t.hour == 10 && t.min == 01 && t.sec == 00) {
//  turnRelayOnAndOff(Relay_4);
//}

  // Wait one second before repeating :)
  delay (1000);

  // Method 1:
  // check each address individually for an alarm condition
  checkTemp(insideThermometer);
  
}

void checkTemp(DeviceAddress deviceAddress) {
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println(sensors.getTempCByIndex(0));
  if (sensors.getTempCByIndex(0) >= upperTempLimit) {
    Serial.print("ALARM!! turning relay off: ");
    turnRelayOff(Relay_1, "Heater relay");
  } else if (sensors.getTempCByIndex(0) <= lowerTempLimit) {
    Serial.print("ALARM!! turning relay on: ");
    turnRelayOn(Relay_1, "Heater relay");
  }
}

void turnRelayOn(int relay_num, String relay_name) {
  Serial.print(relay_name + " ON");
  Serial.println("");
  digitalWrite(relay_num, RELAY_ON);// set the Relay ON
}

void turnRelayOff(int relay_num, String relay_name) {
  digitalWrite(relay_num, RELAY_OFF);// set the Relay OFF
  Serial.print(relay_name + " OFF");
  Serial.println("");
}
