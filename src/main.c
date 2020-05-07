/*
 Garden Control System 1.0
 This uses an Arduino MKR WIFI 1010
 If you're using a different device, you'll want to change some of the code, specifically around Wifi, Server, and NTP
 If you're using a non-network device, you'll want to add code (and hardware like buttons and a screen) to set the system's clock (for timing, lights, etc)
 
*/

#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
#include <RTCZero.h>
#include <utility/wifi_drv.h>
#include "arduino_secrets.h"
#include "constants_and_pins.h"


//Global Variables 
unsigned long health_check = 0;
bool primeMode = false;
unsigned long primeEpoch = 0;
unsigned long phEpoch = 0;
float sysPh = 0.0; 

// PH Voltage conversion (pin voltage / resolution) 
float vConversion = (sysVoltage/adcRes); 

// Lights, Airation, Pump
bool statusArray[3] = {false, false, false};


// Template defs
void printWifiStatus();
void setRtc(unsigned long epoc);
unsigned long getEpoc();
unsigned long sendNTPpacket(IPAddress& address);

// NTP and WIFI Stuff...
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)
// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;
unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServer(129, 6, 15, 28); // time.nist.gov NTP server
int status = WL_IDLE_STATUS;
WiFiServer server(80);
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets


// Create a RTC instance
RTCZero rtc;


void setup() {
  // Setup Pins
  setupPins();

  Serial.begin(9600);

  initializeWifi();
  
  // Enable UDP for NTP query
  Udp.begin(localPort);
  // Turn on the real-time clock
  rtc.begin();

  setTimeFromNTP();

  server.begin();

  // We're Up!
  setRGBLed(0, 64, 0);
  digitalWrite(LED_BUILTIN, HIGH);
}


void setupPins() {
  pinMode(lightPin, OUTPUT);
  pinMode(airPin, OUTPUT);
  pinMode(pumpPin, OUTPUT);
  pinMode(purgePin, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(phDownPin, OUTPUT);
  pinMode(phUpPin, OUTPUT);
  pinMode(buttonPin, INPUT);
  pinMode(phMeasurePin, INPUT); 
  digitalWrite(lightPin, PINOFF);
  digitalWrite(airPin, PINOFF);
  digitalWrite(pumpPin, PINOFF);
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(phDownPin, PINOFF);
  digitalWrite(phUpPin, PINOFF);
  digitalWrite(purgePin, PINOFF);
}


void initializeWifi() {
  WiFi.setHostname(hostname);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(500);
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
  WiFiDrv::pinMode(25, OUTPUT);  //GREEN
  WiFiDrv::pinMode(26, OUTPUT);  //RED
  WiFiDrv::pinMode(27, OUTPUT);  //BLUE

  setRGBLed(0, 0, 64);


  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    setRGBLed(0, 0, 16);
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 5 seconds for connection:
    for (int i = 0; i <= 5; i++) {
      setRGBLed(0, 0, 64);
      delay(500);
      setRGBLed(0, 0, 16);
      delay(500);
    }
  }

  Serial.println("Connected to wifi");
  printWifiStatus();
  setRGBLed(64, 64, 0);
}

void setTimeFromNTP() {
  Serial.println("\nStarting connection to server...");
  

  unsigned long epoch = getEpoc();
  if (epoch == 0) {
    Serial.println("\n Error getting epoch from NTP\n");
    setRGBLed(128, 0, 0);
  } else {
    Serial.println("Setting RTC...\n");
    setRtc(epoch);
  }
}

void setRtc(unsigned long epoch) {
  
  Serial.print("Epoch received: ");
  Serial.println(epoch);
  rtc.setEpoch(epoch);
  Serial.println();
}

unsigned long getEpoc() {
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  if (Udp.parsePacket()) {
    Serial.println("packet received");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = ");
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    return epoch;
  } else {
    return 0;
  }
}

void loop() {
  checkTest();
  checkHelth();
  doPhCheck();
  pollStatus();
  checkPrime();
  setPins();
  processRequest();
  delay(10);
}

void printTime()
{
  print2digits(rtc.getHours());
  Serial.print(":");
  print2digits(rtc.getMinutes());
  Serial.print(":");
  print2digits(rtc.getSeconds());
  Serial.println();
}

// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address) {
  //Serial.println("1");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  //Serial.println("2");
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  //Serial.println("3");

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  //Serial.println("4");
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  //Serial.println("5");
  Udp.endPacket();
  //Serial.println("6");
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void print2digits(int number) {
  if (number < 10) {
    Serial.print("0"); // print a 0 before if the number is < than 10
  }
  Serial.print(number);
}

