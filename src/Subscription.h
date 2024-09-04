#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H
#pragma once

#include <string>

namespace MQTT {

struct Subscription {
    std::string clientId;
    std::string topicFilter;

    Subscription(const std::string& cId, const std::string& tf)
        : clientId(cId), topicFilter(tf) {}

    bool operator==(const Subscription& other) const {
        return clientId == other.clientId && topicFilter == other.topicFilter;
    }

    std::string toString() const {
        return "Subscription(" + clientId + ", " + topicFilter + ")";
    }
};

struct SharedSubscription {
    std::string clientId;
    std::string topicFilter;
    std::string group;

    SharedSubscription(const std::string& cId, const std::string& tf, const std::string& g)
        : clientId(cId), topicFilter(tf), group(g) {}

    bool operator==(const SharedSubscription& other) const
    {   
        return clientId == other.clientId && topicFilter == other.topicFilter && group == other.group;
    }

    std::string toString() const {
        return "SharedSubscription(" + clientId + ", " + topicFilter + ", " + group + ")";
    }
};

} // namespace MQTT

#endif