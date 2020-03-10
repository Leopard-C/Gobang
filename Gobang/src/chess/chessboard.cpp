#include "chessboard.h"

#include <QMouseEvent>
#include <QPainter>
#include <QString>
#include <QDebug>

#include <cmath>


ChessBoard::ChessBoard(QWidget *parent) :
        QWidget(parent)
{
    imgLabel = new QLabel(this);
    imgLabel->setFixedSize(625, 625);
    this->setFixedSize(625, 625);

    pixmap = QPixmap();
    lastPiece = { 0, 0, CHESS_NULL };
    init();
}

void ChessBoard::init() {
    numPieces = 0;
    lastPiece = { 0, 0, CHESS_NULL };
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < ROWS; ++col) {
            chessPieces[row][col] = CHESS_NULL;
        }
    }
    drawChessboard();
}

void ChessBoard::init(int pieces[][ROWS], ChessPieceInfo lastPieceInfo) {
    numPieces = 0;
    lastPiece = lastPieceInfo;
    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < ROWS; ++col) {
            chessPieces[row][col] = pieces[row][col];
            if (chessPieces[row][col] != CHESS_NULL)
                numPieces++;
        }
    }
    flush();
}

void ChessBoard::flush() {
    drawChessboard();

    for (int row = 0; row < ROWS; ++row) {
        for (int col = 0; col < ROWS; ++col) {
            drawPiece(row, col, ChessType(chessPieces[row][col]), false);
        }
    }

    drawPiece(lastPiece.row, lastPiece.col, lastPiece.type, true);
    imgLabel->setPixmap(pixmap);
}

void ChessBoard::drawChessboard() {
    // Background
    //   1. pure color
    //   2. picture
    fillBackground();

    // Draw chess board
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(QPen(Qt::black, 1, Qt::SolidLine));

    int left = startX + boardWidth;
    int right = left + gridWidth * (ROWS - 1);
    int top = startY + boardWidth;
    int bottom = top + gridWidth * (ROWS - 1);

    for (int i = 0; i < ROWS; ++i) {
        // Horizontal lines
        painter.drawLine(left, top + gridWidth * i, right, top + gridWidth * i);
        // vertical lines
        painter.drawLine(left + gridWidth * i, top, left + gridWidth * i, bottom);

        // top & bottom  text (A, B, C, ... , S)
        painter.drawText(QPoint(left + gridWidth * i, startY * 2 / 3), QString('A' + i));
        painter.drawText(QPoint(left + gridWidth * i, bottom + boardWidth + startY / 2), QString('A' + i));

        // left & right  text (1, 2, 3, ... , 15)
        painter.drawText(QPoint(startX / 3, top + gridWidth * i), QString::number(1 + i));
        painter.drawText(QPoint(right + boardWidth + startX / 3, top + gridWidth * i), QString::number(1 + i));
    }

    // board internal lines
    left = startX;
    top = startY;
    right = left + boardWidth * 2 + gridWidth * (ROWS - 1);
    bottom = top + boardWidth * 2 + gridWidth * (ROWS - 1);
    painter.drawLine(left, top, right, top);
    painter.drawLine(right, top, right, bottom);
    painter.drawLine(left, bottom, right, bottom);
    painter.drawLine(left, top, left, bottom);

    // five control points
    painter.setPen(QColor(Qt::black));
    painter.setBrush(QBrush(Qt::black));
    int centerX = startX + boardWidth + (ROWS / 2) * gridWidth;
    int centerY = startY + boardWidth + (ROWS / 2) * gridWidth;
    painter.drawEllipse(QPoint(centerX, centerY), 4, 4);
    painter.drawEllipse(QPoint(centerX - 4 * gridWidth, centerY - 4 * gridWidth), 4, 4);
    painter.drawEllipse(QPoint(centerX + 4 * gridWidth, centerY - 4 * gridWidth), 4, 4);
    painter.drawEllipse(QPoint(centerX - 4 * gridWidth, centerY + 4 * gridWidth), 4, 4);
    painter.drawEllipse(QPoint(centerX + 4 * gridWidth, centerY + 4 * gridWidth), 4, 4);

    imgLabel->setPixmap(pixmap);
    painter.end();
}

