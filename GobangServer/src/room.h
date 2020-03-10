#pragma once

#include "base.h"
#include "thread_pool.h"
#include "player.h"

#include <string>
#include <vector>
#include "jsoncpp/json/json.h"

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

    void addPlayer(const std::string& name, int fd);
    void addWatcher(const std::string& name, int fd);
    void quitPlayer(int fd);
    void quitWatcher(int fd);

    Player& getPlayer1() { return player1; }
    Player& getPlayer2() { return player2; }
    Player* getPlayer(int fd);
    Player* getRival(int my_fd);

    bool shouldDelete() const { return flagShouldDelete; }

private:
    void setPiece(int row, int col, ChessType type);

    bool parseJsonMsg(const Json::Value& root, int fd);

    bool processMsgTypeCmd(const Json::Value& root, int fd);
    bool processMsgTypeResponse(const Json::Value& root, int fd);
    bool processMsgTypeChat(const Json::Value& root, int fd);
    bool processMsgTypeNotify(const Json::Value& root, int fd);

    bool processNotifyRivalInfo(const Json::Value& root, int fd);
    bool processPrepareGame(const Json::Value& root, int fd);
    bool processCancelPrepareGame(const Json::Value& root, int fd);
    bool processStartGame(const Json::Value& root, int fd);
    bool processNewPiece(const Json::Value& root, int fd);
    bool processGameOver(const Json::Value& root, int fd);
    bool processExchangeChessType(const Json::Value& root, int fd);

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

