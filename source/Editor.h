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


inline std::vector<Cell> _buffer {};
inline std::vector<size_t> _line_size(1ull, 0);
extern Config _cfg;

class Editor {
public:
    Mode type;
    Cursor cursor;

    const Vector2 PADDING_BOTTOM_RIGHT { .x = 50.0f, .y = 10.0f };
    const Vector2 PADDING_TOP_LEFT     { .x = 10.0f, .y = 5.0f };
    Camera2D camera;

    const Config &cfg = _cfg;
    std::vector<Cell> &buffer = _buffer;
    std::vector<size_t> &line_size = _line_size;

    Editor(Mode type): type(type), cursor()  {
        buffer.clear();
        line_size.resize(2);
        line_size[1] = 0;

        camera.target = Vector2Zero();
        camera.offset = Vector2Zero();
        camera.rotation = 0.0f;
        camera.zoom = 1.0f;
    }

    virtual ~Editor() = default;

    virtual void add_new_line(size_t size);

    virtual int handle_events();

    virtual void append_at_cursor(char c, Color fg = DEFAULT_FG, std::optional<Color> bg = std::nullopt);
    virtual void append_at_cursor(std::string_view str, Color fg = DEFAULT_FG, std::optional<Color> bg = std::nullopt);
    virtual void append_at_cursor(const char *str, size_t len, Color fg = DEFAULT_FG, std::optional<Color> bg = std::nullopt);
    virtual void set_cells_color(size_t start, size_t len, std::optional<Color> fg = std::nullopt, std::optional<Color> bg = std::nullopt);
    virtual void pop_at_cursor();

    virtual void move_cursor_up(size_t amount);
    virtual void move_cursor_down(size_t amount);
    virtual void move_cursor_left(size_t amount);
    virtual void move_cursor_right(size_t amount);
    virtual void move_cursor_to(size_t row, size_t col);

    virtual void render() = 0;

    virtual void bring_point_into_view(Vector2 point);
    virtual void put_cell(Cell cell, Vector2 &position); // put cell and advance position
    virtual void put_cursor(Vector2 position);

    virtual Vector2 get_cursor_pos();
};
