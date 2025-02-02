#include <main.hpp>

// TFT inits
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);
// PNG png;

// Adafruit Seesaw inits
Adafruit_seesaw ss;
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

//=========================================v==========================================
//                                      pngDraw
//====================================================================================
// This next function will be called during decoding of the png file to
// render each image line to the TFT.  If you use a different TFT library
// you will need to adapt this function to suit.
// Callback function to draw pixels to the display
// void pngDraw(PNGDRAW *pDraw)
// {
//   uint16_t lineBuffer[MAX_IMAGE_WIDTH];
//   png.getLineAsRGB565(pDraw, lineBuffer, PNG_RGB565_BIG_ENDIAN, 0xffffffff);
//   tft.pushImage(IMAGE_XPOS, IMAGE_YPOS + pDraw->y, pDraw->iWidth, 1, lineBuffer);
// }

// TFT write

void tft_write(gamepad_input gp_read)
{
  // Serial.println(String(gp_read.x_pos));
  // Serial.println(String(gp_read.y_pos));
  String toDisp = "LOVE YOU, BANGARAM! <3::"  + String(gp_read.x_pos) + ":"+ String(gp_read.y_pos);
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_GREEN);
  spr.drawString(toDisp, 0, 0, 1);
  spr.pushSprite(0, 0);

  // int16_t rc = png.openFLASH((uint8_t *)panda, sizeof(panda), pngDraw);
  // if (rc == PNG_SUCCESS)
  // {
  //   Serial.println("Successfully opened png file");
  //   Serial.printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
  //   tft.startWrite();
  //   uint32_t dt = millis();
  //   rc = png.decode(NULL, 0);
  //   Serial.print(millis() - dt);
  //   Serial.println("ms");
  //   tft.endWrite();
  // }
}

gamepad_input last_input;
gamepad_input gamepad_read()
{
  // Reverse x/y values to match joystick orientation
  gamepad_input current_input;
  uint16_t x = X_REF - ss.analogRead(14);
  uint16_t y = Y_REF - ss.analogRead(15);

  if ((abs(x - last_input.x_pos) > 3) || (abs(y - last_input.y_pos) > 3))
  {
    // Serial.print("x: ");
    // Serial.print(x);
    // Serial.print(", ");
    // Serial.print("y: ");
    // Serial.println(y);
    last_input.x_pos = x;
    last_input.y_pos = y;
  } 
  current_input.x_pos = last_input.x_pos;
  current_input.y_pos = last_input.y_pos;

  if (digitalRead(IRQ_PIN))
  {
    uint32_t buttons = ss.digitalReadBulk(BUTTON_MASK);

    // Serial.println(buttons, BIN);

    if (!(buttons & (1UL << BUTTON_A)))
    {
      current_input.button_A = true;
      // Serial.println("\n Button A pressed");
    }
    if (!(buttons & (1UL << BUTTON_B)))
    {
      current_input.button_B = true;
      // Serial.println("\n Button B pressed");
    }
    if (!(buttons & (1UL << BUTTON_Y)))
    {
      current_input.button_Y = true;
      // Serial.println("\n Button Y pressed");
    }
    if (!(buttons & (1UL << BUTTON_X)))
    {
      current_input.button_X = true;
      // Serial.println("\n Button X pressed");
    }
    if (!(buttons & (1UL << BUTTON_SELECT)))
    {
      current_input.button_SEL = true;
      // Serial.println("\n Button SELECT pressed");
    }
    if (!(buttons & (1UL << BUTTON_START)))
    {
      current_input.button_SRT = true;
      // Serial.println("\n Button START pressed");
    }
  }
  return current_input;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("\n==Setup Start");

  // for blink
  pinMode(LED_BUILTIN, OUTPUT);

  // tft
  Serial.println("\n===TFT Init Start");
  tft.init();
  tft.setRotation(0);
  spr.setTextDatum(TL_DATUM);
  spr.createSprite(240, 180);
  tft.setTextWrap(true, false); // Wrap in x but not y
  tft.fillScreen(TFT_BLACK);
  Serial.println("\n===TFT Init Done");

  // Seesaw init
  Serial.println("\n===Seesaw Init Start");

  ss.begin(0x50);
  uint32_t version = ((ss.getVersion() >> 16) & 0xFFFF);
  // Configure pins and attach IRQ
  ss.pinModeBulk(BUTTON_MASK, INPUT_PULLUP);
  ss.setGPIOInterrupts(BUTTON_MASK, 1);

  pinMode(IRQ_PIN, INPUT);

  Serial.println("\n===Seesaw Init Done");
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);

  gamepad_input gp_read =  gamepad_read();

  tft_write(gp_read);

  digitalWrite(LED_BUILTIN, LOW);
}
