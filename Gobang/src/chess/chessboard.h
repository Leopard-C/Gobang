#ifndef CHESSBOARD_H
#define CHESSBOARD_H

#include <QPixmap>
#include <QLabel>
#include <QColor>

#include "base.h"
#include "../environment.h"

class ChessBoard : public QWidget
{
    Q_OBJECT
public:
    explicit ChessBoard(QWidget *parent = nullptr);

    enum { ROWS = 15 };
    enum { S_CONTINUE = -2, S_LOSE = -1, S_DRAW = 0, S_WIN = 1 };
    struct ChessPieceInfo {
        int row;
        int col;
        ChessType type;
    };

public:
    // clear all pieces
    void init();
    // or fill chessboard with specical pieces
    void init(int pieces[][ROWS], ChessPieceInfo lastPieceType);

    // not clear pieces, just repaint
    void flush();

    // place a piece
    void setPiece(int row, int col, ChessType type);

    // Set the style of background
    //    1. pure color
    //    2. picture
    BackgroundType getBgType() const { return bgType; }
    QPixmap getBgPicture() const { return bgPicture; }
    QColor getBgColor() const { return bgColor; }
    int getBgTransparency() const { return bgTransparency; }

    void setBgType(BackgroundType type)  { bgType = type; }
    void setBgPicture(const QPixmap& picture) { bgPicture = picture; }
    void setBgColor(const QColor& color) { bgColor = color; }
    void setBgTransparecny(int val) { bgTransparency = val; }

    // Get the image of chessboard, include chess pieces
    //
    QPixmap& getImage() { return pixmap; }

signals:
    void sigWin(int type);
    void sigDraw();

protected:
    void drawChessboard();
    void fillBackground();
    void drawPiece(int row, int col, ChessType type, bool highlight);   // (0 <= row, col <= ROWS)

    bool isValid(int row, int col);
    bool getRowCol(QPoint cursorPos, int& row, int& col);

    //int judge();
    int judge(int row, int col);
    bool judgeInRow(int row, int col, int chessType);
    bool judgeInCol(int row, int col, int chessType);
    bool judgeFromTopleftToBottomRight(int row, int col, int chessType);
    bool judgeFromBottomleftToTopright(int row, int col, int chessType);

protected:

    // Chess pieces
    int chessPieces[ROWS][ROWS];
    int numPieces = 0;

    const int gridWidth = 40;
    const int startX = 27;
    const int startY = 27;
    const int boardWidth = 5;
    const int pieceRadius = 13;
    const int EPS = 10;

    QPixmap pixmap;     // including background image(or color) and chess pieces
    QLabel* imgLabel;

    BackgroundType bgType = BG_PURE_COLOR;
    QColor bgColor = qRgb(200, 150, 100);
    QPixmap bgPicture;
    int bgTransparency = 220;

    ChessPieceInfo lastPiece;
};

#endif // CHESSBOARD_H
