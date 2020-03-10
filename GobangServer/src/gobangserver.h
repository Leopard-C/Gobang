#pragma once

#include "thread_pool.h"
#include <vector>

#include "jsoncpp/json/json.h"
#include "room.h"


class GobangServer {
public:
    GobangServer();

public:
    bool start(int port);
    void stop();

private:
    Room* createRoom();

    bool parseJsonMsg(const Json::Value& root, int fd);

    bool processMsgTypeCmd(const Json::Value& root, int fd);

    bool processCreateRoom(const Json::Value& root, int fd);
    bool processJoinRoom(const Json::Value& root, int fd);
    bool processWatchRoom(const Json::Value& root, int fd);
    bool processDeleteRoom(const Json::Value& root, int fd);

    Room* getRoomById(int id);

private:
    ThreadPool pool;

    std::vector<Room*> rooms;
    std::vector<int> roomsId;

    int socketfd = 0;

    bool isRunning = true;
};
