#include "fonts.h"


ImFont* font_fill[4];
ImFont* font_outline[4];
ImFont* font_fill_title[4];
ImFont* font_outline_title[4];
ImFont* font_neat;


ImFont** getFont(FONT_NAME name, int id)
{
  if (name == NEAT) return &font_neat;
  if (name == FILL) return &font_fill[id];
  if (name == OUTLINE) return &font_outline[id];
  if (name == FILL_TITLE) return &font_fill_title[id];
  if (name == OUTLINE_TITLE) return &font_outline_title[id];
  return NULL;
}


ImFont** getFontArray(FONT_NAME name)
{
  if (name == FILL) return font_fill;
  if (name == OUTLINE) return font_outline;
  if (name == FILL_TITLE) return font_fill_title;
  if (name == OUTLINE_TITLE) return font_outline_title;
  return NULL;
}
