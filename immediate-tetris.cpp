#include "immediate-tetris.h"
#include "imgui.h"
#include "fonts.h"
#include "juice.h"
#include <algorithm>
#include "game_values.h"
#include "highscore.h"
#include <random>
#include <chrono>


GameState game_state = MENU;
char g_action = 0;
int g_timer = 0;
int getTimer() { return g_timer; }
int g_level = 1;
int line_timer = 0;
int line_timer_max = 30;
std::vector<int> g_lines_removed;


char getAction() { return g_action; }
void setAction(char action) 
{ 
  g_action = action; 
}


int get_level() { return g_level; }
void inc_level() { ++g_level; }


// A G B R
const ImU32 palette[8] = {
  0xFFa67b77,
  0xFFb4c247,
  0xFF5c5ac4,
  0xFF4fabcf,
  0xFF76a78c,
  0xFFcb52b5,
  0xFF0b30fe,
  0xFFd34a76,
};
ImU32 getPalette(int i) { return palette[i]; }


//
// Tetronimo and rotation list
//

std::vector< std::pair<int, int> > tetronimo_size_list = 
{
  { 4, 4 },
  { 3, 3 },
  { 3, 3 },
  { 4, 3 },
  { 3, 3 },
  { 3, 3 },
  { 3, 3 },
};

std::vector< std::vector< std::vector<int> > > tetronimo_piece =
{
  // I
  {
    { 
      0, 0, 0, 0,
      1, 1, 1, 1,
      0, 0, 0, 0,
      0, 0, 0, 0,
    },
    {
      0, 0, 1, 0,
      0, 0, 1, 0,
      0, 0, 1, 0,
      0, 0, 1, 0,
    },
    {
      0, 0, 0, 0,
      0, 0, 0, 0,
      1, 1, 1, 1,
      0, 0, 0, 0,
    },
    {
      0, 1, 0, 0,
      0, 1, 0, 0,
      0, 1, 0, 0,
      0, 1, 0, 0,
    }
  },
  // J
  {
    {
      1, 0, 0,
      1, 1, 1,
      0, 0, 0,
    },
    {
      0, 1, 1, 
      0, 1, 0,
      0, 1, 0,
    },
    {
      0, 0, 0,
      1, 1, 1, 
      0, 0, 1,
    },
    {
      0, 1, 0,
      0, 1, 0,
      1, 1, 0,
    }
  },
  // L
  {
    {
      0, 0, 1,
      1, 1, 1,
      0, 0, 0,
    },
    {
      0, 1, 0, 
      0, 1, 0,
      0, 1, 1,
    },
    {
      0, 0, 0,
      1, 1, 1,
      1, 0, 0,
    },
    {
      1, 1, 0,
      0, 1, 0,
      0, 1, 0,
    }
  },
  // O
  {
    {
      0, 1, 1, 0,
      0, 1, 1, 0,
      0, 0, 0, 0,
    },
    {
      0, 1, 1, 0,
      0, 1, 1, 0,
      0, 0, 0, 0,
    },
    {
      0, 1, 1, 0,
      0, 1, 1, 0,
      0, 0, 0, 0,
    },
    {
      0, 1, 1, 0,
      0, 1, 1, 0,
      0, 0, 0, 0,
    }
  },
  // S
  {
    {
      0, 1, 1, 
      1, 1, 0,
      0, 0, 0,
    },
    {
      0, 1, 0,
      0, 1, 1,
      0, 0, 1,
    },
    {
      0, 0, 0,
      0, 1, 1, 
      1, 1, 0,
    },
    {
      1, 0, 0,
      1, 1, 0,
      0, 1, 0,
    }
  },
  // T
  {
    {
      0, 1, 0,
      1, 1, 1,
      0, 0, 0,
    },
    {
      0, 1, 0,
      0, 1, 1,
      0, 1, 0,
    },
    {
      0, 0, 0,
      1, 1, 1,
      0, 1, 0,
    },
    {
      0, 1, 0,
      1, 1, 0,
      0, 1, 0,
    }
  },
  // Z
  {
    {
      1, 1, 0,
      0, 1, 1,
      0, 0, 0,
    },
    {
      0, 0, 1,
      0, 1, 1, 
      0, 1, 0,
    },
    {
      0, 0, 0,
      1, 1, 0,
      0, 1, 1,
    },
    {
      0, 1, 0,
      1, 1, 0,
      1, 0, 0,
    }
  }
};


