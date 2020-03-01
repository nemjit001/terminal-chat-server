#include "Server.h"

Server::Server()
{
    this->stop = false;
    this->buffer = new CircularLineBuffer;

    sock_init();
}

Server::~Server()
{
    sock_close(this->serverSocket);
    sock_quit();
}


int Server::step()
{
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
