#include <stdio.h>
#include <string.h>
#include "Editor.hpp"
#include "EditorRenderer.hpp"

int main() {
    SetConfigFlags(FLAG_WINDOW_MINIMIZED);
    InitWindow(1300, 900, "Txe");
    const float FONT_SIZE = 30.0f;
    const char *FONT_PATH ="resources/fonts/iosevka-term-regular.ttf";

    SetTargetFPS(60);
    Editor editor;
    EditorRenderer renderer(FONT_PATH, FONT_SIZE);

    while (!WindowShouldClose()) {
        BeginDrawing();
        BeginMode2D(renderer.camera);

        int c;
        while ((c = GetCharPressed())) { editor.append_at_cursor((char)c); }
        if (IsKeyPressed(KEY_BACKSPACE) || IsKeyPressedRepeat(KEY_BACKSPACE)) editor.pop_at_cursor();
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressedRepeat(KEY_ENTER))         editor.append_at_cursor('\n');
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressedRepeat(KEY_LEFT))           editor.move_cursor_left(1);
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressedRepeat(KEY_RIGHT))         editor.move_cursor_right(1);
        if (IsKeyPressed(KEY_UP) || IsKeyPressedRepeat(KEY_UP))               editor.move_cursor_up(1);
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressedRepeat(KEY_DOWN))           editor.move_cursor_down(1);

        ClearBackground(BLACK);

        Vector2 pos = Vector2Zero();
        int i = 0;
        for (const string_view &line : editor.lines()) {
            renderer.draw_line(line, WHITE, pos);
            if (i == editor.cursor.row) {
                Vector2 size = renderer.measure_text(line.substr(0, editor.cursor.col));
                Vector2 cursor_pos {.x = size.x, .y = pos.y};
                renderer.draw_cursor(cursor_pos, WHITE);
                renderer.bring_point_into_view(cursor_pos);
            }
            pos.y += renderer.font_height();
            i++;
        }

        EndMode2D();
        EndDrawing();
    }

    renderer.unload();
    CloseWindow();
    return 0;
}
