#pragma once

#include <filesystem>
#include <list>
#include "Editor.h"

namespace fs = std::filesystem;

struct FileExplorer: public Editor {
    size_t current_index = 0;
    fs::path current_dir;
    std::string file_name;
    std::vector<std::string> entries;

    FileExplorer();
    FileExplorer(fs::path current_file);

    void set_current_idx(size_t idx);
    void sort_and_insert_entries();

    void change_dir(const fs::path &path);
    void open_file(const fs::path &path);

    void handle_events() override;

    void move_cursor_up(size_t amount) override;
    void move_cursor_down(size_t amount) override;
    void move_cursor_left(size_t amount) override;
    void move_cursor_right(size_t amount) override;
};
