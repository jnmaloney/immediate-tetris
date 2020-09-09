#include <vector>
#include "imgui.h"


ImFont* font_fill[4];
ImFont* font_outline[4];
ImFont* font_fill_title[4];
ImFont* font_outline_title[4];
ImFont* font_neat;


enum GameState
{
  MENU,
  PLAY,
  LINE,
  GAME_OVER,
  PAUSE
};


GameState game_state = MENU;
char g_action = 0;
char getAction() { return g_action; }
void setAction() { g_action = 0; }
int g_level = 1;
int line_timer = 0;
int line_timer_max = 30;
int g_timer = 0;
std::vector<int> g_lines_removed;


void runloop();
int do_tick();
void frame();
void new_piece();
int get_next_piece();
void restart();
int get_score();
int get_level() { return g_level; }
void inc_level() { ++g_level; }
void remove_lines(const std::vector<int>& lines_to_remove);


// A G B R
const ImU32 palette[8] = {
  0xFFa67b77,
  0xFF0b30fe,
  0xFF5c5ac4,
  0xFFcb52b5,
  0xFF4fabcf,
  0xFF76a78c,
  0xFFb4c247,
  0xFFd34a76,
};


void quit_game()
{
  game_state = PAUSE;
}


void game_over()
{
  game_state = GAME_OVER;
  restart();
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
    //scale = 0.75f;
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

  ImU32 colour = palette[id - 1];
  list->AddRectFilled(a, c, colour);
  list->AddRect(a, c, 0x22000000);  
}


void bubble_text_i(ImFont** a, ImFont** b, int x, int y, int off, std::string text)
{
  for (int i = 0; i < text.length(); ++i)
  {
    int t = (i + off);
    int size = (int)(3.9f * powf(sinf(((t%32)/32.f) * M_PI - 0.04f*g_timer), 56.f));
    ImU32 col = palette[t % 8];

    ImGui::SetCursorPos(ImVec2(x, y-size/2.0));
    ImGui::PushFont(a[size]);
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::Text("%s", text.substr(i, 1).c_str());
    ImGui::PopStyleColor();
    ImGui::PopFont();

    ImGui::SetCursorPos(ImVec2(x, y-size/2.0));
    ImGui::PushFont(b[size]);
    ImGui::Text("%s", text.substr(i, 1).c_str());
    x += ImGui::CalcTextSize(text.substr(i, 1).c_str()).x;
    ImGui::PopFont();
  } 
}


void bubble_text_unformatted_title(int x, int y, int off, std::string text)
{
  bubble_text_i(font_fill_title, font_outline_title, x, y, off, text);
}


void bubble_text_unformatted(int x, int y, int off, std::string text)
{
  bubble_text_i(font_fill, font_outline, x, y, off, text);
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
  ImGui::PushFont(font_outline[0]);
  ImGui::Text("%i", get_score());
  ImGui::PopFont();


  bubble_text_unformatted(255, 295, get_level() + 16 - 2, "Level");
  ImGui::SetCursorPos(ImVec2(255, 320));
  ImGui::PushFont(font_outline[0]);
  ImGui::Text("%i", get_level());
  ImGui::PopFont();
}
