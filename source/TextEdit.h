#pragma once
#include "Config.h"

#include <optional>
#include <string>
#include <functional>

struct TextEdit {
    std::string buffer;
    std::vector<size_t> line_size;
    struct Cursor { size_t row = 0, col = 0; size_t idx = 0; } cursor;

    std::function<bool(TextEdit&)> event_handler;
    std::function<bool(char, TextEdit&)> on_char_pressed;

    bool oneline = false;

    TextEdit(bool oneline = false): line_size(1ull, 0), oneline(oneline) {}

    inline size_t size() const { return buffer.size(); }
    inline const char* data() const { return buffer.data(); }
    char& operator[](size_t i) { return buffer[i]; }

    void handle_events();

    size_t get_idx_prev_word();
    size_t get_idx_next_word();
    StringView get_text(size_t start, size_t end);


    void add_new_line(size_t size);
    void append_at_cursor(StringView str);
    void append_at_cursor(char c);
    void pop_at_cursor(size_t amount = 1);

    void move_cursor_up(size_t amount);
    void move_cursor_down(size_t amount);
    void move_cursor_left(size_t amount);
    void move_cursor_right(size_t amount);
    void move_cursor_to(size_t row, size_t col);
    void move_cursor_to_idx(size_t idx);

    Vector2 get_cursor_pos();
};
