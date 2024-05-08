#ifndef EDITOR_RENDERER_HPP
#define EDITOR_RENDERER_HPP

#include <raylib.h>
#include <assert.h>
#include <string>

using std::string_view;


struct EditorRenderer {
    Font font {0};
    float fs {0.0};

#ifdef USE_SDF_FONT
    Shader shader {0};
#endif // USE_SDF_FONT

    EditorRenderer(const char *font_path, float font_size): fs{font_size} {

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

    inline void draw_line(string_view line, Color c, Vector2 pos) {

#ifdef USE_SDF_FONT
        BeginShaderMode(shader);
        DrawTextEx(font, TextFormat("%.*s", (int)line.size(), line.data()), pos, fs, 1.0f, c);
        EndShaderMode();
#else
        DrawTextEx(font, TextFormat("%.*s", (int)line.size(), line.data()), pos, fs, 1.0f, c);
#endif // USE_SDF_FONT

    }

    ~EditorRenderer() {
#ifdef USE_SDF_FONT
        UnloadShader(shader);
#endif // USE_SDF_FONT
        UnloadFont(font);
    }

    inline int font_height() { return font.baseSize; }
private:
};

#endif // EDITOR_RENDERER_HPP
