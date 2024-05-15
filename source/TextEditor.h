#pragma once
#include "Editor.h"

#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

struct TextEditor: public Editor {
    const Vector2 PADDING_BOTTOM_RIGHT { .x = 50.0f, .y = 10.0f };
    const Vector2 PADDING_TOP_LEFT     { .x = 0.0f, .y = 0.0f };

    fs::path current_file;

    Vector2 origin;
    Rectangle text_view;
    std::string msg;
    std::optional<size_t> start_selection;

    TextEditor(const fs::path& path);

    bool load(const fs::path& path);
    bool save();

    int handle_events() override;
    void on_resize() override;


    std::string get_text(size_t start, size_t end = std::string::npos);
    std::string get_selected_text();
    bool is_selected(size_t i);

    void render() override;
    void render_msg();
    void render_line_number(size_t start);

    void move_text_view_to_point(Vector2 point);

    inline Vector2 world_to_view(Vector2 point) {
        return Vector2 {
            .x = point.x + origin.x - text_view.x,
            .y = point.y + origin.y - text_view.y,
        };
    }
};
