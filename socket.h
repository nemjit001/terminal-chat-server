#ifndef CPP_SOCKET_H
#define CPP_SOCKET_H

#ifdef _WIN32
    #ifndef _WIN32_WINNT
        #define _WIN32_WINNT 0x0A00
    #endif
    #include <winsock2.h>
    #include <Ws2tcpip.h>
#else
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <arpa/inet.h>
    #include <netdb.h>
    #include <unistd.h>
    #include <errno.h>
    typedef int SOCKET;
    #define INVALID_SOCKET (SOCKET (~0))
    #define SOCKET_ERROR (-1)
#endif

int sock_init();
bool sock_valid(SOCKET socket);
int sock_error_code();
int sock_close(SOCKET sock);
int sock_quit();

#endif