#include "api.h"
#include "socket_func.h"

#include "jsoncpp/json/writer.h"
#include <iostream>

namespace API {


/***********************
 * Local functions
***********************/ static bool sendSimpleCmd(SocketFD fd, const std::string& cmd) { Json::Value root;
    root["type"] = "command";
    root["cmd"] = cmd;
    return sendJsonMsg(root, fd);
}

static bool sendSimpleRes(SocketFD fd, const std::string& res_cmd,
        int status_code, const std::string desc) {
    Json::Value root;
    root["type"] = "response";
    root["res_cmd"] = res_cmd;
    root["status"] = status_code;
    root["desc"] = desc;
    return sendJsonMsg(root, fd);
}

static bool sendSimpleNotify(SocketFD fd, const std::string& sub_type) {
    Json::Value root;
    root["type"] = "notify";
    root["sub_type"] = sub_type;
    return sendJsonMsg(root, fd);
}


/******************************************
 * Check recieved message's type
******************************************/
bool isTypeCommand(const Json::Value& root, const std::string& cmd) {
    return !root["type"].isNull() && root["type"].asString() == "command"
        && !root["cmd"].isNull() && root["cmd"].asString() == cmd;
}

bool isTypeNotify(const Json::Value& root, const std::string& sub_type) {
    return !root["type"].isNull() && root["type"].asString() == "notify"
        && !root["sub_type"].isNull() && root["sub_type"] == sub_type;
}


/**************************************
 * Just forward to the other player 
**************************************/
bool forward(SocketFD fd, const Json::Value& root) {
    return sendJsonMsg(root, fd);
}


/***********************
 * Type: Response 
***********************/
bool responseCreateRoom(SocketFD fd, int status_code, const std::string& desc, int room_id) {
    Json::Value root;
    root["type"] =  "response";
    root["res_cmd"] = "create_room";
    root["status"] = status_code;
    root["desc"] = desc;
    root["room_id"] = room_id;
    return sendJsonMsg(root, fd);
}

bool responseJoinRoom(SocketFD fd, int status_code, const std::string &desc,
        const std::string room_name, const std::string rival_name) {
    Json::Value root;
    root["type"] = "response";
    root["res_cmd"] = "join_room";
    root["status"] = status_code;
    root["desc"] = desc; 
    root["room_name"] = room_name;
    root["rival_name"] = rival_name;
    return sendJsonMsg(root, fd);
}

bool responseWatchRoom(SocketFD fd, int status_code, const std::string& desc,
        const std::string room_name) {
    Json::Value root;
    root["type"] = "response";
    root["res_cmd"] = "watch_room";
    root["status"] = status_code;
    root["desc"] = desc; 
    root["room_name"] = room_name;
    return sendJsonMsg(root, fd);
}

bool responsePrepare(SocketFD fd, int status_code, const std::string& desc) {
    return sendSimpleRes(fd, "prepare", status_code, desc);
}


/*************************
 * Type: Notify
*************************/
bool sendChessBoard(SocketFD fd, int chessPieces[][15], ChessPieceInfo last_piece) {
    Json::Value root;
    Json::Value chessboard;
    Json::Value lastChess;
    root["type"] = "notify";
    root["sub_type"] = "chessboard";

    for (int row = 0; row < 15; ++row) {
        for (int col = 0; col < 15; ++col) {
            chessboard.append(chessPieces[row][col]);
        }
    }
    root["layout"] = chessboard;

    lastChess["row"] = last_piece.row;
    lastChess["col"] = last_piece.col;
    lastChess["type"] = last_piece.type;
    root["last_piece"] = lastChess;

    return sendJsonMsg(root, fd);
}

bool notifyRivalInfo(SocketFD fd, const std::string& player_name) {
    Json::Value root;
    root["type"] = "notify";
    root["sub_type"] = "rival_info";
    root["player_name"] = player_name;
    return sendJsonMsg(root, fd);
}

bool notifyNewPiece(SocketFD fd, int row, int col, int chess_type) {
    Json::Value root;
    root["type"] = "notify";
    root["sub_type"] = "new_piece";
    root["row"] = row;
    root["col"] = col;
    root["chess_type"] = chess_type;
    return sendJsonMsg(root, fd);
}

bool notifyGameStart(SocketFD fd) {
    return sendSimpleNotify(fd, "game_start");
}

bool notifyGameCancelPrepare(SocketFD fd) {
    return sendSimpleNotify(fd, "cancel_prepare");
}

bool notifyDisconnect(SocketFD fd, const std::string& player_name) {
    Json::Value root;
    root["type"] = "notify";
    root["sub_type"] = "disconnect";
    root["player_name"] = player_name;
    return sendJsonMsg(root, fd);
}

bool notifyPlayerInfo(SocketFD fd, const std::string& player1_name, int player1_chess_type,
        const std::string& player2_name, int player2_chess_type) {
    Json::Value root;
    root["type"] = "notify";
    root["sub_type"] = "player_info";
    root["player1_name"] = player1_name;
    root["player1_chess_type"] = player1_chess_type;
    root["player2_name"] = player2_name;
    root["player2_chess_type"] = player2_chess_type;
    return sendJsonMsg(root, fd);
}


}; // namespace API

