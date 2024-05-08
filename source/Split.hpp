#ifndef SPLIT_HPP
#define SPLIT_HPP

#include <string>
using namespace std;

struct Split {
private:
    struct Iterator {
        using iterator_category = std::forward_iterator_tag;
        using difference_type = std::ptrdiff_t;
        using value_type = string_view;
        using pointer = string_view;
        using reference = string_view;

        Iterator(string_view content, char c): content(content), c(c) {}

        reference operator*() const { return m_ptr; }
        pointer operator->() { return m_ptr; }

        // Prefix increment
        Iterator& operator++() {
            if (!end) {
                if (m_ptr.data() + m_ptr.size() == content.data() + content.size()) {
                    end = true;
                    m_ptr = string_view(content.data() + content.size(), 0);
                } else {
                    size_t pos = content.find(c, start);
                    m_ptr = content.substr(start, pos - start);
                    start = pos + 1;
                }
            }
            return *this;
        }

        // Postfix increment
        Iterator operator++(int) {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator== (const Iterator& a, const Iterator& b) {
            return a.content == b.content && a.m_ptr == b.m_ptr && a.end == b.end;
        };
        friend bool operator!= (const Iterator& a, const Iterator& b) { return !(a == b); };
        friend Split;
    private:
        Iterator(string_view content): content(content), m_ptr(content.data() + content.size(), 0), end(true) {}
        string_view content, m_ptr;
        char c;
        bool end = false;
        size_t start = 0;
    };

private:
    string_view content;
    char c;
public:
    Split(string_view content, char c): content(content), c(c) {}
    Iterator begin() { return ++Iterator(content, c); }
    Iterator end()   { return Iterator(content); }
};

#endif // SPLIT_HPP
