// Aquario
// DS3231 Library Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
// web: http://www.RinkyDinkElectronics.com/

// Init libraries
#include <DallasTemperature.h>
#include <OneWire.h>
#include <DS3231.h>
#include <SPI.h>
#include <Ethernet.h>

// Define OneWire pin
#define ONE_WIRE_BUS 9

// Define Relay Digital I/O pin number and names
#define RELAY_ON LOW
#define RELAY_OFF HIGH
#define relay_1  2  // SSR
String relay_1_name = "Heater";
#define relay_2  3  // SSR
String relay_2_name = "Lighting";
#define relay_3  4  // SSR
String relay_3_name = "CO2";
#define relay_4  5  // 
String relay_4_name = "Pump1";
#define relay_5  6  // 
String relay_5_name = "Pump2";
#define relay_6  7  // 
String relay_6_name = "Pump3";
#define relay_7  8  // 
String relay_7_name = "Pump4";

// Ethernet init
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 0, 177);
EthernetServer server(80);

void ethernetSetup() {
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

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

  // Ethernet setup
  ethernetSetup();

  // Initialize the rtc object
  rtc.begin();
  setCurrentTime(10, 0, 0);     // Set the time to 12:00:00 (24hr format)

  //set digital pins as outputs
  pinMode(relay_4, OUTPUT);  
  pinMode(relay_5, OUTPUT);  
  pinMode(relay_6, OUTPUT);  
  pinMode(relay_7, OUTPUT);
  
  //-------( Initialize Pins so relays are inactive at reset)----
  digitalWrite(relay_4, RELAY_OFF);
  digitalWrite(relay_5, RELAY_OFF);
  digitalWrite(relay_6, RELAY_OFF);
  digitalWrite(relay_7, RELAY_OFF);  
      
  delay(4000); //Check that all relays are inactive at 

  // temp setup
  tempSensorsSetup();  
}

void loop() {

  delay (2000);
  turnRelayOn(5, "test relay");
  // Wait one second before repeating :)
  delay (1000);
  turnRelayOff(5, "test relay");
  delay (2000);

//  t = rtc.getTimeStr();

//  Serial.println(rtc.getTimeStr());
//
//  checkTemp(insideThermometer);
  serveWebpage();
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

// Serve webpage
void serveWebpage() {
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 10");  // refresh the page automatically every 10 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // Send the command to get temperatures
          sensors.requestTemperatures();
          // output the value of each analog input pin
          int sensorReading = digitalRead(9);
          client.print("<h1 style=\"color: #fff; background-color: #0000cc; padding: 10px; border-radius: 5px; font-family: Arial;\">Temperature is ");
          client.print(sensors.getTempCByIndex(0));
          client.print(" - just saying! </h1>");
          client.println("<br />");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}
