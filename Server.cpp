#include "Server.h"

bool exists(std::vector<user> vec, std::string val)
{
    for (int i = 0; i < vec.size(); i++)
    {
        if (vec.at(i).name == val) return true;
    }

    return false;
}

SOCKET get_socket(std::vector<user> vec, std::string val)
{
    for (int i = 0; i < vec.size(); i++)
    {
        if (vec.at(i).name == val) return vec.at(i).socket;
    }

    return -1;
}

std::string get_name(std::vector<user> vec, SOCKET fd)
{
    for (int i = 0; i < vec.size(); i++)
    {
        if (vec.at(i).socket == fd) return vec.at(i).name;
    }

    return "";
}

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

void Server::createUser(std::string name, SOCKET fd)
{
    user newUser;
    newUser.name = name;
    newUser.socket = fd;

    if (exists(this->connectedUsers, newUser.name)) 
    {
        const char *response = "FAIL-TAKEN\n";
        send(fd, response, strlen(response), 0);
    }
    else if (!std::regex_match(newUser.name, std::regex("[A-Za-z0-9]+")))
    {
        const char *response = "FAIL-INVALID\n";
        send(fd, response, strlen(response), 0);
    }
    else
    {
        this->connectedUsers.push_back(newUser);
        std::string response = "HELLO " + newUser.name + "\n";
        send(fd, response.c_str(), response.length(), 0);
    }
}

void Server::listUsers(SOCKET fd)
{
    std::string userList = "WHO-OK ";

    for (user currUser : this->connectedUsers)
    {
        userList += currUser.name + ",";
    }

    userList = userList.substr(0, userList.length() - 1) + "\n";
    send(fd, userList.c_str(), userList.length(), 0);
}

void Server::passMessage(std::string data, SOCKET fd)
{
    std::string receivingUserName, message, response;
    std::string sendingUsername = get_name(this->connectedUsers, fd);
    std::string acknowledge = "SEND-OK\n";

    int startNameIndex = 0, endNameIndex = 0;

    for (int i = 0; i < data.length(); i++)
    {
        if (data.at(i) == ' ')
        {
            if (startNameIndex == 0)
                startNameIndex = i;
            else if (endNameIndex == 0)
                endNameIndex = i;
            else
                break;
        }
    }

    receivingUserName = data.substr(startNameIndex + 1, endNameIndex - startNameIndex - 1);
    message = data.substr(endNameIndex + 1);

    if (!exists(this->connectedUsers, receivingUserName) || receivingUserName.compare(sendingUsername) == 0) 
    {
        acknowledge = "SEND-FAIL\n";
        send(fd, acknowledge.c_str(), acknowledge.length(), 0);
        return;
    }

    response = "DELIVERY " + sendingUsername + " " + message;

    send(get_socket(this->connectedUsers, receivingUserName), response.c_str(), response.length(), 0);
    send(fd, acknowledge.c_str(), acknowledge.length(), 0);
}

void Server::deleteUser(std::string data)
{
    std::string username;

    for (int i = 0; i < data.length(); i++)
    {
        if (data.at(i) == ' ')
        {
            username = data.substr(i + 1, data.length() - i - 2);
            break;
        }
    }

    std::cout << username << std::endl;

    if (!exists(this->connectedUsers, username)) return;

    for (int i = 0; i < this->connectedUsers.size(); i++)
    {
        if (this->connectedUsers.at(i).name == username)
        {
            sock_close(this->connectedUsers.at(i).socket);
            this->connectedUsers.erase(this->connectedUsers.begin() + i);
        }
    }
}

void Server::handleReceivedData(SOCKET fd)
{
    std::string data = this->socketBuffer->read();

    if (std::regex_match(data, std::regex("HELLO-FROM[\\w\\d\\s]*")))
    {
        std::string username = data.substr(11, data.length());
        this->createUser(username.substr(0, username.length() - 1), fd);
    }
    else if (data.substr(0, 4) == "SEND")
    {
        this->passMessage(data, fd);
    }
    else if (std::regex_match(data, std::regex("WHO[\\w\\d\\s]*")))
    {
        this->listUsers(fd);
    }
    else if (std::regex_match(data, std::regex("QUIT[\\w\\d\\s]*")))
    {
        this->deleteUser(data);
    }
    else
    {
        const char *badRequest = "BAD-RQST-HDR\n";
        send(fd, badRequest, strlen(badRequest), 0);
    }
}

int Server::step()
{
    std::string userInput = this->inputBuffer->read();

    if (std::regex_match(userInput, std::regex("!quit[\\w\\d\\s]*")))
    {
        this->setStopped(true);
    }

    if (std::regex_match(userInput, std::regex("!count[\\w\\d\\s]*")))
    {
        std::cout << "Connected Clients: " << this->connectedSockets.fd_count << std::endl;
    }

    if (std::regex_match(userInput, std::regex("!broadcast[\\w\\d\\s]*")))
    {
        std::string msg = "[Server] " + userInput.substr(11, userInput.length());

        for (int i = 0; i < this->connectedSockets.fd_count; i++)
        {
            send(this->connectedSockets.fd_array[i], msg.c_str(), msg.length(), 0);
        }
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
    const char *greeting = "You have been successfully connected to the server!\n";

    this->clientSocket = accept(this->serverSocket, NULL, NULL);

    if (!sock_valid(this->clientSocket)) return -1;

    FD_SET(this->clientSocket, &this->connectedSockets);
    
    send(this->clientSocket, greeting, strlen(greeting), 0);

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

        this->handleReceivedData(rfds.fd_array[i]);
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

