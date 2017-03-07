// Aquario
// DS3231 Library Copyright (C)2015 Rinky-Dink Electronics, Henning Karlsen. All right reserved
// web: http://www.RinkyDinkElectronics.com/

// Init libraries
#include <DallasTemperature.h>
#include <OneWire.h>
#include <DS3231.h>
#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <Regexp.h>

// Define OneWire pin
#define ONE_WIRE_BUS 9

// size of buffer used to capture HTTP requests
#define REQ_BUF_SZ 100

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
File webFile; // the web page file on the SD card
char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
char req_index = 0; // index into HTTP_req buffer

void ethernetSetup() {
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

void sdCardSetup() {
  // initialize SD card
    Serial.println("Initializing SD card...");
    if (!SD.begin(4)) {
        Serial.println("ERROR - SD card initialization failed!");
        return;    // init failed
    }
    Serial.println("SUCCESS - SD card initialized.");
    // check for state.htm file
    if (!SD.exists("state.htm")) {
        Serial.println("ERROR - Can't find state.htm file!");
        return;  // can't find index file
    }
    Serial.println("SUCCESS - Found state.htm file.");
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

// default lightSchedule
String lightSchedule[7][5] {
  {"Monday", "06:00", "10:00", "16:00", "22:00"},
  {"Tuesday", "06:00", "10:00", "16:00", "22:00"},
  {"Wednesday", "06:00", "10:00", "16:00", "22:00"},
  {"Thursday", "06:00", "10:00", "16:00", "22:00"},
  {"Friday", "06:00", "10:00", "16:00", "22:00"},
  {"Saturday", "08:00", "10:00", "16:00", "22:00"},
  {"Sunday", "08:00", "10:00", "16:00", "22:00"},
};

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
  // disable Ethernet chip
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
  
  // Setup Serial connection
  Serial.begin(9600);

  // Ethernet setup
  ethernetSetup();

  // SD card setup
  sdCardSetup();

  // Initialize the rtc object
  rtc.begin();

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
//  checkTemp(insideThermometer);
  serveWebpage();
  checkSchedule();
  delay(1000);
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
  EthernetClient client = server.available();  // try to get client

    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                // limit the size of the stored received HTTP request
                // buffer first part of HTTP request in HTTP_req array (string)
                // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)
                if (req_index < (REQ_BUF_SZ - 1)) {
                    HTTP_req[req_index] = c;          // save HTTP request character
                    req_index++;
                }
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                      // send a standard http response header
                      client.println("HTTP/1.1 200 OK");
                      // remainder of header follows below, depending on if
                      // web page or XML page is requested
                      // Ajax request - send XML file
                      if (StrContains(HTTP_req, "ajax_inputs")) {
                        // send rest of HTTP header
                        client.println("Content-Type: text/xml");
                        client.println("Connection: keep-alive");
                        client.println();
                        if (StrContains(HTTP_req, "&hr")) {
                          // match state object
                          MatchState ms;
                          char buf [100];
                          ms.Target (HTTP_req);  // set its address
                          char result = ms.Match ("&hr=(%d+)&min=(%d+)&sec=(%d+)", 0);
                          int currentHr = atoi(ms.GetCapture (buf, 0));
                          int currentMin = atoi(ms.GetCapture (buf, 1));
                          int currentSec = atoi(ms.GetCapture (buf, 2));
                          setCurrentTime(currentHr, currentMin, currentSec);
                        }
                        if (StrContains(HTTP_req, "&tt")) {
                          // match state object
                          MatchState ms;
                          char buf [100];
                          ms.Target (HTTP_req);  // set its address
                          char result = ms.Match ("&tt=(%d+)&tuv=(%d+)&tlv=(%d+)", 0);
                          int tankTemp = atoi(ms.GetCapture (buf, 0));
                          int tankTempUpperVar = atoi(ms.GetCapture (buf, 1));
                          int tankTempLowerVar = atoi(ms.GetCapture (buf, 2));
                          setSetTemp(tankTemp);
                          setUpperTempVariance(tankTempUpperVar);
                          setLowerTempVariance(tankTempLowerVar);
                        }
                        if (StrContains(HTTP_req, "&mst1")) {
                          // match state object
                          matchAndUpdateLightSchedule("Monday", "&mst1=(%d+:%d+)&met1=(%d+:%d+)&mst2=(%d+:%d+)&met2=(%d+:%d+)", HTTP_req);
                        }
                        if (StrContains(HTTP_req, "&tst1")) {
                          // match state object
                          matchAndUpdateLightSchedule("Tuesday", "&tst1=(%d+:%d+)&tet1=(%d+:%d+)&tst2=(%d+:%d+)&tet2=(%d+:%d+)", HTTP_req);                       
                        }
                        if (StrContains(HTTP_req, "&wst1")) {
                          // match state object
                          matchAndUpdateLightSchedule("Wednesday", "&wst1=(%d+:%d+)&wet1=(%d+:%d+)&wst2=(%d+:%d+)&wet2=(%d+:%d+)", HTTP_req);                         
                        }
                        if (StrContains(HTTP_req, "&thst1")) {
                          // match state object
                          matchAndUpdateLightSchedule("Thursday", "&thst1=(%d+:%d+)&thet1=(%d+:%d+)&thst2=(%d+:%d+)&thet2=(%d+:%d+)", HTTP_req);                       
                        }
                        if (StrContains(HTTP_req, "&fst1")) {
                          // match state object
                          matchAndUpdateLightSchedule("Friday", "&fst1=(%d+:%d+)&fet1=(%d+:%d+)&fst2=(%d+:%d+)&fet2=(%d+:%d+)", HTTP_req);                      
                        }
                        if (StrContains(HTTP_req, "&sst1")) {
                          // match state object
                          matchAndUpdateLightSchedule("Saturday", "&sst1=(%d+:%d+)&set1=(%d+:%d+)&sst2=(%d+:%d+)&set2=(%d+:%d+)", HTTP_req);                       
                        }
                        if (StrContains(HTTP_req, "&sust1")) {
                          // match state object
                          matchAndUpdateLightSchedule("Sunday", "&sust1=(%d+:%d+)&suet1=(%d+:%d+)&sust2=(%d+:%d+)&suet2=(%d+:%d+)", HTTP_req);                         
                        }
                        // send XML file containing input states
                        XML_response(client);
                    } 
                    
                    else {  // web page request
                        // send rest of HTTP header
                        client.println("Content-Type: text/html");
                        client.println("Connection: keep-alive");
                        client.println();
                        if (StrContains(HTTP_req, "GET /settings.htm")) {
                          webFile = SD.open("settings.htm");
                          if (webFile) {
                              while(webFile.available()) {
                                client.write(webFile.read()); // send web page to client
                              }
                              webFile.close();
                          }
                        }

                        else {
                          // send web page
                          webFile = SD.open("state.htm");        // open web page file
                          if (webFile) {
                              while(webFile.available()) {
                                client.write(webFile.read()); // send web page to client
                              }
                              webFile.close();
                          }
                        }
                    }
                    // display received HTTP request on serial port
                    Serial.print(HTTP_req);
                    // reset buffer index and all buffer elements to 0
                    req_index = 0;
                    StrClear(HTTP_req, REQ_BUF_SZ);
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1000);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}

