#ifndef CONFIG_H
#define CONFIG_H

#include <raylib.h>

const Color DEFAULT_BG = BLACK;
const Color DEFAULT_FG = WHITE;
struct Config {
    Font font;
    int font_size;
    int line_height;
#ifdef USE_SDF_FONT
    Shader font_shader;
#endif // USE_SDF_FONT

    Color cursor_color;
    Color dir_color = SKYBLUE;
    Color file_color = WHITE;
    Color on_cursor_bg_color = GetColor(0x313131ff);
    Color n_on_cursor_bg_color = DEFAULT_BG;

    float spacing = 0.0f;
    int cursor_width = 2;
    int tab_size = 4;
};

#endif // CONFIG_H
