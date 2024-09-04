#include <gtest/gtest.h>
#include "../src/MQTT.h"
#include <vector>

namespace MQTT {

class MQTTTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Setup code if needed
    }

    void TearDown() override
    {
        // Teardown code if needed
    }
};

TEST_F(MQTTTest, FixedHeaderIsValidTest)
{
    FixedHeader header;

    // Test valid PUBLISH packet
    header.type = PacketType::PUBLISH;
    header.qos = QoS::QOS_1;
    header.dup = true;
    header.retain = true;
    EXPECT_TRUE(header.isValid());

    // Test valid SUBSCRIBE packet
    header.type = PacketType::SUBSCRIBE;
    header.qos = QoS::QOS_1;
    header.dup = false;
    header.retain = false;
    EXPECT_TRUE(header.isValid());

    // Test invalid SUBSCRIBE packet (wrong QoS)
    header.qos = QoS::QOS_0;
    EXPECT_FALSE(header.isValid());

    // Test invalid packet type
    header.type = PacketType::RESERVED;
    EXPECT_FALSE(header.isValid());

    // Test invalid QoS value
    header.type = PacketType::PUBLISH;
    header.qos = static_cast<QoS>(3); // Invalid QoS value
    EXPECT_FALSE(header.isValid());

    // Test valid PINGREQ packet
    header.type = PacketType::PINGREQ;
    header.qos = QoS::QOS_0;
    header.dup = false;
    header.retain = false;
    EXPECT_TRUE(header.isValid());

    // Test invalid PINGREQ packet (non-zero flags)
    header.dup = true;
    EXPECT_FALSE(header.isValid());
}

TEST_F(MQTTTest, ConnectPacketTest)
{
    ConnectPacket connect;
    connect.protocolName = "MQTT";
    connect.protocolVersion = Version::MQTT5;
    connect.clientId = "testClient";
    connect.keepAlive = 60;
    connect.cleanStart = true;

    EXPECT_TRUE(connect.isValidProtocol());
    EXPECT_TRUE(connect.isValidClientId());
    EXPECT_EQ(connect.type, PacketType::CONNECT);
    EXPECT_EQ(connect.toString(), "Connect{protocolName=MQTT, protocolVersion=5, clientId=testClient, keepAlive=60, cleanStart=1, username=null, password=null}");
}

TEST_F(MQTTTest, PublishPacketTest)
{
    std::vector<uint8_t> payload = {0x01, 0x02, 0x03};
    PublishPacket publish("test/topic", payload, QoS::QOS_1, true, 1234);

    EXPECT_EQ(publish.type, PacketType::PUBLISH);
    EXPECT_EQ(publish.topicName, "test/topic");
    EXPECT_EQ(publish.qos, QoS::QOS_1);
    EXPECT_TRUE(publish.retain);
    EXPECT_EQ(publish.packetId, 1234);
    EXPECT_EQ(publish.payload, payload);
    EXPECT_EQ(publish.toString(), "Publish{topicName=test/topic, qos=1, packetId=1234}");
}

TEST_F(MQTTTest, PubackPacketTest)
{
    PubackPacket puback(1234);
    puback.reasonCode = ReasonCode::SUCCESS;

    EXPECT_EQ(puback.type, PacketType::PUBACK);
    EXPECT_EQ(puback.packetId, 1234);
    EXPECT_EQ(puback.reasonCode, ReasonCode::SUCCESS);
    EXPECT_EQ(puback.toString(), "Puback{packetId=1234, reasonCode=0}");
}

TEST_F(MQTTTest, PubrecPacketTest)
{
    PubrecPacket pubrec(2345);
    pubrec.reasonCode = ReasonCode::NO_MATCHING_SUBSCRIBERS;

    EXPECT_EQ(pubrec.type, PacketType::PUBREC);
    EXPECT_EQ(pubrec.packetId, 2345);
    EXPECT_EQ(pubrec.reasonCode, ReasonCode::NO_MATCHING_SUBSCRIBERS);
    EXPECT_EQ(pubrec.toString(), "Pubrec{packetId=2345, reasonCode=16}");
}

TEST_F(MQTTTest, PubrelPacketTest)
{
    PubrelPacket pubrel(3456);
    pubrel.reasonCode = ReasonCode::PACKET_IDENTIFIER_NOT_FOUND;

    EXPECT_EQ(pubrel.type, PacketType::PUBREL);
    EXPECT_EQ(pubrel.packetId, 3456);
    EXPECT_EQ(pubrel.reasonCode, ReasonCode::PACKET_IDENTIFIER_NOT_FOUND);
    EXPECT_EQ(pubrel.toString(), "Pubrel{packetId=3456, reasonCode=146}");
}

TEST_F(MQTTTest, PubcompPacketTest)
{
    PubcompPacket pubcomp(4567);
    pubcomp.reasonCode = ReasonCode::SUCCESS;

    EXPECT_EQ(pubcomp.type, PacketType::PUBCOMP);
    EXPECT_EQ(pubcomp.packetId, 4567);
    EXPECT_EQ(pubcomp.reasonCode, ReasonCode::SUCCESS);
    EXPECT_EQ(pubcomp.toString(), "Pubcomp{packetId=4567, reasonCode=0}");
}

TEST_F(MQTTTest, SubscribePacketTest)
{
    SubscribePacket subscribe;
    SubscriptionOptions subOptions{QoS::QOS_2};
    subscribe.packetId = 5678;
    subscribe.subscriptions.push_back(std::make_pair("test/topic", subOptions));

    EXPECT_EQ(subscribe.type, PacketType::SUBSCRIBE);
    EXPECT_EQ(subscribe.packetId, 5678);
    EXPECT_EQ(subscribe.subscriptions.size(), 1);
    EXPECT_EQ(subscribe.subscriptions.front().second.maximumQos, QoS::QOS_2);
    EXPECT_EQ(subscribe.toString(), "Subscribe{packetId=5678}");
}

TEST_F(MQTTTest, UnsubscribePacketTest)
{
    UnsubscribePacket unsubscribe(9012);
    unsubscribe.topicFilters = {"test/topic1", "test/topic2"};

    EXPECT_EQ(unsubscribe.type, PacketType::UNSUBSCRIBE);
    EXPECT_EQ(unsubscribe.packetId, 9012);
    EXPECT_EQ(unsubscribe.topicFilters.size(), 2);
    EXPECT_EQ(unsubscribe.toString(), "Unsubscribe{packetId=9012}");
}

TEST_F(MQTTTest, PingPacketsTest)
{
    PingreqPacket pingreq;
    PingrespPacket pingresp;

    EXPECT_EQ(pingreq.type, PacketType::PINGREQ);
    EXPECT_EQ(pingreq.toString(), "Pingreq{}");
    EXPECT_EQ(pingresp.type, PacketType::PINGRESP);
    EXPECT_EQ(pingresp.toString(), "Pingresp{}");
}


} // namespace
