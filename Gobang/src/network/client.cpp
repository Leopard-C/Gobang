#include "client.h"

#include <cstring>
#include <stdlib.h>

#include <QDebug>

#ifdef WIN32
    #include <winsock2.h>
    #include <windows.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <unistd.h>
    #include <arpa/inet.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
#endif


Client::~Client() {
}

// Just for win32
//
bool Client::InitNetwork() {
#ifdef WIN32
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA wsaData;
    return WSAStartup(sockVersion, &wsaData) == 0;
#endif
}

void Client::disconnect() {
    if (socketfd > 0) {
        //this->closesocket();
        shutdown(socketfd, 2);
        this->closesocket();
        qDebug() << "disconnectd";
    }
}

void Client::closesocket() {
#ifdef WIN32
    ::closesocket(socketfd);
#else
    close(socketfd);
#endif
}

bool Client::connectServer(const char* servAddrStr, int port) {
    if ((socketfd = socket(AF_INET, SOCK_STREAM, 0)) <= 0)
        return false;

    struct sockaddr_in servAddr;
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    servAddr.sin_addr.s_addr = inet_addr(servAddrStr);

    if (connect(socketfd, (struct sockaddr*)&servAddr, sizeof(servAddr)) < 0) {
        this->closesocket();
        return false;
    }

    serverIp = servAddrStr;
    serverPort = port;
    return true;
}

bool Client::sendJsonMsg(const Json::Value &json) {
    return sendJsonMsg(Json::FastWriter().write(json));
}

bool Client::sendJsonMsg(const std::string& msg) {
    if (msg.empty())
        return false;

    int len = msg.length();
    std::string msgSend;
    msgSend.reserve(12 + len);
    msgSend += "length:";
    msgSend += std::to_string(len);
    if (len < 10)
        msgSend += "   ";
    else if (len < 100)
        msgSend += "  ";
    else if (len < 1000)
        msgSend += " ";
    msgSend += "\n";
    msgSend += msg;

    qDebug() << "Sending Msg: ";
    qDebug() << "**************************************";
    qDebug() << QString::fromStdString(msgSend);
    qDebug() << "**************************************";

    send(socketfd, msgSend.c_str(), len + 12, 0);
    return true;
}


int Client::recvJsonMsg(Json::Value& root) {
    char temp[8] = { 0 };
    char lengthStr[5] = { 0 };
    char msgLengthInfo[13] = { 0 };

    int n = recv(socketfd, msgLengthInfo, 12, 0);
    if (n <= 0)
        return -1;
    if (n < 12)
        return 0;

    memcpy(temp, msgLengthInfo, 7);
    memcpy(lengthStr, msgLengthInfo+7, 4);
    temp[7] = 0;
    if (strcmp(temp, "length:") != 0)
        return 0;

    int msgLength = 0;
    sscanf(lengthStr, "%d", &msgLength);

    char* jsonMsg = new char[msgLength+1];
    jsonMsg[msgLength] = 0;
    n = recv(socketfd, jsonMsg, msgLength, 0);

    bool ret = 0;

    do {
        if (n <= 0) {
            ret = -1;
            break;
        }
        if (n < msgLength) {
            break;
        }

        Json::Reader reader;
        if (!reader.parse(jsonMsg, root))
            break;

        ret = 1;
    } while (false);

    if (ret == 1) {
        qDebug() << "Recving Msg: ";
        qDebug() << "**************************************";
        qDebug() << QString(jsonMsg);
        qDebug() << "**************************************";
    }

    delete[] jsonMsg;
    return ret;
}
