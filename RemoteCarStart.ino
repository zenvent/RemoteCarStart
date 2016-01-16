/*
 SMS REMOTE CAR STARTER
  Objective
  Extend the range of my vehicle's auto-start using the cheapest solution I can think of.
  I would also like to keep vehicle modifications to a minimum (or none).
  
  Solution
  Trigger the auto-start remote using an Arduino connected to a cell service. User will 
  send an SMS message to the device, which will validate the message then trigger a relay 
  which will cause the remote to send a signal and the car will start. The device will be 
  left in the car and run off the vehicle's battery.
  
  Skills required
  If you follow the tutorial, basic computer and electronics skills. I am assuming you can 
  make connections using a bread board or soldering iron. I am also assuming your car 
  already has a remote starter installed, and that you are willing to sacrifice one of the 
  remotes.

 Created by Zach Crabtree | 1/16/2015
 For detailed instructions please visit my website:

 http://www.zenvent.com

Download these libraries
https://github.com/Seeed-Studio/GPRS_SIM900/blob/master/GPRS_Shield_Arduino.h
https://github.com/Seeed-Studio/Suli
*/

#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Suli.h>

#define PIN_TX    7
#define PIN_RX    8
#define BAUDRATE  9600
#define PHONE_NUMBER "+1xxxxxxxxxxx" //Add your cell phone number
#define START_COMMAND "Start" //Command you'd like to text
#define MESSAGE_SUCCESS  "Vehicle is running."
#define MESSAGE_FAILED  "Vehicle failed to start."
#define MESSAGE_DENIED "Access Denied."
#define MESSAGE_UNKNOWN "Uknown Command."

#define MESSAGE_LENGTH 160
char message[MESSAGE_LENGTH];
int messageIndex = 0;
bool VEHICLE_RUNNING = false;

char phone[16];
char datetime[24];
const int relayPin =  12;
const int sensorPinOut = 11;
const int sensorPinIn = 10;
int relayState = LOW;

GPRS gprs(PIN_TX, PIN_RX, BAUDRATE); //RX,TX,PWR,BaudRate

void setup() {
  Serial.begin(9600);
  pinMode(13, OUTPUT);
  while (0 != gprs.init()) {
    powerSimOnOff();
    Serial.print("Init Error.\r\n");
  }
  delay(5000);
  powerSimOnOff();
  
  Serial.println("Init Success, ready.");
  pinMode(relayPin, OUTPUT);
  pinMode(sensorPinOut, OUTPUT);
  pinMode(sensorPinIn, INPUT);
  digitalWrite(sensorPinOut, HIGH);
}

void powerSimOnOff() {
  pinMode(9, OUTPUT); 
  digitalWrite(9,LOW);
  delay(1000);
  digitalWrite(9,HIGH);
  delay(2000);
  digitalWrite(9,LOW);
  delay(3000);
}

void loop() {
  Serial.print("\r\nREADING SIM... ");
  messageIndex = gprs.isSMSunread();
  
  if (messageIndex > 0) { //At least, there is one UNREAD SMS
    gprs.readSMS(messageIndex, message, MESSAGE_LENGTH, phone, datetime);
    gprs.deleteSMS(messageIndex);
    Serial.print("\r\nFrom number: ");
    Serial.println(phone);
    Serial.print("Datetime: ");
    Serial.println(datetime);
    Serial.print("Recieved Message: ");
    Serial.println(message);

    if (strcmp(phone, PHONE_NUMBER)==0) {
      if (strcmp(message, START_COMMAND)==0) {
        //If the phone # and command are correct, start car.
        Serial.print("STARTING VEHICLE.\r\n");
        relayState = HIGH;
        digitalWrite(relayPin, relayState);
        delay(5000);
        relayState = LOW;
        digitalWrite(relayPin, relayState);
        delay(10000);
      } else {
        //Phone number is correct, but unkown command was used.
        Serial.print(MESSAGE_UNKNOWN);
        gprs.sendSMS(PHONE_NUMBER, MESSAGE_UNKNOWN);
      }
    } else {
      //Incorrect phone number.
      Serial.print(MESSAGE_DENIED);
      //gprs.sendSMS(PHONE_NUMBER, MESSAGE_DENIED);
    }

      //Determine if vehicle is running, uncomment to use.
      //VEHICLE_RUNNING = (digitalRead(sensorPinIn) == HIGH);
      VEHICLE_RUNNING = true;

      if (VEHICLE_RUNNING) {
        Serial.print(MESSAGE_SUCCESS);
        gprs.sendSMS(PHONE_NUMBER, MESSAGE_SUCCESS);
      } else {
        Serial.print(MESSAGE_FAILED);
        gprs.sendSMS(PHONE_NUMBER, MESSAGE_FAILED);
      }
      
      VEHICLE_RUNNING = false;
    }
    delay(2500);
    digitalWrite(13, LOW);
    delay(2500);
    digitalWrite(13, HIGH);
  }

