#pragma once

#include <filesystem>
#include <list>
#include "Editor.h"

namespace fs = std::filesystem;

struct FileExplorer: public Editor {
    const Color DIR_COLOR = SKYBLUE;
    const Color FILE_COLOR = WHITE;
    const Color ON_CURSOR_BG_COLOR = GetColor(0x313131ff);
    const Color N_ON_CURSOR_BG_COLOR = DEFAULT_BG;
    size_t current_index = 0;
    fs::path current_dir;
    std::string file_name;
    std::vector<std::string> entries;

    FileExplorer();
    FileExplorer(fs::path current_file);

    inline fs::path get_file() { return current_dir / file_name; }

    void set_current_idx(size_t idx);
    void list_entries();

    void change_dir(const fs::path &path);
    void open_file(const fs::path &path);

    int handle_events() override;

    void move_cursor_up(size_t amount) override;
    void move_cursor_down(size_t amount) override;
    void move_cursor_left(size_t amount) override;
    void move_cursor_right(size_t amount) override;
};
