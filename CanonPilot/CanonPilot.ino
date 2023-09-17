#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "CanonBLERemote.h"


// Declaration for SSD1306 display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_MOSI   23
#define OLED_CLK   18
#define OLED_DC    26
#define OLED_CS    5
#define OLED_RESET 16
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

// Encoder pins
#define outputA 21
#define outputB 17
#define EncoderButton 22

enum CurrSetting
{
  Frames = 0,
  Duration = 1,
  Pause = 2,
  Execute = 3
};

enum Shutter
{
  Stop = 0,
  Running = 1
};

int FrameCntr;
int CurrFrame;
int FrameTime;
int CurrTime;
int FramePause;
int PauseTime;
Shutter ShutterState;
CurrSetting SettingSelected;

int FrameCntrTemp;
int CurrFrameTemp;
int FrameTimeTemp;
int CurrTimeTemp;
int FramePauseTemp;
int PauseTimeTemp;
CurrSetting SettingSelectedTemp;
Shutter ShutterStateTemp;

Shutter ShutterStateInt;
Shutter ShutterStatePrev;

bool ExposureActive;
bool PauseActive;

String name_remote = "ESP32 Remote";
CanonBLERemote canon_ble(name_remote);

void setup()
{
  int ButtonState;
  
  Serial.begin(115200);
  InitEncoder();

  if(!display.begin(SSD1306_SWITCHCAPVCC))
  {
    Serial.println(F("SSD1306 allocation failed"));
  }
  
  display.clearDisplay();
  canon_ble.init();
  delay(1000);

  ButtonState = digitalRead(EncoderButton);
  if (ButtonState == LOW)
  {
    display.setTextSize(2);
    display.print("PAIRING");
    display.display();
    delay(500);

    for (int i = 0; i < 3; i++)
    {
      if (!canon_ble.pair(10))
      {
        continue;
      }       
      else
      {
        break;
      }
    }
  }

  display.clearDisplay();
}

void loop()
{
  EncStateMachine();
  UpdateDisplay();
  //delay(2);
}

void UpdateDisplay()
{
  char DispCurrFrame[4];
  char DispFrameCntr[4];
  char DispFrameTime[4];
  char DispCurrTime[4];
  char DispFramePause[3];
  char DispPauseTime[4];
  sprintf(DispCurrFrame, "%03d", CurrFrame);
  sprintf(DispFrameCntr, "%03d", FrameCntr);
  sprintf(DispFrameTime, "%03d", FrameTime);
  sprintf(DispCurrTime, "%03d", CurrTime);
  sprintf(DispFramePause, "%02d", FramePause);
  sprintf(DispPauseTime, "%02d", PauseTime);

  //update display only, if necessary
  if ((FrameCntr != FrameCntrTemp) || (CurrFrame != CurrFrameTemp) || (FrameTime != FrameTimeTemp) || 
      (CurrTime != CurrTimeTemp) || (FramePause != FramePauseTemp) || (PauseTime != PauseTimeTemp) ||
      (SettingSelectedTemp != SettingSelected) || (ShutterStateTemp != ShutterStateInt))
  {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0,0);
    display.println(F(" Frame    Time  Pause"));
    display.print(DispCurrFrame);
    display.print("/");
    if (SettingSelected == Frames)
    {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display.print(DispFrameCntr);
      display.setTextColor(SSD1306_WHITE);
    }
    else
    {
      display.print(DispFrameCntr);
    }
    display.print("   ");
    if (SettingSelected == Duration)
    {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display.print(DispFrameTime);
      display.setTextColor(SSD1306_WHITE);
    }
    else
    {
      display.print(DispFrameTime);
    }
    display.print("s");
    display.print("   ");
    if (SettingSelected == Pause)
    {
      display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      display.print(DispFramePause);
      display.setTextColor(SSD1306_WHITE);
    }
    else
    {
      display.print(DispFramePause);
    }
    display.println("s");
    display.setCursor(0,24);
    display.setTextSize(2); 

    if (ShutterState == Stop)
    {
      switch (SettingSelected)
      {
        case Frames:
          display.println(F(" Set frame"));
          display.println(F("  number"));
          break;

        case Duration:
          display.println(F(" Set frame"));
          display.println(F("   time"));
          break;

        case Pause:
          display.println(F(" Set pause"));
          display.println(F("   time"));
          break;

        case Execute:
          if (ShutterStateInt == Stop)
          {
            display.println(F("   Stop"));
          }
          else
          {
            display.println(F("   Start"));
            display.println(F(" sequence!"));
          }
          break;
      }
    }
    else
    { 
      display.setCursor(0,24);
      display.setTextSize(2);

      if (ExposureActive == true)
      {
        display.print(F("  EXP."));
        display.println(DispCurrFrame);
        display.setTextSize(3);
        display.print(DispCurrTime);
        display.print("/");
        display.println(DispFrameTime);
      }
      else if (PauseActive == true)
      {
        display.println(F("   PAUSE"));
        display.setTextSize(3);
        display.print(" ");
        display.print(DispPauseTime);
        display.print("/");
        display.println(DispFramePause);
      }
    }

    display.display();
  }
  else
  {
    //no change, so do not update display
  }

  FrameCntrTemp = FrameCntr;
  CurrFrameTemp = CurrFrame;
  FrameTimeTemp = FrameTime;
  CurrTimeTemp = CurrTime;
  FramePauseTemp = FramePause;
  PauseTimeTemp = PauseTime;
  SettingSelectedTemp = SettingSelected;
  ShutterStateTemp = ShutterStateInt;
}