#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>

#define sampleTime 50
#define calibrationValue 1.5

#define decreaseMultiplier D0
#define increaseMultiplier D1
#define microphoneInput A0
#define wifiLED D2
#define EPROMAddress 0;
#define calibrationLight D5
long pakcets_sent = 0;

const String deviceName = "Roskilde Location 1";

//WIFI Cridentials
char ssid[] = "netgear";
char pass[] = "r0skilde2018";

//char ssid[] = "Ravi's Surface Studio";
//char pass[] = "RF18_cbsbda_westerdals";

byte mac[6];



const float multiplierChange = 0.00005;
HTTPClient http;

int packetsSent = 0;
bool calibrationMode = true;
int decreaseMultiplierPressed;
int increaseMultiplierPressed;
float multiplier = 1.0;

void setup() {
  pinMode(wifiLED, OUTPUT);
  pinMode(calibrationLight, OUTPUT);
  pinMode(decreaseMultiplier, INPUT);
  pinMode(increaseMultiplier, INPUT);

  Serial.begin(9600);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Trying to connect..");
    WiFi.begin(ssid, pass);
  }

  while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion
    delay(500);
    Serial.println("Trying to connect to " + (String)ssid + " ...");
  }
  digitalWrite(wifiLED, HIGH);
  Serial.println("Connection Successful :) Ready to record record some data.");
}

void loop() {

  decreaseMultiplierPressed = digitalRead(decreaseMultiplier);
  increaseMultiplierPressed = digitalRead(increaseMultiplier);


  if (increaseMultiplierPressed == 1 && decreaseMultiplierPressed == 1) {
    calibrationMode = !calibrationMode;
    Serial.println("Calibration mode is " + calibrationMode ? "true" : "false");
    digitalWrite(calibrationLight, LOW);
    delay(1000);
    digitalWrite(calibrationLight, HIGH);
    delay(1000);
    digitalWrite(calibrationLight, LOW);
  }


  double decibels = readDecibels(6, 42);
  String dbs = String(decibels, 2);


  if (calibrationMode) {
    digitalWrite(calibrationLight, HIGH);
    Serial.print("dB: ");
    Serial.println(dbs);

    decreaseMultiplierPressed = digitalRead(decreaseMultiplier);
    increaseMultiplierPressed = digitalRead(increaseMultiplier);


    if (increaseMultiplierPressed == 1){
      Serial.println("add");
      multiplier = multiplier + 0.05;
    }
    if (decreaseMultiplierPressed == 1){
      Serial.println("subtract");
      multiplier = multiplier - 0.05;
    }

    Serial.print("multiplier: ");
    Serial.println(multiplier);
    Serial.println("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-");


  } else {
    WiFi.macAddress(mac);
    String deviceMac = mac2String(mac);
    String json = "{\"name\":\"" + deviceName + "\",\"db_level\":\"" + dbs + "\", \"mac_address\":\"" + deviceMac + "\"}";

    int response = sendData(json);

    if (response == -1) {
      Serial.println("Error. Something went wrong");
      Serial.println("Checking WiFi connection");
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("No WiFi connection... Re-connecting");
        digitalWrite(wifiLED, LOW);
        WiFi.begin(ssid, pass);
        while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion
          delay(500);
          Serial.println("Trying to connect to " + (String)ssid + " ...");
        }
        Serial.println("Connection Successful :) Ready to record record some data.");
      } else {
        digitalWrite(wifiLED, HIGH);
        Serial.println("WiFi is connected.. Trying to send the next package..");
        Serial.println("=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=");
      }
    } else if (response == 200) {

    } else if (response == 201) {
      packetsSent ++;
      Serial.print("Packets sent: ");
      Serial.println(packetsSent);
    }
    if (packetsSent >= 2147483647 - 1)
      packetsSent = 0;

  }
}




double readDecibels(int a, int b) {
  float start = millis();

  int peak_min = 1024;
  int peak_max = 0;
  while (millis() - start < sampleTime) {
    double sample = analogRead(microphoneInput);

    if (sample < 1024) {
      peak_min = sample < peak_min ? sample : peak_min;
      peak_max = sample > peak_max ? sample : peak_max;
    }
  }

  double peak = peak_max - peak_min;
  return 20 * log10(peak / a) + (b);
}




int sendData(String data) {

  http.begin("http://roskilde.herokuapp.com/api/record?access_token=1234");
  http.addHeader("Content-Type", "application/json");
  int HTTPStatus = http.POST(data);
  http.end();
  return HTTPStatus;
}

String mac2String(byte ar[]) {
  String s;
  for (byte i = 0; i < 6; ++i)
  {
    char buf[3];
    sprintf(buf, "%2X", ar[i]);
    s += buf;
    if (i < 5) s += ':';
  }
  return s;
}

