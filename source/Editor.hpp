#ifndef Editor_HPP
#define Editor_HPP

#include <string>
#include <vector>
using namespace std;

struct Editor {
    int cx, cy;
    string buffer;
    Editor(): cx{0}, cy{0}, buffer{} {}

    inline void append(const char *text, size_t size) { buffer.append(text, size); }
    inline void append(char c)                        { buffer.push_back(c); }
    inline void pop()                                 { buffer.pop_back(); }

    inline vector<string_view> lines() {
        string_view buffer_view(buffer.data(), buffer.size());
        return split(buffer_view, '\n');
    }
    inline vector<string_view> split(string_view sv, char c) {
        vector<string_view> result;
        size_t pos = 0, start = 0;
        do {
            pos = sv.find(c, start);
            result.push_back(sv.substr(start, pos - start));
            start = pos + 1;
        } while (pos != string_view::npos);
        return result;
    }
};
#endif // EDITOR_HPP
