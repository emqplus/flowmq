#ifndef MQTT_H
#define MQTT_H  
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <sstream>

namespace MQTT {

// Protocol version
enum class Version : uint8_t { 
    NONE = 0,
    MQTT31 = 3,
    MQTT311 = 4,
    MQTT5 = 5
};

bool isValideProtocol(std::string protoName, Version protoVersion);

// QoS levels
enum class QoS : uint8_t {
    QOS_0 = 0,
    AT_MOST_ONCE = 0,
    QOS_1 = 1,
    AT_LEAST_ONCE = 1,
    QOS_2 = 2,
    EXACTLY_ONCE = 2
};

// Packet types
enum class PacketType : uint8_t {
    RESERVED = 0,
    CONNECT = 1,
    CONNACK = 2,
    PUBLISH = 3,
    PUBACK = 4,
    PUBREC = 5,
    PUBREL = 6,
    PUBCOMP = 7,
    SUBSCRIBE = 8,
    SUBACK = 9,
    UNSUBSCRIBE = 10,
    UNSUBACK = 11,
    PINGREQ = 12,
    PINGRESP = 13,
    DISCONNECT = 14,
    AUTH = 15
};

// Reason codes
enum class ReasonCode : uint8_t {
    SUCCESS = 0x00,
    NORMAL_DISCONNECTION = 0x00,
    GRANTED_QOS_0 = 0x00,
    GRANTED_QOS_1 = 0x01,
    GRANTED_QOS_2 = 0x02,
    DISCONNECT_WITH_WILL_MESSAGE = 0x04,
    NO_MATCHING_SUBSCRIBERS = 0x10,
    NO_SUBSCRIPTION_EXISTED = 0x11,
    CONTINUE_AUTHENTICATION = 0x18,
    REAUTHENTICATE = 0x19,
    UNSPECIFIED_ERROR = 0x80,
    MALFORMED_PACKET = 0x81,
    PROTOCOL_ERROR = 0x82,
    IMPLEMENTATION_SPECIFIC_ERROR = 0x83,
    UNSUPPORTED_PROTOCOL_VERSION = 0x84,
    CLIENT_IDENTIFIER_NOT_VALID = 0x85,
    BAD_USERNAME_OR_PASSWORD = 0x86,
    NOT_AUTHORIZED = 0x87,
    SERVER_UNAVAILABLE = 0x88,
    SERVER_BUSY = 0x89,
    BANNED = 0x8A,
    SERVER_SHUTTING_DOWN = 0x8B,
    BAD_AUTHENTICATION_METHOD = 0x8C,
    KEEP_ALIVE_TIMEOUT = 0x8D,
    SESSION_TAKEN_OVER = 0x8E,
    TOPIC_FILTER_INVALID = 0x8F,
    TOPIC_NAME_INVALID = 0x90,
    PACKET_IDENTIFIER_IN_USE = 0x91,
    PACKET_IDENTIFIER_NOT_FOUND = 0x92,
    RECEIVE_MAXIMUM_EXCEEDED = 0x93,
    TOPIC_ALIAS_INVALID = 0x94,
    PACKET_TOO_LARGE = 0x95,
    MESSAGE_RATE_TOO_HIGH = 0x96,
    QUOTA_EXCEEDED = 0x97,
    ADMINISTRATIVE_ACTION = 0x98,
    PAYLOAD_FORMAT_INVALID = 0x99,
    RETAIN_NOT_SUPPORTED = 0x9A,
    QOS_NOT_SUPPORTED = 0x9B,
    USE_ANOTHER_SERVER = 0x9C,
    SERVER_MOVED = 0x9D,
    SHARED_SUBSCRIPTIONS_NOT_SUPPORTED = 0x9E,
    CONNECTION_RATE_EXCEEDED = 0x9F,
    MAXIMUM_CONNECT_TIME = 0xA0,
    SUBSCRIPTION_IDENTIFIERS_NOT_SUPPORTED = 0xA1,
    WILDCARD_SUBSCRIPTIONS_NOT_SUPPORTED = 0xA2
};

// Connect return code in MQTT v3.1.1
enum class ConnectReturnCode {
    CONNECTION_ACCEPTED = 0x00,
    UNACCEPTABLE_PROTOCOL_VERSION = 0x01,
    IDENTIFIER_REJECTED = 0x02,
    SERVER_UNAVAILABLE = 0x03,
    BAD_USERNAME_OR_PASSWORD = 0x04,
    NOT_AUTHORIZED = 0x05
}; 

// Property identifiers
enum class PropertyID : uint8_t {
    PAYLOAD_FORMAT_INDICATOR = 0x01,
    MESSAGE_EXPIRY_INTERVAL = 0x02,
    CONTENT_TYPE = 0x03,
    RESPONSE_TOPIC = 0x08,
    CORRELATION_DATA = 0x09,
    SUBSCRIPTION_IDENTIFIER = 0x0B,
    SESSION_EXPIRY_INTERVAL = 0x11,
    ASSIGNED_CLIENT_IDENTIFIER = 0x12,
    SERVER_KEEP_ALIVE = 0x13,
    AUTHENTICATION_METHOD = 0x15,
    AUTHENTICATION_DATA = 0x16,
    REQUEST_PROBLEM_INFORMATION = 0x17,
    WILL_DELAY_INTERVAL = 0x18,
    REQUEST_RESPONSE_INFORMATION = 0x19,
    RESPONSE_INFORMATION = 0x1A,
    SERVER_REFERENCE = 0x1C,
    REASON_STRING = 0x1F,
    RECEIVE_MAXIMUM = 0x21,
    TOPIC_ALIAS_MAXIMUM = 0x22,
    TOPIC_ALIAS = 0x23,
    MAXIMUM_QOS = 0x24,
    RETAIN_AVAILABLE = 0x25,
    USER_PROPERTY = 0x26,
    MAXIMUM_PACKET_SIZE = 0x27,
    WILDCARD_SUBSCRIPTION_AVAILABLE = 0x28,
    SUBSCRIPTION_IDENTIFIER_AVAILABLE = 0x29,
    SHARED_SUBSCRIPTION_AVAILABLE = 0x2A
};

using UserProperties = std::map<std::string, std::string>;

using PropertyValue = std::variant<bool, uint8_t, uint16_t, uint32_t, std::string, UserProperties>;

using Properties = std::map<PropertyID, PropertyValue>;

// MQTT FixedHeader
struct FixedHeader {
    PacketType type : 4;
    bool dup : 1;
    QoS qos : 2;
    bool retain : 1;
    FixedHeader() = default;
    FixedHeader(uint8_t header) {
        type = static_cast<PacketType>((header >> 4) & 0x0F);
        dup = (header & 0x08) != 0;
        qos = static_cast<QoS>((header >> 1) & 0x03);
        retain = (header & 0x01) != 0;  
    }
    bool isValid() {
        if (type == PacketType::RESERVED) {
            return false;
        }
        // Check if the packet type is valid
        if (static_cast<uint8_t>(type) > 15) {
            return false;
        }
        // Check if QoS is valid (0, 1, or 2)
        if (qos > QoS::QOS_2) {
            return false;
        }
        // Check specific rules for different packet types
        switch (type) {
        case PacketType::PUBLISH:
            // PUBLISH can have any combination of flags
            return true;
        case PacketType::PUBREL:
        case PacketType::SUBSCRIBE:
        case PacketType::UNSUBSCRIBE:
            // These packets must have QoS = 1, and dup and retain must be false
            return (qos == QoS::QOS_1) && !dup && !retain;
        default:
            // For all other packet types, dup, QoS, and retain must be 0
            return (qos == QoS::QOS_0) && !dup && !retain;
        }
    }
};

// MQTT Packet (base class)
struct Packet {
    PacketType type;
    Properties properties;
    
