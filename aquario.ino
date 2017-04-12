//////////////////////////////////////////////////////////
//                                                      //
//  Aquario - Open source aquarium management system    //
//  Authors - John Leighton, Leigh Gibbs & Daniel Nix   //
//                                                      //
//////////////////////////////////////////////////////////

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

// Init the DS3231 using the hardware interface
DS3231  rtc(SDA, SCL);

// Init OneWire
OneWire oneWire(ONE_WIRE_BUS);

// Pass OneWire to DallasTemp
DallasTemperature sensors(&oneWire);

// arrays to hold device addresses
DeviceAddress insideThermometer, outsideThermometer;

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

void tempSensorsSetup() {
  sensors.begin();
  if (!sensors.getAddress(insideThermometer, 0)) Serial.println("Unable to find address for Device 0"); 
  if (!sensors.getAddress(outsideThermometer, 1)) Serial.println("Unable to find address for Device 1");
}

void pinSetup() {
  //set digital pins as outputs
  pinMode(5, OUTPUT);  
  pinMode(6, OUTPUT);  
  pinMode(7, OUTPUT);  
  pinMode(8, OUTPUT);
  
  // Initialize Pins so relays are inactive at reset
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(7, HIGH);
  digitalWrite(8, HIGH);  
      
  delay(1000); //Check that all relays are inactive at 
  
  // disable Ethernet chip
  pinMode(10, OUTPUT);
  digitalWrite(10, HIGH);
}

float temp = 30;
float upperTempVariance = 2;
float lowerTempVariance = 5;

void setSetTemp(float temp) {
  temp = temp;
}

void setUpperTempVariance(float upperTempVariance) {
  upperTempVariance = upperTempVariance;
}

void setLowerTempVariance(float lowerTempVariance) {
  lowerTempVariance = lowerTempVariance;
}

// default lightSchedule
String lightSchedule[7][5] {
  {"Monday", "06:00", "10:00", "16:00", "22:00"},
  {"Tuesday", "06:00", "10:00", "16:00", "22:00"},
  {"Wednesday", "06:00", "10:00", "16:00", "22:00"},
  {"Thursday", "06:00", "10:00", "16:00", "22:00"},
  {"Friday", "06:00", "10:00", "16:00", "22:00"},
  {"Saturday", "08:00", "10:00", "16:00", "23:00"},
  {"Sunday", "08:00", "10:00", "16:00", "23:00"},
};

// default co2Schedule
String co2Schedule[7][5] {
  {"Monday", "06:00", "10:00", "16:00", "22:00"},
  {"Tuesday", "06:00", "10:00", "16:00", "22:00"},
  {"Wednesday", "06:00", "10:00", "16:00", "22:00"},
  {"Thursday", "06:00", "10:00", "16:00", "22:00"},
  {"Friday", "06:00", "10:00", "16:00", "22:00"},
  {"Saturday", "08:00", "10:00", "16:00", "23:00"},
  {"Sunday", "08:00", "10:00", "16:00", "23:00"},
};

void setup() {  
  // Pin setup
  pinSetup();
  // Ethernet setup
  ethernetSetup();
  // SD card setup
  sdCardSetup();
  // temp setup
  tempSensorsSetup();  
  // Setup Serial connection
  Serial.begin(9600);
  // Initialize the rtc object
  rtc.begin();
}

void loop() {
  serveWebpage();
  checkSchedule(lightSchedule, 5);
  checkSchedule(co2Schedule, 6);
//  checkTemp(insideThermometer);
  delay(1000);
}

void checkTemp(DeviceAddress deviceAddress) {
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println(sensors.getTempCByIndex(0));
  if (sensors.getTempCByIndex(0) >= temp + upperTempVariance) {
    Serial.print("ALARM!! turning heater off.");
    turnRelayOff(5, "Heater");
  } else if (sensors.getTempCByIndex(0) <= temp - lowerTempVariance) {
    Serial.print("ALARM!! turning heater on.");
    turnRelayOn(5, "Heater");
  }
}

