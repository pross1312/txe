#include "Helper.h"
#include "FileExplorer.h"

#include <cstring>
#include <cassert>
#include <numeric>
#include <algorithm>

using namespace std;

FileExplorer::FileExplorer(): FileExplorer(fs::current_path()) { }

FileExplorer::FileExplorer(fs::path current_file): Editor(Mode::File) {
    if (!current_file.is_absolute()) {
        current_file = fs::absolute(current_file);
    }
    file_name = current_dir.filename();
    change_dir(current_file.parent_path());
}

void FileExplorer::sort_and_insert_entries() {
    buffer.resize(line_size[0]);
    line_size.resize(2);
    line_size[1] = 0;
    cursor.row = 1;
    cursor.col = 0;
    size_t cursor_col = cursor.idx = buffer.size();
    string cur_path = (current_dir / file_name).string();
    sort(entries.begin(), entries.end(), [&cur_path](const string& a, const string& b) {
        return DamerauLevenshteinDistance(a, cur_path, 100) < DamerauLevenshteinDistance(b, cur_path, 100);
    });
    size_t idx = 0;
    for (const string &path : entries) {
        if (idx == current_index) Editor::append_at_cursor(path, WHITE, GetColor(0x313131ff));
        else Editor::append_at_cursor(path);
        if (fs::is_directory(path)) append_at_cursor('/');
        append_at_cursor('\n');
        idx++;
    }
    move_cursor_to(0, cursor_col);
}

void FileExplorer::handle_events() {
    int c;
    while ((c = GetCharPressed())) {
        append_at_cursor((char)c);
        file_name.push_back((char)c);
        sort_and_insert_entries();
    }
    if (is_key_hold(KEY_BACKSPACE)) {
        pop_at_cursor();
        file_name.resize(file_name.size()-1);
        sort_and_insert_entries();
    }
    if (IsKeyPressed(KEY_ENTER) && entries.size() > 0) {
        fs::path path(entries[current_index]);
        if (fs::is_directory(path)) {
            change_dir(path);
        } else {
            open_file(path);
        }
    }
    if (is_key_hold(KEY_LEFT)  || is_ctrl_and_key_hold(KEY_B))     move_cursor_left(1);
    if (is_key_hold(KEY_RIGHT) || is_ctrl_and_key_hold(KEY_F))     move_cursor_right(1);
    if (is_key_hold(KEY_UP)    || is_ctrl_and_key_hold(KEY_P))     move_cursor_up(1);
    if (is_key_hold(KEY_DOWN)  || is_ctrl_and_key_hold(KEY_N))     move_cursor_down(1);
}

void FileExplorer::change_dir(const fs::path &path) {
    if (!path.is_absolute()) current_dir = fs::absolute(path);
    else current_dir = path;
    buffer.clear();
    line_size.resize(1);
    line_size[0] = 0;
    current_index = 0;
    entries.clear();
    cursor.idx = cursor.col = cursor.row = 0;

    const char *dir = current_dir.c_str();
    append_at_cursor(dir, strlen(dir));
    append_at_cursor('/');
    append_at_cursor(file_name);
    append_at_cursor('\n');

    for (auto const &entry : fs::directory_iterator(current_dir)) {
        entries.push_back(entry.path().string());
    }
    sort_and_insert_entries();
}

void FileExplorer::open_file(const fs::path &path) {
    (void)path;
}

void FileExplorer::set_current_idx(size_t idx) {
    if (entries.size() == 0) return;
    assert(idx < entries.size());
    if (idx != current_index) {
        size_t start = std::accumulate(line_size.begin(), line_size.begin()+current_index+1, 0);
        set_cells_color(start, line_size[current_index+1], DEFAULT_FG, nullopt);
        start = std::accumulate(line_size.begin(), line_size.begin()+idx+1, 0);
        set_cells_color(start, line_size[idx+1], DEFAULT_FG, GetColor(0x313131ff));
        current_index = idx;
    }
}

void FileExplorer::move_cursor_up(size_t amount) {
    amount = std::min(current_index, amount);
    if (amount != 0) set_current_idx(current_index - amount);
}

void FileExplorer::move_cursor_down(size_t amount) {
    amount = std::min(entries.size() - 1 - current_index, amount);
    if (amount != 0) set_current_idx(current_index + amount);
}

void FileExplorer::move_cursor_left(size_t amount) {
    (void)amount;
}

void FileExplorer::move_cursor_right(size_t amount) {
    (void)amount;
}
