/*
File:      HSMDsub.ino
Origin:    05-Mar-2025
Author:    tip partridge
Description: Various lower level subroutines for program HSMDriver
*/

//********************************************************
// calcScales - Calculate scale factors for ADC and PWM
//********************************************************
void calcScales()
  {
  scale = 98/(p.scaleSlow/p.scaleFast-1);
  motScale = float(p.PWMTop-p.PWMBot)/98.0; // calculate PWM ticks per encoder tick
  emfScale0 = p.motRes/p.vSupply*255.0;     // PWM per Amp.
  emfScale = emfScale0 * p.scaleFast * ((99+scale)/(encod+scale));
  vScale = ADCRef / 1023.0 * p.vGain;       // Vout per ADC lsb
  iScale = ADCRef / 1023.0 * p.iGain;       // Iout per ADC lsb
  }

//********************************************************
// setG2D - Set runtime Parameters to default values
//********************************************************
void setG2D( Parameters * gg)
  {
  gg->motRes = dmotRes;
  gg->vSupply = dvSupply;
  gg->PWMBot = dPWMBot;
  gg->PWMTop = dPWMTop;
  gg->iGain = diGain;
  gg->vGain = dvGain;
  gg->ADCOff = dADCOff;
  gg->scaleFast = dscaleFast;
  gg->scaleSlow = dscaleSlow;
  }

//********************************************************
// getE2P - Copy parameters from EEPROM to global variables
//********************************************************
void getE2P()
  {
  EEPROM.get( parametersAddr, p);
  }

//**********************************************************
// putP2E - Write runtime Parameters to EEPROM
//**********************************************************
void putP2E()
  {
  EEPROM.put( EEInitAddr, EEInitFlag);
  EEPROM.put( parametersAddr, p);
  }

//**********************************************************
// putD2P - Set EEPROM parameters to default values
//**********************************************************
void putD2E()
  {
  Parameters gg;
  setG2D( & gg);
  EEPROM.put( EEInitAddr, EEInitFlag);
  EEPROM.put( parametersAddr, gg);
  }

//**********************************************************

void newln()
  {
  Serial.println();
  }

//**********************************************************

void printOnOffln( bool state)
  {
  Serial.print(" ");
  if (state) Serial.println(F("On "));
  else  Serial.println(F("Off"));
  }

//**********************************************************

//**********************************************************
// Print formatted default value
//**********************************************************
void Default( int dflt)
  {
  Serial.print(F(" ["));
  Serial.print(dflt);
  Serial.print(F("]: "));
  }
void Default( unsigned int dflt)
  {
  Serial.print(F(" ["));
  Serial.print(dflt);
  Serial.print(F("]: "));
  }
void Default( float dflt)
  {
  Serial.print(F(" ["));
  Serial.print(dflt);
  Serial.print(F("]: "));
  }
void Default( uint8_t dflt)
  {
  Serial.print(F(" ["));
  Serial.print(dflt);
  Serial.print(F("]: "));
  }
void Default( char * dflt)
  {
  Serial.print(F(" ["));
  Serial.print(dflt);
  Serial.print(F("]: "));
  }

//**********************************************************
