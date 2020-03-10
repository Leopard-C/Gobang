#include "chessboardvs.h"
#include <QMouseEvent>
#include <QDebug>

ChessBoardVS::ChessBoardVS(QWidget* parent) :
    ChessBoard(parent)
{
}

void ChessBoardVS::init() {
    ChessBoard::init();
    currentChess = CHESS_BLACK;
}

// Ignore mouse event
// game start: fasle
// game end:   true
void ChessBoardVS::ignoreMouseEvent(bool on) {
    this->setAttribute(Qt::WA_TransparentForMouseEvents, on);
}

void ChessBoardVS::mousePressEvent(QMouseEvent *event) {
    startPos = event->pos();
}

void ChessBoardVS::mouseReleaseEvent(QMouseEvent *event) {
    if (currentChess != yourChess)
        return;

    endPos = event->pos();
    if ((endPos - startPos).manhattanLength() <= gridWidth) {
        int row, col;
        if (getRowCol(endPos, row, col)) {
            emit sigSetPieceByCursor(row, col, yourChess);
            ChessBoard::setPiece(row, col, yourChess);
            currentChess = rivalChess;
        }
    }
}

void ChessBoardVS::setPiece(int row, int col) {
    ChessBoard::setPiece(row, col, rivalChess);
    currentChess = yourChess;
}

void ChessBoardVS::setYourChessType(ChessType newType) {
    yourChess = newType;
    rivalChess = reverse(newType);
}
