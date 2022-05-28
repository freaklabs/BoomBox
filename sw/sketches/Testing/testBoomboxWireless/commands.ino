/**************************************************************************/
// Init command table   
/**************************************************************************/
void cmdTableInit()
{
    // add commands
    cmd.add("play", cmdPlay);
    cmd.add("sleep", cmdSleep);    
    cmd.add("vol", cmdSetVolume); 
    cmd.add("stop", cmdStop);   
    cmd.add("fwrite", cmdfWrite);
    cmd.add("fread", cmdfRead);
    cmd.add("settime", cmdSetDateTime);
    cmd.add("gettime", cmdGetDateTime);   
    cmd.add("5venb", cmd5VEnb); 
    cmd.add("setaddr", cmdSetAddr);
    cmd.add("getaddr", cmdGetAddr);  
    cmd.add("setdevid", cmdSetDevID);
    cmd.add("setdevsite", cmdSetDevSite);
    cmd.add("dump", cmdDumpMeta);
    cmd.add("send", cmdTransmit);
    cmd.add("ls", cmdSdLs);
    cmd.add("fwrite", cmdfWrite);
    cmd.add("fread", cmdfRead);
    cmd.add("setalarm", cmdSetAlarm);
    cmd.add("settimer", cmdSetTimer);
    cmd.add("batt", cmdBattery);
    cmd.add("solar", cmdSolar);    

}

/**************************************************************************/
// cmdSetAddr
/**************************************************************************/
void cmdSetAddr(int arg_cnt, char **args)
{
    getMeta();
    uint8_t addr;
    addr = cmd.conv(args[1], 16);
    lora.setThisAddress(addr);
    meta.devAddr = addr;
    printf_P(PSTR("Device set to address: %02X.\n"), lora.thisAddress());
    putMeta();
}

/**************************************************************************/
// cmdGetAddr
/**************************************************************************/
void cmdGetAddr(int arg_cnt, char **args)
{
    printf_P(PSTR("Device address: %02X.\n"), lora.thisAddress());
}

/**************************************************************************/
// cmdSetDevID
/**************************************************************************/
void cmdSetDevID(int argCnt, char **args)
{
    getMeta();
    memset(meta.devID, 0, sizeof(meta.devID));
    strCat((char *)buf, 1, argCnt, args);
    strncpy(meta.devID, (char *)buf, META_FIELDSIZE_MAX-1); 
    putMeta();
}

/**************************************************************************/
// cmdSetDevSite
/**************************************************************************/
void cmdSetDevSite(int argCnt, char **args)
{
    getMeta();
    memset(meta.devSite, 0, sizeof(meta.devSite));
    strCat((char *)buf, 1, argCnt, args);
    strncpy(meta.devSite, (char *)buf, META_FIELDSIZE_MAX-1); 
    putMeta();
}

/**************************************************************************/
// cmdDumpMeta
/**************************************************************************/
void cmdDumpMeta(int arg_cnt, char **args)
{
    dumpMeta();
}

/**************************************************************************/
// cmdBattery
/**************************************************************************/
void cmdBattery(int arg_cnt, char **args)
{
    printf_P(PSTR("Battery voltage: %.2f.\n"), calcBattery());
}

/**************************************************************************/
// cmdSolar
/**************************************************************************/
void cmdSolar(int arg_cnt, char **args)
{
    printf_P(PSTR("Solar voltage: %.2f.\n"), calcSolar());
}

/********************************************************************/
// 
/********************************************************************/
void cmd5VEnb(int argCnt, char **args)
{
    uint8_t enb = cmd.conv(args[1]);
    if (enb) 
        digitalWrite(bb.pin5VEnb, HIGH);
    else 
        digitalWrite(bb.pin5VEnb, LOW);
}

/**************************************************************************/
// cmdSleep
/**************************************************************************/
void cmdSleep(int argCnt, char **args)
{
    printf_P(PSTR("Going to sleep now...zzzz"));
    driver.sleep();
    delay(100);
    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);
    driver.setModeRx();
    printf_P(PSTR("Waking up now...."));    
}

/**************************************************************************/
// setDateTime
/**************************************************************************/
void cmdSetDateTime(int arg_cnt, char **args)
{
    uint8_t day, mon, year, hr, min, sec;

    year = strtol(args[1], NULL, 10);
    mon = strtol(args[2], NULL, 10);
    day = strtol(args[3], NULL, 10);
    hr = strtol(args[4], NULL, 10);
    min = strtol(args[5], NULL, 10);
    sec = strtol(args[6], NULL, 10);

    rtc.initClock();
    rtc.setDateTime(day, 0, mon, 0, year, hr, min, sec);
    
    printf_P(PSTR("Date: %s, Time: %s\n"), rtc.formatDate(), rtc.formatTime());
}

