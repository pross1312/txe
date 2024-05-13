#include "TextEditor.h"
#include "Helper.h"

#include <numeric>
#include <fstream>
using namespace std;

TextEditor::TextEditor(): Editor(Mode::Text) {
    current_file = nullopt;
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
            if (c == '\t') {
                append_at_cursor(' ');
                append_at_cursor(' ');
                append_at_cursor(' ');
                append_at_cursor(' ');
            } else {
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
