#include "Editor.h"
#include "Helper.h"
#include <numeric>
using namespace std;


void Editor::add_new_line(size_t size) {
    cursor.col = 0;
    cursor.row++;
    line_size.insert(line_size.begin() + cursor.row, size);
}

void Editor::handle_events() {
    int c;
    while ((c = GetCharPressed())) append_at_cursor((char)c);
    if (is_key_hold(KEY_BACKSPACE)) pop_at_cursor();
    if (is_key_hold(KEY_ENTER))     append_at_cursor('\n');
    if (is_key_hold(KEY_LEFT)  || is_ctrl_and_key_hold(KEY_B))     move_cursor_left(1);
    if (is_key_hold(KEY_RIGHT) || is_ctrl_and_key_hold(KEY_F))     move_cursor_right(1);
    if (is_key_hold(KEY_UP)    || is_ctrl_and_key_hold(KEY_P))     move_cursor_up(1);
    if (is_key_hold(KEY_DOWN)  || is_ctrl_and_key_hold(KEY_N))     move_cursor_down(1);
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

void Editor::set_cells_color(size_t start, size_t len, Color fg, optional<Color> bg) {
    size_t length = std::min(start + len, buffer.size());
    for (size_t i = start; i < length; i++) {
        buffer[i].bg = bg;
        buffer[i].fg = fg;
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
        if (cursor.col > line_size[cursor.row]-1) {
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
    if (buffer.size() - 1 - cursor.idx <= amount) cursor.idx = buffer.size();
    else cursor.idx += amount;
    while (amount > 0) {
        if (line_size[cursor.row] - 1 - cursor.col >=  amount) { // skip \n
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
