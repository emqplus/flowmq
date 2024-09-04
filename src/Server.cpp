#include "Topic.h"
#include "Server.h"
#include "Connection.h"
#include <stdexcept>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <thread>

namespace MQTT {
Server::Server(int port) {
    listener = std::make_unique<Listener>(port);
    broker = new Broker();
}

Server::~Server() {
    delete broker;
}

void Server::start() {   
    listener->start();
    std::cout << "MQTT Server is started" << std::endl;
    run();
}

void Server::run() {
    while (true) {
        int clientSocket = listener->acceptConnection();
        if (clientSocket < 0) {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        std::thread clientThread(&Server::handleClient, this, clientSocket);
        clientThread.detach();
    }
}

void Server::handleClient(int clientSocket) {
    Connection connection(clientSocket, broker);
    connection.run();
    closeClient(clientSocket);
}

void Server::closeClient(int clientSocket) {
    close(clientSocket);
}

void Server::stop() {
    if (listener) {
        listener->stop();
    }

    // Signal all client threads to stop
    // This is a placeholder and should be implemented based on how you manage client threads

    std::cout << "Server stopped" << std::endl;
}

}
