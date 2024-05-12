#include <stdio.h>
#include <string.h>
#include "Editor.hpp"
#include "EditorRenderer.hpp"

int main() {
    SetConfigFlags(FLAG_WINDOW_MINIMIZED);
    int factor = 100;
    InitWindow(16*factor, 9*factor, "Txe");
    const float FONT_SIZE = 30.0f;
    const char *FONT_PATH ="resources/fonts/iosevka-term-regular.ttf";

    SetTargetFPS(60);
    Editor editor;
    EditorRenderer renderer(FONT_PATH, FONT_SIZE);

    while (!WindowShouldClose()) {
        BeginDrawing();

        int c;
        while ((c = GetCharPressed())) { editor.append_at_cursor((char)c); }
        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) editor.pop_at_cursor();
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressedRepeat(KEY_ENTER))         editor.append_at_cursor('\n');
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT))           editor.move_cursor_left(1);
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT))         editor.move_cursor_right(1);
        if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP))               editor.move_cursor_up(1);
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN))           editor.move_cursor_down(1);

        ClearBackground(BLACK);

        renderer.render(editor);

        EndDrawing();
    }

    renderer.unload();
    CloseWindow();
    return 0;
}