// Place a piece
void ChessBoard::setPiece(int row, int col, ChessType type) {
    chessPieces[row][col] = type;

    drawPiece(lastPiece.row, lastPiece.col, lastPiece.type, false);
    drawPiece(row, col, type, true);

    lastPiece = { row, col, type };

    // judge
    if (type != CHESS_NULL) {
        int status = judge(row, col);
        if (status == S_WIN)
            emit sigWin(type);
        else if (status == S_DRAW)
            emit sigDraw();
    }
}

bool ChessBoard::isValid(int row, int col) {
    return (row >= 0 && row < ROWS && col >= 0 && col < ROWS &&
            chessPieces[row][col] == CHESS_NULL);
}

bool ChessBoard::getRowCol(QPoint cursorPos, int &row, int &col) {
    int left = startX + boardWidth;
    int right = left + gridWidth * (ROWS - 1);
    int top = startY + boardWidth;
    int bottom = top + gridWidth * (ROWS - 1);

    int x = cursorPos.x();
    int y = cursorPos.y();

    if (x < left - EPS || x > right + EPS)
        return false;
    if (y < top - EPS || y > bottom + EPS)
        return false;

    int countX = abs(x - left) / gridWidth;
    int countY = abs(y - top) / gridWidth;

    int gridLeft = left + countX * gridWidth;
    int gridRight = gridLeft + gridWidth;
    int gridTop = top + countY * gridWidth;
    int gridBottom = gridTop + gridWidth;

    if (abs(x - gridLeft) <= EPS) {
        if (abs(y - gridTop) <= EPS) {
            row = countY;
            col = countX;
            return true;
        }
        else if (abs(y - gridBottom) <= EPS) {
            row = countY + 1;
            col = countX;
            return true;
        }
        else {
            return false;
        }
    }
    else if (abs(x - gridRight) <= EPS) {
        if (abs(y - gridTop) <= EPS) {
            row = countY;
            col = countX + 1;
            return true;
        }
        else if (abs(y - gridBottom) <= EPS) {
            row = countY + 1;
            col = countX + 1;
            return true;
        }
        else {
            return false;
        }
    }
    else {
        return false;
    }
}


