/*
File:      HSMDcommand.ino
Origin:    05-Mar-2025
Author:    tip partridge
Description: User commands for program _HSMDriver
*/

void showParameters()
  {
  Serial.print(F("motRes    (R)  ")); Serial.print(p.motRes); Serial.println(" Ohms");
  Serial.print(F("vSupply   (E)  ")); Serial.print(p.vSupply); Serial.println(" Volts");
  Serial.print(F("iGain     (I)  ")); Serial.print(p.iGain); Serial.println(" Vadcin/Amotor");
  Serial.print(F("vGain     (V)  ")); Serial.print(p.vGain); Serial.println(" Vadcin/Vmotor");
  Serial.print(F("ADCOff    (O)  ")); Serial.print(p.ADCOff); Serial.println(" lsb");
  Serial.print(F("setPWMBot (B)  ")); Serial.print(p.PWMBot); Serial.println(" PWM");
  Serial.print(F("setPWMTop (T)  ")); Serial.print(p.PWMTop); Serial.println(" PWM");
  Serial.print(F("scaleFast (F)  ")); Serial.print(p.scaleFast); Serial.println(" emfScale multiplier");
  Serial.print(F("scaleSlow (S)  ")); Serial.print(p.scaleSlow); Serial.println(" n");
  Serial.print(F("scale          ")); Serial.print(scale); Serial.println(" n");
  Serial.print(F("iScale         ")); Serial.print(iScale,6); Serial.println(" Iout per ADC lsb");
  Serial.print(F("vScale         ")); Serial.print(vScale,6); Serial.println(" Vout per ADC lsb");
  Serial.print(F("motScale       ")); Serial.print(motScale,6); Serial.println(" PWM per encod");
  Serial.print(F("emfScale       ")); Serial.print(emfScale,6); Serial.println(" PWM per Amp");
  Serial.print(F("plot      (P) ")); printOnOffln(plot);
  Serial.print(F("logging   (L) ")); printOnOffln(logging);
  Serial.print(F("control   (C) ")); printOnOffln(control);
  Serial.print(F("Enc=")); Serial.print(encod);
  Serial.print(F("  sw=")); Serial.print(sw);
  Serial.print(F("  V=")); Serial.print(V);
  Serial.print(F("  I=")); Serial.print(I);
  Serial.print(F("  EMF=")); Serial.print(Vemf);
  Serial.print(F("  PWM=")); Serial.print(iPWM);
  newln();
  }

void showPoints()
  {
  int ii;

  Serial.println();
  Serial.print("scale =   "); Serial.println(scale);
  Serial.print("scaleFast =   "); Serial.println(p.scaleFast);
  Serial.print("scaleSlow = "); Serial.println(p.scaleSlow);

  for (ii=1; ii<=99; ii++)
    {
    Serial.print(ii); Serial.print("   ");
    Serial.println(p.scaleFast*(98+scale)/((ii-1)+scale));
    }
  Serial.println();
  }

void setmotRes()
  {
  int ier;
  float fTemp;
  Serial.print("Enter Motor resistance (OHM)");
  ier = getFloat(& fTemp, 10000);
  if (ier > 0)
    {
    if (fTemp>0.0 && fTemp<10.0)
      {
      p.motRes = fTemp;
      }
    }
  if (ier<0) Serial.println();
  Serial.print(F("Motor resistance = "));
  Serial.println(p.motRes);
  }

void setvSupply()
  {
  int ier;
  float fTemp;
  Serial.print("Enter Supply voltage (V)");
  ier = getFloat(& fTemp, 10000);
  if (ier > 0)
    {
    if (fTemp>0.0 && fTemp<100.1)
      {
      p.vSupply = fTemp;
      }
    }
  if (ier<0) Serial.println();
  Serial.print(F("Supply voltage = "));
  Serial.println(p.vSupply);
  }

void setscaleFast()
  {
  int ier;
  float fTemp;
  Serial.print("Enter emf scaleFast");
  ier = getFloat(& fTemp, 10000);
  if (ier > 0)
    {
    if (fTemp>0.0 && fTemp<10.1)
      {
      p.scaleFast = fTemp;
      }
    }
  if (ier<0) Serial.println();
  Serial.print(F("scaleFast = "));
  Serial.println(p.scaleFast);
  }

void setPWMBot()
  {
  int ier;
  int iTemp;
  Serial.print("Enter Minimum PWM (0..255)"); Default(p.PWMBot);
  ier = getInt(& iTemp, 10000);
  if (ier > 0)
    {
    if (iTemp>0 && iTemp<256)
      {
      p.PWMBot = iTemp;
      }
    }
  if (ier<0) Serial.println();
  Serial.print(F("Minimum PWM = "));
  Serial.println(p.PWMBot);
  }

void setPWMTop()
  {
  int ier;
  int iTemp;
  Serial.print("Enter Maximum PWM (0..255)"); Default(p.PWMTop);
  ier = getInt(& iTemp, 10000);
  if (ier > 0)
    {
    if (iTemp>0 && iTemp<256)
      {
      p.PWMTop = iTemp;
      }
    }
  if (ier<0) Serial.println();
  Serial.print(F("Maximum PWM = "));
  Serial.println(p.PWMTop);
  }

void setADCOff()
  {
  int ier;
  int iTemp;
  Serial.print("Enter current ADC offset"); Default(p.ADCOff);
  ier = getInt(& iTemp, 10000);
  if (ier > 0)
    {
    if (iTemp<1024 && iTemp>-1024)
      {
      p.ADCOff = iTemp;
      }
    }
  if (ier<0) Serial.println();
  Serial.print(F("ADC offset = "));
  Serial.println(p.ADCOff);
  }

void setiGain()
  {
  int ier;
  float fTemp;
  Serial.print("Enter Current gain (Vout/Iin)"); Default(p.iGain);
  ier = getFloat(& fTemp, 10000);
  if (ier > 0)
    {
    if (fTemp>=0.0 && fTemp<=10.0)
      {
      p.iGain = fTemp;
      }
    }
  if (ier<0) Serial.println();
  Serial.print(F("Current gain = "));
  Serial.println(p.iGain);
  }

void setvGain()
  {
  int ier;
  float fTemp;
  Serial.print("Enter Voltage gain (Vadcin/Vmotor)"); Default(p.vGain);
  ier = getFloat(& fTemp, 10000);
  if (ier > 0)
    {
    if (fTemp>0.0 && fTemp<100.0)
      {
      p.vGain = fTemp;
      }
    }
  if (ier<0) Serial.println();
  Serial.print(F("Voltage gain = "));
  Serial.println(p.vGain);
  }

void setscaleSlow()
  {
  int ier;
  float fTemp;
  Serial.print("Enter scaleSlow"); Default(p.scaleSlow);
  ier = getFloat(& fTemp, 10000);
  if (ier > 0)
    {
    p.scaleSlow = fTemp;
    }
  if (ier<0) Serial.println();
  Serial.print(F("scaleSlow = "));
  Serial.println(p.scaleSlow);
  }
