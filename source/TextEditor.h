#pragma once
#include "Editor.h"

#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

struct TextEditor: public Editor {
    const Vector2 PADDING_BOTTOM_RIGHT { .x = 50.0f, .y = 10.0f };
    const Vector2 PADDING_TOP_LEFT     { .x = 0.0f, .y = 0.0f };

    std::optional<fs::path> current_file;

    Vector2 origin;
    Rectangle text_view;
    std::string msg;

    TextEditor();
    TextEditor(const char *file);

    bool load(const char *file);
    bool save();

    size_t get_idx_prev_word();
    size_t get_idx_next_word();

    int handle_events() override;
    void on_resize() override;

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
