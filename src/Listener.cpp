#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include "Listener.h"

namespace MQTT {

Listener::Listener(int port) : port{port}, sockfd{-1} {
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
}

Listener::~Listener() {
    if (sockfd != -1) close(sockfd); 
}

void Listener::start() {
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create socket");
    }  

    if (bind(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        throw std::runtime_error("Failed to bind to a port");
    }
    if (listen(sockfd, 10) < 0) {
        throw std::runtime_error("Failed to listen on socket");
    }
    std::cout << "MQTT listener on port " << port << std::endl;
}

int Listener::acceptConnection() {
    sockaddr_in clientAddr{};
    socklen_t clientAddrLen = sizeof(clientAddr);
    return accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrLen);
}

void Listener::stop() {
    if (sockfd != -1) {
        close(sockfd);
        sockfd = -1;
    }
}

}