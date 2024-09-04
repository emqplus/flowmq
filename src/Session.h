#ifndef SESSION_H
#define SESSION_H
#pragma once

#include <string>
#include <queue>
#include <functional>
#include "Message.h"
#include "MQTT.h"
#include "Broker.h"

namespace MQTT {

class Broker;

class Session {
public:
    Session(Broker* broker, const std::string& clientId, bool cleanStart = true);
    ~Session();

    const std::string& getClientId() const { return clientId; }
    bool isConnected() const { return connected; }

    void connect();
    void disconnect();
    void discard();
    //void resume();
    ReasonCode publish(uint16_t packetId,const Message& message);
    void subscribe(const std::string& topic, SubscriptionOptions& options);
    void unsubscribe(const std::string& topic);
    void puback(uint16_t packetId);
    ReasonCode pubrec(uint16_t packetId);
    ReasonCode pubrel(uint16_t packetId);
    void pubcomp(uint16_t packetId);
    void setDeliverCallback(std::function<void(const Message&, uint16_t, QoS)> callback);
    void setDisconnectCallback(std::function<void()> callback);
    void deliver(const std::string& topic, const Message& message);

private:
    std::string clientId;
    bool connected = false;
    bool cleanStart = true; 
    uint16_t packetId = 1;
    std::set<uint16_t> awaitingPubrel;
    std::map<std::string, SubscriptionOptions> subscriptions;
    std::map<uint16_t, std::shared_ptr<Message>> inflightMessages;       
    std::queue<Message*> outgoingMessages;
    std::function<void(const Message&, uint16_t, QoS)> onDeliver;
    std::function<void()> onDisconnect;
    Broker* broker;
    uint16_t nextPacketId();
};

}

#endif
