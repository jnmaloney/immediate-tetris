#include "imgui.h"


enum FONT_NAME
{
  NEAT,
  FILL,
  OUTLINE,
  FILL_TITLE,
  OUTLINE_TITLE,
};


ImFont** getFont(FONT_NAME name, int index);
ImFont** getFontArray(FONT_NAME name);