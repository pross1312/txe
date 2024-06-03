#include "Helper.h"
#include "FileExplorer.h"

#include <cstring>
#include <cerrno>
#include <cstdio>
#include <cassert>
#include <numeric>

using namespace std;

extern Config _cfg;

FileExplorer::FileExplorer(): FileExplorer(fs::current_path()) {}

FileExplorer::FileExplorer(const fs::path& dir): Editor(Mode::File), file_name(true) {
    assert(fs::is_directory(dir) && "Must create FileExplorer with a directory");
    file_name.on_text_changed = [&](StringView text) {
        (void)text;
        list_entries();
    };
    file_name.event_handler = [&](TextEdit& self) {
        if (is_key_hold(KEY_BACKSPACE)) {
            if (self.size() > 0) {
                self.pop_at_cursor();
                list_entries();
            } else if (current_dir.has_parent_path()) {
                change_dir(current_dir.parent_path());
            }
            return true;
        }
        else if (IsKeyPressed(KEY_TAB) && entries.size() > 0) {
            fs::path path(entries[current_index]);
            if (fs::is_directory(path) && (entries.size() == 1 || fs::equivalent(path, current_dir / file_name.buffer))) {
                change_dir(path);
            } else {
                self.move_cursor_to(0, self.size());
                self.pop_at_cursor(self.size());
                self.append_at_cursor(StringView(path.filename().string()));
                list_entries();
            }
            return true;
        }
        if (is_key_hold(KEY_DOWN)    || is_ctrl_and_key_hold(KEY_N)) {
            current_index = (current_index + 1)%entries.size();
            return true;
        }
        if (is_key_hold(KEY_UP)  || is_ctrl_and_key_hold(KEY_P)) {
            if (current_index == 0) current_index = entries.size()-1;
            else current_index--;
            return true;
        }
        return false;
    };

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
    if (current_index >= entries.size()) current_index = 0;
}

int FileExplorer::handle_events() {
    if (IsKeyPressed(KEY_ENTER)) {
        if (entries.size() == 0) {
            if (file_name.size() > 0) return 1;
        } else {
            fs::path path = entries[current_index];
            if (fs::is_directory(path)) change_dir(path);
            else {
                file_name.clear();
                file_name.append_at_cursor(path.c_str());
                return 1;
            }
        }
    } else {
        file_name.handle_events();
    }
    return 0;
}

void FileExplorer::change_dir(const fs::path &path) {
    SetWindowTitle(TextFormat("%s - Txe", path.c_str()));

    if (!path.is_absolute()) current_dir = fs::absolute(path);
    else current_dir = path;
    // fd --base-directory /home/dvtuong --exact-depth 1 -a -c never pro
    file_name.clear();
    list_entries();
}

void FileExplorer::render() {
    const float line_thickness = 2.0f;
    const float path_box_height = _cfg.line_height*2;
    DrawLineEx(Vector2{0.0f, path_box_height}, Vector2{(float)GetScreenWidth(), path_box_height}, line_thickness, WHITE);
    Vector2 render_cursor {.x = 0.0f, .y = (path_box_height - _cfg.line_height)*0.5f};

    const string& dir = current_dir.string();
    for (char ch : dir) {
        put_cell(ch, Attr{.fg = _cfg.dir_color, .bg = nullopt}, render_cursor);
        render_cursor.x += _cfg.char_w(ch);
    }
    if (dir[dir.size()-1] != '/') {
        put_cell('/', Attr{.fg = _cfg.dir_color, .bg = nullopt}, render_cursor);
        render_cursor.x += _cfg.char_w('/');
    }
    put_cursor(Vector2{.x = render_cursor.x + file_name.get_cursor_pos().x, .y = render_cursor.y});
    for (size_t i = 0; i < file_name.size(); i++) {
        Attr attr{.fg = _cfg.file_color, .bg = nullopt};
        if (i == file_name.cursor.idx) attr.fg = _cfg.char_at_cursor_color;
        put_cell(file_name[i], attr, render_cursor);
        render_cursor.x += _cfg.char_w(file_name[i]);
    }
    render_cursor.y = path_box_height + line_thickness + render_cursor.y;
    render_cursor.x = 0.0f;
    for (size_t i = 0; i < entries.size(); i++) {
        const string& line = entries[i];
        Attr attr{.fg = _cfg.file_color, .bg = nullopt};
        if (fs::is_directory(line)) {
            attr.fg = _cfg.dir_color;
        }
        if (i == current_index) {
            attr.bg = _cfg.on_cursor_bg_color;
        }
        put_str(line, attr, render_cursor);
        render_cursor.y += _cfg.line_height;
    }
}
