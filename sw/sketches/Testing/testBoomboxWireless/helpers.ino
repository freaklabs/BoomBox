/********************************************************************/
//  DevMode - allows system to go into developer mode rather than normal operating mode
/********************************************************************/
void selectMode()
{   
    uint32_t now = millis();;
     
    printf_P(PSTR("Press 'c' to go into command mode\n"));
    while ((millis() - now) < START_WAIT_TIME)
    {
      if (Serial.available() > 0)
      {
        if (Serial.read() == 'c')
        {
            normalMode = false;
            printf_P(PSTR("Type 'normal' to go back to normal mode\n"));
            printf_P(PSTR("Command line mode times out in %d minutes"), CMD_MODE_TIME_LIMIT);
            cmdModeTimeCnt = millis();
            return;
        }
      }
    }
    normalMode = true;
    Serial.println("Going into normal operation mode.");
    Serial.flush();
}


/**************************************************************************/
// 
/**************************************************************************/
void showBanner()
{
    //Serial.println(F("-------------------------------------------------"));
    Serial.println(F("Boombox Wireless"));
    Serial.println(F("Wireless Automated Behavioral Response System"));
    Serial.println(F("Designed by FreakLabs"));
    //Serial.println(F("-------------------------------------------------"));
}

/**************************************************************************/
/*!
  Calculate Battery Voltage
*/
/**************************************************************************/
double calcBattery()
{
    return bb.getVbat();
}


/**************************************************************************/
/*!
  Calculate Battery Voltage
*/
/**************************************************************************/
double calcSolar()
{
    return bb.getVsol();
}

/************************************************************************/
//    
//    
/************************************************************************/
ts_t getTime()
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

/**************************************************************************/
// 
/**************************************************************************/
char *printTimeAndDate()
{   
    memset(bufTime, 0, sizeof(bufTime));
    
    ts_t time = getTime();  
    memset(bufTime, 0, sizeof(bufTime));
    sprintf(bufTime, "%04d/%02d/%02d %02d:%02d:%02d", time.year, time.mon, time.mday, time.hour, time.min, time.sec);  
    return bufTime;
}

/**************************************************************************/
// 
/**************************************************************************/
void logEvent(const char *msg)
{
    memset(tmp, 0, sizeof(tmp));
    if (myFile.open(LOGFILE, O_RDWR | O_CREAT | O_APPEND))
    {
        // don't turn off interrupts if I2C will be used
        sprintf(tmp, "%s: %s", printTimeAndDate(), msg);

        myFile.print(tmp);
        myFile.close();
    }
    else
    {
        Serial.println("logEvent - SD File error or not present");
    }
    Serial.println(tmp);
}

/**************************************************************************/
// 
/**************************************************************************/
void logError(const char *msg)
{
    memset(tmp, 0, sizeof(tmp));
    
    if (myFile.open(LOGFILE, O_RDWR | O_CREAT | O_APPEND))
    {
        // don't turn off interrupts if I2C will be used
        sprintf(tmp, "%s\n", msg);
        
        myFile.print(tmp);
        myFile.close();
    }
    else
    {
        Serial.println("logError - SD File error or not present");
    }
    Serial.println(tmp); // moved print outside of the interrupt stoppage
}

/**************************************************************************/
// 
/**************************************************************************/
void logData(const char *msg)
{
    memset(tmp, 0, sizeof(tmp));

    if (myFile.open(DATAFILE, O_RDWR | O_CREAT | O_APPEND))
    {
        // don't turn off interrupts if I2C will be used
        sprintf(tmp, "%s: %s", printTimeAndDate(), msg);
        
        myFile.print(tmp);
        myFile.close();
    }
    else
    {
        Serial.println("logError - SD File error or not present");
    }
    Serial.println(tmp); // moved print outside of the interrupt stoppage
}


/**************************************************************************/
/*!
    Get metadata
*/
/**************************************************************************/
void getMeta()
{  
    EEPROM.get(METADATA_EEPROM_LOC, meta);
}

/**************************************************************************/
/*!
*/
/**************************************************************************/
void putMeta()
{  
    EEPROM.put(METADATA_EEPROM_LOC, meta);
}

/**************************************************************************/
/*!
*/
/**************************************************************************/
void dumpMeta()
{   
    printf_P(PSTR("Device Addr:\t%02X\n"), meta.devAddr);
    printf_P(PSTR("Device ID:     \t%s\n"), meta.devID);
    printf_P(PSTR("Device Site:   \t%s\n"), meta.devSite);   
}

/**************************************************************************/
// 
/**************************************************************************/
void sdDateTime(uint16_t *date, uint16_t *time)
{
    rtc.getDateTime();

    // return date using FAT_DATE macro to format fields
    *date = FAT_DATE(rtc.getYear(), rtc.getMonth(), rtc.getDay());

    // return time using FAT_TIME macro to format fields
    *time = FAT_TIME(rtc.getHour(), rtc.getMinute(), rtc.getSecond());
}

/**************************************************************************/
/*!
    Concatenate multiple strings from the command line starting from the
    given index into one long string separated by spaces.
*/
/**************************************************************************/
int strCat(char *buf, unsigned char index, char arg_cnt, char **args)
{
  uint8_t i, len;
  char *data_ptr;

  data_ptr = buf;
  for (i = 0; i < arg_cnt - index; i++)
  {
    len = strlen(args[i + index]);
    strcpy((char *)data_ptr, (char *)args[i + index]);
    data_ptr += len;
    *data_ptr++ = ' ';
  }
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
static int uart_putchar (char c, FILE *stream)
{
    Serial.write(c);
    return 0;
}   
