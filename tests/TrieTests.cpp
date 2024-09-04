#include <gtest/gtest.h>
#include "../src/Trie.h"

class TrieTest : public ::testing::Test {
protected:
    MQTT::Trie trie;

    void SetUp() override {
        // Set up code here (if needed)
    }

    void TearDown() override {
        // Clean up code here (if needed)
    }
};

TEST_F(TrieTest, InsertAndMatch) {
    trie.insert("sensor/temperature");
    trie.insert("sensor/humidity");
    trie.insert("sensor/+/value");
    trie.insert("actuator/#");

    auto matches = trie.match("sensor/temperature");
    EXPECT_EQ(matches.size(), 1);
    EXPECT_EQ(matches[0], "sensor/temperature");

    matches = trie.match("sensor/pressure");
    EXPECT_EQ(matches.size(), 0);

    matches = trie.match("sensor/temperature/value");
    EXPECT_EQ(matches.size(), 1);
    EXPECT_EQ(matches[0], "sensor/+/value");

    matches = trie.match("actuator/light/on");
    EXPECT_EQ(matches.size(), 1);
    EXPECT_EQ(matches[0], "actuator/#");
}

TEST_F(TrieTest, Remove) {
    trie.insert("sensor/temperature");
    trie.insert("sensor/humidity");

    trie.remove("sensor/temperature");

    auto matches = trie.match("sensor/temperature");
    EXPECT_EQ(matches.size(), 0);

    matches = trie.match("sensor/humidity");
    EXPECT_EQ(matches.size(), 1);
    EXPECT_EQ(matches[0], "sensor/humidity");
}

// Add more tests as needed
