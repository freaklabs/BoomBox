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
    Serial.println("Going into normal operation mode.");
    Serial.flush();
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
    // WDTCSR =  _BV(WDP3) | _BV(WDIE); // 4 seconds
    WDTCSR =  _BV(WDP1) | _BV(WDP2) | _BV(WDIE); // 1 seconds    
    sei();
    //printf("MCUSR: %02X, WDTCSR: %02X\n", MCUSR, WDTCSR);
}

/**************************************************************************/
// This is to implement the printf function from within arduino
/**************************************************************************/
static int uart_putchar (char c, FILE *stream)
{
    Serial.write(c);
    return 0;
}