void quit_game()
{  
  game_state = PAUSE;
}


void game_over()
{
  game_state = GAME_OVER;
}


void do_line_routine(std::vector<int>& lines_removed)
{
  g_lines_removed = lines_removed;
  game_state = LINE;
  line_timer = line_timer_max;
}


void draw_block(int i, int j, int id)
{
  // Check if row is in lines
  float scale = 1.0f;
  if (std::find(g_lines_removed.begin(), g_lines_removed.end(), j) != g_lines_removed.end())
  {
    float w = 11.f; // magic 10
    float inc = 0.5 * line_timer_max * (w - i) / w; 
    float a = (float)(2.f * line_timer + inc - line_timer_max) / (float)(line_timer_max);
    a = std::max(a, 0.0f);
    a = std::min(a, 1.0f);
    scale = a;
  }

  ImDrawList* list = ImGui::GetWindowDrawList(); 
  int size = 24;
  ImVec2 a(5 + i * size, 5 + j * size);
  ImVec2 c(5 + (i + 1) * size, 5 + (j + 1) * size);

  a.x += 0.5f * (1.0 - scale) * size;
  a.y += 0.5f * (1.0 - scale) * size;
  c.x -= 0.5f * (1.0 - scale) * size;
  c.y -= 0.5f * (1.0 - scale) * size;

  if (c.x - a.x < 2.0f) return;

  // Draw the block
  ImU32 colour = palette[id - 1];
  list->AddRectFilled(a, c, colour);
  list->AddRect(a, c, 0x22000000);  
}


void draw_board()
{
  {
    ImDrawList* list = ImGui::GetWindowDrawList(); 
    int size = 24;
    ImVec2 a(5 + 0, 5 + 0);
    ImVec2 c(5 + 10 * size, 5 + 20 * size);
    ImU32 colour = 0xffffffff;
    list->AddRectFilled(a, c, colour);
  }

  frame();
}


void draw_score()
{
  bubble_text_unformatted(255, 175, get_level(), "Score");
  ImGui::SetCursorPos(ImVec2(255, 200));
  //ImGui::PushFont(font_outline[0]);
  ImGui::Text("%i", get_score());
  //ImGui::PopFont();


  bubble_text_unformatted(255, 295, get_level() + 16 - 2, "Level");
  ImGui::SetCursorPos(ImVec2(255, 320));
  //ImGui::PushFont(font_outline[0]);
  ImGui::Text("%i", get_level());
  //ImGui::PopFont();
}


