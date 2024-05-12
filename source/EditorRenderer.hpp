#ifndef EDITOR_RENDERER_HPP
#define EDITOR_RENDERER_HPP

#include <raylib.h>
#include <raymath.h>
#include <assert.h>
#include <string>
#include "Editor.hpp"

using std::string_view;


struct EditorRenderer {
    const float SPACING = 0.0f;
    const float CAMERA_SPEED = 100.0f;
    const int CURSOR_WIDTH = 2;
    const Vector2 PADDING_BOTTOM_RIGHT { .x = 50.0f, .y = 10.0f };
    const Vector2 PADDING_TOP_LEFT     { .x = 10.0f, .y = 5.0f };
    Vector2 render_cursor;
    Font font {};
    float fs {0.0};
    Camera2D camera;

#ifdef USE_SDF_FONT
    Shader shader {};
#endif // USE_SDF_FONT

    EditorRenderer(const char *font_path, float font_size): fs{font_size} {

    camera.target = Vector2{ .x = -PADDING_TOP_LEFT.x, .y = -PADDING_TOP_LEFT.y };
    camera.offset = Vector2Zero();
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

#ifdef USE_SDF_FONT
    int file_size = 0;
    unsigned char *file_data = LoadFileData(font_path, &file_size);
    font.baseSize = font_size;
    font.glyphCount = 95;
    font.glyphs = LoadFontData(file_data, file_size, font_size, NULL, 0, FONT_SDF);
    Image atlas = GenImageFontAtlas(font.glyphs, &font.recs, 95, font_size, 0, 1);
    font.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);
    UnloadFileData(file_data);
    shader = LoadShader(0, "./resources/shaders/sdf.fs");
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);    // Required for SDF font
#else
        font = LoadFont(font_path);
#endif // USE_SDF_FONT
    SetTextLineSpacing(font_size);
    }

    inline void put_char_attr(Cell cell) {
        static char BUF[2] {};
        if (cell.c == '\n') {
            render_cursor.y += fs;
            render_cursor.x = 0.0f;
        } else {
            BUF[0] = cell.c; BUF[1] = 0;
            int char_width = MeasureTextEx(font, BUF, fs, SPACING).x;
            if (cell.bg.has_value()) {
                DrawRectangle(render_cursor.x, render_cursor.y, char_width, fs, cell.bg.value());
            }
#ifdef USE_SDF_FONT
            BeginShaderMode(shader);
                DrawTextEx(font, BUF, render_cursor, fs, SPACING, cell.fg);
            EndShaderMode();
#else
            DrawTextEx(font, BUF, render_cursor, fs, SPACING, cell.fg);
#endif //USE_SDF_FONT
            render_cursor.x += char_width;
        }
    }

    inline void put_cursor(Color c) {
        DrawRectangle(render_cursor.x, render_cursor.y, CURSOR_WIDTH, fs, c);
    }

    inline void draw_line(string_view line, Color c, Vector2 pos) {
        const char *text = TextFormat("%.*s", line.size(), line.data());
#ifdef USE_SDF_FONT
            BeginShaderMode(shader);
                DrawTextEx(font, text, pos, fs, SPACING, c);
            EndShaderMode();
#else
        DrawTextEx(font, text, pos, fs, SPACING, c);
#endif // USE_SDF_FONT
    }

    inline Vector2 measure_text(string_view sv) {
        const char *text = TextFormat("%.*s", sv.size(), sv.data());
        return MeasureTextEx(font, text, fs, SPACING);
    }

    void unload() {
        UnloadFont(font);
#ifdef USE_SDF_FONT
        UnloadShader(shader);
#endif // USE_SDF_FONT
    }

    inline void bring_point_into_view(Vector2 point) {
        Vector2 screen {.x = (float)GetScreenWidth(), .y = (float)GetScreenHeight()};
        if (point.x > camera.target.x + screen.x) camera.target.x += point.x - (camera.target.x + screen.x) + PADDING_BOTTOM_RIGHT.x;
        else if (point.x < camera.target.x) camera.target.x -= camera.target.x - point.x + PADDING_TOP_LEFT.x;
        if (point.y + font_height() > camera.target.y + screen.y) camera.target.y += point.y + font_height() - (camera.target.y + screen.y) + PADDING_BOTTOM_RIGHT.y;
        else if (point.y < camera.target.y) camera.target.y -= camera.target.y - point.y + PADDING_TOP_LEFT.y;
    }

    inline void render(const Editor &editor) {
        BeginMode2D(camera);
        render_cursor = Vector2Zero();
        if (editor.cursor.idx == 0) {
            put_cursor(WHITE);
        }
        for (size_t i = 0; i < editor.buffer.size(); i++) {
            Cell cell = editor.buffer[i];
            put_char_attr(cell);
            if (i+1 == editor.cursor.idx) {
                put_cursor(WHITE);
            }
        }
        EndMode2D();
    }

    inline int font_height() { return fs; }
};

#endif // EDITOR_RENDERER_HPP
