#include <stdio.h>
#include <string.h>
#include "Editor.hpp"
#include "EditorRenderer.hpp"

void init_editor(const char *font_path);

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    InitWindow(100, 100, "Txe");
    const float FONT_SIZE = 26.0f;
    const char *FONT_PATH ="resources/fonts/JetBrainsMono-Regular.ttf";
    const int WIDTH = GetRenderWidth(), HEIGHT = GetRenderHeight(), FPS = 30;
    printf("%d\n", WIDTH);

    SetTargetFPS(FPS);
    Editor editor;
    EditorRenderer renderer(FONT_PATH, FONT_SIZE);
    while (!WindowShouldClose()) {
        BeginDrawing();
        int c;
        while ((c = GetCharPressed())) { editor.append((char)c); }
        while ((c = GetKeyPressed())) {
            switch (c) {
            case KEY_ENTER:
                editor.append('\n');
                break;
            case KEY_BACKSPACE:
                editor.pop();
                break;
            default:
                break;
            }
        }
        ClearBackground(BLACK);
        const vector<string_view> lines = editor.lines();
        for (size_t i = 0; i < lines.size(); i++) {
            renderer.draw_line(lines[i], WHITE, CLITERAL(Vector2) {.x = 0.0f, .y = float(int(i)*renderer.font_height())});
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
