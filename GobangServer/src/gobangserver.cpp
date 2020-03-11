#include "gobangserver.h"

#include "api.h"

#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>


GobangServer::GobangServer() :
    pool(10)
{
#ifdef WIN32
        WORD sockVersion = MAKEWORD(2, 2);
        WSADATA wsaData;
        WSAStartup(sockVersion, &wsaData);
#endif

    pool.enqueue([this](){
        while (isRunning) {
            std::cout << "Rooms in use: " << rooms.size() << std::endl;
            for (auto it = rooms.begin(); it != rooms.end(); ) {
                if ((*it)->shouldDelete()) {
                    std::cout << "Deleting room: " << (*it)->getId() << std::endl;                
                    delete (*it);
                    it = rooms.erase(it);
                }
                else {
                    ++it;
                }
            }

            if (rooms.size() == 0)
                std::this_thread::sleep_for(std::chrono::seconds(5));
            else
                std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
}


void GobangServer::stop() {
    std::cout << "\nStopping server" << std::endl;
    shutdown(socketfd, 2);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Closing socket" << std::endl;
    closeSocket(socketfd);
    isRunning = false;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Quit successfully" << std::endl;
}


bool GobangServer::start(int port) {
    socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd <= 0) {
        printf("create socket error: %s(errno: %d)\n", strerror(errno), errno);
        return false;
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    if (bind(socketfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        printf("bind socket error: %s(errno: %d)\n", strerror(errno), errno);
        closeSocket(socketfd);
        return false;
    }

    if (listen(socketfd, 10) == -1) {
        printf("listen socket error: %s(errno: %d)\n", strerror(errno), errno);
        closeSocket(socketfd);
        return false;
    }

    printf("Running...\n");
    
    while (isRunning) {
        SocketFD connectfd = 0;
        if ((connectfd = accept(socketfd, (struct sockaddr*)NULL, NULL)) <= 0) {
            printf("accept socket error %s(errno: %d)\n", strerror(errno), errno);
            continue;
        }

        printf("Accept one connection.\n");

        pool.enqueue([this, connectfd](){
            Json::Value root;
            int ret = recvJsonMsg(root, connectfd);
            std::cout << "Recv Ret: " << ret << std::endl;;

            if (ret == -1) {
                closeSocket(connectfd);
                return;
            }
            else if (ret > 0){
                parseJsonMsg(root, connectfd);
            }
        });
    }

    return true;
}

bool GobangServer::parseJsonMsg(const Json::Value& root, SocketFD fd) {
    if (root["type"].isNull()) {
        return false;
    }

    std::string type = root["type"].asString();
    if (type == "command") {
        return processMsgTypeCmd(root, fd);
    }

    return true;
}

bool GobangServer::processMsgTypeCmd(const Json::Value& root, SocketFD fd) {
    if (root["cmd"].isNull())
        return false;

    std::string cmd = root["cmd"].asString();

    if (cmd == "create_room")
        return processCreateRoom(root, fd);
    if (cmd == "join_room")
        return processJoinRoom(root, fd);
    if (cmd == "watch_room")
        return processWatchRoom(root, fd);

    return false;
}


bool GobangServer::processCreateRoom(const Json::Value& root, SocketFD fd) {
    if (root["room_name"].isNull() || root["player_name"].isNull())
        return false;
    
    Room* room = createRoom();
    rooms.push_back(room);
    room->setName(root["room_name"].asString());
    room->addPlayer(root["player_name"].asString(), fd);

    return API::responseCreateRoom(fd, 0, "OK", room->getId());
}

bool GobangServer::processJoinRoom(const Json::Value& root, SocketFD fd) {
    if (root["room_id"].isNull() || root["player_name"].isNull())
        return false;

    int roomId = root["room_id"].asInt();
    std::string playerName = root["player_name"].asString();
    Room* room = getRoomById(roomId);

    int statusCode = STATUS_ERROR;
    std::string desc, roomName, rivalname;

    do {
        // The room is not exist
        if (!room) {
            desc = "The room is not exist";
            break;
        }

        int numPlayers = room->getNumPlayers();

        // The room is full
        if (numPlayers == 2) {
            desc = "The room is full";
            break;
        }
        else if (numPlayers == 1) {
            Player& player1 = room->getPlayer1();
            // Error: The name of two players is same
            if (player1.name == playerName) {
                desc = "The name of two players cant't be same";
                break;
            }
            roomName = room->getName();
            rivalname = player1.name;
        }
        else {
            desc = "Server Internal Error. Please try to join another room";
            break;
        }

        room->addPlayer(playerName, fd);
        statusCode = STATUS_OK;
    } while (false);

    // response new player
    API::responseJoinRoom(fd, statusCode, desc, roomName, rivalname);

    if (room && statusCode == STATUS_OK)
        // notify old player
        return API::notifyRivalInfo(room->getPlayer1().socketfd, playerName);
    else
        return false;
}

bool GobangServer::processWatchRoom(const Json::Value& root, SocketFD fd) {
    if (root["room_id"].isNull() || root["player_name"].isNull())
        return false;

    int roomId = root["room_id"].asInt();
    std::string playerName = root["player_name"].asString();
    Room* room = getRoomById(roomId);
    if (room) {
        API::responseWatchRoom(fd, STATUS_OK, "", room->getName());
        room->addWatcher(playerName, fd); 
    }
    else{
        API::responseWatchRoom(fd, STATUS_ERROR, "The room is not exist", "");
    }

    return true;
}

bool GobangServer::processDeleteRoom(const Json::Value& root, SocketFD fd) {

    return true;
}


/*****************************************************************************/
/*****************************************************************************/

Room* GobangServer::createRoom() {
    if (rooms.size() > 100)
        return nullptr;

    Room* room = new Room();

    while (true) {
        int randNum = rand() % 9000 + 1000;
        auto iter = std::find(roomsId.begin(), roomsId.end(), randNum);
        if (iter == roomsId.end()) {
            roomsId.push_back(randNum);
            room->setId(randNum);
            break;
        }
    }

    return room;
}

Room* GobangServer::getRoomById(int id) {
    for (int i = 0, size = rooms.size(); i < size; ++i) {
        if (rooms[i]->getId() == id)
            return rooms[i];
    }
    return nullptr;
}