// Background
//   1. pure color
//   2. picture
void ChessBoard::fillBackground() {
    if (bgType == BG_PURE_COLOR) {
        pixmap = QPixmap(imgLabel->size());
        pixmap.fill(bgColor);
    }
    else {
        pixmap = bgPicture;
        QPixmap temp(pixmap.size());
        temp.fill(Qt::transparent);
        QPainter painter(&temp);

        painter.setCompositionMode(QPainter::CompositionMode_Source);
        painter.drawPixmap(0, 0, pixmap);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        painter.fillRect(temp.rect(), QColor(0, 0, 0, bgTransparency));
        painter.end();

        pixmap = temp.scaled(625, 625, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        imgLabel->setScaledContents(true);
    }

    imgLabel->setPixmap(pixmap);
}


void ChessBoard::drawPiece(int row, int col, ChessType type, bool highlight) {
    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    if (type == CHESS_NULL)
        return;
    else if (type == CHESS_BLACK) {
        painter.setPen(Qt::black);
        painter.setBrush(Qt::black);
    }
    else {
        painter.setPen(Qt::white);
        painter.setBrush(Qt::white);
    }

    int x = startX + boardWidth + gridWidth * col;
    int y = startY + boardWidth + gridWidth * row;

    // chess piece
    painter.drawEllipse(QPoint(x, y), pieceRadius, pieceRadius);

    // highlight: draw a cross-shape
    if (highlight) {
        QPen pen;
        pen.setWidth(3);
        if (type == CHESS_BLACK)
            pen.setColor(Qt::white);
        else
            pen.setColor(Qt::black);
        painter.setPen(pen);
        painter.drawLine(x + 4, y, x + 6, y);
        painter.drawLine(x - 4, y, x - 6, y);
        painter.drawLine(x, y + 4, x, y + 6);
        painter.drawLine(x, y - 4, x, y - 6);
    }

    painter.end();
    imgLabel->setPixmap(pixmap);
    numPieces++;
}

int ChessBoard::judge(int row, int col) {
    int chessType = chessPieces[row][col];

    bool ret= (judgeInRow(row, col, chessType) ||
               judgeInCol(row, col, chessType) ||
               judgeFromTopleftToBottomRight(row, col, chessType) ||
               judgeFromBottomleftToTopright(row, col, chessType)
              );

    if (ret)
        return S_WIN;
    if (numPieces == ROWS * ROWS)
        return S_DRAW;
    else
        return S_CONTINUE;
}

//int ChessBoard::judge() {
//
//    return 0;
//}

// Five in row
bool ChessBoard::judgeInRow(int row, int col, int chessType) {
    int count = 1;
    int l = col - 1, r = col + 1;
    while (l > -1 || r < ROWS) {
        if (l > -1) {
            if (chessPieces[row][l] == chessType) {
                count++;
                l--;
            }
            else {
                l = -1;
            }
        }
        if (r < ROWS) {
            if (chessPieces[row][r] == chessType) {
                count++;
                r++;
            }
            else {
                r = ROWS;
            }
        }
    }

    return count >= 5;
}

// Five in column
bool ChessBoard::judgeInCol(int row, int col, int chessType) {
    int count = 1;
    int t = row - 1, b = row + 1;
    while (t > -1 || b < ROWS) {
        if (t > -1) {
            if (chessPieces[t][col] == chessType) {
                count++;
                t--;
            }
            else {
                t = -1;
            }
        }
        if (b < ROWS) {
            if (chessPieces[b][col] == chessType) {
                count++;
                b++;
            }
            else {
                b = ROWS;
            }
        }
    }

    return count >= 5;
}

// topLeft -> bottomRight
bool ChessBoard::judgeFromTopleftToBottomRight(int row, int col, int chessType) {
    int count = 1;
    int l = col - 1, r = col + 1;
    int t = row - 1, b = row + 1;
    bool bTL = (t > -1 && l > -1);
    bool bBR = (b < ROWS && r < ROWS);

    while (bTL || bBR) {
        if (bTL) {
            if (chessPieces[t][l] == chessType) {
                count++;
                t--;
                l--;
            }
            else {
                t = -1;
            }
        }
        if (bBR) {
            if (chessPieces[b][r] == chessType) {
                count++;
                b++;
                r++;
            }
            else {
                b = ROWS;
            }
        }
        bTL = (t > -1 && l > -1);
        bBR = (b < ROWS && r < ROWS);
    }

    return count >= 5;
}

// bottomLeft -> topRight
bool ChessBoard::judgeFromBottomleftToTopright(int row, int col, int chessType) {
    int count = 1;
    int l = col - 1, r = col + 1;
    int t = row - 1, b = row + 1;
    bool bBL = (b > -1 && l > -1);
    bool bTR = (t < ROWS && r < ROWS);

    while (bBL || bTR) {
        if (bBL) {
            if (chessPieces[b][l] == chessType) {
                count++;
                b++;
                l--;
            }
            else {
                b = -1;
            }
        }
        if (bTR) {
            if (chessPieces[t][r] == chessType) {
                count++;
                t--;
                r++;
            }
            else {
                t = ROWS;
            }
        }
        bBL = (b > -1 && l > -1);
        bTR = (t < ROWS && r < ROWS);
    }

    return count >= 5;
}
