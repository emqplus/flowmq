#include "MQTT.h"
#include <sstream>

namespace MQTT {

std::optional<PropertyValue> Packet::getProperty(PropertyID id)
{
    auto it = properties.find(id);
    if (it != properties.end()) {
        return it->second;
    }   
    return std::nullopt;
}

void Packet::setProperty(PropertyID id, PropertyValue value)
{
    properties[id] = value;
}

std::optional<std::string> Packet::getUserProperty(const std::string &key)
{
    auto it = properties.find(PropertyID::USER_PROPERTY);
    if (it != properties.end()) {
        auto userProperties = std::get<UserProperties>(it->second);
        auto userPropertyIt = userProperties.find(key);
        if (userPropertyIt != userProperties.end()) {
            return userPropertyIt->second;
        }
    }
    return std::nullopt;
}

void Packet::setUserProperty(const std::string &key, const std::string &value)
{
    if (properties.find(PropertyID::USER_PROPERTY) == properties.end()) {
        properties[PropertyID::USER_PROPERTY] = UserProperties();
    }
    std::get<UserProperties>(properties[PropertyID::USER_PROPERTY])[key] = value;
}

}