void imtetris_loop()
{
  ++g_timer;

  ImGui::PushFont(*getFont(NEAT, -1));
  ImGui::Begin("Dialog", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

  if (game_state == MENU)
  {
    bubble_text_unformatted_title(50, 120, 0, "Immediate");
    bubble_text_unformatted_title(97, 175, 1+16, "Tetris");

    // Starting level selection
    ImGui::SetCursorPos(ImVec2(172 - 75, 280));
    ImGui::TextUnformatted("Level");
    const ImS32   s32_zero = 0,   s32_one = 1;//,   s32_fifty = 50, s32_min = INT_MIN/2,   s32_max = INT_MAX/2,    s32_hi_a = INT_MAX/2 - 100,    s32_hi_b = INT_MAX/2;
    ImGui::SameLine();
    ImGui::SetNextItemWidth(90);
    ImGui::InputScalar("##Level", ImGuiDataType_S32, &g_level, &s32_one, NULL, "%i", 0);
    if (g_level < 1) g_level = 1;
    if (g_level > 9) g_level = 9;

    // Your name for high scores
    //ImGui::TextUnformatted("Your name");

    ImGui::SetCursorPos(ImVec2(172 - 25, 320));
    if (ImGui::Button("Start"))
    {
      game_state = PLAY;
    }

    ImGui::SetCursorPos(ImVec2(172 - 95, 420));
    if (ImGui::Button("Show High Scores"))
    {
      game_state = SHOW_HIGH_SCORES;
    }
  }
  else if (game_state == PAUSE)
  {
    ImGui::TextUnformatted("Paused");

    if (ImGui::Button("Continue"))
    {
      game_state = PLAY;
    }

    draw_score();
  }
  else if (game_state == GAME_OVER)
  {
    ImGui::TextUnformatted("Game Over");

    // Show score panel (current/hi)

    ImGui::TextUnformatted("Enter your name to submit high score");
    static char buf[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    ImGui::InputText("Your Name: ", buf, IM_ARRAYSIZE(buf));

    if (ImGui::Button("Submit"))
    {
      score_game_ended(std::string(buf), get_score());
      //game_state = SHOW_HIGH_SCORES;
      game_state = WAIT_FOR_SERVER;
    }

    ImGui::Separator();
    if (ImGui::Button("Main Menu"))
    {
      restart();
      game_state = MENU;
    }

    draw_score();
  }
  else if (game_state == PLAY)
  {      
    runloop();
    draw_board();
    draw_score();
  }
  else if (game_state == LINE)
  {
    draw_board();
    draw_score();
    --line_timer;
    if (line_timer <= 0)
    {
      remove_lines(g_lines_removed);
      g_lines_removed.clear();
      game_state = PLAY;
    }
  }
  else if (game_state == SHOW_HIGH_SCORES)
  {
    ImGui::TextUnformatted("High Scores");

    std::vector<Score> scores;
    score_get(scores);
    for (auto& i: scores)
    {
      ImGui::Text("%s", i.name.c_str());
      ImGui::SameLine();
      ImGui::Text(" %i ", i.score);
    }

    if (ImGui::Button("Main Menu"))
    {
      game_state = MENU;
    }
  }
  else if (game_state == WAIT_FOR_SERVER)
  {
    ImGui::Text("Waiting...");
  }

  ImGui::PopFont();

  ImGui::End();
  if (g_action != 's')
    g_action = 0;
}


int NUM(int x, int y, int piece_id)
{
  // x: 0, 1, 2, 3 (rotation)
  // y: 0, 4, 8, 12 (x pos)
  //    2, 6, 10, 14 (y pos)  
  //    16 (width)
  //    18 (height)
  //    22 (made up, left edge)

  if (y == 22)
  {
    int min_x = 4;
    auto size = tetronimo_size_list[piece_id];
    auto piece = tetronimo_piece[piece_id][x];

    // find width
    for (int i = 0; i < size.first; ++i)
      for (int j = 0; j < size.second; ++j)
        if (piece[i + j * size.first])
          min_x = std::min(min_x, i);
    return min_x;
  }

  if (y == 16)
  {
    int max_x = 0;
    auto size = tetronimo_size_list[piece_id];
    auto piece = tetronimo_piece[piece_id][x];

    // find width
    for (int i = 0; i < size.first; ++i)
      for (int j = 0; j < size.second; ++j)
        if (piece[i + j * size.first])
          max_x = std::max(max_x, i);
    return max_x;
  }

  if (y == 18)
  {
    int max_y = 0;
    auto size = tetronimo_size_list[piece_id];
    auto piece = tetronimo_piece[piece_id][x];

    // find width
    for (int i = 0; i < size.first; ++i)
      for (int j = 0; j < size.second; ++j)
        if (piece[i + j * size.first])
          max_y = std::max(max_y, j);
    return max_y;
  }

  int c = 0;
  int xy = 0;

  if (y == 0) { c = 1; xy = 0; }
  if (y == 4) { c = 2; xy = 0; }
  if (y == 8) { c = 3; xy = 0; }
  if (y == 12) { c = 4; xy = 0; }

  if (y == 2) { c = 1; xy = 1; }
  if (y == 6) { c = 2; xy = 1; }
  if (y == 10) { c = 3; xy = 1; }
  if (y == 14) { c = 4; xy = 1; }

  int k = 0;
  int i = 0;
  int j = 0;
  auto size = tetronimo_size_list[piece_id];
  auto piece = tetronimo_piece[piece_id];
  int n = 0;
  while (k < c)
  {
    if (i == size.first) 
    {
      i = 0;
      ++j;
    }
    if (j == size.second)
      break;

    if (piece[x][i + j * size.first])
    {
      ++k;
      if (xy == 0) n = j;
      if (xy == 1) n = i;
    }

    ++i;
  }
  return n;
}


void event_download_succeded()
{
  if (game_state == WAIT_FOR_SERVER)
    game_state = SHOW_HIGH_SCORES;
}


void shuffle(std::vector<int>& piece_pool)
{
  static int seed = std::chrono::system_clock::now().time_since_epoch().count();
  static std::default_random_engine generator(seed);
  std::shuffle(piece_pool.begin(), piece_pool.end(), generator);
}
