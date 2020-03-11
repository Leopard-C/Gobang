#pragma once

#include "base.h"
#include "player.h"
#include "socket_func.h"
#include "thread_pool.h"
#include "jsoncpp/json/json.h"

#include <string>
#include <vector>

#define MAX_NUM_WATCHERS 20


class Room {
public:
    Room();
    ~Room() {}

public:
    void initChessBoard();

    int getId() const { return id; }
    void setId(int idIn) { id = idIn; }
    const std::string& getName() const { return name; }
    void setName(const std::string& nameIn) { name = nameIn; }

    int getNumPlayers() const { return numPlayers; }
    int getNumWatchers() const { return watchers.size(); }
    bool isFull() const { return watchers.size() >= MAX_NUM_WATCHERS; }

    void addPlayer(const std::string& name, SocketFD fd);
    void addWatcher(const std::string& name, SocketFD fd);
    void quitPlayer(SocketFD fd);
    void quitWatcher(SocketFD fd);

    Player& getPlayer1() { return player1; }
    Player& getPlayer2() { return player2; }
    Player* getPlayer(SocketFD fd);
    Player* getRival(int my_fd);

    bool shouldDelete() const { return flagShouldDelete; }

private:
    void setPiece(int row, int col, ChessType type);

    bool parseJsonMsg(const Json::Value& root, SocketFD fd);

    bool processMsgTypeCmd(const Json::Value& root, SocketFD fd);
    bool processMsgTypeResponse(const Json::Value& root, SocketFD fd);
    bool processMsgTypeChat(const Json::Value& root, SocketFD fd);
    bool processMsgTypeNotify(const Json::Value& root, SocketFD fd);

    bool processNotifyRivalInfo(const Json::Value& root, SocketFD fd);
    bool processPrepareGame(const Json::Value& root, SocketFD fd);
    bool processCancelPrepareGame(const Json::Value& root, SocketFD fd);
    bool processStartGame(const Json::Value& root, SocketFD fd);
    bool processNewPiece(const Json::Value& root, SocketFD fd);
    bool processGameOver(const Json::Value& root, SocketFD fd);
    bool processExchangeChessType(const Json::Value& root, SocketFD fd);

    enum GameStatus {
        GAME_RUNNING = 1,
        GAME_PREPARE = 2,
        GAME_END = 3
    };

private:
    ThreadPool pool;
    bool flagShouldDelete = false;

    int chessPieces[15][15];
    GameStatus gameStatus = GAME_END;
    ChessPieceInfo lastChess;

    int id = -1;
    std::string name;
    int numPlayers = 0;

    Player player1;
    Player player2;

    std::vector<Watcher> watchers;
};

