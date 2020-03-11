#include <string>
#include "jsoncpp/json/json.h"
#include "socket_func.h"
#include "base.h"

const int STATUS_OK = 0;
const int STATUS_ERROR = 1;

namespace API {

bool isTypeCommand(const Json::Value& root, const std::string& cmd);
bool isTypeNotify(const Json::Value& root, const std::string& sub_type);

bool forward(SocketFD fd, const Json::Value& root);

// Type: Response
bool responseCreateRoom(SocketFD fd, int status_code, const std::string& desc, int room_id);
bool responseJoinRoom(SocketFD fd, int status_code, const std::string& desc,
        const std::string room_name, const std::string rival_name);
bool responseWatchRoom(SocketFD fd, int status_code, const std::string& desc,
        const std::string room_name);
bool responsePrepare(SocketFD fd, int status_code, const std::string& desc);

// Type: command
bool sendChessBoard(SocketFD fd, int chessPieces[][15], ChessPieceInfo last_piece);

// Tyep: Notify
bool notifyRivalInfo(SocketFD fd, const std::string& player_name);
bool notifyNewPiece(SocketFD fd, int row, int col, int chess_type);
bool notifyGameStart(SocketFD fd);
bool notifyGameCancelPrepare(SocketFD fd);
bool notifyDisconnect(SocketFD fd, const std::string& player_name);
bool notifyPlayerInfo(SocketFD fd, const std::string& player1_name, int player1_chess_type,
        const std::string& player2_name, int player2_chess_type);

};
