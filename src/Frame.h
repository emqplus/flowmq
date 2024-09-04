#ifndef FRAME_H
#define FRAME_H
#pragma once

#include "MQTT.h"

namespace MQTT {

class Frame {
    Version version = Version::MQTT5;
    Properties doParseProperties(const uint8_t *buffer, size_t length);
public:
    static constexpr size_t MAX_MULTIPLIER = 128 * 128 * 128;
    static constexpr size_t MAX_LENGTH = 268435455; 
    Frame() = default;
    Frame(Version version) : version(version) {}

    void setVersion(Version version) { this->version = version; }

    // Parse MQTT packets
    std::shared_ptr<Packet> parse(const uint8_t *buffer, size_t length);

    // Parse the remaining length field of an MQTT packet
    std::pair<size_t, size_t> decodeRemainingLength(const uint8_t *buffer, size_t length);
    std::pair<size_t, size_t> decodeVariableByteInteger(const uint8_t *buffer, size_t length);
    std::string parseString(const uint8_t* buffer, size_t length);
    uint16_t parsePacketId(const uint8_t* buffer, size_t length);

    // Parse specific packet types
    ConnectPacket parseConnect(const uint8_t *buffer, size_t length);
    ConnackPacket parseConnack(const uint8_t *buffer, size_t length); 
    PublishPacket parsePublish(const FixedHeader &header, const uint8_t *buffer, size_t length);
    PubackPacket parsePuback(const uint8_t *buffer, size_t length);
    PubrecPacket parsePubrec(const uint8_t *buffer, size_t length);
    PubrelPacket parsePubrel(const uint8_t *buffer, size_t length);
    PubcompPacket parsePubcomp(const uint8_t *buffer, size_t length);
    SubscribePacket parseSubscribe(const uint8_t *buffer, size_t length);
    SubackPacket parseSuback(const uint8_t *buffer, size_t length);
    UnsubscribePacket parseUnsubscribe(const uint8_t *buffer, size_t length);
    UnsubackPacket parseUnsuback(const uint8_t *buffer, size_t length);
    PingreqPacket parsePingreq(const uint8_t *buffer, size_t length);
    PingrespPacket parsePingresp(const uint8_t *buffer, size_t length);
    DisconnectPacket parseDisconnect(const uint8_t *buffer, size_t length);
    AuthPacket parseAuth(const uint8_t *buffer, size_t length);

    // Parse properties
    std::pair<Properties, size_t> parseProperties(const uint8_t *buffer, size_t length);

    // Serialize MQTT packets
    std::vector<uint8_t> serialize(const Packet &packet);
    // Serialize the remaining length field
    std::vector<uint8_t> encodeRemainingLength(size_t length);
    // Serialize specific packet types
    std::vector<uint8_t> serializeConnect(const ConnectPacket &packet);
    std::vector<uint8_t> serializeConnack(const ConnackPacket &packet);
    std::vector<uint8_t> serializePublish(const PublishPacket &packet);
    std::vector<uint8_t> serializePuback(const PubackPacket &packet);
    std::vector<uint8_t> serializePubrec(const PubrecPacket &packet);
    std::vector<uint8_t> serializePubrel(const PubrelPacket &packet);
    std::vector<uint8_t> serializePubcomp(const PubcompPacket &packet);
    std::vector<uint8_t> serializeSubscribe(const SubscribePacket &packet);
    std::vector<uint8_t> serializeSuback(const SubackPacket &packet);
    std::vector<uint8_t> serializeUnsubscribe(const UnsubscribePacket &packet);
    std::vector<uint8_t> serializeUnsuback(const UnsubackPacket &packet);
    std::vector<uint8_t> serializePingreq(const PingreqPacket &packet);
    std::vector<uint8_t> serializePingresp(const PingrespPacket &packet);
    std::vector<uint8_t> serializeDisconnect(const DisconnectPacket &packet);
    std::vector<uint8_t> serializeAuth(const AuthPacket &packet);

    // Serialize properties
    std::vector<uint8_t> serializeProperties(const Properties &properties);
};
}

#endif // FRAME_H