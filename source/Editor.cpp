#include "Editor.h"
#include "Helper.h"
#include <numeric>
using namespace std;


void Editor::add_new_line(size_t size) {
    cursor.col = 0;
    cursor.row++;
    line_size.insert(line_size.begin() + cursor.row, size);
}

int Editor::handle_events() {
    int c;
    while ((c = GetCharPressed())) append_at_cursor((char)c);
    if (IsKeyPressed(KEY_TAB))     append_at_cursor('\t');
    if (is_key_hold(KEY_BACKSPACE)) pop_at_cursor();
    if (is_key_hold(KEY_ENTER))     append_at_cursor('\n');
    if (is_key_hold(KEY_LEFT)  || is_alt_and_key_hold(KEY_H))     move_cursor_left(1);
    if (is_key_hold(KEY_RIGHT) || is_alt_and_key_hold(KEY_L))     move_cursor_right(1);
    if (is_key_hold(KEY_UP)    || is_alt_and_key_hold(KEY_K))     move_cursor_up(1);
    if (is_key_hold(KEY_DOWN)  || is_alt_and_key_hold(KEY_J))     move_cursor_down(1);
    return 0;
}

void Editor::append_at_cursor(const char *str, size_t len, Color fg, std::optional<Color> bg) {
    for (size_t i = 0; i < len; i++) {
        append_at_cursor(str[i], fg, bg);
    }
}

void Editor::append_at_cursor(string_view str, Color fg, std::optional<Color> bg) {
    append_at_cursor(str.data(), str.size(), fg, bg);
}

void Editor::append_at_cursor(char c, Color fg, std::optional<Color> bg) {
    Cell cell{c, fg, bg};
    buffer.insert(buffer.begin() + cursor.idx, 1, cell);
    cursor.idx++;
    cursor.col++;
    line_size[cursor.row]++;
    if (c == '\n') {
        size_t new_line_size = line_size[cursor.row] - cursor.col;
        line_size[cursor.row] -= new_line_size;
        add_new_line(new_line_size);
    }
}

void Editor::pop_at_cursor() {
    if (cursor.idx == 0) return;
    buffer.erase(buffer.begin() + cursor.idx-1);
    cursor.idx--;
    if (cursor.col == 0) {
        cursor.col = line_size[cursor.row-1]-1;
        line_size[cursor.row-1] += line_size[cursor.row];
        line_size.erase(line_size.begin() + cursor.row);
        cursor.row--;
    } else {
        cursor.col--;
    }
    line_size[cursor.row]--;
}

void Editor::set_cells_color(size_t start, size_t len, optional<Color> fg, optional<Color> bg) {
    size_t length = std::min(start + len, buffer.size());
    for (size_t i = start; i < length; i++) {
        if (fg.has_value()) buffer[i].fg = fg.value();
        buffer[i].bg = bg;
    }
}

void Editor::move_cursor_to(size_t row, size_t col) {
    if (row >= line_size.size()) {
        cursor.row = line_size.size()-1;
        cursor.col = line_size[cursor.row];
        cursor.idx = buffer.size();
    } else if (col >= line_size[row]) {
        cursor.row = row;
        cursor.col = line_size[row]-1;
        cursor.idx = std::accumulate(line_size.begin(), line_size.begin() + cursor.row, 0) + cursor.col;
    } else {
        cursor.row = row;
        cursor.col = col;
        cursor.idx = std::accumulate(line_size.begin(), line_size.begin() + cursor.row, 0) + cursor.col;
    }
}

void Editor::move_cursor_up(size_t amount) {
    if (cursor.row < amount) {
        cursor.idx = 0;
        cursor.col = 0;
    } else {
        cursor.row -= amount;
        if (cursor.col + 1 > line_size[cursor.row]) {
            cursor.col = line_size[cursor.row]-1;
        }
        cursor.idx = std::accumulate(line_size.begin(), line_size.begin() + cursor.row, 0) + cursor.col;
    }
}

