#include "imgui.h"
#include "juice.h"
#include "fonts.h"
#include "game_values.h"


void bubble_text_i(ImFont** a, ImFont** b, int x, int y, int off, std::string text)
{
  for (int i = 0; i < text.length(); ++i)
  {
    int t = (i + off);
    int size = (int)(3.9f * powf(sinf(((t%32)/32.f) * M_PI - 0.04f*getTimer()), 56.f));
    ImU32 col = getPalette(t % 8);

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
  bubble_text_i(getFontArray(FILL_TITLE), getFontArray(OUTLINE_TITLE), x, y, off, text);
}


void bubble_text_unformatted(int x, int y, int off, std::string text)
{
  bubble_text_i(getFontArray(FILL), getFontArray(OUTLINE), x, y, off, text);
}

