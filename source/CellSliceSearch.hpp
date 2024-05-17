#ifndef SLICE_SEARCH_HPP
#define SLICE_SEARCH_HPP
#include "Editor.h"

#include <string>
#include <cassert>

class CellSliceSearch {
public:
    class Iterator {
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = CellSlice;
        using pointer = CellSlice*;
        using reference = CellSlice&;

        value_type operator*() { return ptr; }
        pointer operator->() { return &ptr; }

        Iterator(const reference data, std::string_view str): data(data), ptr(CellSlice{.data = data.data, .size = str.size()}), str(str) {}
        Iterator() = default;

        Iterator& operator++() {
            size_t str_len = str.size();
            size_t i = ptr.data - data.data + str_len;
            for (; i < data.size - str_len; i++) {
                size_t j = 0;
                for (; j < str_len && data.data[i+j].c == str[j]; j++);
                if (j == str_len) {
                    ptr.data = data.data + i;
                    ptr.size = str_len;
                    return *this;
                }
            }
            ptr.data = data.data + data.size;
            ptr.size = str_len;
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
            size_t i = ptr.data - data.data;
            if (i >= str_len) {
                for (int idx = i - str_len; idx >= 0; idx--) {
                    size_t j = 0;
                    for (; j < str_len && data.data[idx+j].c == str[j]; j++);
                    if (j == str_len) {
                        ptr.data = data.data + idx;
                        ptr.size = str_len;
                        return *this;
                    }
                }
            }
            ptr.data = data.data;
            ptr.size = str_len;
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
        friend CellSliceSearch;
        Iterator(value_type data, value_type ptr, std::string_view str): data(data), ptr(ptr), str(str) {}
        value_type data;
        value_type ptr;
        std::string_view str;
    };

    CellSliceSearch(CellSlice slice, std::string_view str): slice(slice), str(str) {}
    CellSliceSearch() = default;

    Iterator begin() { return ++Iterator(slice, str); }
    Iterator end() { return Iterator(slice, CellSlice{.data = slice.data + slice.size, .size = str.size()}, str); }
private:
    CellSlice slice;
    std::string_view str;
};

#endif // SLICE_SEARCH_HPP
