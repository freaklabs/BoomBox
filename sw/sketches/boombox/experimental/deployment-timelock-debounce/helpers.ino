/********************************************************************/
//  DevMode - allows system to go into developer mode rather than normal operating mode
/********************************************************************/
void selectMode()
{   
    uint32_t now = millis();;
     
    Serial.println(F("Press 'c' to go into command mode"));
    while ((millis() - now) < START_WAIT_TIME)
    {
      if (Serial.available() > 0)
      {
        if (Serial.read() == 'c')
        {
            normalMode = false;
            Serial.println(F("Type 'normal' to go back to normal mode"));
            Serial.println(F("Command line mode times out in 5 minutes"));
            cmdModeTimeCnt = millis();
            return;
        }
      }
    }
    normalMode = true;
    Serial.println(F("Going into normal operation mode."));
    Serial.flush();
}

/************************************************************************/
//   printConfig 
//    
/************************************************************************/
void printConfig()
{
    printf_P(PSTR("Device Name: \t%s\n"), meta.devName);
    printf_P(PSTR("Device ID:   \t%d\n"), meta.devID);
    printf_P(PSTR("Shuffle:     \t%s\n"), meta.shuffleEnable ? "TRUE" : "FALSE");
    printf_P(PSTR("Device Mode: \t%s\n"), meta.devMode ? "TRAILCAM" : "STANDALONE");
    printf_P(PSTR("Max Sounds:  \t%d\n"), meta.maxSounds);
    printf_P(PSTR("Delay Time:  \t%d\n"), meta.delayTime);
    printf_P(PSTR("Off Delay:   \t%d\n"), meta.offDelayTime);
    printf_P(PSTR("Debounce:   \t%s\n"), meta.debounceEnb ? "TRUE" : "FALSE");
    printf_P(PSTR("Debounce Time:  %d\n"), meta.debounceTime);
    printf_P(PSTR("Timelock:   \t%s\n"), meta.timelockEnb ? "TRUE" : "FALSE");
    printf_P(PSTR("Time Off:   \t%d\n"), meta.timelockoff);
    printf_P(PSTR("Time On:   \t%d\n"), meta.timelockon); 
}

/************************************************************************/
//    
//    
/************************************************************************/
bool checkTimelock(uint8_t hour)
{
    // based on a 24 hour format
    if (meta.timelockEnb)
    {
        if ((hour > meta.timelockoff) && (hour < meta.timelockon))
        {
            // we are in the lockout period. Don't play any sounds
            Serial.println(F("Playback disabled due to timelock"));
            return false;
        }
        else if ((hour > meta.timelockon) || (hour < meta.timelockoff))
        {
            Serial.println(F("Playback enabled, timelock is off"));
            return true;
        }
    }
    else
    {
        return true;
    }
    return true;
}

/************************************************************************/
//    
//    
/************************************************************************/
bool checkValidTrigger(uint16_t debounceTime)
{
    uint8_t trigValue = 0;
    
    // Debounce signal
    // if debounce is enabled, and if signal is still high, then continue
    // else don't allow playback
    if (meta.debounceEnb == true)
    {
        delay(debounceTime);
        trigValue = digitalRead(pinTrigIntp);
        if (trigValue == 1)
        {
            // valid signal, allow playback
            Serial.println(F("Valid trigger pulse detected. Allowing playback."));
            return true;
        }
        else
        {
            // this is a 30 minute pulse and not an actual trigger
            Serial.println(F("Non-trigger pulse detected. Not allowing playback."));
            return false;
        }
    }
    else
    {
        // if debounce is not enabled, then just allow sound playback by default
        return true;
    }
}

/**************************************************************************/
/*!
    Concatenate multiple strings from the command line starting from the
    given index into one long string separated by spaces.
*/
/**************************************************************************/
int strCat(char *buf, unsigned char index, char argCnt, char **args)
{
    uint8_t i, len;
    char *data_ptr;

    data_ptr = buf;
    for (i=0; i<argCnt - index; i++)
    {
        len = strlen(args[i+index]);
        strcpy((char *)data_ptr, (char *)args[i+index]);
        data_ptr += len;
        *data_ptr++ = ' ';
    }
    // remove the trailing space
    data_ptr--;
    *data_ptr++ = '\0';

    return data_ptr - buf;
}

/************************************************************************/
// Calculate free RAM
/************************************************************************/
int freeMemory() {
  char top;
  return &top - __brkval;
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
}

/************************************************************************/
// elapsedTime - calculates time elapsed from startTime
// startTime : time to start calculating
/************************************************************************/
uint32_t elapsedTime(uint32_t startTime)
{
  uint32_t stopTime = millis();
  
  if (stopTime >= startTime)
  {
    return stopTime - startTime;
  }
  else
  {
    return (ULONG_MAX - (startTime - stopTime));
  }
}

/**************************************************************************/
// This is to implement the printf function from within arduino
/**************************************************************************/
int uart_putchar (char c, FILE *stream)
{
    (void) stream;
    Serial.write(c);
    return 0;
}
