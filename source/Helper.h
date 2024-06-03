#pragma once
#include <raylib.h>
#include <string>
#include <cstring>

struct StringView {
    const char* data;
    size_t size;
    char operator[](size_t i) { return data[i]; }
    StringView(): data(nullptr), size(0) {}
    StringView(const std::string& str): data(str.data()), size(str.size()) {}
    StringView(const char* str, size_t size): data(str), size(size) {}
    StringView(const char* str): data(str), size(strlen(str)) {}
    operator std::string() const { return std::string(std::string_view(data, size)); }
    operator std::string_view() const { return std::string_view(data, size); }
};

inline float calculate_luminance(Color c){
    return (float) (0.2126*c.r + 0.7152*c.g + 0.0722*c.b);
}

inline bool is_char_in(char ch, std::string_view str) {
    for (char c : str) {
        if (c == ch) return true;
    }
    return false;
}

inline int wrap_index(int i, int i_max) {
   return ((i % i_max) + i_max) % i_max;
}

inline bool is_rect_in_rect(Rectangle rect, Rectangle parent) {
    return !(rect.x + rect.width < parent.x || rect.y + rect.height < parent.y || rect.x > parent.x + parent.width || rect.y > parent.y + parent.height);
}

inline bool is_ctrl_key(KeyboardKey key_code) {
    return (IsKeyDown(KEY_CAPS_LOCK) || IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(key_code);
}

inline bool is_alt_key(KeyboardKey key_code) {
    return (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) && IsKeyPressed(key_code);
}

inline bool is_key_hold(KeyboardKey key_code) {
    if (key_code != KEY_LEFT_CONTROL && IsKeyDown(KEY_LEFT_CONTROL)) return false;
    if (key_code != KEY_LEFT_ALT && IsKeyDown(KEY_LEFT_ALT)) return false;
    if (key_code != KEY_LEFT_SHIFT && IsKeyDown(KEY_LEFT_SHIFT)) return false;
    return IsKeyPressed(key_code) || IsKeyPressedRepeat(key_code);
}

inline bool is_alt_and_key_hold(KeyboardKey key_code) {
    return (IsKeyDown(KEY_LEFT_ALT) || IsKeyDown(KEY_RIGHT_ALT)) && (IsKeyPressed(key_code) || IsKeyPressedRepeat(key_code));
}

inline bool is_ctrl_and_key_hold(KeyboardKey key_code) {
    return (IsKeyDown(KEY_CAPS_LOCK) || IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && (IsKeyPressed(key_code) || IsKeyPressedRepeat(key_code));
}

inline int DamerauLevenshteinDistance(std::string_view source, std::string_view target, int threshold) {

    int length1 = source.size();
    int length2 = target.size();

    // Return trivial case - difference in string lengths exceeds threshhold
    if (std::abs(length1 - length2) > threshold) { return 1e9; }

    // Ensure arrays [i] / length1 use shorter length 
    if (length1 > length2) {
        std::swap(target, source);
        std::swap(length1, length2);
    }

    int maxi = length1;
    int maxj = length2;

    int *dCurrent = new int[maxi + 1];
    int *dMinus1 = new int[maxi + 1];
    int *dMinus2 = new int[maxi + 1];
    int *dSwap;

    for (int i = 0; i <= maxi; i++) { dCurrent[i] = i; }

    int jm1 = 0, im1 = 0, im2 = -1;

    for (int j = 1; j <= maxj; j++) {

        // Rotate
        dSwap = dMinus2;
        dMinus2 = dMinus1;
        dMinus1 = dCurrent;
        dCurrent = dSwap;

        // Initialize
        int minDistance = 1e9;
        dCurrent[0] = j;
        im1 = 0;
        im2 = -1;

        for (int i = 1; i <= maxi; i++) {

            int cost = source[im1] == target[jm1] ? 0 : 1;

            int del = dCurrent[im1] + 1;
            int ins = dMinus1[i] + 1;
            int sub = dMinus1[im1] + cost;

            //Fastest execution for min value of 3 integers
            int min = (del > ins) ? (ins > sub ? sub : ins) : (del > sub ? sub : del);

            if (i > 1 && j > 1 && source[im2] == target[jm1] && source[im1] == target[j - 2])
                min = std::min(min, dMinus2[im2] + cost);

            dCurrent[i] = min;
            if (min < minDistance) { minDistance = min; }
            im1++;
            im2++;
        }
        jm1++;
        if (minDistance > threshold) { return 1e9; }
    }

    int result = dCurrent[maxi];
    return (result > threshold) ? 1e9 : result;
}

inline bool is_rect_in_view(float x, float y, float w, float h) {
    return !(x + w <= 0.0f || y + h <= 0.0f || x >= 0.0f + GetScreenWidth() || y >= 0.0f + GetScreenHeight());
}

inline StringView trim_left(StringView sv) {
    size_t i = 0;
    while (i < sv.size && isspace(sv[i])) i++;
    return StringView(sv.data + i, sv.size - i);
}

inline StringView get_word(StringView sv) {
    for (size_t i = 0; i < sv.size; i++) {
        if (!isalnum(sv[i]) && sv[i] != '_') {
            return StringView(sv.data, i);
        }
    }
    return sv;
};

inline StringView get_line(StringView sv) {
    for (size_t i = 0; i < sv.size; i++) {
        if (sv[i] == '\n') {
            return StringView(sv.data, i);
        }
    }
    return sv;
};

inline StringView get_until_char(StringView sv, char c) {
    for (size_t i = 0; i < sv.size; i++) {
        if (sv[i] == c && sv[i] != '\\') {
            return StringView(sv.data, i+1);
        }
    }
    return sv;
}
