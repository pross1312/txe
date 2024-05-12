#ifndef Editor_HPP
#define Editor_HPP

#include <string>
#include <numeric>
#include <assert.h>
#include <string.h>
#include <vector>
#include <list>
#include <optional>
#include <raylib.h>
#include "Split.hpp"
using namespace std;

constexpr Color DEFAULT_BG = BLACK;
constexpr Color DEFAULT_FG = WHITE;

struct Cell {
    char c;
    Color fg;
    optional<Color> bg;
};

struct Editor {
    struct Cursor { size_t row = 0, col = 0; size_t idx = 0; };
    Cursor cursor;
    vector<Cell> buffer;
    vector<size_t> line_size;

    Editor(): cursor(), buffer(), line_size(1ull, 0ull) {}

    inline void add_new_line(size_t size) {
        cursor.col = 0;
        cursor.row++;
        line_size.insert(line_size.begin() + cursor.row, size);
    }

    inline void append_at_cursor(char c, Color fg = DEFAULT_FG, optional<Color> bg = nullopt) {
        buffer.insert(buffer.begin() + cursor.idx, 1, Cell{c, fg, bg});
        line_size[cursor.row]++;
        cursor.idx++;
        cursor.col++;
        if (c == '\n') {
            size_t new_line_size = line_size[cursor.row] - cursor.col;
            line_size[cursor.row] -= new_line_size;
            add_new_line(new_line_size);
        }
    }

    inline void pop_at_cursor() {
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

    inline void move_cursor_up(size_t amount) {
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

    inline void move_cursor_down(size_t amount) {
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

    inline void move_cursor_left(size_t amount) {
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

    inline void set_cells_color(size_t start, size_t len, Color fg = DEFAULT_FG, optional<Color> bg = nullopt) {
        size_t length = std::min(start + len, buffer.size());
        for (size_t i = start; i < length; i++) {
            buffer[i].bg = bg;
            buffer[i].fg = fg;
        }
    }

    inline void move_cursor_right(size_t amount) {
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

    inline Split lines() {
        string_view buffer_view(buffer.data(), buffer.size());
        return split(buffer_view, '\n');
    }
    inline Split split(string_view sv, char c) { return Split(sv, c); }
};

#endif // EDITOR_HPP
