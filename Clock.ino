/**
   Author: Colin Bernstein
   Title: LED Wall Clock
*/


//Define GPIO pins and frequently used instance variables
const byte a = 2, b = 3, c = 4, d = 5, e = 6, f = 7, g = 8,
           decimal = 9, colon = 10, FOOSpin = 11, inH = A2, inM = 12, inS = 13, tempPin = A0, celciusPin = A1;
byte stage;
boolean time = true, celcius = false;
float tempF;
unsigned long curr;
#include <Wire.h>
#include "RTClib.h"
RTC_DS1307 RTC;
DateTime now;

//Define the 7-segment animations for all 10 numbers, F, C, -, and the ° symbol
byte BCD[14][7] = {{1, 1, 1, 1, 1, 1, 0}, {0, 1, 1, 0, 0, 0, 0}, {1, 1, 0, 1, 1, 0, 1},
  {1, 1, 1, 1, 0, 0, 1}, {0, 1, 1, 0, 0, 1, 1}, {1, 0, 1, 1, 0, 1, 1},
  {1, 0, 1, 1, 1, 1, 1}, {1, 1, 1, 0, 0, 0, 0}, {1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 0, 0, 1, 1}, {1, 0, 0, 0, 1, 1, 1}, {1, 0, 0, 1, 1, 1, 0},
  {0, 0, 0, 0, 0, 0, 1}, {1, 1, 0, 0, 0, 1, 1}
}; //0123456789FC-°

//Initialize the I2C busses, the RTC, and the GPIO modes
void setup() {
  analogReference(INTERNAL);
  Serial.begin(9600);
  Wire.begin();
  RTC.begin();
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  pinMode(a, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(c, OUTPUT);
  pinMode(d, OUTPUT);
  pinMode(e, OUTPUT);
  pinMode(f, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(colon, OUTPUT);
  pinMode(FOOSpin, OUTPUT);
  pinMode(inH, INPUT_PULLUP);
  pinMode(inM, INPUT_PULLUP);
  pinMode(inS, INPUT_PULLUP);
  pinMode(celciusPin, INPUT_PULLUP);
  pinMode(tempPin, INPUT);
}

//Constantly multiplex at a rate that is invisible but still looks bright
void loop() {
  multPlex(1);
  delayMicroseconds(1);
}

//Adjust the RTC's stored time based off of an incoming button press
void button() {
  now = RTC.now();
  boolean H = !(boolean) digitalRead(inH), M = !(boolean) digitalRead(inM), S = !(boolean) digitalRead(inS);
  if (H == HIGH)
    RTC.adjust(DateTime("APR 15 2016", "12:59:45"));
  else if (M == HIGH)
    setTime(now.hour(), now.minute() + 1, now.second(), now.day(), now.month(), now.year());
  else if (S == HIGH)
    setTime(now.hour(), now.minute(), now.second() + 1, now.day(), now.month(), now.year());
}

//Pulse the decade counter to proceed to the next digit
void FOOS() {
  digitalWrite(FOOSpin, HIGH);
  digitalWrite(FOOSpin, LOW);
}

//Flash the value of every corresponding digit instantaneously
void multPlex(float temp) {
  now = RTC.now();
  if (stage < 5)
    stage++;
  else
    stage = 0;
  if (true)
  {
    if (stage == 0)
    {
      digitalWrite(silence, LOW);
      FOOS();
      digitalWrite(silence, HIGH);
      if (hours / 10 == 1)
        displayNum(now.hour() / 10);
      else
        digitalWrite(silence, LOW);
    }
    else if (stage == 1)
    {
      digitalWrite(silence, LOW);
      FOOS();
      digitalWrite(silence, HIGH);
      displayNum(now.hour() % 10);
    }
    else if (stage == 2)
    {
      digitalWrite(silence, LOW);
      FOOS();
      digitalWrite(silence, HIGH);
      displayNum(now.minute() / 10);

    }
    else if (stage == 3)
    {
      digitalWrite(silence, LOW);
      FOOS();
      digitalWrite(silence, HIGH);
      displayNum(now.minute() % 10);
    }
    else if (stage == 4)
    {
      digitalWrite(silence, LOW);
      FOOS();
      digitalWrite(silence, HIGH);
      displayNum(now.second() / 10);
    }
    else if (stage == 5)
    {
      digitalWrite(silence, LOW);
      FOOS();
      digitalWrite(silence, HIGH);
      displayNum(now.second() % 10);
    }
  }
  else
  {
    if (stage == 5)
    {
      digitalWrite(silence, LOW);
      FOOS();
      digitalWrite(silence, HIGH);
      if (celcius)
        displayNum(11);
      else
        displayNum(10);
    }
    else if (stage == 4)
    {
      digitalWrite(silence, LOW);
      FOOS();
      digitalWrite(silence, HIGH);
      displayNum(temp % 10);
    }
  }
}

//Write a BCD number to the cathode controlling chip
void displayNum(byte num)
{
  for (int x = 1; x < 8; x++)
    digitalWrite(x + 3, (boolean) BCD[num][x]);
}
