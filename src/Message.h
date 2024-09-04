#ifndef MESSAGE_H
#define MESSAGE_H
#pragma once

#include <string>
#include <vector>
#include "MQTT.h"

namespace MQTT {

struct Message {
    std::string topic;
    std::vector<uint8_t> payload;
    QoS qos = QoS::QOS_0;
    bool retain = false;

    Message(const std::string &t, const std::vector<uint8_t> &p, QoS q = QoS::QOS_0, bool r = false)
        : topic(t), payload(std::move(p)), qos(q), retain(r) {}

    // Convenience constructor for string payloads
    Message(const std::string &t, const std::string &p, QoS q = QoS::AT_MOST_ONCE, bool r = false)
        : topic(t), payload(std::vector<uint8_t>(p.begin(), p.end())), qos(q), retain(r) {}

    std::string toString() const;
};

}

#endif