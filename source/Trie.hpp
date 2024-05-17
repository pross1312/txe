#ifndef TRIE_HPP
#define TRIE_HPP


#include <string>

struct Trie {
    struct Node {
        std::string_view str {};
        Node* children[128] {};
        Node() = default;
        ~Node() {
            for (Node* child : children) delete child;
        }
    };
    Trie(): root(new Node) {}
    ~Trie() { delete root; }
    void insert(const std::string_view& str) {
        Node* ptr = root;
        for (char c : str) {
            if (ptr->children[(int)c] == nullptr) {
                ptr->children[(int)c] = new Node;
            }
            ptr = ptr->children[(int)c];
        }
        ptr->str = str;
    }
    Node* root;
};

#endif // TRIE_HPP