void turnRelayOn(int relay_num, String relay_name) {
  digitalWrite(relay_num, HIGH);// set the Relay ON
  Serial.print(relay_name + " ON");
  Serial.println("");
}

void turnRelayOff(int relay_num, String relay_name) {
  digitalWrite(relay_num, LOW);// set the Relay OFF
  Serial.print(relay_name + " OFF");
  Serial.println("");
}

void checkSchedule(String schedule[7][5], int relay) {
  String currentTime = rtc.getTimeStr(FORMAT_SHORT);
  String DOW = rtc.getDOWStr();
  Serial.println("--- Current Time is: " + currentTime);
  int DOWIndex = getDayIndex(DOW);
  String startTime1 = schedule[DOWIndex][1];
  String endTime1 = schedule[DOWIndex][2];
  String startTime2 = schedule[DOWIndex][3];
  String endTime2 = schedule[DOWIndex][4];
  if (startTime1 == currentTime || startTime2 == currentTime) {
    digitalWrite(relay, LOW);
  } else if (endTime1 == currentTime || endTime2 == currentTime) {
    digitalWrite(relay, HIGH);
  }
}

int getDayIndex(String day) {
  int index = 0;
  if (day == "Tuesday") {
    index = 1;
  } 
  else if (day == "Wednesday") {
    index = 2;
  } 
  else if (day == "Thursday") {
    index = 3;
  } 
  else if (day == "Friday") {
    index = 4;
  } 
  else if (day == "Saturday") {
    index = 5;
  } 
  else if (day == "Sunday") {
    index = 6;
  } 
  else {
    index = 0; // default to 0 for Monday
  }
  return index;
}

