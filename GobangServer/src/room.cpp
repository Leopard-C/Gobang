#include "room.h"

#include "api.h"
#include "jsoncpp/json/json.h"

#include <algorithm>
#include <iostream>
#include <string>

#define Log(x) std::cout << (x) << std::endl


Room::Room() :
    pool(MAX_NUM_WATCHERS + 2)
{
    initChessBoard();
    lastChess = { 0, 0, CHESS_NULL };
}

void Room::initChessBoard() {
    for (int row = 0; row < 15; ++row) {
        for (int col = 0; col < 15; ++col) {
            chessPieces[row][col] = 0;
        }
    }
}

void Room::setPiece(int row, int col, ChessType type) {
    if (row < 0 || row > 14 || col < 0 || col > 14)
        return;
    chessPieces[row][col] = type;
}

void Room::addPlayer(const std::string& name, SocketFD fd) {
    if (numPlayers == 0) {
        player1 = Player(name, fd, CHESS_BLACK);
        numPlayers++;
    }
    else {
        player2 = Player(name, fd, reverse(player1.type));
        numPlayers++;
    }

    for (auto& watcher : watchers) {
        API::notifyPlayerInfo(watcher.socketfd, player1.name, player1.type,
                player2.name, player2.type);
    }

    pool.enqueue([this, fd](){
        while (1) {
            Json::Value root;
            int ret = recvJsonMsg(root, fd);
            if (ret == -1) {
                Log("Quit");
                quitPlayer(fd);
                return;
            }
            else if (ret > 0){
                parseJsonMsg(root, fd);
            }
        }
    });
}

void Room::addWatcher(const std::string& name, SocketFD fd) {
    watchers.emplace_back(name, fd);
    API::notifyPlayerInfo(fd, player1.name, player1.type, player2.name, player2.type);
    API::sendChessBoard(fd, this->chessPieces, lastChess);

    pool.enqueue([this, fd](){
        std::cout << "add watcher" << std::endl;
        while (1) {
            Json::Value root;
            int ret = recvJsonMsg(root, fd);
            std::cout << "recv watcher msg" << std::endl;
            if (ret == -1) {
                Log("A watcher quit");
                quitWatcher(fd);
                return;
            }
            else if (ret > 0){
                std::cout << "recv watcher msg" << std::endl;
                if (root["type"].isNull() || root["type"].asString() != "chat" ||
                        root["message"].isNull() || root["sender"].isNull()) {
                    return; 
                }
                for (auto& watcher : watchers) {
                    if (watcher.socketfd != fd) {
                        API::forward(watcher.socketfd, root);
                    }
                }
                if (numPlayers >= 1)
                    API::forward(player1.socketfd, root);
                if (numPlayers == 2)
                    API::forward(player2.socketfd, root);
            }
        }
    });
}

void Room::quitPlayer(SocketFD fd) {
    std::cout << "quit player" << std::endl;
    std::cout << numPlayers << std::endl;
    // won't happen
    if (numPlayers <= 0)
        return;

    std::string quitPlayerName = getPlayer(fd)->name;

    if (fd == player1.socketfd) {
        if (numPlayers == 2)
            player1 = player2;  // player1 should be the master player
    }

    numPlayers--;
    if (numPlayers == 0) {
        flagShouldDelete = true;
    }
    else {
        API::notifyDisconnect(player1.socketfd, quitPlayerName);
        for (auto& watcher : watchers)
            API::notifyDisconnect(watcher.socketfd, quitPlayerName);
        gameStatus = GAME_END;
        lastChess = { 0, 0, CHESS_NULL };
    }

    std::cout << numPlayers << std::endl;
    closeSocket(fd);
}

void Room::quitWatcher(SocketFD fd) {
    std::cout << "quit watcher" << std::endl;
    std::cout << watchers.size() << std::endl;

    for (auto it = watchers.begin(); it != watchers.end(); ++it) {
        if (it->socketfd == fd) {
            watchers.erase(it);
            return;
        }
    }

    closeSocket(fd);
    std::cout << watchers.size() << std::endl;
}

bool Room::parseJsonMsg(const Json::Value& root, SocketFD fd) {
    if (root["type"].isNull())
        return false;
    std::string msgType = root["type"].asString();
    
    if (msgType == "command") {
        if (root["cmd"].isNull())
            return false;
        else
            return processMsgTypeCmd(root, fd);
    }
    if (msgType == "response") {
        if (root["res_cmd"].isNull())
            return false;
        else
            return processMsgTypeResponse(root, fd);
    }
    if (msgType == "chat")
        return processMsgTypeChat(root, fd);
    if (msgType == "notify") {
        if (root["sub_type"].isNull())
            return false;
        else
            return processMsgTypeNotify(root, fd);
    }

    return false;
}

