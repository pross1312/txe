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

void TextEditor::handle_events() {
    Editor::handle_events();
    if (is_ctrl_key(KEY_S)) save();
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
