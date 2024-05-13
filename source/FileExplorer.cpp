#include "Helper.h"
#include "FileExplorer.h"
#include "Split.hpp"

#include <cstring>
#include <cerrno>
#include <cstdio>
#include <cassert>
#include <numeric>
#include <algorithm>

using namespace std;

FileExplorer::FileExplorer(): FileExplorer(fs::current_path()) { }

FileExplorer::FileExplorer(fs::path current_file): Editor(Mode::File) {
    if (!current_file.is_absolute()) {
        current_file = fs::absolute(current_file);
    }
    file_name = current_file.filename();
    change_dir(current_file.parent_path());
}

void FileExplorer::list_entries() {
    buffer.resize(line_size[0]);
    line_size.resize(2);
    line_size[1] = 0;
    cursor.row = 1;
    cursor.col = 0;
    cursor.idx = buffer.size();
    current_index = 0;
    entries.clear();

    const char *command = TextFormat("fd --base-directory '%s' --exact-depth 1 -c never \'%.*s\'", current_dir.c_str(), file_name.size(), file_name.data());
    TraceLog(LOG_INFO, command);
    FILE *pipe = popen(command, "r");
    if (pipe == nullptr) {
        TraceLog(LOG_ERROR, strerrorname_np(errno));
    } else {
        static char buffer[128] {};
        while (!feof(pipe)) {
            if (fgets(buffer, 128, pipe) != nullptr) {
                size_t len = strlen(buffer);
                string_view sv(buffer, len - 1 - (buffer[len-2] == '/' ? 1 : 0));
                entries.push_back((current_dir / sv).string());
            }
        }
        pclose(pipe);
    }

    size_t idx = 0;
    for (const string &path : entries) {
        Color fg, bg;
        if (fs::is_directory(path)) fg = SKYBLUE;
        else fg = DEFAULT_FG;
        if (idx == current_index) bg = GetColor(0x313131ff);
        else bg = DEFAULT_BG;
        Editor::append_at_cursor(path, fg, bg);
        if (fs::is_directory(path)) append_at_cursor('/', fg, bg);
        append_at_cursor('\n');
        idx++;
    }
    move_cursor_to(0, line_size[0]-1);
}

int FileExplorer::handle_events() {
    int c;
    while ((c = GetCharPressed())) {
        append_at_cursor((char)c);
        file_name.push_back((char)c);
        list_entries();
    }
    if (is_key_hold(KEY_BACKSPACE)) {
        if (file_name.size() > 0) {
            pop_at_cursor();
            file_name.resize(file_name.size()-1);
            list_entries();
        } else if (current_dir.has_parent_path()) {
            change_dir(current_dir.parent_path());
        }
    }
    if (IsKeyPressed(KEY_ENTER) && entries.size() > 0) {
        fs::path path(entries[current_index]);
        if (fs::is_directory(path)) {
            change_dir(path);
        } else {
            file_name = path.filename().string();
            return 1;// open_file(path);
        }
    }
    if (is_key_hold(KEY_LEFT)  || is_alt_and_key_hold(KEY_H))     move_cursor_left(1);
    if (is_key_hold(KEY_RIGHT) || is_alt_and_key_hold(KEY_L))     move_cursor_right(1);
    if (is_key_hold(KEY_UP)    || is_alt_and_key_hold(KEY_K))     move_cursor_up(1);
    if (is_key_hold(KEY_DOWN)  || is_alt_and_key_hold(KEY_J))     move_cursor_down(1);
    return 0;
}

void FileExplorer::change_dir(const fs::path &path) {
    if (!path.is_absolute()) current_dir = fs::absolute(path);
    else current_dir = path;
    // fd --base-directory /home/dvtuong --exact-depth 1 -a -c never pro
    buffer.clear();
    file_name.clear();
    line_size.resize(1);
    line_size[0] = 0;
    cursor.idx = cursor.col = cursor.row = 0;

    const string &str = current_dir.string();
    append_at_cursor(str);
    if (str[str.size()-1] != '/') append_at_cursor('/');
    append_at_cursor(file_name);
    append_at_cursor('\n');

    list_entries();
}

void FileExplorer::open_file(const fs::path &path) {
    (void)path;
}

void FileExplorer::set_current_idx(size_t idx) {
    if (entries.size() == 0) return;
    assert(idx < entries.size());
    if (idx != current_index) {
        size_t start = std::accumulate(line_size.begin(), line_size.begin()+current_index+1, 0);
        set_cells_color(start, line_size[current_index+1], nullopt, nullopt);
        start = std::accumulate(line_size.begin(), line_size.begin()+idx+1, 0);

        set_cells_color(start, line_size[idx+1], nullopt, GetColor(0x313131ff));
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
