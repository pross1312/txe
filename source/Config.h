#ifndef CONFIG_H
#define CONFIG_H

#include <raylib.h>

const Color DEFAULT_BG = GetColor(0x282828ff);
const Color DEFAULT_FG = WHITE;
struct Config {
    Font font;
    int font_size = 30;
    int line_height = 30;
#ifdef USE_SDF_FONT
    Shader font_shader;
#endif // USE_SDF_FONT

    Color cursor_color = WHITE;

    Color dir_color = SKYBLUE;
    Color file_color = WHITE;
    Color on_cursor_bg_color = GetColor(0x313131ff);
    Color n_on_cursor_bg_color = DEFAULT_BG;

    Color line_number_color = GetColor(0x6d6d6dff);
    Color line_number_region_bg = GetColor(0x222222ff);
    Color current_line_number_color = GetColor(0xdfdfdfff);

    float spacing = 0.0f;
    int cursor_width = 2;
    int tab_size = 4;
    float line_number_width = 70.0f;
};

#endif // CONFIG_H