/**************************************************************************/
// getDateTime
/**************************************************************************/
void cmdGetDateTime(int arg_cnt, char **args)
{
    printf_P(PSTR("Date: %s, Time: %s\n"), rtc.formatDate(), rtc.formatTime());
}

/**************************************************************************/
// setAlarm
/**************************************************************************/
void cmdSetAlarm(int arg_cnt, char **args)
{
    uint8_t day = strtol(args[1], NULL, 10);
    uint8_t hour = strtol(args[2], NULL, 10);
    uint8_t min = strtol(args[3], NULL, 10);

    rtc.setAlarm(min, hour, day, 99);
    printf_P(PSTR("Alarm set for Day: %d, Hour: %d, Min: %d.\n"), day, hour, min);
}

/**************************************************************************/
// setTimer
/**************************************************************************/
void cmdSetTimer(int arg_cnt, char **args)
{
    uint8_t val = strtol(args[1], NULL, 10);
 //   uint8_t freq = strtol(args[2], NULL, 10);

    rtc.setTimer(val, 0x3, false);
    printf_P(PSTR("Timer set for %d minutes.\n"), val);
    rtc.enableTimer();
}

/**************************************************************************/
/*!
    Transmit data to another node wirelessly using Chibi stack.
    Usage: send <addr> <string...>
*/
/**************************************************************************/
void cmdTransmit(int argCnt, char **args)
{
    uint8_t addr, len, buf[100];

    memset(buf, 0, sizeof(buf));

    addr = cmd.conv(args[1], 16);
    len = strCat((char *)buf, 2, argCnt, args);

    printf("%s\n", (char *)buf);
        
    if (lora.sendtoWait(buf, len, addr))
    {
        printf_P(PSTR("Success. Message: %s transmitted.\n"), (char *)buf);
    }
    else
    {
        printf_P(PSTR("sendtoWait failed\n"));     
    }
}

/********************************************************************/
// sdList
/********************************************************************/
void cmdSdLs(int arg_cnt, char **args)
{
    sd.ls(LS_R);
}

/********************************************************************/
// fileRead
/********************************************************************/
void cmdfRead(int arg_cnt, char **args)
{
    if (sdAvailable)
    {
        printf_P(PSTR("Opening %s\n"), LOGFILE);
        
        myFile = sd.open(LOGFILE,  O_RDWR | O_CREAT | O_APPEND);
        
        if (!myFile)
        {
            printf_P(PSTR("Error opening %s\n"), args[1]);
            return;
        }
        
        while (myFile.available())
        {
          Serial.write(myFile.read());
        }
        myFile.close();        
    }
    else
    {
        printf_P(PSTR("No SD card available\n"));
    }    
}

/********************************************************************/
// sdFileWrite
/********************************************************************/
void cmdfWrite(int arg_cnt, char **args)
{
    if (sdAvailable)
    {
        char buf[100];
        printf_P(PSTR("Opening %s\n"), LOGFILE);
        
        myFile = sd.open(LOGFILE,  O_RDWR | O_CREAT | O_APPEND);
        if (!myFile)
        {
            printf_P(PSTR("Error opening %s\n"), args[1]);
            return;
        }
    
    //    printf("Creating data entry and writing to file: %s.\n", LOGFILE);
    //    printf("Format is date, time, temperature, humidity, battery.\n");
        sprintf(buf, "%s,%s,%0.2f\n", rtc.formatDate(), rtc.formatTime(), calcBattery());
    
        Serial.print(buf);
        myFile.print(buf); 
        myFile.close();        
    }
    else
    {
        printf("No SD card available\n");
    }
}

/************************************************************/
// Play track
// Usage: play <track number>
/************************************************************/
void cmdPlay(int arg_cnt, char **args)
{
  uint8_t track = cmd.conv(args[1]);
  if (track > MAX_SOUNDS)
  {
    track = MAX_SOUNDS-1;
  }
  bb.play(track);
}

/************************************************************/
// Set volume
// Usage: vol <volume level>
// volume level is between 0 and 30
/************************************************************/
void cmdSetVolume(int arg_cnt, char **args)
{
  uint8_t vol = cmd.conv(args[1]);
  if (vol > 30)
  {
    vol = 30;
  }
  bb.setVol(vol);
}

/************************************************************/
// Stop playing
/************************************************************/
void cmdStop(int arg_cnt, char **args)
{
  bb.stop();
}
