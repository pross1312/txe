#ifndef Editor_HPP
#define Editor_HPP

#include <string>
#include <filesystem>
#include <assert.h>
#include <vector>
#include <optional>
#include <raylib.h>
namespace fs = std::filesystem;
using namespace std;

constexpr Color DEFAULT_BG = BLACK;
constexpr Color DEFAULT_FG = WHITE;

struct Cell {
    char c;
    Color fg;
    optional<Color> bg;
};

enum Mode {
    Text, File
};

struct Editor {
    struct Cursor { size_t row = 0, col = 0; size_t idx = 0; };
    Cursor cursor;
    vector<Cell> buffer;
    vector<size_t> line_size;
    Mode mode;

    fs::path current_dir;
    optional<string> file_name;

    Editor();
    Editor(const char *file);

    void switch_mode(Mode m);

    bool load(const char *file);
    bool save();

    void put_file_explorer();

    void add_new_line(size_t size);
    void append_at_cursor(char c, Color fg = DEFAULT_FG, optional<Color> bg = nullopt);
    void set_cells_color(size_t start, size_t len, Color fg = DEFAULT_FG, optional<Color> bg = nullopt);
    void pop_at_cursor();

    void move_cursor_up(size_t amount);
    void move_cursor_down(size_t amount);
    void move_cursor_left(size_t amount);
    void move_cursor_right(size_t amount);
    void move_cursor_to(size_t row, size_t col);
};

#endif // EDITOR_HPP
