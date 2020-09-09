#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
//#include <emscripten/fetch.h>
#include <emscripten/html5.h>
#endif
#include "graphics.h"


#include "RenderSystem.h"
#include "RenderQueue.h"
#include "WindowManager.h"
#include "MenuManager.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define USE_AUDIO_MANAGER_VORBIS
#include "AudioManager.h"

#include <algorithm>

#include "immediate-tetris.h"


RenderSystem* g_rs = 0;
WindowManager g_windowManager;
MenuManager g_menuManager;

AudioManager g_audio;


void loop()
{
  g_audio.tick();
  ++g_timer;
  g_menuManager.predraw();
  ImVec2 pos(0.f, 0.f);
  ImVec2 pivot(0, 0);
  ImGui::SetNextWindowPos(pos, 0, pivot);
  ImGui::SetNextWindowSize(ImVec2(WindowManager::getInstance()->width, WindowManager::getInstance()->height));
  ImGui::Begin("Dialog", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
  
  ImGui::PushFont(font_neat);

  if (game_state == MENU)
  {
    bubble_text_unformatted_title(50, 120, 0, "Immediate");
    bubble_text_unformatted_title(97, 175, 1+16, "Tetris");

    ImGui::SetCursorPos(ImVec2(172 - 25, 280));
    if (ImGui::Button("Start"))
    {
      game_state = PLAY;
    }

    ImGui::TextUnformatted("Level");
    const ImS32   s32_zero = 0,   s32_one = 1,   s32_fifty = 50, s32_min = INT_MIN/2,   s32_max = INT_MAX/2,    s32_hi_a = INT_MAX/2 - 100,    s32_hi_b = INT_MAX/2;
    ImGui::SetNextItemWidth(250);
    ImGui::InputScalar("##Level", ImGuiDataType_S32, &g_level, &s32_one, NULL, "%i", 0);
    if (g_level < 1) g_level = 1;
    if (g_level > 9) g_level = 9;
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

    if (ImGui::Button("Restart"))
    {
      game_state = PLAY;
    }

    // ? frame();

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

  ImGui::PopFont();

  ImGui::End();
  g_menuManager.postdraw();

  if (g_action != 's')
    g_action = 0;
};


//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
EM_BOOL key_callback(int action, const EmscriptenKeyboardEvent *e, void *userData)
{
  if (action == EMSCRIPTEN_EVENT_KEYDOWN)
  {
    //if (e->keyCode == GLFW_KEY_LEFT)
    if (strcmp(e->key, "ArrowLeft") == 0)
    {
      g_action = 'a';
    }

    //else if (e->key == GLFW_KEY_RIGHT)
    else if (strcmp(e->key, "ArrowRight") == 0)
    {
      g_action = 'd';      
    }

    //else if (e->key == GLFW_KEY_DOWN)
    else if (strcmp(e->key, "ArrowDown") == 0)
    {
      g_action = 's';
    }

    //else if (e->key == GLFW_KEY_ESCAPE)
    else if (strcmp(e->key, "Escape") == 0)
    {
      g_action = 'q';      
    }

    else
    {
      g_action = 'w';      
    }
  }
  else if (action == EMSCRIPTEN_EVENT_KEYUP)
  {
    //if (e->key == GLFW_KEY_DOWN) g_action = 0;
    if (strcmp(e->key, "ArrowDown") == 0) g_action = 0;
  }

  return true;
}


extern "C" int main(int argc, char** argv)
{
  g_windowManager.width = 345;
  g_windowManager.height = 490;
  g_windowManager.init("Project");
  g_menuManager.init(g_windowManager);

  g_rs = new RenderSystem();
  g_rs->init();

  g_audio.init();
  g_audio.load_ogg("data/ThemeA_by_jnmaloney.ogg");
  g_audio.play(0, true);

  ImGuiIO& io = ImGui::GetIO();
  for (int i = 0; i < 4; ++i)
  {
    font_fill_title[i] = io.Fonts->AddFontFromFileTTF("data/font/vtks-highlight.2.ttf", 48.0f);
    font_outline_title[i] = io.Fonts->AddFontFromFileTTF("data/font/vtks-highlight.3.ttf", 48.0f);
    font_fill[i] = io.Fonts->AddFontFromFileTTF("data/font/vtks-highlight.2.ttf", 24.0f);
    font_outline[i] = io.Fonts->AddFontFromFileTTF("data/font/vtks-highlight.3.ttf", 24.0f);
    font_fill_title[i]->Scale = 1.0 + 0.025 * i;
    font_outline_title[i]->Scale = 1.0 + 0.025 * i;
    font_fill[i]->Scale = 1.0 + 0.025 * i;
    font_outline[i]->Scale = 1.0 + 0.025 * i;
  }

  font_neat = io.Fonts->AddFontFromFileTTF("data/font/OpenSans-Bold.ttf", 24.0f);

  new_piece();

  // Key callbacks
  //glfwSetKeyCallback(g_windowManager.g_window, key_callback);
  emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback);

  #ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(loop, 0, 1);
  #endif

  glfwTerminate();

  return 0;
}
