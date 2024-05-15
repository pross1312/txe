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
    entries.clear();

    const char *command = TextFormat("fd --base-directory '%s' --exact-depth 1 -c never \'%.*s\'", current_dir.c_str(), file_name.size(), file_name.data());
    // TraceLog(LOG_INFO, command);
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
    if (current_index > entries.size()) current_index = 0;
    put_entries();
}

void FileExplorer::put_entries() {
    buffer.resize(line_size[0]);
    line_size.resize(2);
    line_size[1] = 0;
    Cursor old_cursor = cursor;
    cursor.row = 1;
    cursor.col = 0;
    cursor.idx = buffer.size();

    for (size_t i = 0; i < entries.size(); i++) {
        const string& path = entries[(i+current_index)%entries.size()];
        Color fg, bg;
        if (fs::is_directory(path)) fg = cfg.dir_color;
        else fg = cfg.file_color;
        // if (idx == current_index) bg = cfg.on_cursor_bg_color;
        bg = cfg.n_on_cursor_bg_color;
        Editor::append_at_cursor(path, fg, bg);
        if (fs::is_directory(path)) append_at_cursor('/', fg, bg);
        append_at_cursor('\n');
    }

    if (file_name.size() != 0) cursor = old_cursor;
    else move_cursor_to(0, line_size[0]-1);
}

int FileExplorer::handle_events() {
    int c;
    while ((c = GetCharPressed())) {
        append_at_cursor((char)c);
        file_name.push_back((char)c);
        list_entries();
    }
    if (is_alt_key(KEY_BACKSPACE) && file_name.size() > 0) {
        size_t dir_len = current_dir.string().size() + 1;
        size_t idx = std::max(dir_len, get_idx_prev_word());
        file_name = file_name.substr(cursor.idx - idx);
        pop_at_cursor(cursor.idx - idx);
        list_entries();
    }
    else if (is_key_hold(KEY_BACKSPACE)) {
        if (file_name.size() > 0) {
            pop_at_cursor();
            file_name.resize(file_name.size()-1);
            list_entries();
        } else if (current_dir.has_parent_path()) {
            change_dir(current_dir.parent_path());
        }
    }
    else if (IsKeyPressed(KEY_ENTER)) {
        // if (entries.size() > 0 && current_index >= 0) {
        //     fs::path path(entries[current_index]);
        //     if (fs::is_directory(path)) {
        //         change_dir(path);
        //     } else {
        //         file_name = path.filename().string();
        //     }
        // } else {
        fs::path file = get_file();
        if (fs::is_directory(file)) change_dir(file);
        else return 1;
        // }
    }
    else if (IsKeyPressed(KEY_TAB) && entries.size() > 0) {
        fs::path file(entries[current_index]);
        if (entries.size() == 1 && fs::is_directory(file)) {
            change_dir(file);
        } else {
            move_cursor_to(0, line_size[0]-1);
            pop_at_cursor(file_name.size());
            file_name = file.filename().string();
            append_at_cursor(file_name);
            list_entries();
        }
    }
    else if (is_alt_and_key_hold(KEY_F) && cursor.idx < buffer.size()) {
        size_t idx = std::min(line_size[0]-1, get_idx_next_word());
        move_cursor_right(idx - cursor.idx);
    }
    else if (is_alt_and_key_hold(KEY_B) && cursor.idx > 0) {
        size_t dir_len = current_dir.string().size() + 1;
        size_t idx = std::max(dir_len, get_idx_prev_word());
        move_cursor_left(cursor.idx - idx);
    }
    else if (is_key_hold(KEY_LEFT)  || is_ctrl_and_key_hold(KEY_B))     move_cursor_left(1);
    else if (is_key_hold(KEY_RIGHT) || is_ctrl_and_key_hold(KEY_F))     move_cursor_right(1);
    else if (is_key_hold(KEY_UP)    || is_ctrl_and_key_hold(KEY_P)) {
        current_index = (current_index + 1)%entries.size();
        put_entries();
    }
    else if (is_key_hold(KEY_DOWN)  || is_ctrl_and_key_hold(KEY_N)) {
        if (current_index == 0) current_index = entries.size()-1;
        else current_index--;
        put_entries();
    }
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

void FileExplorer::set_current_idx(int idx) {
    (void)idx;
    // if (entries.size() == 0) return;
    // if (idx != current_index) {
    //     if (current_index != -1) {
    //         size_t start = std::accumulate(line_size.begin(), line_size.begin()+current_index+1, 0);
    //         set_cells_color(start, line_size[current_index+1], nullopt, nullopt);
    //     }
    //     if (idx != -1) {
    //         size_t start = std::accumulate(line_size.begin(), line_size.begin()+idx+1, 0);
    //         set_cells_color(start, line_size[idx+1], nullopt, cfg.on_cursor_bg_color);
    //     }
    //     current_index = idx;
    // }
}

void FileExplorer::move_cursor_up(size_t amount) {
    (void)amount;
    // if (current_index >= 0) {
    //     amount = std::min((size_t)current_index + 1, amount);
    //     if (amount > 0) set_current_idx(current_index - (int)amount);
    // }
}

void FileExplorer::move_cursor_down(size_t amount) {
    (void)amount;
    // if (amount != 0) set_current_idx((current_index + amount) % entries.size());
}

void FileExplorer::move_cursor_left(size_t amount) {
    if (cursor.idx >= (line_size[0] - 1 - file_name.size()) + amount) {
        Editor::move_cursor_left(amount);
    }
}

void FileExplorer::move_cursor_right(size_t amount) {
    if (cursor.idx + amount <= line_size[0]-1) {
        Editor::move_cursor_right(amount);
    }
}

void FileExplorer::render() {
    const float line_thickness = 2.0f;
    const float path_box_height = cfg.line_height*2;
    DrawLineEx(Vector2{0.0f, path_box_height}, Vector2{(float)GetScreenWidth(), path_box_height}, line_thickness, WHITE);
    Vector2 render_cursor {.x = 0.0f, .y = (path_box_height - cfg.line_height)*0.5f};

    put_cursor(Vector2{.x = get_cursor_pos().x, .y = render_cursor.y});
    size_t i = 0;
    for (; i+1 < line_size[0]; i++) { // avoid overflow
        put_cell(buffer[i], render_cursor);
        render_cursor.x += get_w(buffer[i].c);
    }
    // bring_point_into_view(render_cursor);
    render_cursor.y = path_box_height + line_thickness + render_cursor.y;
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
