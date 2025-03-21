#include <Arduino.h>
#include <stdint.h>

#include "SparkFun_Qwiic_Joystick_Arduino_Library.h"
#include "SparkFun_Qwiic_OLED.h"

#include <res/qw_fnt_5x7.h>

#include <Wire.h>

QwiicMicroOLED OLED;

const uint8_t masterAddres = 0x12;
const uint8_t otherMasterAddres = 0x10;

const float padW = 3;
const float padH = 8;
float playerPadX = 0;
float playerPadY = ((OLED.getHeight() / 2) - (padH / 2));
float lastVDir = 1;

float cpuPadX = OLED.getWidth() - padW;
float cpuPadY = ((OLED.getHeight() / 2) - (padH / 2));
float cpuPadH = 8;
float chaseSpeed = 1;

const float ballW = 2;
const float ballH = 2;
float ballstartX = ((OLED.getWidth() / 2) - (ballW / 2));
float ballstartY = ((OLED.getHeight() / 2) - (ballH / 2));
float ballX = ballstartX;
float ballY = ballstartY;
float ballDX = 0.25;
float ballDY = 0.25;

int pScore = 0;
int cScore = 0;

const int maxScore = 3;

const uint8_t Address = 0x20;
JOYSTICK joystick;

const int stickZero = 512;
const int stickDeadZone = 200;

uint8_t playerScore = 0;
uint8_t cpuScore = 0;

typedef enum states
{
  PASIVE,
  ACTIVE
} States;

States state = PASIVE;
bool gameOver = false;

uint8_t difficulty = 0;

void InitGame()
{
  playerPadX = 0;
  playerPadY = ((OLED.getHeight() / 2) - (padH / 2));
  lastVDir = 1;

  cpuPadX = OLED.getWidth() - padW;
  cpuPadY = ((OLED.getHeight() / 2) - (padH / 2));

  ballstartX = ((OLED.getWidth() / 2) - (ballW / 2));
  ballstartY = ((OLED.getHeight() / 2) - (ballH / 2));
  ballX = ballstartX;
  ballY = ballstartY;
  ballDX = 0.25;
  ballDY = 0.25;
  chaseSpeed = 1;
  cpuPadH = 8;

  if (difficulty >= 1)
  {
    cpuPadH = 10;
  }
  
  if (difficulty >= 2)
  {
    chaseSpeed = 2;
  }

  pScore = 0;
  cScore = 0;



  gameOver = false;
}

void receiveEvent() 
{
  while (0 < Wire.available()) {
    byte receivedByte = Wire.read();

    if (state == PASIVE)
    {
      if (receivedByte >= 0xf0) {
        state = ACTIVE;
        difficulty = receivedByte - 0xf0;
        InitGame();
      }
    }
  }
}

void requestEvent() {
  if (gameOver)
  {
    Wire.write(0x88);
  }
}

int PlayGame()
{
  int X = joystick.getHorizontal();
  int Y = joystick.getVertical();
  int B = joystick.getButton();

  if  (X > (stickZero + stickDeadZone))
  {
    OLED.rectangle(playerPadX, playerPadY, padW, padH, 0);
    if (playerPadY < OLED.getHeight() - padH)
    {
      playerPadY += (X-512)/250;
      lastVDir = 1;
    } else {
      playerPadY = OLED.getHeight() - padH;
    }
  }
  
  if (X < (stickZero - stickDeadZone))
  {
    OLED.rectangle(playerPadX, playerPadY, padW, padH, 0);
    if (playerPadY > 0 )
    {
      playerPadY += (X-512)/250;
      lastVDir = -1;
    } else {
      playerPadY = 0;
    }
  }

  if (ballY <= 0 || ballY >= OLED.getHeight())
  {
    ballDY *= -1;
  } 

  if (ballX >= OLED.getWidth())
  {
    
    ballX = ballstartX;
    ballY = ballstartY;
    ballDX *= -1;
    ballDY *= -1;
    pScore++;

    OLED.text(4, 10, "You Score!", 1);

    String psString = String(pScore);
    OLED.text(20,20,psString);

    OLED.text(30,20,":");

    String csString = String(cScore);
    OLED.text(40,20,csString);

    OLED.display();
    while(B != 0)
    { 
      B = joystick.getButton();
    }

    OLED.rectangleFill(0, 0, OLED.getWidth(), OLED.getHeight(), 0);
    
    OLED.display();
    
    if (pScore >= maxScore)
    {
      //state = PASIVE;
      gameOver = true;
      //return -1;
    }
  }

  if (ballX <= -ballW || ballX >= OLED.getWidth())
  {
    
    ballX = ballstartX;
    ballY = ballstartY;
    ballDX *= -1;
    ballDY *= -1;
    cScore++;

    OLED.text(4, 10, "CPU scored", 1);

    String psString = String(pScore);
    OLED.text(20,20,psString);

    OLED.text(30,20,":");

    String csString = String(cScore);
    OLED.text(40,20,csString);

    OLED.display();
    while(B != 0)
    { 
      B = joystick.getButton();
    }
    OLED.rectangleFill(0, 0, OLED.getWidth(), OLED.getHeight(), 0);
    
    OLED.display();

    if (cScore >= maxScore)
    {
      //state = PASIVE;
      gameOver = true;
      //return -1;
    }
  }

  if (gameOver == true)
  {
    state = PASIVE;
    return -1;
  }

  if (ballX <= padW && (ballY + ballH) >= playerPadY && ballY <= (playerPadY + padH)) 
  {
    ballDX = .25 + lastVDir*0.125;
  }

  if (ballX >= cpuPadX && (ballY + ballH) >= cpuPadY && ballY <= (cpuPadY + padH)) 
  {
    ballDX = -.25 + lastVDir*0.125;
  }

  OLED.rectangle(ballX, ballY, ballW, ballH, 0);

  OLED.rectangle(cpuPadX, cpuPadY, padW, cpuPadH, 0);

  if (ballY > (cpuPadY + cpuPadH))
  {   
    cpuPadY += chaseSpeed;
  }

  if (ballY < cpuPadY)
  {   
    cpuPadY -= chaseSpeed;
  }

  ballX += ballDX;
  ballY += ballDY;
  //OLED.rectangleFill(0, 0, OLED.getWidth(), OLED.getHeight(), 0);

  OLED.rectangle(cpuPadX, cpuPadY, padW, cpuPadH, 1);
  OLED.rectangle(playerPadX, playerPadY, padW, padH, 1);
  
  OLED.rectangle(ballX, ballY, ballW, ballH, 1);
  OLED.display();
  return 0;
}

void setup()
{
  delay(500);   

  if(!OLED.begin())
  {
    while(1);
  }

  if(!joystick.begin(Wire, Address))
  {
    while(1);
  }

  OLED.display();

  Wire.begin(masterAddres);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

}

void loop()
{
  if (state == ACTIVE)
  {
    PlayGame();
  }
}