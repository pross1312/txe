#include "TextEditor.h"
#include "Helper.h"

#include <functional>
#include <cstring>
#include <numeric>
#include <cassert>
#include <fstream>
using namespace std;

constexpr float SEPERATE_PADDING = 10.0f;

TextEditor::TextEditor(const fs::path& file): Editor(Mode::Text), msg(), is_searching(false) {
    assert(!fs::is_directory(file) && "Must create TextEditor with a file");
    if (!load(file)) {
        TraceLog(LOG_INFO, current_file.c_str());
        SetWindowTitle("Txe");
    } else {
        SetWindowTitle(TextFormat("%s - Txe", file.c_str()));
    }
    on_resize();
}

void TextEditor::on_resize() {
    origin.x = cfg.line_number_width + 2*SEPERATE_PADDING;
    origin.y = 0.0f;

    text_view.x = 0.0f;
    text_view.y = 0.0f;
    text_view.width = GetScreenWidth() - origin.x;
    text_view.height = GetScreenHeight() - cfg.msg_box_height;
}

int TextEditor::handle_searching_events() {
    int c;
    while ((c = GetCharPressed())) {
        pattern.push_back((char)c);
        search_iter = CellSliceSearch(CellSlice{.data = buffer.data(), .size = buffer.size()}, pattern).begin();
        move_cursor_to_idx(search_iter->data - buffer.data());
    }
    if (is_ctrl_key(KEY_G)) {
        is_searching = false;
        msg = "CTRL-G";
    }
    else if (is_key_hold(KEY_ENTER)) {
        move_cursor_to_idx(search_iter->data - buffer.data());
        is_searching = false;
    }
    else if (is_key_hold(KEY_BACKSPACE) && pattern.size() > 0) {
        pattern.resize(pattern.size() - 1);
        CellSliceSearch searcher(CellSlice{.data = buffer.data(), .size = buffer.size()}, pattern);
        search_iter = searcher.begin();
    }
    else if (is_ctrl_key(KEY_S)) {
        ++search_iter;
        CellSliceSearch searcher(CellSlice{.data = buffer.data(), .size = buffer.size()}, pattern);
        if (search_iter == searcher.end()) {
            search_iter = searcher.begin();
        }
        move_cursor_to_idx(search_iter->data - buffer.data());
    }
    else if (is_ctrl_key(KEY_N)) {
        CellSliceSearch searcher(CellSlice{.data = buffer.data(), .size = buffer.size()}, pattern);
        if (search_iter == searcher.begin()) {
            search_iter = searcher.end();
        }
        --search_iter;
        move_cursor_to_idx(search_iter->data - buffer.data());
    }
    return 0;
}

