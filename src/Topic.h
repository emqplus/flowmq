#ifndef TOPIC_H
#define TOPIC_H
#pragma once

#include <string>
#include <vector>

namespace MQTT {
namespace Topic {

// Split an MQTT topic into a vector of topic levels
std::vector<std::string> split(const std::string& topic);

// Join a vector of topic levels into a single MQTT topic
std::string join(const std::vector<std::string>& topicLevels);

// Validate if a topic is well-formed
bool isValid(const std::string& topic);

// Check if a topic matches a topic filter (supporting wildcards + and #)
bool match(const std::string& topic, const std::string& filter);

bool isShared(const std::string& topic);

std::pair<std::string, std::string> splitShared(const std::string& topic);

} 
}

#endif
