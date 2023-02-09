#include "tetris_eng.h"

sTet cur_tet;

int tet_rotate(sTet *tet, sTField *field, uint8_t cw_rot)
{
  uint8_t nrot;
  if(cw_rot) nrot = (tet->rot + 1)%4;
  else
  {
    if(tet->rot == 0) nrot = 3;
    else nrot = tet->rot - 1;
  }
  int nx[4], ny[4];
  if(tet->type == tet_sq)
  {
    for(int n = 0; n < 4; n++) nx[n] = n%2, ny[n] = n/2;
  }
  if(tet->type == tet_line)
  {
    if(nrot == 0 || nrot == 2) for(int n = 0; n < 4; n++) nx[n] = -1 + n, ny[n] = 0;
    if(nrot == 1 || nrot == 3) for(int n = 0; n < 4; n++) nx[n] = 0, ny[n] = -1 + n;
  }
  if(tet->type == tet_L || tet->type == tet_L2 || tet->type == tet_T)
  {
    if(nrot == 0 || nrot == 2)
    {
      for(int n = 0; n < 3; n++) nx[n] = -1 + n, ny[n] = 0;
      if(tet->type == tet_L) nx[3] = 1 - nrot, ny[3] = 1 - nrot;
      if(tet->type == tet_L2) nx[3] = nrot - 1, ny[3] = 1 - nrot;
      if(tet->type == tet_T) nx[3] = 0, ny[3] = 1 - nrot;
    }
    if(nrot == 1 || nrot == 3)
    {
      for(int n = 0; n < 3; n++) nx[n] = 0, ny[n] = -1 + n;
      if(tet->type == tet_L2) ny[3] = 2 - nrot, nx[3] = 2 - nrot;
      if(tet->type == tet_L) ny[3] = nrot - 2, nx[3] = 2 - nrot;
      if(tet->type == tet_T) ny[3] = 0, nx[3] = 2 - nrot;
    }
  }
  if(tet->type == tet_z || tet->type == tet_z2)
  {
    if(nrot == 0 || nrot == 2)
    {
      nx[0] = 0, ny[0] = 0;
      nx[1] = 1, ny[1] = 0;
      nx[2] = 1, nx[3] = 0;
      if(tet->type == tet_z) ny[2] = 1, ny[3] = -1;
      else ny[2] = -1, ny[3] = 1;
    }
    if(nrot == 1 || nrot == 3)
    {
      nx[0] = 0, ny[0] = 0;
      nx[1] = 0, ny[1] = 1;
      ny[2] = 1, ny[3] = 0;
      if(tet->type == tet_z) nx[2] = -1, nx[3] = 1;
      else nx[2] = 1, nx[3] = -1;
    }
  }
  int done = 0;
  int dx_try = 0;
  int dy_try = 0;
  int attempt = 0;
  int place_ok = 0;
  while(1)
  {
    if(attempt == 1) dx_try = 1;
    if(attempt == 2) dx_try = -1;
    if(attempt == 3) dx_try = 2;
    if(attempt == 4) dx_try = -2;
    if(attempt > 4)
    {
      break;
//      if(dy_try == 0) dy_try = 1, dx_try = 0, attempt = 0;
//      else break;
    }
    attempt++;
    place_ok = 1;
    for(int n = 0; n < 4; n++)
    {
      int bx = tet->cx + dx_try;
      int by = tet->cy + dy_try;
      bx += nx[n];
      by += ny[n];
      if(by >= field->SY) continue; //higher than top, no problem
      if(bx < 0 || bx >= field->SX || by < 0)
      {
        place_ok = 0;
        break;
      }
      if(field->field[by * field->SX + bx])
      {
        place_ok = 0;
        break;
      }
    }
    if(place_ok) break;
  }
  if(!place_ok) return 0;
  tet->rot = nrot;
  tet->cx += dx_try;
  tet->cy += dy_try;
  for(int n = 0; n < 4; n++) tet->bx[n] = nx[n], tet->by[n] = ny[n];
  return 1;
}

