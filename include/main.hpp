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
#include <lvgl.h>

/* TFT */
#include <TFT_eSPI.h> //TFT lib
#include <image.h>    // binary array
// #include <PNGdec.h>     //png read from binary array

// Origin for Image disp
#define IMAGE_XPOS ((uint16_t)0)
#define IMAGE_YPOS ((uint16_t)20)
#define MAX_IMAGE_WIDTH 240
#define TFT_HOR_RES 240
#define TFT_VER_RES 320
#define TFT_ROTATION LV_DISPLAY_ROTATION_0
#define TFT_BG_COLOR lv_color_hex(0xAEFF00)
// typedef struct {
//   lv_color_t color;
//   lv_coord_t width;
//   lv_coord_t dash_width;
//   lv_coord_t dash_gap;
//   lv_opa_t opa;
//   lv_blend_mode_t blend_mode  : 2;
//   uint8_t round_start : 1;
//   uint8_t round_end   : 1;
//   uint8_t raw_end     : 1;    /*Do not bother with perpendicular line ending if it's not visible for any reason*/
// } lv_draw_line_dsc_t;

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
#define BUTTON_MASK ((uint32_t)(1UL << BUTTON_X) | (1UL << BUTTON_Y) | (1UL << BUTTON_START) | (1UL << BUTTON_A) | (1UL << BUTTON_B) | (1UL << BUTTON_SELECT))
#define X_REF 524
#define Y_REF 521

// IRQ Pin to attach
#define IRQ_PIN 9

#define SERIAL_DEBUG

#define INIT_BODY_LENGTH 5
#define SNAKE_SPEED 20
typedef struct
{
  bool isUpdated = false;
  int16_t x_pos = X_REF;
  int16_t y_pos = X_REF;
  bool button_A = false;
  bool button_B = false;
  bool button_X = false;
  bool button_Y = false;
  bool button_SRT = false;
  bool button_SEL = false;
} gamepad_input_t;

typedef struct
{
  int16_t pixelX = 0;
  int16_t pixelY = 0;
} snake_part_t;

bool operator==(const gamepad_input_t &lhs, const gamepad_input_t &rhs)
{
  return (
      (lhs.button_A == rhs.button_A) || (lhs.button_B == rhs.button_B) || (lhs.button_X == rhs.button_X) || (lhs.button_Y == rhs.button_Y) || (lhs.button_SEL == rhs.button_SEL) || (lhs.button_SRT == rhs.button_SRT) || (lhs.x_pos == rhs.x_pos) || (lhs.y_pos == rhs.y_pos));
}

String prettify(const gamepad_input_t x)
{
  return (
      " isUpdated: " + String(x.isUpdated) + "\n X: " + String(x.x_pos) + "\n Y: " + String(x.y_pos) + "\n Button A:   " + String(x.button_A) + "\n Button B:   " + String(x.button_B) + "\n Button Y:   " + String(x.button_X) + "\n Button X:   " + String(x.button_Y) + "\n Button SEL: " + String(x.button_SEL) + "\n Button SRT: " + String(x.button_SRT));
}

void drawSnake(const std::deque<lv_point_precise_t> queue)
{
  static lv_obj_t *lastLine = NULL;
  if (lastLine != NULL)
  {
    lv_obj_delete(lastLine);
  }

  lv_obj_t *snakeDraw = lv_line_create(lv_screen_active());
  lv_point_precise_t point_array[queue.size()];
  for (uint8_t index = 0; index < queue.size(); index++)
  {
    point_array[index] = queue[index];
  }
  lv_line_set_points_mutable(snakeDraw, point_array, sizeof(point_array) / sizeof(point_array[0]));
  lv_obj_set_style_line_color(snakeDraw, lv_color_black(), LV_PART_MAIN);
  lv_obj_set_style_line_width(snakeDraw, 5, LV_PART_MAIN);
  lv_obj_set_style_line_rounded(snakeDraw, true, LV_PART_MAIN);
  lv_obj_set_style_line_dash_width(snakeDraw, 10, LV_PART_MAIN);
  lv_obj_set_style_line_dash_gap(snakeDraw, 1, LV_PART_MAIN);

  lv_timer_handler();
  lastLine = snakeDraw;
// #ifdef SERIAL_DEBUG
//   while (!Serial.available())
//   {
//   }
//   Serial.readString();
// #endif
}

/*use Arduinos millis() as tick source*/
static uint32_t my_tick(void)
{
  return millis();
}
