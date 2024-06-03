#ifndef STRINGVIEW_HPP
#define STRINGVIEW_HPP

#include <string>
#include <cstring>

struct StringView {
    const char* data;
    size_t size;
    StringView(): data(nullptr), size(0) {}
    StringView(const std::string_view& str): data(str.data()), size(str.size()) {}
    StringView(const char* str, size_t size): data(str), size(size) {}
    StringView(const char* str): data(str), size(strlen(str)) {}
    bool operator==(const StringView& other) const {
        if (size != other.size) return false;
        return memcmp(data, other.data, size) == 0;
    }
    char operator[](size_t i) const { return data[i]; }
    operator std::string() const { return std::string(std::string_view(data, size)); }
    operator std::string_view() const { return std::string_view(data, size); }
    struct Iterator {
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = char;
        using pointer = value_type*;
        using reference = value_type&;
        value_type operator*() { return ch; }

        Iterator(const StringView& sv, size_t idx = 0): data(sv), idx(idx) { if (idx < data.size) ch = data[idx++]; }
        Iterator() = default;

        Iterator& operator++() {
            if (idx < data.size) ch = data[++idx];
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        Iterator& operator--() {
            if (idx == 0) return *this;
            ch = data[--idx];
            return *this;
        }

        Iterator operator--(int) {
            Iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator==(const Iterator& b) const { return this->data.data == b.data.data && this->data.size == b.data.size && this->idx == b.idx; }
        bool operator!=(const Iterator& b) const { return !(*this == b); }

        const StringView& data;
        char ch;
        size_t idx;
    };
    Iterator begin() { return Iterator(*this); }
    Iterator end() { return Iterator(*this, size); }
};

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

#endif // STRINGVIEW_HPP
