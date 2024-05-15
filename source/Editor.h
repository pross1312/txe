#pragma once
#include "Config.h"

#include <raylib.h>
#include <raymath.h>
#include <optional>
#include <string>
#include <vector>

struct Cell {
    char c;
    Color fg;
    std::optional<Color> bg;
};
struct Cursor { size_t row = 0, col = 0; size_t idx = 0; };
enum Mode     { Text, File };

constexpr size_t INIT_BUFFER_SIZE = 1024;
// inline std::vector<Cell> _buffer {};
// inline std::vector<size_t> _line_size(1ull, 0);
inline std::vector<int> _chars_width(128ull, 0);
extern Config _cfg;

class Editor {
public:
    Mode type;
    Cursor cursor;

    const Config &cfg = _cfg;
    std::vector<Cell> buffer;
    std::vector<size_t> line_size;

    std::vector<int> &chars_width = _chars_width;

    Editor(Mode type, size_t init_buf_size = INIT_BUFFER_SIZE): type(type), cursor(), buffer(), line_size()  {
        buffer.reserve(init_buf_size);
        line_size.reserve(init_buf_size/50);
        line_size.resize(1);
        line_size[0] = 0;

        if (_chars_width[(int)'a'] == 0) update_chars_width();
    }

    virtual ~Editor() = default;

    size_t get_idx_prev_word();
    size_t get_idx_next_word();

    virtual int handle_events();
    virtual void on_resize() {}

    void add_new_line(size_t size);
    void append_at_cursor(std::string_view str, Color fg = DEFAULT_FG, std::optional<Color> bg = std::nullopt);
    void append_at_cursor(const char *str, size_t len, Color fg = DEFAULT_FG, std::optional<Color> bg = std::nullopt);
    virtual void append_at_cursor(char c, Color fg = DEFAULT_FG, std::optional<Color> bg = std::nullopt);
    virtual void set_cells_color(size_t start, size_t len, std::optional<Color> fg = std::nullopt, std::optional<Color> bg = std::nullopt);
    virtual void pop_at_cursor(size_t amount = 1);

    virtual void move_cursor_up(size_t amount);
    virtual void move_cursor_down(size_t amount);
    virtual void move_cursor_left(size_t amount);
    virtual void move_cursor_right(size_t amount);
    virtual void move_cursor_to(size_t row, size_t col);

    virtual void render() = 0;

    virtual void put_cell(Cell cell, Vector2 position, size_t times = 1); // put cell and advance position
    virtual void put_cursor(Vector2 position);

    virtual Vector2 get_cursor_pos();

    inline bool is_rect_in_view(float x, float y, float w, float h) {
        return !(x + w <= 0.0f || y + h <= 0.0f || x >= 0.0f + GetScreenWidth() || y >= 0.0f + GetScreenHeight());
    }
    inline int get_w(char c) {
        if (c == '\t') return chars_width[(int)' '] * cfg.tab_size;
        return chars_width[(int)c];
    }

    void update_chars_width();
};