int TextEditor::handle_events() {
    if (is_searching) return handle_searching_events();

    static bool is_ctrl_x = false;

    Editor::handle_events();
    int c;
    while ((c = GetCharPressed())) append_at_cursor((char)c);
    if (is_ctrl_key(KEY_G)) {
        is_ctrl_x = false;
        start_selection = nullopt;
        msg = "CTRL-G";
    }
    if (is_ctrl_x) {
        if (is_ctrl_key(KEY_S)) {
            is_ctrl_x = false;
            if (save()) {
                msg = "Saved";
            } else {
                msg = TextFormat("Can't save to file %s", current_file.c_str());
            }
        } else if (is_ctrl_key(KEY_F)) {
            is_ctrl_x = false;
            return 1;
        } else if (is_ctrl_key(KEY_C)) exit(0);
    }
    else if (is_key_hold(KEY_ENTER)) {
        append_at_cursor('\n');
    }
    else if (IsKeyPressed(KEY_TAB)) {
        append_at_cursor('\t');
    }
    else if (is_ctrl_key(KEY_S)) {
        is_searching = true;
        msg = "Search: ";
        // pattern.clear();
        if (pattern.size() > 0) search_iter = CellSliceSearch(CellSlice{.data = buffer.data(), .size = buffer.size()}, pattern).begin();
        move_cursor_to_idx(search_iter->data - buffer.data());
    }
    else if (is_key_hold(KEY_BACKSPACE)) {
        pop_at_cursor();
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
    else if (is_ctrl_key(KEY_SPACE)) {
        if (start_selection.has_value()) {
            start_selection = nullopt;
        } else {
            start_selection = cursor.idx;
        }
        msg = "Marking ..";
    }
    else if (is_alt_key(KEY_W) && start_selection.has_value()) {
        string selected_text = get_selected_text();
        SetClipboardText(selected_text.c_str());
        start_selection = nullopt;
    }
    else if (is_ctrl_key(KEY_Y)) {
        const char* text = GetClipboardText();
        if (text != nullptr) {
            append_at_cursor(text);
        }
    }
    else if (is_ctrl_key(KEY_W) && start_selection.has_value()) {
        string selected_text = get_selected_text();
        SetClipboardText(selected_text.c_str());

        size_t start = start_selection.value();
        if (cursor.idx > start) {
            pop_at_cursor(cursor.idx - start);
        } else {
            size_t amount = start - cursor.idx;
            move_cursor_right(amount);
            pop_at_cursor(amount);
        }
        start_selection = nullopt;
    }
    else if (is_ctrl_key(KEY_K)) {
        const size_t last_col = line_size[cursor.row] - (cursor.row == line_size.size()-1 ? 0 : 1);
        size_t start = cursor.idx;
        if (cursor.col == last_col) {
            move_cursor_to(cursor.row+1, 0);
            pop_at_cursor(cursor.idx - start);
        } else {
            move_cursor_to(cursor.row, last_col);
            string cut_text = get_text(start, cursor.idx);
            SetClipboardText(cut_text.c_str());
            pop_at_cursor(cursor.idx - start);
        }
        start_selection = nullopt;
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

bool TextEditor::load(const fs::path& file) {
    current_file = fs::absolute(file);

    std::ifstream fin(file);
    if (!fin.is_open()) {
        TraceLog(LOG_ERROR, "Can't open file %s", file);
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
    std::ofstream fout(current_file);
    if (!fout.is_open()) {
        TraceLog(LOG_ERROR, "Can't open file %s", current_file.c_str());
        return false;
    }

    for (size_t i = 0; i < buffer.size(); i++) {
        fout << buffer[i].c;
    }

    fout.close();
    return true;
}

void TextEditor::move_text_view_to_point(Vector2 point) {
    if (point.x > text_view.x + text_view.width - PADDING_BOTTOM_RIGHT.x) text_view.x += point.x - (text_view.x + text_view.width) + PADDING_BOTTOM_RIGHT.x;
    else if (point.x < text_view.x) text_view.x -= text_view.x - point.x + PADDING_TOP_LEFT.x;

    if (point.y + cfg.line_height > text_view.y + text_view.height - PADDING_BOTTOM_RIGHT.y) text_view.y += point.y + cfg.line_height - (text_view.y + text_view.height) + PADDING_BOTTOM_RIGHT.y;
    else if (point.y < text_view.y) text_view.y -= text_view.y - point.y + PADDING_TOP_LEFT.y;
}

string TextEditor::get_selected_text() {
    if (start_selection.has_value()) {
        size_t start = std::min(start_selection.value(), cursor.idx);
        size_t end = std::max(start_selection.value(), cursor.idx);
        return get_text(start, end);
    }
    return "";
}

bool TextEditor::is_selected(size_t i) {
    if (start_selection.has_value()) {
        size_t start = std::min(start_selection.value(), cursor.idx);
        size_t end = std::max(start_selection.value(), cursor.idx);
        return i >= start && i < end;
    }
    return false;
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
    if (is_searching) {
        for (char ch : pattern) {
            Cell cell {
                .c = ch,
                    .fg = DEFAULT_FG,
                    .bg = nullopt
            };
            put_cell(cell, pos);
            pos.x += get_w(ch);
        }
    }
}

void TextEditor::render() {
    Vector2 cursor_pos = get_cursor_pos();
    if (!is_searching) {
        put_cursor(world_to_view(cursor_pos));
    }
    move_text_view_to_point(cursor_pos);

    Vector2 render_cursor = Vector2Zero();
    size_t i = 0, line = 0;
    while (line < line_size.size() && render_cursor.y + cfg.line_height <= text_view.y) {
        i += line_size[line++];
        render_cursor.y += cfg.line_height;
    }


    for (; i < buffer.size() && render_cursor.y < text_view.y + text_view.height; i++) {
        Cell cell = buffer[i];
        if (i == cursor.idx && !is_searching && cfg.cursor_shape == CursorShape::Block) {
            cell.fg = cfg.char_at_cursor_color;
        }
        int ch_w = get_w(cell.c);
        if (cell.c == '\n') {
            render_cursor.y += cfg.line_height;
            render_cursor.x = 0.0f;
        } else {
            if (i != cursor.idx && is_selected(i)) {
                cell.bg = cfg.on_selection_bg;
            } else if (is_searching) {
                size_t start = search_iter->data - buffer.data();
                if (i >= start && i < start + pattern.size()) {
                    cell.bg = cfg.search_bg;
                    cell.fg = cfg.search_fg;
                }
            }
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
