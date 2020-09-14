#include "system.h"


extern "C" int main(int argc, char** argv)
{
  init();

  #ifdef __EMSCRIPTEN__
  emscripten_set_main_loop(loop, 0, 1);
  #endif

  quit();

  return 0;
}