void setPins() {
  //Serial.println("\nSetting System to the following state:\n");
  //printStatus();
  runLights(statusArray[0]);
  runAir(statusArray[1]);
  runPump(statusArray[2]);
}

void printStatus() {
  Serial.print("Lights: ");
  Serial.print(onOffText(statusArray[0]));
  Serial.print("Air: ");
  Serial.print(onOffText(statusArray[1]));
  Serial.print("Pump: ");
  Serial.print(onOffText(statusArray[2]));
  Serial.println("\n");

}

String onOffText(bool val) {
  if (val) {
    return "On\n";
  } else {
    return "Off\n";
  }
}

String stringify(bool val) {
  if (val) {
    return "true";
  } else {
    return "false";
  }
}
void runAir(bool on) {
  if (on) {
    digitalWrite(airPin, PINON);
  } else {
    digitalWrite(airPin, PINOFF);
  }
}

void runPump(bool on) {
  byte seconds = rtc.getSeconds();
  if (on) {
    if (seconds < PUMP_PULSE_LENGTH) {
      digitalWrite(pumpPin, PINON);
    } else {
      digitalWrite(pumpPin, PINOFF);
    }
  } else {
    digitalWrite(pumpPin, PINOFF);
  }
}

void runLights(bool on) {
  if (on) {
    digitalWrite(lightPin, PINON);
  } else {
    digitalWrite(lightPin, PINOFF);
  }
}

void pollStatus() {
  byte hour = rtc.getHours();
  byte minute = rtc.getMinutes();
  if (hour == LIGHT_OFF_HOURS && minute >= LIGHT_OFF_MINUTES) {
    setLights(false);
  } else if (hour == LIGHT_OFF_HOURS) {
    if (minute <= LIGHT_OFF_MINUTES){
      setLights(true);
    }
  }
  if ((hour >= LIGHT_ON_HOURS) || (hour >= LIGHT_ON_HOURS && minute >= LIGHT_ON_MINUTES )) {
    setLights(true);
  }

  if (
    (minute >= SYSTEM_RUN_OFFSET && minute <= (SYSTEM_RUN_OFFSET + SYSTEM_RUN_LENGTH + AIR_BUFFER)) || (minute >= AIR_RUN_ADDL && minute <= (AIR_RUN_ADDL + AIR_RUN_ADDL_LEN))) {
    setAir(true);
  } else {
    setAir(false);
  }

  if ((minute >= (SYSTEM_RUN_OFFSET + AIR_BUFFER)) && (minute <= (SYSTEM_RUN_LENGTH + SYSTEM_RUN_OFFSET))) {
    setPump(true);
  } else {
    setPump(false);
  }
}

void processRequest() {
  // listen for incoming clients
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    String currentLine = "";
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n') {
          if (currentLine.length() == 0) {
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/json");
            client.println("Connection: close");  // the connection will be closed after completion of the response
            client.println("Refresh: 10");  // refresh the page automatically every 5 sec
            client.println();
            client.print((String)"{\n  \"state\":{\"lights\":" += stringify(statusArray[0]) += (String)", \"air_pump\":" += stringify(statusArray[1]) += (String)", \"fluid_pump\":" += stringify(statusArray[2])  += (String)", \"pH\":" += (String)sysPh += (String)"},\n");
            client.print((String)"  \"systemtime\":{\"hours\":" += (String)rtc.getHours() += (String)", \"minutes\":" += (String)rtc.getMinutes() += (String)", \"seconds\":" += (String)rtc.getSeconds() += (String)"},\n");
            client.print((String)"  \"uptime\":{\"system\":" += (String)millis() += (String)", \"health_counter\":" += (String)health_check += (String)"}\n}\n");

            client.println("\n");
            break;
          } else {
            // you're starting a new line
            currentLine = "";
          }
        }

        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLine += c;
        }
        if (currentLine.endsWith("GET /T")) {
          test();
        }
        if (currentLine.endsWith("GET /PRIME")) {
          primeSystem();
        }
        if (currentLine.endsWith("GET /PURGE")) {
          doPurge();
        }
      }
    }
    // give the web browser time to receive the data
    delay(10);

    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
}

