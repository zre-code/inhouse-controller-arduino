#include <TimerOne.h>
#include "WiFiEsp.h"
#include <Keypad.h>
//#include <WiFi101.h>
#define pl 20
#include <ArduinoJson.h>
// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(A0, A1); // RX, TX
#endif
char ssid[] = "EnFBR#Zd3z47r3jhobPxj6%UX1vusLI1";            // your network SSID (name)
char pass[] = "KVArXhpTqqQOJ*WJ$dqF*7OKyeI1$NLQt&$o1!NcoHx58!gS2vgery4uRFj345r";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
int passlen = 13;
int passlen1;
int red = 12;
int blue = 10;
int green = 11;
int buzzer = 13;
int interruptCounter = 0;
//int door = 9;
int code;
String initStat = "";
String usestat = "";
char Data[pl];
const char* statusresult;
char ID[] = "TINY.KEYP.proto";
byte data_count = 0;
byte mac[6];
bool Pass_is_good;
char customKey;
const byte ROWS = 4;
const byte COLS = 3;
char keys[ROWS][COLS] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
byte rowPins[ROWS] = {
  8, 7, 6, 5
};
byte colPins[COLS] = {
  4, 3, 2
};
Keypad customKeypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS);
char server[] = "10.2.1.8";

unsigned long lastConnectionTime = 0;         // last time you connected to the server, in milliseconds
const unsigned long postingInterval = 10000L; // delay between updates, in milliseconds

// Initialize the Ethernet client object
WiFiEspClient client;
//WiFiSSLClient client;
void setup()
{

  // initialize serial for debugging
  Serial.begin(9600);
  // initialize serial for ESP module
  Serial1.begin(9600);
  // initialize ESP module
  WiFi.init(&Serial1);
  //pinMode(5, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(buzzer, OUTPUT);
  digitalWrite(blue, LOW);
  digitalWrite(red, LOW);
  digitalWrite(green, LOW);
  digitalWrite(buzzer, HIGH);
  Serial.println("KEYPAD SYSTEM");
  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Timer1.initialize();

  Serial.println("You're connected to the network");

  printWifiStatus();
  httpInit();
}


void interruptCallback() {
  if (interruptCounter == 2) {
    data_count = 0;
    interruptCounter = 0;
    initialStatus();
    clearData();
    Serial.println("Changed to white now");
    Timer1.detachInterrupt();
    Serial.println("Timer Detached");
  }
  else {
    interruptCounter++;
  }
}

void loop()
{
  digitalWrite(buzzer, HIGH);
  // if there's incoming data from the net connection send it out the serial port
  // this is for debugging purposes only
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }

  // if 10 seconds have passed since your last connection,
  // then connect again and send data
  if (millis() - lastConnectionTime > postingInterval) {

  }
  if (data_count < passlen - 1)
  {
    digitalWrite(buzzer, HIGH);
    customKey = customKeypad.getKey();
    if (data_count == 0 && customKeypad.isPressed(customKey))
    {
      Timer1.attachInterrupt(interruptCallback, 5000000);
      Serial.println("Timer Attached");
    }




    if (customKey)
    {
      //digitalWrite(buzzer, LOW);
      tone(buzzer, 1200, 50);
      digitalWrite(buzzer, HIGH);
      digitalWrite(green, HIGH);
      digitalWrite(red, HIGH);
      digitalWrite(blue, LOW);
      Data[data_count] = customKey; // store char into data array
      Serial.print("Current input: ");
      Serial.println(Data[data_count]); // print char at said cursor
      data_count++; // increment data array by 1 to store new char, also keep track of the number of chars entered
    }
  }


  if (Data[0] == '*')
  {
    passlen = 13;
    if (data_count == passlen - 1) {
      Timer1.detachInterrupt();
      Serial.println("Timer detached");
      httpInit();
    }
  }

  else
  {
    passlen = 7;
    if (data_count == passlen - 1) {
      Timer1.detachInterrupt();
      Serial.println("Timer detached");
      httpRequest();
    }

  }
}


// this method makes a HTTP connection to the server



