#ifndef BASE_H
#define BASE_H

enum ChessType {
    CHESS_BLACK = -1,
    CHESS_NULL = 0,
    CHESS_WHITE = 1
};

enum BackgroundType {
    BG_PURE_COLOR = 1,
    BG_PICTURE = 2
};

ChessType reverse(ChessType typeIn);


#endif // BASE_H
