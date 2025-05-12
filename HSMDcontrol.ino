/*
File:      HSMDcontrol.ino
Origin:    05-Mar-2025
Author:    tip partridge
Description: Commaand parser for program HSMDriver
*/
void cmdParse( char c)
  {
  switch (c)
    {
    case 'A':
    case 'a': {
              Serial.println(F("IDX V     I     EMF"));
              for (int ii = 0; ii<avgSize; ii++)
                {
                Serial.print(ii);
                Serial.print("  ");
                Serial.print(vAvgBuf[ii]);
                Serial.print("  ");
                Serial.print(iAvgBuf[ii]);
                Serial.print("  ");
                Serial.print(eAvgBuf[ii]);
                Serial.print("  ");
                Serial.println();
                }
              }
              break;
    case 'B':
    case 'b': // setPWMBot
              setPWMBot();
              break;
    case 'C':
    case 'c': control = !control;
              if (control)
                {
                Serial.println(F("control ON"));
                analogWrite(loopLED, controlLedBrite);
                }
              else
                {
                Serial.println(F("control OFF"));
                analogWrite(loopLED, 0);
                }
              break;
    case 'D':
    case 'd': Serial.println(F("Loading Default parameters."));
              setG2D( & p);
              break;
    case 'E':
    case 'e': Serial.print("Write to EEPROM, are you sure? (Y/[N]) ");
              if (getYN( 10000)==1)
                {
                Serial.print(F("Writing parameters to EEPROM..."));
                putP2E();
                Serial.println("done.");
                }
              break;
    case 'F':
    case 'f': // scaleFast
              setscaleFast();
              break;
    case 'I':
    case 'i': // iGain
              setiGain();
              break;
    case 'L':
    case 'l': logging = !logging;
              if (log) plot = false;
              break;
    case 'N':
    case 'n': showPoints();
              break;
    case 'O':
    case 'o': // ADCOff
              setADCOff();
              break;
    case 'P':
    case 'p': plot = !plot;
              if (plot)
                {
                logging = false;
                Serial.println();
                Serial.println("10ms I V Vemf");
                }
              break;
    case 'R':
    case 'r': // motRes
              setmotRes();
              break;
    case 'S':
    case 's': setscaleSlow();
              break;
    case 'T':
    case 't': // setPWMTop
              setPWMTop();
              break;
    case 'U':
    case 'u': // vSupply
              setvSupply();
              break;
    case 'V':
    case 'v': // vGain
              setvGain();
              break;
    case 'X':
    case 'x': showParameters();
              break;
    case '?':
    case '/': menu();
              break;

    default:  Serial.print("Paused - Press any key");
              while (!Serial.available());
              delay(10);
              while (Serial.available()) Serial.read();
              newln();
              Serial.println("Running");
              break;
    }
  }

void menu()
  {
  Serial.print  (F("======= "));
  Serial.print(vers);
  Serial.println(F(" =========================="));
  Serial.println(F(" ? - This Menu           X - Display params"));
  Serial.println(F(" R - Winding resistance  U - Supply Voltage"));
  Serial.println(F(" B - PWM Bottom          T - PWM Top"));
  Serial.println(F(" I - Current gain        V - Voltage gain"));
  Serial.println(F(" O - ADC offset          C - toggle control"));
  Serial.println(F(" S - scaleSlow           G - scaleFast"));
  Serial.println(F("================================================"));
  Serial.println(F(" P - Toggle Plot         L - toggle Logging"));
  Serial.println(F(" E - params to EEPROM    D - load deFault params"));
  Serial.println(F(" A - dump Avg buffers    N - show scale values"));
  Serial.println(F("================================================"));
  }

  
