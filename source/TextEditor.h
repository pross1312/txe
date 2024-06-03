#pragma once
#include "Editor.h"
#include "Searcher.hpp"
#include "Trie.hpp"
#include "TextEdit.h"

#include <filesystem>
#include <optional>

namespace fs = std::filesystem;

struct TextEditor: public Editor {
    const Vector2 PADDING_BOTTOM_RIGHT { .x = 50.0f, .y = 10.0f };
    const Vector2 PADDING_TOP_LEFT     { .x = 0.0f, .y = 0.0f };

    bool is_c_file = false;

    TextEdit buffer;
    std::vector<Attr> attributes;
    std::optional<fs::path> current_file;
    Trie trie;

    StringView msg;
    Searcher::Iterator search_iter;
    enum InputType { None, Search, ReadFileName, YesNo } input_type = InputType::None;
    TextEdit input;

    Vector2 origin;
    Rectangle text_view;

    std::optional<size_t> start_selection;

    TextEditor();
    TextEditor(const fs::path& file_path);

    bool load(const fs::path& path);
    bool save(const fs::path& file_path);

    int handle_events() override;
    void on_resize() override;

    bool is_keyword(StringView token);
    void highlight();

    void move_text_view_to_point(Vector2 point);
    void render() override;
    void render_msg();
    void render_line_number(size_t start);

    StringView get_selected_text();
    bool is_selected(size_t i);

    inline Vector2 world_to_view(Vector2 point) {
        return Vector2 {
            .x = point.x + origin.x - text_view.x,
                .y = point.y + origin.y - text_view.y,
        };
    }
};
