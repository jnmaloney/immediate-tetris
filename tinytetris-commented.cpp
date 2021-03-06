// Modified and adapted from taylorconor/tinytetris
#include <ctime>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <list>
#include <vector>
#include <algorithm>
#include "immediate-tetris.h"

#include <chrono>


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


int x = 431424, y = 598356, r = 427089, px = 247872, py = 799248, pr,
    c = 348480, p = 615696, tick, board[20][10],
    score = 0, next_p = -1;

int get_next_piece() { return next_p; }
int get_score() { return score; }


void restart() {
  memset(&board[0][0], 0, 20 * 10 * sizeof(int));
  next_p = -1;
  next_piece_list.clear();
  score = 0;
}

// extract a 2-bit number from a block entry
int NUM(int x, int y, int piece_id);
int GET_LEFT_NUM(int r, int piece_id) { return NUM(r, 22, piece_id); }
int GET_RIGHT_NUM(int r, int piece_id) { return NUM(r, 16, piece_id); }
int NUM_NEXT(int x, int y) { return NUM(x, y, next_p); }

// create a new piece, don't remove old one (it has landed and should stick)
void new_piece() {
  // Set piece queue
  if (next_piece_list.size() < 7)
  {
    shuffle(piece_pool);
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
  x = px = 4;

  setAction(0);
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
    board[NUM(r, i * 2, p) + y][NUM(r, (i * 2) + 2, p) + x] = v;
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
  //for (int row = y; row <= y + NUM(r, 18, p); row++) { // ??
  for (int row = y; row <= y + 4; row++) {
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

  // check hit bottom of map
  if (y + NUM(r, 18, p) > 19) { // ??
    return 1;
  }

  // check left edge
  if (x + NUM(r, 22, p) < 0)
  {
    return 1;
  }

  // remove piece and do something
  set_piece(px, py, pr, 0);
  c = 0;
  for (int i = 0; i < 8; i += 2) {
    board[y + NUM(r, i * 2, p)][x + NUM(r, (i * 2) + 2, p)] && c++;
  }
  set_piece(px, py, pr, p + 1);
  return c;
}

// slowly tick the piece y position down so the piece falls
int do_tick() {
  std::vector<int> lines_removed;
  int tick_timer = 60 * powf(0.8 - ((get_level() - 1) * 0.007), get_level() - 1);
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

      // // HACK timing
      // static auto t1 = std::chrono::high_resolution_clock::now();
      // static auto t2 = std::chrono::high_resolution_clock::now();
      // printf("timer: %i\n", std::chrono::duration_cast<std::chrono::milliseconds>(t2-t1).count());
      // t2 = t1;
      // t1 = std::chrono::high_resolution_clock::now();
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
    //if ((c = getAction()) == 'a' && x > 0 && !check_hit(x - 1, y, r)) {
    if ((c = getAction()) == 'a' && !check_hit(x - 1, y, r)) {
      x--;
    }
    if (c == 'd' && x + NUM(r, 16, p) < 9 && !check_hit(x + 1, y, r)) { // ??
      x++;
    }
    if (c == 's') {
      tick += 30;
    }

    // Rotation
    if (c == 'w') {

      (r+=1) %= 4;
      
      // kick off wall
      while (x + GET_LEFT_NUM(r, p) < 0) ++x;
      while (x + GET_RIGHT_NUM(r, p) > 9) --x;

      bool rotate_blocked = true;
      if (!check_hit(x, y, r)) 
      {
        rotate_blocked = false;
      }
      // kick right
      if (rotate_blocked && !check_hit(x + 1, y, r)) 
      {
         x = x + 1;
        rotate_blocked = false;
      }
      // kick left
      if (rotate_blocked && !check_hit(x - 1, y, r))
      {
        x = x - 1;
        rotate_blocked = false;        
      }

      if (rotate_blocked)
      {
        // reset (don't rotate)
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
