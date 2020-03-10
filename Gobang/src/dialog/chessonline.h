#ifndef CHESSONLINE_H
#define CHESSONLINE_H

#include <QDialog>
#include <QLabel>
#include <QPixmap>
#include <QPushButton>
#include <QTextEdit>

#include "../chess/chessboardvs.h"
#include "../chess/player.h"
#include "../network/client.h"
#include "../widget/chathistory.h"
#include "settingpanel.h"


class ChessOnline : public QDialog
{
    Q_OBJECT
public:
    ChessOnline(Player* you, Player* rival, QWidget* parent = nullptr);
    ~ChessOnline();

public:
    void setClient(Client* clientIn);
    void setRoomName(const QString& nameIn) { roomName = nameIn; }
    void setRoomId(int idIn) { roomId = idIn; }
    void updateInfo();

    // Using for multi-thread
    // QWidget(and subclassed) should only be used or modified in GUI thread
    // The program may crash if QWidget was used in other thread
    // This problem can be solved by using "signals and slots"
signals:
    void sigCritical(QWidget* parent, const QString& title, const QString& text, int button);
    void sigInformation(QWidget* parent, const QString& title, const QString& text, int button);
    void sigProcessMsgTypeCommand(const Json::Value& root);
    void sigProcessMsgTypeNotify(const Json::Value& root);
    void sigProcessMsgTypeResponse(const Json::Value& root);
    void sigProcessMsgTypeChat(const Json::Value& root);

public slots:
    void onStart();
    void onCopyToClipboard();
    void onShowSettingPanel();
    void onScreenshot();
    void onSetPieceByCursor(int row, int col, int type);
    void onGameWin(int type);
    void onGameDraw();
    void onSettingsChanged(const SettingsInfo& settings);
    void onExchangeChessType();

protected:
    virtual void closeEvent(QCloseEvent *) override;
    virtual bool eventFilter(QObject *watched, QEvent *event) override;
    virtual void keyPressEvent(QKeyEvent* event) override;

private:
    void setupLayout();
    void startRecvMsg();
    void changeTurn();
    void gameOver();
    void exchangeChessType();


    /*******************************
     * Parse and process recv msg
    *******************************/
public:
    void parseJsonMsg(const Json::Value& root);

public slots:
    void processMsgTypeCommand(const Json::Value& root);
    void processMsgTypeNotify(const Json::Value& root);
    void processMsgTypeResponse(const Json::Value& root);
    void processMsgTypeChat(const Json::Value& root);

private:
    enum GameStatus { GAME_RUNNING = 1, GAME_PREPARE = 2, GAME_END = 3 };

    GameStatus gameStatus = GAME_END;
    bool activeDisconnect = false;

    ChessBoardVS* chessBoard = nullptr;
    Player* playerYou = nullptr;
    Player* playerRival = nullptr;
    Client* client = nullptr;
    QString roomName;
    int roomId;

    QPushButton* btnSetting = nullptr;
    QPushButton* btnStart = nullptr;
    QPushButton* btnScreenshot = nullptr;
    QPushButton* btnExchange = nullptr;

    QLabel* labelRoomId;
    QLabel* labelRoomName;
    QLabel* labelServerIp;
    QLabel* labelServerPort;
    QLabel* labelYourName1;
    QLabel* labelYourName2;
    QLabel* labelRivalName;
    QLabel* labelYourTurn;
    QLabel* labelRivalTurn;

    QPixmap whiteChess;
    QPixmap blackChess;
    QPixmap inPreparation;
    QPixmap sameColorWithBg;
    QPixmap rivalDisconnect;
    QPixmap* yourChess;
    QPixmap* rivalChess;

    bool isQuit = false;
    bool isForceQuit = false;
    bool isYourTurn = false;

    ChatHistory* chatHistory;
    QTextEdit* chatInput;
};

#endif // CHESSONLINE_H
