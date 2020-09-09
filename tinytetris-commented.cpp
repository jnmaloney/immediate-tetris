// Modified and adapted from taylorconor/tinytetris
#include <ctime>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <list>
#include <vector>
#include <algorithm>
#include <random>
#include "immediate-tetris.h"


int all_line_count = 0;
int goals[9] = 
{
  5,    //   5
  15,   //  10
  30,   //  15
  50,   //  20
  75,   //  25
 105,   //  30
 140,   //  35
 180,   //  40
 225,   //  45
};

std::list<int> next_piece_list;
std::vector<int> piece_pool = { 0, 1, 2, 3, 4, 5, 6 };


// block layout is: {w-1,h-1}{x0,y0}{x1,y1}{x2,y2}{x3,y3} (two bits each)
int x = 431424, y = 598356, r = 427089, px = 247872, py = 799248, pr,
    c = 348480, p = 615696, tick, board[20][10],
    block[7][4] = {{x, y, x, y},
                   {r, p, r, p},
                   {c, c, c, c},
                   {599636, 431376, 598336, 432192},
                   {411985, 610832, 415808, 595540},
                   {px, py, px, py},
                   {614928, 399424, 615744, 428369}},
    score = 0, next_p = -1;

int get_next_piece() { return next_p; }
int get_score() { return score; }


void restart() {
  memset(&board[0][0], 0, 20 * 10 * sizeof(int));
  next_p = -1;
  score = 0;
}

// extract a 2-bit number from a block entry
int NUM(int x, int y) { return 3 & block[p][x] >> y; }
int NUM_NEXT(int x, int y) { return 3 & block[next_p][x] >> y; }

// create a new piece, don't remove old one (it has landed and should stick)
void new_piece() {
  // Set piece queue
  if (next_piece_list.size() < 7)
  {
    int seed = y;
    std::shuffle(piece_pool.begin(), piece_pool.end(), std::default_random_engine(seed));
    for (auto i: piece_pool)
    {
      next_piece_list.push_back(i);
    }
  }   

  if (next_p < 0) { // weird init
    next_p = next_piece_list.front();
    next_piece_list.pop_front();
  }
  
  // Make new piece (and adjust next piece)
  y = py = 0;
  p = next_p;
  
  next_p = next_piece_list.front();
  next_piece_list.pop_front();

  r = pr = 0;
  x = px = 5;

  setAction();
}

// draw the board and score
void frame() {
  for (int i = 0; i < 20; i++) {
    for (int j = 0; j < 10; j++) {
      if (board[i][j])
        draw_block(j, i, board[i][j]);
    }
  }

  // Draw next piece (1)
  int r = 0;
  int y = 1;
  int x = 10;
  for (int i = 0; i < 8; i += 2) {
    int a = NUM_NEXT(r, i * 2) + y;
    int b = NUM_NEXT(r, (i * 2) + 2) + x;
    draw_block(b, a, next_p + 1);
  }
}

// set the value fo the board for a particular (x,y,r) piece
void set_piece(int x, int y, int r, int v) {
  for (int i = 0; i < 8; i += 2) {
    board[NUM(r, i * 2) + y][NUM(r, (i * 2) + 2) + x] = v;
  }
}

// move a piece from old (p*) coords to new
void update_piece() {
  set_piece(px, py, pr, 0);
  set_piece(px = x, py = y, pr = r, p + 1);
}

// remove line(s) from the board if they're full
int remove_line(std::vector<int>& lines_removed) {
  int nlines = 0;
  for (int row = y; row <= y + NUM(r, 18); row++) {
    c = 1;
    for (int i = 0; i < 10; i++) {
      c *= board[row][i];
    }
    if (!c) {
      continue;
    }
    ++nlines;
    lines_removed.push_back(row);
  }
  return nlines;
}

void remove_lines(const std::vector<int>& lines_to_remove) {
  for (int row : lines_to_remove) {
    for (int i = row - 1; i > 0; i--) {
       memcpy(&board[i + 1][0], &board[i][0], 40);
    }
    memset(&board[0][0], 0, 10);
  }

  new_piece();
}

// check if placing p at (x,y,r) will be a collision
int check_hit(int x, int y, int r) {
  if (y + NUM(r, 18) > 19) {
    return 1;
  }
  set_piece(px, py, pr, 0);
  c = 0;
  for (int i = 0; i < 8; i += 2) {
    board[y + NUM(r, i * 2)][x + NUM(r, (i * 2) + 2)] && c++;
  }
  set_piece(px, py, pr, p + 1);
  return c;
}

// TODO: Time = (0.8-((Level-1)*0.007))^(Level-1) 
// slowly tick the piece y position down so the piece falls
int do_tick() {
  std::vector<int> lines_removed;
  int tick_timer = 33 - (get_level() * 3);
  if (++tick > tick_timer) {
    tick = 0;
    int nlines = 0;
    if (check_hit(x, y + 1, r)) {
      if (!y) {
        game_over();
        return 0;
      }
      nlines = remove_line(lines_removed);
      if (nlines == 0) new_piece();
    } else {
      y++;
      update_piece();
    }
    if (nlines == 1) {
      score +=   40 * get_level();
      all_line_count += 1;
    }
    if (nlines == 2) {
      score +=  100 * get_level();
      all_line_count += 3;
    }
    if (nlines == 3) {
      score +=  300 * get_level();
      all_line_count += 5;
    }
    if (nlines == 4) {
      score += 1200 * get_level();
      all_line_count += 8;
    }
    if (get_level() < 9 && all_line_count > goals[get_level()])
    {
      inc_level();
    }
  }
  if (lines_removed.size()) do_line_routine(lines_removed);
  return 1;
}

// main game loop with wasd input checking
void runloop() {
  if (do_tick()) {
    if ((c = getAction()) == 'a' && x > 0 && !check_hit(x - 1, y, r)) {
      x--;
    }
    if (c == 'd' && x + NUM(r, 16) < 9 && !check_hit(x + 1, y, r)) {
      x++;
    }
    if (c == 's') {
      tick += 30;
    }
    if (c == 'w') {
      (r+=3) %= 4;
      while (x + NUM(r, 16) > 9) {
        x--;
      }
      if (check_hit(x, y, r)) {
        x = px;
        r = pr;
      }
    }
    if (c == 'q') {
      quit_game();
    }
    update_piece();
  }
}
