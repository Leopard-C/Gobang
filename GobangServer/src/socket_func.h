#pragma once

#include <stdlib.h>

#ifdef _WIN32
    #include <winsock2.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
    #define SocketFD unsigned long long
#else
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #define SocketFD int
#endif


#include "jsoncpp/json/json.h"


bool sendJsonMsg(const std::string& msg, SocketFD fd);
bool sendJsonMsg(const Json::Value& jsonMsg, SocketFD fd);

int recvJsonMsg(Json::Value& root, SocketFD fd);

void closeSocket(SocketFD fd);
