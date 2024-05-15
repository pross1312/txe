#include "TextEditor.h"
#include "Helper.h"
#include "FileExplorer.h"

#include <stdio.h>
#include <string.h>

Config _cfg;

inline constexpr int FONT_SIZE = 30;
inline constexpr const char *FONT_PATH = "resources/fonts/iosevka-term-regular.ttf";
inline constexpr const char *FONT_SHADER_PATH = "./resources/shaders/sdf.fs";

void init_config() {
#ifdef USE_SDF_FONT
    int file_size = 0;
    unsigned char *file_data = LoadFileData(FONT_PATH, &file_size);
    _cfg.font.baseSize = _cfg.font_size;
    _cfg.font.glyphCount = 95;
    _cfg.font.glyphs = LoadFontData(file_data, file_size, _cfg.font_size, NULL, 0, FONT_SDF);
    Image atlas = GenImageFontAtlas(_cfg.font.glyphs, &_cfg.font.recs, 95, _cfg.font_size, 0, 1);
    _cfg.font.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);
    UnloadFileData(file_data);
    _cfg.font_shader = LoadShader(0, FONT_SHADER_PATH);
    SetTextureFilter(_cfg.font.texture, TEXTURE_FILTER_BILINEAR);    // Required for SDF font
#else
    _cfg.font = LoadFont(FONT_PATH);
#endif // USE_SDF_FONT
    SetTextLineSpacing(_cfg.line_height);
}

void destroy_config() {
    UnloadFont(_cfg.font);
#ifdef USE_SDF_FONT
    UnloadShader(_cfg.font_shader);
#endif // USE_SDF_FONT
}

int main(int argc, const char **argv) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MINIMIZED);

    InitWindow(0, 0, "Txe");

    SetTargetFPS(60);

    init_config();

    Editor *editor = nullptr;
    if (argc > 1) {
        editor = new TextEditor(argv[1]);
    } else {
        editor = new FileExplorer;
    }
    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            editor->on_resize();
        }
        // float time = (float)GetTime();
        // BeginShaderMode(_cfg.font_shader);
        // int time_loc = GetShaderLocation(_cfg.font_shader, "time");
        // SetShaderValue(_cfg.font_shader, time_loc, &time, SHADER_UNIFORM_FLOAT);
        // EndShaderMode();
        BeginDrawing();
        ClearBackground(DEFAULT_BG);

        if (editor->handle_events() == 1) {
            switch (editor->type) {
            case Mode::Text: {
                TextEditor *tx = static_cast<TextEditor*>(editor);
                Editor* new_editor = new FileExplorer(fs::absolute(tx->current_file).parent_path());
                delete editor;
                editor = new_editor;
            } break;
            case Mode::File: {
                Editor* new_editor = new TextEditor(static_cast<FileExplorer*>(editor)->get_file());
                delete editor;
                editor = new_editor;
            } break;
            }
        }


        editor->render();

        EndDrawing();
    }

    destroy_config();
    CloseWindow();
    return 0;
}