void httpInit() {
  
    noTone(buzzer);
    digitalWrite(blue, HIGH);
    digitalWrite(buzzer, HIGH);
    //  client.stop();
    Serial.println("HTTP INIT");
    // if there's a successful connection
    if (client.connect(server, 80)) {
      digitalWrite(green, LOW);
      digitalWrite(red, LOW);
      Serial.println("");
      Serial.println("Connecting...");
      client.print(F("GET /api/v1/closed/access/pad/use"));
      client.print("?ip=");
      client.print(WiFi.localIP());
      {
        WiFi.macAddress(mac);
        client.print("&mac=");
        client.print(mac[5], HEX);
        client.print(":");
        client.print(mac[4], HEX);
        client.print(":");
        client.print(mac[3], HEX);
        client.print(":");
        client.print(mac[2], HEX);
        client.print(":");
        client.print(mac[1], HEX);
        client.print(":");
        client.print(mac[0], HEX);
      }
      client.print("&action=register");
      client.print("&code=");
      client.print(Data);
      client.println();

      Serial.print("passlen  ");
      Serial.println(passlen);
      // note the time that the connection was made
      lastConnectionTime = millis();
      char status[132] = {0};
      client.readBytesUntil('\r', status, sizeof(status));
      if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
        Serial.print(F("Unexpected response: "));
        Serial.println(status);
        clearData();
      }
      char endOfHeaders[] = "\r\n\r\n";
      if (!client.find(endOfHeaders)) {
        Serial.println(F("Invalid response"));

      }
      const size_t bufferSize = JSON_OBJECT_SIZE(2) + 30;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      const char* json = "{\"passlength\":4,\"status\":error}";
      JsonObject& root = jsonBuffer.parseObject(status);
      passlen1 = root["length"];
      const char* statusresult = root["status"];
      initStat = statusresult;
      if (initStat == "success")
      {
        passlen = passlen1 + 1;
        Serial.println ("check equaled");
        digitalWrite(green, HIGH);
        digitalWrite(red, HIGH);
        validPass();
      }
      else
      {
        digitalWrite(green, HIGH);
        digitalWrite(red, HIGH);
        badPass();
      }
    }
    else{
      Serial.println("No connection fam");
    }
    clearData();
}
void httpRequest() {
  noTone(buzzer);
  digitalWrite(blue, HIGH);
  digitalWrite(buzzer, HIGH);
  Serial.println("HTTP REQUEST");
  if (data_count == passlen - 1 )
  {
    client.stop();
    // if there's a successful connection
    if (client.connect(server, 80)) {
      digitalWrite(green, LOW);
      digitalWrite(red, LOW);
      Serial.println("");
      Serial.println("Connecting...");
      client.print(F("GET /api/v1/closed/access/pad/use"));
      client.print("?ip=");
      client.print(WiFi.localIP());
      {
        WiFi.macAddress(mac);
        client.print("&mac=");
        client.print(mac[5], HEX);
        client.print(":");
        client.print(mac[4], HEX);
        client.print(":");
        client.print(mac[3], HEX);
        client.print(":");
        client.print(mac[2], HEX);
        client.print(":");
        client.print(mac[1], HEX);
        client.print(":");
        client.print(mac[0], HEX);
      }
      client.print("&action=use");
      client.print("&code=");
      client.print(Data);
      client.println();
      Serial.print("passlen  ");
      Serial.println(passlen);
      // note the time that the connection was made
      lastConnectionTime = millis();
      char status[132] = {0};
      client.readBytesUntil('\r', status, sizeof(status));
      if (strcmp(status, "HTTP/1.1 200 OK") != 0) {
        Serial.print(F("Unexpected response: "));
        Serial.println(status);
        clearData();
      }
      char endOfHeaders[] = "\r\n\r\n";
      if (!client.find(endOfHeaders)) {
        Serial.println(F("Invalid response"));

      }
      const size_t bufferSize = JSON_OBJECT_SIZE(2) + 30;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.parseObject(status);
      passlen1 = root["length"];
      passlen = passlen1 + 1;
      const char* statusresultuse = root["status"];
      usestat = statusresultuse;
      if (usestat == "error")
      {
        digitalWrite(green, HIGH);
        digitalWrite(red, HIGH);
        badPass();
      }
      if (usestat == "success")
      {
        digitalWrite(green, HIGH);
        digitalWrite(red, HIGH);
        validPass();
      }
    }
    else {
      Serial.println("No connection fam");
    }

    clearData();
  }

}

void clearData()
{
  while (data_count != 0)
  { // This can be used for any array size,
    Data[data_count--] = 0; //clear array for new data
    clearData();
  }
  return;

}
void badPass()
{
  Timer1.detachInterrupt();
  digitalWrite(red, LOW);
  tone(buzzer, 1200, 150);
  digitalWrite(red, HIGH);
  delay(150);
  digitalWrite(red, LOW);
  delay(300);
  tone(buzzer, 1200, 150);
  digitalWrite(red, HIGH);
  delay(150);
  digitalWrite(red, LOW);;
  delay(300);
  tone(buzzer, 1200, 250);
  delay(50);
  digitalWrite(red, HIGH);
  digitalWrite(buzzer, HIGH);
  Serial.println("Bad Pass");
  initialStatus();
}

void validPass()
{
  Timer1.detachInterrupt();
  digitalWrite(green, LOW);
  tone(buzzer, 1800, 50);
  digitalWrite(green, HIGH);
  delay(50);
  digitalWrite(green, LOW);
  delay(50);
  digitalWrite(green, HIGH);
  tone(buzzer, 1800, 50);
  delay(50);
  digitalWrite(green, LOW);
  delay(50);
  digitalWrite(green, HIGH);
  tone(buzzer, 1800, 50);
  delay(50);
  digitalWrite(green, LOW);
  delay(50);
  digitalWrite(green, HIGH);
  digitalWrite(buzzer, HIGH);
  Serial.println("Valid Pass");
  initialStatus();
}
void initialStatus() {
  digitalWrite(blue, LOW);
  digitalWrite(red, LOW);
  digitalWrite(green, LOW);
}
void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal stre ngth
  long rssi = WiFi.RSSI();
  Serial.print("Signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
