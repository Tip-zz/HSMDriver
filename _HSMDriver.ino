/*
File:       _HSMDriver.ino
Origin:     28-Feb-2025
Author:     tip partridge
Description: Motor driver for HiSmith.  Uses current feedback to stabalize speed via feed-forward.
    First version just compensated for resistive voltage drop across windings.  Worked well a higher speed
    but poorly at low speed which is where I wanted it to work.  Not sure how to model this so intrduced
    scaleSlow and gainHigh: current multiplier at encoder==99 and at encoder==1. Klugey, but seems to work.
    Use 2.5V precision reference for ADCREF and to provide 0.12V reference for shunt amp.  Shunt is 0.05 Ohm and
    shunt amp gain = 100, so reading clips at 0.476A.  Voltage readback isn't used except in Plot and
    Log mode to make nice display show total motor voltage and effective voltage.
    Note: Voltage sense MPC602 OP-AMP common mode input Vss-0.3, Vdd-1.2 so with VCC = 4.8V max = 3.6V.

Revisions:
    03-Mar-2025 (TEP) v0.0 Basic function
    13-Apr-2025 (TEP) v0.1 Works well enough to commit to perf board.
    26-Apr-2025 (TEP) v0.21 Works on perf. Add close loop button and LED.
    04-May-2025 (TEP) v1.5 Add scaleSlow and scaleFast stuff.
*/
#define vers "HSMDriver v1.5"
#include <EEPROM.h>

// user defaults
#define dmotRes 0.68      // motor winding resistance. Motor 0.52 + cable 0.16
#define dvSupply 24.0     // motor supply voltage
#define diGain 0.2        // Current sense (Amotor/Vadcin) = 1 / (0.05 Ohm * 100 gain)
#define dvGain 12.1       // Voltage sense (Vmotor/Vadcin) = 121k/10k
#define dADCOff 0         // Current ADC offset to fine tune
#define dPWMBot 15        // Startup PWM level
#define dPWMTop 220       // Max PWM level, leave room for feedback
#define dscaleFast 1.2    // scale factor to adjust emfScale at encoder = 99
#define dscaleSlow 8      // scale factor to adjust emfScale at encoder = 1

// Pin definitions
#define encAPin 2   // Speed control encoder A phase
#define encBPin 3   // Speed control encoder B phase
#define swPin 4     // Speed control encoder button
#define CLK 5       // 7-seg Display clock
#define DIO 6       // 7-seg Display data
#define motPin 9    // Motor PWM
#define loopSw 10   // closed loop button
#define loopLED 11  // closed loop LED
#define curPin A0   // Motor current analog read
#define refPin A1   // 0.119V reference for current shunt amp
#define volPin A7   // Motor voltage analog read

// 4 digit 7-seg display in DigitalTube library
#include <TM1637.h>
TM1637 Disp4(CLK, DIO);   // 4 digit LED display handler
//
#define bounceMillis 10
#define ADCRef 2.5    // ADC reference voltage
#define avgSize 10    // averaging buffers size
#define repRate 10    // 10ms per update
#define baseControlLedBrite 8  // base brightness for control LED

float V, I, Vemf;     // working volts, amps, effective volts
float scale;          // PWM control scailing
int iPWM;             // PWM value
int encod;            // encoder value
int lastEncod;        // previous encoder value
bool sw, lastSw;      // switch value and previous value
bool lastA;           // encoder phase A last
bool nextA, nextB;    // encoder phase A and B
bool lastControlButton = true; // control toggle button last
bool plot = false;    // plot mode flag
bool logging = false; // plot mode flag
bool control;         // current control mode flag
char c;               // Serial data for user commands

//calculated constants
float motScale;       // PWM ticks per encoder tick
float emfScale0;      // ideal PWM ticks per ampere
float emfScale;       // adjusted PWM ticks per ampere when control is on
float vScale;         // output volts per ADC lsb
float iScale;         // output current per ADC lsb

