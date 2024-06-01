/********************************************************************/
//  DevMode - allows system to go into developer mode rather than normal operating mode
/********************************************************************/
void selectMode()
{   
    uint32_t now = millis();;
     
    Serial.println("Press 'c' to go into command mode");
    while ((millis() - now) < START_WAIT_TIME)
    {
      if (Serial.available() > 0)
      {
        if (Serial.read() == 'c')
        {
            normalMode = false;
            printf_P(PSTR("Type 'normal' to go back to normal mode"));
            printf_P(PSTR("Command line mode times out in %d minutes"), CMD_MODE_TIME_LIMIT);
            cmdModeTimeCnt = millis();
            return;
        }
      }
    }
    normalMode = true;
    intervalMinutes = 0;
    Serial.println("Going into normal operation mode.");
    Serial.flush();
}

/**************************************************************************/
// 
/**************************************************************************/
void playSound()
{
    wdt_disable();
    
    bbb.wake();  

    // delay for delayTime milliseconds after trigger has happened. 
    // This delays playing the sound immediately after trigger 
    delay(delayTime);
        
    if (index < MAX_SOUNDS)
    {
        index++;        
    }
    else
    {
        index = 1;
    }
    Serial.print("Playing index: ");
    Serial.println(index);

    // enable amp
    bbb.ampEnable();
    delay(AMP_ENABLE_DELAY);

    // play music here
    bbb.play(index);   

    // delay for durationTime milliseconds. should be adjusted to 
    // longest sample that will be played
    delay(durationTime);    

    // disable amp before going to sleep
    bbb.ampDisable();  
    
    // delay for offDelayTime milliseconds. This is the time after durationTime expires but we do not allow another sound
    // to be triggered.
    delay(offDelayTime);      

    // restart the watchdog timer
    configWdt();        
}

/**************************************************************************/
// 
/**************************************************************************/
void getMeta()
{
    EEPROM.get(METADATA_EEPROM_LOC, meta);    
}

/**************************************************************************/
// 
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
    printf_P(PSTR("Device ID:   \t%d\n"),  meta.boomboxId);
    printf_P(PSTR("Interval:    \t%d\n"),  meta.playbackInterval);
}

/**************************************************************************/
/*! configure watchdog timer, interrupt only mode
*/
/**************************************************************************/
void configWdt()
{
    /* A 'timed' sequence is required for configuring the WDT, so we need to 
     * disable interrupts here.
     */
    cli(); 
    wdt_reset();
    MCUSR &= ~_BV(WDRF);
    /* Start the WDT Config change sequence. */
    WDTCSR |= _BV(WDCE) | _BV(WDE);
    /* Configure the prescaler and the WDT for interrupt mode only*/
    
#if (TESTONLY == 1)    
    WDTCSR =  _BV(WDP1) | _BV(WDP2) | _BV(WDIE); // 1 seconds
#else
    WDTCSR =  _BV(WDP3) | _BV(WDIE); // 4 seconds
#endif

    sei();
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
    (void) stream;
    
    Serial.write(c);
    return 0;
}
