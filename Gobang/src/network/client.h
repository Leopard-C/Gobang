#ifndef CLIENT_H
#define CLIENT_H

#include <string>
#include <json/json.h>

class Client {
public:
    Client() {}
    ~Client();
public:
    static bool InitNetwork();

    bool connectServer(const char* servAddr, int port);
    void closesocket();
    void disconnect();

    bool sendJsonMsg(const Json::Value& json);
    bool sendJsonMsg(const std::string& msg);
    int recvJsonMsg(Json::Value& root);

    const char* getServerIp() const { return serverIp.c_str(); }
    int getServerPort() const { return serverPort; }

private:

#ifdef WIN32
    unsigned long long socketfd = 0;
#else
    int socketfd = 0;
#endif
    std::string serverIp;
    int serverPort;
};

#endif // CLIENT_H

