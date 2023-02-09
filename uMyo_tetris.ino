#include <Arduino.h>

#include <uMyo_BLE.h>

#include <FastLED.h>
#include "tetris_eng.h"

#define NUM_LEDS 122
#define DATA_PIN 13

#define TETR_SX 11
#define TETR_SY 11

CRGB leds[NUM_LEDS];
uint8_t tetr_field[TETR_SX*TETR_SY];
uint8_t rgb_states[TETR_SX*TETR_SY*3];

sTField field;

void setup() {
  field.field = tetr_field;
  field.SX = TETR_SX;
  field.SY = TETR_SY;
  field.step_time = 250;
  field.place_time = 800;
  field.drop_time = 200;
  field.blow_time = 600;
  field.over_time = 3000;
  Serial.begin(115200);
  uMyo.begin();
  
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
  for(int x = 0; x < NUM_LEDS; x++)
  {
    if(x%6 == 0) leds[x] = CRGB(5, 0, 0);
    if(x%6 == 1) leds[x] = CRGB(0, 5, 0);
    if(x%6 == 2) leds[x] = CRGB(0, 0, 5);
    if(x%6 == 3) leds[x] = CRGB(5, 5, 0);
    if(x%6 == 4) leds[x] = CRGB(0, 5, 5);
    if(x%6 == 5) leds[x] = CRGB(5, 0, 5);
  }
  FastLED.show();
  delay(1000);  
  tet_game_step(&field, action_start);
}

float angle_diff(float a1, float a2)
{
  float diff = a1 - a2;
  if(diff > 3.1415926) diff = -2*3.1415926 + diff;
  if(diff < -3.1415926) diff = 2*3.1415926 + diff;
  return diff;
}

uint32_t prev_ms = 0;
float avg_roll1 = 0;
float proc_roll1 = 0;
int rot_act_state = 0;
float loc_max = 100;
float zero_roll = 0;
float zero_yaw = 0;
int zero_inited = 0;

void loop() {
  float pitch1 = 0, pitch2 = 0;
  float level1 = 0, level2 = 0;
  float roll1 = 0, roll2 = 0;
  float yaw1 = 0, yaw2 = 0;
  int dev_cnt = uMyo.getDeviceCount();
  if(dev_cnt > 0)
  {
    level1 = uMyo.getAverageMuscleLevel(0);
    roll1 = uMyo.getRoll(0);
    pitch1 = uMyo.getPitch(0);
    yaw1 = uMyo.getYaw(0);
    if(!zero_inited)
    {
      zero_roll = roll1;
      zero_yaw = yaw1;
      zero_inited = 1;
    }
    zero_yaw *= 0.9995;
    zero_yaw += 0.0005*yaw1;
  }
  Serial.printf("Y %g P %g R %g\n", yaw1, pitch1, roll1);
  uint32_t ms = millis();
  static float last_roll_sw = 0;
  float roll_scale = 0.15;
  float droll = roll1 - last_roll_sw;
  static int roll_state = 0;
  int cur_action = action_none;
  static uint32_t prev_roll_tm = 0;
  
  if(droll > roll_scale && cur_action == action_none)
  {
    cur_action = action_ccw;
    roll_state--;
    last_roll_sw = roll1;
  }
  if(droll < -roll_scale && cur_action == action_none)
  {
    cur_action = action_cw;
    roll_state++;
    last_roll_sw = roll1;
  }

  static float last_yaw_sw = 0;
  float dyaw = angle_diff(yaw1, last_yaw_sw);
  float yaw_scale = 0.03;
  static int yaw_state = 0;
  if(dyaw > yaw_scale && cur_action == action_none)
  {
    cur_action = action_right;
    if(yaw_state < 7)
      yaw_state++;
    last_yaw_sw += yaw_scale;// yaw1;
  }
  if(dyaw < -yaw_scale && cur_action == action_none)
  {
    cur_action = action_left;
    if(yaw_state > -7)
      yaw_state--;
    last_yaw_sw -= yaw_scale;//yaw1;
  }


  int tgt_x = yaw_state;
  int tgt_y = -1234;
  int tgt_rot = roll_state;
  while(tgt_rot < 0) tgt_rot += 4;
  tgt_rot %= 4;
  tet_game_step(&field, cur_action);
  tet_draw_array(&field, rgb_states);
  leds[0] = 0;
  for(int l = 0; l < field.SX*field.SY; l++)
  {
    int x = 10 - l/11;
    int y = l%11;
    if(x%2) y = 10 - y;
    int r, g, b;
    int idx = y*11 + x;
    r = rgb_states[idx*3];
    g = rgb_states[idx*3+1];
    b = rgb_states[idx*3+2];
    leds[1+l] = CRGB(r, g, b);
  }
  FastLED.show();
  delay(5);
}