void XML_response(EthernetClient cl ) {
  sensors.requestTemperatures();    
  cl.print("<?xml version = \"1.0\" ?>");
  cl.print("<inputs>");
  cl.print("<tankTemperature>");
  cl.print(sensors.getTempCByIndex(0));
  cl.print("</tankTemperature>");
  cl.print("<currentTime>");
  cl.print(rtc.getTimeStr());
  cl.print("</currentTime>");
  cl.print("<currentDate>");
  cl.print(rtc.getDateStr());
  cl.print("</currentDate>");
  cl.print("<currentDay>");
  cl.print(rtc.getDOWStr());
  cl.print("</currentDay>");
  cl.print("<MondayLightsStartTime1>");
  cl.print(lightSchedule[0][1]);
  cl.print("</MondayLightsStartTime1>");
  cl.print("<MondayLightsEndTime1>");
  cl.print(lightSchedule[0][2]);
  cl.print("</MondayLightsEndTime1>");
  cl.print("<MondayLightsStartTime2>");
  cl.print(lightSchedule[0][3]);
  cl.print("</MondayLightsStartTime2>");
  cl.print("<MondayLightsEndTime2>");
  cl.print(lightSchedule[0][4]);
  cl.print("</MondayLightsEndTime2>");
  cl.print("<TuesdayLightsStartTime1>");
  cl.print(lightSchedule[1][1]);
  cl.print("</TuesdayLightsStartTime1>");
  cl.print("<TuesdayLightsEndTime1>");
  cl.print(lightSchedule[1][2]);
  cl.print("</TuesdayLightsEndTime1>");
  cl.print("<TuesdayLightsStartTime2>");
  cl.print(lightSchedule[1][3]);
  cl.print("</TuesdayLightsStartTime2>");
  cl.print("<TuesdayLightsEndTime2>");
  cl.print(lightSchedule[1][4]);
  cl.print("</TuesdayLightsEndTime2>");
  cl.print("<WednesdayLightsStartTime1>");
  cl.print(lightSchedule[2][1]);
  cl.print("</WednesdayLightsStartTime1>");
  cl.print("<WednesdayLightsEndTime1>");
  cl.print(lightSchedule[2][2]);
  cl.print("</WednesdayLightsEndTime1>");
  cl.print("<WednesdayLightsStartTime2>");
  cl.print(lightSchedule[2][3]);
  cl.print("</WednesdayLightsStartTime2>");
  cl.print("<WednesdayLightsEndTime2>");
  cl.print(lightSchedule[2][4]);
  cl.print("</WednesdayLightsEndTime2>");
  cl.print("<ThursdayLightsStartTime1>");
  cl.print(lightSchedule[3][1]);
  cl.print("</ThursdayLightsStartTime1>");
  cl.print("<ThursdayLightsEndTime1>");
  cl.print(lightSchedule[3][2]);
  cl.print("</ThursdayLightsEndTime1>");
  cl.print("<ThursdayLightsStartTime2>");
  cl.print(lightSchedule[3][3]);
  cl.print("</ThursdayLightsStartTime2>");
  cl.print("<ThursdayLightsEndTime2>");
  cl.print(lightSchedule[3][4]);
  cl.print("</ThursdayLightsEndTime2>");
  cl.print("<FridayLightsStartTime1>");
  cl.print(lightSchedule[4][1]);
  cl.print("</FridayLightsStartTime1>");
  cl.print("<FridayLightsEndTime1>");
  cl.print(lightSchedule[4][2]);
  cl.print("</FridayLightsEndTime1>");
  cl.print("<FridayLightsStartTime2>");
  cl.print(lightSchedule[4][3]);
  cl.print("</FridayLightsStartTime2>");
  cl.print("<FridayLightsEndTime2>");
  cl.print(lightSchedule[4][4]);
  cl.print("</FridayLightsEndTime2>");
  cl.print("<SaturdayLightsStartTime1>");
  cl.print(lightSchedule[5][1]);
  cl.print("</SaturdayLightsStartTime1>");
  cl.print("<SaturdayLightsEndTime1>");
  cl.print(lightSchedule[5][2]);
  cl.print("</SaturdayLightsEndTime1>");
  cl.print("<SaturdayLightsStartTime2>");
  cl.print(lightSchedule[5][3]);
  cl.print("</SaturdayLightsStartTime2>");
  cl.print("<SaturdayLightsEndTime2>");
  cl.print(lightSchedule[5][4]);
  cl.print("</SaturdayLightsEndTime2>");
  cl.print("<SundayLightsStartTime1>");
  cl.print(lightSchedule[6][1]);
  cl.print("</SundayLightsStartTime1>");
  cl.print("<SundayLightsEndTime1>");
  cl.print(lightSchedule[6][2]);
  cl.print("</SundayLightsEndTime1>");
  cl.print("<SundayLightsStartTime2>");
  cl.print(lightSchedule[6][3]);
  cl.print("</SundayLightsStartTime2>");
  cl.print("<SundayLightsEndTime2>");
  cl.print(lightSchedule[6][4]);
  cl.print("</SundayLightsEndTime2>");
  cl.print("</inputs>");
}

