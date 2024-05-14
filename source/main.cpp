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
    _cfg.spacing = 0.0f;
    _cfg.cursor_width = 2;

    _cfg.cursor_color = WHITE;
    _cfg.dir_color = SKYBLUE;
    _cfg.file_color = WHITE;
    _cfg.on_cursor_bg_color = GetColor(0x313131ff);
    _cfg.n_on_cursor_bg_color = DEFAULT_BG;

    _cfg.font_size = FONT_SIZE;
    _cfg.line_height = FONT_SIZE;
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
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);
    int factor = 110;
    InitWindow(16*factor, 9*factor, "Txe");

    SetTargetFPS(60);

    init_config();

    Editor *editor = new TextEditor("README.md");
    if (argc > 1) {
        if (editor->type == Mode::Text && !static_cast<TextEditor*>(editor)->load(argv[1])) {
            abort();
        }
    }
    while (!WindowShouldClose()) {
        // float time = (float)GetTime();
        // BeginShaderMode(_cfg.font_shader);
        // int time_loc = GetShaderLocation(_cfg.font_shader, "time");
        // SetShaderValue(_cfg.font_shader, time_loc, &time, SHADER_UNIFORM_FLOAT);
        // EndShaderMode();
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

        editor->render();

        EndDrawing();
    }

    destroy_config();
    CloseWindow();
    return 0;
}
