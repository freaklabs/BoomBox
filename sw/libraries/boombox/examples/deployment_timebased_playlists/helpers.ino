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


#if (BOOMBOX == 1)
/************************************************************/
// Helper function to initialize the playlist
/************************************************************/
void initPlaylists()
{
    for (int i=0; i<NUM_PLAYLISTS; i++)
    {
        playlist[i] = (uint8_t *)malloc(meta.numSounds[i]);    
        if (!playlist[i])
        {
            Serial.println(F("ERROR: playlist init failed"));
            Serial.flush();
            return;
        }
    }    

    // create sequential playlist with indices that start from 1
    for (uint8_t i = 0; i<meta.numSounds[0]; i++)
    {
        playlist[0][i] = i+1;
    }

    for (uint8_t i = 0; i<meta.numSounds[1]; i++)
    {
        playlist[1][i] = meta.numSounds[0]+i+1;
    } 

    if (meta.shuffleEnable)
    {
        for (int i=0; i<NUM_PLAYLISTS; i++)
        {   
            shufflePlaylist(i);
        }
    }
}

/************************************************************/
// Shuffle playlist
/************************************************************/
void shufflePlaylist(uint8_t listNum)
{
    uint16_t i, nvalues, startIndex;
    uint8_t *list = NULL;

    if (listNum > NUM_PLAYLISTS-1)
    {
        Serial.println(F("Exceeded max playlists."));
        return;
    }

    nvalues = meta.numSounds[listNum];
    startIndex = listNum ? meta.numSounds[0] : 0;
    list = playlist[listNum];

    if (!list)
    {
        Serial.println(F("ERROR: playlist is uninitialized"));
        Serial.flush();
        return;
    }

    // create sequential playlist with indices that start from 1
    for (i = 0; i<nvalues; i++)
    {
        list[i] = startIndex+i+1;
    }

    //Serial.println(F("Shuffling playlist"));

    // shuffle playlist
    for(i = 0; i < nvalues-1; i++) 
    {
        uint16_t c = random(nvalues-i);

        /* swap */
        uint16_t t = list[i]; 
        list[i] = list[i+c]; 
        list[i+c] = t;    
    }
    //dumpPlaylist(0);
    //dumpPlaylist(1);
}

/************************************************************************/
//  check if the realtime clock time is within the correct boundaries  
/************************************************************************/
bool playlistChooser(uint8_t timeHour, uint8_t timeMin) 
{
    uint16_t target, list0Time, list1Time, duration, timeOn;
    
    // calculates miinutes from midnight for target
    target = timeHour * 60 + timeMin;
     
    // calculate onTime and offTime as minutes from midnight
    list0Time = (meta.playListTime[0].hour * 60) + meta.playListTime[0].min;
    list1Time = (meta.playListTime[1].hour * 60) + meta.playListTime[1].min;

    // calculate total duration of active period
    duration = (list1Time - list0Time + MIN_PER_DAY) % MIN_PER_DAY;

    // check if our current time falls within the onTime
    timeOn = (target - list0Time + MIN_PER_DAY) % MIN_PER_DAY;

    //printf_P(PSTR("Target: %d, list0Time: %d, list1Time: %d, timeon: %d, duration: %d\n"), target, list0Time, list1Time, timeOn, duration);
    Serial.flush();

    // check if we're within our active duration
    if (timeOn < duration) 
    {
        return 0;  // playlist 0
    }
    else
    {
        return 1;  //playlist 1
    }
}

/************************************************************/
// 
/************************************************************/
uint8_t getNextSound(uint8_t listNum)
{
    uint8_t retVal;
    uint8_t *list;

    // if we've reached the end of the playlist, 
    // shuffle playlist and then restart               
    if (index > meta.numSounds[listNum]-1)
    {
        index = 0;

        if (meta.shuffleEnable)
        {
            shufflePlaylist(listNum);
        }
    }
    list = playlist[listNum];
    retVal = list[index];

    // print out index and value   
    printf_P(PSTR("Index: %d, Val: %d\n"), index, list[index]);
    index++; 
    
    return retVal;
}

/************************************************************************/
//  dump playlist  
/************************************************************************/
void dumpPlaylist(uint8_t listNum)
{
    uint8_t *list;
    list = playlist[listNum];
    
    for (int i=0; i<meta.numSounds[listNum]; i++)
    {
        Serial.print(list[i]);
        Serial.print(", ");
    }
    Serial.println();
}
#endif  

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
