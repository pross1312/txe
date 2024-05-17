#ifndef CONFIG_H
#define CONFIG_H
#include "Helper.h"

#include <raylib.h>
#include <vector>
enum CursorShape {
    Block, Line, Underline
};
const Color DEFAULT_BG = GetColor(0x282828ff);
const Color DEFAULT_FG = WHITE;
struct Config {
    Font font;
    int font_size = 30;
    int line_height = 30;
#ifdef USE_SDF_FONT
    Shader font_shader;
#endif // USE_SDF_FONT
    Color on_selection_bg = GetColor(0x3b3b3bff);

    Color cursor_color = WHITE;
    Color char_at_cursor_color = calculate_luminance(cursor_color) < 140.0f ? WHITE : BLACK;
    CursorShape cursor_shape = CursorShape::Block;
    float cursor_width = cursor_shape == CursorShape::Line ? 2 : 12;
    float cursor_height = cursor_shape == CursorShape::Underline ? 2 : line_height;

    Color search_fg = GetColor(0x282828ff);
    Color search_bg = GetColor(0x73cef4ff);

    Color keyword_color = GetColor(0xc9d05cff);
    Color directive_color = GetColor(0xb3deefff);
    Color comment_color = GetColor(0x757575ff);
    Color string_color = GetColor(0xd3b987ff);

    Color dir_color = SKYBLUE;
    Color file_color = WHITE;
    Color on_cursor_bg_color = GetColor(0x535353ff);
    Color n_on_cursor_bg_color = DEFAULT_BG;

    Color line_number_color = GetColor(0x6d6d6dff);
    Color line_number_region_bg = GetColor(0x222222ff);
    Color current_line_number_color = GetColor(0xdfdfdfff);

    float spacing = 0.0f;
    int tab_size = 4;
    float line_number_width = 70.0f;
    float msg_box_height = line_height + 5.0f;
    const char* list_words = " ;:<>.,!@#$%^&*-=+()[]{}\n\t\r";
    std::vector<const char*> keywords = {
        "#include", "#ifdef", "#ifndef", "#endif", "#define", "#pragma",
        "unsigned", "int", "float", "double", "size_t", "char", "void", "const", "constexpr",
        "enum", "class", "struct", "return", "if", "else", "while", "for", "switch", "case", "default", "break", "continue",
        "inline", "static", "using", "namespace", "typedef"
    };
};

#endif // CONFIG_H
