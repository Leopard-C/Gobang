#include <string>
#include "jsoncpp/json/json.h"
#include "base.h"

const int STATUS_OK = 0;
const int STATUS_ERROR = 1;

namespace API {

bool isTypeCommand(const Json::Value& root, const std::string& cmd);
bool isTypeNotify(const Json::Value& root, const std::string& sub_type);

bool forward(int fd, const Json::Value& root);

// Type: Response
bool responseCreateRoom(int fd, int status_code, const std::string& desc, int room_id);
bool responseJoinRoom(int fd, int status_code, const std::string& desc,
        const std::string room_name, const std::string rival_name);
bool responseWatchRoom(int fd, int status_code, const std::string& desc,
        const std::string room_name);
bool responsePrepare(int fd, int status_code, const std::string& desc);

// Type: command
bool sendChessBoard(int fd, int chessPieces[][15], ChessPieceInfo last_piece);

// Tyep: Notify
bool notifyRivalInfo(int fd, const std::string& player_name);
bool notifyNewPiece(int fd, int row, int col, int chess_type);
bool notifyGameStart(int fd);
bool notifyGameCancelPrepare(int fd);
bool notifyDisconnect(int fd, const std::string& player_name);
bool notifyPlayerInfo(int fd, const std::string& player1_name, int player1_chess_type,
        const std::string& player2_name, int player2_chess_type);

};
