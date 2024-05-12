#include <cstring>
#include "FileExplorer.h"

using namespace std;

FileExplorer::FileExplorer(): FileExplorer(fs::current_path()) { }

FileExplorer::FileExplorer(fs::path current_file): Editor(Mode::File) {
    if (!current_file.is_absolute()) {
        current_file = fs::absolute(current_file);
    }
    current_dir = current_file.parent_path();
    file_name = current_file.filename();

    const char *dir = current_dir.c_str();
    size_t len = strlen(dir);
    for (size_t i = 0; i < len; i++) {
        append_at_cursor(dir[i]);
    }
    append_at_cursor('/');
    if (file_name.has_value()) {
        for (char c : file_name.value()) append_at_cursor(c);
    }
}

void FileExplorer::pop_at_cursor() {
    Editor::pop_at_cursor();
}


void FileExplorer::move_cursor_up(size_t amount) {
    (void)amount;
}

void FileExplorer::move_cursor_down(size_t amount) {
    (void)amount;
}

void FileExplorer::move_cursor_left(size_t amount) {
    (void)amount;
}

void FileExplorer::move_cursor_right(size_t amount) {
    (void)amount;
}

void FileExplorer::move_cursor_to(size_t row, size_t col) {
    (void)row; (void)col;
}