int tet_move(sTet *tet, sTField *field, int dir)
{
  int dx, dy;
  if(dir == move_down) dx = 0, dy = -1;
  else if(dir == move_left) dx = -1, dy = 0;
  else if(dir == move_right) dx = 1, dy = 0;
  else if(dir == move_up) dx = 0, dy = 1;
  else return 0; //unknown move -> failure

  int place_ok = 1;
  for(int n = 0; n < 4; n++)
  {
    int bx = tet->cx + tet->bx[n] + dx;
    int by = tet->cy + tet->by[n] + dy;
    if(by >= field->SY)
    {
      if(bx < 0 || bx >= field->SX)
      {
        place_ok = 0;
        break;
      }
      continue; //higher than top, no problem
    }
    if(bx < 0 || bx >= field->SX || by < 0)
    {
      place_ok = 0;
      break;
    }
    if(field->field[by * field->SX + bx])
    {
      place_ok = 0;
      break;
    }
  }
  if(place_ok)
  {
    tet->cx += dx;
    tet->cy += dy;
    return 1;
  }
  return 0;
}

int tet_place(sTet *tet, sTField *field)
{
  for(int n = 0; n < 4; n++)
  {
    int bx = tet->cx + tet->bx[n];
    int by = tet->cy + tet->by[n];
    if(by >= field->SY) continue; //higher than top - nothing to do
    if(bx < 0 || bx >= field->SX || by < 0) return 0; //error, shouldn't happen, random behavior is ok
    field->field[by * field->SX + bx] = tet->type;
  }
  return 1;
}

uint32_t tet_check_field(sTField *field)
{
  uint32_t res = 0;
  for(int y = 0; y < field->SY; y++)
  {
    int full_line = 1;
    for(int x = 0; x < field->SX; x++)
      if(!field->field[y * field->SX + x]) {full_line = 0; break;}
    if(full_line) res |= 1<<y;
  }
  return res;
}

void tet_remove_full(sTField *field)
{
  for(int y = 0; y < field->SY; y++)
  {
    int full_line = 1;
    for(int x = 0; x < field->SX; x++)
      if(!field->field[y * field->SX + x]) {full_line = 0; break;}
    if(full_line)
    {
      for(int y2 = y; y2 < field->SY-1; y2++)
      {
        for(int x = 0; x < field->SX; x++) 
          field->field[y2*field->SX + x] = field->field[(y2+1)*field->SX + x];
      }
      for(int x = 0; x < field->SX; x++) 
        field->field[(field->SY-1) * field->SX + x] = 0;
      y--;
    }
  }
}

void tet_init(sTField *field)
{
  for(int x = 0; x < field->SX*field->SY; x++) 
    field->field[x] = 0;
}

int tet_add_shape(sTField *field, uint8_t type)
{
  if(type == 0) //random
    type = 200 + rand()%7;
  cur_tet.type = type;
  cur_tet.cx = field->SX/2;
  cur_tet.cy = field->SY-1;
  cur_tet.rot = 3;
  return tet_rotate(&cur_tet, field, 1);
}

uint32_t prev_game_ms = 0;
uint8_t game_state = game_waitstart;
uint32_t game_prev_act = 0;

void tet_game_moveto(sTField *field, int tgt_x, int tgt_y, int tgt_rot)
{
  if(game_state != game_waitstep) return;
  int need_move_x = 1;
  int need_move_y = 1;
  int need_rot = 1;
  if(tgt_x == -1234) need_move_x = 0;
  if(tgt_y == -1234) need_move_y = 0;
  if(tgt_rot == -1234) need_rot = 0;
  tgt_x += field->SX/2;
  if(tgt_x < 0) tgt_x = 0;
  if(tgt_x >= field->SX) tgt_x = field->SX-1;
  int attempts = 0;
  if(need_rot)
    while(cur_tet.rot != tgt_rot && attempts++ < 5) tet_rotate(&cur_tet, field, 1);
  attempts = 0;
  if(need_move_x)
  {
    while(cur_tet.cx != tgt_x && attempts++ < field->SX)
    {
      if(cur_tet.cx > tgt_x)
        tet_move(&cur_tet, field, move_left);
      else
        tet_move(&cur_tet, field, move_right);
    }
  }
}

