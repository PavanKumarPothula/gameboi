#include <FreeRTOS.h>
#include <task.h>
#include <map>
#include <EEPROM.h>
#include <Arduino.h>
#include <SPI.h>
// #include "TFT_config.h"
#include <TFT_eSPI.h>
#include "User_Setup.h"

#include <PNGdec.h>
#include <image.h>

// TFT
// Set delay after plotting the sprite
// #define DELAY 1000

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
#define DEPTH
#define MAX_IMAGE_WIDTH 240 // Adjust for your images

int16_t xpos = 0;
int16_t ypos = 20;

PNG png;




//=========================================v==========================================
//                                      pngDraw
//====================================================================================
// This next function will be called during decoding of the png file to
// render each image line to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
// Callback function to draw pixels to the display
void pngDraw(PNGDRAW *pDraw) {
  uint16_t lineBuffer[MAX_IMAGE_WIDTH];
  png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
  tft.pushImage(xpos, ypos + pDraw->y, pDraw->iWidth, 1, lineBuffer);
}


void setup() {
  // for blink
  pinMode(LED_BUILTIN,OUTPUT);
  Serial.begin(115200);
  Serial.println("\n\n Using the PNGdec library");

  // tft
  tft.init();
  tft.setRotation(0);
  spr.setTextDatum(TL_DATUM);
  spr.createSprite(240,20);
  // spr.fillSprite(TFT_WHITE);
  tft.fillScreen(TFT_BLACK);
  // spr.setScrollRect(0, 0, 128, 128, TFT_ORANGE);     // here we set scroll gap fill color to blue
  Serial.println("\r\nInitialisation done.");
  
}

void loop() {
  digitalWrite(LED_BUILTIN,HIGH);
  sleep_ms(500);

  // spr.fillSprite(TFT_DARKGREEN);
  spr.drawString("LOVE YOU, BANGARAM! <3", 40,0,2);
  spr.pushSprite(0,0);
  // tft.fillScreen(TFT_BLUE);
  int16_t rc = png.openFLASH((uint8_t *)panda, sizeof(panda), pngDraw);
  if (rc == PNG_SUCCESS) {
    Serial.println("Successfully opened png file");
    Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
    tft.startWrite();
    uint32_t dt = millis();
    rc = png.decode(NULL, 0);
    Serial.print(millis() - dt); Serial.println("ms");
    tft.endWrite();
    png.close(); // not needed for memory->memory decode
  }
  // delay(3000);
  // tft.fillScreen(random(0x10000));
  digitalWrite(LED_BUILTIN,LOW);
  sleep_ms(500);
  // tft.fillScreen(random(0x10000));
  // tft.fillScreen(TFT_DARKGREEN);
  // spr.setTextColor(TFT_BLUE); // White text, no background

}

