#include <main.hpp>


// TFT inits
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);

// LVGL
/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// Adafruit Seesaw inits
Adafruit_seesaw ss;

xQueueHandle graphicUpdateQueue;

// TFT write
void TFT_write(void *pvParameters)
{
  static gamepad_input gp_read;

  // Queue of parts from tail to head.
  static std::deque<snakePart> snakeBody({
      {50, 10},
      {50, 11},
      {50, 12},
      {50, 13},
      {50, 14},
  });
  // If nothing updates, the snake moves along the same direction (Right by default).
  static int8_t XDirection = 1;
  static int8_t YDirection = 0;

#ifdef SERIAL_DEBUG
  Serial.println("#1::Entering TFT_write task \n");
#endif

  for (;;)
  {
    bool inputRecieved = false;
    if (!uxQueueMessagesWaiting(graphicUpdateQueue))
    {
#ifdef SERIAL_DEBUG
      Serial.println("#1::No messages in the queue yielding");
      Serial.println("#1::Continuing in the same direction");
#endif
      inputRecieved = false;
    }
    else
    {
      inputRecieved = xQueueReceive(graphicUpdateQueue, &gp_read, 100 / portTICK_PERIOD_MS);
#ifdef SERIAL_DEBUG
      Serial.println("#1::New Message Read");
#endif
      XDirection = (gp_read.x_pos) ? (gp_read.x_pos / abs(gp_read.x_pos)) : 0;
      YDirection = (gp_read.y_pos) ? (gp_read.y_pos / abs(gp_read.y_pos)) : 0;
    }
#ifdef SERIAL_DEBUG
    Serial.println("#1::" + String(gp_read.x_pos));
    Serial.println("#1::" + String(gp_read.y_pos));
#endif

    String toDisp = String(gp_read.x_pos) + ":" + String(gp_read.y_pos);
    spr.fillSprite(0x9dcb);
    spr.setTextColor(TFT_BLACK);
    spr.drawString(toDisp, 0, 0);

    // Snake Movement
    // for (uint8_t index = 0; index < snakeBody.size() - 1)
    //   ; index++)
    // {
    //   snakeBody[index].pixelX += XDirection;
    //   snakeBody[index].pixelY += YDirection;
    // }
    // snakePart tail = snakeBody.front();
    snakeBody.pop_front();
    snakePart newHead = snakeBody.back();
    newHead.pixelX += XDirection;
    newHead.pixelY += YDirection;
    snakeBody.push_back(newHead);

    for (uint8_t index = 0; index < snakeBody.size(); index++)
    {
      spr.drawPixel(snakeBody[index].pixelX, snakeBody[index].pixelY, TFT_BLACK);
    }
    spr.pushSprite(0, 0);
    delay(500);
  }
#ifdef SERIAL_DEBUG
  Serial.println("#1::Exiting TFT_write task \n");
#endif
}

void gamepad_read(void *pvParameters)
{
  static gamepad_input last_input;
  // Read and parse the current input of the game pad
#ifdef SERIAL_DEBUG
  Serial.println("#2::Entering gamepad_read task \n");
#endif

  gamepad_input current_input;
  for (;;)
  {
    int16_t x = X_REF - ss.analogRead(14);
    int16_t y = Y_REF - ss.analogRead(15);

    if ((abs(x - last_input.x_pos) > 3) || (abs(y - last_input.y_pos) > 3))
    {
      last_input.x_pos = x;
      last_input.y_pos = y;
      current_input.isUpdated = true;
    }

    current_input.x_pos = last_input.x_pos;
    current_input.y_pos = last_input.y_pos;

    if (digitalRead(IRQ_PIN))
    {
      uint32_t buttons = ss.digitalReadBulk(BUTTON_MASK);

      if (!(buttons & (1UL << BUTTON_A)))
      {
        current_input.button_A = true;
        current_input.isUpdated = true;
      }
      if (!(buttons & (1UL << BUTTON_B)))
      {
        current_input.button_B = true;
        current_input.isUpdated = true;
      }
      if (!(buttons & (1UL << BUTTON_Y)))
      {
        current_input.button_Y = true;
        current_input.isUpdated = true;
      }
      if (!(buttons & (1UL << BUTTON_X)))
      {
        current_input.button_X = true;
        current_input.isUpdated = true;
      }
      if (!(buttons & (1UL << BUTTON_SELECT)))
      {
        current_input.button_SEL = true;
        current_input.isUpdated = true;
      }
      if (!(buttons & (1UL << BUTTON_START)))
      {
        current_input.button_SRT = true;
        current_input.isUpdated = true;
      }
    }

    if (current_input.isUpdated)
    {
#ifdef SERIAL_DEBUG
      Serial.println("#2::New input detected");
      Serial.println(prettify(current_input));
#endif
      xQueueSend(graphicUpdateQueue, &current_input, pdMS_TO_TICKS(100));

#ifdef SERIAL_DEBUG
      Serial.println("#2::Pushed the message onto the queue");
#endif
    }
    delay(500);
  }
#ifdef SERIAL_DEBUG
  Serial.println("#2::Exiting gamepad_read task \n");
#endif
}

void main_init()
{
  lv_init();
  lv_tick_set_cb(my_tick);
  lv_display_t *disp = lv_tft_espi_create(TFT_WIDTH, TFT_HEIGHT, draw_buf, sizeof(draw_buf));
  lv_display_set_rotation(disp, TFT_ROTATION);
  lv_obj_t *screen = lv_obj_create(NULL);
  lv_obj_t *label = lv_label_create( lv_screen_active() );
  lv_label_set_text( label, "Hello Arduino, I'm LVGL!" );
  lv_obj_align( label, LV_ALIGN_CENTER, 0, 0 );
// lv_obj_t *textArea = lv_textarea_create(screen);
//   const char *strToDisp = "IT JUST WORKS";
//   lv_textarea_add_text(textArea, strToDisp);
  // lv_screen_load(screen);
  lv_timer_handler(); /* let the GUI do its work */

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
  tft.fillScreen(0x9dcb);
  spr.setTextDatum(TL_DATUM);
  spr.createSprite(240, 180);
  // spr.fillSprite(TFT_BLACK);
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
              2 * configMINIMAL_STACK_SIZE,
              NULL,
              configMAX_PRIORITIES - 1,
              NULL);
  xTaskCreate(TFT_write,
              "GamePad Read",
              2 * configMINIMAL_STACK_SIZE,
              NULL,
              configMAX_PRIORITIES - 1,
              NULL);
  vTaskStartScheduler();
}

void loop()
{
  digitalWrite(LED_BUILTIN, HIGH);

#ifdef SERIAL_DEBUG
  Serial.println("LOOP::Hello!(!!THIS SHOULD NEVER BE HIT!!)");
#endif
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}