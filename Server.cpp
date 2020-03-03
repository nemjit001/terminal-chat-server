#include "Server.h"

Server::Server()
{
    sock_init();
    this->interval.tv_sec = 1;
    this->interval.tv_usec = 0;

    memset(&this->connectedSockets, 0, sizeof(this->connectedSockets));
    FD_ZERO(&this->connectedSockets);

    this->stop = false;
    this->socketBuffer = new CircularLineBuffer;
    this->inputBuffer = new CircularLineBuffer;
}

Server::~Server()
{
    sock_close(this->serverSocket);
    sock_close(this->clientSocket);
    sock_quit();

    this->stopThreads();

    delete this->socketBuffer;
    delete this->inputBuffer;
}

void Server::setup(const char *address, int port)
{
    sockaddr_in service;
    this->serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (!sock_valid(this->serverSocket)) this->setStopped(true);

    service.sin_family = AF_INET;
    service.sin_addr.s_addr = inet_addr(address);
    service.sin_port = htons(port);

    if (bind(this->serverSocket, (SOCKADDR *) & service, sizeof (service)) != 0) this->setStopped(true);

    if (listen(this->serverSocket, SOMAXCONN) == SOCKET_ERROR) this->setStopped(true);

    this->startThreads();
}

int Server::step()
{
    std::cout << this->socketBuffer->read();

    std::string userInput = this->inputBuffer->read();

    if (userInput.compare("!quit\n") == 0) return -1;

    if (userInput.compare("!count\n") == 0)
    {
        std::cout << "Connected Clients: " << this->connectedSockets.fd_count << std::endl;
    }

    return 0;
}

bool Server::isStopped()
{
    return this->stop;
}

void Server::setStopped(bool val)
{
    this->stop = val;
}

int Server::listenOnServerSocket()
{
    this->clientSocket = accept(this->serverSocket, NULL, NULL);

    if (!sock_valid(this->clientSocket)) return -1;

    FD_SET(this->clientSocket, &this->connectedSockets);

    return 0;
}

int Server::readFromClientSockets()
{
    fd_set rfds = this->connectedSockets;
    int socketCount = select(0, &rfds, NULL, NULL, &this->interval);

    for (int i = 0; i < socketCount; i++)
    {
        char buffer[4096];

        int bytesReceived = recv(rfds.fd_array[i], buffer, 4096, 0);

        if (bytesReceived < 0)
        {
            FD_CLR(rfds.fd_array[i], &this->connectedSockets);
        }

        this->socketBuffer->write(buffer, bytesReceived);
    }

    return 0;
}

int Server::readFromStdin()
{
    std::string userInput;
    getline(std::cin, userInput);

    userInput += '\n';

    if (this->inputBuffer->write(userInput.c_str(), userInput.length())) return userInput.length();

    return -1;
}

