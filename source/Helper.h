#pragma once
#include <raylib.h>
#include <string>

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
