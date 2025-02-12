// ********************** //
// Includes and defines   //
// ********************** //

/* Arduino */
#include <Arduino.h>

/* FreeRTOS */
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <map>
#include <EEPROM.h>
#include <SPI.h>

/* TFT */
#include <TFT_eSPI.h>   //TFT lib
#include <image.h>      // binary array
#include <PNGdec.h>     //png read from binary array

// Origin for Image disp
#define IMAGE_XPOS ((uint16_t)0)
#define IMAGE_YPOS ((uint16_t)20)
#define MAX_IMAGE_WIDTH 240

/* GamePad Library */
#include <Adafruit_seesaw.h>

// Button Maps
#define BUTTON_START 16
#define BUTTON_SELECT 0
#define BUTTON_X 6
#define BUTTON_Y 2
#define BUTTON_A 5
#define BUTTON_B 1

// Joystick Maps
#define X_AXIS 14
#define Y_AXIS 15

// For pull up of bulk pins
#define BUTTON_MASK ((uint32_t) (1UL << BUTTON_X) | (1UL << BUTTON_Y) | (1UL << BUTTON_START) | (1UL << BUTTON_A) | (1UL << BUTTON_B) | (1UL << BUTTON_SELECT))
#define X_REF 524
#define Y_REF 521

// IRQ Pin to attach 
#define IRQ_PIN 9

#define SERIAL_DEBUG


typedef struct
{
  int16_t x_pos = X_REF;
  int16_t y_pos = X_REF;
  bool button_A = false;
  bool button_B = false;
  bool button_X = false;
  bool button_Y = false;
  bool button_SRT = false;
  bool button_SEL = false;
} gamepad_input;

bool operator==(const gamepad_input& lhs, const gamepad_input& rhs)
{
    return ( 
                (lhs.button_A == rhs.button_A) 
                || (lhs.button_B == rhs.button_B) 
                || (lhs.button_X == rhs.button_X) 
                || (lhs.button_Y == rhs.button_Y) 
                || (lhs.button_SEL == rhs.button_SEL) 
                || (lhs.button_SRT == rhs.button_SRT)
                || (lhs.x_pos == rhs.x_pos) 
                || (lhs.y_pos == rhs.y_pos)
    );  
}

String prettify (const gamepad_input x)
{
  return (
              " X: " + String(x.x_pos) 
          + "\n Y: " + String(x.y_pos) 
          + "\n Button A:   " +String(x.button_A)  
          + "\n Button B:   " + String(x.button_B) 
          + "\n Button Y:   " + String(x.button_X) 
          + "\n Button X:   " + String(x.button_Y) 
          + "\n Button SEL: " + String(x.button_SEL) 
          + "\n Button SRT: " + String(x.button_SRT)
  );
}