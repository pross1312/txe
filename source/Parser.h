#pragma once
#include "Editor.h"

#include <string>

struct Parser {
    class Iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = CellSlice;
        using pointer = CellSlice;
        using reference = CellSlice&;

        Iterator(const reference& content, const std::string_view& chars);

        Iterator& operator*();
        Iterator* operator->();

        // Prefix increment
        Iterator& operator++();

        // Postfix increment
        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator== (const Iterator& a, const Iterator& b) {
            return a.token.data == b.token.data && a.token.size == b.token.size && a.end == b.end;
        };
        friend bool operator!= (const Iterator& a, const Iterator& b) { return !(a == b); };

    public:
        value_type token;
        bool new_line;

    private:
        friend Parser;
        Iterator(value_type data);
        value_type content;
        const std::string_view chars;
        bool end;
    };

    Parser(CellSlice content, const std::string_view& chars);
    Iterator begin();
    Iterator end();

    CellSlice content;
    const std::string_view chars;
};
