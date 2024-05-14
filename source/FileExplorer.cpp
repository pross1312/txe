#include "Helper.h"
#include "FileExplorer.h"
#include "Split.hpp"

#include <cstring>
#include <cerrno>
#include <cstdio>
#include <cassert>
#include <numeric>

using namespace std;

FileExplorer::FileExplorer(): FileExplorer(fs::current_path()) {}

FileExplorer::FileExplorer(fs::path current_file): Editor(Mode::File) {
    if (!current_file.is_absolute()) {
        current_file = fs::absolute(current_file);
    }
    file_name = "";
    fs::path dir = fs::is_directory(current_file) ? current_file : current_file.parent_path();
    change_dir(dir);
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
        if (fs::is_directory(path)) fg = cfg.dir_color;
        else fg = cfg.file_color;
        if (idx == current_index) bg = cfg.on_cursor_bg_color;
        else bg = cfg.n_on_cursor_bg_color;
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
    else if (IsKeyPressed(KEY_ENTER) && entries.size() > 0) {
        fs::path path(entries[current_index]);
        if (fs::is_directory(path)) {
            change_dir(path);
        } else {
            file_name = path.filename().string();
            return 1;// open_file(path);
        }
    }
    else if (is_key_hold(KEY_LEFT)  || is_ctrl_and_key_hold(KEY_B))     move_cursor_left(1);
    else if (is_key_hold(KEY_RIGHT) || is_ctrl_and_key_hold(KEY_F))     move_cursor_right(1);
    else if (is_key_hold(KEY_UP)    || is_ctrl_and_key_hold(KEY_P))     move_cursor_up(1);
    else if (is_key_hold(KEY_DOWN)  || is_ctrl_and_key_hold(KEY_N))     move_cursor_down(1);
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
    append_at_cursor(str, cfg.dir_color);
    if (str[str.size()-1] != '/') append_at_cursor('/', cfg.dir_color);
    append_at_cursor(file_name, cfg.file_color);
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

        set_cells_color(start, line_size[idx+1], nullopt, cfg.on_cursor_bg_color);
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

void FileExplorer::render() {
    const float line_thickness = 2.0f;
    const float path_box_height = cfg.line_height*2;
    DrawLineEx(Vector2{0.0f, path_box_height}, Vector2{(float)GetScreenWidth(), path_box_height}, line_thickness, WHITE);
    Vector2 render_cursor {.x = 0.0f, .y = (path_box_height - cfg.line_height)*0.5f};
    size_t i = 0;
    for (; i+1 < line_size[0]; i++) { // avoid overflow
        put_cell(buffer[i], render_cursor);
        render_cursor.x += get_w(buffer[i].c);
    }
    put_cursor(render_cursor);
    // bring_point_into_view(render_cursor);
    render_cursor.y = path_box_height + line_thickness;
    render_cursor.x = 0.0f;
    i++;
    while (i < buffer.size()) {
        Cell cell = buffer[i++];
        if (cell.c == '\n') {
            render_cursor.y += cfg.line_height;
            render_cursor.x = 0.0f;
        } else {
            put_cell(cell, render_cursor);
            render_cursor.x += get_w(cell.c);
        }
    }
}
