#include <gtest/gtest.h>
#include "Broker.h"
#include "Session.h"

class BrokerTest : public ::testing::Test
{
protected:
    MQTT::Broker *broker = new MQTT::Broker();
};

TEST_F(BrokerTest, InitializationTest)
{
    EXPECT_NO_THROW(MQTT::Broker());
}

TEST_F(BrokerTest, ConnectClientTest)
{
    MQTT::Session session(broker, "client1");
    session.connect();
    EXPECT_EQ(broker->getConnectedClients(), 1);
}

TEST_F(BrokerTest, DisconnectClientTest)
{
    MQTT::Session session(broker, "client1");
    session.connect();
    session.disconnect();
    EXPECT_EQ(broker->getConnectedClients(), 0);
}

TEST_F(BrokerTest, PublishMessageTest)
{
    //broker.connectClient("subscriber");
    broker->subscribe("subscriber", "test/topic");
    MQTT::Message message("test/topic", "Hello, MQTT!");
    broker->publish(message);
}

TEST_F(BrokerTest, SubscribeTest)
{
    broker->subscribe("client1", "test/topic");
    EXPECT_TRUE(broker->isSubscribed("client1", "test/topic"));
}

TEST_F(BrokerTest, UnsubscribeTest)
{
    broker->subscribe("client1", "test/topic");
    broker->unsubscribe("client1", "test/topic");
    EXPECT_FALSE(broker->isSubscribed("client1", "test/topic"));
}
