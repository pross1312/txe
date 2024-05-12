#pragma once

#include <filesystem>
#include "Editor.h"

namespace fs = std::filesystem;

struct FileExplorer: public Editor {
    fs::path current_dir;
    std::optional<std::string> file_name;

    FileExplorer();
    FileExplorer(fs::path current_file);

    void pop_at_cursor() override;

    void move_cursor_up(size_t amount) override;
    void move_cursor_down(size_t amount) override;
    void move_cursor_left(size_t amount) override;
    void move_cursor_right(size_t amount) override;
    void move_cursor_to(size_t row, size_t col) override;
};
