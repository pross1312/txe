#ifndef Editor_HPP
#define Editor_HPP

#include <string>
#include <vector>
#include "Split.hpp"
using namespace std;

struct Editor {
    int cx, cy;
    string buffer;

    Editor(): cx{0}, cy{0}, buffer{} {}

    inline void append(const char *text, size_t size) { buffer.append(text, size); cx += size; }
    inline void append(char c)                        { buffer.push_back(c);       cx ++;      }
    inline void pop()                                 { buffer.pop_back();         cx --;      }

    inline Split lines() {
        string_view buffer_view(buffer.data(), buffer.size());
        return split(buffer_view, '\n');
    }
    inline Split split(string_view sv, char c) { return Split(sv, c); }
};

#endif // EDITOR_HPP