void Editor::move_cursor_down(size_t amount) {
    if (cursor.row + amount >= line_size.size()) {
        cursor.row = line_size.size() - 1;
        cursor.col = line_size[cursor.row];
        cursor.idx = buffer.size();
    } else {
        cursor.row += amount;
        if (cursor.col > line_size[cursor.row]-1) {
            cursor.col = line_size[cursor.row]-1;
        }
        cursor.idx = std::accumulate(line_size.begin(), line_size.begin() + cursor.row, 0) + cursor.col;
    }
}

void Editor::move_cursor_left(size_t amount) {
    if (cursor.idx >= amount) cursor.idx -= amount;
    else cursor.idx = 0;
    while (amount > 0) {
        if (cursor.col >= amount) {
            cursor.col -= amount;
            break;
        } else if (cursor.row == 0) {
            cursor.col = 0;
            break;
        } else {
            amount -= cursor.col;
            amount--;
            cursor.col = line_size[cursor.row-1]-1;
            cursor.row--;
        }
    }
}

void Editor::move_cursor_right(size_t amount) {
    if (buffer.size() - cursor.idx <= amount) cursor.idx = buffer.size();
    else cursor.idx += amount;
    while (amount > 0) {
        if (line_size[cursor.row] - cursor.col >= amount + 1) { // skip \n
            cursor.col += amount;
            break;
        } else if (cursor.row == line_size.size()-1) {
            cursor.col = line_size[cursor.row];
            break;
        } else {
            amount -= line_size[cursor.row] - cursor.col;
            cursor.col = 0;
            cursor.row++;
        }
    }
}

void Editor::put_cell(Cell cell, Vector2 &position, size_t times) {
    static char BUF[2] {};
    if (cell.c == '\t') { // replace tab with space
        cell.c = ' ';
        put_cell(cell, position, (size_t)cfg.tab_size);
    } else if (cell.c == '\n') {
        position.y += cfg.line_height;
        position.x = 0.0f;
    } else {
        BUF[0] = cell.c; BUF[1] = 0;
        int char_width = MeasureTextEx(cfg.font, BUF, cfg.font_size, cfg.spacing).x;
        for (size_t i = 0; i < times; i++) {
            Vector2 pos = world_to_view(position);

            if (is_rect_in_view(position.x, position.y, char_width, cfg.line_height)) {
                if (cell.bg.has_value()) {
                    DrawRectangle(pos.x, pos.y, char_width, cfg.font_size, cell.bg.value());
                }
#ifdef USE_SDF_FONT
                BeginShaderMode(cfg.font_shader);
                    DrawTextEx(cfg.font, BUF, pos, cfg.font_size, cfg.spacing, cell.fg);
                EndShaderMode();
#else
                DrawTextEx(cfg.font, BUF, pos, cfg.font_size, cfg.spacing, cell.fg);
#endif //USE_SDF_FONT
            }
            position.x += char_width;
        }
    }
}

void Editor::put_cursor(Vector2 position) {
    position = world_to_view(position);
    DrawRectangle(position.x, position.y, cfg.cursor_width, cfg.line_height, cfg.cursor_color);
}

Vector2 Editor::get_cursor_pos() {
    static char BUF[2] {};
    Vector2 result { 0.0f, (float)cfg.line_height * cursor.row };
    for (size_t i = cursor.idx - cursor.col; i < cursor.idx; i++) {
        BUF[0] = buffer[i].c;
        if (BUF[0] == '\t') {
            BUF[0] = ' ';
            result.x += MeasureTextEx(cfg.font, BUF, cfg.font_size, cfg.spacing).x * cfg.tab_size;
        } else {
            result.x += MeasureTextEx(cfg.font, BUF, cfg.font_size, cfg.spacing).x;
        }
    }
    return result;
}

void Editor::bring_point_into_view(Vector2 point) {
    if (point.x > view.x + view.width - PADDING_BOTTOM_RIGHT.x) view.x += point.x - (view.x + view.width) + PADDING_BOTTOM_RIGHT.x;
    else if (point.x < view.x) view.x -= view.x - point.x + PADDING_TOP_LEFT.x;

    if (point.y + cfg.line_height > view.y + view.height - PADDING_BOTTOM_RIGHT.y) view.y += point.y + cfg.line_height - (view.y + view.height) + PADDING_BOTTOM_RIGHT.y;
    else if (point.y < view.y) view.y -= view.y - point.y + PADDING_TOP_LEFT.y;
}
