#pragma once

#include <filesystem>
#include <optional>

#include "Editor.h"
namespace fs = std::filesystem;

struct TextEditor: public Editor {
    std::optional<fs::path> current_file;

    TextEditor();
    TextEditor(const char *file);

    bool load(const char *file);
    bool save();

    void handle_events() override;
};
