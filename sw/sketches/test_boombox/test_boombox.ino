#include "chibi.h"
#include "MP3FLASH16P.h"
#include <avr/sleep.h>
#include <avr/power.h>

MP3FLASH16P mp3;

uint8_t pinPIR = 2;
uint8_t pinButton = 3;
uint8_t pinShutdn = 5;
uint8_t pinBoostEnb = 4;
uint8_t pinMp3Enb = 6;
uint8_t pinCurrEnb = 7;
uint8_t pinArefEnb = 10;
uint8_t pinRangeEnb = 16;
uint8_t pinPIREnb = 17;

uint8_t intPIR = 0;
uint8_t intButton = 1;

uint8_t vol;
volatile uint8_t pirFlag = 0;
volatile uint8_t buttonFlag = 0;

/************************************************************/
// setup
/************************************************************/
void setup() 
{
  
  pinMode(pinPIR, INPUT_PULLUP);
  pinMode(pinButton, INPUT_PULLUP);

  pinMode(pinBoostEnb, OUTPUT);
  pinMode(pinShutdn, OUTPUT);
  pinMode(pinMp3Enb, OUTPUT);
  pinMode(pinCurrEnb, OUTPUT);
  pinMode(pinArefEnb, OUTPUT);
  pinMode(pinRangeEnb, OUTPUT);
  pinMode(pinPIREnb, OUTPUT);

  digitalWrite(pinBoostEnb, HIGH);
  digitalWrite(pinShutdn, HIGH);  
  digitalWrite(pinMp3Enb, HIGH);
  digitalWrite(pinCurrEnb, HIGH);
  digitalWrite(pinArefEnb, HIGH);
  digitalWrite(pinRangeEnb, HIGH);
  digitalWrite(pinPIREnb, HIGH);

  vol = 25;
  mp3.init(11, vol);
  mp3.setVolume(vol);

  attachInterrupt(intPIR, irqPIR, FALLING);
  attachInterrupt(intButton, irqButton, FALLING);

  chibiCmdInit(57600);
  chibiCmdAdd("play", cmdPlay);
  chibiCmdAdd("stop", cmdStop);
  chibiCmdAdd("vol", cmdSetVolume);
  chibiCmdAdd("pause", cmdPause);
  chibiCmdAdd("resume", cmdResume);
  chibiCmdAdd("sleep", cmdSleep);
}

/************************************************************/
// loop
/************************************************************/
void loop() 
{
  chibiCmdPoll();

  if (buttonFlag == 1)
  {
    delay(200);
    buttonFlag = 0;
    
    Serial.println("Button pushed");
    //mp3.playNext();
    doSleep();
  }

  if (pirFlag)
  {
    pirFlag = 0;
    
    Serial.println("PIR event");
    //mp3.playNext();
    //delay(50);
  }
}

/************************************************************/
// IRQ
/************************************************************/
void irqButton(void)
{
  buttonFlag = 1;
}

void irqPIR(void)
{
  pirFlag = 1;
}

/************************************************************/
// helper functions
/************************************************************/
void doSleep()
{
  Serial.println("Going to sleep now");
  digitalWrite(pinBoostEnb, LOW);
  digitalWrite(pinMp3Enb, LOW);
  digitalWrite(pinCurrEnb, LOW);
  digitalWrite(pinArefEnb, LOW);
  digitalWrite(pinRangeEnb, LOW);
  digitalWrite(pinPIREnb, LOW);
  digitalWrite(pinShutdn, LOW);

  
  // disable UART
  UCSR0B = 0x00;


  // set all inputs
  //PORTC = 0x00;
  //DDRC = 0x00;

  //PORTD = 0x00;
  //DDRD = 0x00;

  printf("Sleeping MCU\n");
  delay(100);

  ADCSRA &= ~(1 << ADEN);    // Disable ADC

  // write sleep mode
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();                       // setting up for sleep ...
  sleep_mode();
}

/************************************************************/
// command line functions
/************************************************************/
void cmdPlay(int arg_cnt, char **args)
{
  uint8_t file = chibiCmdStr2Num(args[1], 10);
  mp3.playFile(file, vol);
}

void cmdSetVolume(int arg_cnt, char **args)
{
  vol = chibiCmdStr2Num(args[1], 10);
  mp3.setVolume(vol);
}

void cmdPause(int arg_cnt, char **args)
{
  mp3.pause();
}

void cmdResume(int arg_cnt, char **args)
{
  mp3.resume();
}

void cmdStop(int arg_cnt, char **args)
{
  mp3.stopPlay();
}

void cmdSleep(int arg_cnt, char **args)
{
  doSleep();
}



