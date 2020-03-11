#pragma once

#include "jsoncpp/json/json.h"
#include "room.h"
#include "socket_func.h"
#include "thread_pool.h"


#include <vector>

class GobangServer {
public:
    GobangServer();

public:
    bool start(int port);
    void stop();

private:
    Room* createRoom();

    bool parseJsonMsg(const Json::Value& root, SocketFD fd);

    bool processMsgTypeCmd(const Json::Value& root, SocketFD fd);

    bool processCreateRoom(const Json::Value& root, SocketFD fd);
    bool processJoinRoom(const Json::Value& root, SocketFD fd);
    bool processWatchRoom(const Json::Value& root, SocketFD fd);
    bool processDeleteRoom(const Json::Value& root, SocketFD fd);

    Room* getRoomById(int id);

private:
    ThreadPool pool;

    std::vector<Room*> rooms;
    std::vector<int> roomsId;

    SocketFD socketfd = 0;

    bool isRunning = true;
};
