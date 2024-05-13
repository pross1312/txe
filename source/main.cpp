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

        editor->handle_events();

        ClearBackground(BLACK);

        renderer.render(editor);

        EndDrawing();
    }

    renderer.unload();
    CloseWindow();
    return 0;
}
