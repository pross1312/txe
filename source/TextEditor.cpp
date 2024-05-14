#include "TextEditor.h"
#include "Helper.h"

#include <functional>
#include <numeric>
#include <fstream>
using namespace std;

constexpr float SEPERATE_PADDING = 10.0f;

TextEditor::TextEditor(): Editor(Mode::Text) {
    SetWindowTitle("Txe");
    current_file = nullopt;
    on_resize();
}
TextEditor::TextEditor(const char *file): TextEditor() {
    if (!load(file)) {
        abort();
    }
    current_file = fs::absolute(fs::path(file));
    SetWindowTitle(TextFormat("%s - Txe", file));
}

void TextEditor::on_resize() {
    origin.x = cfg.line_number_width + 2*SEPERATE_PADDING;
    origin.y = 0.0f;

    text_view.x = 0.0f;
    text_view.y = 0.0f;
    text_view.width = GetScreenWidth() - origin.x;
    text_view.height = GetScreenHeight() - cfg.msg_box_height;
}

size_t TextEditor::get_idx_prev_word() {
    bool found = false;
    size_t i = cursor.idx;
    for (; i != 0; i--) {
        if (isspace(buffer[i-1].c) && found) break;
        if (!isspace(buffer[i-1].c)) found = true;
    }
    return i;
}

size_t TextEditor::get_idx_next_word() {
    bool found = false;
    size_t i = cursor.idx;
    for (; i < buffer.size(); i++) {
        if (isspace(buffer[i].c) && found) break;
        if (!isspace(buffer[i].c)) found = true;
    }
    return i;
}

int TextEditor::handle_events() {
    static bool is_ctrl_x = false;
    Editor::handle_events();

    if (is_ctrl_key(KEY_G)) {
        is_ctrl_x = false;
        msg = "CTRL-G";
    }
    if (is_ctrl_x) {
        if (is_ctrl_key(KEY_S)) {
            is_ctrl_x = false;
            if (current_file.has_value()) {
                if (save()) {
                    msg = "Saved";
                } else {
                    msg = TextFormat("Can't save to file %s", current_file.value().c_str());
                }
            } else {
                msg = "No file path";
            }
        } else if (is_ctrl_key(KEY_F)) {
            is_ctrl_x = false;
            return 1;
        } else if (is_ctrl_key(KEY_C)) exit(0);
    }
    else if (is_alt_and_key_hold(KEY_BACKSPACE)) {
        pop_at_cursor(cursor.idx - get_idx_prev_word());
    }
    else if (is_alt_and_key_hold(KEY_F) && cursor.idx < buffer.size()) {
        move_cursor_right(get_idx_next_word() - cursor.idx);
    }
    else if (is_alt_and_key_hold(KEY_B) && cursor.idx > 0) {
        move_cursor_left(cursor.idx - get_idx_prev_word());
    }
    else if (is_ctrl_key(KEY_X)) {
        is_ctrl_x = true;
        msg = "CTRL-X ..";
    }
    else if (is_ctrl_key(KEY_A)) {
        cursor.idx -= cursor.col;
        cursor.col = 0;
    }
    else if (is_ctrl_key(KEY_E)) {
        size_t new_cursor_col = line_size[cursor.row];
        if (cursor.row != line_size.size()-1) new_cursor_col -= 1;
        cursor.idx += new_cursor_col - cursor.col;
        cursor.col = new_cursor_col;
    }
    else if (IsKeyDown(KEY_LEFT_ALT) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_COMMA)) {
        cursor.idx = 0;
        cursor.row = 0;
        cursor.col = 0;
    }
    else if (IsKeyDown(KEY_LEFT_ALT) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_PERIOD)) {
        cursor.idx = buffer.size();
        cursor.row = line_size.size()-1;
        cursor.col = line_size[cursor.row];
    }
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

void TextEditor::render_msg() {
    Vector2 pos {.x = 0.0f, .y = (float)GetScreenHeight() - (cfg.line_height + cfg.msg_box_height)*0.5f};
    DrawRectangle(0.0f, (float)GetScreenHeight() - cfg.msg_box_height, (float)GetScreenWidth(), cfg.msg_box_height, DEFAULT_BG);
    for (char ch : msg) {
        Cell cell {
            .c = ch,
            .fg = DEFAULT_FG,
            .bg = nullopt
        };
        put_cell(cell, pos);
        pos.x += get_w(ch);
    }
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
    render_msg();
}

void TextEditor::render_line_number(size_t start) {
    DrawRectangle(0.0f, 0.0f, cfg.line_number_width + SEPERATE_PADDING, text_view.height, cfg.line_number_region_bg);
    Vector2 pos {.x = origin.x - 2*SEPERATE_PADDING, .y = origin.y + (float)start * cfg.line_height - text_view.y};
    for (size_t line = start; line < line_size.size() && pos.y < text_view.height; line++, pos.y += cfg.line_height) {
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
