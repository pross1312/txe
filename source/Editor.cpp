#include "Editor.h"
#include "Helper.h"
#include <numeric>
#include <cassert>
using namespace std;

extern Config _cfg;

void Editor::put_cell(char ch, Attr attr, Vector2 position, size_t times) {
    static char BUF[2] {};
    if (ch == '\t') { // replace tab with space
        ch = ' ';
        put_cell(ch, attr, position, (size_t)_cfg.tab_size);
    } else {
        BUF[0] = ch;
        int ch_w = _cfg.char_w(ch);
        for (size_t i = 0; i < times; i++) {
            if (is_rect_in_view(position.x, position.y, ch_w, _cfg.line_height)) {

                if (attr.bg.has_value()) {
                    DrawRectangle(position.x, position.y, ch_w, _cfg.font_size, attr.bg.value());
                }
#ifdef USE_SDF_FONT
                BeginShaderMode(_cfg.font_shader);
                    DrawTextEx(_cfg.font, BUF, position, _cfg.font_size, _cfg.spacing, attr.fg);
                EndShaderMode();
#else
                DrawTextEx(_cfg.font, BUF, position, _cfg.font_size, _cfg.spacing, attr.fg);
#endif //USE_SDF_FONT
            }
            position.x += ch_w;
        }
    }
}

void Editor::put_cursor(Vector2 position) {
    DrawRectangle(position.x, position.y + _cfg.line_height - _cfg.cursor_height, _cfg.cursor_width, _cfg.cursor_height, _cfg.cursor_color);
}

// Vector2 Editor::get_cursor_pos() {
//     Vector2 result { 0.0f, (float)_cfg.line_height * cursor.row };
//     for (size_t i = cursor.idx - cursor.col; i < cursor.idx; i++) {
//         char ch = buffer[i].c;
//         result.x += _cfg.char_w(ch);
//     }
//     return result;
// }
