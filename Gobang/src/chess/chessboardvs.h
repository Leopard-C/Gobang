#ifndef CHESSBOARDVS_H
#define CHESSBOARDVS_H

#include "chessboard.h"
#include "../network/client.h"


class ChessBoardVS : public ChessBoard
{
    Q_OBJECT
public:
    ChessBoardVS(QWidget* parent = nullptr);

public:
    void init();
    void setYourChessType(ChessType newType);
    void ignoreMouseEvent(bool on);
    ChessType getCurrChessType() const { return currentChess; }
    void setPiece(int row, int col);

signals:
    void sigSetPieceByCursor(int row, int col, int type);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;

private:
    ChessType currentChess = CHESS_BLACK;
    ChessType yourChess = CHESS_NULL;
    ChessType rivalChess = CHESS_NULL;

    QPoint startPos;
    QPoint endPos;
};

#endif // CHESSBOARDVS_H
