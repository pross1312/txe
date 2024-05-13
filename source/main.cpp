#include <stdio.h>
#include <string.h>
#include "TextEditor.h"
#include "Helper.h"
#include "FileExplorer.h"
#include "EditorRenderer.hpp"

int main(int argc, const char **argv) {
    SetConfigFlags(FLAG_WINDOW_MINIMIZED);
    int factor = 100;
    InitWindow(16*factor, 9*factor, "Txe");
    const float FONT_SIZE = 30.0f;
    const char *FONT_PATH ="resources/fonts/iosevka-term-regular.ttf";

    SetTargetFPS(60);
    Editor *editor = new TextEditor("README.md");
    if (argc > 1) {
        if (editor->type == Mode::Text && !static_cast<TextEditor*>(editor)->load(argv[1])) {
            abort();
        }
    }
    EditorRenderer renderer(FONT_PATH, FONT_SIZE);

    while (!WindowShouldClose()) {
        BeginDrawing();

        if (editor->handle_events() == 1) {
            switch (editor->type) {
            case Mode::Text: {
                TextEditor *tx = static_cast<TextEditor*>(editor);
                if (tx->current_file.has_value()) {
                    fs::path current_file = tx->current_file.value();
                    delete editor;
                    editor = new FileExplorer(current_file);
                } else {
                    delete editor;
                    editor = new FileExplorer();
                }
            } break;
            case Mode::File: {
                fs::path file = static_cast<FileExplorer*>(editor)->get_file();
                delete editor;
                editor = new TextEditor(file.c_str());
            } break;
            }
        }

        ClearBackground(BLACK);

        renderer.render(editor);

        EndDrawing();
    }

    renderer.unload();
    CloseWindow();
    return 0;
}
