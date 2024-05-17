#include "Parser.h"

Parser::Iterator::Iterator(const reference& content, const std::string_view& chars): token(content), content(content), chars(chars), end(false) {}
Parser::Iterator::Iterator(value_type data): token(CellSlice{.data = data.data + data.size, .size = 0}), content(data), end(true) {}

Parser::Iterator& Parser::Iterator::operator*() {
    return *this;
}
Parser::Iterator* Parser::Iterator::operator->() {
    return this;
}

Parser::Iterator& Parser::Iterator::operator++() {
    new_line = false;
    if (!end) {
        if (content.size == 0) {
            end = true;
            token.data = content.data + content.size;
            token.size = 0;
        } else {
            size_t i = 0;
            for (; i < content.size; i++) {
                if (chars.find(content.data[i].c) != std::string_view::npos) {
                    token.data = content.data;
                    token.size = i;
                    // skip all space
                    do {
                        if (content.data[i].c == '\n') new_line = true;
                        i++;
                    } while (i < content.size && chars.find(content.data[i].c) != std::string_view::npos);
                    break;
                }
            }
            content.data += i;
            content.size -= i;
        }
    }
    return *this;
}

Parser::Parser(CellSlice content, const std::string_view& chars): content(content), chars(chars) {}

Parser::Iterator Parser::begin() {
    return ++Parser::Iterator(content, chars);
}
Parser::Iterator Parser::end() {
    return Iterator(content);
}
