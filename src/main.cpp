#include <main.hpp>

// TFT inits
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

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

static gamepad_input last_input;

xQueueHandle graphicUpdateQueue;

// TFT write
void TFT_write(void *pvParameters)
{

#ifdef SERIAL_DEBUG
  Serial.println("Entering TFT_write task \n");
#endif

  gamepad_input gp_read;
  if ( !uxQueueMessagesWaiting(graphicUpdateQueue) ||  (xQueueReceive(graphicUpdateQueue, &gp_read, 100/portTICK_PERIOD_MS) == pdFALSE))
  {
    return;
  }

#ifdef SERIAL_DEBUG
  Serial.println(String(gp_read.x_pos));
  Serial.println(String(gp_read.y_pos));
#endif
  String toDisp = "LOVE YOU, BANGARAM! <3::" + String(gp_read.x_pos) + ":" + String(gp_read.y_pos);
  spr.drawString(toDisp, 0, 0, 1);
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_GREEN);
  spr.pushSprite(0, 0);
#ifdef SERIAL_DEBUG
  Serial.println("Exiting TFT_write task \n");
#endif
}

void gamepad_read(void *pvParameters)
{
  // Read and parse the current input of the game pad
#ifdef SERIAL_DEBUG
  Serial.println("Entering gamepad_read task \n");
#endif

  gamepad_input current_input;
  uint16_t x = X_REF - ss.analogRead(14);
  uint16_t y = Y_REF - ss.analogRead(15);

  if ((abs(x - last_input.x_pos) > 3) || (abs(y - last_input.y_pos) > 3))
  {
#ifdef SERIAL_DEBUG
    Serial.print("x: ");
    Serial.print(x);
    Serial.print(", ");
    Serial.print("y: ");
    Serial.println(y);
#endif
    last_input.x_pos = x;
    last_input.y_pos = y;
  }

  current_input.x_pos = last_input.x_pos;
  current_input.y_pos = last_input.y_pos;

  if (digitalRead(IRQ_PIN))
  {
    uint32_t buttons = ss.digitalReadBulk(BUTTON_MASK);

#ifdef SERIAL_DEBUG
    Serial.println(buttons, BIN);
#endif

    if (!(buttons & (1UL << BUTTON_A)))
    {
      current_input.button_A = true;
#ifdef SERIAL_DEBUG
      Serial.println("\n Button A pressed");
#endif
    }
    if (!(buttons & (1UL << BUTTON_B)))
    {
      current_input.button_B = true;
#ifdef SERIAL_DEBUG
      Serial.println("\n Button B pressed");
#endif
    }
    if (!(buttons & (1UL << BUTTON_Y)))
    {
      current_input.button_Y = true;
#ifdef SERIAL_DEBUG
      Serial.println("\n Button Y pressed");
#endif
    }
    if (!(buttons & (1UL << BUTTON_X)))
    {
      current_input.button_X = true;
#ifdef SERIAL_DEBUG
      Serial.println("\n Button X pressed");
#endif
    }
    if (!(buttons & (1UL << BUTTON_SELECT)))
    {
      current_input.button_SEL = true;
#ifdef SERIAL_DEBUG
      Serial.println("\n Button SELECT pressed");
#endif
    }
    if (!(buttons & (1UL << BUTTON_START)))
    {
      current_input.button_SRT = true;
#ifdef SERIAL_DEBUG
      Serial.println("\n Button START pressed");
#endif
    }
  }
  xQueueSend(graphicUpdateQueue, &current_input, 100/portTICK_RATE_MS);
#ifdef SERIAL_DEBUG
  Serial.println("Exiting gamepad_read task \n");
#endif
}

void main_init()
{
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
  while (!Serial.available()){}
  Serial.println("\n==Setup Start");
#endif
  // for blink
  pinMode(LED_BUILTIN, OUTPUT);

// tft
#ifdef SERIAL_DEBUG
  Serial.println("\n===TFT Init Start");
#endif
  tft.init();
  tft.setRotation(0);
  spr.setTextDatum(TL_DATUM);
  spr.createSprite(240, 180);
  tft.setTextWrap(true, false); // Wrap in x but not y
  tft.fillScreen(TFT_BLACK);
#ifdef SERIAL_DEBUG
  Serial.println("\n===TFT Init Done");
#endif

// Seesaw init
#ifdef SERIAL_DEBUG
  Serial.println("\n===Seesaw Init Start");
#endif

  ss.begin(0x50);
  uint32_t version = ((ss.getVersion() >> 16) & 0xFFFF);
  // Configure pins and attach IRQ
  ss.pinModeBulk(BUTTON_MASK, INPUT_PULLUP);
  ss.setGPIOInterrupts(BUTTON_MASK, 1);

  pinMode(IRQ_PIN, INPUT);

#ifdef SERIAL_DEBUG
  Serial.println("\n===Seesaw Init Done");
#endif
}

void setup()
{
  main_init();
  graphicUpdateQueue = xQueueCreate(8, sizeof(gamepad_input));
  xTaskCreate(gamepad_read,
              "GamePad Read",
              configMINIMAL_STACK_SIZE,
              NULL,
              configMAX_PRIORITIES - 1,
              NULL);
  xTaskCreate(TFT_write,
              "GamePad Read",
              configMINIMAL_STACK_SIZE,
              NULL,
              configMAX_PRIORITIES - 1,
              NULL);
  vTaskStartScheduler();
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);

#ifdef SERIAL_DEBUG
  Serial.println("Hello!");
#endif
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}