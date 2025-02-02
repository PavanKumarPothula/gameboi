// ********************** //
// Includes and defines   //
// ********************** //

/* Arduino */
#include <Arduino.h>

/* FreeRTOS */
#include <FreeRTOS.h>
#include <task.h>
#include <map>
#include <EEPROM.h>
#include <SPI.h>

/* TFT */
#include <TFT_eSPI.h>   //TFT lib
#include "User_Setup.h" // Pins defs + others
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