void checkTest() {
  int count = 0;
  int limit = 10;
  /*
    if (digitalRead(buttonPin) == LOW) {
    Serial.println("Test mode entered, hold to prime system...\n");
    setRGBLed(0,45,56);
    while (digitalRead(buttonPin) == LOW && count <= limit){
      count++;
      delay(1000);
    }
    if (count > 1 && count < limit/2){
      test();
    }
    if (count >= limit) {
      primeSystem();
    }
    Serial.println("Returning to normalcy...\n");
    setRGBLed(0,64,0);
    } */
}
void test() {
  setRGBLed(0, 64, 64);
  setStatus(false, false, false);
  setPins();
  Serial.println("Test Routine On:\n");
  Serial.println("Lights...\n");
  runLights(true);
  delay(5000);
  runLights(false);
  Serial.println("Air Pump...\n");
  runAir(true);
  delay(5000);
  runAir(false);
  Serial.println("Fluid Pump...\n");
  runPump(true);
  delay(5000);
  runPump(false);
  Serial.println("Complete.\n");
  setRGBLed(0, 64, 0);
}


void setRGBLed(int r, int g, int b) {
  WiFiDrv::analogWrite(25, g);  // for configurable brightness
  WiFiDrv::analogWrite(26, r);  // for configurable brightness
  WiFiDrv::analogWrite(27, b);  // for configurable brightness
}

void primeSystem() {
  setRGBLed(96, 32, 96);
  primeEpoch = rtc.getEpoch() + PRIME_LENGTH;
  primeMode = true;
}

void checkPrime() {
  if (primeMode) {
    setStatus(true, true, true);
    if (rtc.getEpoch() > primeEpoch) {
      primeEpoch = 0;
      primeMode = false;
      setStatus(true, false, false);
    }
  }
}

void halt() {
  setStatus(false, false, false);
}

void setStatus(bool lights, bool airPump, bool fluidPump) {
  if (lights != NULL) {
    statusArray[0] = lights;
  }
  if (airPump != NULL) {
    statusArray[1] = airPump;
  }
  if (fluidPump != NULL) {
    statusArray[2] = fluidPump;
  }
}

void setPump(bool pump){
  statusArray[2] = pump;
}
void setAir(bool air){
  statusArray[1] = air;
}
void setLights(bool lights){
  statusArray[0] = lights;
}

void checkHelth() {
  health_check++;
  if (health_check > HEALTH_LIMIT) {
    doHealthCleanup();
  }

}

void doHealthCleanup() {

  unsigned long epoch = getEpoc();
  if (epoch == 0) {
    Serial.println("\n Error getting epoch from NTP\n");
    setRGBLed(128, 0, 0);
    delay(1000);
  } else {
    setRtc(epoch);
    health_check=0;
  }
}

float getPh() {
  int measurementValue = sampleMeasurementAvg(analogSampleSize, phMeasurePin);
  float adjustedValue = measurementValue * vConversion;
  Serial.println("Measured analog input with average value: " + (String)measurementValue + " and adjusted value of: " + (String)adjustedValue + "\n");
  // pH = adjustedValue * phSensorVoltageScalar; 
  return (float)7.0; 
}

int sampleMeasurementAvg(int sampleSize, int samplePin){
  int measurementArray[sampleSize];
  int sum = 0;
  int avg = 0;
  for (int i = 0; i < sampleSize; i++){
    measurementArray[i] = analogRead(samplePin);
    delay(25);
  }
  // Sum the array, discarding the first N value
  for (int j = discardFirstN; j < sampleSize; j++) {
    sum += measurementArray[j];
  }
  avg = (sum / (sampleSize - discardFirstN)); 
  return avg;
  
}


void doPhCheck() {
  sysPh = getPh();
  Serial.println("Got Ph of: " + (String)sysPh + "\n");
  if ( phEpoch + phUpdateFreq < rtc.getEpoch() ) {
    phAdjust(sysPh);
  }
}

void phAdjust(float ph){
    if ( ph - phThreshold > idealPh ) {
      doPhDown();
      phEpoch = rtc.getEpoch();
      Serial.println("Adjusted pH DOWN \n");
    } else if ( ph + phThreshold < idealPh ) {
      doPhUp();
      phEpoch = rtc.getEpoch();
      Serial.println("Adjusted pH UP \n");
    }
 }

void doPhUp(){
    digitalWrite(phUpPin, PINON);
    delay(500);
    digitalWrite(phUpPin, PINOFF);
}

void doPhDown(){
    digitalWrite(phDownPin, PINON);
    delay(500);
    digitalWrite(phDownPin, PINOFF);
}

void doPurge(){
    digitalWrite(purgePin, PINON);
    delay(20000);
    digitalWrite(purgePin, PINOFF);
}
