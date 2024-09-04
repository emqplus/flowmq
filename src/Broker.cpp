#include "Broker.h"
#include "Topic.h"
#include "Session.h"
#include <iostream>
#include <thread>
#include <random>

namespace MQTT {

Broker::Broker() : trie(std::make_unique<Trie>()) {}

void Broker::insertSession(const std::string &clientId, Session* session) {
    sessions[clientId] = session;
}

Session* Broker::findSession(const std::string &clientId) const {
    auto it = sessions.find(clientId);
    if (it != sessions.end()) {
        return it->second;
    }
    return nullptr;
}

void Broker::removeSession(const std::string &clientId) {
    sessions.erase(clientId);
}

void Broker::removeSession(Session* session) {
    sessions.erase(session->getClientId());
}   

int Broker::getConnectedClients() const {
    return sessions.size();
}

bool Broker::isSubscribed(const std::string &clientId, const std::string &topicFilter) const {
    auto it = subscriptions.find(topicFilter);
    if (it != subscriptions.end()) {
        return it->second.find(clientId) != it->second.end();
    }
    return false;
}

std::set<std::string> Broker::getSubscriptions(const std::string &topicFilter) {
    std::set<std::string> clientIds;
    if (subscriptions.find(topicFilter) != subscriptions.end()) {
        clientIds = subscriptions[topicFilter];
    }
    return clientIds;
}

void Broker::subscribe(const std::string &clientId, const std::string &topicFilter) {
    std::set<std::string> clientIds;
    if (subscriptions.find(topicFilter) == subscriptions.end()) {
        trie->insert(topicFilter);
    } else {
        clientIds = subscriptions[topicFilter];
    }
    clientIds.insert(clientId);
    subscriptions[topicFilter] = clientIds;
}

void Broker::unsubscribe(const std::string &clientId, const std::string &topicFilter) {
    std::set<std::string> clientIds;
    if (subscriptions.find(topicFilter) == subscriptions.end()) {
        return;
    }

    clientIds = subscriptions[topicFilter];
    clientIds.erase(clientId);

    if (clientIds.empty()) {
        trie->remove(topicFilter);
        subscriptions.erase(topicFilter);
    } else {
        subscriptions[topicFilter] = clientIds;
    }
}

void Broker::sharedSubscribe(const std::string &clientId, const std::string &topicFilter, const std::string &group) {
    printf("Broker::sharedSubscribe: %s %s %s\n", clientId.c_str(), topicFilter.c_str(), group.c_str());
    SharedSubscription sharedSubscription(clientId, topicFilter, group);
    if (sharedSubscriptions.find(topicFilter) == sharedSubscriptions.end()) {
        sharedSubscriptions[topicFilter] = std::vector<SharedSubscription>();
    }   
    sharedSubscriptions[topicFilter].push_back(sharedSubscription);
    trie->insert(topicFilter);
}

void Broker::sharedUnsubscribe(const std::string &clientId, const std::string &topicFilter, const std::string &group) {   
    SharedSubscription sharedSubscription(clientId, topicFilter, group);
    if (sharedSubscriptions.find(topicFilter) == sharedSubscriptions.end()) {
        return;
    }
    auto& sharedSubs = sharedSubscriptions[topicFilter];
    auto it = std::find(sharedSubs.begin(), sharedSubs.end(), sharedSubscription);
    if (it != sharedSubs.end()) {
        sharedSubs.erase(it);
    }
    if (sharedSubs.empty()) {
        sharedSubscriptions.erase(topicFilter);
    }
}

static int randIdx(int size) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, size - 1);
    return dis(gen);
}

void Broker::publish(const Message &message) {
    std::vector<std::string> topicFilters = trie->match(message.topic);
    for (const auto &topicFilter : topicFilters) {
        std::set<std::string> clientIds = getSubscriptions(topicFilter);
        for (const auto &clientId : clientIds) {
            Session* session = findSession(clientId);
            if (session) {   
                session->deliver(topicFilter, message);
            }
        }
        if (!sharedSubscriptions.empty()) {
            auto it = sharedSubscriptions.find(topicFilter);
            if (it != sharedSubscriptions.end()) {
                auto idx = randIdx(it->second.size());
                SharedSubscription sharedSubscription = it->second.at(idx);
                Session* session = findSession(sharedSubscription.clientId);
                if (session) {
                    session->deliver(topicFilter, message);
                }
            }
        }
    }
}

} // namespace MQTT
