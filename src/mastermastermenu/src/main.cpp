#include <Arduino.h>
#include <stdint.h>

#include "SparkFun_Qwiic_Joystick_Arduino_Library.h"
#include "SparkFun_Qwiic_OLED.h"

#include <res/qw_fnt_5x7.h>

#include <Wire.h>

QwiicMicroOLED OLED;

const uint8_t masterAddres = 0x10;
const uint8_t otherMasterAddres = 0x12;

const uint8_t stickAddress = 0x20;
JOYSTICK joystick;

const int stickZero = 512;
const int stickDeadZone = 200;

uint8_t playerScore = 0;
uint8_t cpuScore = 0;

bool upPressed = false;
bool upDown = false;

bool downPressed = false;
bool downDown = false;

bool buttonPressed = false;
bool buttonDown = false;


uint8_t dificulty = 0;
uint8_t menuNr = 0;
uint8_t selNr = 0;

//uint8_t state = 0;

typedef enum states
{
  ACTIVE,
  PASIVE
} States;

States state = ACTIVE;

// void receiveEvent() 
// {
//   while (0 < Wire.available()) {
//     byte receivedAdress = Wire.read();

//     if (receivedAdress==0x88) {
//       state = ACTIVE;
//       setup();
//     }
//   }
// }


void DoMenu()
{
  int Y = joystick.getHorizontal();
  int B = joystick.getButton();
  buttonPressed = false;
  upPressed = false;
  downPressed = false;

  if (Y <= stickZero - stickDeadZone)
  {
    if (upDown == false)
    {
      upPressed = true;
    }
    upDown = true;
  }

  if (Y >= stickZero + stickDeadZone)
  {
    if (downDown == false)
    {
      downPressed = true;
    }
    downDown = true;
  }

  if ((Y < stickZero + stickDeadZone) && (Y > stickZero - stickDeadZone))
  {
    downDown = false;
    upDown = false;
  }

  if (B==0) 
  {
    if (buttonDown == false)
    {
      buttonPressed = true;
    }
    buttonDown = true;
  }
  if (B!=0)
  {
    buttonDown = false;
  }
  
  switch (menuNr)
  {
  case 0:
    
    if (buttonPressed)
    {
      OLED.rectangleFill(1,1,60,44,0);
      OLED.text(2, 2, "Difficulty");

      OLED.text(1, 16, ">");

      OLED.text(14, 16, "normal");
      OLED.text(20, 26, "hard");
      OLED.text(16, 36, "silly");
      OLED.display();
      menuNr = 1;
    }
    break;
  case 1:
    
    if (downPressed)
    {
      selNr++;
      selNr %= 3;
      OLED.rectangleFill(0,16,8,30,0);
      OLED.text(1, 16+(selNr*10), ">");
      OLED.display();
    }

    if (upPressed)
    {
      if (selNr > 0)
      {
        selNr--;
      }
      OLED.rectangleFill(0,16,8,30,0);
      OLED.text(1, 16+(selNr*10), ">");
      OLED.display();
    }

    if (buttonPressed)
    {
      dificulty = selNr;
      
      menuNr = 0;
      state = PASIVE;
      OLED.rectangleFill(0,0,63,47,0);
      OLED.display();

      Wire.beginTransmission(otherMasterAddres);
      byte writeByte = 0xf0 + selNr;
      Wire.endTransmission();
      selNr = 0;

    }
    break;
  default:
    break;
  }
}


void setup()
{
  delay(500);   

  if(!OLED.begin())
  {
    while(1);
  }

  if(!joystick.begin(Wire, stickAddress))
  {
    while(1);
  }

  Wire.begin();

  OLED.text(2, 2, "Piing Pong");
  OLED.text(14, 20, "press");
  OLED.text(6, 30, "joystick"); 
  OLED.display();
}

void DoPassiveMode()
{
    Wire.requestFrom(otherMasterAddres,1);
    uint8_t response = 0;
      
    while (Wire.available())
    {
      response = Wire.read();
    }
      
    if (response == 0x88)
    {
      state = ACTIVE;
      OLED.text(2, 2, "Piing Pong");
      OLED.text(14, 20, "press");
      OLED.text(6, 30, "joystick"); 
      OLED.display();
    }
    delay(2000);
}

void loop()
{
  if (state == ACTIVE)
  {
    DoMenu();
  }
  else
  {
    DoPassiveMode();
  }
}