    std::optional<PropertyValue> getProperty(PropertyID id);
    void setProperty(PropertyID id, PropertyValue value);
    std::optional<std::string> getUserProperty(const std::string &key);
    void setUserProperty(const std::string &key, const std::string &value);

    Packet() = default;
    explicit Packet(PacketType t) : type(t) {}
    virtual ~Packet() = default;
    virtual std::string toString() const = 0;
};

struct ConnectFlags {
    bool username : 1;
    bool password : 1;
    bool willRetain : 1;
    QoS willQoS : 2;
    bool willFlag : 1;
    bool cleanStart : 1;
    uint8_t _reserved : 1;
    ConnectFlags() = default;
    ConnectFlags(uint8_t flags) {
        username = (flags & 0x80) != 0;
        password = (flags & 0x40) != 0;
        willRetain = (flags & 0x20) != 0;
        willQoS = static_cast<QoS>((flags >> 3) & 0x03);
        willFlag = (flags & 0x04) != 0;
        cleanStart = (flags & 0x02) != 0;
        _reserved = flags & 0x01;
    }
};

struct ConnectPacket : Packet {
    std::string protocolName;
    Version protocolVersion;
    std::string clientId;
    uint16_t keepAlive;
    bool cleanStart;
    bool isBridge = false;
    std::optional<std::string> username;
    std::optional<std::string> password;
    bool willFlag;
    QoS willQos;
    bool willRetain;
    std::optional<std::string> willTopic;
    std::optional<std::string> willMsg;
    std::optional<Properties> willProperties;
    ConnectPacket() : Packet(PacketType::CONNECT) {};
    ~ConnectPacket() {}
    bool isValidProtocol() {
        if (protocolName == "MQTT" && (protocolVersion == Version::MQTT5 || protocolVersion == Version::MQTT311)) {
            return true;
        }
        if (protocolName == "MQIsdp" && protocolVersion == Version::MQTT31) {
            return true;
        }
        return false;
    }
    bool isValidClientId() { return true; };
    std::string toString() const override {
        std::stringstream ss;
        ss << "Connect{" << "protocolName=" << protocolName << ", protocolVersion=" << static_cast<int>(protocolVersion) 
            << ", clientId=" << clientId << ", keepAlive=" << keepAlive << ", cleanStart=" << cleanStart
            << ", username=" << (username ? *username : "null") << ", password=" << (password ? *password : "null") << "}";
        return ss.str();
    }
};

struct ConnackPacket : Packet {
    bool sessionPresent;
    ReasonCode reasonCode;
    ConnackPacket() = default;
    ConnackPacket(bool sessionPresent, ReasonCode reasonCode)
        : sessionPresent(sessionPresent), reasonCode(reasonCode) {}
    ConnackPacket(PacketType type, bool sessionPresent, ReasonCode reasonCode)
        : Packet(type), sessionPresent(sessionPresent), reasonCode(reasonCode) {}
    ~ConnackPacket() = default; 
    std::string toString() const override {
        return "Connack{sessionPresent=" + std::to_string(sessionPresent) + ", reasonCode=" + std::to_string(static_cast<int>(reasonCode)) + "}";
    }
};

struct PublishPacket : Packet {
    std::string topicName;
    QoS qos = QoS::QOS_0;
    bool dup = false;
    bool retain = false;
    uint16_t packetId;  // Only for QoS > 0
    std::vector<uint8_t> payload;
    PublishPacket() : Packet(PacketType::PUBLISH) {};
    PublishPacket(const std::string& topic, const std::vector<uint8_t>& payload, QoS qos = QoS::QOS_0, bool retain = false, uint16_t packetId = 0)
        : Packet(PacketType::PUBLISH), topicName(topic), payload(payload), qos(qos), retain(retain), packetId(packetId) {}
    ~PublishPacket() = default;
    std::string toString() const override {
        return "Publish{topicName=" + topicName + ", qos=" + std::to_string(static_cast<int>(qos)) + ", packetId=" + std::to_string(packetId) + "}";
    }
};

struct PubackPacket : Packet {
    uint16_t packetId;
    ReasonCode reasonCode;
    PubackPacket() 
        : Packet(PacketType::PUBACK), packetId(0), reasonCode(ReasonCode::SUCCESS) {}
    PubackPacket(uint16_t id)
        : Packet(PacketType::PUBACK), packetId(id), reasonCode(ReasonCode::SUCCESS) {}
    PubackPacket(uint16_t id, ReasonCode code)
        : Packet(PacketType::PUBACK), packetId(id), reasonCode(code) {}
    ~PubackPacket() = default;
    std::string toString() const override {
        return "Puback{packetId=" + std::to_string(packetId) + "}";
    }
};

struct PubrecPacket : Packet {
    uint16_t packetId;
    ReasonCode reasonCode;
    PubrecPacket() 
        : Packet(PacketType::PUBREC), packetId(0), reasonCode(ReasonCode::SUCCESS) {}
    PubrecPacket(uint16_t id)
        : Packet(PacketType::PUBREC), packetId(id), reasonCode(ReasonCode::SUCCESS) {}
    PubrecPacket(uint16_t id, ReasonCode code)
        : Packet(PacketType::PUBREC), packetId(id), reasonCode(code) {}
    ~PubrecPacket() = default;
    std::string toString() const override {
        return "Pubrec{packetId=" + std::to_string(packetId) + "}";
    }
};

struct PubrelPacket : Packet {
    uint16_t packetId;
    ReasonCode reasonCode;
    PubrelPacket() 
        : Packet(PacketType::PUBREL), packetId(0), reasonCode(ReasonCode::SUCCESS) {}
    PubrelPacket(uint16_t id)
        : Packet(PacketType::PUBREL), packetId(id), reasonCode(ReasonCode::SUCCESS) {}
    PubrelPacket(uint16_t id, ReasonCode code)
        : Packet(PacketType::PUBREL), packetId(id), reasonCode(code) {}
    ~PubrelPacket() = default;
    std::string toString() const override {
        return "Pubrel{packetId=" + std::to_string(packetId) + "}";
    }
};

struct PubcompPacket : Packet {
    uint16_t packetId;
    ReasonCode reasonCode;
    PubcompPacket() 
        : Packet(PacketType::PUBCOMP), packetId(0), reasonCode(ReasonCode::SUCCESS) {}
    PubcompPacket(uint16_t id)
        : Packet(PacketType::PUBCOMP), packetId(id), reasonCode(ReasonCode::SUCCESS) {}
    PubcompPacket(uint16_t id, ReasonCode code)
        : Packet(PacketType::PUBCOMP), packetId(id), reasonCode(code) {}
    ~PubcompPacket() = default;
    std::string toString() const override {
        return "Pubcomp{packetId=" + std::to_string(packetId) + "}";
    }
};

enum class RetainHandling {
    SEND_RETAINED_MESSAGES_AT_SUBSCRIBE,
    SEND_RETAINED_MESSAGES_AT_SUBSCRIBE_IF_NEW,
    DO_NOT_SEND_RETAINED_MESSAGES
};

struct SubscriptionOptions {
    QoS maximumQos;
    bool noLocal;
    bool retainAsPublished;
    RetainHandling retainHandling;
};

struct SubscribePacket : Packet {
    uint16_t packetId;
    std::vector<std::pair<std::string, SubscriptionOptions>> subscriptions;
    SubscribePacket() : Packet(PacketType::SUBSCRIBE) {};
    ~SubscribePacket() = default;
    std::string toString() const override {
        return "Subscribe{packetId=" + std::to_string(packetId) + "}";
    }
};

struct SubackPacket : Packet {
    uint16_t packetId;
    std::vector<ReasonCode> reasonCodes;
    SubackPacket() : Packet(PacketType::SUBACK) {};
    SubackPacket(uint16_t id) : Packet(PacketType::SUBACK), packetId(id) {}
    ~SubackPacket() = default;
    std::string toString() const override {
        return "Suback{packetId=" + std::to_string(packetId) + "}";    
    }
};

struct UnsubscribePacket : Packet {
    uint16_t packetId;
    std::vector<std::string> topicFilters;
    UnsubscribePacket() : Packet(PacketType::UNSUBSCRIBE) {};
    explicit UnsubscribePacket(uint16_t id) : Packet(PacketType::UNSUBSCRIBE), packetId(id) {}
    ~UnsubscribePacket() = default;
    std::string toString() const override {
        return "Unsubscribe{packetId=" + std::to_string(packetId) + "}";
    }
};

struct UnsubackPacket : Packet {
    uint16_t packetId;
    std::vector<ReasonCode> reasonCodes;
    UnsubackPacket() : Packet(PacketType::UNSUBACK) {};
    explicit UnsubackPacket(uint16_t id) : Packet(PacketType::UNSUBACK), packetId(id) {}
    ~UnsubackPacket() = default;
    std::string toString() const override {
        return "Unsuback{packetId=" + std::to_string(packetId) + "}";
    }   
};

struct DisconnectPacket : Packet {
    ReasonCode reasonCode;
    Properties properties;
    DisconnectPacket() : Packet(PacketType::DISCONNECT), reasonCode(ReasonCode::NORMAL_DISCONNECTION) {}
    explicit DisconnectPacket(ReasonCode rc) : Packet(PacketType::DISCONNECT), reasonCode(rc) {}
    ~DisconnectPacket() = default;
    std::string toString() const override {
        return "Disconnect{reasonCode=" + std::to_string(static_cast<int>(reasonCode)) + "}";
    }
};

struct AuthPacket : Packet {
    ReasonCode reasonCode;
    std::string authMethod;
    std::vector<uint8_t> authData;
    AuthPacket() : Packet(PacketType::AUTH), reasonCode(ReasonCode::SUCCESS) {}
    AuthPacket(ReasonCode rc) : Packet(PacketType::AUTH), reasonCode(rc) {}
    ~AuthPacket() = default;
    std::string toString() const override {
        return "Auth{reasonCode=" + std::to_string(static_cast<int>(reasonCode)) + "}";
    }
};

struct PingreqPacket : Packet {
    PingreqPacket() : Packet(PacketType::PINGREQ) {}
    ~PingreqPacket() = default;
    std::string toString() const override {
        return "Pingreq{}";
    }
};

struct PingrespPacket : Packet {
    PingrespPacket() : Packet(PacketType::PINGRESP) {}
    ~PingrespPacket() = default;
    std::string toString() const override {
        return "Pingresp{}";
    }
};

} // namespace MQTT

#endif
