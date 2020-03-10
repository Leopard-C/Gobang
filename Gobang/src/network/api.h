#ifndef API_H
#define API_H

#include <string>
#include <QString>
#include "client.h"

const int STATUS_OK = 0;
const int STATUS_ERROR = 1;

namespace API {

bool isTypeCommand(const Json::Value& root, const std::string& cmd_name);
bool isTypeNotify(const Json::Value& root, const std::string& sub_type);
bool isTypeResponse(const Json::Value& root, const std::string& res_cmd);

bool createRoom(Client* client, const QString& room_name, const QString& your_name);
bool joinRoom(Client* client, int room_id, const QString& your_name);
bool watchRoom(Client* client, int room_id, const QString& your_name);
bool exchangeChessType(Client* client);
bool prepareGame(Client* client, const QString& player_name);
bool cancelPrepareGame(Client* client, const QString& player_name);

bool responseExchageChessType(Client* client, bool yesOrNo);

bool notifyNewPiece(Client* client, int row, int col, int chess_type);
bool notifyRivalInfo(Client* client, const QString& player_name);
bool notifyGameOverWin(Client* client, int chess_type);
bool notifyGameOverDraw(Client* client);

bool sendChatMessage(Client* client, const QString& msg, const QString& sender);

}

#endif // API_H
