#include "TextEdit.h"
#include "Helper.h"

#include <numeric>
#include <cassert>
using namespace std;

extern Config _cfg;

void TextEdit::handle_events() {
    if (event_handler && event_handler(*this)) return;
    char c;
    while ((c = GetCharPressed())) {
        if (!on_char_pressed || !on_char_pressed((char)c, *this)) append_at_cursor((char)c);
    }
    if (!oneline && IsKeyPressed(KEY_ENTER)) append_at_cursor('\n');
    if (IsKeyPressed(KEY_TAB)) append_at_cursor('\t');
    if (is_key_hold(KEY_LEFT) || is_ctrl_and_key_hold(KEY_B)) move_cursor_left(1);
    if (is_key_hold(KEY_RIGHT) || is_ctrl_and_key_hold(KEY_F)) move_cursor_right(1);
    if (!oneline && (is_key_hold(KEY_UP) || is_ctrl_and_key_hold(KEY_P))) move_cursor_up(1);
    if (!oneline && (is_key_hold(KEY_DOWN) || is_ctrl_and_key_hold(KEY_N))) move_cursor_down(1);
    if (is_key_hold(KEY_BACKSPACE)) pop_at_cursor(1);
    if (is_alt_and_key_hold(KEY_BACKSPACE)) pop_at_cursor(cursor.idx - get_idx_prev_word());
    if (is_alt_and_key_hold(KEY_F)) move_cursor_right(get_idx_next_word() - cursor.idx);
    if (is_alt_and_key_hold(KEY_B)) move_cursor_left(cursor.idx - get_idx_prev_word());
}

void TextEdit::add_new_line(size_t size) {
    cursor.col = 0;
    cursor.row++;
    line_size.insert(line_size.begin() + cursor.row, size);
}

void TextEdit::append_at_cursor(string_view str) {
    for (char ch : str) {
        append_at_cursor(ch);
    }
}

void TextEdit::append_at_cursor(char c) {
    if (oneline && c == '\n') return;
    buffer.insert(buffer.begin() + cursor.idx, 1, c);
    cursor.idx++;
    cursor.col++;
    line_size[cursor.row]++;
    if (c == '\n') {
        size_t new_line_size = line_size[cursor.row] - cursor.col;
        line_size[cursor.row] -= new_line_size;
        add_new_line(new_line_size);
    }
}

void TextEdit::pop_at_cursor(size_t amount) {
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

void TextEdit::move_cursor_to(size_t row, size_t col) {
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

void TextEdit::move_cursor_to_idx(size_t idx) {
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

size_t TextEdit::get_idx_prev_word() {
    bool found_word = false;
    size_t i = cursor.idx;
    for (; i != 0; i--) {
        if (!is_char_in(buffer[i-1], _cfg.list_words)) found_word = true;
        else if (found_word) break;
    }
    return i;
}

size_t TextEdit::get_idx_next_word() {
    bool found_word = false;
    size_t i = cursor.idx;
    for (; i < buffer.size(); i++) {
        if (!is_char_in(buffer[i], _cfg.list_words)) found_word = true;
        else if (found_word) break;
    }
    return i;
}

string_view TextEdit::get_text(size_t start, size_t end) {
    assert(end >= start);
    return string_view(buffer).substr(start, end);
}


void TextEdit::move_cursor_up(size_t amount) {
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

void TextEdit::move_cursor_down(size_t amount) {
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

void TextEdit::move_cursor_left(size_t amount) {
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

void TextEdit::move_cursor_right(size_t amount) {
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

Vector2 TextEdit::get_cursor_pos() {
    Vector2 result { 0.0f, (float)_cfg.line_height * cursor.row };
    for (size_t i = cursor.idx - cursor.col; i < cursor.idx; i++) {
        char ch = buffer[i];
        result.x += _cfg.char_w(ch);
    }
    return result;
}
