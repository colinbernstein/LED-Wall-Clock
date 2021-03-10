/**
   Author: Colin Bernstein
   Title: LED Wall Clock (DST3231 DST Compatible)
*/


//Define GPIO pins and frequently used instance variables

#include <Time.h>
#include <TimeLib.h>
#include <DS3232RTC.h>
#include <DS3231.h>
#include "Wire.h"

#define DS3231_I2C_ADDRESS 0x68

DS3231 Clock; //Initialize the DS3231

//Define the 7-segment animations for all 10 numbers, F, C, -, and the ° symbol
const byte a = 2, b = 3, c = 4, d = 5, e = 6, f = 7, g = 8,
           decimal = 9, FOOSpin = 11, DSTButton = 12, /*inH = 10, inM = 12, inS = 13,*/ tempPin = A0, celciusPin = A1;
const boolean BCD[14][7] = {
  {1, 1, 1, 1, 1, 1, 0}, {0, 1, 1, 0, 0, 0, 0}, {1, 1, 0, 1, 1, 0, 1},
  {1, 1, 1, 1, 0, 0, 1}, {0, 1, 1, 0, 0, 1, 1}, {1, 0, 1, 1, 0, 1, 1},
  {1, 0, 1, 1, 1, 1, 1}, {1, 1, 1, 0, 0, 0, 0}, {1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 0, 0, 1, 1}, {1, 0, 0, 0, 1, 1, 1}, {1, 0, 0, 1, 1, 1, 0},
  {0, 0, 0, 0, 0, 0, 1}, {1, 1, 0, 0, 0, 1, 1}
}; //0123456789FC-°
volatile byte stage;
int temp, cycle;
unsigned long timeStampedTime, DSTpressed;
boolean timeTemp, celcius = false, AMPM, timeStamped = false, DST = false; //true = +1 hour

//Initialize the I2C busses, RTC, GPIO modes, and square wave pulse
void setup() {
  Wire.begin();
  pinMode(a, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(c, OUTPUT);
  pinMode(d, OUTPUT);
  pinMode(e, OUTPUT);
  pinMode(f, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(decimal, OUTPUT);
  pinMode(FOOSpin, OUTPUT);
  pinMode(DSTButton, INPUT_PULLUP);
  pinMode(tempPin, INPUT);
  //setDS3231time(35, 06, 20, 7, 23, 4, 17);  // seconds, minutes, hours, day, date, month, year
  RTC.squareWave(SQWAVE_1_HZ);
  delay(10);
}

//Constantly multiplex at a rate that is invisible but still looks bright
void loop() {
  multPlex();
  if (!digitalRead(DSTButton) && millis() - DSTpressed > 500) {
    DSTpressed = millis();
    DST = !DST;
  }
}

//Set the current time on the DS3231
void setDS3231time(byte second, byte minute, byte hour, byte dayOfWeek, byte
                   dayOfMonth, byte month, byte year) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(dayOfMonth));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.endTransmission();
}

//Read the current time from the DS3231
void readDS3231time(byte * second, byte * minute, byte * hour, byte * dayOfWeek, byte * dayOfMonth, byte * month, byte * year) {
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  *second = bcdToDec(Wire.read() & 0x7f);
  *minute = bcdToDec(Wire.read());
  *hour = bcdToDec(Wire.read() & 0x3f);
  *dayOfWeek = bcdToDec(Wire.read());
  *dayOfMonth = bcdToDec(Wire.read());
  *month = bcdToDec(Wire.read());
  *year = bcdToDec(Wire.read());
}

//Mark a time stamp in milliseconds
void stamp() {
  timeStamped = true;
  timeStampedTime = millis();
}

//Pulse the decade counter to proceed to the next digit
void FOOS() {
  digitalWrite(FOOSpin, HIGH);
  digitalWrite(FOOSpin, LOW);
}

//Blank a digit
void off() {
  for (int i = 2; i < 10; i++)
    digitalWrite(i, LOW);
}

//Display a BCD number to a digit
void displayNum(byte num) {
  for (int x = 0; x < 7; x++)
    digitalWrite(x + 2, BCD[num][x]);
}

