#include <gtest/gtest.h>
#include "../src/Frame.h"
#include <vector>
#include <stdexcept>

namespace MQTT {

class FrameTest : public ::testing::Test 
{
protected:
    Frame *frame = new Frame();
};

TEST_F(FrameTest, ParseConnectPacket) 
{
    std::vector<uint8_t> payload = {
        0x00, 0x04, 'M', 'Q', 'T', 'T',  // Protocol name
        0x05,                            // Protocol version
        0b00000010,                      // Connect flags (10000010)
                                         // Username flag: 0
                                         // Password flag: 0
                                         // Will retain: 0
                                         // Will QoS: 00
                                         // Will flag: 0
                                         // Clean start: 1
                                         // Reserved: 0
        0x00, 0x3C,                      // Keep alive
        0x00,                            // Properties length
        0x00, 0x03, 'a', 'b', 'c'        // Client ID
    };

    auto packet = frame->parseConnect(payload.data(), payload.size());

    EXPECT_EQ(packet.protocolName, "MQTT");
    EXPECT_EQ(packet.protocolVersion, Version::MQTT5);
    EXPECT_TRUE(packet.cleanStart);
    EXPECT_FALSE(packet.willFlag);
    EXPECT_EQ(packet.willQos, QoS::QOS_0);
    EXPECT_FALSE(packet.willRetain);
    EXPECT_EQ(packet.keepAlive, 60);
    EXPECT_EQ(packet.clientId, "abc");
}

TEST_F(FrameTest, ParsePublishPacket) 
{
    FixedHeader header;
    header.type = PacketType::PUBLISH;
    header.dup = false;
    header.qos = MQTT::QoS::QOS_1;
    header.retain = true;

    std::vector<uint8_t> payload = {
        0x00, 0x05, 't', 'o', 'p', 'i', 'c',  // Topic name
        0x00, 0x0A,                           // Packet identifier
        0x00,                                 // Properties length
        'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'  // Payload
    };

    auto packet = frame->parsePublish(header, payload.data(), payload.size());

    EXPECT_EQ(packet.topicName, "topic");
    EXPECT_EQ(packet.packetId, 10);
    EXPECT_EQ(packet.payload, std::vector<uint8_t>({'H', 'e', 'l', 'l', 'o', ' ', 'W', 'o', 'r', 'l', 'd'}));
}

TEST_F(FrameTest, ParseSubscribePacket) 
{
    std::vector<uint8_t> payload = {
        0x00, 0x0A,                           // Packet identifier
        0x00,                                 // Properties length
        0x00, 0x05, 't', 'o', 'p', 'i', 'c',  // Topic filter
        0x01                                  // QoS
    };

    auto packet = frame->parseSubscribe(payload.data(), payload.size());

    EXPECT_EQ(packet.packetId, 10);
    ASSERT_EQ(packet.subscriptions.size(), 1);
    EXPECT_EQ(packet.subscriptions[0].first, "topic");
    EXPECT_EQ(packet.subscriptions[0].second.maximumQos, QoS::QOS_1);
}

TEST_F(FrameTest, ParseUnsubscribePacket) 
{
    std::vector<uint8_t> payload = {
        0x00, 0x0B,                           // Packet identifier
        0x00,                                 // Properties length
        0x00, 0x05, 't', 'o', 'p', 'i', 'c'   // Topic filter
    };

    auto packet = frame->parseUnsubscribe(payload.data(), payload.size());

    EXPECT_EQ(packet.packetId, 11);
    ASSERT_EQ(packet.topicFilters.size(), 1);
    EXPECT_EQ(packet.topicFilters[0], "topic");
}

TEST_F(FrameTest, ParsePingReqPacket) 
{
    std::vector<uint8_t> bytes = {0xC0, 0x00};
    auto packet = frame->parsePingreq(bytes.data(), bytes.size());

    EXPECT_EQ(packet.type, PacketType::PINGREQ);
}

TEST_F(FrameTest, ParseDisconnectPacket) 
{
    std::vector<uint8_t> payload = {
        0x00,  // Reason Code (Normal disconnection)
        0x00   // Properties length
    };

    auto packet = frame->parseDisconnect(payload.data(), payload.size());

    EXPECT_EQ(packet.type, PacketType::DISCONNECT);
    EXPECT_EQ(packet.reasonCode, ReasonCode::SUCCESS);
}

TEST_F(FrameTest, ParseAuthPacket) 
{
    std::vector<uint8_t> payload = {
        0x18,  // Reason Code (Continue authentication)
        0x00   // Properties length
    };

    auto packet = frame->parseAuth(payload.data(), payload.size());

    EXPECT_EQ(packet.type, PacketType::AUTH);
    EXPECT_EQ(packet.reasonCode, ReasonCode::CONTINUE_AUTHENTICATION);
}

TEST_F(FrameTest, SerializeConnectPacket) 
{
    ConnectPacket packet; 
    packet.type = PacketType::CONNECT;
    packet.protocolName = "MQTT";
    packet.protocolVersion = Version::MQTT5;
    packet.cleanStart = true;
    packet.keepAlive = 60;
    packet.clientId = "testclient";

    auto serialized = frame->serializeConnect(packet);

    ASSERT_EQ(serialized.size(), 24);
    EXPECT_EQ(serialized[0], 0x10);  // CONNECT packet type
    EXPECT_EQ(serialized[1], 22);    // Remaining length
    // ... Add more specific checks for the serialized packet
}

TEST_F(FrameTest, SerializePublishPacket) 
{
    PublishPacket packet;
    packet.type = PacketType::PUBLISH;
    packet.dup = false;
    packet.qos = MQTT::QoS::QOS_1;
    packet.retain = true;
    packet.topicName = "test/topic";
    packet.packetId = 1234;
    packet.payload = {'t', 'e', 's', 't'};

    auto serialized = frame->serializePublish(packet);

    ASSERT_EQ(serialized.size(), 21);
    EXPECT_EQ(serialized[0], 0x33);  // PUBLISH packet type with QoS 1 and retain
    EXPECT_EQ(serialized[1], 19);    // Remaining length
    // ... Add more specific checks for the serialized packet
}

TEST_F(FrameTest, DecodeRemainingLength) 
{
    std::vector<uint8_t> input = {0x7F};
    auto [length, bytes]= frame->decodeRemainingLength(input.data(), input.size());
    EXPECT_EQ(length, 127);
    EXPECT_EQ(bytes, 1);

    input = {0x80, 0x01};
    std::tie(length, bytes) = frame->decodeRemainingLength(input.data(), input.size());
    EXPECT_EQ(length, 128);
    EXPECT_EQ(bytes, 2);

    input = {0xFF, 0xFF, 0xFF, 0x7F};
    std::tie(length, bytes) = frame->decodeRemainingLength(input.data(), input.size());
    EXPECT_EQ(length, 268435455);
    EXPECT_EQ(bytes, 4);
}

TEST_F(FrameTest, EncodeRemainingLength) 
{
    auto encoded = frame->encodeRemainingLength(127);
    EXPECT_EQ(encoded, std::vector<uint8_t>({0x7F}));

    encoded = frame->encodeRemainingLength(128);
    EXPECT_EQ(encoded, std::vector<uint8_t>({0x80, 0x01}));

    encoded = frame->encodeRemainingLength(268435455); 
    EXPECT_EQ(encoded, std::vector<uint8_t>({0xFF, 0xFF, 0xFF, 0x7F}));
}

}