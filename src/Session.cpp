#include <string>
#include <cstdint>
#include <string>
#include <functional>
#include <iostream>
#include "MQTT.h"
#include "Topic.h"
#include "Session.h"
#include "Broker.h"
#include "Message.h"

namespace MQTT {
Session::Session(Broker *broker, const std::string &clientId, bool cleanStart)
    : broker(broker), clientId(clientId), connected(false), cleanStart(cleanStart) {}

Session::~Session() {
    std::cout << "Session::~Session: " << clientId << std::endl;
    broker->removeSession(this);
}

void Session::connect() {
    connected = true;
    broker->insertSession(clientId, this);
    printf("Session::connect: %s\n", clientId.c_str());
}

void Session::discard() {
    printf("Session::discard: %s\n", clientId.c_str());
    disconnect();
}

void Session::disconnect() {
    broker->removeSession(this);
    connected = false;
    onDisconnect();
}

ReasonCode Session::publish(uint16_t packetId, const Message &message) {
    broker->publish(message);
    if (message.qos == QoS::QOS_2) {
        awaitingPubrel.insert(packetId);
    }
    return ReasonCode::SUCCESS;
}

void Session::puback(uint16_t packetId) {
    inflightMessages.erase(packetId);
}

ReasonCode Session::pubrec(uint16_t packetId) {
    inflightMessages.erase(packetId);
    return ReasonCode::SUCCESS;
}

ReasonCode Session::pubrel(uint16_t packetId) {
    awaitingPubrel.erase(packetId);
    return ReasonCode::SUCCESS;
}

void Session::pubcomp(uint16_t packetId) {
    // broker->pubcomp(packet);
}

void Session::subscribe(const std::string &topicFilter, SubscriptionOptions &options) {
    if (Topic::isShared(topicFilter)) {
        auto [group, realTopicFilter] = Topic::splitShared(topicFilter);
        broker->sharedSubscribe(clientId, realTopicFilter, group);
        subscriptions[realTopicFilter] = options;
    }
    else {
        broker->subscribe(clientId, topicFilter);
        subscriptions[topicFilter] = options;
    }
}

void Session::unsubscribe(const std::string &topicFilter) {
    if (Topic::isShared(topicFilter)) {
        auto [group, realTopicFilter] = Topic::splitShared(topicFilter);
        broker->sharedUnsubscribe(clientId, realTopicFilter, group);
        subscriptions.erase(realTopicFilter);
    } else {
        broker->unsubscribe(clientId, topicFilter);
        subscriptions.erase(topicFilter);
    }
}

void Session::setDeliverCallback(std::function<void(const Message &, uint16_t, QoS)> callback) {
    onDeliver = callback;
}

void Session::deliver(const std::string &topic, const Message &message) {
    printf("session deliver to %s\n", topic.c_str());
    QoS qos = message.qos;
    if (subscriptions.find(topic) != subscriptions.end()) {
        SubscriptionOptions options = subscriptions[topic];
        qos = options.maximumQos;
    }
    if (qos > QoS::QOS_0) {
        packetId = nextPacketId();
        inflightMessages[packetId] = std::make_shared<Message>(message);
    }
    onDeliver(message, packetId, qos);
}

void Session::setDisconnectCallback(std::function<void()> callback) {
    onDisconnect = callback;
}

inline uint16_t Session::nextPacketId() {
    if (packetId++ > UINT16_MAX) {
        packetId = 1;
    }
    return packetId;
}

}