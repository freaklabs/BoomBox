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

/**************************************************************************/
/*!
*/
/**************************************************************************/
void dumpPlaylist()
{
    if (boombox._playlist != NULL)
    {
        printf_P(PSTR("Playlist: \n"));
        for (int i=0; i<meta.maxSounds; i++)
        {
            printf_P(PSTR("Index: %d, Val: %d.\n"), i, boombox._playlist[i]);
        }        
    }
}

/**************************************************************************/
/*!
*/
/**************************************************************************/
bool checkPlaybackAllowed(uint8_t hour)
{
    if (meta.stopTime > meta.startTime)
    {
        if ((hour >= meta.startTime) && (hour < meta.stopTime))
        {
            return true;      
        }        
    }
    else if (meta.stopTime < meta.startTime)
    {
        if (((hour >= meta.startTime) && hour > meta.stopTime) || ((hour < meta.startTime) && (hour < meta.stopTime)))
        {
            return true;
        }       
    }
    else
    {
        // in case they are equal, consider that playback always allowed
        return true;
    }
    return false;  
}

/************************************************************************/
// Calculate free RAM
/************************************************************************/
int freeMemory() {
  char top;
  return &top - __brkval;
  return __brkval ? &top - __brkval : &top - __malloc_heap_start;
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
int uart_putchar(char c, FILE *stream)
{
    (void) stream;
    Serial.write(c);
    return 0;
}
