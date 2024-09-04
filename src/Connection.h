#ifndef CONNECTION_H
#define CONNECTION_H
#pragma once

#include <sys/socket.h>
#include <cstdint>
#include "MQTT.h"
#include "Message.h"
#include <memory>
#include "Frame.h"

namespace MQTT {
class Broker;
class Session;

class Connection {
    enum class State {
        IDLE,
        CONNECTED,
        DISCONNECTED
    };  

public:
    explicit Connection(int sockfd, Broker* broker);
    ~Connection() = default;
    void run();
    bool isConnected() const;

    void handleIncoming(std::shared_ptr<Packet> packet);
    void handleConnect(std::shared_ptr<ConnectPacket> packet);
    void handlePublish(std::shared_ptr<PublishPacket> packet);
    void handlePuback(std::shared_ptr<PubackPacket> packet);
    void handlePubrec(std::shared_ptr<PubrecPacket> packet);
    void handlePubrel(std::shared_ptr<PubrelPacket> packet);
    void handlePubcomp(std::shared_ptr<PubcompPacket> packet);
    void handleSubscribe(std::shared_ptr<SubscribePacket> packet);
    void handleUnsubscribe(std::shared_ptr<UnsubscribePacket> packet);
    void handlePingreq(std::shared_ptr<PingreqPacket> packet);
    void handlePingresp(std::shared_ptr<PingrespPacket> packet);
    void handleDisconnect(std::shared_ptr<DisconnectPacket> packet);
    void handleAuth(std::shared_ptr<AuthPacket> packet);    

    void handleDeliver(const Message& message, uint16_t packetId, QoS qos);
    void sendPacket(Packet &packet);

private:
    int sockfd;
    State state;
    Frame frame;
    std::shared_ptr<Session> session;
    Broker* broker;
    static const int BUFFER_SIZE = 1024;
    uint8_t buffer[BUFFER_SIZE];
};
} // namespace MQTT

#endif
