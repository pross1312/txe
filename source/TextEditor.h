#pragma once
#include "Editor.h"

#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

struct TextEditor: public Editor {
    std::optional<fs::path> current_file;

    TextEditor();
    TextEditor(const char *file);

    bool load(const char *file);
    bool save();

    int handle_events() override;
    
    void render() override;
};