//Return the binary value of the given decimal
byte decToBcd(byte val) {
  return ( (val / 10 * 16) + (val % 10) );
}

//Return the decimal value of the given binary number
byte bcdToDec(byte val) {
  return ( (val / 16 * 10) + (val % 16) );
}

//Flash the value of every corresponding digit instantaneously
void multPlex() {
  byte s, m, h, dW, dM, mO, y, lastSec;
  readDS3231time(&s, &m, &h, &dW, &dM, &mO, &y);
  off();
  FOOS();
  if (!timeStamped && timeTemp && s % 10 == 1)
  {
    timeTemp = false;
    lastSec = s;
    RTC.squareWave(SQWAVE_NONE);
  }
  else if (!timeTemp && s % 10 == 3)
  {
    timeTemp = true;
    RTC.squareWave(SQWAVE_1_HZ);
  }
  if (cycle == 5000)
    cycle = 0;
  else
    cycle++;
  if (timeStamped && millis() - timeStampedTime >= 5000)
    timeStamped = false;
  if (stage < 5)
    stage++;
  else
    stage = 0;
  AMPM = h + DST >= 12 ? false : true;
  if (timeTemp)
  {
    /*if (cycle % 200 == 0)
      {
      if (!(boolean) digitalRead(inH))
      {
        stamp();
        if (h == 23)
          setDS3231time(s, m, h - 23, dW, dM, mO, y);
        else
          setDS3231time(s, m, h + 1, dW, dM, mO, y);
      }
      else if (!(boolean) digitalRead(inM))
      {
        stamp();
        if (m == 59)
          setDS3231time(s, m - 59, h, dW, dM, mO, y);
        else
          setDS3231time(s, m + 1, h, dW, dM, mO, y);
      }
      else if (!(boolean) digitalRead(inS))
      {
        stamp();
        if (s == 59)
          setDS3231time(s - 59, m, h, dW, dM, mO, y);
        else
          setDS3231time(s + 1, m, h, dW, dM, mO, y);
      }
      } */
    /*if (stage == 0)
      if ((h == 0 || (h > 9 && h < 13) || h > 21))
        displayNum(1);
      else
        off();*/
    if (stage == 0 && (h + DST == 0 || (h + DST > 9 && h + DST < 13) || h + DST > 21))
      displayNum(1);
    else if (stage == 1)
    {
      if (h + DST <= 12)
        if (h + DST == 0)
          displayNum(2);
        else
          displayNum((h + DST) % 10);
      else
        displayNum((h + DST - 12) % 10);
      digitalWrite(decimal, AMPM);
    }
    else if (stage == 2)
      displayNum(m / 10);
    else if (stage == 3)
      displayNum(m % 10);
    else if (stage == 4)
      displayNum(s / 10);
    else if (stage == 5)
      displayNum(s % 10);
  }
  else
  {
    /* if (!(boolean) digitalRead(celciusPin) && cycle % 100 == 0)
      celcius = !celcius; */
    // if (celcius)
    //   temp = Clock.readTemperature() * 10;
    // else
    temp = Clock.readTemperature() * 18 + 320;
    byte digits;
    boolean positive = temp >= 0;
    if (temp >= 1000 || temp <= -100)
      digits = 4;
    else if (temp >= 100 || temp <= -10)
      digits = 3;
    else if (temp >= 10 || temp <= -1)
      digits = 2;
    if (stage == 5)
      if (celcius)
        displayNum(11);
      else
        displayNum(10);
    else if (stage == 4)
      displayNum(13);
    else if (stage == 3)
      displayNum(abs(temp) % 10);
    else if (stage == 2)
    {
      digitalWrite(decimal, HIGH);
      displayNum(abs(temp) % 100 / 10);
    }
    else if (stage == 1 && digits >= 3)
    {
      if (positive)
        displayNum(abs(temp) % 1000 / 100);
      else
        displayNum(12);
    }
    else if (stage == 0)
      if (digits == 4)
        if (temp >= 0)
          displayNum(abs(temp) / 1000);
        else
          displayNum(12);
  }
}
