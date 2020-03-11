#pragma once

#include "base.h"
#include <string>


struct Player {
    Player() {}
    Player(const std::string& name, int fd, ChessType type) :
        name(name), socketfd(fd), type(type) {}
    Player(const Player& rhs) {
        name = rhs.name;
        socketfd = rhs.socketfd;
        type = rhs.type;
        prepare = rhs.prepare;
    }
    Player& operator=(const Player& rhs) {
        if (this != &rhs) {
            this->name = rhs.name;
            this->socketfd = rhs.socketfd;
            this->type = rhs.type;
            this->prepare = rhs.prepare;
        }
        return *this;
    }
    std::string name;
    int socketfd = 0;
    ChessType type = CHESS_BLACK;
    bool prepare = false;
};

struct Watcher {
    Watcher(const std::string& name, int fd) :
        name(name), socketfd(fd) {}
    Watcher(const Watcher& rhs) {
        name = rhs.name;
        socketfd = rhs.socketfd;
    }
    std::string name;
    int socketfd;
};

