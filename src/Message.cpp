#include "Message.h"
#include <sstream>

namespace MQTT {

std::string Message::toString() const {
    std::stringstream ss;
    ss << "Topic: " << topic << ", Payload: " << payload.size() << ", QoS: " << static_cast<int>(qos) << ", Retain: " << retain;
    return ss.str();
}

}