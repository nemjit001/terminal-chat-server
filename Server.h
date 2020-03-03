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
    fd_set connectedSockets;
    timeval interval;
    SOCKET serverSocket, clientSocket;
    std::thread listenThread, connectionThread, userInputThread;
    CircularLineBuffer *socketBuffer, *inputBuffer;

    inline void startThreads()
    {
        this->listenThread = std::thread(&Server::listenOnServerSocketThread, this);
        this->connectionThread = std::thread(&Server::readFromClientSocketsThread, this);
        this->userInputThread = std::thread(&Server::readFromStdinThread, this);
    }

    inline void stopThreads()
    {
        this->listenThread.join();
        this->connectionThread.join();
        this->userInputThread.join();
    }

    inline void listenOnServerSocketThread()
    {
        while(!this->isStopped())
        {
            auto res = this->listenOnServerSocket();
            if (res < 0)
            {
                this->setStopped(true);
            }
        }
    }

    inline void readFromClientSocketsThread()
    {
        while(!this->isStopped())
        {
            auto res = this->readFromClientSockets();
            if (res < 0)
            {
                this->setStopped(true);
            }
        }
    }

    inline void readFromStdinThread()
    {
        while(!this->isStopped())
        {
            auto res = this->readFromStdin();
            if (res < 0)
            {
                this->setStopped(true);
            }
        }
    } 

public:
    Server();
    ~Server();
    int step();
    void setup(const char *address, int port);
    bool isStopped();
    void setStopped(bool val);
    int readFromClientSockets();
    int listenOnServerSocket();
    int readFromStdin();
};

#endif