float vAvgBuf[avgSize+1];   // averaging buffer
float iAvgBuf[avgSize+1];   // averaging buffer
float eAvgBuf[avgSize+1];   // averaging buffer
float vAvgSum, iAvgSum, eAvgSum;  // running sum
int avgPnt;                 // buffer pointer
byte controlLedBrite;       // control LED brightness
unsigned long repTime, repTime0;  // loop timer
int rep = 0;                // tick mark in plot each 10 passes

struct Parameters   // These saved in EEPROM
  {
  float motRes;     // motor winding resistance
  float vSupply;    // motor supply voltage
  int PWMBot;       // Base PWM
  int PWMTop;       // Maximum PWM
  float iGain;      // Current sense (V/A)
  float vGain;      // Voltage sense (V/V)
  int ADCOff;       // Current ADC offset
  float scaleFast;  // factor to adjust emfScale for later points
  float scaleSlow;  // 
  };
Parameters p;       // working parameters

// ***** EEPROM **************************************************
const unsigned long EEInitFlag = 0x444d5348L;  // this 32 bit value flags that EEPROM is initialized = 0x44 4d 53 48  DMSH 
union Ini     // EEPROM initialized flag, can be long or array of 4 characters, just for fun
  {
  unsigned long iniflg;
  char inistr[4];
  };
// EEPROM state
Ini g_ini;              // special value indicates EEPROM has been initialized
// EEPROM layout
const int EEInitAddr = 0;   // address in EEPROM of flag that says EEPROM has been initialized
const unsigned int parametersAddr = EEInitAddr + sizeof(g_ini);
// ***** end EEPROM **********************************************

// ***************   SSSSS   EEEEEE   TTTTTTTT   UU   UU   PPPPP    *********
// ***************  SS       EE          TT      UU   UU   PP   PP  *********
// Setup *********    SS     EEEEE       TT      UU   UU   PPPPP    *********
// ***************      SS   EE          TT      UU   UU   PP       *********
// ***************  SSSSS    EEEEEE      TT        UUU     PP       *********

void setup()
  {
  Serial.begin(115200);
  Serial.println();
  Serial.println(vers);
  analogReference(EXTERNAL);
  pinMode( encAPin, INPUT_PULLUP);
  pinMode( encBPin, INPUT_PULLUP);
  pinMode( swPin, INPUT_PULLUP);
  pinMode( motPin, OUTPUT);
  pinMode( loopSw, INPUT_PULLUP);
  pinMode( loopLED, OUTPUT);
// ***** check EEPROM status *************************************
  EEPROM.get( EEInitAddr, g_ini.iniflg);
  if (g_ini.iniflg == EEInitFlag) // EEPROM initialized?
    {
    getE2P();       // load global parameters from EEPROM
    }
  else              // EEPROM blank, use defaults
    {
    Serial.print(F("Initializing EEPROM.  "));
    putD2E();       // Load default parameters to EEPROM
    getE2P();       // load global parameters from EEPROM
    }
// ***************************************************************

  calcScales();
  Disp4.clearDisplay();
  Disp4.set(7);         // LED display max brightness
  Disp4.point(false);   // display's colon off
///  avgTime0 = millis();
  TCCR1B = TCCR1B & B11111000 | B00000001; // for PWM frequency of 31372.55 Hz
  Serial.println(F("(Press ? for menu)"));
  control = true;
  controlLedBrite = baseControlLedBrite; 
  analogWrite(loopLED, controlLedBrite);
  lastA = digitalRead(encAPin);
  repTime0 = millis();
///  lastB = digitalRead(encBPin);
  }

//*******************  LL       OOOOO     OOOOO    PPPPPP   **************************
//*******************  LL      00   00   00   00   PP   PP  **************************
// Loop *************  LL      00   O0   00   00   PPPPPP   **************************
//*******************  LL      00   OO   00   00   PP       **************************
//*******************  LLLLLL   00O00     OOOOO    PP       **************************

