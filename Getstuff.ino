/*
* File: Getstuff.ino
* Origin: 29-Jan-2022
* Author: Tip Partridge
* Descrition:
*   User input routines for string, y/n,  int, float
* Revision History:
*   19-Oct-2023 (TEP) Return null string for timeout
*   19-Jan-2025 (TEP) Use .write instead of .print to fix BS handling. Return -2 for abort.
*                     No newline for timeout.
*   
*/
#define null 0
#define bs 8
#define lf 10
#define cr 13
#define ctrlz 26
#define esc 27
#define del 127

//******** getStr *********************************
int getStr( char * str, int maxLen, int mode, const unsigned long timeout)
// Get ASCII string from Serial.
// Returns number of characters in string or -1 for timeout or -2 for abort.
// str = pointer to null terminated return string of type char[size].
// maxLen = maximum desired string length, size-1 maximum.
// mode: 0 = ASCII, 1 = integer, 2 = float.
// timeout = inactivity timeout in ms. Null string returned for timeout.
// backspace function implemented for bs or del keys.
  {
  unsigned long time0;    // Start time for timeout.
  unsigned long timeNow;  // current time.
  char c;                 // current character.
  int cpnt = 0;           // index into str.
  str[0] = null;          // init str to zero length null terminated string.
  while (true)            // function is terminated by return statement.
    {
    time0 = millis();
    while (!Serial.available())
      {
      timeNow = millis();
      if (timeNow - time0 > timeout)
        {
///        Serial.println();
        str[0] = null;
        return -1;
        }
      }
    c = Serial.read();
    if (
      (mode == 0  && cpnt < maxLen && c >= ' ' && c <= '~') ||
      (mode == 1  && cpnt < maxLen && ((c >= '0' && c <= '9') || (c == '-' && cpnt == 0))) ||
      (mode == 2  && cpnt < maxLen && ((c >= '0' && c <= '9') || c == '.' || c == ',' || (c == '-' && cpnt == 0)))
      )
      {
      str[cpnt++] = c;
      Serial.write( c);
      }
    else if ((c == bs || c == del) && cpnt > 0)
      {
      Serial.write( bs);
      Serial.write( ' ');
      Serial.write( bs);
      cpnt -= 1;
      }
    else if (c == ctrlz || c == esc)
      {
      str[0] = null;
      return( -2);
      }
    else if (c == cr || c == lf)
      {
      Serial.println();
      if (cpnt >= 0)
        {
        str[cpnt] = null;   // null terminate string
        return( cpnt);
        }
      else return( 0);
      }
    }
  }

//********* getChr *********************************

char getChr( const unsigned long timeout)
// Return a character, NULL for timeout.
  {
  unsigned long time0;    // Start time for timeout.
  unsigned long timeNow;  // current time.
  char c;                 // current character.
  while (true)            // function is terminated by return statement.
    {
    time0 = millis();
    while (!Serial.available())
      {
      timeNow = millis();
      if (timeNow - time0 > timeout) return 0;
      }
    c = Serial.read();
    if (c >= ' ') Serial.write( c);
    return c;
    }
  }

//********* getYN *********************************

int getYN( const unsigned long timeout)
// return 0 for No
// return 1 for Yes
// return 2 for none
// return -1 for timeout or invalid input.
// only first character is checked.
  {
  char ynstr[15];      // String from getStr
  int nchr;           // number of chars from getStr
  nchr = getStr( ynstr, 14, 0, timeout);
  if (ynstr[0] == 'N' || ynstr[0] == 'n') return(0);
  else if (ynstr[0] == 'Y' || ynstr[0] == 'y') return(1);
  else if (nchr==0) return(2);
  else return(-1);
  }

//********* getInt *********************************

int getInt( int * anint, const unsigned long timeout)
// Get integer number from Serial.
// Returns number of digits input or -1 for timeout.
// anint = pointer to integer return. Unchanged if none entered.
// timeout = inactivity timeout in ms.
// backspace function implemented for bs or del keys.
// Non-numeric characters are skipped.
  {
  char istr[15];      // String from getStr
  int nchr;           // number of chars from getStr
  int cpnt;           // index into istr
  char c;             // current char
  bool Neg = false;   // TRUE if we get a minus sign
  int rtnVal = 0;     // Value returned in anint
  nchr = getStr( istr, 14, 1, timeout);
  if (nchr > 0)
    {
    for (cpnt = 0; cpnt < nchr; cpnt++)
      {
      c = istr[cpnt];
      if (c >= '0' && c <= '9')
        {
        c = c - '0';
        rtnVal = rtnVal * 10 + c;
        }
      else if (cpnt == 0 && c == '-') Neg = true;
      }
    if (Neg) *anint = -1 * rtnVal;
    else  *anint = rtnVal;
    }
  return( nchr);
  }

//********* getLng *********************************

int getLng( long * anint, const unsigned long timeout)
// Get long integer number from Serial.
// Returns number of digits input or -1 for timeout.
// anint = pointer to integer return. Unchanged if none entered.
// timeout = inactivity timeout in ms.
// backspace function implemented for bs or del keys.
// Non-numeric characters are skipped.
  {
  char istr[15];      // String from getStr
  int nchr;           // number of chars from getStr
  int cpnt;           // index into istr
  char c;             // current char
  bool Neg = false;   // TRUE if we get a minus sign
  long rtnVal = 0;     // Value returned in anint
  nchr = getStr( istr, 14, 1, timeout);
  if (nchr > 0)
    {
    for (cpnt = 0; cpnt < nchr; cpnt++)
      {
      c = istr[cpnt];
      if (c >= '0' && c <= '9')
        {
        c = c - '0';
        rtnVal = rtnVal * 10 + c;
        }
      else if (cpnt == 0 && c == '-') Neg = true;
      }
    if (Neg) *anint = -1 * rtnVal;
    else  *anint = rtnVal;
    }
  return( nchr);
  }

//********* getFloat *********************************

int getFloat( float * aflt, const unsigned long timeout)
// Get floating point number from Serial.
// Returns number of digits input or -1 for timeout.
// aflt = pointer to float return. Unchanged if none entered.
// timeout = inactivity timeout in ms.
// backspace function implemented for bs or del keys.
// Non-numeric characters are skipped.
  {
  char fstr[15];      // String from getStr
  int nchr;           // number of chars from getStr
  int cpnt;           // index into fstr
  char c;             // current char
  bool Neg = false;   // TRUE if we get a minus sign
  bool Dot = false;   // TRUE once we get a decimal point
  float Mult;         // power of 10 multiplier for fractional part of float
  float rtnVal = 0.0; // Value returned in aflt
  nchr = getStr( fstr, 14, 2, timeout);
  if (nchr > 0)
    {
    for (cpnt = 0; cpnt < nchr; cpnt++)
      {
      c = fstr[cpnt];
      if (c >= '0' && c <= '9')
        {
        c = c - '0';
        if (!Dot) rtnVal = rtnVal * 10.0 + c;
        else
          {
          rtnVal = rtnVal + Mult * c;
          Mult /= 10.0;
          }
        }
      else if ((c == '.' || c == ',') && !Dot)
        {
        Mult = 0.1;
        Dot = true;
        }
      else if (cpnt == 0 && c == '-') Neg = true;
      }
    if (Neg) *aflt = -1.0 * rtnVal;
    else  *aflt = rtnVal;
    }
  return( nchr);
  }

// Getstuff
