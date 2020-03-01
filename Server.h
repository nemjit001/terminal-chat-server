#ifndef CPP_SERVER_H
#define CPP_SERVER_H

#include "socket.h"
#include "CircularLineBuffer.h"
#include <thread>
#include <iostream>

#ifndef _WIN32
    #include <pthread.h>
#endif

class Server
{
private:
    bool stop;
    SOCKET serverSocket;
    CircularLineBuffer *buffer;
public:
    Server();
    ~Server();
    int step();
    bool isStopped();
    void setStopped(bool val);
};

#endif