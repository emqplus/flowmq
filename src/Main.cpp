
#include "Server.h"
#include <iostream>

int main(int argc, char* argv[]) {
     try {
        MQTT::Server server(1883); // Default MQTT port
        server.start();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}