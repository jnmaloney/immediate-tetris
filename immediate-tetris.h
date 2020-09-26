#pragma once

#include <vector>
#include <string>


enum GameState
{
  MENU,
  PLAY,
  LINE,
  GAME_OVER,
  PAUSE,
  SHOW_HIGH_SCORES,
  WAIT_FOR_SERVER,
};


char getAction();
void setAction(char action);
void runloop();
int do_tick();
void frame();
void new_piece();
int get_next_piece();
void restart();
int get_score();
int get_level();
void inc_level();
void remove_lines(const std::vector<int>& lines_to_remove);



void quit_game();
void game_over();
void do_line_routine(std::vector<int>& lines_removed);
void draw_block(int i, int j, int id);
void draw_board();
void draw_score();


void imtetris_loop();