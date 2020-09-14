#include <stdio.h>

#include "system.h"

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
#include "fonts.h"
#include "juice.h"


RenderSystem* g_rs = 0;
WindowManager g_windowManager;
MenuManager g_menuManager;

AudioManager g_audio;

void quit()
{
  glfwTerminate();
}


void init()
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
    *getFont(FILL_TITLE, i) = io.Fonts->AddFontFromFileTTF("data/font/vtks-highlight.2.ttf", 48.0f);
    (*getFont(FILL_TITLE, i))->Scale = 1.0 + 0.025 * i;
    *getFont(OUTLINE_TITLE, i) = io.Fonts->AddFontFromFileTTF("data/font/vtks-highlight.3.ttf", 48.0f);
    (*getFont(OUTLINE_TITLE, i))->Scale = 1.0 + 0.025 * i;
    *getFont(FILL, i) = io.Fonts->AddFontFromFileTTF("data/font/vtks-highlight.2.ttf", 24.0f);
    (*getFont(FILL, i))->Scale = 1.0 + 0.025 * i;
    *getFont(OUTLINE, i) = io.Fonts->AddFontFromFileTTF("data/font/vtks-highlight.3.ttf", 24.0f);
    (*getFont(OUTLINE, i))->Scale = 1.0 + 0.025 * i;
  }

  *getFont(NEAT, -1) = io.Fonts->AddFontFromFileTTF("data/font/OpenSans-Bold.ttf", 24.0f);

  new_piece();

  // Key callbacks
  emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback);
}


void loop()
{
  g_audio.tick();
  g_menuManager.predraw();

  ImVec2 pos(0.f, 0.f);
  ImVec2 pivot(0, 0);
  ImGui::SetNextWindowPos(pos, 0, pivot);
  ImGui::SetNextWindowSize(ImVec2(WindowManager::getInstance()->width, WindowManager::getInstance()->height));
  imtetris_loop();      
  g_menuManager.postdraw();
};


//void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
EM_BOOL key_callback(int action, const EmscriptenKeyboardEvent *e, void *userData)
{
  if (action == EMSCRIPTEN_EVENT_KEYDOWN)
  {
    if (strcmp(e->key, "ArrowLeft") == 0)
    {
      setAction('a');
    }

    else if (strcmp(e->key, "ArrowRight") == 0)
    {
      setAction('d');
    }

    else if (strcmp(e->key, "ArrowDown") == 0)
    {
      setAction('s');
    }

    else if (strcmp(e->key, "Escape") == 0)
    {
      setAction('q');
    }

    else
    {
      setAction('w');
    }
  }
  else if (action == EMSCRIPTEN_EVENT_KEYUP)
  {
    if (strcmp(e->key, "ArrowDown") == 0) 
      setAction(0);
  }

  return true;
}

