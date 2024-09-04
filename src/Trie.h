#ifndef TRIE_H
#define TRIE_H
#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <optional>

namespace MQTT { 

class Trie {
private:
    struct TrieNode {
        std::unordered_map<std::string, std::unique_ptr<TrieNode>> children;
        std::optional<std::string> topicFilter;

        TrieNode() : topicFilter(std::nullopt) {}
    };

    std::unique_ptr<TrieNode> root;

public:
    Trie() : root(std::make_unique<TrieNode>()) {}

    void insert(const std::string& topicFilter);
    void remove(const std::string& topicFilter);
    std::vector<std::string> match(const std::string& topic);
};

}

#endif // TRIE_H
