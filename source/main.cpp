#include "Helper.h"
#include "TextEditor.h"
#include "FileExplorer.h"

Config _cfg;

inline constexpr int FONT_SIZE = 30;
inline constexpr const char *FONT_PATH = "resources/fonts/iosevka-term-regular.ttf";
inline constexpr const char *FONT_SHADER_PATH = "./resources/shaders/sdf.fs";

void init_config(const fs::path& exe_dir) {
#ifdef USE_SDF_FONT
    int file_size = 0;
    unsigned char *file_data = LoadFileData((exe_dir / FONT_PATH).c_str(), &file_size);
    _cfg.font.baseSize = _cfg.font_size;
    _cfg.font.glyphCount = 95;
    _cfg.font.glyphs = LoadFontData(file_data, file_size, _cfg.font_size, NULL, 0, FONT_SDF);
    Image atlas = GenImageFontAtlas(_cfg.font.glyphs, &_cfg.font.recs, 95, _cfg.font_size, 0, 1);
    _cfg.font.texture = LoadTextureFromImage(atlas);
    UnloadImage(atlas);
    UnloadFileData(file_data);
    _cfg.font_shader = LoadShader(0, (exe_dir / FONT_SHADER_PATH).c_str());
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

    init_config(fs::absolute(argv[0]).parent_path());

    Editor *editor = nullptr;
    if (argc > 1) {
        editor = new TextEditor(argv[1]);
    } else {
        editor = new TextEditor;
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
                Editor* new_editor = nullptr;
                if (tx->current_file.has_value()) {
                    new_editor = new FileExplorer(fs::absolute(tx->current_file.value()).parent_path());
                } else {
                    new_editor = new FileExplorer();
                }
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
