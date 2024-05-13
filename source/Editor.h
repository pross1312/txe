#pragma once

#include <raylib.h>
#include <optional>
#include <string>
#include <vector>

struct Cell {
    char c;
    Color fg;
    std::optional<Color> bg;
};
struct Cursor { size_t row = 0, col = 0; size_t idx = 0; };
enum Mode {
    Text, File
};

constexpr Color DEFAULT_BG = BLACK;
constexpr Color DEFAULT_FG = WHITE;

class Editor {
public:
    Mode type;
    Cursor cursor;
    std::vector<Cell> buffer;
    std::vector<size_t> line_size;

    Editor(Mode type): type(type), cursor(), buffer(), line_size(1ull, 0ull) {}

    virtual void add_new_line(size_t size);

    virtual void handle_events();

    virtual void append_at_cursor(char c, Color fg = DEFAULT_FG, std::optional<Color> bg = std::nullopt);
    virtual void append_at_cursor(std::string_view str, Color fg = DEFAULT_FG, std::optional<Color> bg = std::nullopt);
    virtual void append_at_cursor(const char *str, size_t len, Color fg = DEFAULT_FG, std::optional<Color> bg = std::nullopt);
    virtual void set_cells_color(size_t start, size_t len, Color fg = DEFAULT_FG, std::optional<Color> bg = std::nullopt);
    virtual void pop_at_cursor();

    virtual void move_cursor_up(size_t amount);
    virtual void move_cursor_down(size_t amount);
    virtual void move_cursor_left(size_t amount);
    virtual void move_cursor_right(size_t amount);
    virtual void move_cursor_to(size_t row, size_t col);
};
