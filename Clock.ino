#include <Wire.h>
#include "RTClib.h"
#define DS1307_I2C_ADDRESS 0x68
const byte a = 2, b = 3, c = 4, d = 5, e = 6, f = 7, g = 8,
           decimal = 9, FOOSpin = 11, inH = 10, inM = 12, inS = 13, tempPin = A0, celciusPin = A1;
const boolean BCD[14][7] = {
  {1, 1, 1, 1, 1, 1, 0}, {0, 1, 1, 0, 0, 0, 0}, {1, 1, 0, 1, 1, 0, 1},
  {1, 1, 1, 1, 0, 0, 1}, {0, 1, 1, 0, 0, 1, 1}, {1, 0, 1, 1, 0, 1, 1},
  {1, 0, 1, 1, 1, 1, 1}, {1, 1, 1, 0, 0, 0, 0}, {1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 0, 0, 1, 1}, {1, 0, 0, 0, 1, 1, 1}, {1, 0, 0, 1, 1, 1, 0},
  {0, 0, 0, 0, 0, 0, 1}, {1, 1, 0, 0, 0, 1, 1}
}; //0123456789FC-°
volatile byte stage;
volatile int temp, cycle;
volatile float tempReading;
uint32_t timeStampTime;
boolean timeTemp = true, celcius = false, AMPM = true, timeStamp = false;
RTC_DS1307 RTC;
DateTime now1;

void setup() {
  analogReference(INTERNAL);
  //Serial.begin(9600);
  Wire.begin();
  RTC.begin();
  pinMode(a, OUTPUT);
  pinMode(b, OUTPUT);
  pinMode(c, OUTPUT);
  pinMode(d, OUTPUT);
  pinMode(e, OUTPUT);
  pinMode(f, OUTPUT);
  pinMode(g, OUTPUT);
  pinMode(decimal, OUTPUT);
  pinMode(FOOSpin, OUTPUT);
  pinMode(inH, INPUT_PULLUP);
  pinMode(inM, INPUT_PULLUP);
  pinMode(inS, INPUT_PULLUP);
  pinMode(celciusPin, INPUT_PULLUP);
  pinMode(tempPin, INPUT);
  // RTC.adjust(DateTime(__DATE__, __TIME__));
  Wire.beginTransmission(DS1307_I2C_ADDRESS);
  Wire.write(0x07); // move pointer to SQW address
  Wire.write(0x10); // sends 0x10 (hex) 00010000 (binary)
  Wire.endTransmission();
  delay(10);
}
void loop() {
  multPlex();
  if (cycle % 100 == 0)
    tempReading = ((analogRead(tempPin) / 1024.0) * 5000.0 / 10.0);
  if (!timeStamp && timeTemp && cycle == 5000)
    timeTemp = false;
  else if (!timeTemp && cycle == 2000)
    timeTemp = true;
  if (cycle == 5000)
    cycle = 0;
  else
    cycle++;
  if (timeStamp && now1.unixtime() - timeStampTime >= 5) //This line is faulty
    timeStamp = false;
}

void stamp() {
  timeStamp = true;
  timeStampTime = now1.unixtime();
}

void FOOS() {
  digitalWrite(FOOSpin, HIGH);
  digitalWrite(FOOSpin, LOW);
}

void off() {
  digitalWrite(a, LOW);
  digitalWrite(b, LOW);
  digitalWrite(c, LOW);
  digitalWrite(d, LOW);
  digitalWrite(e, LOW);
  digitalWrite(f, LOW);
  digitalWrite(g, LOW);
  digitalWrite(decimal, LOW);
}
void displayNum(byte num) {
  for (int x = 0; x < 7; x++)
    digitalWrite(x + 2, BCD[num][x]);
}

void multPlex() {
  now1 = RTC.now();
  off();
  FOOS();
  if (stage < 5)
    stage++;
  else
    stage = 0;
  AMPM = now1.hour() >= 12 ? false : true;
  if (timeTemp)
  {
    if (cycle % 200 == 0)
    {
      if (!(boolean) digitalRead(inH))
      {
        stamp();
        RTC.adjust(now1.unixtime() + 3600);
      }
      else if (!(boolean) digitalRead(inM))
      {
        stamp();
        if (now1.minute() == 59)
          RTC.adjust(now1.unixtime() - 3540);
        else
          RTC.adjust(now1.unixtime() + 60);
      }
      else if (!(boolean) digitalRead(inS))
      {
        stamp();
        if (now1.second() == 59)
          RTC.adjust(now1.unixtime() - 59);
        else
          RTC.adjust(now1.unixtime() + 1);
      }
    }
    if (stage == 0)
      if ((now1.hour() == 0 || (now1.hour() > 9 && now1.hour() < 13) || now1.hour() > 21))
        displayNum(1);
      else
        off();
    else if (stage == 1)
    {
      if (now1.hour() <= 12)
        if (now1.hour() == 0)
          displayNum(2);
        else
          displayNum(now1.hour() % 10);
      else
        displayNum((now1.hour() - 12) % 10);
      if (!AMPM)
        digitalWrite(decimal, HIGH);
    }
    else if (stage == 2)
      displayNum(now1.minute() / 10);
    else if (stage == 3)
      displayNum(now1.minute() % 10);
    else if (stage == 4)
      displayNum(now1.second() / 10);
    else if (stage == 5)
      displayNum(now1.second() % 10);
  }
  else
  {
    if (!(boolean) digitalRead(celciusPin) && cycle % 100 == 0)
      celcius = !celcius;
    if (celcius)
      temp = tempReading * 10;
    else
      temp = (((tempReading * 9 ) / 5) + 32 ) * 10;
    byte digits;
    if (temp >= 1000 || temp <= -100)
      digits = 4;
    else if (temp >= 100 || temp <= -10)
      digits = 3;
    if (stage == 5)
      if (celcius)
        displayNum(11);
      else
        displayNum(10);
    else if (stage == 4)
      displayNum(13);
    else if (stage == 3)
      displayNum(temp % 10);
    else if (stage == 2)
    {
      digitalWrite(decimal, HIGH);
      displayNum(temp % 100 / 10);
    }
    else if (stage == 1)
      if (digits >= 3)
        displayNum(temp % 1000 / 100);
      else if (stage == 0)
        if (digits == 4)
          if (temp >= 0)
            displayNum(temp / 1000);
          else
            displayNum(12);
  }
}
