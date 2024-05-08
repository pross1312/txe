#ifndef EDITOR_RENDERER_HPP
#define EDITOR_RENDERER_HPP

#include <raylib.h>
#include <raymath.h>
#include <assert.h>
#include <string>

using std::string_view;


struct EditorRenderer {
    const float SPACING = 1.0f;
    const float CAMERA_SPEED = 100.0f;
    const int CURSOR_WIDTH = 2;
    const Vector2 PADDING = { .x = 20.0f, .y = 5.0f };
    Font font {0};
    float fs {0.0};
    Camera2D camera;

#ifdef USE_SDF_FONT
    Shader shader {0};
#endif // USE_SDF_FONT

    EditorRenderer(const char *font_path, float font_size): fs{font_size} {

    camera.target = Vector2{ .x = -PADDING.x, .y = -PADDING.y };
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
    }

    inline Vector2 draw_line(string_view line, Color c, Vector2 pos) {
#ifdef USE_SDF_FONT
            BeginShaderMode(shader);
                DrawTextEx(font, TextFormat("%.*s", (int)line.size(), line.data()), pos, fs, SPACING, c);
            EndShaderMode();
#else
        DrawTextEx(font, TextFormat("%.*s", (int)line.size(), line.data()), pos, fs, SPACING, c);
#endif // USE_SDF_FONT
        return MeasureTextEx(font, line.data(), fs, SPACING);
    }

    inline void draw_cursor(Vector2 pos, Color c) {
        DrawRectangle(pos.x, pos.y, CURSOR_WIDTH, fs, c);
        // float dis = Vector2Distance(pos, camera.target);
        // if (dis > CAMERA_SPEED*1.0f) {
        //     camera.target.x += (pos.x - camera.target.x)/dis * CAMERA_SPEED * GetFrameTime();
        //     camera.target.y += (pos.y - camera.target.y)/dis * CAMERA_SPEED * GetFrameTime();
        // }
    }

    ~EditorRenderer() {
#ifdef USE_SDF_FONT
        UnloadShader(shader);
#endif // USE_SDF_FONT
        UnloadFont(font);
    }

    inline int font_height() { return font.baseSize; }
};

#endif // EDITOR_RENDERER_HPP
