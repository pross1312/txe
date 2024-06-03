#ifndef SLICE_SEARCH_HPP
#define SLICE_SEARCH_HPP
#include "Editor.h"

#include <string>
#include <cassert>

class Searcher {
public:
    class Iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = StringView;
        using pointer = StringView*;
        using reference = StringView&;

        value_type operator*() { return ptr; }
        pointer operator->() { return &ptr; }

        Iterator(const reference data, StringView str): data(data), ptr(data.data, 0), str(str) {}
        Iterator() = default;

        Iterator& operator++() {
            size_t str_len = str.size;
            size_t i = ptr.data - data.data + ptr.size;
            for (; i < data.size - str_len; i++) {
                if (StringView(data.data + i, str_len) == str) {
                    ptr = StringView(data.data + i, str_len);
                    return *this;
                }
            }
            ptr = StringView(data.data + data.size, 0);
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        Iterator& operator--() {
            // assert(ptr.data != data.data + data.size && "Can't search back from end");
            size_t str_len = str.size;
            size_t i = ptr.data - data.data;
            if (i >= str_len) {
                for (int idx = i - str_len; idx >= 0; idx--) {
                    size_t j = 0;
                    for (; j < str_len && data[idx+j] == str[j]; j++);
                    if (j == str_len) {
                        ptr = StringView(data.data + idx, str_len);
                        return *this;
                    }
                }
            }
            ptr = StringView(data.data, str_len);
            return *this;
        }

        Iterator operator--(int) {
            Iterator tmp = *this;
            --(*this);
            return tmp;
        }

        friend bool operator==(const Iterator& a, const Iterator& b) {
            return a.ptr.data == b.ptr.data && a.ptr.size == b.ptr.size;
        }
        friend bool operator!=(const Iterator& a, const Iterator& b) {
            return !(a == b);
        }

    private:
        friend Searcher;
        Iterator(value_type data, value_type ptr, StringView str): data(data), ptr(ptr), str(str) {}
        value_type data;
        value_type ptr;
        StringView str;
    };

    Searcher(StringView slice, StringView str): slice(slice), str(str) {}
    Searcher(const std::string_view& slice, const std::string_view& str): slice(slice), str(str) {}
    Searcher() = default;

    Iterator begin() { return ++Iterator(slice, str); }
    Iterator end() { return Iterator(slice, StringView(slice.data + slice.size, 0), str); }
private:
    StringView slice;
    StringView str;
};

#endif // SLICE_SEARCH_HPP
