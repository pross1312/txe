#include "Editor.h"
#include "Helper.h"
#include <numeric>
#include <cassert>
using namespace std;


void Editor::add_new_line(size_t size) {
    cursor.col = 0;
    cursor.row++;
    line_size.insert(line_size.begin() + cursor.row, size);
}

int Editor::handle_events() {
         if (is_key_hold(KEY_LEFT)  || is_ctrl_and_key_hold(KEY_B))     move_cursor_left(1);
    else if (is_key_hold(KEY_RIGHT) || is_ctrl_and_key_hold(KEY_F))     move_cursor_right(1);
    else if (is_key_hold(KEY_UP)    || is_ctrl_and_key_hold(KEY_P))     move_cursor_up(1);
    else if (is_key_hold(KEY_DOWN)  || is_ctrl_and_key_hold(KEY_N))     move_cursor_down(1);
    return 0;
}

void Editor::append_at_cursor(string_view str, Color fg, std::optional<Color> bg) {
    for (char ch : str) {
        append_at_cursor(ch, fg, bg);
    }
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

void Editor::pop_at_cursor(size_t amount) {
    if (amount > cursor.idx) amount = cursor.idx;
    cursor.idx -= amount;
    buffer.erase(buffer.begin() + cursor.idx, buffer.begin() + cursor.idx + amount);
    while (amount > 0) {
        if (cursor.col >= amount) {
            cursor.col -= amount;
            line_size[cursor.row] -= amount;
            break;
        } else if (cursor.row == 0) {
            line_size[cursor.row] -= cursor.col;
            cursor.col = 0;
            break;
        } else {
            amount -= cursor.col;
            amount--;
            size_t new_cursor_col = line_size[cursor.row-1]-1;
            line_size[cursor.row-1] += -1 + line_size[cursor.row] - cursor.col;
            cursor.col = new_cursor_col;
            line_size.erase(line_size.begin() + cursor.row);
            cursor.row--;
        }
    }
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
        cursor.col = line_size[row] - (row == line_size.size()-1 ? 0 : 1);
        cursor.idx = std::accumulate(line_size.begin(), line_size.begin() + cursor.row, 0) + cursor.col;
    } else {
        cursor.row = row;
        cursor.col = col;
        cursor.idx = std::accumulate(line_size.begin(), line_size.begin() + cursor.row, 0) + cursor.col;
    }
}

void Editor::move_cursor_to_idx(size_t idx) {
    if (idx >= buffer.size()) {
        cursor.idx = buffer.size();
        cursor.row = line_size.size() - 1;
        cursor.col = line_size[cursor.row];
    } else {
        cursor.idx = idx;
        size_t line = 0;
        while (idx >= line_size[line]) {
            idx -= line_size[line];
            line++;
        }
        cursor.row = line;
        if (line == line_size.size()-1 && line == 0) {
            cursor.col = 0;
        } else {
            cursor.col = idx;
        }
    }
}

size_t Editor::get_idx_prev_word() {
    bool found_word = false;
    size_t i = cursor.idx;
    for (; i != 0; i--) {
        if (!is_char_in(buffer[i-1].c, cfg.list_words)) found_word = true;
        else if (found_word) break;
    }
    return i;
}

size_t Editor::get_idx_next_word() {
    bool found_word = false;
    size_t i = cursor.idx;
    for (; i < buffer.size(); i++) {
        if (!is_char_in(buffer[i].c, cfg.list_words)) found_word = true;
        else if (found_word) break;
    }
    return i;
}

string Editor::get_text(size_t start, size_t end) {
    assert(end >= start);
    string result;
    result.reserve(end - start);
    while (start < buffer.size() && start < end) result.push_back(buffer[start++].c);
    return result;
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
    if (amount > cursor.idx) amount = cursor.idx;
    cursor.idx -= amount;
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
    if (cursor.idx + amount > buffer.size()) amount = buffer.size() - cursor.idx;
    cursor.idx += amount;
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

void Editor::put_cell(Cell cell, Vector2 position, size_t times) {
    static char BUF[2] {};
    if (cell.c == '\t') { // replace tab with space
        cell.c = ' ';
        put_cell(cell, position, (size_t)cfg.tab_size);
    } else {
        BUF[0] = cell.c;
        int ch_w = get_w(cell.c);
        for (size_t i = 0; i < times; i++) {
            if (is_rect_in_view(position.x, position.y, ch_w, cfg.line_height)) {

                if (cell.bg.has_value()) {
                    DrawRectangle(position.x, position.y, ch_w, cfg.font_size, cell.bg.value());
                }
#ifdef USE_SDF_FONT
                BeginShaderMode(cfg.font_shader);
                    DrawTextEx(cfg.font, BUF, position, cfg.font_size, cfg.spacing, cell.fg);
                EndShaderMode();
#else
                DrawTextEx(cfg.font, BUF, position, cfg.font_size, cfg.spacing, cell.fg);
#endif //USE_SDF_FONT
            }
            position.x += ch_w;
        }
    }
}

void Editor::put_cursor(Vector2 position) {
    DrawRectangle(position.x, position.y + cfg.line_height - cfg.cursor_height, cfg.cursor_width, cfg.cursor_height, cfg.cursor_color);
}

Vector2 Editor::get_cursor_pos() {
    Vector2 result { 0.0f, (float)cfg.line_height * cursor.row };
    for (size_t i = cursor.idx - cursor.col; i < cursor.idx; i++) {
        char ch = buffer[i].c;
        result.x += get_w(ch);
    }
    return result;
}

void Editor::update_chars_width() {
    char BUF[2] {};
    for (int i = 32; i < 128; i++) {
        BUF[0] = (char)i;
        chars_width[i] = MeasureTextEx(cfg.font, BUF, cfg.font_size, cfg.spacing).x;
    }
}
