#ifndef LISTENER_H
#define LISTENER_H
#pragma once

#include <netinet/in.h>

namespace MQTT {

class Listener {
    int port;
    int sockfd;
    struct sockaddr_in address;
public:
    Listener(int port);
    ~Listener();

    void start();
    int acceptConnection();
    void stop();
};

}

#endif