bool Room::processMsgTypeCmd(const Json::Value& root, SocketFD fd) {
    std::string cmd = root["cmd"].asString();
    if (cmd == "prepare")
        return processPrepareGame(root, fd);
    if (cmd == "cancel_prepare")
        return processCancelPrepareGame(root, fd);
    if (cmd == "exchange")
        return processExchangeChessType(root, fd);
    return true;
}

bool Room::processMsgTypeResponse(const Json::Value& root, SocketFD fd) {
    std::string res_cmd = root["res_cmd"].asString();
    if (res_cmd == "prepare") {
        if (root["accept"].isNull())
            return false;
        if (root["accept"].asBool()) {
            ChessType tmp = player1.type;
            player1.type = player2.type;
            player2.type = tmp;
        }
    }
    return API::forward(getRival(fd)->socketfd, root);
}

bool Room::processMsgTypeChat(const Json::Value& root, SocketFD fd) {
    for (auto& watcher : watchers) {
        if (watcher.socketfd != fd) {
            API::forward(watcher.socketfd, root);
        }
    }
    return API::forward(getRival(fd)->socketfd, root); 
}


bool Room::processMsgTypeNotify(const Json::Value& root, SocketFD fd) {
    std::string subType = root["sub_type"].asString();
    if (subType == "new_piece")
        return processNewPiece(root, fd);
    if (subType == "game_over")
        return processGameOver(root, fd);
    if (subType == "rival_info")
        return processNotifyRivalInfo(root, fd);
    return false;
}

bool Room::processNotifyRivalInfo(const Json::Value& root, SocketFD fd) {
    Player* player = getPlayer(fd);
    if (!root["player_name"].isNull())
        player->name = root["player_name"].asString();

    return API::forward(getRival(fd)->socketfd, root);
}

bool Room::processPrepareGame(const Json::Value& root, SocketFD fd) {
    Player* player = getPlayer(fd);

    if (numPlayers == 2) {
        player->prepare = true;
        if (player1.prepare && player2.prepare) {
            gameStatus = GAME_RUNNING;
            initChessBoard();
            lastChess = { 0, 0, CHESS_NULL };
            player1.prepare = false;
            player2.prepare = false;
            API::notifyGameStart(fd);
            for (auto& watcher : watchers)
                API::notifyGameStart(watcher.socketfd);
            return API::notifyGameStart(getRival(fd)->socketfd);
        }
        else {
            gameStatus = GAME_PREPARE;
            API::responsePrepare(fd, STATUS_OK, "OK");
            for (auto& watcher : watchers)
                API::forward(watcher.socketfd, root);
            return API::forward(getRival(fd)->socketfd, root);
        }
    }
    else {
        player->prepare = false;
        std::string desc = "Please wait for the other player to join";
        return API::responsePrepare(fd, STATUS_ERROR, desc);
    }
}

bool Room::processCancelPrepareGame(const Json::Value& root, SocketFD fd) {
    getPlayer(fd)->prepare = false;
    for (auto& watcher : watchers)
        API::notifyGameCancelPrepare(watcher.socketfd);
    return  API::notifyGameCancelPrepare(getRival(fd)->socketfd);
}


bool Room::processNewPiece(const Json::Value& root, SocketFD fd) {
    if (root["row"].isNull() || root["col"].isNull() || root["chess_type"].isNull())
        return false;
    int row = root["row"].asInt();
    int col = root["col"].asInt();
    int chessType = root["chess_type"].asInt();
    setPiece(row, col, ChessType(chessType));
    lastChess = { row, col, chessType };

    for (auto& watcher : watchers) {
        API::notifyNewPiece(watcher.socketfd, row, col, chessType);
    }

    return API::notifyNewPiece(getRival(fd)->socketfd, row, col, chessType);
}

bool Room::processGameOver(const Json::Value& root, SocketFD fd) {
    if (root["game_result"].isNull())
        return false;

    if (root["game_result"].asString() == "win") {
        if (root["chess_type"].isNull())
            return false;
        ChessType winChessType = ChessType(root["chess_type"].asInt());
    }
    else {  //Draw

    }

    gameStatus = GAME_END;
    std::cout << "game over" << std::endl;
    return true;
}

bool Room::processExchangeChessType(const Json::Value& root, SocketFD fd) {
    if (numPlayers != 2)
        return false;

    return API::forward(getRival(fd)->socketfd, root);
}

Player* Room::getPlayer(SocketFD fd) {
    if (player1.socketfd == fd)
        return &player1;
    else if (player2.socketfd == fd)
        return &player2;
    else
        return nullptr;
}

Player* Room::getRival(int my_fd) {
    if (player1.socketfd == my_fd)
        return &player2;
    else if (player2.socketfd == my_fd)
        return &player1;
    return nullptr;
}