void loop()
  {
// Pause on keystroke
  if (Serial.available())
    {
    c = Serial.read();
    if (c>=32) cmdParse( c);
    calcScales();
    }
// check close loop button
  if (!digitalRead( loopSw))
    {
    if (lastControlButton)
      {
      control = !control;
      if (control)
        analogWrite(loopLED, controlLedBrite);
      else
        analogWrite(loopLED, 0);
      lastControlButton = false;
      delay(100);   // debounce
      }
    }
  else lastControlButton = true;
//
// check encoder and button
  sw = digitalRead(swPin);
  nextA = digitalRead(encAPin);
  nextB = digitalRead(encBPin);
  if (lastA!=nextA)
    {
    lastA = nextA;
    if (!nextA) // falling edge
      {
      if (!nextB)
        {
        encod -= 1;
        if (encod<0) encod = 0;
        }
      else
        {
        encod += 1;
        if (encod>99) encod = 99;
        }
      }
    emfScale = emfScale0 * p.scaleFast * ((98+scale)/(encod-1+scale));
    delay(bounceMillis);  // Max bounce speced as 5ms
    }
  if (!sw) encod = 0;


repTime = millis();
if (repTime - repTime0 >= repRate)
{
  repTime0 - repTime;

//
// Read voltage and current
//
  V = analogRead(volPin) * vScale;
  I = (analogRead(curPin)+p.ADCOff-analogRead(refPin)) * iScale;
  Vemf = V - I * p.motRes;
//
// Calculate running averages
//
  vAvgSum += (V - vAvgBuf[avgPnt]);
  iAvgSum += (I - iAvgBuf[avgPnt]);
  eAvgSum += (Vemf - eAvgBuf[avgPnt]);
  vAvgBuf[avgPnt] = V;
  iAvgBuf[avgPnt] = I;
  eAvgBuf[avgPnt++] = Vemf;
  if (avgPnt>=avgSize) avgPnt = 0;
  V = vAvgSum / avgSize;
  I = iAvgSum / avgSize;
  Vemf = eAvgSum / avgSize;
//
// Set control LED brightness
//
  controlLedBrite = control * byte((I / iScale) * ((255.0-baseControlLedBrite)/1023.0) + baseControlLedBrite);
  analogWrite(loopLED, controlLedBrite);
//
// Calculate PWM
//
  iPWM = (encod-1) * motScale + p.PWMBot;
  if (control) iPWM = iPWM + (I*emfScale);
  if (encod==0) iPWM=0;
  else if (iPWM>255) iPWM=255;
  else if (iPWM<0) iPWM=0;
//
// plot voltage and current
  if (plot)
    {
    if (rep>=9)
      {
      Serial.print(0.1);
      rep=0;
      }
    else
      {
      Serial.print(0.01);
      rep+=1;
      }
    Serial.print(" ");
    Serial.print(I*10.0,3);
    Serial.print(" ");
    Serial.print(V);
    Serial.print(" ");
    Serial.print(Vemf);
    Serial.println();
    }
  else if (logging)
    {
    Serial.print("encod= ");
    Serial.print(encod);
    Serial.print("  I= ");
    Serial.print(I,4);
    Serial.print("  V= ");
    Serial.print(V);
    Serial.print("  Vemf= ");
    Serial.print(Vemf);
    Serial.print("  PWM= ");
    Serial.print(iPWM);
//Serial.print("  ADC= "); Serial.print(analogRead(curPin)-analogRead(refPin));
//Serial.print("  ADCi= "); Serial.print(analogRead(curPin));
//Serial.print("  ADCr= "); Serial.print(analogRead(refPin));
Serial.print("  LED= "); Serial.print(controlLedBrite);
    Serial.println();
    }

  if ((encod!=lastEncod) || (sw!=lastSw))
    {
// Numeric display
    Disp4.display(2, encod%10);   // lower digit
    if (encod/10)
      Disp4.display(1, encod/10); // upper digit
    else
      Disp4.display(1, 0x7f);     // upper digit blank

    if (logging)
      {
      Serial.print("  ");
      Serial.print(encod);
      Serial.print("  ");
      Serial.print(sw);
      Serial.print("  ");
      Serial.print(encod-lastEncod);
      Serial.print("  ");
      Serial.print(round(encod * motScale + p.PWMBot));
      Serial.println();
      }
    lastEncod = encod;
    lastSw = sw;
    }
//
// Set PWM
//
  analogWrite(motPin, iPWM);
  }
}
