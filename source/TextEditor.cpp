#include "TextEditor.h"
#include "Helper.h"

#include <numeric>
#include <fstream>
using namespace std;

constexpr float SEPERATE_PADDING = 10.0f;

TextEditor::TextEditor(): Editor(Mode::Text) {
    current_file = nullopt;
    origin.x = cfg.line_number_width + 2*SEPERATE_PADDING;
    origin.y = 0.0f;
    text_view.x = 0.0f;
    text_view.y = 0.0f;
    text_view.width = GetScreenWidth() - origin.x;
    text_view.height = GetScreenHeight();
}
TextEditor::TextEditor(const char *file): TextEditor() {
    current_file = fs::absolute(fs::path(file));
    if (!load(file)) {
        abort();
    }
}

int TextEditor::handle_events() {
    Editor::handle_events();
    if (is_ctrl_key(KEY_S)) save();
    if (IsKeyDown(KEY_LEFT_ALT) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_H)) {
        cursor.idx = 0;
        cursor.row = 0;
        cursor.col = 0;
    }
    if (IsKeyDown(KEY_LEFT_ALT) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_L)) {
        cursor.idx = buffer.size();
        cursor.row = line_size.size()-1;
        cursor.col = line_size[cursor.row];
    }
    if (is_ctrl_key(KEY_P)) return 1;
    return 0;
}

bool TextEditor::load(const char *file) {
    current_file = fs::absolute(fs::path(file));

    std::ifstream fin(file);
    if (!fin.is_open()) {
        TraceLog(LOG_ERROR, TextFormat("Can't open file %s", file));
        return false;
    }

    char c;
    while (!fin.eof() && (c = fin.get())) {
        if (isprint(c) || isspace(c)) {
            if (c != '\r') {
                append_at_cursor(c);
            }
        }
    }

    fin.close();
    return true;
}

bool TextEditor::save() {
    if (current_file.has_value()) {
        const fs::path &file_path = current_file.value();
        std::ofstream fout(file_path);
        if (!fout.is_open()) {
            TraceLog(LOG_ERROR, TextFormat("Can't open file %s", file_path.c_str()));
            return false;
        }

        for (size_t i = 0; i < buffer.size(); i++) {
            fout << buffer[i].c;
        }

        fout.close();
        return true;
    }
    return false;
}

void TextEditor::move_text_view_to_point(Vector2 point) {
    if (point.x > text_view.x + text_view.width - PADDING_BOTTOM_RIGHT.x) text_view.x += point.x - (text_view.x + text_view.width) + PADDING_BOTTOM_RIGHT.x;
    else if (point.x < text_view.x) text_view.x -= text_view.x - point.x + PADDING_TOP_LEFT.x;

    if (point.y + cfg.line_height > text_view.y + text_view.height - PADDING_BOTTOM_RIGHT.y) text_view.y += point.y + cfg.line_height - (text_view.y + text_view.height) + PADDING_BOTTOM_RIGHT.y;
    else if (point.y < text_view.y) text_view.y -= text_view.y - point.y + PADDING_TOP_LEFT.y;
}

void TextEditor::render() {

    Vector2 render_cursor = get_cursor_pos();
    move_text_view_to_point(render_cursor);
    put_cursor(world_to_view(render_cursor));

    render_cursor = Vector2Zero();
    size_t i = 0, line = 0;
    while (line < line_size.size() && render_cursor.y + cfg.line_height <= text_view.y) {
        i += line_size[line++];
        render_cursor.y += cfg.line_height;
    }


    while (i < buffer.size() && render_cursor.y < text_view.y + text_view.height) {
        Cell cell = buffer[i++];
        int ch_w = get_w(cell.c);
        if (cell.c == '\n') {
            render_cursor.y += cfg.line_height;
            render_cursor.x = 0.0f;
        } else {
            if (render_cursor.x < text_view.x + text_view.width && render_cursor.x + ch_w > text_view.x) {
                put_cell(cell, world_to_view(render_cursor));
            }
            render_cursor.x += ch_w;
        }
    }
    render_line_number(line);
}

void TextEditor::render_line_number(size_t start) {
    float screen_height = GetScreenHeight();
    DrawRectangle(0.0f, 0.0f, cfg.line_number_width + SEPERATE_PADDING, screen_height, cfg.line_number_region_bg);
    Vector2 pos {.x = origin.x - 2*SEPERATE_PADDING, .y = origin.y + (float)start * cfg.line_height - text_view.y};
    for (size_t line = start; line < line_size.size() && pos.y < screen_height; line++, pos.y += cfg.line_height) {
        pos.x = origin.x - 2*SEPERATE_PADDING;
        int line_temp = line+1;
        Color fg = cursor.row == line ? cfg.current_line_number_color : cfg.line_number_color;
        while (line_temp > 0) {
            int n = line_temp%10;
            char c_n = n + '0';
            pos.x -= get_w(c_n);
            put_cell(Cell{.c = c_n, .fg = fg, .bg = nullopt}, pos);
            line_temp /= 10;
        }
    }
}