void tet_game_step(sTField *field, int req_action)
{
  uint32_t ms = millis();
  if(ms - prev_game_ms < 10) return;
  prev_game_ms = ms;
  if(game_state == game_waitstart)
  {
    game_prev_act = ms;
//    if(req_action != action_start) return;
    tet_init(field);
    game_state = game_needfigure;
    return;
  }
  if(game_state == game_needfigure)
  {
    int res = tet_add_shape(field, 0);
    if(res) game_state = game_waitstep;
    else game_state = game_over;
    game_prev_act = ms;
    return;
  }
  if(game_state == game_over)
  {
    if(ms < game_prev_act + field->over_time) return;
    tet_init(field);
    game_state = game_waitstart;
    game_prev_act = ms;
    return;
  }
  if(game_state == game_waitblow)
  {
    if(ms < game_prev_act + field->blow_time) return;
    tet_remove_full(field);
    game_state = game_needfigure;
    game_prev_act = ms;
    return;
  }
  if(game_state == game_dropend)
  {
    if(ms < game_prev_act + field->blow_time) return;
    game_state = game_figend;
    game_prev_act = ms;
    return;
  }
  if(game_state == game_figend)
  {
    if(tet_check_field(field)) game_state = game_waitblow;
    else game_state = game_needfigure;
    game_prev_act = ms;
    return;
  }
  if(game_state == game_waitstep)
  {
    if(req_action != action_none)
    {
      if(req_action == action_drop)
      {
        while(1)
        {
          int moved_free = tet_move(&cur_tet, field, move_down);
          if(!moved_free)
          {
            tet_place(&cur_tet, field);
            game_state = game_dropend;
            game_prev_act = ms;
            break;
          }
        }
      }
      if(req_action == action_left) tet_move(&cur_tet, field, move_left);
      if(req_action == action_right) tet_move(&cur_tet, field, move_right);
      if(req_action == action_cw) tet_rotate(&cur_tet, field, 1);
      if(req_action == action_ccw) tet_rotate(&cur_tet, field, 0);
    }
    if(ms < game_prev_act + field->step_time) return;
    int moved_free = tet_move(&cur_tet, field, move_down);
    if(!moved_free && ms > game_prev_act + field->place_time)
    {
      tet_place(&cur_tet, field);
      game_state = game_figend;
    }
    if(moved_free)
      game_prev_act = ms;
    return;
  }
}


void tet_draw_array(sTField *field, uint8_t *rgb_out)
{
  uint32_t ms = millis();
  if(game_state == game_over)
  {
    int phase = (ms - game_prev_act) * 1000 / field->over_time;
    phase = 500 - phase;
    if(phase < 0) phase = -phase;
    phase = 500 - phase;
    int full_lines = field->SY * phase / 500;
    for(int x = 0; x < field->SX; x++)
      for(int y = 0; y < field->SY; y++)
      {
        int r = 2, g = 0, b = 0;
        if(y <= full_lines) r = 30, g = 30, b = 30;
        int idx = 3*(y*field->SX + x);
        rgb_out[idx] = r;
        rgb_out[idx+1] = g;
        rgb_out[idx+2] = b;
      }
    return;
  }
  for(int x = 0; x < field->SX; x++)
    for(int y = 0; y < field->SY; y++)
    {
      int r = 2, g = 0, b = 0;
      int type = field->field[y*field->SX + x];
      if(type != 0) r = 15, g = 5, b = 20; 
      int idx = 3*(y*field->SX + x);
      rgb_out[idx] = r;
      rgb_out[idx+1] = g;
      rgb_out[idx+2] = b;
    }

  for(int n = 0; n < 4; n++)
  {
    int xx = cur_tet.cx + cur_tet.bx[n];
    int yy = cur_tet.cy + cur_tet.by[n];
    if(xx >= 0 && xx < field->SX && yy >= 0 && yy < field->SY)
    {
      int r = 10, g = 20, b = 5;
      int idx = 3*(yy*field->SX + xx);
      rgb_out[idx] = r;
      rgb_out[idx+1] = g;
      rgb_out[idx+2] = b;
      
    }
  }
  
  if(game_state == game_waitblow)
  {
    uint32_t mask = tet_check_field(field);
    int phase = (ms - game_prev_act) * 1000 / field->blow_time;
    for(int x = 0; x < field->SX; x++)
      for(int y = 0; y < field->SY; y++)
      {
        if(mask & 1<<y)
        {
          int r = 2, g = 0, b = 0;
          if(phase < 700)
            r = 5 + phase/10, g = 5 + phase/10, b = 5 + phase/10;
          else
            r = g = b = 76 - (phase-700)/4;
          int idx = 3*(y*field->SX + x);
          rgb_out[idx] = r;
          rgb_out[idx+1] = g;
          rgb_out[idx+2] = b;
        }
      }
  }
}
