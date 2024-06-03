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
        using value_type = std::string_view;
        using pointer = std::string_view*;
        using reference = std::string_view&;

        value_type operator*() { return ptr; }
        pointer operator->() { return &ptr; }

        Iterator(const reference data, std::string_view str): data(data), ptr(data.data(), str.size()), str(str) {}
        Iterator() = default;

        Iterator& operator++() {
            size_t str_len = str.size();
            size_t i = ptr.data() - data.data() + str_len;
            for (; i < data.size() - str_len; i++) {
                if (std::string_view(data.data() + i, str_len) == str) {
                    ptr = std::string_view(data.data() + i, str_len);
                    return *this;
                }
            }
            ptr = std::string_view(data.data() + data.size(), str_len);
            return *this;
        }

        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        Iterator& operator--() {
            // assert(ptr.data != data.data + data.size && "Can't search back from end");
            size_t str_len = str.size();
            size_t i = ptr.data() - data.data();
            if (i >= str_len) {
                for (int idx = i - str_len; idx >= 0; idx--) {
                    size_t j = 0;
                    for (; j < str_len && data[idx+j] == str[j]; j++);
                    if (j == str_len) {
                        ptr = std::string_view(data.data() + idx, str_len);
                        return *this;
                    }
                }
            }
            ptr = std::string_view(data.data(), str_len);
            return *this;
        }

        Iterator operator--(int) {
            Iterator tmp = *this;
            --(*this);
            return tmp;
        }

        friend bool operator==(const Iterator& a, const Iterator& b) {
            return a.ptr.data() == b.ptr.data() && a.ptr.size() == b.ptr.size();
        }
        friend bool operator!=(const Iterator& a, const Iterator& b) {
            return !(a == b);
        }

    private:
        friend Searcher;
        Iterator(value_type data, value_type ptr, std::string_view str): data(data), ptr(ptr), str(str) {}
        value_type data;
        value_type ptr;
        std::string_view str;
    };

    Searcher(std::string_view slice, std::string_view str): slice(slice), str(str) {}
    Searcher() = default;

    Iterator begin() { return ++Iterator(slice, str); }
    Iterator end() { return Iterator(slice, std::string_view(slice.data() + slice.size(), str.size()), str); }
private:
    std::string_view slice;
    std::string_view str;
};

#endif // SLICE_SEARCH_HPP
