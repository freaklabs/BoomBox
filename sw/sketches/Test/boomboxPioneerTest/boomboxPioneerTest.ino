#include <SdFat.h>
#include <Rtc_Pcf8563.h>
#include <cmdArduino.h>
#include <boomboxAdvanced.h>
#include <avr/wdt.h>

// error codes
#define ERROR_SD_INIT       2
#define ERROR_LOW_BATTERY   3
#define ERROR_FILE_OPEN     4

// logfile
#define LOGFILE "LOGFILE.TXT"
#define ERRORFILE "ERRFILE.TXT"

#define MAX_SOUNDS 8

// create objects
SdFat sd;
File myFile;
File myErrorFile;
Rtc_Pcf8563 rtc;
FILE uartout = {0}; 

uint8_t index = 0;
uint32_t offDelayTime = 0;
uint8_t pinSdSelN = 15;

/************************************************************/
// SETUP
/************************************************************/
void setup() 
{
    // for printf
    fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
    stdout = &uartout ;
    bb.init();
    
    cmd.begin(57600);

    cmd.add("play", cmdPlay);
    cmd.add("sleep", cmdSleep);    
    cmd.add("vol", cmdSetVolume); 
    cmd.add("stop", cmdStop);   
    cmd.add("fwrite", cmdfWrite);
    cmd.add("fread", cmdfRead);
    cmd.add("settime", cmdSetDateTime);
    cmd.add("gettime", cmdGetDateTime);   
    cmd.add("5venb", cmd5VEnb); 

    // init SD card
    if (sd.begin(pinSdSelN) != true)
    {
        Serial.println("No SD Card");
/*
        for (int i=0; i<3; i++)
        {
            errorHandler(ERROR_SD_INIT);
            delay(1000);  
        }
*/        
    }

    // set up the file system callback so that the file can be date and timestamped
    myFile.dateTimeCallback(sdDateTime);         
}

/************************************************************/
// LOOP
/************************************************************/
void loop() 
{
    cmd.poll();

    if (bb.isAuxEvent() == true)
    {    
        // button has been pushed 
        Serial.println("Trigger event.");
        
        // play sound. 
        if (index < MAX_SOUNDS)
        {        
                index++;
                Serial.print("Index: ");
                Serial.println(index);
            }
            else
            {
                index = 1;
                Serial.print("Index: ");
                Serial.println(index);
        }
        bb.play(index);
        
        delay(offDelayTime);
        bb.clearAuxFlag();
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

/**************************************************************************/
//
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
    
    printf("Date: %s, Time: %s\n", rtc.formatDate(), rtc.formatTime());
}

/**************************************************************************/
//
/**************************************************************************/
void cmdGetDateTime(int arg_cnt, char **args)
{
    printf("Date: %s, Time: %s\n", rtc.formatDate(), rtc.formatTime());
}

/********************************************************************/
// SD Card list directory
/********************************************************************/
void cmdSdLs(int arg_cnt, char **args)
{
    sd.ls(LS_R);
}

/********************************************************************/
// SD Card read file
/********************************************************************/
void cmdfRead(int arg_cnt, char **args)
{
    printf("Opening %s\n", LOGFILE);
    
    myFile = sd.open(LOGFILE,  O_RDWR | O_CREAT | O_APPEND);
    
    if (!myFile)
    {
        printf("Error opening %s\n", args[1]);
        return;
    }
    
    while (myFile.available())
    {
      Serial.write(myFile.read());
    }
    myFile.close();
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

/********************************************************************/
// SD Card write file
/********************************************************************/
void cmdfWrite(int arg_cnt, char **args)
{
    char buf[200];
    printf( "Opening %s\n", LOGFILE);

    myFile = sd.open(LOGFILE,  O_RDWR | O_CREAT | O_APPEND);
    if (!myFile)
    {
        printf("Error opening %s\n", args[1]);
        return;
    }

//    printf("Creating data entry and writing to file: %s.\n", LOGFILE);
//    printf("Format is date, time, temperature, humidity, battery.\n");
    sprintf(buf, "%s,%s,%0.2f\n", rtc.formatDate(), rtc.formatTime(), bb.getVbat());

    Serial.print(buf);
    myFile.print(buf); 
    myFile.close();
}

/************************************************************/
// Go into hibernation mode
/************************************************************/
void cmdSleep(int arg_cnt, char **args)
{
  bb.sleep();
  //LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);

  // need to wake up if you sleep. 
  // can only wake up from external interrupt, 
  // ie: button push or motion event
  bb.wake();
}

// flash led error codes and log errors
void errorHandler(int error)
{
    // display error
    if (error == ERROR_SD_INIT)
    {
        Serial.println("ERROR - No SD Card inserted");
    }
    else if (error == ERROR_LOW_BATTERY)
    {
        Serial.println("ERROR - Low Battery.");
    }
    else if (error == ERROR_FILE_OPEN)
    {
        Serial.println("ERROR - File cannot be opened.");
    }

/*
    // flash error code
    for (int i=0; i<error; i++)
    {
        digitalWrite(pinLED, HIGH);
        delay(LED_FLASH_TIME);
        digitalWrite(pinLED, LOW);
        delay(LED_FLASH_TIME);
    }
    delay(LED_OFF_TIME);
*/
    // write error to logfile
    if (error == ERROR_SD_INIT)
    {
        return;
    }
    else
    {
        myErrorFile = sd.open(ERRORFILE, O_WRITE | O_CREAT | O_APPEND);
        if (myErrorFile == true)
        {
            myErrorFile.print(rtc.formatDate());
            myErrorFile.print(",");
            myErrorFile.print(rtc.formatTime());
            myErrorFile.print(",");
            if (error == ERROR_LOW_BATTERY)
            {
                myErrorFile.println("Error - Low battery event.");
            }
            else if (error == ERROR_FILE_OPEN)
            {
               myErrorFile.println("Error - Logfile cannot be opened."); 
            }
            myErrorFile.close();
        }
    }
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
// This is to implement the printf function from within arduino
/**************************************************************************/
int uart_putchar (char c, FILE *stream)
{
    Serial.write(c);
    return 0;
}
