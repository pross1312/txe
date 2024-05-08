#include <stdio.h>
#include <string.h>
#include "Editor.hpp"
#include "EditorRenderer.hpp"

void init_editor(const char *font_path);

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MINIMIZED);
    InitWindow(1300, 900, "Txe");
    const float FONT_SIZE = 30.0f;
    const char *FONT_PATH ="resources/fonts/iosevka-term-regular.ttf";
    const int WIDTH = GetRenderWidth(), HEIGHT = GetRenderHeight(), FPS = 60;
    printf("%d\n", WIDTH);

    SetTargetFPS(FPS);
    Editor editor;
    EditorRenderer renderer(FONT_PATH, FONT_SIZE);
    while (!WindowShouldClose()) {
        BeginDrawing();
        BeginMode2D(renderer.camera);
        int c;
        while ((c = GetCharPressed())) { editor.append((char)c); }
        while ((c = GetKeyPressed())) {
            switch (c) {
            case KEY_ENTER:
                editor.append('\n');
                break;
            case KEY_BACKSPACE:
                if (editor.buffer.size() > 0) editor.pop();
                break;
            default:
                break;
            }
        }
        ClearBackground(BLACK);
        Vector2 cursor_pos { .x = 0.0f, .y = -float(renderer.font_height())};
        for (const string_view& line : editor.lines()) {
            cursor_pos.y += renderer.font_height();
            Vector2 size = renderer.draw_line(line, WHITE, CLITERAL(Vector2) {.x = 0.0f, .y = cursor_pos.y});
            cursor_pos.x = size.x;
        }
        renderer.draw_cursor(cursor_pos, WHITE);
        EndMode2D();
        EndDrawing();
    }
    renderer.unload();
    CloseWindow();
    return 0;
}
