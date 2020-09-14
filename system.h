#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif

void init();
void quit();
void loop();
EM_BOOL key_callback(int action, const EmscriptenKeyboardEvent *e, void *userData);
