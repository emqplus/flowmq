#pragma once
#include <string>
#include "Listener.h"
#include "Broker.h"

namespace MQTT {

class Broker;
class Server {
public:
    explicit Server(int port);
    ~Server();

    void start();
    void handleClient(int clientSocket);
    void closeClient(int clientSocket);
    void stop();

private:
    void run();
    Broker* broker;
    std::unique_ptr<Listener> listener;
    
};

} // namespace MQTT
