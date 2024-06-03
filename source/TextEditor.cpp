#include "TextEditor.h"

#include <functional>
#include <cstring>
#include <numeric>
#include <cassert>
#include <fstream>
using namespace std;

extern Config _cfg;
constexpr float SEPERATE_PADDING = 10.0f;

TextEditor::TextEditor(const fs::path& file): Editor(Mode::Text), attributes(), pattern(true) {
    assert(!fs::is_directory(file) && "Must create TextEditor with a file");
    if (!load(file)) {
        TraceLog(LOG_INFO, current_file.c_str());
        SetWindowTitle("Txe");
    } else {
        SetWindowTitle(TextFormat("%s - Txe", file.c_str()));
    }
    on_resize();
    for (const char* str : _cfg.keywords) {
        trie.insert(str);
    }

    pattern.event_handler = [&](TextEdit& self) {
        char c;
        while ((c = GetCharPressed())) {
            self.append_at_cursor(c);
            search_iter = Searcher(string_view(buffer.buffer), self.buffer).begin();
            buffer.move_cursor_to_idx(search_iter->data() - buffer.data());
        }
        if (is_key_hold(KEY_ENTER)) {
            buffer.move_cursor_to_idx(search_iter->data() - buffer.data());
            is_searching = false;
            return true;
        }
        if (is_key_hold(KEY_BACKSPACE) && self.size() > 0) {
            self.pop_at_cursor();
            Searcher searcher(string_view(buffer.buffer), self.buffer);
            search_iter = searcher.begin();
            return true;
        }
        if (is_ctrl_key(KEY_S)) {
            ++search_iter;
            Searcher searcher(string_view(buffer.buffer), self.buffer);
            if (search_iter == searcher.end()) {
                search_iter = searcher.begin();
            }
            buffer.move_cursor_to_idx(search_iter->data() - buffer.data());
            return true;
        }
        if (is_ctrl_key(KEY_N)) {
            Searcher searcher(string_view(buffer.buffer), self.buffer);
            if (search_iter == searcher.begin()) {
                search_iter = searcher.end();
            }
            --search_iter;
            buffer.move_cursor_to_idx(search_iter->data() - buffer.data());
            return true;
        }
        return false;
    };
}

StringView TextEditor::get_selected_text() {
    if (start_selection.has_value()) {
        size_t start = std::min(start_selection.value(), buffer.cursor.idx);
        size_t end = std::max(start_selection.value(), buffer.cursor.idx);
        return StringView(buffer.data() + start, end - start);
    }
    return StringView("");
}

bool TextEditor::is_selected(size_t i) {
    if (start_selection.has_value()) {
        size_t start = std::min(start_selection.value(), buffer.cursor.idx);
        size_t end = std::max(start_selection.value(), buffer.cursor.idx);
        return i >= start && i < end;
    }
    return false;
}

void TextEditor::on_resize() {
    origin.x = _cfg.line_number_width + 2*SEPERATE_PADDING;
    origin.y = 0.0f;

    text_view.x = 0.0f;
    text_view.y = 0.0f;
    text_view.width = GetScreenWidth() - origin.x;
    text_view.height = GetScreenHeight() - _cfg.msg_box_height;
}

bool TextEditor::is_keyword(StringView token) {
    Trie::Node* ptr = trie.root;
    for (size_t i = 0; ptr != nullptr && i < token.size; i++) {
        ptr = ptr->children[(int)token[i]];
    }
    return ptr != nullptr && ptr->str.size() != 0;
}

