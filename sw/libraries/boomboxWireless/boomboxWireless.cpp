#include "boomboxWireless.h"

SoftwareSerial ss(20, 21);
Boombox bb;

volatile bool auxFlag;

/*----------------------------------------------------------*/
// Initializing the MP3 player
/*----------------------------------------------------------*/

/************************************************************/
// Constructor
/************************************************************/
Boombox::Boombox()
{
    pinMode(pinAmpShutdn, OUTPUT);
    pinMode(pinMp3Enb, OUTPUT);
    pinMode(pin5VEnb, OUTPUT);

    digitalWrite(pinAmpShutdn, HIGH);
    digitalWrite(pinMp3Enb, HIGH);
    digitalWrite(pin5VEnb, HIGH);
    
    attachInterrupt(intAux, Boombox::irqAux, RISING);
    setVol(_vol);
}

/************************************************************/
// init
/************************************************************/
void Boombox::init()
{
    auxFlag = false;
    ss.begin(9600);
}

/*----------------------------------------------------------*/
// Methods for controlling the MP3 player
/*----------------------------------------------------------*/

/************************************************************/
//
/************************************************************/
void Boombox::dispBanner()
{
    Serial.println("-------------------------------------------");
    Serial.println("Boombox");
    Serial.print("Last modified: ");
    Serial.println(__DATE__);
    Serial.println("-------------------------------------------");
}

/************************************************************/
//
/************************************************************/
void Boombox::play(uint8_t file)
{
    uint8_t buf[8] = { 0x7E, 0xFF, 0x06, 0x03, 0X00, 0x00, file , 0xEF };
    _sendCmd(buf, sizeof(buf));
}

/************************************************************/
//
/************************************************************/
void Boombox::playBusy(uint8_t file)
{
    uint8_t buf[8] = { 0x7E, 0xFF, 0x06, 0x03, 0X00, 0x00, file , 0xEF };
    _sendCmd(buf, sizeof(buf));
    while (isBusy())
    {

    }
    delay(250);
}

/************************************************************/
//
/************************************************************/
bool Boombox::isBusy()
{
    return digitalRead(pinBusy) == LOW;
}

/************************************************************/
//
/************************************************************/
void Boombox::setVol(uint8_t vol)
{
    _vol = constrain(vol, 0, 30);
}

/************************************************************/
//
/************************************************************/
void Boombox::playNext()
{
    uint8_t buf[8] = { 0x7E, 0xFF, 0x06, 0x01, 0x00, 0x00, 0x00, 0xEF };
    _sendCmd(buf, sizeof(buf));
}

/************************************************************/
//
/************************************************************/
void Boombox::playPrev()
{
    uint8_t buf[8] = { 0x7E, 0xFF, 0x06, 0x01, 0x00, 0x00, 0x00, 0xEF };
    _sendCmd(buf, sizeof(buf));
}

/************************************************************/
//
/************************************************************/
void Boombox::stop()
{
    uint8_t buf[8] = { 0x7E, 0xFF, 0x06, 0x16, 0x00, 0x00, 0x00, 0XEF };
    _sendCmd(buf, sizeof(buf));
}

/************************************************************/
//
/************************************************************/
void Boombox::pause()
{
    uint8_t buf[8] = {0x7E, 0xFF, 0x06, 0x0E, 0x00, 0x00, 0x00, 0xEF};
    _sendCmd(buf, sizeof(buf));
}

/************************************************************/
//
/************************************************************/
void Boombox::resume()
{
    uint8_t buf[8] = {0x7E, 0xFF, 0x06, 0x0D, 0x00, 0x00, 0x00, 0xEF};
    _sendCmd(buf, sizeof(buf));
}

/************************************************************/
//
/************************************************************/
void Boombox::irqAux(void)
{
   auxFlag = true;
}

/************************************************************/
//
/************************************************************/
bool Boombox::isAuxEvent()
{
    return auxFlag;
}

/************************************************************/
//
/************************************************************/
void Boombox::clearAuxFlag()
{
    // add delay to debounce events
    delay(1000);
    auxFlag = false;
}

/*----------------------------------------------------------*/
// Power management
/*----------------------------------------------------------*/

/************************************************************/
//
/************************************************************/
void Boombox::sleep()
{
    // shut down everything else
    digitalWrite(pinMp3Enb, LOW);
    digitalWrite(pinAmpShutdn, LOW);
    digitalWrite(pin5VEnb, LOW);

    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);
}

/************************************************************/
//
/************************************************************/
void Boombox::wake()
{
    digitalWrite(pinMp3Enb, HIGH);
    digitalWrite(pinAmpShutdn, HIGH);
    digitalWrite(pin5VEnb, HIGH); 

    // need a delay here to start up the mp3 player
    delay(1000);
}

/************************************************************/
// get battery voltage
/************************************************************/
float Boombox::getVbat()
{
   int battAdc = analogRead(pinVbat);
    float voltsPerAdcUnit = ADC_REF_VOLTAGE / ADC_LEVELS;   // this is the voltage per ADC unit
    float pinVoltage = battAdc * voltsPerAdcUnit;           // this is the voltage at the pin
    float battVoltage = pinVoltage * ADC_SCALE_FACTOR;      // this is the voltage at the battery
    return battVoltage;
}

/************************************************************/
// get solar voltage
/************************************************************/
float Boombox::getVsol()
{
   int solarAdc = analogRead(pinVsol);
    float voltsPerAdcUnit = ADC_REF_VOLTAGE / ADC_LEVELS;   // this is the voltage per ADC unit
    float pinVoltage = solarAdc * voltsPerAdcUnit;           // this is the voltage at the pin
    float solarVoltage = pinVoltage * ADC_SCALE_FACTOR;      // this is the voltage at the battery
    return solarVoltage;
}

/*----------------------------------------------------------*/
// Helper functions
/*----------------------------------------------------------*/

/************************************************************/
// sendCmd
/************************************************************/
void Boombox::_sendCmd(uint8_t *buf, uint8_t len)
{
    cli();
    for (int i=0; i<len; i++)
    {
        ss.write( buf[i] );
    }
    sei();
}