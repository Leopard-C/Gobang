#pragma once

enum ChessType {
    CHESS_BLACK = -1,
    CHESS_NULL = 0,
    CHESS_WHITE = 1
};

struct ChessPieceInfo {
    int row;
    int col;
    int type;
};

ChessType reverse(ChessType typeIn);

