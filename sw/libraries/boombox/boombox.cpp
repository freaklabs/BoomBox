#include "boombox.h"
SoftwareSerial ss(9, 8);

Boombox bb;

volatile bool pirFlag;
volatile bool auxFlag;

/*----------------------------------------------------------*/
// Initializing the MP3 player
/*----------------------------------------------------------*/

/************************************************************/
// Constructor
/************************************************************/
Boombox::Boombox()
{ 
    pinMode(pinBusy, INPUT);
    pinMode(pinPIR, INPUT_PULLUP);
    pinMode(pinButton, INPUT_PULLUP);

    pinMode(pinAmpShutdn, OUTPUT);
    pinMode(pinBoostEnb, OUTPUT);
    pinMode(pinMp3Enb, OUTPUT);
    pinMode(pinCurrEnb, OUTPUT);
    pinMode(pinRangeEnb, OUTPUT);
    pinMode(pinPIREnb, OUTPUT);

    digitalWrite(pinBoostEnb, HIGH);
    digitalWrite(pinAmpShutdn, HIGH);  
    digitalWrite(pinMp3Enb, HIGH);
    digitalWrite(pinCurrEnb, HIGH);
    digitalWrite(pinRangeEnb, HIGH);
    digitalWrite(pinPIREnb, HIGH);

    attachInterrupt(intPIR, Boombox::irqPIR, FALLING);
    attachInterrupt(intAux, Boombox::irqAux, FALLING);
    setVol(_vol);
}

/************************************************************/
// init
/************************************************************/
void Boombox::init()
{
    auxFlag = false;
    pirFlag = false;

    ss.begin(9600);
    Serial.begin(57600);
}

/*----------------------------------------------------------*/
// Methods for controlling the MP3 player
/*----------------------------------------------------------*/

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
bool Boombox::isBusy()
{
    return digitalRead(pinBusy) == LOW;
}

/************************************************************/
// 
/************************************************************/
void Boombox::irqPIR(void)
{
    pirFlag = true;
}

/************************************************************/
// 
/************************************************************/
bool Boombox::isPIREvent()
{
    return pirFlag;
}

/************************************************************/
// 
/************************************************************/
void Boombox::clearPIRFlag()
{
    pirFlag = 0;
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
    auxFlag = 0;
}

/*----------------------------------------------------------*/
// Power management
/*----------------------------------------------------------*/

/************************************************************/
// 
/************************************************************/
void Boombox::sleep()
{
    // send sleep command to mp3 chip
    uint8_t buf[8] = {0x7E, 0xFF, 0x06, 0x0A, 0x00, 0x00, 0x00, 0xEF};
    _sendCmd(buf, sizeof(buf));

    // shut down everything else
    digitalWrite(pinBoostEnb, LOW);
    digitalWrite(pinMp3Enb, LOW);
    digitalWrite(pinCurrEnb, LOW);
    digitalWrite(pinRangeEnb, LOW);
    digitalWrite(pinPIREnb, LOW);
    digitalWrite(pinAmpShutdn, LOW);

    // disable UART
    UCSR0B = 0x00;

    printf("Sleeping MCU\n");
    delay(100);

    ADCSRA &= ~(1 << ADEN);    // Disable ADC

    // write sleep mode
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();                       // setting up for sleep ...
    sleep_mode();
}

/************************************************************/
// 
/************************************************************/
void Boombox::wake()
{
    digitalWrite(pinBoostEnb, HIGH);
    digitalWrite(pinMp3Enb, HIGH);
    digitalWrite(pinCurrEnb, HIGH);
    digitalWrite(pinRangeEnb, HIGH);
    digitalWrite(pinPIREnb, HIGH);
    digitalWrite(pinAmpShutdn, HIGH);

    UCSR0B = 0x98;
    ADCSRA |= (1 << ADEN); 

    uint8_t buf[8] = {0x7E, 0xFF, 0x06, 0x0B, 0x00, 0x00, 0x00, 0xEF};
    _sendCmd(buf, sizeof(buf));
}

/*----------------------------------------------------------*/
// Helper functions
/*----------------------------------------------------------*/

/************************************************************/
// sendCmd
/************************************************************/
void Boombox::_sendCmd(uint8_t *buf, uint8_t len)
{
    for (int i=0; i<len; i++) 
    {
        ss.write( buf[i] ); 
    }
}