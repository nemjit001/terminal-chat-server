#ifndef CPP_SERVER_H
#define CPP_SERVER_H

#include "socket.h"
#include "CircularLineBuffer.h"
#include <thread>
#include <iostream>
#include <regex>
#include <vector>

#ifndef _WIN32
    #include <pthread.h>
#endif

typedef struct user_t
{
    std::string name;
    SOCKET socket;
} user;

class Server
{
private:
    bool stop;
    fd_set connectedSockets;
    timeval interval;
    SOCKET serverSocket, clientSocket;
    std::thread listenThread, connectionThread, userInputThread;
    CircularLineBuffer *socketBuffer, *inputBuffer;
    std::vector<user> connectedUsers;

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
    void handleReceivedData(SOCKET fd);
    void createUser(std::string name, SOCKET fd);
    void listUsers(SOCKET fd);
    void passMessage(std::string data, SOCKET fd);
    void deleteUser(SOCKET fd);
};

#endif