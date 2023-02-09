#include <Arduino.h>

enum
{
  tet_line = 200,
  tet_L,
  tet_L2,
  tet_T,
  tet_sq,
  tet_z,
  tet_z2,
  tet_none
};

enum
{
  move_down = 0,
  move_left,
  move_right,
  move_up
};

enum
{
  game_waitstart = 0,
  game_needfigure,
  game_waitstep,
  game_figdrop,
  game_figend,
  game_dropend,
  game_waitblow,
  game_over
};

enum
{
  action_none = 0,
  action_start,
  action_left,
  action_right,
  action_cw,
  action_ccw,
  action_drop
};

typedef struct sTField
{
  uint8_t *field;
  int SX;
  int SY;
  int step_time;
  int drop_time;
  int place_time;
  int blow_time;
  int over_time;
};

typedef struct sTet
{
  uint8_t type;
  uint8_t rot;
  int bx[4];
  int by[4];
  uint8_t cx;
  uint8_t cy;
};

int tet_rotate(sTet *tet, sTField *field, uint8_t cw_rot);
int tet_move(sTet *tet, sTField *field, int dir);
int tet_place(sTet *tet, sTField *field);
uint32_t tet_check_field(sTField *field);
void tet_remove_full(sTField *field);
void tet_game_moveto(sTField *field, int tgt_x, int tgt_y, int tgt_rot);
void tet_game_step(sTField *field, int req_action);
void tet_draw_array(sTField *field, uint8_t *rgb_out);