void TextEditor::highlight() {
    attributes.resize(buffer.size());
    static auto color = [&](StringView slice, Color c) {
        size_t start = slice.data - buffer.data();
        for (size_t i = 0; i < slice.size; i++) attributes[start + i].fg = c;
    };
    StringView slice(buffer.buffer);
    while (slice.size > 0) {
        slice = trim_left(slice);
        if (slice.size == 0) break;
        switch (slice[0]) {
            case '/': {
                if (slice.size >= 2) {
                    if (slice[1] == '/') {
                        StringView line = get_line(slice);
                        color(line, _cfg.comment_color);
                        slice.data += line.size;
                        slice.size -= line.size;
                    }
                    else if (slice[1] == '*') {
                        StringView comment = slice;
                        while (slice.size >= 2 && (slice[0] != '*' || slice[1] != '/')) {
                            slice.data++;
                            slice.size--;
                        }
                        if (slice.size >= 2) {
                            slice.data += 2;
                            slice.size -= 2;
                        } else {
                            slice.data += 1;
                            slice.size -= 1;
                        }
                        comment.size -= slice.size;
                        color(comment, _cfg.comment_color);
                    } else {
                        slice.data++;
                        slice.size--;
                    }
                } else {
                    slice.data++;
                    slice.size--;
                }
            } break;
            case '"': {
                slice.data++; slice.size--;
                StringView string = get_until_char(slice, '"');
                slice.data += string.size;
                slice.size -= string.size;
                string.data--; string.size++;
                color(string, _cfg.string_color);
            } break;
            case '#': {
                StringView directive = slice;
                slice.data++; slice.size--;
                slice = trim_left(slice);
                StringView word = get_word(slice);
                directive.size = (word.data - directive.data) + word.size;
                color(directive, _cfg.directive_color);
                slice.data += word.size; slice.size -= word.size;
                StringView line = get_line(slice);
                color(line, _cfg.string_color);
                slice.data += line.size; slice.size -= line.size;
            } break;
            case '\'': {
                size_t len = (size_t)std::min((unsigned long long)slice.size, 3ull);
                color(StringView{slice.data,  len}, _cfg.string_color);
                slice.data += len; slice.size -= len;
            } break;
            default: {
                 if (isalnum(slice[0])) {
                     StringView word = get_word(slice);
                     if (is_keyword(word)) color(word, _cfg.keyword_color);
                     else color(word, DEFAULT_FG);
                     slice.data += word.size;
                     slice.size -= word.size;
                 } else {
                     // TraceLog(LOG_ERROR, "Can't parse %c", slice[0]);
                     attributes[slice.data - buffer.data()].fg = DEFAULT_FG;
                     slice.data++; slice.size--;
                 }
            }
        }
    }
}

void TextEditor::render_line_number(size_t start) {
    DrawRectangle(0.0f, 0.0f, _cfg.line_number_width + SEPERATE_PADDING, text_view.height, _cfg.line_number_region_bg);
    Vector2 pos {.x = origin.x - 2*SEPERATE_PADDING, .y = origin.y + (float)start * _cfg.line_height - text_view.y};
    for (size_t line = start; line < buffer.line_size.size() && pos.y < text_view.height; line++, pos.y += _cfg.line_height) {
        pos.x = origin.x - 2*SEPERATE_PADDING;
        int line_temp = line+1;
        Color fg = buffer.cursor.row == line ? _cfg.current_line_number_color : _cfg.line_number_color;
        while (line_temp > 0) {
            int n = line_temp%10;
            char c_n = n + '0';
            pos.x -= _cfg.char_w(c_n);
            put_cell(c_n, Attr{.fg = fg, .bg = nullopt}, pos);
            line_temp /= 10;
        }
    }
}

void TextEditor::render_msg() {
    Vector2 pos {.x = 0.0f, .y = (float)GetScreenHeight() - (_cfg.line_height + _cfg.msg_box_height)*0.5f};
    DrawRectangle(0.0f, (float)GetScreenHeight() - _cfg.msg_box_height, (float)GetScreenWidth(), _cfg.msg_box_height, DEFAULT_BG);
    if (msg.data != nullptr && msg.size > 0) {
        for (size_t i = 0; i < msg.size; i++) {
            put_cell(msg[i], Attr{.fg = DEFAULT_FG, .bg = nullopt}, pos);
            pos.x += _cfg.char_w(msg[i]);
        }
    }
    if (is_searching) {
        for (char ch : pattern.buffer) {
            put_cell(ch, Attr{.fg = DEFAULT_FG, .bg = nullopt}, pos);
            pos.x += _cfg.char_w(ch);
        }
    }
}

void TextEditor::move_text_view_to_point(Vector2 point) {
    if (point.x > text_view.x + text_view.width - PADDING_BOTTOM_RIGHT.x) text_view.x += point.x - (text_view.x + text_view.width) + PADDING_BOTTOM_RIGHT.x;
    else if (point.x < text_view.x) text_view.x -= text_view.x - point.x + PADDING_TOP_LEFT.x;

    if (point.y + _cfg.line_height > text_view.y + text_view.height - PADDING_BOTTOM_RIGHT.y) text_view.y += point.y + _cfg.line_height - (text_view.y + text_view.height) + PADDING_BOTTOM_RIGHT.y;
    else if (point.y < text_view.y) text_view.y -= text_view.y - point.y + PADDING_TOP_LEFT.y;
}

