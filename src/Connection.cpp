#include "Frame.h"
#include "Connection.h"
#include "Session.h"
#include "Broker.h"
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <memory>

namespace MQTT {

Connection::Connection(int sockfd, Broker* broker)
    : sockfd(sockfd), state(State::IDLE), broker(broker) { }

static std::string toHexString(const uint8_t *data, size_t length) {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (size_t i = 0; i < length; ++i) {
        ss << std::setw(2) << static_cast<int>(data[i]);
        if (i % 16 == 15)
            ss << '\n';
        else if (i % 2 == 1)
            ss << ' ';
    }
    return ss.str();
}

void Connection::run() {
    ssize_t bytesRead = 0;
    while ((state != State::DISCONNECTED) && (bytesRead = read(sockfd, buffer, BUFFER_SIZE)) > 0) {
        try {
            std::cout << toHexString(buffer, bytesRead) << std::endl;
            handleIncoming(frame.parse(buffer, bytesRead));
        } catch (const std::exception &e) {
            std::cerr << "Error processing packet: " << e.what() << std::endl;
            state = State::DISCONNECTED;
        }
    }
}

bool Connection::isConnected() const {
    return state == State::CONNECTED;
}

void Connection::handleIncoming(std::shared_ptr<Packet> packet) {
    switch (packet->type) {
    case PacketType::CONNECT:
        handleConnect(std::static_pointer_cast<ConnectPacket>(packet));
        break;
    case PacketType::PUBLISH:
        handlePublish(std::static_pointer_cast<PublishPacket>(packet));
        break;
    case PacketType::PUBACK:
        handlePuback(std::static_pointer_cast<PubackPacket>(packet));
        break;
    case PacketType::PUBREC:
        handlePubrec(std::static_pointer_cast<PubrecPacket>(packet));
        break;
    case PacketType::PUBREL:
        handlePubrel(std::static_pointer_cast<PubrelPacket>(packet));
        break;
    case PacketType::PUBCOMP:
        handlePubcomp(std::static_pointer_cast<PubcompPacket>(packet));
        break;
    case PacketType::SUBSCRIBE:
        handleSubscribe(std::static_pointer_cast<SubscribePacket>(packet));
        break;
    case PacketType::UNSUBSCRIBE:
        handleUnsubscribe(std::static_pointer_cast<UnsubscribePacket>(packet));
        break;
    case PacketType::PINGREQ:
        handlePingreq(std::static_pointer_cast<PingreqPacket>(packet));
        break;
    case PacketType::DISCONNECT:
        handleDisconnect(std::static_pointer_cast<DisconnectPacket>(packet));
        break;
    case PacketType::AUTH:
        handleAuth(std::static_pointer_cast<AuthPacket>(packet));   
        break;
    default:
        throw std::runtime_error("Unhandled packet type");
    }
}

void Connection::handleConnect(std::shared_ptr<ConnectPacket> connect) {
    if(state != State::IDLE) {
        throw std::runtime_error("Bad connect packet");
    }
    frame.setVersion(connect->protocolVersion);
    Session* session;
    if(connect->cleanStart) {
        Session* oldSession = broker->findSession(connect->clientId);
        if(oldSession) {
            oldSession->discard();
        }
        session = new Session(broker, connect->clientId);
    } else {
        Session* oldSession = broker->findSession(connect->clientId);
        if(!oldSession) {
            session = new Session(broker, connect->clientId);
        } else {
            session = oldSession;
        }
    }
    session->setDeliverCallback([this](const Message& message, uint16_t packetId, QoS qos) {
        handleDeliver(message, packetId, qos);
    });
    session->setDisconnectCallback([this]() {
        state = State::DISCONNECTED;
        close(sockfd); //TODO: ????
    });
    session->connect();    
    this->session = std::shared_ptr<Session>(session);
    state = State::CONNECTED;
    printf("New client connected: %s\n", connect->clientId.c_str());
    ConnackPacket connack{PacketType::CONNACK, false, ReasonCode::SUCCESS};
    sendPacket(connack);
}

void Connection::handlePublish(std::shared_ptr<PublishPacket> publish) {
    Message message{publish->topicName, publish->payload, publish->qos, publish->retain};
    ReasonCode reason = session->publish(publish->packetId, message);
    if (publish->qos == QoS::QOS_1) {    
        PubackPacket puback{publish->packetId, reason};
        sendPacket(puback);
    } else if (publish->qos == QoS::QOS_2) {
        PubrecPacket pubrec{publish->packetId, reason};
        sendPacket(pubrec);
    }
}

void Connection::handlePuback(std::shared_ptr<PubackPacket> puback) {
    session->puback(puback->packetId);
}      

void Connection::handlePubrec(std::shared_ptr<PubrecPacket> pubrec) {
    ReasonCode reason = session->pubrec(pubrec->packetId);
    PubrelPacket pubrel{pubrec->packetId, reason};
    sendPacket(pubrel);
}

void Connection::handlePubrel(std::shared_ptr<PubrelPacket> pubrel) {
    ReasonCode reason = session->pubrel(pubrel->packetId);
    PubcompPacket pubcomp{pubrel->packetId, reason};
    sendPacket(pubcomp);
}

void Connection::handlePubcomp(std::shared_ptr<PubcompPacket> pubcomp) {
    session->pubcomp(pubcomp->packetId);
}

void Connection::handleSubscribe(std::shared_ptr<SubscribePacket> subscribe) { 
    printf("handleSubscribe: %d\n", subscribe->packetId);
    // Add subscriptions to the session
    for (const auto& subscription : subscribe->subscriptions) {
        printf("Subscription: %s\n", subscription.first.c_str());
        session->subscribe(subscription.first, const_cast<MQTT::SubscriptionOptions&>(subscription.second));
    }
    SubackPacket suback{subscribe->packetId};    
    printf("Suback packet: %d\n", subscribe->packetId);

    // Accept all subscriptions with QoS 0
    for (size_t i = 0; i < subscribe->subscriptions.size(); ++i) {   
        suback.reasonCodes.push_back(ReasonCode::GRANTED_QOS_0);
    }
    sendPacket(suback);

}

void Connection::handleUnsubscribe(std::shared_ptr<UnsubscribePacket> unsubscribe) {
    for (const auto& topic : unsubscribe->topicFilters) {
        session->unsubscribe(topic);
    }
    UnsubackPacket unsuback{unsubscribe->packetId};
    sendPacket(unsuback);
}

void Connection::handlePingreq(std::shared_ptr<PingreqPacket> pingreq) {
    PingrespPacket pingResp;
    sendPacket(pingResp);
}

void Connection::handleDisconnect(std::shared_ptr<DisconnectPacket> disconnect) {
    state = State::DISCONNECTED;
}   

void Connection::handleAuth(std::shared_ptr<AuthPacket> auth) {
    // Handle AUTH packet
    // TODO: Implement auth logic
    // For now, just send an AUTH packet
    AuthPacket authResp;
    sendPacket(authResp);
}

void Connection::handleDeliver(const Message& message, uint16_t packetId, QoS qos) {
    PublishPacket publish{message.topic, message.payload, qos, message.retain};
    publish.packetId = packetId;
    printf("Deliver message: %s\n", publish.toString().c_str());
    sendPacket(publish);
}

void Connection::sendPacket(Packet& packet) {
    auto data = frame.serialize(packet);
    write(sockfd, data.data(), data.size());
}   
} // namespace MQTT
