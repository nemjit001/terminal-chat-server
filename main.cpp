#include <iostream>
#include <thread>
#include <chrono>

#include "Server.h"

int main(int argc, char **argv)
{
    std::cout << "Starting server..." << std::endl;
    Server *server = new Server();
    std::cout << "Server started!" << std::endl;

    while(!server->isStopped() && server->step() == 0)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    std::cout << "Stopping server..." << std::endl;
    delete server;
    return 0;
}
