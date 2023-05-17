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
//    
//    
/************************************************************************/
#if (BOOMBOX == 1)
ts_t rtcGetTime()
{
    ts_t time;
    memset(&time, 0, sizeof(time));

    rtc.getDateTime();
    time.year = 2000 + rtc.getYear();
    time.mon = rtc.getMonth();
    time.mday = rtc.getDay();
    time.hour = rtc.getHour();
    time.min = rtc.getMinute();
    time.sec = rtc.getSecond();
    return time;   
}

/************************************************************************/
//    
//    
/************************************************************************/
bool withinActiveTime(uint8_t timeHour, uint8_t timeMin) 
{
    uint16_t target, hour, min, startTime, endTime, duration, timeOn;
    
    // calculates miinutes fro midnight for target
    target = timeHour * 60 + timeMin;
     
    // calculate onTime and offTime as minutes from midnight
    startTime = (meta.onTime.hour * 60) + meta.onTime.min;
    endTime = (meta.offTime.hour * 60) + meta.offTime.min;

    // calculate total duration of active period
    duration = (endTime - startTime + MIN_PER_DAY) % MIN_PER_DAY;

    // check if our current time falls within the onTime
    timeOn = (target - startTime + MIN_PER_DAY) % MIN_PER_DAY;

    printf_P(PSTR("Target: %d, startTime: %d, endTime: %d, timeon: %d, duration: %d\n"), target, startTime, endTime, timeOn, duration);
    Serial.flush();

    // check if we're within our active duration
    if (timeOn < duration) 
    {
        return 1;  //within interval
    }
    else
    {
        return 0;  //not within interval
    }
}
#endif  

/**************************************************************************/
// 
/**************************************************************************/
char *rtcPrintTimeAndDate()
{
#if (BOOMBOX == 1)  
    ts_t time = rtcGetTime();
    memset(bufTime, 0, sizeof(bufTime));
    sprintf(bufTime, "%04d/%02d/%02d %02d:%02d:%02d", time.year, time.mon, time.mday, time.hour, time.min, time.sec);
#else
    sprintf(bufTime, "<unavailable>");
#endif    
    return bufTime;
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
int uart_putchar (char c, FILE *stream)
{
    (void) stream;
    Serial.write(c);
    return 0;
}
