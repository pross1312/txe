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
            if (pos != string::npos)  {
                pos = content.find(c, start);
                m_ptr = content.substr(start, pos - start);
                start = pos + 1;
            } else {
                m_ptr = string_view(content.data() + content.size(), string_view::npos);
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
            return a.m_ptr.data() == b.m_ptr.data() && a.m_ptr.size() == b.m_ptr.size();
        };
        friend bool operator!= (const Iterator& a, const Iterator& b) { return !(a == b); };
        friend Split;
    private:
        Iterator(string_view content, string_view m_ptr): content(content), m_ptr(m_ptr) {}
        string_view content, m_ptr;
        char c;
        size_t pos = 0, start = 0;
    };

private:
    string_view content;
    char c;
public:
    Split(string_view content, char c): content(content), c(c) {}
    Iterator begin() { return ++Iterator(content, c); }
    Iterator end()   { return Iterator(content, string_view(content.data()+content.size(), string_view::npos)); }
};

#endif // SPLIT_HPP