void TextEditor::render() {
    if (is_c_file) highlight();
    Vector2 cursor_pos = buffer.get_cursor_pos();
    move_text_view_to_point(cursor_pos);
    if (!is_searching) {
        put_cursor(world_to_view(cursor_pos));
    }
    Vector2 render_cursor = Vector2Zero();
    size_t i = 0, line = 0;
    while (line < buffer.line_size.size() && render_cursor.y + _cfg.line_height <= text_view.y) {
        i += buffer.line_size[line++];
        render_cursor.y += _cfg.line_height;
    }
    for (; i < buffer.size() && render_cursor.y < text_view.y + text_view.height; i++) {
        Attr attr;
        if (is_c_file) attr = attributes[i];
        if (i == buffer.cursor.idx && !is_searching && _cfg.cursor_shape == CursorShape::Block) {
            attr.fg = _cfg.char_at_cursor_color;
        }
        int ch_w = _cfg.char_w(buffer[i]);
        if (buffer[i] == '\n') {
            render_cursor.y += _cfg.line_height;
            render_cursor.x = 0.0f;
        } else {
            if (i != buffer.cursor.idx && is_selected(i)) {
                attr.bg = _cfg.on_selection_bg;
            } else if (is_searching) {
                size_t start = search_iter->data() - buffer.data();
                if (i >= start && i < start + pattern.size()) {
                    attr.bg = _cfg.search_bg;
                    attr.fg = _cfg.search_fg;
                }
            }
            if (render_cursor.x < text_view.x + text_view.width && render_cursor.x + ch_w > text_view.x) {
                put_cell(buffer[i], attr, world_to_view(render_cursor));
            }
            render_cursor.x += ch_w;
        }
    }
    render_line_number(line);
    render_msg();
}

int TextEditor::handle_searching_events() {
    if (is_ctrl_key(KEY_G)) {
        is_searching = false;
        msg = "CTRL-G";
    } else {
        pattern.handle_events();
    }
    return 0;
}

int TextEditor::handle_events() {
    const size_t screen_lines = (text_view.height / _cfg.line_height);
    if (is_searching) return handle_searching_events();

    static bool is_ctrl_x = false;
    TextEdit::Cursor& cursor = buffer.cursor;

    buffer.handle_events();
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
    else if (is_ctrl_key(KEY_S)) {
        is_searching = true;
        msg = "Search: ";
        // pattern.clear();
        if (pattern.size() > 0) search_iter = Searcher(string_view(buffer.buffer), pattern.buffer).begin();
        buffer.move_cursor_to_idx(search_iter->data() - buffer.data());
    }
    else if (is_ctrl_and_key_hold(KEY_V)) buffer.move_cursor_down(screen_lines);
    else if (is_alt_and_key_hold(KEY_V)) buffer.move_cursor_up(screen_lines);
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
        StringView selected_text = get_selected_text();
        SetClipboardText(string(selected_text).c_str());
        start_selection = nullopt;
    }
    else if (is_ctrl_key(KEY_Y)) {
        const char* text = GetClipboardText();
        if (text != nullptr) {
            buffer.append_at_cursor(text);
        }
    }
    else if (is_ctrl_key(KEY_W) && start_selection.has_value()) {
        StringView selected_text = get_selected_text();
        SetClipboardText(string(selected_text).c_str());

        size_t start = start_selection.value();
        if (cursor.idx > start) {
            buffer.pop_at_cursor(cursor.idx - start);
        } else {
            size_t amount = start - cursor.idx;
            buffer.move_cursor_right(amount);
            buffer.pop_at_cursor(amount);
        }
        start_selection = nullopt;
    }
    else if (is_ctrl_key(KEY_K)) {
        const size_t last_col = buffer.line_size[cursor.row] - (cursor.row == buffer.line_size.size()-1 ? 0 : 1);
        size_t start = cursor.idx;
        if (cursor.col == last_col) {
            buffer.move_cursor_to(cursor.row+1, 0);
            buffer.pop_at_cursor(cursor.idx - start);
        } else {
            buffer.move_cursor_to(cursor.row, last_col);
            StringView cut_text(buffer.data() + start, cursor.idx - start);
            SetClipboardText(string(cut_text).c_str());
            buffer.pop_at_cursor(cursor.idx - start);
        }
        start_selection = nullopt;
    }
    else if (IsKeyDown(KEY_LEFT_ALT) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_COMMA)) {
        cursor.idx = 0;
        cursor.row = 0;
        cursor.col = 0;
    }
    else if (IsKeyDown(KEY_LEFT_ALT) && IsKeyDown(KEY_LEFT_SHIFT) && IsKeyPressed(KEY_PERIOD)) {
        cursor.idx = buffer.size();
        cursor.row = buffer.line_size.size()-1;
        cursor.col = buffer.line_size[cursor.row];
    }
    return 0;
}

bool TextEditor::load(const fs::path& file) {
    current_file = fs::absolute(file);
    const string extension = current_file.extension().string();
    if (extension == ".cpp" || extension == ".c" || extension == ".hpp" || extension == ".h" || extension == ".cc") {
        is_c_file = true;
    }

    std::ifstream fin(file);
    if (!fin.is_open()) {
        TraceLog(LOG_ERROR, "Can't open file %s", file);
        return false;
    }

    char c;
    while (!fin.eof() && (c = fin.get())) {
        if (isprint(c) || isspace(c)) {
            if (c != '\r') {
                buffer.append_at_cursor(c);
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
        fout << buffer[i];
    }

    fout.close();
    return true;
}
