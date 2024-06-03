#pragma once
#include "Config.h"

#include <raylib.h>
#include <raymath.h>
#include <optional>
#include <string>
#include <vector>

enum Mode     { Text, File };
struct Attr {
    Color fg = DEFAULT_FG;
    std::optional<Color> bg = std::nullopt;
};

struct Editor {
    Mode type;

    Editor(Mode type): type(type) {}

    virtual void on_resize() {}
    virtual int handle_events() = 0;
    virtual ~Editor() = default;

    virtual void render() = 0;

    virtual void put_cell(char ch, Attr attr, Vector2 position, size_t times = 1); // put cell and advance position
    virtual void put_cursor(Vector2 position);
};
