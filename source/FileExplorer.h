#pragma once
#include "Editor.h"
#include "TextEdit.h"

#include <filesystem>
#include <list>

namespace fs = std::filesystem;

struct FileExplorer: public Editor {
    size_t current_index = 0;
    fs::path current_dir;
    TextEdit file_name;
    std::vector<std::string> entries;

    FileExplorer();
    FileExplorer(const fs::path& current_file);

    inline fs::path get_file() { return current_dir / file_name.buffer; }

    void list_entries();

    void change_dir(const fs::path &path);

    int handle_events() override;

    void render() override;
};
