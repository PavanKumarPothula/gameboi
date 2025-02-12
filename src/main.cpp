#include <main.hpp>

// TFT inits
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

// Adafruit Seesaw inits
Adafruit_seesaw ss;

static gamepad_input last_input;

xQueueHandle graphicUpdateQueue;

// TFT write
void TFT_write(void *pvParameters)
{

#ifdef SERIAL_DEBUG
  Serial.println("#1::Entering TFT_write task \n");
#endif

  gamepad_input gp_read;

  for (;;)
  {
    if (!uxQueueMessagesWaiting(graphicUpdateQueue) || (xQueueReceive(graphicUpdateQueue, &gp_read, 100 / portTICK_PERIOD_MS) == pdFALSE))
    {
#ifdef SERIAL_DEBUG
      Serial.println("#1::No messages in the queue yielding\n");
#endif
      taskYIELD();
    }
#ifdef SERIAL_DEBUG
      Serial.println("#1::New Message Detected\n");
#endif

#ifdef SERIAL_DEBUG
    Serial.println("#1::" + String(gp_read.x_pos));
    Serial.println("#1::" + String(gp_read.y_pos));
#endif
    String toDisp = "LOVE YOU, BANGARAM! <3::" + String(gp_read.x_pos) + ":" + String(gp_read.y_pos);
    spr.fillSprite(TFT_BLACK);
    spr.drawString(toDisp, 0, 0);
    spr.pushSprite(0, 0);
  }
#ifdef SERIAL_DEBUG
  Serial.println("#1::Exiting TFT_write task \n");
#endif
}

void gamepad_read(void *pvParameters)
{
  // Read and parse the current input of the game pad
#ifdef SERIAL_DEBUG
  Serial.println("#2::Entering gamepad_read task \n");
#endif

  gamepad_input current_input;
  for (;;)
  {
    bool isNew = false;
    int16_t x = X_REF - ss.analogRead(14);
    int16_t y = Y_REF - ss.analogRead(15);

    if ((abs(x - last_input.x_pos) > 3) || (abs(y - last_input.y_pos) > 3))
    {
      last_input.x_pos = x;
      last_input.y_pos = y;
      isNew = true;
    }

    current_input.x_pos = last_input.x_pos;
    current_input.y_pos = last_input.y_pos;

    if (digitalRead(IRQ_PIN))
    {
      uint32_t buttons = ss.digitalReadBulk(BUTTON_MASK);

      if (!(buttons & (1UL << BUTTON_A)))
      {
        current_input.button_A = true;
      }
      if (!(buttons & (1UL << BUTTON_B)))
      {
        current_input.button_B = true;
      }
      if (!(buttons & (1UL << BUTTON_Y)))
      {
        current_input.button_Y = true;
      }
      if (!(buttons & (1UL << BUTTON_X)))
      {
        current_input.button_X = true;
      }
      if (!(buttons & (1UL << BUTTON_SELECT)))
      {
        current_input.button_SEL = true;
      }
      if (!(buttons & (1UL << BUTTON_START)))
      {
        current_input.button_SRT = true;
      }
    }
    
    if (last_input == current_input)
    {
    #ifdef SERIAL_DEBUG
      Serial.println("#2::New input detected\n");
      Serial.println(prettify(current_input));
    #endif      
      xQueueSend(graphicUpdateQueue, &current_input, 100 / portTICK_RATE_MS);
    }
    vTaskDelay(100 / portTICK_RATE_MS);
  }
#ifdef SERIAL_DEBUG
  Serial.println("#2::Exiting gamepad_read task \n");
#endif
}

void main_init()
{
#ifdef SERIAL_DEBUG
  Serial.begin(115200);
  while (!Serial.available())
  {
  }
  Serial.println("MAIN::==Setup Start");
#endif
  // for blink
  pinMode(LED_BUILTIN, OUTPUT);

// tft
#ifdef SERIAL_DEBUG
  Serial.println("MAIN::===TFT Init Start");
#endif
  tft.init();
  tft.setRotation(0);
  tft.setTextWrap(true, false); // Wrap in x but not y
  tft.fillScreen(TFT_BLACK);
  spr.setTextDatum(TL_DATUM);
  spr.createSprite(240, 180);
  spr.fillSprite(TFT_BLACK);
  spr.setTextColor(TFT_GREEN);  

#ifdef SERIAL_DEBUG
  Serial.println("MAIN::===TFT Init Done");
#endif

// Seesaw init
#ifdef SERIAL_DEBUG
  Serial.println("MAIN::===Seesaw Init Start");
#endif

  ss.begin(0x50);
  uint32_t version = ((ss.getVersion() >> 16) & 0xFFFF);
  // Configure pins and attach IRQ
  ss.pinModeBulk(BUTTON_MASK, INPUT_PULLUP);
  ss.setGPIOInterrupts(BUTTON_MASK, 1);

  pinMode(IRQ_PIN, INPUT);

#ifdef SERIAL_DEBUG
  Serial.println("MAIN::===Seesaw Init Done");
#endif
}

void setup()
{
  main_init();
  graphicUpdateQueue = xQueueCreate(20, sizeof(gamepad_input));
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
  Serial.println("LOOP::Hello!");
#endif
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}