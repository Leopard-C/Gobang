#ifndef PLAYER_H
#define PLAYER_H

#include <QString>

#include "base.h"


class Player
{
public:
    Player() {}
    Player(const QString& name, ChessType type, bool isMaster) :
        name(name), type(type), is_master(isMaster) {}

    void setName(const QString& newName) { name = newName; }
    void setType(ChessType newType) { type = newType; }
    void setMaster(bool val) { is_master = val; }
    const QString& getName() const { return name; }
    ChessType getType() const { return type; }
    bool isMaster() const { return is_master; }

private:
    QString name = "";
    ChessType type = CHESS_BLACK;
    bool is_master = false;
};

#endif // PLAYER_H
