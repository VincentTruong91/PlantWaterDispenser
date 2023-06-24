/*
* Author: Vincent Truong
* Class: Spring 2023 IoT Project
* Project: Automatic/Manual Plant Watering System
* Date: Mar 2023 - June 2023
*/

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>


/* IMPORTANT: FILL IN YOUR OWN AUTHORIZATION TOKEN, 
 * WIFI USER (SSID), AND WIFI PASSWORD (PASS) 
*/
char auth[] = "################################";
char ssid[] = "#######################";
char pass[] = "#####################";

BlynkTimer timer;
bool Relay = 0;

// We wil use PIN 33 as the SENSOR pin
#define sensor 33

// We can use any GPIO pins but we will use PIN 4
#define waterPump 4


bool automate = false;

void setup() {
  Serial.begin(9600);
  delay(1000);

  // We then INITIALIZE the ESP32 WiFi module
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
  pinMode(waterPump, OUTPUT);
  
  // WHEN CONNECTING, TURN OFF THE WATER PUMP
  digitalWrite(waterPump, LOW);

  // WE WILL USE BLYNK AS THE CONTROLLER THAT CONNECTS TO THE PROVIDED NETWORK
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
  timer.setInterval(100L, soilMoistureSensor);
}


// THE FIRST BUTTON TURNS THE PUMP ON/OFF
BLYNK_WRITE(V1) {
  Relay = param.asInt();

  if (Relay == 1) {
    digitalWrite(waterPump, LOW);
  }
  if (Relay == 0){
    digitalWrite(waterPump, HIGH);
  }
}

// THE SECOND BUTTON ENABLES MANUAL/AUTOMATIC WATERING MODES
BLYNK_WRITE(V2) {
  Relay = param.asInt();
  if (Relay == 1) { // If automation is turned on
    automate = true;
  } 
  else { // If automation is turned off
    automate = false;
  }
}


/* THIS FUNCTION READS THE MOISTURE OF SOIL USING
 * A SOIL MOISTURE SENSOR AND THEN MAPS THE VALUE
*/
void soilMoistureSensor() {
  int value = analogRead(sensor);
  value = map(value, 0, 4096, 0, 100); //esp32 is a 12 bit, so 12 bit = 4096, while 10 bit = 1024
  value = (value - 100) * -1; //the value is from 100 to 0, so it flips it (making it negative), then it changes it to positive
  
  // IF YOU WANT TO SEE THE HUMIDITY % IN THE SERIAL MONITOR, UNCOMMENT THE 2 LINES BELOW
  //Serial.print("Humidity %: ");
  //Serial.println(value*2);

  /* THEN IT'LL TAKE THE CALCULATED VALUE AND SEND IT TO BLYNK,
   * CHANGING THE GAUGE OF WATER MOISTURE IN REAL TIME
  */
  Blynk.virtualWrite(V0, value*2);
}

void loop() {

  Blynk.run();

  int moistureValue = analogRead(sensor);
  moistureValue = map(moistureValue, 0, 4096, 0, 100);
  moistureValue = (moistureValue - 100) * -1;
  moistureValue *= 2;

  /* UNCOMMENT THESE BOTTOM 2 LINES TO SEE THE STATUS OF THE AUTOMATION
  * 
  * If automation contains a 0, then automation is on
  * otherwise, a 1 means that automation is off
  * and manual mode is enabled
  */
  //Serial.print("Automation: ");
  //Serial.println(automate);

  //AUTOMATION STATUS NEEDS TO BE CONSTANTLY CHECKED
  if(automate == 0){
    if (moistureValue < 50) {
      digitalWrite(waterPump, HIGH);  // Turn on water pump
    } 
    else {
      digitalWrite(waterPump, LOW);  // Turn off water pump
    }
  }

  Blynk.run();
  timer.run();
  soilMoistureSensor();
}
