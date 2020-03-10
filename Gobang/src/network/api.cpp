#include "api.h"

#include <json/json.h>

namespace API {

/***********************
 * Local functions
***********************/
static bool sendSimpleCmd(Client* client, const std::string& cmd) {
    Json::Value root;
    root["type"] = "command";
    root["cmd"] = cmd;
    return client->sendJsonMsg(root);
}

/******************************************
 * Check recieved message's type
******************************************/
bool isTypeCommand(const Json::Value& root, const std::string& cmd_name) {
    return !root["type"].isNull() && root["type"].asString() == "command" &&
           !root["cmd"].isNull() && root["cmd"].asString() == cmd_name;
}

bool isTypeNotify(const Json::Value& root, const std::string& sub_type) {
    return !root["type"].isNull() && root["type"].asString() == "notify" &&
           !root["sub_type"].isNull() && root["sub_type"].asString() == sub_type;
}

bool isTypeResponse(const Json::Value& root, const std::string& res_cmd) {
    return !root["type"].isNull() && root["type"].asString() == "response" &&
           !root["res_cmd"].isNull() && root["res_cmd"].asString() == res_cmd &&
           !root["status"].isNull();
}

bool createRoom(Client* client, const QString& room_name, const QString& your_name) {
    Json::Value root;
    root["type"] = "command";
    root["cmd"] = "create_room";
    root["room_name"] = room_name.toStdString();
    root["player_name"] = your_name.toStdString();
    return client->sendJsonMsg(root);
}

bool joinRoom(Client* client, int room_id, const QString& your_name) {
    Json::Value root;
    root["type"] = "command";
    root["cmd"] = "join_room";
    root["room_id"] = room_id;
    root["player_name"] = your_name.toStdString();
    return client->sendJsonMsg(root);
}

bool watchRoom(Client* client, int room_id, const QString& your_name) {
    Json::Value root;
    root["type"] = "command";
    root["cmd"] = "watch_room";
    root["room_id"] = room_id;
    root["player_name"] = your_name.toStdString();
    return client->sendJsonMsg(root);
}

bool exchangeChessType(Client* client) {
    return sendSimpleCmd(client, "exchange");
}

bool prepareGame(Client* client, const QString& player_name) {
    Json::Value root;
    root["type"] = "command";
    root["cmd"] = "prepare";
    root["player_name"] = player_name.toStdString();
    return client->sendJsonMsg(root);
}

bool cancelPrepareGame(Client* client, const QString& player_name) {
    Json::Value root;
    root["type"] = "command";
    root["cmd"] = "cancel_prepare";
    root["player_name"] = player_name.toStdString();
    return client->sendJsonMsg(root);
}


/***********************
 * Type: Response
***********************/
bool responseExchageChessType(Client* client, bool yesOrNo) {
    Json::Value root;
    root["type"] = "response";
    root["res_cmd"] = "exchange";
    root["accept"] = yesOrNo;
    return client->sendJsonMsg(root);
}

/*************************
 * Type: Notify
*************************/
bool notifyNewPiece(Client* client, int row, int col, int chess_type) {
    Json::Value root;
    root["type"] = "notify";
    root["sub_type"] = "new_piece";
    root["row"] = row;
    root["col"] = col;
    root["chess_type"] = chess_type;
    return client->sendJsonMsg(root);
}

bool notifyRivalInfo(Client* client, const QString& player_name) {
    Json::Value root;
    root["type"] = "notify";
    root["sub_type"] = "rival_info";
    root["player_name"] = player_name.toStdString();
    return client->sendJsonMsg(root);
}

bool notifyGameOverWin(Client* client, int chess_type) {
    Json::Value root;
    root["type"] = "notify";
    root["sub_type"] = "game_over";
    root["game_result"] = "win";
    root["chess_type"] = chess_type;
    return client->sendJsonMsg(root);
}

bool notifyGameOverDraw(Client* client) {
    Json::Value root;
    root["type"] = "notify";
    root["sub_type"] = "game_over";
    root["game_result"] = "draw";
    root["chess_type"] = 0;     // 0 == CHESS_NULL
    return client->sendJsonMsg(root);
}

bool sendChatMessage(Client* client, const QString& msg, const QString& sender) {
    Json::Value root;
    root["type"] = "chat";
    root["message"] = msg.toStdString();
    root["sender"] = sender.toStdString();
    return client->sendJsonMsg(root);
}

} // namespace API
