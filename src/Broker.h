#ifndef BROKER_H
#define BROKER_H
#pragma once

#include <set>
#include <map>
#include <unordered_map>
#include <memory>
#include "Trie.h"
#include "Message.h"
#include "Subscription.h"

namespace MQTT {
class Session;
class Broker {
    std::unique_ptr<Trie> trie;
    std::unordered_map<std::string, Session*> sessions;
    std::unordered_map<std::string, std::set<std::string>> subscriptions;
    std::map<std::string, std::vector<SharedSubscription>> sharedSubscriptions;

public:
    explicit Broker();
    ~Broker() = default;
    void insertSession(const std::string &clientId, Session* session);
    Session* findSession(const std::string &clientId) const;
    void removeSession(const std::string &clientId);
    void removeSession(Session* session);

    void subscribe(const std::string &clientId, const std::string &topicFilter);
    void unsubscribe(const std::string &clientId, const std::string &topicFilter);
    void sharedSubscribe(const std::string &clientId, const std::string &topicFilter, const std::string &group);
    void sharedUnsubscribe(const std::string &clientId, const std::string &topicFilter, const std::string &group);
    void publish(const Message &message);

    int getConnectedClients() const;
    bool isSubscribed(const std::string& clientId, const std::string& topicFilter) const;
    std::set<std::string> getSubscriptions(const std::string &topicFilter);
};
}

#endif