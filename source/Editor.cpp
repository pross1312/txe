#include "Editor.hpp"
#include <numeric>
#include <string.h>
#include <fstream>

Editor::Editor(): cursor(), buffer(), line_size(1ull, 0ull), mode(Mode::Text), current_dir(fs::current_path()), file_name(nullopt) {}
Editor::Editor(const char *file): Editor() {
    if (!load(file)) {
        abort();
    }
}

void Editor::switch_mode(Mode m) {
    mode = m;
    buffer.clear();
    line_size.clear();
    line_size.push_back(0);
    cursor.idx = cursor.col = cursor.row = 0;
    switch (mode) {
    case Mode::Text:
        if (file_name.has_value()) load((current_dir / file_name.value()).c_str());
        break;
    case Mode::File:
        put_file_explorer();
        break;
    }
}

void Editor::add_new_line(size_t size) {
    cursor.col = 0;
    cursor.row++;
    line_size.insert(line_size.begin() + cursor.row, size);
}

void Editor::append_at_cursor(char c, Color fg, optional<Color> bg) {
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

void Editor::set_cells_color(size_t start, size_t len, Color fg, optional<Color> bg) {
    size_t length = std::min(start + len, buffer.size());
    for (size_t i = start; i < length; i++) {
        buffer[i].bg = bg;
        buffer[i].fg = fg;
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

void Editor::put_file_explorer() {
    const char *dir = current_dir.c_str();
    size_t len = strlen(dir);
    for (size_t i = 0; i < len; i++) {
        append_at_cursor(dir[i]);
    }
    append_at_cursor('/');
    if (file_name.has_value()) {
        for (char c : file_name.value()) append_at_cursor(c);
    }
}

bool Editor::load(const char *file) {
    current_dir = fs::absolute(fs::path(file)).parent_path();
    file_name = file;

    std::ifstream fin(file);
    if (!fin.is_open()) {
        TraceLog(LOG_ERROR, TextFormat("Can't open file %s", file));
        return false;
    }

    char c;
    while (!fin.eof() && (c = fin.get())) {
        if (isprint(c) || isspace(c)) {
            if (c == '\t') {
                append_at_cursor(' ');
                append_at_cursor(' ');
                append_at_cursor(' ');
                append_at_cursor(' ');
            } else {
                append_at_cursor(c);
            }
        }
    }

    fin.close();
    return true;
}

bool Editor::save() {
    if (file_name.has_value()) {
        fs::path file_path = current_dir / file_name.value();
        std::ofstream fout(file_path);
        if (!fout.is_open()) {
            TraceLog(LOG_ERROR, TextFormat("Can't open file %s", file_path.c_str()));
            return false;
        }

        for (size_t i = 0; i < buffer.size(); i++) {
            fout << buffer[i].c;
        }

        fout.close();
        return true;
    }
    return false;
}