void serveWebpage() {
  File webFile; // the web page file on the SD card
  #define REQ_BUF_SZ 100 // size of buffer used to capture HTTP requests
  char HTTP_req[REQ_BUF_SZ] = {0}; // buffered HTTP request stored as null terminated string
  char req_index = 0; // index into HTTP_req buffer
  
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
                    HTTP_req[req_index] = c; // save HTTP request character
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
                      if (StrContains(HTTP_req, "aquario_data")) {
                        // send rest of HTTP header
                        client.println("Content-Type: text/xml");
                        client.println("Connection: keep-alive");
                        client.println();
                        if (StrContains(HTTP_req, "&hr")) {
                          matchAndUpdateAquarioData("currentTime", "&hr=(%d+)&min=(%d+)&sec=(%d+)", HTTP_req);
                        }
                        if (StrContains(HTTP_req, "&day")) {
                          matchAndUpdateAquarioData("currentDate", "&day=(%d+)&mon=(%d+)&year=(%d+)", HTTP_req);
                        }
                        if (StrContains(HTTP_req, "&tt")) {
                          matchAndUpdateAquarioData("tankTemps", "&tt=(%d+)&tuv=(%d+)&tlv=(%d+)", HTTP_req);
                        }
                        XML_response_AquarioData(client);
                      }
                      else if (StrContains(HTTP_req, "lights_schedules")) {
                        // send rest of HTTP header
                        client.println("Content-Type: text/xml");
                        client.println("Connection: keep-alive");
                        client.println();
  
                        if (StrContains(HTTP_req, "&mst1")) {
                          matchAndUpdateSchedule(lightSchedule, "Monday", "&mst1=(%d+:%d+)&met1=(%d+:%d+)&mst2=(%d+:%d+)&met2=(%d+:%d+)", HTTP_req);
                        }
                        if (StrContains(HTTP_req, "&tst1")) {
                          matchAndUpdateSchedule(lightSchedule, "Tuesday", "&tst1=(%d+:%d+)&tet1=(%d+:%d+)&tst2=(%d+:%d+)&tet2=(%d+:%d+)", HTTP_req);                     
                        }
                        if (StrContains(HTTP_req, "&wst1")) {
                          matchAndUpdateSchedule(lightSchedule, "Wednesday", "&wst1=(%d+:%d+)&wet1=(%d+:%d+)&wst2=(%d+:%d+)&wet2=(%d+:%d+)", HTTP_req);                       
                        }
                        if (StrContains(HTTP_req, "&thst1")) {
                          matchAndUpdateSchedule(lightSchedule, "Thursday", "&thst1=(%d+:%d+)&thet1=(%d+:%d+)&thst2=(%d+:%d+)&thet2=(%d+:%d+)", HTTP_req);                      
                        }
                        if (StrContains(HTTP_req, "&fst1")) {
                          matchAndUpdateSchedule(lightSchedule, "Friday", "&fst1=(%d+:%d+)&fet1=(%d+:%d+)&fst2=(%d+:%d+)&fet2=(%d+:%d+)", HTTP_req);                  
                        }
                        if (StrContains(HTTP_req, "&sst1")) {
                          matchAndUpdateSchedule(lightSchedule, "Saturday", "&sst1=(%d+:%d+)&set1=(%d+:%d+)&sst2=(%d+:%d+)&set2=(%d+:%d+)", HTTP_req);                     
                        }
                        if (StrContains(HTTP_req, "&sust1")) {
                          matchAndUpdateSchedule(lightSchedule, "Sunday", "&sust1=(%d+:%d+)&suet1=(%d+:%d+)&sust2=(%d+:%d+)&suet2=(%d+:%d+)", HTTP_req);                        
                        }
                        XML_response_Schedules(client, lightSchedule); 
                      }
                      else if (StrContains(HTTP_req, "co2_schedules")) {
                        // send rest of HTTP header
                        client.println("Content-Type: text/xml");
                        client.println("Connection: keep-alive");
                        client.println();
  
                        if (StrContains(HTTP_req, "&co2mst1")) {
                          matchAndUpdateSchedule(co2Schedule, "Monday", "co2&mst1=(%d+:%d+)&co2met1=(%d+:%d+)&co2mst2=(%d+:%d+)&co2met2=(%d+:%d+)", HTTP_req);
                        }
                        if (StrContains(HTTP_req, "&co2tst1")) {
                          matchAndUpdateSchedule(co2Schedule, "Tuesday", "&co2tst1=(%d+:%d+)&co2tet1=(%d+:%d+)&co2tst2=(%d+:%d+)&co2tet2=(%d+:%d+)", HTTP_req);                   
                        }
                        if (StrContains(HTTP_req, "&co2wst1")) {
                          matchAndUpdateSchedule(co2Schedule, "Wednesday", "&co2wst1=(%d+:%d+)&co2wet1=(%d+:%d+)&co2wst2=(%d+:%d+)&co2wet2=(%d+:%d+)", HTTP_req);                        
                        }
                        if (StrContains(HTTP_req, "&thst1")) {
                          matchAndUpdateSchedule(co2Schedule, "Thursday", "&co2thst1=(%d+:%d+)&co2thet1=(%d+:%d+)&co2thst2=(%d+:%d+)&co2thet2=(%d+:%d+)", HTTP_req);                    
                        }
                        if (StrContains(HTTP_req, "&co2fst1")) {
                          matchAndUpdateSchedule(co2Schedule, "Friday", "&co2fst1=(%d+:%d+)&co2fet1=(%d+:%d+)&co2fst2=(%d+:%d+)&co2fet2=(%d+:%d+)", HTTP_req);                   
                        }
                        if (StrContains(HTTP_req, "&co2sst1")) {
                          matchAndUpdateSchedule(co2Schedule, "Saturday", "&co2sst1=(%d+:%d+)&co2set1=(%d+:%d+)&co2sst2=(%d+:%d+)&co2set2=(%d+:%d+)", HTTP_req);                    
                        }
                        if (StrContains(HTTP_req, "&co2sust1")) {
                          matchAndUpdateSchedule(co2Schedule, "Sunday", "&co2sust1=(%d+:%d+)&co2suet1=(%d+:%d+)&co2sust2=(%d+:%d+)&co2suet2=(%d+:%d+)", HTTP_req);                        
                        }
                        XML_response_Schedules(client, co2Schedule); 
                    } 
                    
                    else {  // web page request
                        // send rest of HTTP header
                        client.println("Content-Type: text/html");
                        client.println("Connection: keep-alive");
                        client.println();

                        if (StrContains(HTTP_req, "GET /lights.htm")) {
                          webFile = SD.open("lights.htm");
                          serveWebFileAndClose(webFile, client);
                        }

                        else if (StrContains(HTTP_req, "GET /co2.htm")) {
                          webFile = SD.open("co2.htm");
                          serveWebFileAndClose(webFile, client);
                        }

                        else {
                          webFile = SD.open("state.htm");
                          serveWebFileAndClose(webFile, client);
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
        delay(1000); // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)
}

void serveWebFileAndClose(File webFile, EthernetClient client) {
  if (webFile) {
    while(webFile.available()) {
      client.write(webFile.read()); // send web page to client
    }
    webFile.close();
  }
}

void XML_response_AquarioData(EthernetClient cl) {
  sensors.requestTemperatures();    
  cl.print("<?xml version = \"1.0\" ?>");
  cl.print("<inputs>");
  cl.print("<tankTemperature>");
  cl.print(sensors.getTempCByIndex(0));
  cl.print("</tankTemperature>");
  cl.print("<currentTime>");
  cl.print(rtc.getTimeStr(FORMAT_SHORT));
  cl.print("</currentTime>");
  cl.print("<currentDate>");
  cl.print(rtc.getDateStr());
  cl.print("</currentDate>");
  cl.print("<currentDay>");
  cl.print(rtc.getDOWStr());
  cl.print("</currentDay>");
  cl.print("</inputs>");
}

void XML_response_Schedules(EthernetClient cl, String schedule[7][5]) {
  cl.print("<?xml version = \"1.0\" ?>");
  cl.print("<inputs>");
  cl.print("<MondayStartTime1>");
  cl.print(schedule[0][1]);
  cl.print("</MondayStartTime1>");
  cl.print("<MondayEndTime1>");
  cl.print(schedule[0][2]);
  cl.print("</MondayEndTime1>");
  cl.print("<MondayLightsStartTime2>");
  cl.print(schedule[0][3]);
  cl.print("</MondayStartTime2>");
  cl.print("<MondayEndTime2>");
  cl.print(schedule[0][4]);
  cl.print("</MondayEndTime2>");
  cl.print("<TuesdayStartTime1>");
  cl.print(schedule[1][1]);
  cl.print("</TuesdayStartTime1>");
  cl.print("<TuesdayEndTime1>");
  cl.print(schedule[1][2]);
  cl.print("</TuesdayEndTime1>");
  cl.print("<TuesdayStartTime2>");
  cl.print(schedule[1][3]);
  cl.print("</TuesdayStartTime2>");
  cl.print("<TuesdayEndTime2>");
  cl.print(schedule[1][4]);
  cl.print("</TuesdayEndTime2>");
  cl.print("<WednesdayStartTime1>");
  cl.print(schedule[2][1]);
  cl.print("</WednesdayStartTime1>");
  cl.print("<WednesdayEndTime1>");
  cl.print(schedule[2][2]);
  cl.print("</WednesdayEndTime1>");
  cl.print("<WednesdayStartTime2>");
  cl.print(schedule[2][3]);
  cl.print("</WednesdayStartTime2>");
  cl.print("<WednesdayEndTime2>");
  cl.print(schedule[2][4]);
  cl.print("</WednesdayEndTime2>");
  cl.print("<ThursdayStartTime1>");
  cl.print(schedule[3][1]);
  cl.print("</ThursdayStartTime1>");
  cl.print("<ThursdayEndTime1>");
  cl.print(schedule[3][2]);
  cl.print("</ThursdayEndTime1>");
  cl.print("<ThursdayStartTime2>");
  cl.print(schedule[3][3]);
  cl.print("</ThursdayStartTime2>");
  cl.print("<ThursdayEndTime2>");
  cl.print(schedule[3][4]);
  cl.print("</ThursdayEndTime2>");
  cl.print("<FridayStartTime1>");
  cl.print(schedule[4][1]);
  cl.print("</FridayStartTime1>");
  cl.print("<FridayEndTime1>");
  cl.print(schedule[4][2]);
  cl.print("</FridayEndTime1>");
  cl.print("<FridayStartTime2>");
  cl.print(schedule[4][3]);
  cl.print("</FridayStartTime2>");
  cl.print("<FridayEndTime2>");
  cl.print(schedule[4][4]);
  cl.print("</FridayEndTime2>");
  cl.print("<SaturdayStartTime1>");
  cl.print(schedule[5][1]);
  cl.print("</SaturdayStartTime1>");
  cl.print("<SaturdayEndTime1>");
  cl.print(schedule[5][2]);
  cl.print("</SaturdayEndTime1>");
  cl.print("<SaturdayStartTime2>");
  cl.print(schedule[5][3]);
  cl.print("</SaturdayStartTime2>");
  cl.print("<SaturdayEndTime2>");
  cl.print(schedule[5][4]);
  cl.print("</SaturdayEndTime2>");
  cl.print("<SundayStartTime1>");
  cl.print(schedule[6][1]);
  cl.print("</SundayStartTime1>");
  cl.print("<SundayEndTime1>");
  cl.print(schedule[6][2]);
  cl.print("</SundayEndTime1>");
  cl.print("<SundayStartTime2>");
  cl.print(schedule[6][3]);
  cl.print("</SundayStartTime2>");
  cl.print("<SundayEndTime2>");
  cl.print(schedule[6][4]);
  cl.print("</SundayEndTime2>");
  cl.print("</inputs>");
}

void StrClear(char *str, char length) {
  for (int i = 0; i < length; i++) {
    str[i] = 0;
  }
}

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

void matchAndUpdateSchedule(String schedule[7][5], String DOW, char* matcher, char* HTTP_req) {
  // match state object
  MatchState ms;
  char buf [100];
  ms.Target (HTTP_req);  // set its address
  char result = ms.Match (matcher, 0);
  String startTime1 = ms.GetCapture (buf, 0);
  String endTime1 = ms.GetCapture (buf, 1);
  String startTime2 = ms.GetCapture (buf, 2);
  String endTime2 = ms.GetCapture (buf, 3);
  updateSchedule(schedule, DOW, startTime1, endTime1, startTime2, endTime2);   
}

void updateSchedule(String schedule[7][5], String day, String startTime1, String endTime1, String startTime2, String endTime2) {
  int index = getDayIndex(day);
  Serial.println("-------------");
  Serial.println("Updating schedule for ");
  Serial.print(day);
  schedule[index][1] = startTime1;
  schedule[index][2] = endTime1;
  schedule[index][3] = startTime2;
  schedule[index][4] = endTime2;
  Serial.println("-------------");
}

void matchAndUpdateAquarioData(String dataToUpdate, char* matcher, char* HTTP_req) {
  // match state object
  MatchState ms;
  char buf [100];
  ms.Target (HTTP_req);  // set its address
  char result = ms.Match (matcher, 0);
  int firstMatch = atoi(ms.GetCapture (buf, 0));
  int secondMatch = atoi(ms.GetCapture (buf, 1));
  int thirdMatch = atoi(ms.GetCapture (buf, 2));

  if (dataToUpdate == "currentTime") {
    rtc.setTime(firstMatch, secondMatch, thirdMatch);
  }
  else if (dataToUpdate == "currentDate") {
    rtc.setDate(firstMatch, secondMatch, thirdMatch);
    rtc.setDOW();
  }
  else if (dataToUpdate == "tankTemps") {
    setSetTemp(firstMatch);
    setUpperTempVariance(secondMatch);
    setLowerTempVariance(thirdMatch);
  }
}
