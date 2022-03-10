#include "boombox.h"
SoftwareSerial ss(9, 8);

Boombox bb;

volatile bool pirFlag;
volatile bool auxFlag;
uint32_t startTime;

/*----------------------------------------------------------*/
// Initializing the MP3 player
/*----------------------------------------------------------*/

/************************************************************/
// Constructor
/************************************************************/
Boombox::Boombox()
{
    pinMode(pinBusy, INPUT);
    pinMode(pinButton, INPUT);

    pinMode(pinAmpShutdn, OUTPUT);
    pinMode(pinBoostEnb, OUTPUT);
    pinMode(pinMp3Enb, OUTPUT);
    pinMode(pin5vEnb, OUTPUT);

    digitalWrite(pinBoostEnb, HIGH);
    digitalWrite(pinAmpShutdn, HIGH);
    digitalWrite(pinMp3Enb, HIGH);
    digitalWrite(pin5vEnb, LOW);

    attachInterrupt(intAux, Boombox::irqAux, RISING);
}
/************************************************************/
// init
/************************************************************/
void Boombox::init()
{
    auxFlag = false;
    pirFlag = false;

    ss.begin(9600);
    setVol(_vol);
}

/*----------------------------------------------------------*/
// Methods for controlling the MP3 player
/*----------------------------------------------------------*/

/************************************************************/
// initialize the pushbutton
/************************************************************/
void Boombox::buttonInit()
{
    pinMode(pinButton, INPUT_PULLUP);
}

/************************************************************/
//
/************************************************************/
void Boombox::dispBanner()
{
    Serial.println("-------------------------------------------");
    Serial.println("Boombox");
    Serial.println("Designed by FreakLabs and Meredith Palmer");
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
    // add delay to debounce events
    delay(200);
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
    // add delay to debounce events
    delay(100);
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
    digitalWrite(pinBoostEnb, LOW);

    LowPower.powerDown(SLEEP_FOREVER, ADC_OFF, BOD_ON);
}

/************************************************************/
//
/************************************************************/
void Boombox::wake()
{
    digitalWrite(pinBoostEnb, HIGH);
    digitalWrite(pinMp3Enb, HIGH);

    // need a delay here to start up the mp3 player
    delay(100);
}

/************************************************************/
//
/************************************************************/
void Boombox::ampEnable()
{
    digitalWrite(pinAmpShutdn, HIGH);    
}

/************************************************************/
//
/************************************************************/
void Boombox::ampDisable()
{
    digitalWrite(pinAmpShutdn, LOW);    
}

/************************************************************/
//
/************************************************************/
void Boombox::reg5vEnable()
{
    digitalWrite(pin5vEnb, HIGH);    
}

/************************************************************/
//
/************************************************************/
void Boombox::reg5vDisable()
{
    digitalWrite(pin5vEnb, LOW);    
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