// sets every element of str to 0 (clears array)
void StrClear(char *str, char length) {
    for (int i = 0; i < length; i++) {
        str[i] = 0;
    }
}

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, char *sfind) {
    char found = 0;
    char index = 0;
    char len;
    len = strlen(str);
    if (strlen(sfind) > len) {
        return 0;
    }
    while (index < len) {
        if (str[index] == sfind[found]) {
            found++;
            if (strlen(sfind) == found) {
                return 1;
            }
        } 
        
        else {
            found = 0;
        }
        index++;
    }
    return 0;
}

void matchAndUpdateLightSchedule(String DOW, char* matcher, char* HTTP_req) {
  // match state object
  MatchState ms;
  char buf [100];
  ms.Target (HTTP_req);  // set its address
  char result = ms.Match (matcher, 0);
  String startTime1 = ms.GetCapture (buf, 0);
  String endTime1 = ms.GetCapture (buf, 1);
  String startTime2 = ms.GetCapture (buf, 2);
  String endTime2 = ms.GetCapture (buf, 3);
  updateLightSchedule(DOW, startTime1, endTime1, startTime2, endTime2);   
}

void updateLightSchedule(String day, String startTime1, String endTime1, String startTime2, String endTime2) {
  int index = getDayIndex(day);
  Serial.println("-------------");
  Serial.println("Updating Schedule for ");
  Serial.print(day);
  lightSchedule[index][1] = startTime1;
  lightSchedule[index][2] = endTime1;
  lightSchedule[index][3] = startTime2;
  lightSchedule[index][4] = endTime2;
  Serial.println("-------------");
}

void checkSchedule() {
  String currentTime = rtc.getTimeStr(FORMAT_SHORT);
  String DOW = rtc.getDOWStr();
  Serial.println("--- Current Time is: " + currentTime);
  int DOWIndex = getDayIndex(DOW);
  String startTime1 = lightSchedule[DOWIndex][1];
  String endTime1 = lightSchedule[DOWIndex][2];
  String startTime2 = lightSchedule[DOWIndex][3];
  String endTime2 = lightSchedule[DOWIndex][4];
  if (startTime1 == currentTime || startTime2 == currentTime) {
    digitalWrite(relay_4, LOW);
  } else if (endTime1 == currentTime || endTime2 == currentTime) {
    digitalWrite(relay_4, HIGH);
  }
}

int getDayIndex(String day) {
  int index = 0;
  if (day == "Monday") {
    index = 0;
  } else if (day == "Tuesday") {
    index = 1;
  } else if (day == "Wednesday") {
    index = 2;
  } else if (day == "Thursday") {
    index = 3;
  } else if (day == "Friday") {
    index = 4;
  } else if (day == "Saturday") {
    index = 5;
  } else if (day == "Sunday") {
    index = 6;
  } else {
    index = 0; // default if can't find day
  }
  return index;
}
