#pragma once
#include "Editor.h"

#include <filesystem>
#include <list>

namespace fs = std::filesystem;

struct FileExplorer: public Editor {
    size_t current_index = 0;
    fs::path current_dir;
    std::string file_name;
    std::vector<std::string> entries;

    FileExplorer();
    FileExplorer(const fs::path& current_file);

    inline fs::path get_file() { return current_dir / file_name; }

    void set_current_idx(int idx);
    void list_entries();
    void put_entries();

    void change_dir(const fs::path &path);
    void open_file(const fs::path &path);

    int handle_events() override;

    void move_cursor_up(size_t amount) override;
    void move_cursor_down(size_t amount) override;
    void move_cursor_left(size_t amount) override;
    void move_cursor_right(size_t amount) override;

    void render() override;
};
